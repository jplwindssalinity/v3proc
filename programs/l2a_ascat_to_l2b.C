//==============================================================//
// Copyright (C) 2016, California Institute of Technology.      //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#define L2A_FILE_KEYWORD "L2A_FILE"
#define L2B_FILE_KEYWORD "L2B_FILE"
#define FILL_VALUE -9999



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
    char* l2a_file = config_list.Get(L2A_FILE_KEYWORD);
    char* l2b_file = config_list.Get(L2B_FILE_KEYWORD);

    GMF gmf;
    if(!ConfigGMF(&gmf, &config_list)) {
        fprintf(stderr, "%s: error configuring GMF\n", command);
        exit(1);
    }
    Kp kp;
    if(!ConfigKp(&kp, &config_list)) {
        fprintf(stderr, "%s: error configuring Kp\n", command);
        exit(1);
    }

    L2A l2a;
    l2a.SetInputFilename(l2a_file);

    l2a.OpenForReading();
    l2a.ReadHeader();

    int ncti = l2a.header.crossTrackBins;
    int nati = l2a.header.alongTrackBins;

    // Read in L2A file
    L2AFrame*** l2a_swath;
    l2a_swath = (L2AFrame***)make_array(sizeof(L2AFrame *), 2, ncti, nati);
    while(l2a.ReadDataRec()) {
        L2AFrame* this_frame = new L2AFrame();
        this_frame->CopyFrame(this_frame, &l2a.frame);
         *(*(l2a_swath + this_frame->cti) + this_frame->ati) = this_frame;
    }
    l2a.Close();

    char code_b_t_day_start[CODE_B_TIME_LENGTH] = "1970-001T00:00:00.000";
    strncpy(code_b_t_day_start, config_list.Get("REV_START_TIME"), 8);

    ETime etime_start, etime_stop, etime_day_start;

    etime_day_start.FromCodeB(code_b_t_day_start);
    etime_start.FromCodeB(config_list.Get("REV_START_TIME"));
    etime_stop.FromCodeB(config_list.Get("REV_STOP_TIME"));

    // Extract year and day-of-year for attributes
    int start_year = atoi(strtok(&code_b_t_day_start[0], "-"));
    int start_doy = atoi(strtok(NULL, "T"));

    double t_start = etime_start.GetTime();
    double t_stop = etime_stop.GetTime();
    double t_day_start = etime_day_start.GetTime();

    WindSwath wind_swath;
    wind_swath.Allocate(ncti, nati);

    int l2b_size = ncti * nati;
    std::vector<float> lat(l2b_size), lon(l2b_size);
    std::vector<float> ascat_spd_dirth(l2b_size), ascat_dir_dirth(l2b_size);
    std::vector<float> ascat_spd_sel(l2b_size), ascat_dir_sel(l2b_size);
    std::vector<float> ascat_ambiguity_spd(l2b_size*4);
    std::vector<float> ascat_ambiguity_dir(l2b_size*4);
    std::vector<float> sigma0_fore(l2b_size);
    std::vector<float> sigma0_mid(l2b_size);
    std::vector<float> sigma0_aft(l2b_size);
    std::vector<float> inc_fore(l2b_size), azi_fore(l2b_size);
    std::vector<float> inc_mid(l2b_size), azi_mid(l2b_size);
    std::vector<float> inc_aft(l2b_size), azi_aft(l2b_size);
    std::vector<uint16> quality_flag(l2b_size);
    std::vector<float> row_time(nati);
    std::vector<uint8> num_ambiguities(l2b_size);

    for(int ati=0; ati<nati; ++ati) {

        row_time[ati] = 
            t_start + (t_stop-t_start)*(double)ati/(double)nati - t_day_start;

        if(ati%100 == 0)
            fprintf(stdout, "%d of %d\n", ati, nati);

        for(int cti=0; cti<ncti; ++cti) {

            int l2bidx = ati + nati*cti;

            // initialized datasets
            lat[l2bidx] = FILL_VALUE;
            lon[l2bidx] = FILL_VALUE;
            ascat_spd_dirth[l2bidx] = FILL_VALUE;
            ascat_dir_dirth[l2bidx] = FILL_VALUE;
            num_ambiguities[l2bidx] = 0;
            ascat_spd_sel[l2bidx] = FILL_VALUE;
            ascat_dir_sel[l2bidx] = FILL_VALUE;
            sigma0_fore[l2bidx] = FILL_VALUE;
            sigma0_mid[l2bidx] = FILL_VALUE;
            sigma0_aft[l2bidx] = FILL_VALUE;
            inc_fore[l2bidx] = FILL_VALUE;
            inc_mid[l2bidx] = FILL_VALUE;
            inc_aft[l2bidx] = FILL_VALUE;
            azi_fore[l2bidx] = FILL_VALUE;
            azi_mid[l2bidx] = FILL_VALUE;
            azi_aft[l2bidx] = FILL_VALUE;

            for(int iamb = 0; iamb < 4; ++iamb) {
                ascat_ambiguity_spd[l2bidx] = FILL_VALUE;
                ascat_ambiguity_dir[l2bidx] = FILL_VALUE;
            }

            // Check for valid measList at this WVC
            if(!l2a_swath[cti][ati])
                continue;

            MeasList* ml = &(l2a_swath[cti][ati]->measList);

            LonLat lonlat = ml->AverageLonLat();
            lon[l2bidx] = rtd * lonlat.longitude;
            lat[l2bidx] = rtd * lonlat.latitude;

            // wrap to [-180, 180) interval
            while(lon[l2bidx]>=180) lon[l2bidx] -= 360;
            while(lon[l2bidx]<-180) lon[l2bidx] += 360;

            float sum_s0[6], sum_s02[6];
            float sum_cos_azi[6], sum_sin_azi[6];
            float sum_inc[6];
            int cnts[6];

            for(int ibeam=0; ibeam<6; ++ibeam) {
                sum_s0[ibeam] = 0;
                sum_s02[ibeam] = 0;
                sum_cos_azi[ibeam] = 0;
                sum_sin_azi[ibeam] = 0;
                sum_inc[ibeam] = 0;
                cnts[ibeam] = 0;
            }

            int any_land = 0;
            int any_ice = 0;

            // Average over each beam per WVC
            for(Meas* meas = ml->GetHead(); meas; meas = ml->GetNext()) {
                // Skip land flagged observations
                if(meas->landFlag) {
                    if(meas->landFlag == 1 || meas->landFlag == 3)
                        any_land = 1;

                    if(meas->landFlag == 2 || meas->landFlag == 3)
                        any_ice = 1;

                    continue;
                }

                cnts[meas->beamIdx]++;
                sum_s0[meas->beamIdx] += meas->value;
                sum_s02[meas->beamIdx] += pow(meas->value, 2);
                sum_inc[meas->beamIdx] += meas->incidenceAngle;
                sum_cos_azi[meas->beamIdx] += cos(meas->eastAzimuth);
                sum_sin_azi[meas->beamIdx] += sin(meas->eastAzimuth);
            }

            // Create averaged measurement list
            MeasList ml_avg;
            for(int ibeam=0; ibeam<6; ++ibeam) {

                // Check inc angles
                float this_inc = sum_inc[ibeam]/(float)cnts[ibeam] * rtd;
                if(ibeam==1||ibeam==4) {
                    if(this_inc < 25 || this_inc > 55)
                        continue;
                } else {
                    if(this_inc < 33.7 || this_inc > 64)
                        continue;
                }

                if(cnts[ibeam]>0) {
                    Meas* this_meas = new Meas();
                    this_meas->value = sum_s0[ibeam]/(float)cnts[ibeam];
                    this_meas->measType = Meas::VV_MEAS_TYPE;
                    this_meas->incidenceAngle =
                        sum_inc[ibeam]/(float)cnts[ibeam];

                    this_meas->eastAzimuth = atan2(
                        sum_sin_azi[ibeam], sum_cos_azi[ibeam]);

                    this_meas->A = 
                        (sum_s02[ibeam]-pow(sum_s0[ibeam], 2)/(float)cnts[ibeam])/
                        (float)(cnts[ibeam]-1);

                    ml_avg.Append(this_meas);

                    if(ibeam == 0 || ibeam == 3) {
                        sigma0_fore[l2bidx] = this_meas->value;
                        inc_fore[l2bidx] = this_meas->incidenceAngle * rtd;
                        azi_fore[l2bidx] = pe_rad_to_gs_deg(
                            this_meas->eastAzimuth);

                    } else if(ibeam == 1 || ibeam == 4) {
                        sigma0_mid[l2bidx] = this_meas->value;
                        inc_mid[l2bidx] = this_meas->incidenceAngle * rtd;
                        azi_mid[l2bidx] = pe_rad_to_gs_deg(
                            this_meas->eastAzimuth);

                    } else if(ibeam == 2 || ibeam == 5) {
                        sigma0_aft[l2bidx] = this_meas->value;
                        inc_aft[l2bidx] = this_meas->incidenceAngle * rtd;
                        azi_aft[l2bidx] = pe_rad_to_gs_deg(
                            this_meas->eastAzimuth);
                    }
                }
            }

            WVC* wvc = new WVC();
            gmf.RetrieveWinds_S3(&ml_avg, &kp, wvc);
            if(wvc->ambiguities.NodeCount() > 0)
                wind_swath.Add(cti, ati, wvc);
            else
                delete wvc;
        }
    }
    free_array((void*)l2a_swath, 2, ncti, nati);

    // Nudge
    WindField nudge_field;
    nudge_field.ReadType(
        config_list.Get("NUDGE_WINDFIELD_FILE"),
        config_list.Get("NUDGE_WINDFIELD_TYPE"));

    int max_rank_for_nudging;
    config_list.GetInt("MAX_RANK_FOR_NUDGING", &max_rank_for_nudging);

    wind_swath.GetNudgeVectors(&nudge_field);
    wind_swath.Nudge(max_rank_for_nudging);

    // Median filter ambig removal
    int median_filter_max_passes, median_filter_window_size;
    config_list.GetInt("MEDIAN_FILTER_MAX_PASSES", &median_filter_max_passes);
    config_list.GetInt("MEDIAN_FILTER_WINDOW_SIZE", &median_filter_window_size);

    // 4 pass-pass
    wind_swath.MedianFilter_4Pass(
        median_filter_window_size, median_filter_max_passes, 0);

    // copy out ambiguities before DIRTH
    for(int ati=0; ati<nati; ++ati) {
        for(int cti=0; cti<ncti; ++cti) {
            int l2bidx = ati + nati*cti;

            WVC* wvc = wind_swath.GetWVC(cti, ati);
            if(!wvc)
                continue;

            ascat_spd_sel[l2bidx] = wvc->selected->spd;
            ascat_dir_sel[l2bidx] = pe_rad_to_gs_deg(wvc->selected->dir);
            num_ambiguities[l2bidx] = wvc->ambiguities.NodeCount();

            int j_amb = 0;
            for(WindVectorPlus* wvp = wvc->ambiguities.GetHead(); wvp;
                wvp = wvc->ambiguities.GetNext()){

                int ambidx = j_amb + 4*l2bidx;
                ascat_ambiguity_spd[ambidx] = wvp->spd;
                ascat_ambiguity_dir[ambidx] = pe_rad_to_gs_deg(wvp->dir);
                ++j_amb;
            }
        }
    }

    // DIRTH Pass
    wind_swath.MedianFilter(
        median_filter_window_size, median_filter_max_passes, 0, 0, 1);

    // copy DIRTH outputs
    for(int ati=0; ati<nati; ++ati) {
        for(int cti=0; cti<ncti; ++cti) {
            int l2bidx = ati + nati*cti;

            WVC* wvc = wind_swath.GetWVC(cti, ati);
            if(!wvc)
                continue;

            ascat_spd_dirth[l2bidx] = wvc->selected->spd;
            ascat_dir_dirth[l2bidx] = pe_rad_to_gs_deg(wvc->selected->spd);
        }
    }
    

    // Write out HDF5 file
    hid_t file_id = H5Fcreate(
        l2b_file, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    H5LTset_attribute_string(file_id, "/", "REVNO", config_list.Get("REVNO"));
    H5LTset_attribute_int(file_id, "/", "REV_START_YEAR", &start_year, 1);
    H5LTset_attribute_int(file_id, "/", "REV_START_DAY_OF_YEAR", &start_doy, 1);
    H5LTset_attribute_int(file_id, "/", "Number of Cross Track Bins", &ncti, 1);
    H5LTset_attribute_int(file_id, "/", "Number of Cross Track Bins", &ncti, 1);
    H5LTset_attribute_int(file_id, "/", "Number of Along Track Bins", &nati, 1);

    H5LTset_attribute_string(
        file_id, "/", "REV_START_TIME", config_list.Get("REV_START_TIME"));

    H5LTset_attribute_string(
        file_id, "/", "REV_STOP_TIME", config_list.Get("REV_STOP_TIME"));

    H5LTset_attribute_string(
        file_id, "/", "QS_ICEMAP_FILE", config_list.Get("QS_ICEMAP_FILE"));

    H5LTset_attribute_string(
        file_id, "/", "CROSSTRACK_RESOLUTION",
        config_list.Get("CROSSTRACK_RESOLUTION"));

    H5LTset_attribute_string(
        file_id, "/", "ALONGTRACK_RESOLUTION",
        config_list.Get("ALONGTRACK_RESOLUTION"));

    float _fill_value = FILL_VALUE;
    uint16 _ushort_fill_value = 65535;
    unsigned char uchar_fill_value = 0;

    float valid_max, valid_min;

    hsize_t dims[2] = {ncti, nati};
    hsize_t dims_amb[3] = {ncti, nati, 4};

    hsize_t ati_dim = nati;

    valid_max = 90; valid_min = -90;
    H5LTmake_dataset(file_id, "lat", 2, dims, H5T_NATIVE_FLOAT, &lat[0]);
    H5LTset_attribute_string(file_id, "lat", "long_name", "latitude");
    H5LTset_attribute_string(file_id, "lat", "units", "Degrees");
    H5LTset_attribute_float(file_id, "lat", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(file_id, "lat", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "lat", "valid_min", &valid_min, 1);

    valid_max = 180; valid_min = -180;
    H5LTmake_dataset(file_id, "lon", 2, dims, H5T_NATIVE_FLOAT, &lon[0]);
    H5LTset_attribute_string(file_id, "lon", "long_name", "longitude");
    H5LTset_attribute_string(file_id, "lon", "units", "Degrees");
    H5LTset_attribute_float(file_id, "lon", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(file_id, "lon", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "lon", "valid_min", &valid_min, 1);

    valid_max = 100; valid_min = 0;
    H5LTmake_dataset(
        file_id, "ascat_spd_dirth", 2, dims, H5T_NATIVE_FLOAT,
        &ascat_spd_dirth[0]);
    H5LTset_attribute_string(
        file_id, "ascat_spd_dirth", "long_name", "ASCAT wind speed DIRTH");
    H5LTset_attribute_string(
        file_id, "ascat_spd_dirth", "units", "m/s");
    H5LTset_attribute_float(
        file_id, "ascat_spd_dirth", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(
        file_id, "ascat_spd_dirth", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ascat_spd_dirth", "valid_min", &valid_min, 1);


    H5LTmake_dataset(
        file_id, "ascat_spd_sel", 2, dims, H5T_NATIVE_FLOAT,
        &ascat_spd_sel[0]);
    H5LTset_attribute_string(
        file_id, "ascat_spd_sel", "long_name", "ASCAT wind speed selected");
    H5LTset_attribute_string(
        file_id, "ascat_spd_sel", "units", "m/s");
    H5LTset_attribute_float(
        file_id, "ascat_spd_sel", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(
        file_id, "ascat_spd_sel", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ascat_spd_sel", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "ascat_ambiguity_spd", 3, dims_amb, H5T_NATIVE_FLOAT,
        &ascat_ambiguity_spd[0]);
    H5LTset_attribute_string(
        file_id, "ascat_ambiguity_spd", "long_name",
        "ASCAT wind speed ambiguities");
    H5LTset_attribute_string(
        file_id, "ascat_ambiguity_spd", "units", "m/s");
    H5LTset_attribute_float(
        file_id, "ascat_ambiguity_spd", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(
        file_id, "ascat_ambiguity_spd", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ascat_ambiguity_spd", "valid_min", &valid_min, 1);


    valid_max = 360; valid_min = 0;
    H5LTmake_dataset(
        file_id, "ascat_dir_dirth", 2, dims, H5T_NATIVE_FLOAT,
        &ascat_dir_dirth[0]);
    H5LTset_attribute_string(
        file_id, "ascat_dir_dirth", "long_name", "ASCAT wind direction DIRTH");
    H5LTset_attribute_string(
        file_id, "ascat_dir_dirth", "units", "degrees");
    H5LTset_attribute_float(
        file_id, "ascat_dir_dirth", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(
        file_id, "ascat_dir_dirth", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ascat_dir_dirth", "valid_min", &valid_min, 1);


    H5LTmake_dataset(
        file_id, "ascat_dir_sel", 2, dims, H5T_NATIVE_FLOAT,
        &ascat_dir_sel[0]);
    H5LTset_attribute_string(
        file_id, "ascat_dir_sel", "long_name", "ASCAT wind direction selected");
    H5LTset_attribute_string(
        file_id, "ascat_dir_sel", "units", "degrees");
    H5LTset_attribute_float(
        file_id, "ascat_dir_sel", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(
        file_id, "ascat_dir_sel", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ascat_dir_sel", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "ascat_ambiguity_dir", 3, dims_amb, H5T_NATIVE_FLOAT,
        &ascat_ambiguity_dir[0]);
    H5LTset_attribute_string(
        file_id, "ascat_ambiguity_dir", "long_name",
        "ASCAT ambiguity wind direction");
    H5LTset_attribute_string(
        file_id, "ascat_ambiguity_dir", "units", "degrees");
    H5LTset_attribute_float(
        file_id, "ascat_ambiguity_dir", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(
        file_id, "ascat_ambiguity_dir", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ascat_ambiguity_dir", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "quality_flag", 2, dims, H5T_NATIVE_USHORT, &quality_flag[0]);

    valid_max = 86400; valid_min = 0;
    H5LTmake_dataset(
        file_id, "row_time", 1, &ati_dim, H5T_NATIVE_FLOAT, &row_time[0]);
    H5LTset_attribute_string(
        file_id, "row_time", "long_name",
        "Approximate observation time for each row");
    H5LTset_attribute_string(file_id, "row_time", "units", "UTC seconds of day");
    H5LTset_attribute_float(file_id, "row_time", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "row_time", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "num_ambiguities", 2, dims, H5T_NATIVE_UCHAR,
        &num_ambiguities[0]);
    H5LTset_attribute_string(
        file_id, "num_ambiguities", "long_name",
        "Number of wind vector ambiguties");
    H5LTset_attribute_uchar(
        file_id, "num_ambiguities", "_FillValue", &uchar_fill_value, 1);

    valid_max = 30; valid_min = -0.01;
    H5LTmake_dataset(
        file_id, "sigma0_fore", 2, dims, H5T_NATIVE_FLOAT,
        &sigma0_fore[0]);
    H5LTset_attribute_string(
        file_id, "sigma0_fore", "long_name", "Sigma0 for fore look");
    H5LTset_attribute_string(file_id, "sigma0_fore", "units", "n/a");
    H5LTset_attribute_float(file_id, "sigma0_fore", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "sigma0_fore", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "sigma0_mid", 2, dims, H5T_NATIVE_FLOAT,
        &sigma0_mid[0]);
    H5LTset_attribute_string(
        file_id, "sigma0_mid", "long_name", "Sigma0 for mid look");
    H5LTset_attribute_string(file_id, "sigma0_mid", "units", "n/a");
    H5LTset_attribute_float(file_id, "sigma0_mid", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "sigma0_mid", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "sigma0_aft", 2, dims, H5T_NATIVE_FLOAT,
        &sigma0_aft[0]);
    H5LTset_attribute_string(
        file_id, "sigma0_aft", "long_name", "Sigma0 for aft look");
    H5LTset_attribute_string(file_id, "sigma0_aft", "units", "n/a");
    H5LTset_attribute_float(file_id, "sigma0_aft", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "sigma0_aft", "valid_min", &valid_min, 1);


    valid_max = 90; valid_min = 0;
    H5LTmake_dataset(
        file_id, "inc_fore", 2, dims, H5T_NATIVE_FLOAT,
        &inc_fore[0]);
    H5LTset_attribute_string(
        file_id, "inc_fore", "long_name", "Incidence angle for fore look");
    H5LTset_attribute_string(file_id, "inc_fore", "units", "n/a");
    H5LTset_attribute_float(file_id, "inc_fore", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "inc_fore", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "inc_mid", 2, dims, H5T_NATIVE_FLOAT,
        &inc_mid[0]);
    H5LTset_attribute_string(
        file_id, "inc_mid", "long_name", "Incidence angle for mid look");
    H5LTset_attribute_string(file_id, "inc_mid", "units", "degrees");
    H5LTset_attribute_float(file_id, "inc_mid", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "inc_mid", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "inc_aft", 2, dims, H5T_NATIVE_FLOAT,
        &inc_aft[0]);
    H5LTset_attribute_string(
        file_id, "inc_aft", "long_name", "Incidence angle for aft look");
    H5LTset_attribute_string(file_id, "inc_aft", "units", "degrees");
    H5LTset_attribute_float(file_id, "inc_aft", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "inc_aft", "valid_min", &valid_min, 1);

    valid_max = 360; valid_min = 0;
    H5LTmake_dataset(
        file_id, "azi_fore", 2, dims, H5T_NATIVE_FLOAT,
        &azi_fore[0]);
    H5LTset_attribute_string(
        file_id, "azi_fore", "long_name", "Azimuth angle for fore look");
    H5LTset_attribute_string(file_id, "azi_fore", "units", "degrees");
    H5LTset_attribute_float(file_id, "azi_fore", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "azi_fore", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "azi_mid", 2, dims, H5T_NATIVE_FLOAT,
        &azi_mid[0]);
    H5LTset_attribute_string(
        file_id, "azi_mid", "long_name", "Azimuth angle for mid look");
    H5LTset_attribute_string(file_id, "azi_mid", "units", "degrees");
    H5LTset_attribute_float(file_id, "azi_mid", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "azi_mid", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "azi_aft", 2, dims, H5T_NATIVE_FLOAT,
        &azi_aft[0]);
    H5LTset_attribute_string(
        file_id, "azi_aft", "long_name", "Azimuth angle for aft look");
    H5LTset_attribute_string(file_id, "azi_aft", "units", "degrees");
    H5LTset_attribute_float(file_id, "azi_aft", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "azi_aft", "valid_min", &valid_min, 1);


    H5Fclose(file_id);
    return(0);
}
