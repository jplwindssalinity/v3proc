//==============================================================//
// Copyright (C) 2017, California Institute of Technology.      //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

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

    ConfigList config_list;
    config_list.Read(config_file);
    config_list.ExitForMissingKeywords();

    GMF gmf;
    ConfigGMF(&gmf, &config_list);

    Kp kp;
    ConfigKp(&kp, &config_list);

    ETime etime;
    etime.FromCodeB("1970-001T00:00:00.000");
    double time_base = (double)etime.GetSec() + (double)etime.GetMs()/1000;

    etime.FromCodeB(config_list.Get("REV_START_TIME"));
    double rev_start =
        (double)etime.GetSec() + (double)etime.GetMs()/1000 - time_base;

    etime.FromCodeB(config_list.Get("REV_STOP_TIME"));
    double rev_stop =
        (double)etime.GetSec() + (double)etime.GetMs()/1000 - time_base;

    char* l2b_file = config_list.Get("L2B_COMBINED_FILE");

    L2A l2a_ascat, l2a_scatsat;
    L2B l2b_scatsat, l2b_scatsat_nofilt;

    l2a_ascat.SetInputFilename(config_list.Get("L2A_ASCAT_FILE"));
    l2a_ascat.OpenForReading();
    l2a_ascat.ReadHeader();

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

    int nati = l2a_ascat.header.alongTrackBins;
    int ncti = l2a_ascat.header.crossTrackBins;

    if (l2a_scatsat.header.alongTrackBins != nati || 
        l2a_scatsat.header.crossTrackBins != ncti || 
        l2b_scatsat.frame.swath.GetAlongTrackBins() != nati ||
        l2b_scatsat.frame.swath.GetCrossTrackBins() != ncti ||
        l2b_scatsat_nofilt.frame.swath.GetAlongTrackBins() != nati ||
        l2b_scatsat_nofilt.frame.swath.GetCrossTrackBins() != ncti) {
        fprintf(stderr, "Size mismatch!\n");
        exit(1);
    }

    // Read in L2A ASCAT file
    L2AFrame*** l2a_ascat_swath;
    l2a_ascat_swath = (L2AFrame***)make_array(sizeof(L2AFrame *), 2, ncti, nati);
    while(l2a_ascat.ReadDataRec()) {
        L2AFrame* this_frame = new L2AFrame();
        this_frame->CopyFrame(this_frame, &l2a_ascat.frame);
         *(*(l2a_ascat_swath + this_frame->cti) + this_frame->ati) = this_frame;
    }
    l2a_ascat.Close();

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

    std::vector<double> row_time(nati);
    std::vector<float> ascat_time_diff(l2b_size);
    std::vector<float> lat(l2b_size), lon(l2b_size);
    std::vector<float> ascat_lat(l2b_size), ascat_lon(l2b_size);


    std::vector<float> anc_spd(l2b_size), anc_dir(l2b_size);
    std::vector<float> scatsat_only_spd(l2b_size), scatsat_only_dir(l2b_size);
    std::vector<float> spd(l2b_size), dir(l2b_size);

    std::vector<float> scatsat_only_ambiguity_spd(l2b_size*4);
    std::vector<float> scatsat_only_ambiguity_dir(l2b_size*4);
    std::vector<float> ambiguity_spd(l2b_size*4);
    std::vector<float> ambiguity_dir(l2b_size*4);
    
    std::vector<uint8> num_ambiguities(l2b_size);
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

    std::vector<float> ascat_sigma0_fore(l2b_size);
    std::vector<float> ascat_sigma0_mid(l2b_size);
    std::vector<float> ascat_sigma0_aft(l2b_size);
    std::vector<float> ascat_var_sigma0_fore(l2b_size);
    std::vector<float> ascat_var_sigma0_mid(l2b_size);
    std::vector<float> ascat_var_sigma0_aft(l2b_size);
    std::vector<float> ascat_inc_fore(l2b_size);
    std::vector<float> ascat_inc_mid(l2b_size);
    std::vector<float> ascat_inc_aft(l2b_size);
    std::vector<float> ascat_azi_fore(l2b_size);
    std::vector<float> ascat_azi_mid(l2b_size);
    std::vector<float> ascat_azi_aft(l2b_size);

    WindSwath wind_swath;
    wind_swath.Allocate(ncti, nati);

    for(int ati=0; ati<nati; ++ati) {
        if(ati%100 == 0)
            fprintf(stdout, "%d of %d\n", ati, nati);

        row_time[ati] = 
            rev_start + (rev_stop-rev_start)*(double)ati/(double)nati;

        for(int cti=0; cti<ncti; ++cti) {

            int l2bidx = ati + nati*cti;

            // initialized datasets
            num_ambiguities[l2bidx] = 255;
            scatsat_only_num_ambiguities[l2bidx] = 255;
            ascat_time_diff[l2bidx] = FILL_VALUE;
            lat[l2bidx] = FILL_VALUE;
            lon[l2bidx] = FILL_VALUE;
            ascat_lon[l2bidx] = FILL_VALUE;
            ascat_lon[l2bidx] = FILL_VALUE;
            anc_spd[l2bidx] = FILL_VALUE;
            anc_dir[l2bidx] = FILL_VALUE;
            scatsat_only_spd[l2bidx] = FILL_VALUE;
            scatsat_only_dir[l2bidx] = FILL_VALUE;
            spd[l2bidx] = FILL_VALUE;
            dir[l2bidx] = FILL_VALUE;

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

            ascat_sigma0_fore[l2bidx] = FILL_VALUE;
            ascat_sigma0_mid[l2bidx] = FILL_VALUE;
            ascat_sigma0_aft[l2bidx] = FILL_VALUE;
            ascat_var_sigma0_fore[l2bidx] = FILL_VALUE;
            ascat_var_sigma0_mid[l2bidx] = FILL_VALUE;
            ascat_var_sigma0_aft[l2bidx] = FILL_VALUE;
            ascat_inc_fore[l2bidx] = FILL_VALUE;
            ascat_inc_mid[l2bidx] = FILL_VALUE;
            ascat_inc_aft[l2bidx] = FILL_VALUE;
            ascat_azi_fore[l2bidx] = FILL_VALUE;
            ascat_azi_mid[l2bidx] = FILL_VALUE;
            ascat_azi_aft[l2bidx] = FILL_VALUE;


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


            MeasList* scatsat_ml = &(l2a_scatsat_swath[cti][ati]->measList);

            LonLat lonlat = scatsat_ml->AverageLonLat();
            lon[l2bidx] = rtd * lonlat.longitude;
            lat[l2bidx] = rtd * lonlat.latitude;

            // wrap to [-180, 180) interval
            while(lon[l2bidx]>=180) lon[l2bidx] -= 360;
            while(lon[l2bidx]<-180) lon[l2bidx] += 360;

            float sum_s0[2][2], sum_s02[2][2];
            float sum_cos_azi[2][2], sum_sin_azi[2][2];
            float sum_inc[2][2];
            int cnts[2][2];

            for(int ilook=0; ilook < 2; ++ilook) {
                for(int ipol=0; ipol < 2; ++ipol) {
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

                sum_s0[ilook][ipol] += meas->value;
                sum_s02[ilook][ipol] += pow(meas->value, 2);
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
                        sum_s02[ilook][ipol]/(float)cnts[ilook][ipol];

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
                        scatsat_sigma0_hh_fore[l2bidx] = this_meas->value;
                        scatsat_var_sigma0_hh_fore[l2bidx] = this_meas->C;
                        scatsat_inc_hh_fore[l2bidx] = 
                            this_meas->incidenceAngle * rtd;
                        scatsat_azi_hh_fore[l2bidx] = this_azi;

                    } else if(ipol == 0 && ilook == 1) {
                        scatsat_sigma0_hh_aft[l2bidx] = this_meas->value;
                        scatsat_var_sigma0_hh_aft[l2bidx] = this_meas->C;
                        scatsat_inc_hh_aft[l2bidx] = 
                            this_meas->incidenceAngle * rtd;
                        scatsat_azi_hh_aft[l2bidx] = this_azi;

                    } else if(ipol == 1 && ilook == 0) {
                        scatsat_sigma0_vv_fore[l2bidx] = this_meas->value;
                        scatsat_var_sigma0_vv_fore[l2bidx] = this_meas->C;
                        scatsat_inc_vv_fore[l2bidx] = 
                            this_meas->incidenceAngle * rtd;
                        scatsat_azi_vv_fore[l2bidx] = this_azi;

                    } else if(ipol == 1 && ilook == 1) {
                        scatsat_sigma0_vv_aft[l2bidx] = this_meas->value;
                        scatsat_var_sigma0_vv_aft[l2bidx] = this_meas->C;
                        scatsat_inc_vv_aft[l2bidx] = 
                            this_meas->incidenceAngle * rtd;
                        scatsat_azi_vv_aft[l2bidx] = this_azi;
                    }
                }
            }

            if(l2a_ascat_swath[cti][ati]) {
                MeasList* ascat_ml = &(l2a_ascat_swath[cti][ati]->measList);

                LonLat lonlat = ascat_ml->AverageLonLat();
                ascat_lon[l2bidx] = rtd * lonlat.longitude;
                ascat_lat[l2bidx] = rtd * lonlat.latitude;

                // wrap to [-180, 180) interval
                while(ascat_lon[l2bidx]>=180) ascat_lon[l2bidx] -= 360;
                while(ascat_lon[l2bidx]<-180) ascat_lon[l2bidx] += 360;


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

                    if(cnts[ibeam]>0) {
                        Meas* this_meas = new Meas();
                        this_meas->value = sum_s0[ibeam]/(float)cnts[ibeam];
                        this_meas->measType = Meas::C_BAND_VV_MEAS_TYPE;
                        this_meas->incidenceAngle =
                            sum_inc[ibeam]/(float)cnts[ibeam];

                        this_meas->eastAzimuth = atan2(
                            sum_sin_azi[ibeam], sum_cos_azi[ibeam]);

                        double this_var = 
                            sum_s02[ibeam]/(float)cnts[ibeam];

                        this_var *= (float)cnts[ibeam]/(float)(cnts[ibeam]-1);

                        this_meas->A = 1;
                        this_meas->B = 0;
                        this_meas->C = this_var;
                        this_meas->numSlices = -1;

                        ml_joint.Append(this_meas);

                        if(ibeam == 0 || ibeam == 3) {
                            ascat_sigma0_fore[l2bidx] = this_meas->value;
                            ascat_var_sigma0_fore[l2bidx] = this_meas->C;
                            ascat_inc_fore[l2bidx] = 
                                this_meas->incidenceAngle * rtd;
                            ascat_azi_fore[l2bidx] = pe_rad_to_gs_deg(
                                this_meas->eastAzimuth);

                        } else if(ibeam == 1 || ibeam == 4) {
                            ascat_sigma0_mid[l2bidx] = this_meas->value;
                            ascat_var_sigma0_mid[l2bidx] = this_meas->C;
                            ascat_inc_mid[l2bidx] = 
                                this_meas->incidenceAngle * rtd;
                            ascat_azi_mid[l2bidx] = pe_rad_to_gs_deg(
                                this_meas->eastAzimuth);

                        } else if(ibeam == 2 || ibeam == 5) {
                            ascat_sigma0_aft[l2bidx] = this_meas->value;
                            ascat_var_sigma0_aft[l2bidx] = this_meas->C;
                            ascat_inc_aft[l2bidx] = 
                                this_meas->incidenceAngle * rtd;
                            ascat_azi_aft[l2bidx] = pe_rad_to_gs_deg(
                                this_meas->eastAzimuth);
                        }
                    }
                }
                ascat_time_diff[l2bidx] = (
                    sum_time/(float)cnts_time - row_time[ati])/60;
            }


            WVC* wvc = new WVC();
            gmf.RetrieveWinds_S3(&ml_joint, &kp, wvc);
