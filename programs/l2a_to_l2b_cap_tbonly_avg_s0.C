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

            float sum_s0[2][2]; // [fore-0, aft-1][v-0, h-1]
            float sum_A[2][2];
            int cnts[2][2];

            float sum_cos_azi[2]; // fore - 0; aft 1
            float sum_sin_azi[2];
            float sum_inc[2];

            float inc_fore, inc_aft;
            float azi_fore, azi_aft;

            for(int ilook = 0; ilook < 2; ++ilook) {
                sum_inc[ilook] = 0;
                sum_sin_azi[ilook] = 0;
                sum_cos_azi[ilook] = 0;
                for(int ipol = 0; ipol < 2; ++ipol) {
                    sum_s0[ilook][ipol] = 0;
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
                } else {
                    continue;
                }

                cnts[idx_look][idx_pol]++;
                sum_s0[idx_look][idx_pol] += meas->value;
                sum_A[idx_look][idx_pol] += meas->A;
                sum_inc[idx_look] += meas->incidenceAngle;
                sum_cos_azi[idx_look] += cos(meas->eastAzimuth);
                sum_sin_azi[idx_look] += sin(meas->eastAzimuth);
            }

            // Compute averaged s0 observations from four looks
            // Angles computed in degrees and clockwise from north
            int cnts_fore = cnts[0][0] + cnts[0][1];
            if(cnts_fore>0) {
                inc_fore = rtd*sum_inc[0]/(float)cnts_fore;
                azi_fore = rtd*atan2(sum_cos_azi[0], sum_sin_azi[0]);
            }

            int cnts_aft = cnts[1][0] + cnts[1][1];
            if(cnts_aft>0) {
                inc_aft = rtd*sum_inc[1]/(float)cnts_aft;
                azi_aft = rtd*atan2(sum_cos_azi[1], sum_sin_azi[1]);
            }

            MeasList s0_ml_avg;
            if(cnts[0][0]) {
                Meas* this_meas = new Meas();
                this_meas->value = sum_s0[0][0]/(float)cnts[0][0];
                this_meas->measType = Meas::VV_MEAS_TYPE;
                this_meas->incidenceAngle = dtr * inc_fore;
                this_meas->eastAzimuth = gs_deg_to_pe_rad(azi_fore);
                this_meas->A = sum_A[0][0]/pow((float)cnts[0][0], 2);
                s0_ml_avg.Append(this_meas);
            }

            if(cnts[1][0]) {
                Meas* this_meas = new Meas();
                this_meas->value = sum_s0[1][0]/(float)cnts[1][0];
                this_meas->measType = Meas::VV_MEAS_TYPE;
                this_meas->incidenceAngle = dtr * inc_fore;
                this_meas->eastAzimuth = gs_deg_to_pe_rad(azi_fore);
                this_meas->A = sum_A[1][0]/pow((float)cnts[1][0], 2);
                s0_ml_avg.Append(this_meas);
            }

            if(cnts[0][1]) {
                Meas* this_meas = new Meas();
                this_meas->value = sum_s0[0][1]/(float)cnts[0][1];
                this_meas->measType = Meas::HH_MEAS_TYPE;
                this_meas->incidenceAngle = dtr * inc_fore;
                this_meas->eastAzimuth = gs_deg_to_pe_rad(azi_fore);
                this_meas->A = sum_A[0][1]/pow((float)cnts[0][1], 2);
                s0_ml_avg.Append(this_meas);
            }

            if(cnts[1][1]) {
                Meas* this_meas = new Meas();
                this_meas->value = sum_s0[1][1]/(float)cnts[1][1];
                this_meas->measType = Meas::HH_MEAS_TYPE;
                this_meas->incidenceAngle = dtr * inc_fore;
                this_meas->eastAzimuth = gs_deg_to_pe_rad(azi_fore);
                this_meas->A = sum_A[1][1]/pow((float)cnts[1][1], 2);
                s0_ml_avg.Append(this_meas);
            }


            MeasList tb_ml;
            // Build up tb_ml from L2B_TB_FILE
            if(l2b_tbonly.n_v_fore[l2bidx] > 0) {
                Meas* this_meas = new Meas();
                this_meas->value = l2b_tbonly.tb_v_fore[l2bidx];
                this_meas->measType = Meas::L_BAND_TBV_MEAS_TYPE;
                this_meas->incidenceAngle = dtr * l2b_tbonly.inc_fore[l2bidx];
                this_meas->eastAzimuth = gs_deg_to_pe_rad(
                    l2b_tbonly.azi_fore[l2bidx]);
                this_meas->A = pow(l2b_tbonly.nedt_v_fore[l2bidx], 2);
                tb_ml.Append(this_meas);
            }

            if(l2b_tbonly.n_h_fore[l2bidx] > 0) {
                Meas* this_meas = new Meas();
                this_meas->value = l2b_tbonly.tb_h_fore[l2bidx];
                this_meas->measType = Meas::L_BAND_TBH_MEAS_TYPE;
                this_meas->incidenceAngle = dtr * l2b_tbonly.inc_fore[l2bidx];
                this_meas->eastAzimuth = gs_deg_to_pe_rad(
                    l2b_tbonly.azi_fore[l2bidx]);
                this_meas->A = pow(l2b_tbonly.nedt_h_fore[l2bidx], 2);
                tb_ml.Append(this_meas);
            }

            if(l2b_tbonly.n_v_aft[l2bidx] > 0) {
                Meas* this_meas = new Meas();
                this_meas->value = l2b_tbonly.tb_v_aft[l2bidx];
                this_meas->measType = Meas::L_BAND_TBV_MEAS_TYPE;
                this_meas->incidenceAngle = dtr * l2b_tbonly.inc_aft[l2bidx];
                this_meas->eastAzimuth = gs_deg_to_pe_rad(
                    l2b_tbonly.azi_aft[l2bidx]);
                this_meas->A = pow(l2b_tbonly.nedt_v_aft[l2bidx], 2);
                tb_ml.Append(this_meas);
            }

            if(l2b_tbonly.n_h_aft[l2bidx] > 0) {
                Meas* this_meas = new Meas();
                this_meas->value = l2b_tbonly.tb_h_aft[l2bidx];
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
                &tb_ml, &s0_ml_avg, init_spd, init_sss, this_anc_spd, this_anc_dir,
                this_anc_sst, this_anc_swh, this_anc_rr, anc_spd_std_prior,
                active_weight, passive_weight, wvc);

