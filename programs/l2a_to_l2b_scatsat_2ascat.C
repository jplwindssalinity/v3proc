//==============================================================//
// Copyright (C) 2017, California Institute of Technology.      //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//
#define ANCILLARY_SST2B_FILE_KEYWORD "ANCILLARY_SST2B_FILE"
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

    int optind = 2;
    int ascat_only = 0;
    int scatsat_only = 0;
    int drop_ascat[2] = {0, 0};
    float ku_s0_adj_lin = 1;
    while((optind < argc) && (argv[optind][0] == '-')) {
        std::string sw = argv[optind];
        if(sw == "--ascat-only") {
            ascat_only = 1;
            printf("ASCAT only\n");
        } else if(sw == "--drop-ascat-a") {
            drop_ascat[0] = 1;
            printf("Drop ASCAT-A\n");
        } else if(sw == "--drop-ascat-b") {
            drop_ascat[1] = 1;
            printf("Drop ASCAT-B\n");
        } else if(sw == "--scatsat-only") {
            scatsat_only = 1;
            printf("SCATSAT only\n");
        } else if(sw == "--ku-s0-adj") {
            ku_s0_adj_lin = pow(10.0, 0.1*atof(argv[++optind]));
            printf("Ku sigma0 adj %f\n", ku_s0_adj_lin);
        } else {
            fprintf(stderr,"%s: Unknown option: %s\n", command, sw.c_str());
            exit(1);
        }
        optind++;
    }

    ConfigList config_list;
    config_list.Read(config_file);
    config_list.ExitForMissingKeywords();

    GMF* gmf;

    config_list.DoNothingForMissingKeywords();
    SSTGMF sst_gmf(&config_list);
    char* sstgmf_basepath = config_list.Get("SST_GMF_BASEPATH");
    config_list.ExitForMissingKeywords();
    int do_sst_gmf = (sstgmf_basepath != NULL);
    std::vector<float> anc_sst;
    anc_sst.resize(152*3248);

    if (do_sst_gmf) {
        printf("Using SST GMF\n");
        char* anc_sst_filename = config_list.Get(ANCILLARY_SST2B_FILE_KEYWORD);
        FILE* anc_sst_ifp = fopen(anc_sst_filename, "r");
        fread(&anc_sst[0], sizeof(float), 152*3248, anc_sst_ifp);
        fclose(anc_sst_ifp);
    } else {
        printf("Not using SST GMF\n");
        gmf = new GMF();
        ConfigGMF(gmf, &config_list);
    }

    Kp kp;
    ConfigKp(&kp, &config_list);

    ETime etime;
    etime.FromCodeB("1970-001T00:00:00.000");
    double time_base = (double)etime.GetSec() + (double)etime.GetMs()/1000;

    etime.FromCodeB("2016-001T00:00:00.000");
    double out_time_base = (double)etime.GetSec() + (double)etime.GetMs()/1000;

    etime.FromCodeB(config_list.Get("REV_START_TIME"));
    double rev_start =
        (double)etime.GetSec() + (double)etime.GetMs()/1000 - time_base;

    etime.FromCodeB(config_list.Get("REV_STOP_TIME"));
    double rev_stop =
        (double)etime.GetSec() + (double)etime.GetMs()/1000 - time_base;

    char* l2b_file = config_list.Get("L2B_COMBINED_FILE");

    L2A l2a_ascat_a, l2a_ascat_b, l2a_scatsat;
    L2B l2b_scatsat, l2b_scatsat_nofilt;

    l2a_ascat_a.SetInputFilename(config_list.Get("L2A_ASCAT_A_FILE"));
    l2a_ascat_a.OpenForReading();
    l2a_ascat_a.ReadHeader();

    l2a_ascat_b.SetInputFilename(config_list.Get("L2A_ASCAT_B_FILE"));
    l2a_ascat_b.OpenForReading();
    l2a_ascat_b.ReadHeader();

    l2a_scatsat.SetInputFilename(config_list.Get("L2A_SCATSAT_FILE"));
    l2a_scatsat.OpenForReading();
    l2a_scatsat.ReadHeader();

    l2b_scatsat.SetInputFilename(config_list.Get("L2B_SCATSAT_FILE"));
    l2b_scatsat.OpenForReading();
    l2b_scatsat.ReadHeader();
    l2b_scatsat.ReadDataRec();
    l2b_scatsat.Close();

    l2b_scatsat_nofilt.SetInputFilename(
        config_list.Get("L2B_SCATSAT_NO_MEDFILT_FILE"));
    l2b_scatsat_nofilt.OpenForReading();
    l2b_scatsat_nofilt.ReadHeader();
    l2b_scatsat_nofilt.ReadDataRec();
    l2b_scatsat_nofilt.Close();

    int nati = l2a_ascat_a.header.alongTrackBins;
    int ncti = l2a_ascat_a.header.crossTrackBins;

    if (l2a_scatsat.header.alongTrackBins != nati || 
        l2a_scatsat.header.crossTrackBins != ncti || 
        l2a_ascat_b.header.alongTrackBins != nati || 
        l2a_ascat_b.header.crossTrackBins != ncti || 
        l2b_scatsat.frame.swath.GetAlongTrackBins() != nati ||
        l2b_scatsat.frame.swath.GetCrossTrackBins() != ncti ||
        l2b_scatsat_nofilt.frame.swath.GetAlongTrackBins() != nati ||
        l2b_scatsat_nofilt.frame.swath.GetCrossTrackBins() != ncti) {
        fprintf(stderr, "Size mismatch!\n");
        exit(1);
    }

    // Read in L2A ASCAT file
    std::vector<L2AFrame***> l2a_ascat_swaths(2);
    l2a_ascat_swaths[0] = (L2AFrame***)make_array(
        sizeof(L2AFrame *), 2, ncti, nati);
    while(l2a_ascat_a.ReadDataRec()) {
        L2AFrame* this_frame = new L2AFrame();
        this_frame->CopyFrame(this_frame, &l2a_ascat_a.frame);
         *(*(l2a_ascat_swaths[0] + this_frame->cti) + this_frame->ati) = this_frame;
    }
    l2a_ascat_a.Close();

    l2a_ascat_swaths[1] = (L2AFrame***)make_array(
        sizeof(L2AFrame *), 2, ncti, nati);
    while(l2a_ascat_b.ReadDataRec()) {
        L2AFrame* this_frame = new L2AFrame();
        this_frame->CopyFrame(this_frame, &l2a_ascat_b.frame);
         *(*(l2a_ascat_swaths[1] + this_frame->cti) + this_frame->ati) = this_frame;
    }
    l2a_ascat_b.Close();

    // Read in L2A SCATSAT file
    L2AFrame*** l2a_scatsat_swath;
    l2a_scatsat_swath = (L2AFrame***)make_array(sizeof(L2AFrame *), 2, ncti, nati);
    while(l2a_scatsat.ReadDataRec()) {
        L2AFrame* this_frame = new L2AFrame();
        this_frame->CopyFrame(this_frame, &l2a_scatsat.frame);
         *(*(l2a_scatsat_swath + this_frame->cti) + this_frame->ati) = this_frame;
    }
    l2a_scatsat.Close();

    // Output arrays
    int l2b_size = ncti * nati;

    std::vector<double> row_time(nati), out_row_time(nati);
    std::vector<float> lat(l2b_size), lon(l2b_size);
    std::vector<float> spd(l2b_size), dir(l2b_size);
    std::vector<float> ascat_only_spd(l2b_size), ascat_only_dir(l2b_size);
    std::vector<float> anc_spd(l2b_size), anc_dir(l2b_size), out_anc_sst(l2b_size);
    std::vector<float> ambiguity_obj(l2b_size*4);
    std::vector<float> ambiguity_spd(l2b_size*4);
    std::vector<float> ambiguity_dir(l2b_size*4);
    std::vector<uint8> num_ambiguities(l2b_size);
    std::vector<uint16> flags(l2b_size);
    std::vector<uint16> eflags(l2b_size);

    std::vector<float> ascat_only_ambiguity_spd(l2b_size*4);
    std::vector<float> ascat_only_ambiguity_dir(l2b_size*4);
    std::vector<uint8> ascat_only_num_ambiguities(l2b_size);

    std::vector<float> scatsat_only_spd(l2b_size), scatsat_only_dir(l2b_size);
    std::vector<float> scatsat_only_spd_uncorrected(l2b_size);
    std::vector<float> scatsat_rain_impact(l2b_size);
    std::vector<float> scatsat_only_ambiguity_spd(l2b_size*4);
    std::vector<float> scatsat_only_ambiguity_dir(l2b_size*4);
    std::vector<uint8> scatsat_only_num_ambiguities(l2b_size);
    std::vector<float> scatsat_sigma0_hh_fore(l2b_size);
    std::vector<float> scatsat_sigma0_hh_aft(l2b_size);
    std::vector<float> scatsat_sigma0_vv_fore(l2b_size);
    std::vector<float> scatsat_sigma0_vv_aft(l2b_size);
    std::vector<float> scatsat_var_sigma0_hh_fore(l2b_size);
    std::vector<float> scatsat_var_sigma0_hh_aft(l2b_size);
    std::vector<float> scatsat_var_sigma0_vv_fore(l2b_size);
    std::vector<float> scatsat_var_sigma0_vv_aft(l2b_size);
    std::vector<float> scatsat_inc_hh_fore(l2b_size);
    std::vector<float> scatsat_inc_hh_aft(l2b_size);
    std::vector<float> scatsat_inc_vv_fore(l2b_size);
    std::vector<float> scatsat_inc_vv_aft(l2b_size);
    std::vector<float> scatsat_azi_hh_fore(l2b_size);
    std::vector<float> scatsat_azi_hh_aft(l2b_size);
    std::vector<float> scatsat_azi_vv_fore(l2b_size);
    std::vector<float> scatsat_azi_vv_aft(l2b_size);
    std::vector<float> scatsat_tb_h(l2b_size);
    std::vector<float> scatsat_tb_v(l2b_size);

    std::vector<std::vector<float> > ascat_time_diff(2);
    std::vector<std::vector<float> > ascat_lat(2), ascat_lon(2);
    std::vector<std::vector<float> > ascat_sigma0_fore(2);
    std::vector<std::vector<float> > ascat_sigma0_mid(2);
    std::vector<std::vector<float> > ascat_sigma0_aft(2);
    std::vector<std::vector<float> > ascat_var_sigma0_fore(2);
    std::vector<std::vector<float> > ascat_var_sigma0_mid(2);
    std::vector<std::vector<float> > ascat_var_sigma0_aft(2);
    std::vector<std::vector<float> > ascat_inc_fore(2);
    std::vector<std::vector<float> > ascat_inc_mid(2);
    std::vector<std::vector<float> > ascat_inc_aft(2);
    std::vector<std::vector<float> > ascat_azi_fore(2);
    std::vector<std::vector<float> > ascat_azi_mid(2);
    std::vector<std::vector<float> > ascat_azi_aft(2);
    std::vector<std::vector<uint8> > ascat_midbeam_index(2);

    for(int ii = 0; ii < 2; ++ii) {
        ascat_time_diff[ii].resize(l2b_size);
        ascat_lat[ii].resize(l2b_size);
        ascat_lon[ii].resize(l2b_size);
        ascat_sigma0_fore[ii].resize(l2b_size);
        ascat_sigma0_mid[ii].resize(l2b_size);
        ascat_sigma0_aft[ii].resize(l2b_size);
        ascat_var_sigma0_fore[ii].resize(l2b_size);
        ascat_var_sigma0_mid[ii].resize(l2b_size);
        ascat_var_sigma0_aft[ii].resize(l2b_size);
        ascat_inc_fore[ii].resize(l2b_size);
        ascat_inc_mid[ii].resize(l2b_size);
        ascat_inc_aft[ii].resize(l2b_size);
        ascat_azi_fore[ii].resize(l2b_size);
        ascat_azi_mid[ii].resize(l2b_size);
        ascat_azi_aft[ii].resize(l2b_size);
        ascat_midbeam_index[ii].resize(l2b_size);
    }

    WindSwath wind_swath, wind_swath_ascat_only;
    wind_swath.Allocate(ncti, nati);
    wind_swath_ascat_only.Allocate(ncti, nati);

    for(int ati=0; ati<nati; ++ati) {
        if(ati%100 == 0)
            fprintf(stdout, "%d of %d\n", ati, nati);

        row_time[ati] = 
            rev_start + (rev_stop-rev_start)*(double)ati/(double)nati;

        out_row_time[ati] = row_time[ati] - out_time_base;

        for(int cti=0; cti<ncti; ++cti) {

            int l2bidx = ati + nati*cti;

            if(do_sst_gmf) {
                int anc_idx = cti + ncti*ati;
                float this_sst = anc_sst[anc_idx];
                out_anc_sst[l2bidx] = this_sst + 273.15;
                sst_gmf.Get(this_sst, &gmf);
            }

            // initialized datasets
            flags[l2bidx] = 65535;
            eflags[l2bidx] = 65535;
            num_ambiguities[l2bidx] = 255;
            ascat_only_num_ambiguities[l2bidx] = 255;
            scatsat_only_num_ambiguities[l2bidx] = 255;
            lat[l2bidx] = FILL_VALUE;
            lon[l2bidx] = FILL_VALUE;
            anc_spd[l2bidx] = FILL_VALUE;
            anc_dir[l2bidx] = FILL_VALUE;
            scatsat_only_spd[l2bidx] = FILL_VALUE;
            scatsat_only_spd_uncorrected[l2bidx] = FILL_VALUE;
            scatsat_only_dir[l2bidx] = FILL_VALUE;
            scatsat_rain_impact[l2bidx] = FILL_VALUE;
            spd[l2bidx] = FILL_VALUE;
            dir[l2bidx] = FILL_VALUE;
            ascat_only_spd[l2bidx] = FILL_VALUE;
            ascat_only_dir[l2bidx] = FILL_VALUE;

            scatsat_sigma0_hh_fore[l2bidx] = FILL_VALUE;
            scatsat_sigma0_hh_aft[l2bidx] = FILL_VALUE;
            scatsat_sigma0_vv_fore[l2bidx] = FILL_VALUE;
            scatsat_sigma0_vv_aft[l2bidx] = FILL_VALUE;
            scatsat_var_sigma0_hh_fore[l2bidx] = FILL_VALUE;
            scatsat_var_sigma0_hh_aft[l2bidx] = FILL_VALUE;
            scatsat_var_sigma0_vv_fore[l2bidx] = FILL_VALUE;
            scatsat_var_sigma0_vv_aft[l2bidx] = FILL_VALUE;
            scatsat_inc_hh_fore[l2bidx] = FILL_VALUE;
            scatsat_inc_hh_aft[l2bidx] = FILL_VALUE;
            scatsat_inc_vv_fore[l2bidx] = FILL_VALUE;
            scatsat_inc_vv_aft[l2bidx] = FILL_VALUE;
            scatsat_azi_hh_fore[l2bidx] = FILL_VALUE;
            scatsat_azi_hh_aft[l2bidx] = FILL_VALUE;
            scatsat_azi_vv_fore[l2bidx] = FILL_VALUE;
            scatsat_azi_vv_aft[l2bidx] = FILL_VALUE;
            scatsat_tb_h[l2bidx] = FILL_VALUE;
            scatsat_tb_v[l2bidx] = FILL_VALUE;

            for(int ii=0; ii<4; ++ii) {
                int ambidx = ii + 4*l2bidx;
                ambiguity_spd[ambidx] = FILL_VALUE;
                ambiguity_dir[ambidx] = FILL_VALUE;
                ambiguity_obj[ambidx] = FILL_VALUE;
                ascat_only_ambiguity_spd[ambidx] = FILL_VALUE;
                ascat_only_ambiguity_dir[ambidx] = FILL_VALUE;
            }

            for(int ii=0; ii<2; ++ii) {
                ascat_lon[ii][l2bidx] = FILL_VALUE;
                ascat_lat[ii][l2bidx] = FILL_VALUE;
                ascat_time_diff[ii][l2bidx] = FILL_VALUE;
                ascat_sigma0_fore[ii][l2bidx] = FILL_VALUE;
                ascat_sigma0_mid[ii][l2bidx] = FILL_VALUE;
                ascat_sigma0_aft[ii][l2bidx] = FILL_VALUE;
                ascat_var_sigma0_fore[ii][l2bidx] = FILL_VALUE;
                ascat_var_sigma0_mid[ii][l2bidx] = FILL_VALUE;
                ascat_var_sigma0_aft[ii][l2bidx] = FILL_VALUE;
                ascat_inc_fore[ii][l2bidx] = FILL_VALUE;
                ascat_inc_mid[ii][l2bidx] = FILL_VALUE;
                ascat_inc_aft[ii][l2bidx] = FILL_VALUE;
                ascat_azi_fore[ii][l2bidx] = FILL_VALUE;
                ascat_azi_mid[ii][l2bidx] = FILL_VALUE;
                ascat_azi_aft[ii][l2bidx] = FILL_VALUE;
                ascat_midbeam_index[ii][l2bidx] = 255;
            }

            // Get pointer to prev L2B file and L2A swath
            WVC* ku_ambig_wvc = l2b_scatsat_nofilt.frame.swath.GetWVC(cti, ati);
            WVC* ku_dirth_wvc = l2b_scatsat.frame.swath.GetWVC(cti, ati);

            if(!l2a_scatsat_swath[cti][ati] || !ku_ambig_wvc || !ku_dirth_wvc)
                continue;

            anc_spd[l2bidx] = ku_dirth_wvc->nudgeWV->spd;
            anc_dir[l2bidx] = pe_rad_to_gs_deg(ku_dirth_wvc->nudgeWV->dir);
            if(anc_dir[l2bidx]>180) anc_dir[l2bidx]-=360;

            scatsat_only_spd[l2bidx] = ku_dirth_wvc->selected->spd;
            scatsat_only_dir[l2bidx] = pe_rad_to_gs_deg(
                ku_dirth_wvc->selected->dir);
            if(scatsat_only_dir[l2bidx]>180) scatsat_only_dir[l2bidx]-=360;

            scatsat_only_spd_uncorrected[l2bidx] =
                ku_dirth_wvc->selected->spd + ku_dirth_wvc->speedBias;

            if (ku_dirth_wvc->rainCorrectedSpeed != -1)
                scatsat_rain_impact[l2bidx] = ku_dirth_wvc->rainImpact;

            MeasList* scatsat_ml = &(l2a_scatsat_swath[cti][ati]->measList);

            LonLat lonlat = scatsat_ml->AverageLonLat();
            lon[l2bidx] = rtd * lonlat.longitude;
            lat[l2bidx] = rtd * lonlat.latitude;

            // wrap to [-180, 180) interval
            while(lon[l2bidx]>=180) lon[l2bidx] -= 360;
            while(lon[l2bidx]<-180) lon[l2bidx] += 360;

            float sum_s0[2][2], sum_s02[2][2], sum_tb[2];
            float sum_cos_azi[2][2], sum_sin_azi[2][2];
            float sum_inc[2][2];
            int cnts[2][2];


            for(int ipol=0; ipol < 2; ++ipol) {
                sum_tb[ipol] = 0;
                for(int ilook=0; ilook < 2; ++ilook) {
                    sum_s0[ilook][ipol] = 0;
                    sum_s02[ilook][ipol] = 0;
                    sum_cos_azi[ilook][ipol] = 0;
                    sum_sin_azi[ilook][ipol] = 0;
                    sum_inc[ilook][ipol] = 0;
                    cnts[ilook][ipol] = 0;
                }
            }

            for(Meas* meas = scatsat_ml->GetHead(); meas;
                meas = scatsat_ml->GetNext()) {

                int ilook = 0;
                if(meas->scanAngle > pi/2 && meas->scanAngle < 3*pi/2)
                    ilook = 1;

                int ipol = meas->beamIdx;

                sum_tb[ipol] += meas->txPulseWidth;
                sum_s0[ilook][ipol] += meas->value*ku_s0_adj_lin;
                sum_s02[ilook][ipol] += pow(meas->value*ku_s0_adj_lin, 2);
                sum_cos_azi[ilook][ipol] += cos(meas->eastAzimuth);
                sum_sin_azi[ilook][ipol] += sin(meas->eastAzimuth);
                sum_inc[ilook][ipol] += meas->incidenceAngle;
                cnts[ilook][ipol]++;
            }

            MeasList ml_joint;

            for(int ilook=0; ilook < 2; ++ilook) {
                for(int ipol=0; ipol < 2; ++ipol) {

                    if(cnts[ilook][ipol] < 2)
                        continue;

                    Meas* this_meas = new Meas();
                    this_meas->value = 
                        sum_s0[ilook][ipol]/(float)cnts[ilook][ipol];

                    this_meas->incidenceAngle = 
                        sum_inc[ilook][ipol]/(float)cnts[ilook][ipol];

                    this_meas->eastAzimuth = atan2(
                        sum_sin_azi[ilook][ipol], sum_cos_azi[ilook][ipol]);

                    this_meas->beamIdx = ipol;

                    this_meas->measType = 
                        (ipol == 0) ? Meas::HH_MEAS_TYPE : Meas::VV_MEAS_TYPE;

                    double this_var = 
                        sum_s02[ilook][ipol]/(float)cnts[ilook][ipol] - 
                        pow(this_meas->value, 2);

                    this_var *= (float)cnts[ilook][ipol] / 
                        (float)(cnts[ilook][ipol]-1);

                    this_meas->A = 1;
                    this_meas->B = 0;
                    this_meas->C = this_var;
                    this_meas->numSlices = -1;

                    ml_joint.Append(this_meas);

                    float this_azi = pe_rad_to_gs_deg(this_meas->eastAzimuth);
                    while(this_azi>=180) this_azi -= 360;
                    while(this_azi<-180) this_azi += 360;

                    if(ipol == 0 && ilook == 0) {
                        scatsat_sigma0_hh_fore[l2bidx] =
                            this_meas->value/ku_s0_adj_lin;
                        scatsat_var_sigma0_hh_fore[l2bidx] =
                            this_meas->C / pow(ku_s0_adj_lin, 2);
                        scatsat_inc_hh_fore[l2bidx] = 
                            this_meas->incidenceAngle * rtd;
                        scatsat_azi_hh_fore[l2bidx] = this_azi;

                    } else if(ipol == 0 && ilook == 1) {
                        scatsat_sigma0_hh_aft[l2bidx] =
                            this_meas->value/ku_s0_adj_lin;
                        scatsat_var_sigma0_hh_aft[l2bidx] =
                            this_meas->C / pow(ku_s0_adj_lin, 2);
                        scatsat_inc_hh_aft[l2bidx] = 
                            this_meas->incidenceAngle * rtd;
                        scatsat_azi_hh_aft[l2bidx] = this_azi;

                    } else if(ipol == 1 && ilook == 0) {
                        scatsat_sigma0_vv_fore[l2bidx] =
                            this_meas->value/ku_s0_adj_lin;
                        scatsat_var_sigma0_vv_fore[l2bidx] =
                            this_meas->C / pow(ku_s0_adj_lin, 2);
                        scatsat_inc_vv_fore[l2bidx] = 
                            this_meas->incidenceAngle * rtd;
                        scatsat_azi_vv_fore[l2bidx] = this_azi;

                    } else if(ipol == 1 && ilook == 1) {
                        scatsat_sigma0_vv_aft[l2bidx] =
                            this_meas->value/ku_s0_adj_lin;
                        scatsat_var_sigma0_vv_aft[l2bidx] =
                            this_meas->C / pow(ku_s0_adj_lin, 2);
                        scatsat_inc_vv_aft[l2bidx] = 
                            this_meas->incidenceAngle * rtd;
                        scatsat_azi_vv_aft[l2bidx] = this_azi;
                    }
                }
            }

            scatsat_tb_h[l2bidx] = sum_tb[0] / (cnts[0][0]+cnts[1][0]);
            scatsat_tb_v[l2bidx] = sum_tb[1] / (cnts[0][1]+cnts[1][1]);

            if(ascat_only)
                ml_joint.FreeContents();

            int any_ascat_land[2] = {0, 0};
            int any_ascat_ice[2] = {0, 0};

            // First compute time offset for each ASCAT
            for(int imetop = 0; imetop < 2; ++imetop) {

                // If nothing here skip
                if(!l2a_ascat_swaths[imetop][cti][ati] || scatsat_only ||
                    drop_ascat[imetop])
                    continue;

                MeasList* ascat_ml =
                    &(l2a_ascat_swaths[imetop][cti][ati]->measList);

                double sum_time = 0;
                int cnts_time = 0;

                // Average over each beam per WVC
                for(Meas* meas = ascat_ml->GetHead(); meas;
                    meas = ascat_ml->GetNext()) {
                    sum_time += meas->C;
                    cnts_time++;
                }
                ascat_time_diff[imetop][l2bidx] = (
                    sum_time/(float)cnts_time - row_time[ati]);
            }

            int use_this_ascat[2] = {0, 0};
            if (fabs(ascat_time_diff[0][l2bidx]) <
                fabs(ascat_time_diff[1][l2bidx])) use_this_ascat[0] = 1;

            if (fabs(ascat_time_diff[1][l2bidx]) <
                fabs(ascat_time_diff[0][l2bidx])) use_this_ascat[1] = 1;


            // loop over both ASCATs
            for(int imetop = 0; imetop < 2; ++imetop) {

                // If nothing here skip
                if(!l2a_ascat_swaths[imetop][cti][ati] || scatsat_only ||
                    drop_ascat[imetop])
                    continue;

                MeasList* ascat_ml =
                    &(l2a_ascat_swaths[imetop][cti][ati]->measList);

                // Step through ASCAT meas, discard more than 50 minutes and
                // land/ice.
                if (1) { // scope this down
                    Meas* meas = ascat_ml->GetHead();
                    while(meas) {
                        int delete_it = 0;
                        if(fabs(row_time[ati] - meas->C) > 50*60)
                            delete_it = 1;

                        if(meas->landFlag) {
                            if(meas->landFlag == 1 || meas->landFlag == 3)
                                any_ascat_land[imetop] = 1;

                            if(meas->landFlag == 2 || meas->landFlag == 3)
                                any_ascat_ice[imetop] = 1;

                            delete_it = 1;
                        }

                        if(delete_it) {
                            meas = ascat_ml->RemoveCurrent();
                            delete meas;
                            meas = ascat_ml->GetCurrent();
                        } else {
                            meas = ascat_ml->GetNext();
                        }
                    }
                }

                // If tossed all of them skip
                if(ascat_ml->NodeCount() == 0)
                    continue;

                LonLat lonlat = ascat_ml->AverageLonLat();
                ascat_lon[imetop][l2bidx] = rtd * lonlat.longitude;
                ascat_lat[imetop][l2bidx] = rtd * lonlat.latitude;

                // wrap to [-180, 180) interval
                if(ascat_lon[imetop][l2bidx] >= 180)
                    ascat_lon[imetop][l2bidx] -= 360;

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

                double sum_time = 0;
                int cnts_time = 0;

                // Average over each beam per WVC
                for(Meas* meas = ascat_ml->GetHead(); meas;
                    meas = ascat_ml->GetNext()) {

                    cnts[meas->beamIdx]++;
                    sum_s0[meas->beamIdx] += meas->value;
                    sum_s02[meas->beamIdx] += pow(meas->value, 2);
                    sum_inc[meas->beamIdx] += meas->incidenceAngle;
                    sum_cos_azi[meas->beamIdx] += cos(meas->eastAzimuth);
                    sum_sin_azi[meas->beamIdx] += sin(meas->eastAzimuth);

                    sum_time += meas->C;
                    cnts_time++;
                }

                // Create averaged measurements
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

                    if(cnts[ibeam] < 2)
                        continue;

                    Meas* this_meas = new Meas();
                    this_meas->value = sum_s0[ibeam]/(float)cnts[ibeam];
                    this_meas->measType = Meas::C_BAND_VV_MEAS_TYPE;
                    this_meas->incidenceAngle =
                        sum_inc[ibeam]/(float)cnts[ibeam];

                    this_meas->eastAzimuth = atan2(
                        sum_sin_azi[ibeam], sum_cos_azi[ibeam]);

                    double this_var = 
                        sum_s02[ibeam]/(float)cnts[ibeam] -
                        pow(this_meas->value, 2);

                    this_var *= (float)cnts[ibeam]/(float)(cnts[ibeam]-1);

                    this_meas->A = 1;
                    this_meas->B = 0;
                    this_meas->C = this_var;
                    this_meas->numSlices = -1;

                    // Only append on with smallest matchup time
                    if(use_this_ascat[imetop])
                        ml_joint.Append(this_meas);
                    else
                        delete this_meas;

                    float this_azi = pe_rad_to_gs_deg(
                        this_meas->eastAzimuth);
                    if(this_azi >= 180) this_azi -= 360;

                    if(ibeam == 0 || ibeam == 3) {
                        ascat_sigma0_fore[imetop][l2bidx] = this_meas->value;
                        ascat_var_sigma0_fore[imetop][l2bidx] = this_meas->C;
                        ascat_inc_fore[imetop][l2bidx] = this_inc;
                        ascat_azi_fore[imetop][l2bidx] = this_azi;

                    } else if(ibeam == 1 || ibeam == 4) {
                        ascat_midbeam_index[imetop][l2bidx] = ibeam;
                        ascat_sigma0_mid[imetop][l2bidx] = this_meas->value;
                        ascat_var_sigma0_mid[imetop][l2bidx] = this_meas->C;
                        ascat_inc_mid[imetop][l2bidx] = this_inc;
                        ascat_azi_mid[imetop][l2bidx] = this_azi;

                    } else if(ibeam == 2 || ibeam == 5) {
                        ascat_sigma0_aft[imetop][l2bidx] = this_meas->value;
                        ascat_var_sigma0_aft[imetop][l2bidx] = this_meas->C;
                        ascat_inc_aft[imetop][l2bidx] = this_inc;
                        ascat_azi_aft[imetop][l2bidx] = this_azi;
                    }
                }
            }

            if(ml_joint.NodeCount() == 0)
                continue;

            WVC* wvc = new WVC();
            gmf->RetrieveWinds_S3(&ml_joint, &kp, wvc);
//             gmf->RetrieveWinds_S3(scatsat_ml, &kp, wvc);

            if(wvc->ambiguities.NodeCount() > 0) {

                // Nudge it with the SCATSAT only DIRTH wind direction
                wvc->selected = wvc->GetNearestToDirection(
                    ku_dirth_wvc->selected->dir);
                wind_swath.Add(cti, ati, wvc);

                // Set quality flags
                flags[l2bidx] = 0;
                eflags[l2bidx] = 0;

                // land
                if (ku_dirth_wvc->landiceFlagBits == 1 || 
                    ku_dirth_wvc->landiceFlagBits == 3)
                    flags[l2bidx] |= 0x0080;

                // ice
                if (ku_dirth_wvc->landiceFlagBits == 2 ||
                    ku_dirth_wvc->landiceFlagBits == 3)
                    flags[l2bidx] |= 0x0100;

                // rain
                if(ku_dirth_wvc->rainFlagBits & RAIN_FLAG_UNUSABLE)
                    flags[l2bidx] |= 0x1000;

                if(ku_dirth_wvc->rainFlagBits & RAIN_FLAG_RAIN)
                    flags[l2bidx] |= 0x2000;

            } else
                delete wvc;

            // Next level of scope so vars get destroyed
            if (1) {
                // toss Ku meas flavors
                Meas* meas = ml_joint.GetHead();
                while(meas) {
                    if (meas->measType == Meas::HH_MEAS_TYPE ||
                        meas->measType == Meas::VV_MEAS_TYPE) {
                        meas = ml_joint.RemoveCurrent();
                        delete meas;
                        meas = ml_joint.GetCurrent();
                    } else {
                        meas = ml_joint.GetNext();
                    }
                }
            }

            if(ml_joint.NodeCount() == 0)
                continue;

            WVC* ascat_only_wvc = new WVC();
            gmf->RetrieveWinds_S3(&ml_joint, &kp, ascat_only_wvc);
            if(ascat_only_wvc->ambiguities.NodeCount() > 0) {
                // Nudge it with the SCATSAT only DIRTH wind direction
                ascat_only_wvc->selected = ascat_only_wvc->GetNearestToDirection(
                    ku_dirth_wvc->selected->dir);
                wind_swath_ascat_only.Add(cti, ati, ascat_only_wvc);

            } else
                delete ascat_only_wvc;
        }
    }

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
            WVC* ku_ambig_wvc = l2b_scatsat_nofilt.frame.swath.GetWVC(cti, ati);
            WVC* c_ambig_wvc = wind_swath_ascat_only.GetWVC(cti, ati);

            if(!wvc || !ku_ambig_wvc)
                continue;

            num_ambiguities[l2bidx] = wvc->ambiguities.NodeCount();
            scatsat_only_num_ambiguities[l2bidx] =
                ku_ambig_wvc->ambiguities.NodeCount();

            int j_amb = 0;
            for(WindVectorPlus* wvp = wvc->ambiguities.GetHead(); wvp;
                wvp = wvc->ambiguities.GetNext()){

                int ambidx = j_amb + 4*l2bidx;
                ambiguity_spd[ambidx] = wvp->spd;
                ambiguity_dir[ambidx] = pe_rad_to_gs_deg(wvp->dir);
                ambiguity_obj[ambidx] = wvp->obj;
                if(ambiguity_dir[ambidx]>180) ambiguity_dir[ambidx] -= 360;
                ++j_amb;
            }

            j_amb = 0;
            for(WindVectorPlus* wvp = ku_ambig_wvc->ambiguities.GetHead(); wvp;
                wvp = ku_ambig_wvc->ambiguities.GetNext()){

                int ambidx = j_amb + 4*l2bidx;
                scatsat_only_ambiguity_spd[ambidx] = wvp->spd;
                scatsat_only_ambiguity_dir[ambidx] = pe_rad_to_gs_deg(wvp->dir);
                if(scatsat_only_ambiguity_dir[ambidx]>180)
                    scatsat_only_ambiguity_dir[ambidx] -= 360;
                ++j_amb;
            }

            if(!c_ambig_wvc)
                continue;

            ascat_only_num_ambiguities[l2bidx] =
                c_ambig_wvc->ambiguities.NodeCount();

            j_amb = 0;
            for(WindVectorPlus* wvp = c_ambig_wvc->ambiguities.GetHead(); wvp;
                wvp = c_ambig_wvc->ambiguities.GetNext()){

                int ambidx = j_amb + 4*l2bidx;
                ascat_only_ambiguity_spd[ambidx] = wvp->spd;
                ascat_only_ambiguity_dir[ambidx] = pe_rad_to_gs_deg(wvp->dir);
                if(ascat_only_ambiguity_dir[ambidx]>180)
                    ascat_only_ambiguity_dir[ambidx] -= 360;
                ++j_amb;
            }


        }
    }

    // DIRTH Pass on joint C/Ku swath
    wind_swath.MedianFilter(
        median_filter_window_size, median_filter_max_passes, 0, 0, 1);

    // Do ASCAT-only ambiguity removal and DIRTH
    wind_swath_ascat_only.MedianFilter_4Pass(
        median_filter_window_size, median_filter_max_passes, 0);

    wind_swath_ascat_only.MedianFilter(
        median_filter_window_size, median_filter_max_passes, 0, 0, 1);

    // copy DIRTH outputs
    for(int ati=0; ati<nati; ++ati) {
        for(int cti=0; cti<ncti; ++cti) {
            int l2bidx = ati + nati*cti;

            WVC* wvc = wind_swath.GetWVC(cti, ati);
            if(wvc) {
                spd[l2bidx] = wvc->selected->spd;
                dir[l2bidx] = pe_rad_to_gs_deg(wvc->selected->dir);
                if(dir[l2bidx]>180) dir[l2bidx] -= 360;
            }

            wvc = wind_swath_ascat_only.GetWVC(cti, ati);
            if(wvc) {
                ascat_only_spd[l2bidx] = wvc->selected->spd;
                ascat_only_dir[l2bidx] = pe_rad_to_gs_deg(wvc->selected->dir);
                if(dir[l2bidx]>180) dir[l2bidx] -= 360;
            }
        }
    }


    hid_t file_id = H5Fcreate(
        l2b_file, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

   float valid_max, valid_min;
    hsize_t dims[3] = {ncti, nati, 4};
    hsize_t dims_amb[3] = {ncti, nati, 4};

    H5LTmake_dataset(
        file_id, "row_time", 1, (dims+1), H5T_NATIVE_DOUBLE,
        &out_row_time[0]);
    H5LTset_attribute_string(
        file_id, "row_time", "units", "seconds since 2016-1-1 00:00:00 UTC");
    

    valid_max = 90*60; valid_min = -90*60;
    H5LTmake_dataset(
        file_id, "ascat_a_time_diff", 2, dims, H5T_NATIVE_FLOAT,
        &ascat_time_diff[0][0]);
    H5LTset_attribute_float(
        file_id, "ascat_a_time_diff", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ascat_a_time_diff", "valid_min", &valid_min, 1);
    H5LTset_attribute_string(file_id, "ascat_a_time_diff", "units", "s");

    H5LTmake_dataset(
        file_id, "ascat_b_time_diff", 2, dims, H5T_NATIVE_FLOAT,
        &ascat_time_diff[1][0]);
    H5LTset_attribute_float(
        file_id, "ascat_b_time_diff", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ascat_b_time_diff", "valid_min", &valid_min, 1);
    H5LTset_attribute_string(file_id, "ascat_b_time_diff", "units", "s");

    valid_max = 9999; valid_min = -1;
    H5LTmake_dataset(
        file_id, "scatsat_rain_impact", 2, dims, H5T_NATIVE_FLOAT,
        &scatsat_rain_impact[0]);
    H5LTset_attribute_float(
        file_id, "scatsat_rain_impact", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "scatsat_rain_impact", "valid_min", &valid_min, 1);


    valid_max = 90; valid_min = -90;
    H5LTmake_dataset(file_id, "lat", 2, dims, H5T_NATIVE_FLOAT, &lat[0]);
    H5LTset_attribute_float(file_id, "lat", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "lat", "valid_min", &valid_min, 1);
    H5LTset_attribute_string(file_id, "lat", "units", "degrees_north");

//     H5LTmake_dataset(
//         file_id, "ascat_a_lat", 2, dims, H5T_NATIVE_FLOAT, &ascat_lat[0][0]);
//     H5LTset_attribute_float(file_id, "ascat_a_lat", "valid_max", &valid_max, 1);
//     H5LTset_attribute_float(file_id, "ascat_a_lat", "valid_min", &valid_min, 1);
//     H5LTset_attribute_string(file_id, "ascat_a_lat", "units", "degrees_north");
// 
//     H5LTmake_dataset(
//         file_id, "ascat_b_lat", 2, dims, H5T_NATIVE_FLOAT, &ascat_lat[1][0]);
//     H5LTset_attribute_float(file_id, "ascat_a_lat", "valid_max", &valid_max, 1);
//     H5LTset_attribute_float(file_id, "ascat_a_lat", "valid_min", &valid_min, 1);
//     H5LTset_attribute_string(file_id, "ascat_a_lat", "units", "degrees_north");

    valid_max = 180; valid_min = -180;
    H5LTmake_dataset(file_id, "lon", 2, dims, H5T_NATIVE_FLOAT, &lon[0]);
    H5LTset_attribute_float(file_id, "lon", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "lon", "valid_min", &valid_min, 1);
    H5LTset_attribute_string(file_id, "lon", "units", "degrees_east");

//     H5LTmake_dataset(
//         file_id, "ascat_a_lon", 2, dims, H5T_NATIVE_FLOAT, &ascat_lon[0][0]);
//     H5LTset_attribute_float(file_id, "ascat_a_lon", "valid_max", &valid_max, 1);
//     H5LTset_attribute_float(file_id, "ascat_a_lon", "valid_min", &valid_min, 1);
//     H5LTset_attribute_string(file_id, "ascat_a_lon", "units", "degrees_north");
// 
//     H5LTmake_dataset(
//         file_id, "ascat_b_lon", 2, dims, H5T_NATIVE_FLOAT, &ascat_lon[1][0]);
//     H5LTset_attribute_float(file_id, "ascat_b_lon", "valid_max", &valid_max, 1);
//     H5LTset_attribute_float(file_id, "ascat_b_lon", "valid_min", &valid_min, 1);
//     H5LTset_attribute_string(file_id, "ascat_b_lon", "units", "degrees_north");

    if(do_sst_gmf) {
        valid_max = 340; valid_min = 0;
        H5LTmake_dataset(
            file_id, "anc_sst", 2, dims, H5T_NATIVE_FLOAT, &out_anc_sst[0]);
        H5LTset_attribute_float(file_id, "anc_sst", "valid_max", &valid_max, 1);
        H5LTset_attribute_float(file_id, "anc_sst", "valid_min", &valid_min, 1);
        H5LTset_attribute_string(file_id, "anc_sst", "units", "K");
    }


    valid_max = 100; valid_min = 0;
    H5LTmake_dataset(file_id, "anc_spd", 2, dims, H5T_NATIVE_FLOAT, &anc_spd[0]);
    H5LTset_attribute_float(file_id, "anc_spd", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "anc_spd", "valid_min", &valid_min, 1);
    H5LTset_attribute_string(file_id, "anc_spd", "units", "m s-1");

    H5LTmake_dataset(
        file_id, "scatsat_only_spd_uncorrected", 2, dims, H5T_NATIVE_FLOAT, 
        &scatsat_only_spd_uncorrected[0]);
    H5LTset_attribute_float(
        file_id, "scatsat_only_spd_uncorrected", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "scatsat_only_spd_uncorrected", "valid_min", &valid_min, 1);
    H5LTset_attribute_string(
        file_id, "scatsat_only_spd_uncorrected", "units", "m s-1");

    H5LTmake_dataset(
        file_id, "scatsat_only_spd", 2, dims, H5T_NATIVE_FLOAT, 
        &scatsat_only_spd[0]);
    H5LTset_attribute_float(
        file_id, "scatsat_only_spd", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "scatsat_only_spd", "valid_min", &valid_min, 1);
    H5LTset_attribute_string(
        file_id, "scatsat_only_spd", "units", "m s-1");

    H5LTmake_dataset(file_id, "spd", 2, dims, H5T_NATIVE_FLOAT, &spd[0]);
    H5LTset_attribute_float(file_id, "spd", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "spd", "valid_min", &valid_min, 1);
    H5LTset_attribute_string(file_id, "spd", "units", "m s-1");

    H5LTmake_dataset(
        file_id, "ascat_only_spd", 2, dims, H5T_NATIVE_FLOAT,
        &ascat_only_spd[0]);
    H5LTset_attribute_float(
        file_id, "ascat_only_spd", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ascat_only_spd", "valid_min", &valid_min, 1);
    H5LTset_attribute_string(file_id, "ascat_only_spd", "units", "m s-1");

    H5LTmake_dataset(
        file_id, "scatsat_only_ambiguity_spd", 3, dims, H5T_NATIVE_FLOAT,
        &scatsat_only_ambiguity_spd[0]);
    H5LTset_attribute_float(
        file_id, "scatsat_only_ambiguity_spd", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "scatsat_only_ambiguity_spd", "valid_min", &valid_min, 1);
    H5LTset_attribute_string(
        file_id, "scatsat_only_ambiguity_spd", "units", "m s-1");

    H5LTmake_dataset(
        file_id, "ambiguity_spd", 3, dims, H5T_NATIVE_FLOAT, &ambiguity_spd[0]);
    H5LTset_attribute_float(
        file_id, "ambiguity_spd", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ambiguity_spd", "valid_min", &valid_min, 1);
    H5LTset_attribute_string(
        file_id, "ambiguity_spd", "units", "m s-1");

    H5LTmake_dataset(
        file_id, "ascat_only_ambiguity_spd", 3, dims, H5T_NATIVE_FLOAT,
        &ascat_only_ambiguity_spd[0]);
    H5LTset_attribute_float(
        file_id, "ascat_only_ambiguity_spd", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ascat_only_ambiguity_spd", "valid_min", &valid_min, 1);
    H5LTset_attribute_string(
        file_id, "ascat_only_ambiguity_spd", "units", "m s-1");


    valid_max = 180; valid_min = -180;
    H5LTmake_dataset(
        file_id, "anc_dir", 2, dims, H5T_NATIVE_FLOAT, &anc_dir[0]);
    H5LTset_attribute_float(file_id, "anc_dir", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "anc_dir", "valid_min", &valid_min, 1);
    H5LTset_attribute_string(file_id, "anc_dir", "units", "degrees");

    H5LTmake_dataset(
        file_id, "scatsat_only_dir", 2, dims, H5T_NATIVE_FLOAT,
        &scatsat_only_dir[0]);
    H5LTset_attribute_float(
        file_id, "scatsat_only_dir", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "scatsat_only_dir", "valid_min", &valid_min, 1);
    H5LTset_attribute_string(
        file_id, "scatsat_only_dir", "units", "degrees");

    H5LTmake_dataset(file_id, "dir", 2, dims, H5T_NATIVE_FLOAT, &dir[0]);
    H5LTset_attribute_float(file_id, "dir", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "dir", "valid_min", &valid_min, 1);
    H5LTset_attribute_string(file_id, "dir", "units", "degrees");

    H5LTmake_dataset(
        file_id, "ascat_only_dir", 2, dims, H5T_NATIVE_FLOAT,
        &ascat_only_dir[0]);
    H5LTset_attribute_float(
        file_id, "ascat_only_dir", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ascat_only_dir", "valid_min", &valid_min, 1);
    H5LTset_attribute_string(file_id, "ascat_only_dir", "units", "degrees");

    H5LTmake_dataset(
        file_id, "ambiguity_dir", 3, dims, H5T_NATIVE_FLOAT,
        &ambiguity_dir[0]);
    H5LTset_attribute_float(
        file_id, "ambiguity_dir", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ambiguity_dir", "valid_min", &valid_min, 1);
    H5LTset_attribute_string(file_id, "ambiguity_dir", "units", "degrees");

    H5LTmake_dataset(
        file_id, "scatsat_only_ambiguity_dir", 3, dims, H5T_NATIVE_FLOAT,
        &scatsat_only_ambiguity_dir[0]);
    H5LTset_attribute_float(
        file_id, "scatsat_only_ambiguity_dir", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "scatsat_only_ambiguity_dir", "valid_min", &valid_min, 1);
    H5LTset_attribute_string(
        file_id, "scatsat_only_ambiguity_dir", "units", "degrees");

    H5LTmake_dataset(
        file_id, "ascat_only_ambiguity_dir", 3, dims, H5T_NATIVE_FLOAT,
        &ascat_only_ambiguity_dir[0]);
    H5LTset_attribute_float(
        file_id, "ascat_only_ambiguity_dir", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ascat_only_ambiguity_dir", "valid_min", &valid_min, 1);
    H5LTset_attribute_string(
        file_id, "ascat_only_ambiguity_dir", "units", "degrees");

    valid_max = 300; valid_min = 0;
    H5LTmake_dataset(
        file_id, "scatsat_tb_h", 2, dims, H5T_NATIVE_FLOAT,
        &scatsat_tb_h[0]);
    H5LTset_attribute_float(
        file_id, "scatsat_tb_h", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "scatsat_tb_h", "valid_min", &valid_min, 1);
    H5LTset_attribute_string(
        file_id, "scatsat_tb_h", "units", "degrees K");

    H5LTmake_dataset(
        file_id, "scatsat_tb_v", 2, dims, H5T_NATIVE_FLOAT,
        &scatsat_tb_v[0]);
    H5LTset_attribute_float(
        file_id, "scatsat_tb_v", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "scatsat_tb_v", "valid_min", &valid_min, 1);
    H5LTset_attribute_string(
        file_id, "scatsat_tb_v", "units", "degrees K");

    valid_max = 10; valid_min = -0.01;
    H5LTmake_dataset(
        file_id, "scatsat_sigma0_hh_fore", 2, dims, H5T_NATIVE_FLOAT,
        &scatsat_sigma0_hh_fore[0]);
    H5LTset_attribute_float(
        file_id, "scatsat_sigma0_hh_fore", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "scatsat_sigma0_hh_fore", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "scatsat_sigma0_hh_aft", 2, dims, H5T_NATIVE_FLOAT,
        &scatsat_sigma0_hh_aft[0]);
    H5LTset_attribute_float(
        file_id, "scatsat_sigma0_hh_aft", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "scatsat_sigma0_hh_aft", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "scatsat_sigma0_vv_fore", 2, dims, H5T_NATIVE_FLOAT,
        &scatsat_sigma0_vv_fore[0]);
    H5LTset_attribute_float(
        file_id, "scatsat_sigma0_vv_fore", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "scatsat_sigma0_vv_fore", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "scatsat_sigma0_vv_aft", 2, dims, H5T_NATIVE_FLOAT,
        &scatsat_sigma0_vv_aft[0]);
    H5LTset_attribute_float(
        file_id, "scatsat_sigma0_vv_aft", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "scatsat_sigma0_vv_aft", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "ascat_a_sigma0_fore", 2, dims, H5T_NATIVE_FLOAT,
        &ascat_sigma0_fore[0][0]);
    H5LTset_attribute_float(
        file_id, "ascat_a_sigma0_fore", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ascat_a_sigma0_fore", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "ascat_a_sigma0_mid", 2, dims, H5T_NATIVE_FLOAT,
        &ascat_sigma0_mid[0][0]);
    H5LTset_attribute_float(
        file_id, "ascat_a_sigma0_mid", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ascat_a_sigma0_mid", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "ascat_a_sigma0_aft", 2, dims, H5T_NATIVE_FLOAT,
        &ascat_sigma0_aft[0][0]);
    H5LTset_attribute_float(
        file_id, "ascat_a_sigma0_aft", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ascat_a_sigma0_aft", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "ascat_b_sigma0_fore", 2, dims, H5T_NATIVE_FLOAT,
        &ascat_sigma0_fore[1][0]);
    H5LTset_attribute_float(
        file_id, "ascat_b_sigma0_fore", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ascat_b_sigma0_fore", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "ascat_b_sigma0_mid", 2, dims, H5T_NATIVE_FLOAT,
        &ascat_sigma0_mid[1][0]);
    H5LTset_attribute_float(
        file_id, "ascat_b_sigma0_mid", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ascat_b_sigma0_mid", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "ascat_b_sigma0_aft", 2, dims, H5T_NATIVE_FLOAT,
        &ascat_sigma0_aft[1][0]);
    H5LTset_attribute_float(
        file_id, "ascat_b_sigma0_aft", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ascat_b_sigma0_aft", "valid_min", &valid_min, 1);


    H5LTmake_dataset(
        file_id, "scatsat_var_sigma0_hh_fore", 2, dims, H5T_NATIVE_FLOAT,
        &scatsat_var_sigma0_hh_fore[0]);
    H5LTset_attribute_float(
        file_id, "scatsat_var_sigma0_hh_fore", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "scatsat_var_sigma0_hh_fore", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "scatsat_var_sigma0_hh_aft", 2, dims, H5T_NATIVE_FLOAT,
        &scatsat_var_sigma0_hh_aft[0]);
    H5LTset_attribute_float(
        file_id, "scatsat_var_sigma0_hh_aft", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "scatsat_var_sigma0_hh_aft", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "scatsat_var_sigma0_vv_fore", 2, dims, H5T_NATIVE_FLOAT,
        &scatsat_var_sigma0_vv_fore[0]);
    H5LTset_attribute_float(
        file_id, "scatsat_var_sigma0_vv_fore", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "scatsat_var_sigma0_vv_fore", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "scatsat_var_sigma0_vv_aft", 2, dims, H5T_NATIVE_FLOAT,
        &scatsat_var_sigma0_vv_aft[0]);
    H5LTset_attribute_float(
        file_id, "scatsat_var_sigma0_vv_aft", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "scatsat_var_sigma0_vv_aft", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "ascat_a_var_sigma0_fore", 2, dims, H5T_NATIVE_FLOAT,
        &ascat_var_sigma0_fore[0][0]);
    H5LTset_attribute_float(
        file_id, "ascat_a_var_sigma0_fore", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ascat_a_var_sigma0_fore", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "ascat_a_var_sigma0_mid", 2, dims, H5T_NATIVE_FLOAT,
        &ascat_var_sigma0_mid[0][0]);
    H5LTset_attribute_float(
        file_id, "ascat_a_var_sigma0_mid", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ascat_a_var_sigma0_mid", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "ascat_a_var_sigma0_aft", 2, dims, H5T_NATIVE_FLOAT,
        &ascat_var_sigma0_aft[0][0]);
    H5LTset_attribute_float(
        file_id, "ascat_a_var_sigma0_aft", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ascat_a_var_sigma0_aft", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "ascat_b_var_sigma0_fore", 2, dims, H5T_NATIVE_FLOAT,
        &ascat_var_sigma0_fore[1][0]);
    H5LTset_attribute_float(
        file_id, "ascat_b_var_sigma0_fore", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ascat_b_var_sigma0_fore", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "ascat_b_var_sigma0_mid", 2, dims, H5T_NATIVE_FLOAT,
        &ascat_var_sigma0_mid[1][0]);
    H5LTset_attribute_float(
        file_id, "ascat_b_var_sigma0_mid", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ascat_b_var_sigma0_mid", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "ascat_b_var_sigma0_aft", 2, dims, H5T_NATIVE_FLOAT,
        &ascat_var_sigma0_aft[1][0]);
    H5LTset_attribute_float(
        file_id, "ascat_b_var_sigma0_aft", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ascat_b_var_sigma0_aft", "valid_min", &valid_min, 1);


    valid_max = 90; valid_min = 0;
    H5LTmake_dataset(
        file_id, "scatsat_inc_hh_fore", 2, dims, H5T_NATIVE_FLOAT,
        &scatsat_inc_hh_fore[0]);
    H5LTset_attribute_float(
        file_id, "scatsat_inc_hh_fore", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "scatsat_inc_hh_fore", "valid_min", &valid_min, 1);
    H5LTset_attribute_string(
        file_id, "scatsat_inc_hh_fore", "units", "degrees");

    H5LTmake_dataset(
        file_id, "scatsat_inc_hh_aft", 2, dims, H5T_NATIVE_FLOAT,
        &scatsat_inc_hh_aft[0]);
    H5LTset_attribute_float(
        file_id, "scatsat_inc_hh_aft", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "scatsat_inc_hh_aft", "valid_min", &valid_min, 1);
    H5LTset_attribute_string(
        file_id, "scatsat_inc_hh_aft", "units", "degrees");

    H5LTmake_dataset(
        file_id, "scatsat_inc_vv_fore", 2, dims, H5T_NATIVE_FLOAT,
        &scatsat_inc_vv_fore[0]);
    H5LTset_attribute_float(
        file_id, "scatsat_inc_vv_fore", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "scatsat_inc_vv_fore", "valid_min", &valid_min, 1);
    H5LTset_attribute_string(
        file_id, "scatsat_inc_vv_fore", "units", "degrees");

    H5LTmake_dataset(
        file_id, "scatsat_inc_vv_aft", 2, dims, H5T_NATIVE_FLOAT,
        &scatsat_inc_vv_aft[0]);
    H5LTset_attribute_float(
        file_id, "scatsat_inc_vv_aft", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "scatsat_inc_vv_aft", "valid_min", &valid_min, 1);
    H5LTset_attribute_string(
        file_id, "scatsat_inc_vv_aft", "units", "degrees");

    H5LTmake_dataset(
        file_id, "ascat_a_inc_fore", 2, dims, H5T_NATIVE_FLOAT,
        &ascat_inc_fore[0][0]);
    H5LTset_attribute_float(
        file_id, "ascat_a_inc_fore", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ascat_a_inc_fore", "valid_min", &valid_min, 1);
    H5LTset_attribute_string(
        file_id, "ascat_a_inc_fore", "units", "degrees");

    H5LTmake_dataset(
        file_id, "ascat_a_inc_mid", 2, dims, H5T_NATIVE_FLOAT,
        &ascat_inc_mid[0][0]);
    H5LTset_attribute_float(
        file_id, "ascat_a_inc_mid", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ascat_a_inc_mid", "valid_min", &valid_min, 1);
    H5LTset_attribute_string(
        file_id, "scatsat_inc_vv_aft", "units", "degrees");

    H5LTmake_dataset(
        file_id, "ascat_a_inc_aft", 2, dims, H5T_NATIVE_FLOAT,
        &ascat_inc_aft[0][0]);
    H5LTset_attribute_float(
        file_id, "ascat_a_inc_aft", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ascat_a_inc_aft", "valid_min", &valid_min, 1);
    H5LTset_attribute_string(
        file_id, "ascat_a_inc_aft", "units", "degrees");

    H5LTmake_dataset(
        file_id, "ascat_b_inc_fore", 2, dims, H5T_NATIVE_FLOAT,
        &ascat_inc_fore[1][0]);
    H5LTset_attribute_float(
        file_id, "ascat_b_inc_fore", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ascat_b_inc_fore", "valid_min", &valid_min, 1);
    H5LTset_attribute_string(
        file_id, "ascat_b_inc_fore", "units", "degrees");

    H5LTmake_dataset(
        file_id, "ascat_b_inc_mid", 2, dims, H5T_NATIVE_FLOAT,
        &ascat_inc_mid[1][0]);
    H5LTset_attribute_float(
        file_id, "ascat_b_inc_mid", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ascat_b_inc_mid", "valid_min", &valid_min, 1);
    H5LTset_attribute_string(
        file_id, "scatsat_inc_vv_aft", "units", "degrees");

    H5LTmake_dataset(
        file_id, "ascat_b_inc_aft", 2, dims, H5T_NATIVE_FLOAT,
        &ascat_inc_aft[1][0]);
    H5LTset_attribute_float(
        file_id, "ascat_b_inc_aft", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ascat_b_inc_aft", "valid_min", &valid_min, 1);
    H5LTset_attribute_string(
        file_id, "ascat_b_inc_aft", "units", "degrees");

    valid_max = 180; valid_min = -180;
    H5LTmake_dataset(
        file_id, "scatsat_azi_hh_fore", 2, dims, H5T_NATIVE_FLOAT,
        &scatsat_azi_hh_fore[0]);
    H5LTset_attribute_float(
        file_id, "scatsat_azi_hh_fore", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "scatsat_azi_hh_fore", "valid_min", &valid_min, 1);
    H5LTset_attribute_string(
        file_id, "scatsat_azi_hh_fore", "units", "degrees");

    H5LTmake_dataset(
        file_id, "scatsat_azi_hh_aft", 2, dims, H5T_NATIVE_FLOAT,
        &scatsat_azi_hh_aft[0]);
    H5LTset_attribute_float(
        file_id, "scatsat_azi_hh_aft", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "scatsat_azi_hh_aft", "valid_min", &valid_min, 1);
    H5LTset_attribute_string(
        file_id, "scatsat_azi_hh_aft", "units", "degrees");

    H5LTmake_dataset(
        file_id, "scatsat_azi_vv_fore", 2, dims, H5T_NATIVE_FLOAT,
        &scatsat_azi_vv_fore[0]);
    H5LTset_attribute_float(
        file_id, "scatsat_azi_vv_fore", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "scatsat_azi_vv_fore", "valid_min", &valid_min, 1);
    H5LTset_attribute_string(
        file_id, "scatsat_azi_vv_fore", "units", "degrees");

    H5LTmake_dataset(
        file_id, "scatsat_azi_vv_aft", 2, dims, H5T_NATIVE_FLOAT,
        &scatsat_azi_vv_aft[0]);
    H5LTset_attribute_float(
        file_id, "scatsat_azi_vv_aft", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "scatsat_azi_vv_aft", "valid_min", &valid_min, 1);
    H5LTset_attribute_string(
        file_id, "scatsat_azi_vv_aft", "units", "degrees");

    H5LTmake_dataset(
        file_id, "ascat_a_azi_fore", 2, dims, H5T_NATIVE_FLOAT,
        &ascat_azi_fore[0][0]);
    H5LTset_attribute_float(
        file_id, "ascat_a_azi_fore", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ascat_a_azi_fore", "valid_min", &valid_min, 1);
    H5LTset_attribute_string(
        file_id, "ascat_a_azi_fore", "units", "degrees");

    H5LTmake_dataset(
        file_id, "ascat_a_azi_mid", 2, dims, H5T_NATIVE_FLOAT,
        &ascat_azi_mid[0][0]);
    H5LTset_attribute_float(
        file_id, "ascat_a_azi_mid", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ascat_a_azi_mid", "valid_min", &valid_min, 1);
    H5LTset_attribute_string(
        file_id, "ascat_a_azi_mid", "units", "degrees");

    H5LTmake_dataset(
        file_id, "ascat_a_azi_aft", 2, dims, H5T_NATIVE_FLOAT,
        &ascat_azi_aft[0][0]);
    H5LTset_attribute_float(
        file_id, "ascat_a_azi_aft", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ascat_a_azi_aft", "valid_min", &valid_min, 1);
    H5LTset_attribute_string(
        file_id, "ascat_a_azi_aft", "units", "degrees");

    H5LTmake_dataset(
        file_id, "ascat_b_azi_fore", 2, dims, H5T_NATIVE_FLOAT,
        &ascat_azi_fore[1][0]);
    H5LTset_attribute_float(
        file_id, "ascat_b_azi_fore", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ascat_b_azi_fore", "valid_min", &valid_min, 1);
    H5LTset_attribute_string(
        file_id, "ascat_b_azi_fore", "units", "degrees");

    H5LTmake_dataset(
        file_id, "ascat_b_azi_mid", 2, dims, H5T_NATIVE_FLOAT,
        &ascat_azi_mid[1][0]);
    H5LTset_attribute_float(
        file_id, "ascat_b_azi_mid", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ascat_b_azi_mid", "valid_min", &valid_min, 1);
    H5LTset_attribute_string(
        file_id, "ascat_b_azi_mid", "units", "degrees");

    H5LTmake_dataset(
        file_id, "ascat_b_azi_aft", 2, dims, H5T_NATIVE_FLOAT,
        &ascat_azi_aft[1][0]);
    H5LTset_attribute_float(
        file_id, "ascat_b_azi_aft", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ascat_b_azi_aft", "valid_min", &valid_min, 1);
    H5LTset_attribute_string(
        file_id, "ascat_b_azi_aft", "units", "degrees");

    valid_max = 0; valid_min = -199;
    H5LTmake_dataset(
        file_id, "ambiguity_obj", 3, dims, H5T_NATIVE_FLOAT,
        &ambiguity_obj[0]);
    H5LTset_attribute_float(
        file_id, "ambiguity_obj", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ambiguity_obj", "valid_min", &valid_min, 1);

    unsigned char uchar_fill_value = 255;
    unsigned char uchar_valid_max = 4;
    unsigned char uchar_valid_min = 0;
    H5LTmake_dataset(
        file_id, "num_ambiguities", 2, dims, H5T_NATIVE_UCHAR,
        &num_ambiguities[0]);
    H5LTset_attribute_uchar(
        file_id, "num_ambiguities", "valid_max", &uchar_valid_max, 1);
    H5LTset_attribute_uchar(
        file_id, "num_ambiguities", "valid_min", &uchar_valid_min, 1);

    H5LTmake_dataset(
        file_id, "scatsat_only_num_ambiguities", 2, dims, H5T_NATIVE_UCHAR,
        &scatsat_only_num_ambiguities[0]);
    H5LTset_attribute_uchar(
        file_id, "scatsat_only_num_ambiguities", "valid_max", &uchar_valid_max,
        1);
    H5LTset_attribute_uchar(
        file_id, "scatsat_only_num_ambiguities", "valid_min", &uchar_valid_min,
        1);

    H5LTmake_dataset(
        file_id, "ascat_a_midbeam_index", 2, dims, H5T_NATIVE_UCHAR,
        &ascat_midbeam_index[0][0]);
    H5LTset_attribute_uchar(
        file_id, "ascat_a_midbeam_index", "valid_max", &uchar_valid_max,
        1);
    H5LTset_attribute_uchar(
        file_id, "ascat_a_midbeam_index", "valid_min", &uchar_valid_min,
        1);

    H5LTmake_dataset(
        file_id, "ascat_b_midbeam_index", 2, dims, H5T_NATIVE_UCHAR,
        &ascat_midbeam_index[1][0]);
    H5LTset_attribute_uchar(
        file_id, "ascat_b_midbeam_index", "valid_max", &uchar_valid_max,
        1);
    H5LTset_attribute_uchar(
        file_id, "ascat_b_midbeam_index", "valid_min", &uchar_valid_min,
        1);

    H5LTmake_dataset(
        file_id, "ascat_only_num_ambiguities", 2, dims, H5T_NATIVE_UCHAR,
        &ascat_only_num_ambiguities[0]);
    H5LTset_attribute_uchar(
        file_id, "ascat_only_num_ambiguities", "valid_max", &uchar_valid_max,
        1);
    H5LTset_attribute_uchar(
        file_id, "ascat_only_num_ambiguities", "valid_min", &uchar_valid_min,
        1);

    uint16 u16_value = 65535;
    H5LTmake_dataset(file_id, "flags", 2, dims, H5T_NATIVE_USHORT, &flags[0]);
    H5LTset_attribute_ushort(file_id, "flags", "_FillValue", &u16_value, 1);

    u16_value = 128;
    H5LTset_attribute_ushort(file_id, "flags", "LAND_FLAG", &u16_value, 1);
    u16_value = 256;
    H5LTset_attribute_ushort(file_id, "flags", "ICE_FLAG", &u16_value, 1);
    u16_value = 4096;
    H5LTset_attribute_ushort(file_id, "flags", "RAIN_FLAG_VALID", &u16_value, 1);
    u16_value = 8192;
    H5LTset_attribute_ushort(file_id, "flags", "RAIN_FLAG_RAIN", &u16_value, 1);
    H5Fclose(file_id);

    // free the l2a swaths
    free_array((void *)l2a_ascat_swaths[0], 2, ncti, nati);
    free_array((void *)l2a_ascat_swaths[1], 2, ncti, nati);
    free_array((void *)l2a_scatsat_swath, 2, ncti, nati);
 
    if(!do_sst_gmf) delete gmf;
    return(0);
}

