//==============================================================//
// Copyright (C) 2015, California Institute of Technology.      //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#define L2A_TB_FILE_KEYWORD "L2A_TB_FILE"
#define L2B_TB_FILE_KEYWORD "L2B_TB_FILE"
#define TB_FLAT_MODEL_FILE_KEYWORD "TB_FLAT_MODEL_FILE"
#define TB_ROUGH_MODEL_FILE_KEYWORD "TB_ROUGH_MODEL_FILE"
#define S0_ROUGH_MODEL_FILE_KEYWORD "S0_ROUGH_MODEL_FILE"
#define ANC_SSS_FILE_KEYWORD "ANC_SSS_FILE"
#define ANC_SST_FILE_KEYWORD "ANC_SST_FILE"
#define ANC_SWH_FILE_KEYWORD "ANC_SWH_FILE"
#define ANC_U10_FILE_KEYWORD "ANC_U10_FILE"
#define ANC_V10_FILE_KEYWORD "ANC_V10_FILE"
#define FILL_VALUE -9999

//----------//
// INCLUDES //
//----------//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <vector>
#include "hdf5.h"
#include "hdf5_hl.h"
#include "List.h"
#include "BufferedList.h"
#include "Misc.h"
#include "ConfigList.h"
#include "L2A.h"
#include "Kp.h"
#include "ConfigSim.h"
#include "L2B.h"
#include "L2AToL2B.h"
#include "Tracking.h"
#include "Array.h"
#include "Meas.h"
#include "CAPGMF.h"
#include "CAPWind.h"
#include "CAPWindSwath.h"
#include "GMF.h"
#include "Constants.h"


//-----------//
// TEMPLATES //
//-----------//

// Class declarations needed for templates
// eliminates need to include the entire header file
class AngleInterval;

template class List<StringPair>;
template class List<Meas>;
template class List<EarthPosition>;
template class List<WindVectorPlus>;
template class List<MeasSpot>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<off_t>;
template class List<OffsetList>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;
template class List<AngleInterval>;
template class std::list<string>;
template class std::map<string,string,Options::ltstr>;

//--------------//
// MAIN PROGRAM //
//--------------//

