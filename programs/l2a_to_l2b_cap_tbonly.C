//==============================================================//
// Copyright (C) 2015, California Institute of Technology.      //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#define L2A_FILE_KEYWORD "L2A_FILE"
#define L2B_CAP_FILE_KEYWORD "L2B_CAP_FILE"
#define L2B_TB_FILE_KEYWORD "L2B_TB_FILE"
#define TB_FLAT_MODEL_FILE_KEYWORD "TB_FLAT_MODEL_FILE"
#define TB_ROUGH_MODEL_FILE_KEYWORD "TB_ROUGH_MODEL_FILE"
#define S0_ROUGH_MODEL_FILE_KEYWORD "S0_ROUGH_MODEL_FILE"
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
#include "L2BTBOnly.h"

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

int main(int argc, char* argv[]) {

    const char* command = no_path(argv[0]);
    char* config_file = argv[1];
    char* l2b_file_s3 = NULL;
    if(argc == 3)
        l2b_file_s3 = argv[2];

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

    char* l2a_s0_file = config_list.Get(L2A_FILE_KEYWORD);
    char* l2b_s0_file = config_list.Get(L2B_FILE_KEYWORD);
    char* l2b_tb_file = config_list.Get(L2B_TB_FILE_KEYWORD);
    char* l2b_cap_file = config_list.Get(L2B_CAP_FILE_KEYWORD);
    char* tb_flat_file = config_list.Get(TB_FLAT_MODEL_FILE_KEYWORD);
    char* tb_rough_file = config_list.Get(TB_ROUGH_MODEL_FILE_KEYWORD);
    char* s0_rough_file = config_list.Get(S0_ROUGH_MODEL_FILE_KEYWORD);

    // Configure the model functions
    CAPGMF cap_gmf;
    cap_gmf.ReadFlat(tb_flat_file);
    cap_gmf.ReadRough(tb_rough_file);
    cap_gmf.ReadModelS0(s0_rough_file);

    L2A l2a_s0;
    l2a_s0.SetInputFilename(l2a_s0_file);
    l2a_s0.OpenForReading();
    l2a_s0.ReadHeader();

    // Read in L2B radar-only file
    L2B l2b_s0;
    l2b_s0.SetInputFilename(l2b_s0_file);
    l2b_s0.OpenForReading();
    l2b_s0.ReadHeader();
    l2b_s0.ReadDataRec();
    l2b_s0.Close();

    L2B l2b_s3_s0;
    if(l2b_file_s3) {
        l2b_s3_s0.SetInputFilename(l2b_file_s3);
        l2b_s3_s0.OpenForReading();
        l2b_s3_s0.ReadHeader();
        l2b_s3_s0.ReadDataRec();
        l2b_s3_s0.Close();
    }

    printf("%s\n", l2b_tb_file);
    L2BTBOnly l2b_tbonly(l2b_tb_file);

    int ncti = l2b_s0.frame.swath.GetCrossTrackBins();
    int nati = l2b_s0.frame.swath.GetAlongTrackBins();

    if(l2a_s0.header.alongTrackBins != nati || l2b_tbonly.nati != nati ||
       l2a_s0.header.crossTrackBins != ncti || l2b_tbonly.ncti != ncti ||
       (l2b_file_s3 && l2b_s3_s0.frame.swath.GetAlongTrackBins() != nati) ||
       (l2b_file_s3 && l2b_s3_s0.frame.swath.GetCrossTrackBins() != ncti) ) {
        fprintf(stderr, "Size mismatch!\n");
        exit(1);
    }

    // Read in L2A S0 file
    L2AFrame*** l2a_s0_swath;
    l2a_s0_swath = (L2AFrame***)make_array(sizeof(L2AFrame *), 2, ncti, nati);
    while(l2a_s0.ReadDataRec()) {
        L2AFrame* this_frame = new L2AFrame();
        this_frame->CopyFrame(this_frame, &l2a_s0.frame);
         *(*(l2a_s0_swath + this_frame->cti) + this_frame->ati) = this_frame;
    }
    l2a_s0.Close();

    CAPWindSwath cap_wind_swath;
    cap_wind_swath.Allocate(ncti, nati);

    // Output arrays
    int l2b_size = ncti * nati;

    for(int ati=0; ati<nati; ++ati) {
        if(ati%100 == 0)
            fprintf(stdout, "%d of %d\n", ati, nati);

        for(int cti=0; cti<ncti; ++cti) {

            int l2bidx = ati + nati*cti;

            // Check for valid measList at this WVC
            if(!l2a_s0_swath[cti][ati])
                continue;

            MeasList* s0_ml = &(l2a_s0_swath[cti][ati]->measList);

            int any_ice = 0;
            int any_land = 0;

            // Check for land / ice flagged sigma0s
            Meas* meas = s0_ml->GetHead();
            while(meas) {
                if(meas->landFlag) {
                    if(meas->landFlag == 1 || meas->landFlag == 3)
                        any_land = 1;

                    if(meas->landFlag == 2 || meas->landFlag == 3)
                        any_ice = 1;

                    meas = s0_ml->RemoveCurrent();
                    delete meas;
                    meas = s0_ml->GetCurrent();
                } else {
                    meas = s0_ml->GetNext();
                }
            }

            MeasList tb_ml;

            // Build up tb_ml from L2B_TB_FILE
            if(l2b_tbonly.n_v_fore[l2bidx] > 0) {
                Meas* this_meas = new Meas();
                this_meas->value = 
                    l2b_tbonly.tb_v_fore[l2bidx];
                this_meas->measType = Meas::L_BAND_TBV_MEAS_TYPE;
                this_meas->incidenceAngle = dtr * l2b_tbonly.inc_fore[l2bidx];
                this_meas->eastAzimuth = gs_deg_to_pe_rad(
                    l2b_tbonly.azi_fore[l2bidx]);
                this_meas->A = pow(l2b_tbonly.nedt_v_fore[l2bidx], 2);
                tb_ml.Append(this_meas);
            }

            if(l2b_tbonly.n_h_fore[l2bidx] > 0) {
                Meas* this_meas = new Meas();
                this_meas->value = 
                    l2b_tbonly.tb_h_fore[l2bidx];
                this_meas->measType = Meas::L_BAND_TBH_MEAS_TYPE;
                this_meas->incidenceAngle = dtr * l2b_tbonly.inc_fore[l2bidx];
                this_meas->eastAzimuth = gs_deg_to_pe_rad(
                    l2b_tbonly.azi_fore[l2bidx]);
                this_meas->A = pow(l2b_tbonly.nedt_h_fore[l2bidx], 2);
                tb_ml.Append(this_meas);
            }

            if(l2b_tbonly.n_v_aft[l2bidx] > 0) {
                Meas* this_meas = new Meas();
                this_meas->value =
                    l2b_tbonly.tb_v_aft[l2bidx];
                this_meas->measType = Meas::L_BAND_TBV_MEAS_TYPE;
                this_meas->incidenceAngle = dtr * l2b_tbonly.inc_aft[l2bidx];
                this_meas->eastAzimuth = gs_deg_to_pe_rad(
                    l2b_tbonly.azi_aft[l2bidx]);
                this_meas->A = pow(l2b_tbonly.nedt_v_aft[l2bidx], 2);
                tb_ml.Append(this_meas);
            }

            if(l2b_tbonly.n_h_aft[l2bidx] > 0) {
                Meas* this_meas = new Meas();
                this_meas->value =
                    l2b_tbonly.tb_h_aft[l2bidx];
                this_meas->measType = Meas::L_BAND_TBH_MEAS_TYPE;
                this_meas->incidenceAngle = dtr * l2b_tbonly.inc_aft[l2bidx];
                this_meas->eastAzimuth = gs_deg_to_pe_rad(
                    l2b_tbonly.azi_aft[l2bidx]);
                this_meas->A = pow(l2b_tbonly.nedt_h_aft[l2bidx], 2);
                tb_ml.Append(this_meas);
            }

            WVC* s0_wvc = l2b_s0.frame.swath.GetWVC(cti, ati);
            WVC* s3_wvc = NULL;
            if(l2b_file_s3)
                s3_wvc = l2b_s3_s0.frame.swath.GetWVC(cti, ati);

            if(!s0_wvc || tb_ml.NodeCount() == 0 || s0_ml->NodeCount() == 0)
                continue;

            CAPWVC* wvc = new CAPWVC();
            WindVectorPlus* nudgeWV = new WindVectorPlus();
            nudgeWV->spd = s0_wvc->nudgeWV->spd;
            nudgeWV->dir = s0_wvc->nudgeWV->dir;

            wvc->nudgeWV = nudgeWV;
            wvc->s0_flag = s0_wvc->qualFlag;

            float init_spd = nudgeWV->spd;
            float init_dir = nudgeWV->dir;
            float init_sss = l2b_tbonly.smap_sss[l2bidx];
            float this_anc_spd = nudgeWV->spd;
            float this_anc_dir = nudgeWV->dir;
            float this_anc_sst = l2b_tbonly.anc_sst[l2bidx];
            float this_anc_swh = l2b_tbonly.anc_swh[l2bidx];
            float this_anc_rr = -9999;
            float anc_spd_std_prior = 1000;

            float active_weight = 1;
            float passive_weight = 1;

            if(this_anc_swh<0 || this_anc_swh > 20)
                this_anc_swh = -9999;

            // If radar-only file specified use that for nudging
            if(l2b_file_s3 && s3_wvc) {
                init_spd = s3_wvc->selected->spd;
                init_dir = s3_wvc->selected->dir;

                if(init_spd>20) {
                    float new_spd, new_dir, new_sss, new_obj;
                    cap_gmf.Retrieve(
                        &tb_ml, NULL, init_spd, init_dir,
                        init_sss, init_spd, init_dir,
                        this_anc_sst, this_anc_swh, 0, 1.5, 0,
                        1, CAPGMF::RETRIEVE_SPEED_SALINITY,
                        &new_spd, &new_dir, &new_sss, &new_obj);

                    if(init_spd < 30) {
                        init_sss += (new_sss-init_sss) * (init_spd-20) / 10;
                    } else {
                        init_sss = new_sss;
                    }
                }
            }

            cap_gmf.CAPGMF::BuildSolutionCurvesTwoStep(
                &tb_ml, s0_ml, init_spd, init_sss, this_anc_spd, this_anc_dir,
                this_anc_sst, this_anc_swh, this_anc_rr, anc_spd_std_prior,
                active_weight, passive_weight, wvc);

//             cap_gmf.CAPGMF::BuildSolutionCurves(
//                 &tb_ml, s0_ml, init_spd, init_sss, this_anc_spd, this_anc_dir,
//                 this_anc_sst, this_anc_swh, this_anc_rr, anc_spd_std_prior,
//                 active_weight, passive_weight, wvc);

            wvc->BuildSolutions();

            int asdf = 0;

            if(wvc->ambiguities.NodeCount() > 0) {
                cap_wind_swath.Add(cti, ati, wvc);
                wvc->selected = wvc->GetNearestAmbig(init_dir);
            } else {
                delete wvc;
            }
        }
    }

    // Do ambig removal only if radar-only l2b file not specified
    if(!l2b_file_s3) {
        cap_wind_swath.ThreshNudge(0.05);
        cap_wind_swath.MedianFilter(3, 200, 0);

    } else {
        cap_wind_swath.MedianFilter(3, 200, 2);
    }

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

    // Write outputs
    std::vector<float> row_time(nati);
    std::vector<float> lat(l2b_size), lon(l2b_size);
    std::vector<float> inc_fore(l2b_size), inc_aft(l2b_size);
    std::vector<float> azi_fore(l2b_size), azi_aft(l2b_size);
    std::vector<float> anc_spd(l2b_size), anc_dir(l2b_size), anc_sss(l2b_size);
    std::vector<float> anc_sst(l2b_size), anc_swh(l2b_size);
    
    std::vector<float> smap_cap_sss(l2b_size);
    std::vector<float> smap_cap_spd(l2b_size), smap_cap_dir(l2b_size);
    std::vector<float> smap_radar_spd(l2b_size), smap_radar_dir(l2b_size);

    std::vector<uint8> num_ambiguities(l2b_size);
    std::vector<float> smap_cap_ambiguity_spd(l2b_size*4);
    std::vector<float> smap_cap_ambiguity_dir(l2b_size*4);

    std::vector<float> sigma0_hh_fore(l2b_size), sigma0_hh_aft(l2b_size);
    std::vector<float> sigma0_vv_fore(l2b_size), sigma0_vv_aft(l2b_size);
    std::vector<float> sigma0_hv_fore(l2b_size), sigma0_hv_aft(l2b_size);
    std::vector<float> sigma0_vh_fore(l2b_size), sigma0_vh_aft(l2b_size);

    std::vector<float> sigma0_hh_fore_std(l2b_size), sigma0_hh_aft_std(l2b_size);
    std::vector<float> sigma0_vv_fore_std(l2b_size), sigma0_vv_aft_std(l2b_size);
    std::vector<float> sigma0_hv_fore_std(l2b_size), sigma0_hv_aft_std(l2b_size);
    std::vector<float> sigma0_vh_fore_std(l2b_size), sigma0_vh_aft_std(l2b_size);

    std::vector<uint16> quality_flag(l2b_size);

    for(int ati=0; ati<nati; ++ati) {

        row_time[ati] = 
            t_start + (t_stop-t_start)*(double)ati/(double)nati - t_day_start;

        for(int cti=0; cti<ncti; ++cti) {
            int l2bidx = ati + nati*cti;

            // Set defaults
            lat[l2bidx] = FILL_VALUE;
            lon[l2bidx] = FILL_VALUE;
            inc_fore[l2bidx] = FILL_VALUE;
            inc_aft[l2bidx] = FILL_VALUE;
            azi_fore[l2bidx] = FILL_VALUE;
            azi_aft[l2bidx] = FILL_VALUE;
            anc_spd[l2bidx] = FILL_VALUE;
            anc_dir[l2bidx] = FILL_VALUE;
            anc_sss[l2bidx] = FILL_VALUE;
            anc_sst[l2bidx] = FILL_VALUE;
            anc_swh[l2bidx] = FILL_VALUE;
            smap_cap_sss[l2bidx] = FILL_VALUE;
            smap_cap_spd[l2bidx] = FILL_VALUE;
            smap_cap_dir[l2bidx] = FILL_VALUE;
            smap_radar_spd[l2bidx] = FILL_VALUE;
            smap_radar_dir[l2bidx] = FILL_VALUE;
            sigma0_hh_fore[l2bidx] = FILL_VALUE;
            sigma0_hh_aft[l2bidx] = FILL_VALUE;
            sigma0_vv_fore[l2bidx] = FILL_VALUE;
            sigma0_vv_aft[l2bidx] = FILL_VALUE;
            sigma0_hv_fore[l2bidx] = FILL_VALUE;
            sigma0_hv_aft[l2bidx] = FILL_VALUE;
            sigma0_vh_fore[l2bidx] = FILL_VALUE;
            sigma0_vh_aft[l2bidx] = FILL_VALUE;
            sigma0_hh_fore_std[l2bidx] = FILL_VALUE;
            sigma0_hh_aft_std[l2bidx] = FILL_VALUE;
            sigma0_vv_fore_std[l2bidx] = FILL_VALUE;
            sigma0_vv_aft_std[l2bidx] = FILL_VALUE;
            sigma0_hv_fore_std[l2bidx] = FILL_VALUE;
            sigma0_hv_aft_std[l2bidx] = FILL_VALUE;
            sigma0_vh_fore_std[l2bidx] = FILL_VALUE;
            sigma0_vh_aft_std[l2bidx] = FILL_VALUE;
            num_ambiguities[l2bidx] = 0;
            quality_flag[l2bidx] = 65535;

            for(int i_amb = 0; i_amb < 4; ++i_amb) {
                int ambidx = i_amb + 4*l2bidx;
                smap_cap_ambiguity_spd[ambidx] = FILL_VALUE;
                smap_cap_ambiguity_dir[ambidx] = FILL_VALUE;
            }

            MeasList* s0_ml = &(l2a_s0_swath[cti][ati]->measList);
            CAPWVC* wvc = cap_wind_swath.swath[cti][ati];
            WVC* s0_wvc = l2b_s0.frame.swath.GetWVC(cti, ati);
            if(l2b_file_s3)
                s0_wvc = l2b_s3_s0.frame.swath.GetWVC(cti, ati);

            if(!wvc || !s0_wvc)
                continue;

            // Average the Sigma0s into looks
            float sum_s0[2][4]; // [fore-0, aft-1][v-0, h-1]
            float sum2_s0[2][4]; // [fore-0, aft-1][v-0, h-1]
            float sum_A[2][4]; // [fore-0, aft-1][v-0, h-1]
            int cnts[2][4];

            float sum_cos_azi[2]; // fore - 0; aft 1
            float sum_sin_azi[2];
            float sum_inc[2];

            for(int ilook = 0; ilook < 2; ++ilook) {
                sum_inc[ilook] = 0;
                sum_sin_azi[ilook] = 0;
                sum_cos_azi[ilook] = 0;
                for(int ipol = 0; ipol < 4; ++ipol) {
                    sum_s0[ilook][ipol] = 0;
                    sum2_s0[ilook][ipol] = 0;
                    sum_A[ilook][ipol] = 0;
                    cnts[ilook][ipol] = 0;
                }
            }

            int any_ice = 0;
            int any_land = 0;

            for(Meas* meas = s0_ml->GetHead(); meas; meas = s0_ml->GetNext()) {
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

                if(meas->measType == Meas::VV_MEAS_TYPE) {
                    idx_pol = 0;
                } else if(meas->measType == Meas::HH_MEAS_TYPE) {
                    idx_pol = 1;
                } else if(meas->measType == Meas::VH_MEAS_TYPE) {
                    idx_pol = 2;
                } else if(meas->measType == Meas::HV_MEAS_TYPE) {
                    idx_pol = 3;
                } else {
                    continue;
                }

                cnts[idx_look][idx_pol]++;
                sum_s0[idx_look][idx_pol] += meas->value;
                sum2_s0[idx_look][idx_pol] += pow(meas->value, 2);
                sum_A[idx_look][idx_pol] += meas->A;
                sum_inc[idx_look] += meas->incidenceAngle;
                sum_cos_azi[idx_look] += cos(meas->eastAzimuth);
                sum_sin_azi[idx_look] += sin(meas->eastAzimuth);
            }

            int cnts_fore = cnts[0][0] + cnts[0][1] + cnts[0][2] + cnts[0][3];
            if(cnts_fore>0) {
                inc_fore[l2bidx] = rtd*sum_inc[0]/(float)cnts_fore;
                azi_fore[l2bidx] = rtd*atan2(sum_cos_azi[0], sum_sin_azi[0]);
            }
            int cnts_aft = cnts[1][0] + cnts[1][1] + cnts[1][2] + cnts[1][3];
            if(cnts_aft>0) {
                inc_aft[l2bidx] = rtd*sum_inc[1]/(float)cnts_aft;
                azi_aft[l2bidx] = rtd*atan2(sum_cos_azi[1], sum_sin_azi[1]);
            }

            if(cnts[0][0]) {
                sigma0_vv_fore[l2bidx] = sum_s0[0][0]/(float)cnts[0][0];
                sigma0_vv_fore_std[l2bidx] = sqrt(
                    sum2_s0[0][0]/(float)cnts[0][0]-
                    pow(sigma0_vv_fore[l2bidx], 2));
            }

            if(cnts[1][0]) {
                sigma0_vv_aft[l2bidx] = sum_s0[1][0]/(float)cnts[1][0];
                sigma0_vv_aft_std[l2bidx] = sqrt(
                    sum2_s0[1][0]/(float)cnts[1][0]-
                    pow(sigma0_vv_aft[l2bidx], 2));
            }

            if(cnts[0][1]) {
                sigma0_hh_fore[l2bidx] = sum_s0[0][1]/(float)cnts[0][1];
                sigma0_hh_fore_std[l2bidx] = sqrt(
                    sum2_s0[0][1]/(float)cnts[0][1]-
                    pow(sigma0_hh_fore[l2bidx], 2));
            }

            if(cnts[1][1]) {
                sigma0_hh_aft[l2bidx] = sum_s0[1][1]/(float)cnts[1][1];
                sigma0_hh_aft_std[l2bidx] = sqrt(
                    sum2_s0[1][1]/(float)cnts[1][1]-
                    pow(sigma0_hh_aft[l2bidx], 2));
            }

            if(cnts[0][2]) {
                sigma0_vh_fore[l2bidx] = sum_s0[0][2]/(float)cnts[0][2];
                sigma0_vh_fore_std[l2bidx] = sqrt(
                    sum2_s0[0][2]/(float)cnts[0][2]-
                    pow(sigma0_vh_fore[l2bidx], 2));
            }

            if(cnts[1][2]) {
                sigma0_vh_aft[l2bidx] = sum_s0[1][2]/(float)cnts[1][2];
                sigma0_vh_aft_std[l2bidx] = sqrt(
                    sum2_s0[1][2]/(float)cnts[1][2]-
                    pow(sigma0_vh_aft[l2bidx], 2));
            }

            if(cnts[0][3]) {
                sigma0_hv_fore[l2bidx] = sum_s0[0][3]/(float)cnts[0][3];
                sigma0_hv_fore_std[l2bidx] = sqrt(
                    sum2_s0[0][3]/(float)cnts[0][3]-
                    pow(sigma0_hv_fore[l2bidx], 2));
            }

            if(cnts[1][3]) {
                sigma0_hv_aft[l2bidx] = sum_s0[1][3]/(float)cnts[1][3];
                sigma0_hv_aft_std[l2bidx] = sqrt(
                    sum2_s0[1][3]/(float)cnts[1][3]-
                    pow(sigma0_hv_aft[l2bidx], 2));
            }


            // Copy to the output arrays
            LonLat lonlat = s0_ml->AverageLonLat();

            lon[l2bidx] = rtd * lonlat.longitude;
            lat[l2bidx] = rtd * lonlat.latitude;

            // wrap to [-180, 180) interval
            while(lon[l2bidx]>=180) lon[l2bidx] -= 360;
            while(lon[l2bidx]<-180) lon[l2bidx] += 360;

            anc_spd[l2bidx] = wvc->nudgeWV->spd;
            anc_dir[l2bidx] = pe_rad_to_gs_deg(wvc->nudgeWV->dir);
            if(anc_dir[l2bidx]>180) anc_dir[l2bidx]-=360;

            anc_sss[l2bidx] = l2b_tbonly.anc_sss[l2bidx];
            anc_sst[l2bidx] = l2b_tbonly.anc_sst[l2bidx];
            anc_swh[l2bidx] = l2b_tbonly.anc_swh[l2bidx];

            smap_cap_sss[l2bidx] = wvc->selected->sss;
            smap_cap_spd[l2bidx] = wvc->selected->spd;
            smap_cap_dir[l2bidx] = pe_rad_to_gs_deg(wvc->selected->dir);
            if(smap_cap_dir[l2bidx]>180) smap_cap_dir[l2bidx]-= 360;

            smap_radar_spd[l2bidx] = s0_wvc->selected->spd;
            smap_radar_dir[l2bidx] = pe_rad_to_gs_deg(s0_wvc->selected->dir);
            if(smap_radar_dir[l2bidx]>180) smap_radar_dir[l2bidx]-= 360;

            num_ambiguities[l2bidx] = wvc->ambiguities.NodeCount();

            // set ambiguity_spd, ambiguity_dir arrays
            int j_amb = 0;
            for(CAPWindVectorPlus* wvp = wvc->ambiguities.GetHead(); wvp;
                wvp = wvc->ambiguities.GetNext()){

                int ambidx = j_amb + 4*l2bidx;
                smap_cap_ambiguity_spd[ambidx] = wvp->spd;
                smap_cap_ambiguity_dir[ambidx] = pe_rad_to_gs_deg(wvp->spd);
                if(smap_cap_ambiguity_dir[ambidx] > 180)
                    smap_cap_ambiguity_dir[ambidx] -= 360;

                ++j_amb;
            }

            // Quality flagging TBD
            quality_flag[l2bidx] = 0;

        }
    }

    // Write out HDF5 file
    hid_t file_id = H5Fcreate(
        l2b_cap_file, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

    H5LTset_attribute_string(file_id, "/", "REVNO", config_list.Get("REVNO"));
    H5LTset_attribute_int(file_id, "/", "REV_START_YEAR", &start_year, 1);
    H5LTset_attribute_int(file_id, "/", "REV_START_DAY_OF_YEAR", &start_doy, 1);
    H5LTset_attribute_int(file_id, "/", "Number of Cross Track Bins", &ncti, 1);
    H5LTset_attribute_int(file_id, "/", "Number of Cross Track Bins", &ncti, 1);
    H5LTset_attribute_int(file_id, "/", "Number of Along Track Bins", &nati, 1);

    H5LTset_attribute_string(
        file_id, "/", "TB_CRID", config_list.Get("TB_CRID"));

    H5LTset_attribute_string(
        file_id, "/", "S0_CRID", config_list.Get("S0_CRID"));

    H5LTset_attribute_string(
        file_id, "/", "L1B_TB_LORES_ASC_FILE",
        config_list.Get("L1B_TB_LORES_ASC_FILE"));

    H5LTset_attribute_string(
        file_id, "/", "L1B_TB_LORES_DEC_FILE",
        config_list.Get("L1B_TB_LORES_DEC_FILE"));

    H5LTset_attribute_string(
        file_id, "/", "L1B_S0_LORES_ASC_FILE",
        config_list.Get("L1B_S0_LORES_ASC_FILE"));

    H5LTset_attribute_string(
        file_id, "/", "L1B_S0_LORES_DEC_FILE",
        config_list.Get("L1B_S0_LORES_DEC_FILE"));

    H5LTset_attribute_string(
        file_id, "/", "REV_START_TIME", config_list.Get("REV_START_TIME"));

    H5LTset_attribute_string(
        file_id, "/", "REV_STOP_TIME", config_list.Get("REV_STOP_TIME"));

    float _fill_value = FILL_VALUE;
    uint16 _ushort_fill_value = 65535;

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


    unsigned char uchar_fill_value = 0;
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
        file_id, "smap_cap_dir", 2, dims, H5T_NATIVE_FLOAT, &smap_cap_dir[0]);
    H5LTset_attribute_string(
        file_id, "smap_cap_dir", "long_name",
        "SMAP Combined Active / Passive wind direction");
    H5LTset_attribute_string(
        file_id, "smap_cap_dir", "units", "Degrees");
    H5LTset_attribute_float(
        file_id, "smap_cap_dir", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(
        file_id, "smap_cap_dir", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "smap_cap_dir", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "smap_radar_dir", 2, dims, H5T_NATIVE_FLOAT,
        &smap_radar_dir[0]);
    H5LTset_attribute_string(
        file_id, "smap_radar_dir", "long_name",
        "SMAP Radar-only wind direction");
    H5LTset_attribute_string(
        file_id, "smap_radar_dir", "units", "Degrees");
    H5LTset_attribute_float(
        file_id, "smap_radar_dir", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(
        file_id, "smap_radar_dir", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "smap_radar_dir", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "smap_cap_ambiguity_dir", 3, dims_amb, H5T_NATIVE_FLOAT,
        &smap_cap_ambiguity_dir[0]);
    H5LTset_attribute_string(
        file_id, "smap_cap_ambiguity_dir", "long_name",
        "SMAP Combined Active / Passive ambiguity wind direction");
    H5LTset_attribute_string(file_id, "smap_cap_ambiguity_dir", "units", "Degrees");
    H5LTset_attribute_float(
        file_id, "smap_cap_ambiguity_dir", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(
        file_id, "smap_cap_ambiguity_dir", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "smap_cap_ambiguity_dir", "valid_min", &valid_min, 1);

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

    valid_max = 40; valid_min = 0;
    H5LTmake_dataset(file_id, "anc_sss", 2, dims, H5T_NATIVE_FLOAT, &anc_sss[0]);
    H5LTset_attribute_string(
        file_id, "anc_sss", "long_name", "HYCOM salinity");
    H5LTset_attribute_string(file_id, "anc_sss", "units", "PSU");
    H5LTset_attribute_float(file_id, "anc_sss", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(file_id, "anc_sss", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "anc_sss", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "smap_cap_sss", 2, dims, H5T_NATIVE_FLOAT, &smap_cap_sss[0]);
    H5LTset_attribute_string(
        file_id, "smap_cap_sss", "long_name",
        "SMAP combined active / passive sea surface salinity");
    H5LTset_attribute_string(file_id, "smap_cap_sss", "units", "PSU");
    H5LTset_attribute_float(file_id, "smap_cap_sss", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(file_id, "smap_cap_sss", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "smap_cap_sss", "valid_min", &valid_min, 1);

    valid_max = 100; valid_min = 0;
    H5LTmake_dataset(
        file_id, "smap_cap_spd", 2, dims, H5T_NATIVE_FLOAT, &smap_cap_spd[0]);
    H5LTset_attribute_string(
        file_id, "smap_cap_spd", "long_name",
        "SMAP combined active / passive wind speed");
    H5LTset_attribute_string(file_id, "smap_cap_spd", "units", "Meters/second");
    H5LTset_attribute_float(file_id, "smap_cap_spd", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(file_id, "smap_cap_spd", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "smap_cap_spd", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "smap_radar_spd", 2, dims, H5T_NATIVE_FLOAT,
        &smap_radar_spd[0]);
    H5LTset_attribute_string(
        file_id, "smap_radar_spd", "long_name",
        "SMAP radar-only wind speed");
    H5LTset_attribute_string(file_id, "smap_radar_spd", "units", "Meters/second");
    H5LTset_attribute_float(file_id, "smap_radar_spd", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(file_id, "smap_radar_spd", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "smap_radar_spd", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "smap_cap_ambiguity_spd", 3, dims_amb, H5T_NATIVE_FLOAT,
        &smap_cap_ambiguity_spd[0]);
    H5LTset_attribute_string(
        file_id, "smap_cap_ambiguity_spd", "long_name",
        "SMAP combined active / passive ambiguity wind speed");
    H5LTset_attribute_string(
        file_id, "smap_cap_ambiguity_spd", "units", "Meters/second");
    H5LTset_attribute_float(
        file_id, "smap_cap_ambiguity_spd", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(
        file_id, "smap_cap_ambiguity_spd", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "smap_cap_ambiguity_spd", "valid_min", &valid_min, 1);


    valid_max = 10; valid_min = -0.01;
    H5LTmake_dataset(
        file_id, "sigma0_hh_fore", 2, dims, H5T_NATIVE_FLOAT,
        &sigma0_hh_fore[0]);
    H5LTset_attribute_string(
        file_id, "sigma0_hh_fore", "long_name",
        "SMAP HH fore look normalized radar cross-section");
    H5LTset_attribute_float(file_id, "sigma0_hh_fore", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(file_id, "sigma0_hh_fore", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "sigma0_hh_fore", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "sigma0_hh_aft", 2, dims, H5T_NATIVE_FLOAT,
        &sigma0_hh_aft[0]);
    H5LTset_attribute_string(
        file_id, "sigma0_hh_aft", "long_name",
        "SMAP HH aft look normalized radar cross-section");
    H5LTset_attribute_float(file_id, "sigma0_hh_aft", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(file_id, "sigma0_hh_aft", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "sigma0_hh_aft", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "sigma0_vv_fore", 2, dims, H5T_NATIVE_FLOAT,
        &sigma0_vv_fore[0]);
    H5LTset_attribute_string(
        file_id, "sigma0_vv_fore", "long_name",
        "SMAP VV fore look normalized radar cross-section");
    H5LTset_attribute_float(file_id, "sigma0_vv_fore", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(file_id, "sigma0_vv_fore", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "sigma0_vv_fore", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "sigma0_vv_aft", 2, dims, H5T_NATIVE_FLOAT,
        &sigma0_vv_aft[0]);
    H5LTset_attribute_string(
        file_id, "sigma0_vv_aft", "long_name",
        "SMAP VV aft look normalized radar cross-section");
    H5LTset_attribute_float(file_id, "sigma0_vv_aft", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(file_id, "sigma0_vv_aft", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "sigma0_vv_aft", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "sigma0_hv_fore", 2, dims, H5T_NATIVE_FLOAT,
        &sigma0_hv_fore[0]);
    H5LTset_attribute_string(
        file_id, "sigma0_hv_fore", "long_name",
        "SMAP HV fore look normalized radar cross-section");
    H5LTset_attribute_float(file_id, "sigma0_hv_fore", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(file_id, "sigma0_hv_fore", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "sigma0_hv_fore", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "sigma0_hv_aft", 2, dims, H5T_NATIVE_FLOAT,
        &sigma0_hv_aft[0]);
    H5LTset_attribute_string(
        file_id, "sigma0_hv_aft", "long_name",
        "SMAP HV aft look normalized radar cross-section");
    H5LTset_attribute_float(file_id, "sigma0_hv_aft", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(file_id, "sigma0_hv_aft", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "sigma0_hv_aft", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "sigma0_vh_fore", 2, dims, H5T_NATIVE_FLOAT,
        &sigma0_vh_fore[0]);
    H5LTset_attribute_string(
        file_id, "sigma0_vh_fore", "long_name",
        "SMAP VH fore look normalized radar cross-section");
    H5LTset_attribute_float(file_id, "sigma0_vh_fore", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(file_id, "sigma0_vh_fore", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "sigma0_vh_fore", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "sigma0_vh_aft", 2, dims, H5T_NATIVE_FLOAT,
        &sigma0_vh_aft[0]);
    H5LTset_attribute_string(
        file_id, "sigma0_vh_aft", "long_name",
        "SMAP VH aft look normalized radar cross-section");
    H5LTset_attribute_float(file_id, "sigma0_vh_aft", "_FillValue", &_fill_value, 1);
    H5LTset_attribute_float(file_id, "sigma0_vh_aft", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "sigma0_vh_aft", "valid_min", &valid_min, 1);

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
    H5LTset_attribute_ushort(
        file_id, "quality_flag", "_FillValue", &_ushort_fill_value, 1);

    H5LTmake_dataset(
        file_id, "sigma0_hh_fore_std", 2, dims, H5T_NATIVE_FLOAT,
        &sigma0_hh_fore_std[0]);

    H5LTmake_dataset(
        file_id, "sigma0_hh_aft_std", 2, dims, H5T_NATIVE_FLOAT,
        &sigma0_hh_aft_std[0]);

    H5LTmake_dataset(
        file_id, "sigma0_vv_fore_std", 2, dims, H5T_NATIVE_FLOAT,
        &sigma0_vv_fore_std[0]);

    H5LTmake_dataset(
        file_id, "sigma0_vv_aft_std", 2, dims, H5T_NATIVE_FLOAT,
        &sigma0_vv_aft_std[0]);

    H5LTmake_dataset(
        file_id, "sigma0_hv_fore_std", 2, dims, H5T_NATIVE_FLOAT,
        &sigma0_hv_fore_std[0]);

    H5LTmake_dataset(
        file_id, "sigma0_hv_aft_std", 2, dims, H5T_NATIVE_FLOAT,
        &sigma0_hv_aft_std[0]);

    H5LTmake_dataset(
        file_id, "sigma0_vh_fore_std", 2, dims, H5T_NATIVE_FLOAT,
        &sigma0_vh_fore_std[0]);

    H5LTmake_dataset(
        file_id, "sigma0_vh_aft_std", 2, dims, H5T_NATIVE_FLOAT,
        &sigma0_vh_aft_std[0]);




    H5Fclose(file_id);
    // free the arrays
    free_array((void *)l2a_s0_swath, 2, ncti, nati);

    return(0);
}