//             gmf.RetrieveWinds_S3(scatsat_ml, &kp, wvc);

            if(wvc->ambiguities.NodeCount() > 0) {

                // Nudge it with the SCATSAT only DIRTH wind direction
                wvc->selected = wvc->GetNearestToDirection(
                    ku_dirth_wvc->selected->dir);
                wind_swath.Add(cti, ati, wvc);
            } else
                delete wvc;
        }
    }
    // Nudge
//     WindField nudge_field;
//     nudge_field.ReadType(
//         config_list.Get("NUDGE_WINDFIELD_FILE"),
//         config_list.Get("NUDGE_WINDFIELD_TYPE"));
// 
//     int max_rank_for_nudging;
//     config_list.GetInt("MAX_RANK_FOR_NUDGING", &max_rank_for_nudging);
// 
//     float nudge_thresholds[2];
//     config_list.GetFloat("NEAR_SWATH_NUDGE_THRESHOLD", &nudge_thresholds[0]);
//     config_list.GetFloat("FAR_SWATH_NUDGE_THRESHOLD", &nudge_thresholds[1]);
// 
//     wind_swath.GetNudgeVectors(&nudge_field);
//     wind_swath.ThresNudge(max_rank_for_nudging, nudge_thresholds);

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

            spd[l2bidx] = wvc->selected->spd;
            dir[l2bidx] = pe_rad_to_gs_deg(wvc->selected->dir);
            if(dir[l2bidx]>180) dir[l2bidx] -= 360;
        }
    }


    hid_t file_id = H5Fcreate(
        l2b_file, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

   float valid_max, valid_min;
    hsize_t dims[3] = {ncti, nati, 4};
    hsize_t dims_amb[3] = {ncti, nati, 4};

    H5LTmake_dataset(
        file_id, "row_time", 1, dims, H5T_NATIVE_FLOAT,
        &row_time[0]);

    valid_max = 90; valid_min = 0;
    H5LTmake_dataset(
        file_id, "ascat_time_diff", 2, dims, H5T_NATIVE_FLOAT,
        &ascat_time_diff[0]);
    H5LTset_attribute_float(
        file_id, "ascat_time_diff", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ascat_time_diff", "valid_min", &valid_min, 1);


    valid_max = 90; valid_min = -90;
    H5LTmake_dataset(file_id, "lat", 2, dims, H5T_NATIVE_FLOAT, &lat[0]);
    H5LTset_attribute_float(file_id, "lat", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "lat", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "ascat_lat", 2, dims, H5T_NATIVE_FLOAT, &ascat_lat[0]);
    H5LTset_attribute_float(file_id, "ascat_lat", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "ascat_lat", "valid_min", &valid_min, 1);

    valid_max = 180; valid_min = -180;
    H5LTmake_dataset(file_id, "lon", 2, dims, H5T_NATIVE_FLOAT, &lon[0]);
    H5LTset_attribute_float(file_id, "lon", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "lon", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "ascat_lon", 2, dims, H5T_NATIVE_FLOAT, &ascat_lon[0]);
    H5LTset_attribute_float(file_id, "ascat_lon", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "ascat_lon", "valid_min", &valid_min, 1);

    valid_max = 100; valid_min = 0;
    H5LTmake_dataset(file_id, "anc_spd", 2, dims, H5T_NATIVE_FLOAT, &anc_spd[0]);
    H5LTset_attribute_float(file_id, "anc_spd", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "anc_spd", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "scatsat_only_spd", 2, dims, H5T_NATIVE_FLOAT, 
        &scatsat_only_spd[0]);
    H5LTset_attribute_float(
        file_id, "scatsat_only_spd", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "scatsat_only_spd", "valid_min", &valid_min, 1);

    H5LTmake_dataset(file_id, "spd", 2, dims, H5T_NATIVE_FLOAT, &spd[0]);
    H5LTset_attribute_float(file_id, "spd", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "spd", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "scatsat_only_ambiguity_spd", 3, dims, H5T_NATIVE_FLOAT,
        &scatsat_only_ambiguity_spd[0]);
    H5LTset_attribute_float(
        file_id, "scatsat_only_ambiguity_spd", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "scatsat_only_ambiguity_spd", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "ambiguity_spd", 3, dims, H5T_NATIVE_FLOAT, &ambiguity_spd[0]);
    H5LTset_attribute_float(
        file_id, "ambiguity_spd", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ambiguity_spd", "valid_min", &valid_min, 1);


    valid_max = 180; valid_min = -180;
    H5LTmake_dataset(
        file_id, "anc_dir", 2, dims, H5T_NATIVE_FLOAT, &anc_dir[0]);
    H5LTset_attribute_float(file_id, "anc_dir", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "anc_dir", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "scatsat_only_dir", 2, dims, H5T_NATIVE_FLOAT,
        &scatsat_only_dir[0]);
    H5LTset_attribute_float(
        file_id, "scatsat_only_dir", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "scatsat_only_dir", "valid_min", &valid_min, 1);

    H5LTmake_dataset(file_id, "dir", 2, dims, H5T_NATIVE_FLOAT, &dir[0]);
    H5LTset_attribute_float(file_id, "dir", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "dir", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "ambiguity_dir", 3, dims, H5T_NATIVE_FLOAT,
        &ambiguity_dir[0]);
    H5LTset_attribute_float(
        file_id, "ambiguity_dir", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ambiguity_dir", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "scatsat_only_ambiguity_dir", 3, dims, H5T_NATIVE_FLOAT,
        &scatsat_only_ambiguity_dir[0]);
    H5LTset_attribute_float(
        file_id, "scatsat_only_ambiguity_dir", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "scatsat_only_ambiguity_dir", "valid_min", &valid_min, 1);

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
        file_id, "ascat_sigma0_fore", 2, dims, H5T_NATIVE_FLOAT,
        &ascat_sigma0_fore[0]);
    H5LTset_attribute_float(
        file_id, "ascat_sigma0_fore", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ascat_sigma0_fore", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "ascat_sigma0_mid", 2, dims, H5T_NATIVE_FLOAT,
        &ascat_sigma0_mid[0]);
    H5LTset_attribute_float(
        file_id, "ascat_sigma0_mid", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ascat_sigma0_mid", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "ascat_sigma0_aft", 2, dims, H5T_NATIVE_FLOAT,
        &ascat_sigma0_aft[0]);
    H5LTset_attribute_float(
        file_id, "ascat_sigma0_aft", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ascat_sigma0_aft", "valid_min", &valid_min, 1);


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
        file_id, "ascat_var_sigma0_fore", 2, dims, H5T_NATIVE_FLOAT,
        &ascat_var_sigma0_fore[0]);
    H5LTset_attribute_float(
        file_id, "ascat_var_sigma0_fore", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ascat_var_sigma0_fore", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "ascat_var_sigma0_mid", 2, dims, H5T_NATIVE_FLOAT,
        &ascat_var_sigma0_mid[0]);
    H5LTset_attribute_float(
        file_id, "ascat_var_sigma0_mid", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ascat_var_sigma0_mid", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "ascat_var_sigma0_aft", 2, dims, H5T_NATIVE_FLOAT,
        &ascat_var_sigma0_aft[0]);
    H5LTset_attribute_float(
        file_id, "ascat_var_sigma0_aft", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ascat_var_sigma0_aft", "valid_min", &valid_min, 1);


    valid_max = 90; valid_min = 0;
    H5LTmake_dataset(
        file_id, "scatsat_inc_hh_fore", 2, dims, H5T_NATIVE_FLOAT,
        &scatsat_inc_hh_fore[0]);
    H5LTset_attribute_float(
        file_id, "scatsat_inc_hh_fore", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "scatsat_inc_hh_fore", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "scatsat_inc_hh_aft", 2, dims, H5T_NATIVE_FLOAT,
        &scatsat_inc_hh_aft[0]);
    H5LTset_attribute_float(
        file_id, "scatsat_inc_hh_aft", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "scatsat_inc_hh_aft", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "scatsat_inc_vv_fore", 2, dims, H5T_NATIVE_FLOAT,
        &scatsat_inc_vv_fore[0]);
    H5LTset_attribute_float(
        file_id, "scatsat_inc_vv_fore", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "scatsat_inc_vv_fore", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "scatsat_inc_vv_aft", 2, dims, H5T_NATIVE_FLOAT,
        &scatsat_inc_vv_aft[0]);
    H5LTset_attribute_float(
        file_id, "scatsat_inc_vv_aft", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "scatsat_inc_vv_aft", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "ascat_inc_fore", 2, dims, H5T_NATIVE_FLOAT,
        &ascat_inc_fore[0]);
    H5LTset_attribute_float(
        file_id, "ascat_inc_fore", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ascat_inc_fore", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "ascat_inc_mid", 2, dims, H5T_NATIVE_FLOAT,
        &ascat_inc_mid[0]);
    H5LTset_attribute_float(
        file_id, "ascat_inc_mid", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ascat_inc_mid", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "ascat_inc_aft", 2, dims, H5T_NATIVE_FLOAT,
        &ascat_inc_aft[0]);
    H5LTset_attribute_float(
        file_id, "ascat_inc_aft", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ascat_inc_aft", "valid_min", &valid_min, 1);


    valid_max = 180; valid_min = -180;
    H5LTmake_dataset(
        file_id, "scatsat_azi_hh_fore", 2, dims, H5T_NATIVE_FLOAT,
        &scatsat_azi_hh_fore[0]);
    H5LTset_attribute_float(
        file_id, "scatsat_azi_hh_fore", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "scatsat_azi_hh_fore", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "scatsat_azi_hh_aft", 2, dims, H5T_NATIVE_FLOAT,
        &scatsat_azi_hh_aft[0]);
    H5LTset_attribute_float(
        file_id, "scatsat_azi_hh_aft", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "scatsat_azi_hh_aft", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "scatsat_azi_vv_fore", 2, dims, H5T_NATIVE_FLOAT,
        &scatsat_azi_vv_fore[0]);
    H5LTset_attribute_float(
        file_id, "scatsat_azi_vv_fore", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "scatsat_azi_vv_fore", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "scatsat_azi_vv_aft", 2, dims, H5T_NATIVE_FLOAT,
        &scatsat_azi_vv_aft[0]);
    H5LTset_attribute_float(
        file_id, "scatsat_azi_vv_aft", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "scatsat_azi_vv_aft", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "ascat_azi_fore", 2, dims, H5T_NATIVE_FLOAT,
        &ascat_azi_fore[0]);
    H5LTset_attribute_float(
        file_id, "ascat_azi_fore", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ascat_azi_fore", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "ascat_azi_mid", 2, dims, H5T_NATIVE_FLOAT,
        &ascat_azi_mid[0]);
    H5LTset_attribute_float(
        file_id, "ascat_azi_mid", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ascat_azi_mid", "valid_min", &valid_min, 1);

    H5LTmake_dataset(
        file_id, "ascat_azi_aft", 2, dims, H5T_NATIVE_FLOAT,
        &ascat_azi_aft[0]);
    H5LTset_attribute_float(
        file_id, "ascat_azi_aft", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(
        file_id, "ascat_azi_aft", "valid_min", &valid_min, 1);

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

    H5Fclose(file_id);

    // free the l2a swaths
    free_array((void *)l2a_ascat_swath, 2, ncti, nati);
    free_array((void *)l2a_scatsat_swath, 2, ncti, nati);
    return(0);
}

