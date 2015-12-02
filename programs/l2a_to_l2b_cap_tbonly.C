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

    int ncti = l2a_s0.header.crossTrackBins;
    int nati = l2a_s0.header.alongTrackBins;

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

    L2BTBOnly l2b_tbonly(l2b_tb_file);

    // Output arrays
    int l2b_size = ncti * nati;

    for(int ati=0; ati<nati; ++ati) {
        if(ati%50 == 0)
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

            if(tb_ml.NodeCount() == 0 || s0_ml->NodeCount() == 0)
                continue;

            CAPWVC* wvc = new CAPWVC();
            WindVectorPlus* nudgeWV = new WindVectorPlus();

            if(any_land)
                wvc->s0_flag |= L2B_QUAL_FLAG_LAND;

            if(any_ice)
                wvc->s0_flag |= L2B_QUAL_FLAG_ICE;

            // set nudge from L2B tbonly file
            nudgeWV->spd = l2b_tbonly.anc_spd[l2bidx];
            nudgeWV->dir = gs_deg_to_pe_rad(l2b_tbonly.anc_dir[l2bidx]);

            wvc->nudgeWV = nudgeWV;

            float active_weight = 1;
            float passive_weight = 1;
            float init_spd = l2b_tbonly.smap_spd[l2bidx];
            float init_dir = nudgeWV->dir;
            float init_sss = l2b_tbonly.smap_sss[l2bidx];
            float this_anc_spd = nudgeWV->spd;
            float this_anc_dir = nudgeWV->dir;
            float this_anc_sst = l2b_tbonly.anc_sst[l2bidx];
            float this_anc_swh = l2b_tbonly.anc_swh[l2bidx];
            float this_anc_rr = 0;
            float anc_spd_std_prior = 100;

            cap_gmf.CAPGMF::BuildSolutionCurvesTwoStep(
                &tb_ml, s0_ml, init_spd, init_sss, this_anc_spd, this_anc_dir,
                this_anc_sst, this_anc_swh, this_anc_rr, anc_spd_std_prior,
                active_weight, passive_weight, wvc);

            wvc->BuildSolutions();

            if(wvc->ambiguities.NodeCount() > 0) {
                cap_wind_swath.Add(cti, ati, wvc);
                wvc->selected = wvc->GetNearestAmbig(init_dir);
            } else {
                delete wvc;
            }
        }
    }

    cap_wind_swath.ThreshNudge(0.05);
    cap_wind_swath.MedianFilter(3, 200);

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

            CAPWVC* wvc = cap_wind_swath.swath[cti][ati];

            if(!wvc)
                continue;

            // switch back to clockwise from noth convention, to degrees, and wrap
            float this_cap_dir = 450 - rtd * wvc->selected->dir;
            while(this_cap_dir>=180) this_cap_dir-=360;
            while(this_cap_dir<-180) this_cap_dir+=360;

            lat[cti][ati] = l2b_tbonly.lat[l2bidx];
            lon[cti][ati] = l2b_tbonly.lon[l2bidx];

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
