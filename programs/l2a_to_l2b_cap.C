//==============================================================//
// Copyright (C) 2015, California Institute of Technology.      //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#define L2A_TB_FILE_KEYWORD "L2A_TB_FILE"
#define L2B_CAP_FILE_KEYWORD "L2B_CAP_FILE"
#define TB_FLAT_MODEL_FILE_KEYWORD "TB_FLAT_MODEL_FILE"
#define TB_ROUGH_MODEL_FILE_KEYWORD "TB_ROUGH_MODEL_FILE"
#define S0_ROUGH_MODEL_FILE_KEYWORD "S0_ROUGH_MODEL_FILE"
#define ANC_SSS_FILE_KEYWORD "ANC_SSS_FILE"
#define ANC_SST_FILE_KEYWORD "ANC_SST_FILE"
#define ANC_SWH_FILE_KEYWORD "ANC_SWH_FILE"
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

    // Get filenames from config file
    config_list.ExitForMissingKeywords();
    char* l2a_tb_file = config_list.Get(L2A_TB_FILE_KEYWORD);
    char* l2a_s0_file = config_list.Get(L2A_FILE_KEYWORD);
    char* l2b_cap_file = config_list.Get(L2B_CAP_FILE_KEYWORD);
    char* l2b_s0_file = config_list.Get(L2B_FILE_KEYWORD);
    char* tb_flat_file = config_list.Get(TB_FLAT_MODEL_FILE_KEYWORD);
    char* tb_rough_file = config_list.Get(TB_ROUGH_MODEL_FILE_KEYWORD);
    char* s0_rough_file = config_list.Get(S0_ROUGH_MODEL_FILE_KEYWORD);
    char* anc_sss_file = config_list.Get(ANC_SSS_FILE_KEYWORD);
    char* anc_sst_file = config_list.Get(ANC_SST_FILE_KEYWORD);
    char* anc_swh_file = config_list.Get(ANC_SWH_FILE_KEYWORD);

    // Configure the model functions
    CAPGMF cap_gmf;
    cap_gmf.ReadFlat(tb_flat_file);
    cap_gmf.ReadRough(tb_rough_file);
    cap_gmf.ReadModelS0(s0_rough_file);

    // Config the Kp object
    Kp kp;
    ConfigKp(&kp, &config_list);

    L2A l2a_s0, l2a_tb;
    l2a_s0.SetInputFilename(l2a_s0_file);
    l2a_tb.SetInputFilename(l2a_tb_file);

    L2B l2b_s0;
    l2b_s0.SetInputFilename(l2b_s0_file);

    // Open the various L2A and L2B files for input
    l2a_s0.OpenForReading();
    l2a_tb.OpenForReading();
    l2b_s0.OpenForReading();

    // Read the headers and compare swath sizes
    l2a_s0.ReadHeader();
    l2a_tb.ReadHeader();
    l2b_s0.ReadHeader();

    // Read in L2B radar-only file
    l2b_s0.ReadDataRec();
    l2b_s0.Close();

    int ncti = l2b_s0.frame.swath.GetCrossTrackBins();
    int nati = l2b_s0.frame.swath.GetAlongTrackBins();
    if(l2a_s0.header.alongTrackBins != nati ||
       l2a_tb.header.alongTrackBins != nati ||
       l2a_s0.header.crossTrackBins != ncti ||
       l2a_tb.header.crossTrackBins != ncti) {
        fprintf(stderr, "Size mismatch!\n");
        exit(1);
    }

    // Read in L2A TB file
    L2AFrame*** l2a_tb_swath;
    l2a_tb_swath = (L2AFrame***)make_array(sizeof(L2AFrame *), 2, ncti, nati);
    while(l2a_tb.ReadDataRec()) {
        L2AFrame* this_frame = new L2AFrame();
        this_frame->CopyFrame(this_frame, &l2a_tb.frame);
         *(*(l2a_tb_swath + this_frame->cti) + this_frame->ati) = this_frame;
    }
    l2a_tb.Close();

    // Read in L2A S0 file
    L2AFrame*** l2a_s0_swath;
    l2a_s0_swath = (L2AFrame***)make_array(sizeof(L2AFrame *), 2, ncti, nati);
    while(l2a_s0.ReadDataRec()) {
        L2AFrame* this_frame = new L2AFrame();
        this_frame->CopyFrame(this_frame, &l2a_s0.frame);
         *(*(l2a_s0_swath + this_frame->cti) + this_frame->ati) = this_frame;
    }
    l2a_s0.Close();

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

    CAP_ANC_L2B anc_sss(anc_sss_file);
    CAP_ANC_L2B anc_sst(anc_sst_file);
    CAP_ANC_L2B anc_swh(anc_swh_file);

    for(int ati=0; ati<nati; ++ati) {
        if(ati%100 == 0)
            fprintf(stdout, "%d of %d\n", ati, nati);

        for(int cti=0; cti<ncti; ++cti) {

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
            if(!s0_wvc || !l2a_tb_swath[cti][ati] || !l2a_s0_swath[cti][ati])
                continue;

            MeasList* tb_ml = &(l2a_tb_swath[cti][ati]->measList);
            MeasList* s0_ml = &(l2a_s0_swath[cti][ati]->measList);

            float init_spd = s0_wvc->selected->spd;
            float init_dir = s0_wvc->selected->dir;
            float init_sss = anc_sss.data[ati][cti][0];
            float this_anc_sst = anc_sst.data[ati][cti][0];
            float this_anc_swh = anc_swh.data[ati][cti][0];
            float this_anc_rr = -9999;

            if(init_sss<0 || this_anc_sst<-10)
                continue;

            float this_cap_spd, this_cap_dir, this_cap_sss, min_obj;

            float active_weight = 1;
            float passive_weight = 1;

            if(this_anc_swh<0 || this_anc_swh > 20)
                this_anc_swh = -9999;

            this_anc_swh = -9999;

            cap_gmf.Retrieve(
                tb_ml, s0_ml, init_spd, init_dir, init_sss, this_anc_sst,
                this_anc_swh, this_anc_rr, active_weight, passive_weight,
                CAPGMF::RETRIEVE_SPEED_ONLY, &this_cap_spd, &this_cap_dir,
                &this_cap_sss, &min_obj);

            // switch back to clockwise from noth convention, to degrees, and wrap
            float radar_only_dir = 450.0 - rtd * s0_wvc->selected->dir;
            while(radar_only_dir>=180) radar_only_dir-=360;
            while(radar_only_dir<-180) radar_only_dir+=360;

            this_cap_dir = 450 - rtd * this_cap_dir;
            while(this_cap_dir>=180) this_cap_dir-=360;
            while(this_cap_dir<-180) this_cap_dir+=360;

            float this_lon = s0_wvc->lonLat.longitude*rtd;
            while(this_lon>=180) this_lon-=360;
            while(this_lon<-180) this_lon+=360;

            lat[cti][ati] = s0_wvc->lonLat.latitude*rtd;
            lon[cti][ati] = this_lon;
            s0_spd[cti][ati] = s0_wvc->selected->spd;
            s0_dir[cti][ati] = radar_only_dir;
            s0_flg[cti][ati] = s0_wvc->qualFlag;

//             printf("%d %d %f %f %f %f\n", cti, ati, s0_wvc->selected->spd, 
//                 radar_only_dir, this_cap_spd, this_cap_dir);

            // insert some QA here to set cap_flag???
            cap_spd[cti][ati] = this_cap_spd;
            cap_dir[cti][ati] = this_cap_dir;
            cap_sss[cti][ati] = this_cap_sss;
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
    free_array((void *)l2a_tb_swath, 2, ncti, nati);
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