//             cap_gmf.CAPGMF::BuildSolutionCurves(
//                 &tb_ml, s0_ml, init_spd, init_sss, this_anc_spd, this_anc_dir,
//                 this_anc_sst, this_anc_swh, this_anc_rr, anc_spd_std_prior,
//                 active_weight, passive_weight, wvc);

            wvc->BuildSolutions();

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

    // Write outputs
    float** lat = (float**)make_array(sizeof(float), 2, ncti, nati);
    float** lon = (float**)make_array(sizeof(float), 2, ncti, nati);
    float** s0_spd = (float**)make_array(sizeof(float), 2, ncti, nati);
    float** s0_dir = (float**)make_array(sizeof(float), 2, ncti, nati);

    unsigned int** s0_flg = (unsigned int**)make_array(
        sizeof(unsigned int), 2, ncti, nati);

    float** cap_spd = (float**)make_array(sizeof(float), 2, ncti, nati);
    float** cap_dir = (float**)make_array(sizeof(float), 2, ncti, nati);
    float** cap_sss = (float**)make_array(sizeof(float), 2, ncti, nati);
    unsigned char** cap_flg = (unsigned char**)make_array(
        sizeof(unsigned char), 2, ncti, nati);

    for(int ati=0; ati<nati; ++ati) {
        for(int cti=0; cti<ncti; ++cti) {

            int l2bidx = ati + nati*cti;

            // init with fill value
            lat[cti][ati] = FILL_VALUE;
            lon[cti][ati] = FILL_VALUE;
            s0_spd[cti][ati] = FILL_VALUE;
            s0_dir[cti][ati] = FILL_VALUE;
            s0_flg[cti][ati] = 0;
            cap_spd[cti][ati] = FILL_VALUE;
            cap_dir[cti][ati] = FILL_VALUE;
            cap_sss[cti][ati] = FILL_VALUE;
            cap_flg[cti][ati] = 255;

            WVC* s0_wvc = l2b_s0.frame.swath.GetWVC(cti, ati);
            CAPWVC* wvc = cap_wind_swath.swath[cti][ati];

            if(!wvc || !s0_wvc)
                continue;

            // switch back to clockwise from noth convention, to degrees, and wrap
            float radar_only_dir = 450.0 - rtd * s0_wvc->selected->dir;
            while(radar_only_dir>=180) radar_only_dir-=360;
            while(radar_only_dir<-180) radar_only_dir+=360;

            float this_cap_dir = 450 - rtd * wvc->selected->dir;
            while(this_cap_dir>=180) this_cap_dir-=360;
            while(this_cap_dir<-180) this_cap_dir+=360;

            lat[cti][ati] = l2b_tbonly.lat[l2bidx];
            lon[cti][ati] = l2b_tbonly.lon[l2bidx];
            s0_spd[cti][ati] = s0_wvc->selected->spd;
            s0_dir[cti][ati] = radar_only_dir;
            s0_flg[cti][ati] = s0_wvc->qualFlag;

            // insert some QA here to set cap_flag???
            cap_spd[cti][ati] = wvc->selected->spd;
            cap_dir[cti][ati] = this_cap_dir;
            cap_sss[cti][ati] = wvc->selected->sss;
        }
    }

    // Write it out
    FILE* ofp = fopen(l2b_cap_file, "w");
    write_array(ofp, &lon[0], sizeof(float), 2, ncti, nati);
    write_array(ofp, &lat[0], sizeof(float), 2, ncti, nati);
    write_array(ofp, &s0_spd[0], sizeof(float), 2, ncti, nati);
    write_array(ofp, &s0_dir[0], sizeof(float), 2, ncti, nati);
    write_array(ofp, &s0_flg[0], sizeof(unsigned int), 2, ncti, nati);
    write_array(ofp, &cap_spd[0], sizeof(float), 2, ncti, nati);
    write_array(ofp, &cap_dir[0], sizeof(float), 2, ncti, nati);
    write_array(ofp, &cap_sss[0], sizeof(float), 2, ncti, nati);
    write_array(ofp, &cap_flg[0], sizeof(unsigned char), 2, ncti, nati);
    fclose(ofp);

    // free the arrays
    free_array((void *)l2a_s0_swath, 2, ncti, nati);

    free_array(lat, 2, ncti, nati);
    free_array(lon, 2, ncti, nati);
    free_array(s0_spd, 2, ncti, nati);
    free_array(s0_dir, 2, ncti, nati);
    free_array(s0_flg, 2, ncti, nati);
    free_array(cap_spd, 2, ncti, nati);
    free_array(cap_dir, 2, ncti, nati);
    free_array(cap_sss, 2, ncti, nati);
    free_array(cap_flg, 2, ncti, nati);

    return(0);
}
