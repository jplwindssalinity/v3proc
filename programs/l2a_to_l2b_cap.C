//==============================================================//
// Copyright (C) 2015, California Institute of Technology.      //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#define L2A_TB_FILE_KEYWORD "L2A_TB_FILE"
#define L2B_CAP_FILE_KEYWORD "L2B_CAP_FILE"
#define TB_FLAT_MODEL_FILE_KEYWORD "TB_FLAT_MODEL_FILE"
#define TB_ROUGH_MODEL_FILE_KEYWORD "TB_ROUGH_MODEL_FILE"
#define FILL_VALUE -9999

//----------//
// INCLUDES //
//----------//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <nlopt.hpp>
#include "List.h"
#include "BufferedList.h"
#include "Misc.h"
#include "ConfigList.h"
#include "L2A.h"
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

// For use with NLopt
typedef struct {
    MeasList* tb_ml;
    MeasList* s0_ml;
    WVC* s0_wvc;
    GMF* gmf;
    CAPGMF* cap_gmf;
    double anc_sst;
    double anc_swh;
    double anc_rr;
} CAPAncillary;

// For use with NLopt
double cap_obj_func(unsigned n, const double* x, double* grad, void* data) {

    // optimization variables
    float trial_spd = (float)x[0];
    float trial_dir = (float)x[1];
    float trial_sss = (float)x[2];

    CAPAncillary* cap_anc = (CAPAncillary*)data;

    double obj = 0;
    // Loop over TB observations
    for(Meas* meas = cap_anc->tb_ml->GetHead(); meas;
        meas = cap_anc->tb_ml->GetNext()){

        float model_tb;
        float chi = trial_dir - meas->eastAzimuth + pi;

        cap_anc->cap_gmf->GetTB(
            meas->measType, meas->incidenceAngle, cap_anc->anc_sst, trial_sss,
            trial_spd, chi, &model_tb);

        double var = pow(meas->A-1.0, 2) * model_tb * model_tb;
        obj += pow(meas->value - model_tb, 2) / var;
    }

    // Loop over s0 observations
    for(Meas* meas = cap_anc->s0_ml->GetHead(); meas;
        meas = cap_anc->s0_ml->GetNext()){

        // Compute model S0 (replace this stub!!!)
        float model_s0;
        float chi = trial_dir - meas->eastAzimuth + pi;

        cap_anc->gmf->GetInterpolatedValue(
            meas->measType, meas->incidenceAngle, trial_spd, chi, &model_s0);

        double var = pow(meas->A-1.0, 2) * model_s0 * model_s0;
        obj += pow(meas->value - model_s0, 2) / var;
    }

    return(obj);
}

int retrieve_cap(MeasList* tb_ml, MeasList* s0_ml, WVC* s0_wvc, double anc_sst,
                 double anc_swh, double anc_rr, double anc_sss, double* spd,
                 double* dir, double* sss, double* min_obj) {

    // Construct the optimization object
    nlopt::opt opt(nlopt::LN_COBYLA, 3);

    // Set contraints
    std::vector<double> lb(3), ub(3);

    lb[0] = 0; ub[0] = 100; // wind speed
    lb[1] = 0; ub[1] = two_pi; // wind direction
    lb[2] = 10; ub[2] = 40; // salinity

    // Set the various ancillary stuff needed
    CAPAncillary cap_anc;
    cap_anc.tb_ml = tb_ml;
    cap_anc.s0_ml = s0_ml;
    cap_anc.s0_wvc = s0_wvc;
    cap_anc.anc_sst = anc_sst;
    cap_anc.anc_swh = anc_swh;
    cap_anc.anc_rr = anc_rr;

    // Config the optimization object for this problem
    opt.set_lower_bounds(lb);
    opt.set_upper_bounds(ub);
    opt.set_min_objective(cap_obj_func, &cap_anc);

    std::vector<double> x(3);

    // init guess at radar-only DIRTH wind vector and ancillary SSS
    x[0] = s0_wvc->specialVector->spd;
    x[1] = s0_wvc->specialVector->dir;
    x[2] = anc_sss;

    // Solve it!
    double minf;
    nlopt::result result = opt.optimize(x, minf);

    // Copy outputs
    *spd = x[0];
    *dir = x[1];
    *sss = x[2];
    *min_obj = minf;

    return(1);
}

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

    // Configure the model functions
    GMF gmf;
    ConfigGMF(&gmf, &config_list);

    CAPGMF cap_gmf;
    cap_gmf.ReadFlat(tb_flat_file);
    cap_gmf.ReadRough(tb_rough_file);

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

    // Read in L2B radar-only file
    l2b_s0.ReadDataRec();
    l2b_s0.Close();

    float cap_spd[ncti][nati];
    float cap_dir[ncti][nati];
    float cap_sss[ncti][nati];

    // Placeholders!!!
    double anc_sst, anc_swh, anc_rr, anc_sss;


    for(int ati=0; ati<nati; ++ati) {
        for(int cti=0; cti<ncti; ++cti) {

            // init with fill value
            cap_spd[cti][ati] = FILL_VALUE;
            cap_dir[cti][ati] = FILL_VALUE;
            cap_sss[cti][ati] = FILL_VALUE;

            WVC* s0_wvc = l2b_s0.frame.swath.GetWVC(cti, ati);
            if(!s0_wvc || !l2a_tb_swath[cti][ati] || !l2a_s0_swath[cti][ati])
                continue;

            MeasList* tb_ml = &(l2a_tb_swath[cti][ati]->measList);
            MeasList* s0_ml = &(l2a_s0_swath[cti][ati]->measList);

            double this_cap_spd, this_cap_dir, this_cap_sss, min_obj;

            retrieve_cap(
                tb_ml, s0_ml, s0_wvc, anc_sst, anc_swh, anc_rr, anc_sss,
                &this_cap_spd, &this_cap_dir, &this_cap_sss, &min_obj);

            // insert some QA here??
            cap_spd[cti][ati] = this_cap_spd;
            cap_dir[cti][ati] = this_cap_dir;
            cap_sss[cti][ati] = this_cap_sss;
        }
    }

    // Write out the results?...

    // free the arrays
    free_array((void *)l2a_tb_swath, 2, ncti, nati);
    free_array((void *)l2a_s0_swath, 2, ncti, nati);

    return(0);
}




