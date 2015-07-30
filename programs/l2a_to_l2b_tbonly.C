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
    float** lat = (float**)make_array(sizeof(float), 2, ncti, nati);
    float** lon = (float**)make_array(sizeof(float), 2, ncti, nati);
    float** tb_h_fore = (float**)make_array(sizeof(float), 2, ncti, nati);
    float** tb_h_aft = (float**)make_array(sizeof(float), 2, ncti, nati);
    float** tb_v_fore = (float**)make_array(sizeof(float), 2, ncti, nati);
    float** tb_v_aft = (float**)make_array(sizeof(float), 2, ncti, nati);
    float** inc_fore = (float**)make_array(sizeof(float), 2, ncti, nati);
    float** inc_aft = (float**)make_array(sizeof(float), 2, ncti, nati);
    float** azi_fore = (float**)make_array(sizeof(float), 2, ncti, nati);
    float** azi_aft = (float**)make_array(sizeof(float), 2, ncti, nati);
    float** anc_spd = (float**)make_array(sizeof(float), 2, ncti, nati);
    float** anc_dir = (float**)make_array(sizeof(float), 2, ncti, nati);
    float** anc_sss = (float**)make_array(sizeof(float), 2, ncti, nati);
    float** anc_sst = (float**)make_array(sizeof(float), 2, ncti, nati);
    float** anc_swh = (float**)make_array(sizeof(float), 2, ncti, nati);
    float** tb_spd = (float**)make_array(sizeof(float), 2, ncti, nati);
    float** tb_sss = (float**)make_array(sizeof(float), 2, ncti, nati);


    for(int ati=0; ati<nati; ++ati) {
        if(ati%50 == 0)
            fprintf(stdout, "%d of %d\n", ati, nati);

        for(int cti=0; cti<ncti; ++cti) {

            // Hack to index into (approx) correct location in ancillary files.
            int anc_cti = (is_25km) ? cti*2 : cti;
            int anc_ati = (is_25km) ? ati*2 : ati;

            // Initialize to fill values
            lon[cti][ati] = FILL_VALUE;
            lat[cti][ati] = FILL_VALUE;
            tb_h_fore[cti][ati] = FILL_VALUE;
            tb_h_aft[cti][ati] = FILL_VALUE;
            tb_v_fore[cti][ati] = FILL_VALUE;
            tb_v_aft[cti][ati] = FILL_VALUE;
            inc_fore[cti][ati] = FILL_VALUE;
            inc_aft[cti][ati] = FILL_VALUE;
            azi_fore[cti][ati] = FILL_VALUE;
            azi_aft[cti][ati] = FILL_VALUE;
            anc_spd[cti][ati] = FILL_VALUE;
            anc_dir[cti][ati] = FILL_VALUE;
            anc_sss[cti][ati] = FILL_VALUE;
            anc_sst[cti][ati] = FILL_VALUE;
            anc_swh[cti][ati] = FILL_VALUE;
            tb_spd[cti][ati] = FILL_VALUE;
            tb_sss[cti][ati] = FILL_VALUE;

            // Check for valid measList at this WVC
            if(!l2a_tb_swath[cti][ati])
                continue;

            MeasList* tb_ml = &(l2a_tb_swath[cti][ati]->measList);

            LonLat lonlat = tb_ml->AverageLonLat();

            lon[cti][ati] = rtd * lonlat.longitude;
            lat[cti][ati] = rtd * lonlat.latitude;

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
                inc_fore[cti][ati] = rtd*sum_inc[0]/(float)cnts_fore;
                azi_fore[cti][ati] = rtd*atan2(sum_cos_azi[0], sum_sin_azi[0]);
            }

            int cnts_aft = cnts[1][0] + cnts[1][1];
            if(cnts_aft>0) {
                inc_aft[cti][ati] = rtd*sum_inc[1]/(float)cnts_aft;
                azi_aft[cti][ati] = rtd*atan2(sum_cos_azi[1], sum_sin_azi[1]);
            }

            MeasList tb_ml_avg;
            if(cnts[0][0]) {
                tb_v_fore[cti][ati] = sum_tb[0][0]/(float)cnts[0][0];

                Meas* this_meas = new Meas();
                this_meas->value = tb_v_fore[cti][ati] - dtbv;
                this_meas->measType = Meas::L_BAND_TBV_MEAS_TYPE;
                this_meas->incidenceAngle = dtr * inc_fore[cti][ati];
                this_meas->eastAzimuth = gs_deg_to_pe_rad(azi_fore[cti][ati]);
                this_meas->A = sum_A[0][0]/pow((float)cnts[0][0], 2);
                tb_ml_avg.Append(this_meas);
            }

            if(cnts[1][0]) {
                tb_v_aft[cti][ati] = sum_tb[1][0]/(float)cnts[1][0];

                Meas* this_meas = new Meas();
                this_meas->value = tb_v_aft[cti][ati] - dtbv;
                this_meas->measType = Meas::L_BAND_TBV_MEAS_TYPE;
                this_meas->incidenceAngle = dtr * inc_aft[cti][ati];
                this_meas->eastAzimuth = gs_deg_to_pe_rad(azi_aft[cti][ati]);
                this_meas->A = sum_A[1][0]/pow((float)cnts[1][0], 2);
                tb_ml_avg.Append(this_meas);
            }

            if(cnts[0][1]) {
                tb_h_fore[cti][ati] = sum_tb[0][1]/(float)cnts[0][1];

                Meas* this_meas = new Meas();
                this_meas->value = tb_h_fore[cti][ati] - dtbh;
                this_meas->measType = Meas::L_BAND_TBH_MEAS_TYPE;
                this_meas->incidenceAngle = dtr * inc_fore[cti][ati];
                this_meas->eastAzimuth = gs_deg_to_pe_rad(azi_fore[cti][ati]);
                this_meas->A = sum_A[0][1]/pow((float)cnts[0][1], 2);
                tb_ml_avg.Append(this_meas);
            }

            if(cnts[1][1]) {
                tb_h_aft[cti][ati] = sum_tb[1][1]/(float)cnts[1][1];

                Meas* this_meas = new Meas();
                this_meas->value = tb_h_aft[cti][ati] - dtbh;
                this_meas->measType = Meas::L_BAND_TBH_MEAS_TYPE;
                this_meas->incidenceAngle = dtr * inc_aft[cti][ati];
                this_meas->eastAzimuth = gs_deg_to_pe_rad(azi_aft[cti][ati]);
                this_meas->A = sum_A[1][1]/pow((float)cnts[1][1], 2);
                tb_ml_avg.Append(this_meas);
            }

            anc_spd[cti][ati] = sqrt(
                pow(cap_anc_u10.data[anc_ati][anc_cti][0], 2) +
                pow(cap_anc_v10.data[anc_ati][anc_cti][0], 2));

            anc_dir[cti][ati] = rtd * atan2(
                cap_anc_u10.data[anc_ati][anc_cti][0],
                cap_anc_v10.data[anc_ati][anc_cti][0]);

            anc_sst[cti][ati] = cap_anc_sst.data[anc_ati][anc_cti][0] + 273.16;
            anc_sss[cti][ati] = cap_anc_sss.data[anc_ati][anc_cti][0];
            anc_swh[cti][ati] = cap_anc_swh.data[anc_ati][anc_cti][0];

            if(tb_ml_avg.NodeCount() > 0) {
                float final_dir, final_spd, final_sss, final_obj;
                cap_gmf.Retrieve(
                    &tb_ml_avg, NULL, anc_spd[cti][ati], anc_dir[cti][ati],
                    anc_sss[cti][ati], anc_spd[cti][ati], anc_dir[cti][ati], 
                    anc_sst[cti][ati], anc_swh[cti][ati], 0, 0, 1,
                    CAPGMF::RETRIEVE_SPEED_SALINITY,
                    &final_spd, &final_dir, &final_sss, &final_obj);

                tb_sss[cti][ati] = final_sss;
                tb_spd[cti][ati] = final_spd;
            }
        }
    }

    // Write it out
    FILE* ofp = fopen(l2b_tb_file, "w");
    write_array(ofp, &lon[0], sizeof(float), 2, ncti, nati);
    write_array(ofp, &lat[0], sizeof(float), 2, ncti, nati);
    write_array(ofp, &tb_h_fore[0], sizeof(float), 2, ncti, nati);
    write_array(ofp, &tb_h_aft[0], sizeof(float), 2, ncti, nati);
    write_array(ofp, &tb_v_fore[0], sizeof(float), 2, ncti, nati);
    write_array(ofp, &tb_v_aft[0], sizeof(float), 2, ncti, nati);
    write_array(ofp, &inc_fore[0], sizeof(float), 2, ncti, nati);
    write_array(ofp, &inc_aft[0], sizeof(float), 2, ncti, nati);
    write_array(ofp, &azi_fore[0], sizeof(float), 2, ncti, nati);
    write_array(ofp, &azi_aft[0], sizeof(float), 2, ncti, nati);
    write_array(ofp, &anc_spd[0], sizeof(float), 2, ncti, nati);
    write_array(ofp, &anc_dir[0], sizeof(float), 2, ncti, nati);
    write_array(ofp, &anc_sss[0], sizeof(float), 2, ncti, nati);
    write_array(ofp, &anc_sst[0], sizeof(float), 2, ncti, nati);
    write_array(ofp, &anc_swh[0], sizeof(float), 2, ncti, nati);
    write_array(ofp, &tb_spd[0], sizeof(float), 2, ncti, nati);
    write_array(ofp, &tb_sss[0], sizeof(float), 2, ncti, nati);
    fclose(ofp);

    // free the arrays
    free_array(lat, 2, ncti, nati);
    free_array(lon, 2, ncti, nati);
    free_array(tb_h_fore, 2, ncti, nati);
    free_array(tb_h_aft, 2, ncti, nati);
    free_array(tb_v_fore, 2, ncti, nati);
    free_array(tb_v_aft, 2, ncti, nati);
    free_array(inc_fore, 2, ncti, nati);
    free_array(inc_aft, 2, ncti, nati);
    free_array(azi_fore, 2, ncti, nati);
    free_array(azi_aft, 2, ncti, nati);
    free_array(anc_spd, 2, ncti, nati);
    free_array(anc_dir, 2, ncti, nati);
    free_array(anc_sss, 2, ncti, nati);
    free_array(anc_sst, 2, ncti, nati);
    free_array(anc_swh, 2, ncti, nati);
    free_array(tb_spd, 2, ncti, nati);
    free_array(tb_sss, 2, ncti, nati);

    return(0);
}