int main(int argc, char* argv[]) {

    const char* command = no_path(argv[0]);
    char* config_file = argv[1];

    float dtbv = 0;
    float dtbh = 0;
    if(argc == 4) {
        dtbv = atof(argv[2]);
        dtbh = atof(argv[3]);
    }

    ConfigList config_list;
    if(!config_list.Read(config_file)) {
        fprintf(
            stderr, "%s: error reading config file %s\n", command, config_file);
        exit(1);
    }

    // Get swath grid size
    int is_25km = 0;
    int along_track_resolution;
    config_list.GetInt("ALONGTRACK_RESOLUTION", &along_track_resolution);
    if(along_track_resolution == 25)
        is_25km = 1;

    // Get filenames from config file
    config_list.ExitForMissingKeywords();
    char* l2a_tb_file = config_list.Get(L2A_TB_FILE_KEYWORD);
    char* l2b_tb_file = config_list.Get(L2B_TB_FILE_KEYWORD);
    char* tb_flat_file = config_list.Get(TB_FLAT_MODEL_FILE_KEYWORD);
    char* tb_rough_file = config_list.Get(TB_ROUGH_MODEL_FILE_KEYWORD);
    char* s0_rough_file = config_list.Get(S0_ROUGH_MODEL_FILE_KEYWORD);
    char* anc_sss_file = config_list.Get(ANC_SSS_FILE_KEYWORD);
    char* anc_sst_file = config_list.Get(ANC_SST_FILE_KEYWORD);
    char* anc_swh_file = config_list.Get(ANC_SWH_FILE_KEYWORD);
    char* anc_u10_file = config_list.Get(ANC_U10_FILE_KEYWORD);
    char* anc_v10_file = config_list.Get(ANC_V10_FILE_KEYWORD);

    // Configure the model functions
    CAPGMF cap_gmf;
    cap_gmf.ReadFlat(tb_flat_file);
    cap_gmf.ReadRough(tb_rough_file);
    cap_gmf.ReadModelS0(s0_rough_file);

    L2A l2a_tb;
    l2a_tb.SetInputFilename(l2a_tb_file);

    l2a_tb.OpenForReading();
    l2a_tb.ReadHeader();

    int ncti = l2a_tb.header.crossTrackBins;
    int nati = l2a_tb.header.alongTrackBins;

    // Read in L2A TB file
    L2AFrame*** l2a_tb_swath;
    l2a_tb_swath = (L2AFrame***)make_array(sizeof(L2AFrame *), 2, ncti, nati);
    while(l2a_tb.ReadDataRec()) {
        L2AFrame* this_frame = new L2AFrame();
        this_frame->CopyFrame(this_frame, &l2a_tb.frame);
         *(*(l2a_tb_swath + this_frame->cti) + this_frame->ati) = this_frame;
    }
    l2a_tb.Close();

    CAP_ANC_L2B cap_anc_sss(anc_sss_file);
    CAP_ANC_L2B cap_anc_sst(anc_sst_file);
    CAP_ANC_L2B cap_anc_swh(anc_swh_file);
    CAP_ANC_L2B cap_anc_u10(anc_u10_file);
    CAP_ANC_L2B cap_anc_v10(anc_v10_file);

    // Output arrays
    int l2b_size = ncti * nati;
    std::vector<float> lat(l2b_size), lon(l2b_size);
    std::vector<float> tb_h_fore(l2b_size), tb_h_aft(l2b_size);
    std::vector<float> tb_v_fore(l2b_size), tb_v_aft(l2b_size);
    std::vector<float> inc_fore(l2b_size), inc_aft(l2b_size);
    std::vector<float> azi_fore(l2b_size), azi_aft(l2b_size);
    std::vector<float> anc_spd(l2b_size), anc_dir(l2b_size);
    std::vector<float> anc_sss(l2b_size), anc_sst(l2b_size), anc_swh(l2b_size);
    std::vector<float> tb_sss(l2b_size), tb_spd(l2b_size);
    std::vector<uint16> tb_flg(l2b_size);

    for(int ati=0; ati<nati; ++ati) {
        if(ati%50 == 0)
            fprintf(stdout, "%d of %d\n", ati, nati);

        for(int cti=0; cti<ncti; ++cti) {

            int l2bidx = ati + nati*cti;

            // Hack to index into (approx) correct location in ancillary files.
            int anc_cti = (is_25km) ? cti*2 : cti;
            int anc_ati = (is_25km) ? ati*2 : ati;

            // Initialize to fill values
            lon[l2bidx] = FILL_VALUE;
            lat[l2bidx] = FILL_VALUE;
            tb_h_fore[l2bidx] = FILL_VALUE;
            tb_h_aft[l2bidx] = FILL_VALUE;
            tb_v_fore[l2bidx] = FILL_VALUE;
            tb_v_aft[l2bidx] = FILL_VALUE;
            inc_fore[l2bidx] = FILL_VALUE;
            inc_aft[l2bidx] = FILL_VALUE;
            azi_fore[l2bidx] = FILL_VALUE;
            azi_aft[l2bidx] = FILL_VALUE;
            anc_spd[l2bidx] = FILL_VALUE;
            anc_dir[l2bidx] = FILL_VALUE;
            anc_sss[l2bidx] = FILL_VALUE;
            anc_sst[l2bidx] = FILL_VALUE;
            anc_swh[l2bidx] = FILL_VALUE;
            tb_sss[l2bidx] = FILL_VALUE;
            tb_spd[l2bidx] = FILL_VALUE;
            tb_flg[l2bidx] = 65535;

            // Check for valid measList at this WVC
            if(!l2a_tb_swath[cti][ati])
                continue;

            // Check for useable ancillary data
            if(cap_anc_sst.data[anc_ati][anc_cti][0] < 0)
                continue;

            MeasList* tb_ml = &(l2a_tb_swath[cti][ati]->measList);

            LonLat lonlat = tb_ml->AverageLonLat();

            lon[l2bidx] = rtd * lonlat.longitude;
            lat[l2bidx] = rtd * lonlat.latitude;

            // wrap to [-180, 180) interval
            while(lon[l2bidx]>=180) lon[l2bidx] -= 360;
            while(lon[l2bidx]<-180) lon[l2bidx] += 360;

            // Average the tbs and inc/azimuth angle over the fore/aft looks.
            float sum_tb[2][2]; // [fore-0, aft-1][v-0, h-1]
            float sum_A[2][2];
            int cnts[2][2];

            float sum_cos_azi[2]; // fore - 0; aft 1
            float sum_sin_azi[2];
            float sum_inc[2];

            for(int ilook = 0; ilook < 2; ++ilook) {
                sum_inc[ilook] = 0;
                sum_sin_azi[ilook] = 0;
                sum_cos_azi[ilook] = 0;
                for(int ipol = 0; ipol < 2; ++ipol) {
                    sum_tb[ilook][ipol] = 0;
                    sum_A[ilook][ipol] = 0;
                    cnts[ilook][ipol] = 0;
                }
            }

            for(Meas* meas = tb_ml->GetHead(); meas; meas = tb_ml->GetNext()) {

                // Skip land flagged observations
                if(meas->landFlag)
                    continue;

                int idx_look = 0;
                int idx_pol = -1;

                if(meas->scanAngle > pi/2 && meas->scanAngle < 3*pi/2)
                    idx_look = 1;

                if(meas->measType == Meas::L_BAND_TBV_MEAS_TYPE) {
                    idx_pol = 0;
                } else if(meas->measType == Meas::L_BAND_TBH_MEAS_TYPE) {
                    idx_pol = 1;
                } else {
                    continue;
                }

                cnts[idx_look][idx_pol]++;
                sum_tb[idx_look][idx_pol] += meas->value;
                sum_A[idx_look][idx_pol] += meas->A;
                sum_inc[idx_look] += meas->incidenceAngle;
                sum_cos_azi[idx_look] += cos(meas->eastAzimuth);
                sum_sin_azi[idx_look] += sin(meas->eastAzimuth);
            }

            // Compute averaged TB observations from four looks
            // Angles computed in degrees and clockwise from north
            int cnts_fore = cnts[0][0] + cnts[0][1];
            if(cnts_fore>0) {
                inc_fore[l2bidx] = rtd*sum_inc[0]/(float)cnts_fore;
                azi_fore[l2bidx] = rtd*atan2(sum_cos_azi[0], sum_sin_azi[0]);
            }

            int cnts_aft = cnts[1][0] + cnts[1][1];
            if(cnts_aft>0) {
                inc_aft[l2bidx] = rtd*sum_inc[1]/(float)cnts_aft;
                azi_aft[l2bidx] = rtd*atan2(sum_cos_azi[1], sum_sin_azi[1]);
            }

            MeasList tb_ml_avg;
            if(cnts[0][0]) {
                tb_v_fore[l2bidx] = sum_tb[0][0]/(float)cnts[0][0];

                Meas* this_meas = new Meas();
                this_meas->value = tb_v_fore[l2bidx] - dtbv;
                this_meas->measType = Meas::L_BAND_TBV_MEAS_TYPE;
                this_meas->incidenceAngle = dtr * inc_fore[l2bidx];
                this_meas->eastAzimuth = gs_deg_to_pe_rad(azi_fore[l2bidx]);
                this_meas->A = sum_A[0][0]/pow((float)cnts[0][0], 2);
                tb_ml_avg.Append(this_meas);
            }

            if(cnts[1][0]) {
                tb_v_aft[l2bidx] = sum_tb[1][0]/(float)cnts[1][0];

                Meas* this_meas = new Meas();
                this_meas->value = tb_v_aft[l2bidx] - dtbv;
                this_meas->measType = Meas::L_BAND_TBV_MEAS_TYPE;
                this_meas->incidenceAngle = dtr * inc_aft[l2bidx];
                this_meas->eastAzimuth = gs_deg_to_pe_rad(azi_aft[l2bidx]);
                this_meas->A = sum_A[1][0]/pow((float)cnts[1][0], 2);
                tb_ml_avg.Append(this_meas);
            }

            if(cnts[0][1]) {
                tb_h_fore[l2bidx] = sum_tb[0][1]/(float)cnts[0][1];

                Meas* this_meas = new Meas();
                this_meas->value = tb_h_fore[l2bidx] - dtbh;
                this_meas->measType = Meas::L_BAND_TBH_MEAS_TYPE;
                this_meas->incidenceAngle = dtr * inc_fore[l2bidx];
                this_meas->eastAzimuth = gs_deg_to_pe_rad(azi_fore[l2bidx]);
                this_meas->A = sum_A[0][1]/pow((float)cnts[0][1], 2);
                tb_ml_avg.Append(this_meas);
            }

            if(cnts[1][1]) {
                tb_h_aft[l2bidx] = sum_tb[1][1]/(float)cnts[1][1];

                Meas* this_meas = new Meas();
                this_meas->value = tb_h_aft[l2bidx] - dtbh;
                this_meas->measType = Meas::L_BAND_TBH_MEAS_TYPE;
                this_meas->incidenceAngle = dtr * inc_aft[l2bidx];
                this_meas->eastAzimuth = gs_deg_to_pe_rad(azi_aft[l2bidx]);
                this_meas->A = sum_A[1][1]/pow((float)cnts[1][1], 2);
                tb_ml_avg.Append(this_meas);
            }

            anc_spd[l2bidx] = sqrt(
                pow(cap_anc_u10.data[anc_ati][anc_cti][0], 2) +
                pow(cap_anc_v10.data[anc_ati][anc_cti][0], 2));

            anc_dir[l2bidx] = rtd * atan2(
                cap_anc_u10.data[anc_ati][anc_cti][0],
                cap_anc_v10.data[anc_ati][anc_cti][0]);

            anc_sst[l2bidx] = cap_anc_sst.data[anc_ati][anc_cti][0] + 273.16;
            anc_sss[l2bidx] = cap_anc_sss.data[anc_ati][anc_cti][0];
            anc_swh[l2bidx] = cap_anc_swh.data[anc_ati][anc_cti][0];

            // Check validity of ancillary data
            if(anc_swh[l2bidx]>10)
                anc_swh[l2bidx] = FILL_VALUE;

            if(tb_ml_avg.NodeCount() > 0) {
                float final_dir, final_spd, final_sss, final_obj;
                cap_gmf.Retrieve(
                    &tb_ml_avg, NULL, anc_spd[l2bidx], anc_dir[l2bidx],
                    anc_sss[l2bidx], anc_spd[l2bidx], anc_dir[l2bidx], 
                    anc_sst[l2bidx], anc_swh[l2bidx], 0, 0, 1,
                    CAPGMF::RETRIEVE_SPEED_SALINITY,
                    &final_spd, &final_dir, &final_sss, &final_obj);

                tb_sss[l2bidx] = final_sss;
                tb_spd[l2bidx] = final_spd;

                // Quality flag (TBD)
                tb_flg[l2bidx] = 0;
            }
        }
    }

    // Write out HDF5 file
    hid_t file_id = H5Fcreate(
        l2b_tb_file, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

    H5LTset_attribute_string(file_id, "/", "REVNO", config_list.Get("REVNO"));

    H5LTset_attribute_string(
        file_id, "/", "REV_START_TIME", config_list.Get("REV_START_TIME"));

    H5LTset_attribute_string(
        file_id, "/", "REV_STOP_TIME", config_list.Get("REV_STOP_TIME"));

    H5LTset_attribute_string(
        file_id, "/", "TB_CRID", config_list.Get("TB_CRID"));

    H5LTset_attribute_string(
        file_id, "/", "L1B_TB_LORES_ASC_FILE",
        config_list.Get("L1B_TB_LORES_ASC_FILE"));

    H5LTset_attribute_string(
        file_id, "/", "L1B_TB_LORES_DEC_FILE",
        config_list.Get("L1B_TB_LORES_DEC_FILE"));

    H5LTset_attribute_float(file_id, "/", "Delta TBH", &dtbh, 1);
    H5LTset_attribute_float(file_id, "/", "Delta TBV", &dtbv, 1);

    H5LTset_attribute_string(
        file_id, "/", "QS_ICEMAP_FILE", config_list.Get("QS_ICEMAP_FILE"));

    H5LTset_attribute_string(
        file_id, "/", "TB_FLAT_MODEL_FILE",
        config_list.Get("TB_FLAT_MODEL_FILE"));

    H5LTset_attribute_string(
        file_id, "/", "TB_ROUGH_MODEL_FILE",
        config_list.Get("TB_ROUGH_MODEL_FILE"));

    H5LTset_attribute_string(
        file_id, "/", "ANC_U10_FILE", config_list.Get("ANC_U10_FILE"));

    H5LTset_attribute_string(
        file_id, "/", "ANC_V10_FILE", config_list.Get("ANC_V10_FILE"));

    H5LTset_attribute_string(
        file_id, "/", "ANC_SSS_FILE", config_list.Get("ANC_SSS_FILE"));

    H5LTset_attribute_string(
        file_id, "/", "ANC_SST_FILE", config_list.Get("ANC_SST_FILE"));

    H5LTset_attribute_string(
        file_id, "/", "ANC_SWH_FILE", config_list.Get("ANC_SWH_FILE"));

    H5LTset_attribute_string(
        file_id, "/", "CROSSTRACK_RESOLUTION",
        config_list.Get("CROSSTRACK_RESOLUTION"));

    H5LTset_attribute_string(
        file_id, "/", "ALONGTRACK_RESOLUTION",
        config_list.Get("ALONGTRACK_RESOLUTION"));

    float _fill_value = FILL_VALUE;
    uint16 _ushort_fill_value = 65535;

    hsize_t dims[2] = {ncti, nati};
    H5LTmake_dataset(file_id, "lat", 2, dims, H5T_NATIVE_FLOAT, &lat[0]);
    H5LTset_attribute_string(file_id, "lat", "long_name", "latitude");
    H5LTset_attribute_string(file_id, "lat", "units", "Degrees");
    H5LTset_attribute_float(file_id, "lat", "_FillValue", &_fill_value, 1);

    H5LTmake_dataset(file_id, "lon", 2, dims, H5T_NATIVE_FLOAT, &lon[0]);
    H5LTset_attribute_string(file_id, "lon", "long_name", "longitude");
    H5LTset_attribute_string(file_id, "lon", "units", "Degrees");
    H5LTset_attribute_float(file_id, "lon", "_FillValue", &_fill_value, 1);

    H5LTmake_dataset(
        file_id, "tb_h_fore", 2, dims, H5T_NATIVE_FLOAT, &tb_h_fore[0]);
    H5LTset_attribute_string(
        file_id, "tb_h_fore", "long_name",
        "Average brightness temperature for all H-pol fore looks");
    H5LTset_attribute_string(file_id, "tb_h_fore", "units", "Degrees kelvin");
    H5LTset_attribute_float(file_id, "tb_h_fore", "_FillValue", &_fill_value, 1);

    H5LTmake_dataset(
        file_id, "tb_h_aft", 2, dims, H5T_NATIVE_FLOAT, &tb_h_aft[0]);
    H5LTset_attribute_string(
        file_id, "tb_h_aft", "long_name",
        "Average brightness temperature for all H-pol aft looks");
    H5LTset_attribute_string(file_id, "tb_h_aft", "units", "Degrees kelvin");
    H5LTset_attribute_float(file_id, "tb_h_aft", "_FillValue", &_fill_value, 1);

    H5LTmake_dataset(
        file_id, "tb_v_fore", 2, dims, H5T_NATIVE_FLOAT, &tb_v_fore[0]);
    H5LTset_attribute_string(
        file_id, "tb_v_fore", "long_name",
        "Average brightness temperature for all V-pol fore looks");
    H5LTset_attribute_string(file_id, "tb_v_fore", "units", "Degrees kelvin");
    H5LTset_attribute_float(file_id, "tb_v_fore", "_FillValue", &_fill_value, 1);

    H5LTmake_dataset(
        file_id, "tb_v_aft", 2, dims, H5T_NATIVE_FLOAT, &tb_v_aft[0]);
    H5LTset_attribute_string(
        file_id, "tb_v_aft", "long_name",
        "Average brightness temperature for all V-pol aft looks");
    H5LTset_attribute_string(file_id, "tb_v_aft", "units", "Degrees kelvin");
    H5LTset_attribute_float(file_id, "tb_v_aft", "_FillValue", &_fill_value, 1);

    H5LTmake_dataset(
        file_id, "inc_fore", 2, dims, H5T_NATIVE_FLOAT, &inc_fore[0]);
    H5LTset_attribute_string(
        file_id, "inc_fore", "long_name", "Cell incidence angle fore look");
    H5LTset_attribute_string(file_id, "inc_fore", "units", "Degrees");
    H5LTset_attribute_float(file_id, "inc_fore", "_FillValue", &_fill_value, 1);

    H5LTmake_dataset(
        file_id, "inc_aft", 2, dims, H5T_NATIVE_FLOAT, &inc_aft[0]);
    H5LTset_attribute_string(
        file_id, "inc_aft", "long_name", "Cell incidence angle aft look");
    H5LTset_attribute_string(file_id, "inc_aft", "units", "Degrees");
    H5LTset_attribute_float(file_id, "inc_aft", "_FillValue", &_fill_value, 1);

    H5LTmake_dataset(
        file_id, "azi_fore", 2, dims, H5T_NATIVE_FLOAT, &azi_fore[0]);
    H5LTset_attribute_string(
        file_id, "azi_fore", "long_name", "Cell azimuth angle fore look");
    H5LTset_attribute_string(file_id, "azi_fore", "units", "Degrees");
    H5LTset_attribute_float(file_id, "azi_fore", "_FillValue", &_fill_value, 1);

    H5LTmake_dataset(file_id, "azi_aft", 2, dims, H5T_NATIVE_FLOAT, &azi_aft[0]);
    H5LTset_attribute_string(
        file_id, "azi_aft", "long_name", "Cell azimuth angle aft look");
    H5LTset_attribute_string(file_id, "azi_aft", "units", "Degrees");
    H5LTset_attribute_float(file_id, "azi_aft", "_FillValue", &_fill_value, 1);

    H5LTmake_dataset(file_id, "anc_spd", 2, dims, H5T_NATIVE_FLOAT, &anc_spd[0]);
    H5LTset_attribute_string(
        file_id, "anc_spd", "long_name",
        "10 meter NCEP wind speed (scaled by 1.03)");
    H5LTset_attribute_string(file_id, "anc_spd", "units", "Meters/second");
    H5LTset_attribute_float(file_id, "anc_spd", "_FillValue", &_fill_value, 1);

    H5LTmake_dataset(file_id, "anc_dir", 2, dims, H5T_NATIVE_FLOAT, &anc_dir[0]);
    H5LTset_attribute_string(
        file_id, "anc_dir", "long_name",
        "NCEP wind direction (oceanographic convention)");
    H5LTset_attribute_string(file_id, "anc_dir", "units", "Degrees");
    H5LTset_attribute_float(file_id, "anc_dir", "_FillValue", &_fill_value, 1);

    H5LTmake_dataset(file_id, "anc_sss", 2, dims, H5T_NATIVE_FLOAT, &anc_sss[0]);
    H5LTset_attribute_string(
        file_id, "anc_sss", "long_name", "HYCOM salinity");
    H5LTset_attribute_string(file_id, "anc_sss", "units", "PSU");
    H5LTset_attribute_float(file_id, "anc_sss", "_FillValue", &_fill_value, 1);

    H5LTmake_dataset(file_id, "anc_sst", 2, dims, H5T_NATIVE_FLOAT, &anc_sst[0]);
    H5LTset_attribute_string(
        file_id, "anc_sst", "long_name",
        "NOAA Optimum Interpolation sea surface temperature");
    H5LTset_attribute_string(file_id, "anc_sst", "units", "Degrees kelvin");
    H5LTset_attribute_float(file_id, "anc_sst", "_FillValue", &_fill_value, 1);

    H5LTmake_dataset(file_id, "anc_swh", 2, dims, H5T_NATIVE_FLOAT, &anc_swh[0]);
    H5LTset_attribute_string(
        file_id, "anc_swh", "long_name",
        "NOAA WaveWatch III Significant wave height");
    H5LTset_attribute_string(file_id, "anc_swh", "units", "Meters");
    H5LTset_attribute_float(file_id, "anc_swh", "_FillValue", &_fill_value, 1);

    H5LTmake_dataset(file_id, "tb_spd", 2, dims, H5T_NATIVE_FLOAT, &tb_spd[0]);
    H5LTset_attribute_string(
        file_id, "tb_spd", "long_name", "SMAP TB Wind Speed");
    H5LTset_attribute_string(file_id, "tb_spd", "units", "Meters/second");
    H5LTset_attribute_float(file_id, "tb_spd", "_FillValue", &_fill_value, 1);

    H5LTmake_dataset(file_id, "tb_sss", 2, dims, H5T_NATIVE_FLOAT, &tb_sss[0]);
    H5LTset_attribute_string(
        file_id, "tb_sss", "long_name", "SMAP TB Salinity");
    H5LTset_attribute_string(file_id, "tb_sss", "units", "PSU");
    H5LTset_attribute_float(file_id, "tb_sss", "_FillValue", &_fill_value, 1);

    H5LTmake_dataset(file_id, "tb_flg", 2, dims, H5T_NATIVE_USHORT, &tb_flg[0]);
    H5LTset_attribute_string(
        file_id, "tb_flg", "long_name", "Quality flag");
    H5LTset_attribute_string(
        file_id, "tb_flg", "Bit definitions", "TBD");
    H5LTset_attribute_ushort(
        file_id, "tb_flg", "_FillValue", &_ushort_fill_value, 1);

    H5Fclose(file_id);
    return(0);
}

