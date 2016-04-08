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
#define L2B_TB_BIAS_ADJ_FILE_KEYWORD "L2B_TB_BIAS_ADJ_FILE"
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

#define QUAL_FLAG_USEABLE 0x1
#define QUAL_FLAG_FOUR_LOOKS 0x2
#define QUAL_FLAG_POINTING 0x4
#define QUAL_FLAG_SST_TOO_COLD 0x40
#define QUAL_FLAG_LAND 0x80
#define QUAL_FLAG_ICE 0x100

//--------------//
// MAIN PROGRAM //
//--------------//

int main(int argc, char* argv[]) {

    const char* command = no_path(argv[0]);
    char* config_file = argv[1];

    float dtbv_fore_asc = 0;
    float dtbh_fore_asc = 0;
    float dtbv_aft_asc = 0;
    float dtbh_aft_asc = 0;
    float dtbv_fore_dec = 0;
    float dtbh_fore_dec = 0;
    float dtbv_aft_dec = 0;
    float dtbh_aft_dec = 0;
    if(argc == 4) {
        dtbv_fore_asc = atof(argv[2]);
        dtbh_fore_asc = atof(argv[3]);
        dtbv_aft_asc = atof(argv[2]);
        dtbh_aft_asc = atof(argv[3]);
        dtbv_fore_dec = atof(argv[2]);
        dtbh_fore_dec = atof(argv[3]);
        dtbv_aft_dec = atof(argv[2]);
        dtbh_aft_dec = atof(argv[3]);
    } else if(argc == 10) {
        dtbv_fore_asc = atof(argv[2]);
        dtbh_fore_asc = atof(argv[3]);
        dtbv_aft_asc = atof(argv[4]);
        dtbh_aft_asc = atof(argv[5]);
        dtbv_fore_dec = atof(argv[6]);
        dtbh_fore_dec = atof(argv[7]);
        dtbv_aft_dec = atof(argv[8]);
        dtbh_aft_dec = atof(argv[9]);
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

    config_list.DoNothingForMissingKeywords();
    char* l2b_tb_bias_adj_file = config_list.Get(L2B_TB_BIAS_ADJ_FILE_KEYWORD);
    config_list.ExitForMissingKeywords();

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

    CAP_ANC_L2B cap_anc_tb_bias(l2b_tb_bias_adj_file);
    if(l2b_tb_bias_adj_file)
        cap_anc_tb_bias.Read(l2b_tb_bias_adj_file);

    // Output arrays
    int l2b_size = ncti * nati;
    std::vector<float> lat(l2b_size), lon(l2b_size);
    std::vector<float> tb_h_fore(l2b_size), tb_h_aft(l2b_size);
    std::vector<float> tb_v_fore(l2b_size), tb_v_aft(l2b_size);
    std::vector<float> tb_v_bias_adj(l2b_size), tb_h_bias_adj(l2b_size);
    std::vector<float> nedt_h_fore(l2b_size), nedt_h_aft(l2b_size);
    std::vector<float> nedt_v_fore(l2b_size), nedt_v_aft(l2b_size);
    std::vector<uint8> n_h_fore(l2b_size), n_h_aft(l2b_size);
    std::vector<uint8> n_v_fore(l2b_size), n_v_aft(l2b_size);
    std::vector<float> inc_fore(l2b_size), inc_aft(l2b_size);
    std::vector<float> azi_fore(l2b_size), azi_aft(l2b_size);
    std::vector<float> anc_spd(l2b_size), anc_dir(l2b_size);
    std::vector<float> anc_sss(l2b_size), anc_sst(l2b_size), anc_swh(l2b_size);
    std::vector<float> smap_sss(l2b_size), smap_spd(l2b_size);
    std::vector<float> smap_spdonly(l2b_size), smap_high_spd(l2b_size);
    std::vector<float> smap_high_dir(l2b_size), smap_high_dir_smooth(l2b_size);
    std::vector<float> smap_sss_bias_adj(l2b_size), smap_spd_bias_adj(l2b_size);
    std::vector<uint16> quality_flag(l2b_size);
    std::vector<float> row_time(nati);
    std::vector<uint8> num_ambiguities(l2b_size);

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

    CAPWindSwath cap_wind_swath;
    cap_wind_swath.Allocate(ncti, nati);

    for(int ati=0; ati<nati; ++ati) {

        row_time[ati] = 
            t_start + (t_stop-t_start)*(double)ati/(double)nati - t_day_start;

        if(ati%100 == 0)
            fprintf(stdout, "%d of %d; %f\n", ati, nati, row_time[ati]);

        // Set the bias adjustments per ascending / decending portion of orbit.
        float dtbh_fore = (ati < nati/2) ? dtbh_fore_asc : dtbh_fore_dec;
        float dtbv_fore = (ati < nati/2) ? dtbv_fore_asc : dtbv_fore_dec;
        float dtbh_aft = (ati < nati/2) ? dtbh_aft_asc : dtbh_aft_dec;
        float dtbv_aft = (ati < nati/2) ? dtbv_aft_asc : dtbv_aft_dec;

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
            tb_v_bias_adj[l2bidx] = FILL_VALUE;
            tb_h_bias_adj[l2bidx] = FILL_VALUE;
            nedt_h_fore[l2bidx] = FILL_VALUE;
            nedt_h_aft[l2bidx] = FILL_VALUE;
            nedt_v_fore[l2bidx] = FILL_VALUE;
            nedt_v_aft[l2bidx] = FILL_VALUE;
            inc_fore[l2bidx] = FILL_VALUE;
            inc_aft[l2bidx] = FILL_VALUE;
            azi_fore[l2bidx] = FILL_VALUE;
            azi_aft[l2bidx] = FILL_VALUE;
            anc_spd[l2bidx] = FILL_VALUE;
            anc_dir[l2bidx] = FILL_VALUE;
            anc_sss[l2bidx] = FILL_VALUE;
            anc_sst[l2bidx] = FILL_VALUE;
            anc_swh[l2bidx] = FILL_VALUE;
            smap_sss[l2bidx] = FILL_VALUE;
            smap_spd[l2bidx] = FILL_VALUE;
            smap_sss_bias_adj[l2bidx] = FILL_VALUE;
            smap_spd_bias_adj[l2bidx] = FILL_VALUE;
            n_h_fore[l2bidx] = 0;
            n_h_aft[l2bidx] = 0;
            n_v_fore[l2bidx] = 0;
            n_v_aft[l2bidx] = 0;
            quality_flag[l2bidx] = 65535;
            smap_spdonly[l2bidx] = FILL_VALUE;
            smap_high_spd[l2bidx] = FILL_VALUE;
            smap_high_dir[l2bidx] = FILL_VALUE;

            // Check for valid measList at this WVC
            if(!l2a_tb_swath[cti][ati])
                continue;

            // Check for useable ancillary data
            if(cap_anc_sst.data[0][anc_ati][anc_cti] < 0)
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

            int any_land = 0;
            int any_ice = 0;

            for(Meas* meas = tb_ml->GetHead(); meas; meas = tb_ml->GetNext()) {

                // Skip land flagged observations
                if(meas->landFlag) {
                    if(meas->landFlag == 1 || meas->landFlag == 3)
                        any_land = 1;

                    if(meas->landFlag == 2 || meas->landFlag == 3)
                        any_ice = 1;

                    continue;
                }

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
                tb_v_fore[l2bidx] = sum_tb[0][0]/(float)cnts[0][0] - dtbv_fore;
                nedt_v_fore[l2bidx] = sqrt(
                    sum_A[0][0]/pow((float)cnts[0][0], 2));
                n_v_fore[l2bidx] = cnts[0][0];

                Meas* this_meas = new Meas();
                this_meas->value = tb_v_fore[l2bidx];
                this_meas->measType = Meas::L_BAND_TBV_MEAS_TYPE;
                this_meas->incidenceAngle = dtr * inc_fore[l2bidx];
                this_meas->eastAzimuth = gs_deg_to_pe_rad(azi_fore[l2bidx]);
                this_meas->A = sum_A[0][0]/pow((float)cnts[0][0], 2);
                tb_ml_avg.Append(this_meas);
            }

            if(cnts[1][0]) {
                tb_v_aft[l2bidx] = sum_tb[1][0]/(float)cnts[1][0] - dtbv_aft;
                nedt_v_aft[l2bidx] = sqrt(
                    sum_A[1][0]/pow((float)cnts[1][0], 2));
                n_v_aft[l2bidx] = cnts[1][0];

                Meas* this_meas = new Meas();
                this_meas->value = tb_v_aft[l2bidx];
                this_meas->measType = Meas::L_BAND_TBV_MEAS_TYPE;
                this_meas->incidenceAngle = dtr * inc_aft[l2bidx];
                this_meas->eastAzimuth = gs_deg_to_pe_rad(azi_aft[l2bidx]);
                this_meas->A = sum_A[1][0]/pow((float)cnts[1][0], 2);
                tb_ml_avg.Append(this_meas);
            }

            if(cnts[0][1]) {
                tb_h_fore[l2bidx] = sum_tb[0][1]/(float)cnts[0][1] - dtbh_fore;
                nedt_h_fore[l2bidx] = sqrt(
                    sum_A[0][1]/pow((float)cnts[0][1], 2));
                n_h_fore[l2bidx] = cnts[1][0];

                Meas* this_meas = new Meas();
                this_meas->value = tb_h_fore[l2bidx];
                this_meas->measType = Meas::L_BAND_TBH_MEAS_TYPE;
                this_meas->incidenceAngle = dtr * inc_fore[l2bidx];
                this_meas->eastAzimuth = gs_deg_to_pe_rad(azi_fore[l2bidx]);
                this_meas->A = sum_A[0][1]/pow((float)cnts[0][1], 2);
                tb_ml_avg.Append(this_meas);
            }

            if(cnts[1][1]) {
                tb_h_aft[l2bidx] = sum_tb[1][1]/(float)cnts[1][1] - dtbh_aft;
                nedt_h_aft[l2bidx] = sqrt(
                    sum_A[1][1]/pow((float)cnts[1][1], 2));
                n_h_aft[l2bidx] = cnts[1][1];

                Meas* this_meas = new Meas();
                this_meas->value = tb_h_aft[l2bidx];
                this_meas->measType = Meas::L_BAND_TBH_MEAS_TYPE;
                this_meas->incidenceAngle = dtr * inc_aft[l2bidx];
                this_meas->eastAzimuth = gs_deg_to_pe_rad(azi_aft[l2bidx]);
                this_meas->A = sum_A[1][1]/pow((float)cnts[1][1], 2);
                tb_ml_avg.Append(this_meas);
            }

            anc_spd[l2bidx] = 1.03 * sqrt(
                pow(cap_anc_u10.data[0][anc_ati][anc_cti], 2) +
                pow(cap_anc_v10.data[0][anc_ati][anc_cti], 2));

            anc_dir[l2bidx] = rtd * atan2(
                cap_anc_u10.data[0][anc_ati][anc_cti],
                cap_anc_v10.data[0][anc_ati][anc_cti]);

            anc_sst[l2bidx] = cap_anc_sst.data[0][anc_ati][anc_cti] + 273.16;
            anc_sss[l2bidx] = cap_anc_sss.data[0][anc_ati][anc_cti];
            anc_swh[l2bidx] = cap_anc_swh.data[0][anc_ati][anc_cti];

            float this_anc_dir = gs_deg_to_pe_rad(anc_dir[l2bidx]);

            // Check validity of ancillary data
            if(anc_swh[l2bidx]>10)
                anc_swh[l2bidx] = FILL_VALUE;

            if(tb_ml_avg.NodeCount() > 0) {
                float final_dir, final_spd, final_sss, final_obj;

                // Combined SSS and SPD
                float anc_spd_std_prior = 1.5;
                cap_gmf.Retrieve(
                    &tb_ml_avg, NULL, anc_spd[l2bidx], this_anc_dir,
                    anc_sss[l2bidx], anc_spd[l2bidx], this_anc_dir,
                    anc_sst[l2bidx], anc_swh[l2bidx], 0, anc_spd_std_prior, 0,
                    1, CAPGMF::RETRIEVE_SPEED_SALINITY,
                    &final_spd, &final_dir, &final_sss, &final_obj);

                smap_sss[l2bidx] = final_sss;
                smap_spd[l2bidx] = final_spd;

                anc_spd_std_prior = 100;
                cap_gmf.Retrieve(
                    &tb_ml_avg, NULL, anc_spd[l2bidx], this_anc_dir,
                    smap_sss[l2bidx], anc_spd[l2bidx], this_anc_dir,
                    anc_sst[l2bidx], anc_swh[l2bidx], 0, anc_spd_std_prior, 0,
                    1, CAPGMF::RETRIEVE_SPEED_ONLY,
                    &final_spd, &final_dir, &final_sss, &final_obj);

                smap_spdonly[l2bidx] = final_spd;

                // Do vector wind processing
                CAPWVC* wvc = new CAPWVC();

                cap_gmf.CAPGMF::BuildSolutionCurvesSpdOnly(
                    &tb_ml_avg, NULL, anc_spd[l2bidx], anc_sss[l2bidx],
                    anc_spd[l2bidx], this_anc_dir, anc_sst[l2bidx],
                    anc_swh[l2bidx], 0, 100, 0, 1, wvc);

                wvc->BuildSolutions();

                num_ambiguities[l2bidx] = wvc->ambiguities.NodeCount();

                if(wvc->ambiguities.NodeCount() > 0) {
                    cap_wind_swath.Add(cti, ati, wvc);
                    wvc->selected = wvc->GetNearestAmbig(this_anc_dir);
                } else {
                    delete wvc;
                }

                // Retreive salinity and speed again using bias adjusted TB
                if(l2b_tb_bias_adj_file) {
                    float this_tb_v_bias_adj = 
                        -cap_anc_tb_bias.data[0][anc_ati][anc_cti];

                    float this_tb_h_bias_adj = 
                        -cap_anc_tb_bias.data[1][anc_ati][anc_cti];

                    if(fabs(this_tb_v_bias_adj) < 3 && 
                       fabs(this_tb_h_bias_adj) < 3) {

                        tb_v_bias_adj[l2bidx] = this_tb_v_bias_adj;
                        tb_h_bias_adj[l2bidx] = this_tb_h_bias_adj;

                        // iterate over tb_ml_avg, adjust tbs
                        for(Meas* meas = tb_ml_avg.GetHead(); meas; ) {
                            if(meas->measType == Meas::L_BAND_TBH_MEAS_TYPE) {
                                meas->value += tb_h_bias_adj[l2bidx];
                            } else {
                                meas->value += tb_v_bias_adj[l2bidx];
                            }
                            meas = tb_ml_avg.GetNext();
                        }

                        // Combined SSS and SPD
                        float anc_spd_std_prior = 1.5;
                        cap_gmf.Retrieve(
                            &tb_ml_avg, NULL, anc_spd[l2bidx], this_anc_dir,
                            anc_sss[l2bidx], anc_spd[l2bidx], this_anc_dir,
                            anc_sst[l2bidx], anc_swh[l2bidx], 0, 
                            anc_spd_std_prior, 0, 1, 
                            CAPGMF::RETRIEVE_SPEED_SALINITY, &final_spd,
                            &final_dir, &final_sss, &final_obj);

                        smap_sss_bias_adj[l2bidx] = final_sss;
                        smap_spd_bias_adj[l2bidx] = final_spd;
                    }
                }

                // Quality flagging
                quality_flag[l2bidx] = 0;

                // Check for all four looks
                if(tb_ml_avg.NodeCount() != 4)
                    quality_flag[l2bidx] |= QUAL_FLAG_FOUR_LOOKS;

                // Check if any TBs tossed from this cell due to land
                if(any_land)
                    quality_flag[l2bidx] |= QUAL_FLAG_LAND;

                // Check if any TBs tossed from this cell due to ice
                if(any_ice)
                    quality_flag[l2bidx] |= QUAL_FLAG_ICE;

                // Check for low SST (poor salinity sensitivity at low SST)
                if(anc_sst[l2bidx] < 278.16)
                    quality_flag[l2bidx] |= QUAL_FLAG_SST_TOO_COLD;

                if(fabs(inc_fore[l2bidx]-40) > 0.2 || 
                   fabs(inc_aft[l2bidx]-40) > 0.2)
                   quality_flag[l2bidx] |= QUAL_FLAG_POINTING;

                // Overall data quality mask
                uint16 QUAL_MASK = (
                    QUAL_FLAG_FOUR_LOOKS | QUAL_FLAG_LAND | QUAL_FLAG_ICE |
                    QUAL_FLAG_POINTING);

                if(quality_flag[l2bidx] & QUAL_MASK)
                    quality_flag[l2bidx] |= QUAL_FLAG_USEABLE;

            }
        }
    }

    cap_wind_swath.MedianFilter(3, 200, 0, 1);
    for(int ati=0; ati<nati; ++ati) {
        for(int cti=0; cti<ncti; ++cti) {

            int l2bidx = ati + nati*cti;
            CAPWVC* wvc = cap_wind_swath.swath[cti][ati];

            if(!wvc)
                continue;

            smap_high_spd[l2bidx] = wvc->selected->spd;
            smap_high_dir[l2bidx] = 450.0 - rtd * wvc->selected->dir;
            while(smap_high_dir[l2bidx]>=180) smap_high_dir[l2bidx]-=360;
            while(smap_high_dir[l2bidx]<-180) smap_high_dir[l2bidx]+=360;
        }
    }

    cap_wind_swath.MedianFilter(3, 200, 2, 0);
    for(int ati=0; ati<nati; ++ati) {
        for(int cti=0; cti<ncti; ++cti) {

            int l2bidx = ati + nati*cti;
            CAPWVC* wvc = cap_wind_swath.swath[cti][ati];

            if(!wvc)
                continue;

            smap_high_dir_smooth[l2bidx] = 450.0 - rtd * wvc->selected->dir;
            while(smap_high_dir_smooth[l2bidx]>=180) smap_high_dir_smooth[l2bidx]-=360;
            while(smap_high_dir_smooth[l2bidx]<-180) smap_high_dir_smooth[l2bidx]+=360;
        }
    }

    // Write out HDF5 file
    hid_t file_id = H5Fcreate(
        l2b_tb_file, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

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
        file_id, "/", "TB_CRID", config_list.Get("TB_CRID"));

    H5LTset_attribute_string(
        file_id, "/", "L1B_TB_LORES_ASC_FILE",
        config_list.Get("L1B_TB_LORES_ASC_FILE"));

    H5LTset_attribute_string(
        file_id, "/", "L1B_TB_LORES_DEC_FILE",
        config_list.Get("L1B_TB_LORES_DEC_FILE"));

    H5LTset_attribute_float(
        file_id, "/", "Delta TBH Fore Ascending", &dtbh_fore_asc, 1);
    H5LTset_attribute_float(
        file_id, "/", "Delta TBH Aft Ascending", &dtbh_aft_asc, 1);
    H5LTset_attribute_float(
        file_id, "/", "Delta TBV Fore Ascending", &dtbv_fore_asc, 1);
    H5LTset_attribute_float(
        file_id, "/", "Delta TBV Aft Ascending", &dtbv_aft_asc, 1);

    H5LTset_attribute_float(
        file_id, "/", "Delta TBH Fore Decending", &dtbh_fore_dec, 1);
    H5LTset_attribute_float(
        file_id, "/", "Delta TBH Aft Decending", &dtbh_aft_dec, 1);
    H5LTset_attribute_float(
        file_id, "/", "Delta TBV Fore Decending", &dtbv_fore_dec, 1);
    H5LTset_attribute_float(
        file_id, "/", "Delta TBV Aft Decending", &dtbv_aft_dec, 1);

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

    float valid_max, valid_min;

    hsize_t dims[2] = {ncti, nati};

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

    valid_max = 340; valid_min = 0;
    H5LTmake_dataset(
        file_id, "tb_h_fore", 2, dims, H5T_NATIVE_FLOAT, &tb_h_fore[0]);
    H5LTset_attribute_string(
        file_id, "tb_h_fore", "long_name",
        "Average brightness temperature for all H-pol fore looks");
    H5LTset_attribute_string(file_id, "tb_h_fore", "units", "Degrees kelvin");
    H5LTset_attribute_float(file_id, "tb_h_fore", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(file_id, "tb_h_fore", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "tb_h_fore", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "tb_h_aft", 2, dims, H5T_NATIVE_FLOAT, &tb_h_aft[0]);
    H5LTset_attribute_string(
        file_id, "tb_h_aft", "long_name",
        "Average brightness temperature for all H-pol aft looks");
    H5LTset_attribute_string(file_id, "tb_h_aft", "units", "Degrees kelvin");
    H5LTset_attribute_float(file_id, "tb_h_aft", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(file_id, "tb_h_aft", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "tb_h_aft", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "tb_v_fore", 2, dims, H5T_NATIVE_FLOAT, &tb_v_fore[0]);
    H5LTset_attribute_string(
        file_id, "tb_v_fore", "long_name",
        "Average brightness temperature for all V-pol fore looks");
    H5LTset_attribute_string(file_id, "tb_v_fore", "units", "Degrees kelvin");
    H5LTset_attribute_float(file_id, "tb_v_fore", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(file_id, "tb_v_fore", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "tb_v_fore", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "tb_v_aft", 2, dims, H5T_NATIVE_FLOAT, &tb_v_aft[0]);
    H5LTset_attribute_string(
        file_id, "tb_v_aft", "long_name",
        "Average brightness temperature for all V-pol aft looks");
    H5LTset_attribute_string(file_id, "tb_v_aft", "units", "Degrees kelvin");
    H5LTset_attribute_float(file_id, "tb_v_aft", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(file_id, "tb_v_aft", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "tb_v_aft", "valid_min", &valid_min, 1);

    valid_max = 3; valid_min = -3;
    H5LTmake_dataset(
        file_id, "tb_v_bias_adj", 2, dims, H5T_NATIVE_FLOAT, &tb_v_bias_adj[0]);
    H5LTset_attribute_string(
        file_id, "tb_v_bias_adj", "long_name",
        "Brightness temperature bias adjustment for all V-pol looks");
    H5LTset_attribute_string(file_id, "tb_v_bias_adj", "units", "Degrees kelvin");
    H5LTset_attribute_float(file_id, "tb_v_bias_adj", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(file_id, "tb_v_bias_adj", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "tb_v_bias_adj", "valid_min", &valid_min, 1);

    valid_max = 3; valid_min = -3;
    H5LTmake_dataset(
        file_id, "tb_h_bias_adj", 2, dims, H5T_NATIVE_FLOAT, &tb_h_bias_adj[0]);
    H5LTset_attribute_string(
        file_id, "tb_h_bias_adj", "long_name",
        "Brightness temperature bias adjustment for all H-pol looks");
    H5LTset_attribute_string(file_id, "tb_h_bias_adj", "units", "Degrees kelvin");
    H5LTset_attribute_float(file_id, "tb_h_bias_adj", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(file_id, "tb_h_bias_adj", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "tb_h_bias_adj", "valid_min", &valid_min, 1);

    valid_max = 3; valid_min = 0;
    H5LTmake_dataset(
        file_id, "nedt_h_fore", 2, dims, H5T_NATIVE_FLOAT, &nedt_h_fore[0]);
    H5LTset_attribute_string(
        file_id, "nedt_h_fore", "long_name",
        "Aggregated noise equivilent Delta T for H-pol fore look");
    H5LTset_attribute_string(file_id, "nedt_h_fore", "units", "Degrees kelvin");
    H5LTset_attribute_float(file_id, "nedt_h_fore", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(file_id, "nedt_h_fore", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "nedt_h_fore", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "nedt_h_aft", 2, dims, H5T_NATIVE_FLOAT, &nedt_h_aft[0]);
    H5LTset_attribute_string(
        file_id, "nedt_h_aft", "long_name",
        "Aggregated noise equivilent Delta T for H-pol aft look");
    H5LTset_attribute_string(file_id, "nedt_h_aft", "units", "Degrees kelvin");
    H5LTset_attribute_float(file_id, "nedt_h_aft", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(file_id, "nedt_h_aft", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "nedt_h_aft", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "nedt_v_fore", 2, dims, H5T_NATIVE_FLOAT, &nedt_v_fore[0]);
    H5LTset_attribute_string(
        file_id, "nedt_v_fore", "long_name",
        "Aggregated noise equivilent Delta T for V-pol fore look");
    H5LTset_attribute_string(file_id, "nedt_v_fore", "units", "Degrees kelvin");
    H5LTset_attribute_float(file_id, "nedt_v_fore", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(file_id, "nedt_v_fore", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "nedt_v_fore", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "nedt_v_aft", 2, dims, H5T_NATIVE_FLOAT, &nedt_v_aft[0]);
    H5LTset_attribute_string(
        file_id, "nedt_v_aft", "long_name",
        "Aggregated noise equivilent Delta T for V-pol aft look");
    H5LTset_attribute_string(file_id, "nedt_v_aft", "units", "Degrees kelvin");
    H5LTset_attribute_float(file_id, "nedt_v_aft", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(file_id, "nedt_v_aft", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "nedt_v_aft", "valid_min", &valid_min, 1);

    unsigned char uchar_fill_value = 0;
    H5LTmake_dataset(
        file_id, "n_h_fore", 2, dims, H5T_NATIVE_UCHAR, &n_h_fore[0]);
    H5LTset_attribute_string(
        file_id, "n_h_fore", "long_name",
        "Number of L1B TBs aggregated into H-pol fore look");
    H5LTset_attribute_uchar(file_id, "n_h_fore", "_FillValue", &uchar_fill_value, 1);

    H5LTmake_dataset(
        file_id, "n_h_aft", 2, dims, H5T_NATIVE_UCHAR, &n_h_aft[0]);
    H5LTset_attribute_string(
        file_id, "n_h_aft", "long_name",
        "Number of L1B TBs aggregated into H-pol aft look");
    H5LTset_attribute_uchar(file_id, "n_h_aft", "_FillValue", &uchar_fill_value, 1);

    H5LTmake_dataset(
        file_id, "n_v_fore", 2, dims, H5T_NATIVE_UCHAR, &n_v_fore[0]);
    H5LTset_attribute_string(
        file_id, "n_v_fore", "long_name",
        "Number of L1B TBs aggregated into V-pol fore look");
    H5LTset_attribute_uchar(file_id, "n_v_fore", "_FillValue", &uchar_fill_value, 1);

    H5LTmake_dataset(
        file_id, "n_v_aft", 2, dims, H5T_NATIVE_UCHAR, &n_v_aft[0]);
    H5LTset_attribute_string(
        file_id, "n_v_aft", "long_name",
        "Number of L1B TBs aggregated into V-pol aft look");
    H5LTset_attribute_uchar(file_id, "n_v_aft", "_FillValue", &uchar_fill_value, 1);

    H5LTmake_dataset(
        file_id, "num_ambiguities", 2, dims, H5T_NATIVE_UCHAR,
        &num_ambiguities[0]);
    H5LTset_attribute_string(
        file_id, "num_ambiguities", "long_name",
        "Number of wind vector ambiguties");
    H5LTset_attribute_uchar(
        file_id, "num_ambiguities", "_FillValue", &uchar_fill_value, 1);

    valid_max = 90; valid_min = 0;
    H5LTmake_dataset(
        file_id, "inc_fore", 2, dims, H5T_NATIVE_FLOAT, &inc_fore[0]);
    H5LTset_attribute_string(
        file_id, "inc_fore", "long_name", "Cell incidence angle fore look");
    H5LTset_attribute_string(file_id, "inc_fore", "units", "Degrees");
    H5LTset_attribute_float(file_id, "inc_fore", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(file_id, "inc_fore", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "inc_fore", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "inc_aft", 2, dims, H5T_NATIVE_FLOAT, &inc_aft[0]);
    H5LTset_attribute_string(
        file_id, "inc_aft", "long_name", "Cell incidence angle aft look");
    H5LTset_attribute_string(file_id, "inc_aft", "units", "Degrees");
    H5LTset_attribute_float(file_id, "inc_aft", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(file_id, "inc_aft", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "inc_aft", "valid_min", &valid_min, 1);

    valid_max = 180; valid_min = -180;
    H5LTmake_dataset(
        file_id, "azi_fore", 2, dims, H5T_NATIVE_FLOAT, &azi_fore[0]);
    H5LTset_attribute_string(
        file_id, "azi_fore", "long_name", "Cell azimuth angle fore look");
    H5LTset_attribute_string(file_id, "azi_fore", "units", "Degrees");
    H5LTset_attribute_float(file_id, "azi_fore", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(file_id, "azi_fore", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "azi_fore", "valid_min", &valid_min, 1);

    H5LTmake_dataset(file_id, "azi_aft", 2, dims, H5T_NATIVE_FLOAT, &azi_aft[0]);
    H5LTset_attribute_string(
        file_id, "azi_aft", "long_name", "Cell azimuth angle aft look");
    H5LTset_attribute_string(file_id, "azi_aft", "units", "Degrees");
    H5LTset_attribute_float(file_id, "azi_aft", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(file_id, "azi_aft", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "azi_aft", "valid_min", &valid_min, 1);

    valid_max = 100; valid_min = 0;
    H5LTmake_dataset(file_id, "anc_spd", 2, dims, H5T_NATIVE_FLOAT, &anc_spd[0]);
    H5LTset_attribute_string(
        file_id, "anc_spd", "long_name",
        "10 meter NCEP wind speed (scaled by 1.03)");
    H5LTset_attribute_string(file_id, "anc_spd", "units", "Meters/second");
    H5LTset_attribute_float(file_id, "anc_spd", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(file_id, "anc_spd", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "anc_spd", "valid_min", &valid_min, 1);

    valid_max = 180; valid_min = -180;
    H5LTmake_dataset(file_id, "anc_dir", 2, dims, H5T_NATIVE_FLOAT, &anc_dir[0]);
    H5LTset_attribute_string(
        file_id, "anc_dir", "long_name",
        "NCEP wind direction (oceanographic convention)");
    H5LTset_attribute_string(file_id, "anc_dir", "units", "Degrees");
    H5LTset_attribute_float(file_id, "anc_dir", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(file_id, "anc_dir", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "anc_dir", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "smap_high_dir", 2, dims, H5T_NATIVE_FLOAT, &smap_high_dir[0]);
    H5LTset_attribute_string(
        file_id, "smap_high_dir", "long_name",
        "SMAP wind direction using ancillary SSS");
    H5LTset_attribute_string(
        file_id, "smap_high_dir", "units", "Degrees");
    H5LTset_attribute_float(
        file_id, "smap_high_dir", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(
        file_id, "smap_high_dir", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "smap_high_dir", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "smap_high_dir_smooth", 2, dims, H5T_NATIVE_FLOAT,
        &smap_high_dir_smooth[0]);
    H5LTset_attribute_string(
        file_id, "smap_high_dir_smooth", "long_name",
        "SMAP wind direction using ancillary SSS and DIRTH smoothing");
    H5LTset_attribute_string(
        file_id, "smap_high_dir_smooth", "units", "Degrees");
    H5LTset_attribute_float(
        file_id, "smap_high_dir_smooth", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(
        file_id, "smap_high_dir_smooth", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "smap_high_dir_smooth", "valid_min", &valid_min, 1);

    valid_max = 40; valid_min = 0;
    H5LTmake_dataset(file_id, "anc_sss", 2, dims, H5T_NATIVE_FLOAT, &anc_sss[0]);
    H5LTset_attribute_string(
        file_id, "anc_sss", "long_name", "HYCOM salinity");
    H5LTset_attribute_string(file_id, "anc_sss", "units", "PSU");
    H5LTset_attribute_float(file_id, "anc_sss", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(file_id, "anc_sss", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "anc_sss", "valid_min", &valid_min, 1);

    valid_max = 340; valid_min = 0;
    H5LTmake_dataset(file_id, "anc_sst", 2, dims, H5T_NATIVE_FLOAT, &anc_sst[0]);
    H5LTset_attribute_string(
        file_id, "anc_sst", "long_name",
        "NOAA Optimum Interpolation sea surface temperature");
    H5LTset_attribute_string(file_id, "anc_sst", "units", "Degrees kelvin");
    H5LTset_attribute_float(file_id, "anc_sst", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(file_id, "anc_sst", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "anc_sst", "valid_min", &valid_min, 1);

    valid_max = 25; valid_min = 0;
    H5LTmake_dataset(file_id, "anc_swh", 2, dims, H5T_NATIVE_FLOAT, &anc_swh[0]);
    H5LTset_attribute_string(
        file_id, "anc_swh", "long_name",
        "NOAA WaveWatch III significant wave height");
    H5LTset_attribute_string(file_id, "anc_swh", "units", "Meters");
    H5LTset_attribute_float(file_id, "anc_swh", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(file_id, "anc_swh", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "anc_swh", "valid_min", &valid_min, 1);

    valid_max = 100; valid_min = 0;
    H5LTmake_dataset(file_id, "smap_spd", 2, dims, H5T_NATIVE_FLOAT, &smap_spd[0]);
    H5LTset_attribute_string(
        file_id, "smap_spd", "long_name", "SMAP wind speed");
    H5LTset_attribute_string(file_id, "smap_spd", "units", "Meters/second");
    H5LTset_attribute_float(file_id, "smap_spd", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(file_id, "smap_spd", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "smap_spd", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "smap_high_spd", 2, dims, H5T_NATIVE_FLOAT, &smap_high_spd[0]);
    H5LTset_attribute_string(
        file_id, "smap_high_spd", "long_name",
        "SMAP wind speed using ancillary SSS");
    H5LTset_attribute_string(
        file_id, "smap_high_spd", "units", "Meters/second");
    H5LTset_attribute_float(
        file_id, "smap_high_spd", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(
        file_id, "smap_high_spd", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "smap_high_spd", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "smap_spd_bias_adj", 2, dims, H5T_NATIVE_FLOAT,
        &smap_spd_bias_adj[0]);
    H5LTset_attribute_string(
        file_id, "smap_spd_bias_adj", "long_name",
        "SMAP wind speed using bias adjusted brightness temperatures");
    H5LTset_attribute_string(
        file_id, "smap_spd_bias_adj", "units", "Meters/second");
    H5LTset_attribute_float(
        file_id, "smap_spd_bias_adj", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(
        file_id, "smap_spd_bias_adj", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "smap_spd_bias_adj", "valid_min", &valid_min, 1);

    valid_max = 40; valid_min = 0;
    H5LTmake_dataset(file_id, "smap_sss", 2, dims, H5T_NATIVE_FLOAT, &smap_sss[0]);
    H5LTset_attribute_string(
        file_id, "smap_sss", "long_name", "SMAP sea surface salinity");
    H5LTset_attribute_string(file_id, "smap_sss", "units", "PSU");
    H5LTset_attribute_float(file_id, "smap_sss", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(file_id, "smap_sss", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "smap_sss", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "smap_sss_bias_adj", 2, dims, H5T_NATIVE_FLOAT,
        &smap_sss_bias_adj[0]);
    H5LTset_attribute_string(
        file_id, "smap_sss_bias_adj", "long_name",
        "SMAP sea surface salinity using bias adjusted brightness temperatures");
    H5LTset_attribute_string(
        file_id, "smap_sss_bias_adj", "units", "PSU");
    H5LTset_attribute_float(
        file_id, "smap_sss_bias_adj", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(
        file_id, "smap_sss_bias_adj", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "smap_sss_bias_adj", "valid_min", &valid_min, 1);

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
        file_id, "quality_flag", 2, dims, H5T_NATIVE_USHORT, &quality_flag[0]);
    H5LTset_attribute_string(
        file_id, "quality_flag", "long_name", "Quality flag");

    uint16 flag_bits;
    flag_bits = QUAL_FLAG_USEABLE;
    H5LTset_attribute_ushort(
        file_id, "quality_flag", "QUAL_FLAG_USABLE", &flag_bits, 1);

    flag_bits = QUAL_FLAG_FOUR_LOOKS;
    H5LTset_attribute_ushort(
        file_id, "quality_flag", "QUAL_FLAG_FOUR_LOOKS", &flag_bits, 1);

    flag_bits = QUAL_FLAG_POINTING;
    H5LTset_attribute_ushort(
        file_id, "quality_flag", "QUAL_FLAG_POINTING", &flag_bits, 1);

    flag_bits = QUAL_FLAG_LAND;
    H5LTset_attribute_ushort(
        file_id, "quality_flag", "QUAL_FLAG_LAND", &flag_bits, 1);

    flag_bits = QUAL_FLAG_ICE;
    H5LTset_attribute_ushort(
        file_id, "quality_flag", "QUAL_FLAG_ICE", &flag_bits, 1);

    flag_bits = QUAL_FLAG_SST_TOO_COLD;
    H5LTset_attribute_ushort(
        file_id, "quality_flag", "QUAL_FLAG_SST_TOO_COLD", &flag_bits, 1);

    H5LTset_attribute_ushort(
        file_id, "quality_flag", "_FillValue", &_ushort_fill_value, 1);

    H5Fclose(file_id);
    return(0);
}

