//==============================================================//
// Copyright (C) 2019, California Institute of Technology.      //
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
#include "Array.h"
#include "Meas.h"
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
    char* l2a_file = argv[1];
    char* l2b_no_filter_file = argv[2];
    char* l2b_dirth_file = argv[3];
    char* out_file = argv[4];
    int optind = 5;
    int is_ascat = 0;
    while((optind < argc) && (argv[optind][0] == '-')) {
        std::string sw = argv[optind];
        if(sw == "--is-ascat") {
            is_ascat = 1;
            printf("ASCAT only\n");
        } else {
            fprintf(stderr,"%s: Unknown option: %s\n", command, sw.c_str());
            exit(1);
        }
        optind++;
    }

    L2A l2a;
    L2B l2b, l2b_nofilt;

    l2a.SetInputFilename(l2a_file);
    l2a.OpenForReading();
    l2a.ReadHeader();

    l2b.SetInputFilename(l2b_dirth_file);
    l2b.OpenForReading();
    l2b.ReadHeader();
    l2b.ReadDataRec();
    l2b.Close();

    l2b_nofilt.SetInputFilename(l2b_no_filter_file);
    l2b_nofilt.OpenForReading();
    l2b_nofilt.ReadHeader();
    l2b_nofilt.ReadDataRec();
    l2b_nofilt.Close();

    int nati = l2a.header.alongTrackBins;
    int ncti = l2a.header.crossTrackBins;
    if (l2b.frame.swath.GetAlongTrackBins() != nati ||
        l2b.frame.swath.GetCrossTrackBins() != ncti ||
        l2b_nofilt.frame.swath.GetAlongTrackBins() != nati ||
        l2b_nofilt.frame.swath.GetCrossTrackBins() != ncti) {
        fprintf(stderr, "Size mismatch!\n");
        exit(1);
    }

    L2AFrame*** l2a_swath;
    l2a_swath = (L2AFrame***)make_array(sizeof(L2AFrame *), 2, ncti, nati);
    while(l2a.ReadDataRec()) {
        L2AFrame* this_frame = new L2AFrame();
        this_frame->CopyFrame(this_frame, &l2a.frame);
         *(*(l2a_swath + this_frame->cti) + this_frame->ati) = this_frame;
    }
    l2a.Close();

    // Output arrays
    int l2b_size = ncti * nati;

    std::vector<float> lat(l2b_size), lon(l2b_size);
    std::vector<float>
        amb_spd(l2b_size*4), amb_dir(l2b_size*4), amb_obj(l2b_size*4);
    std::vector<uint8> num_ambiguities(l2b_size);

    std::vector<float> s0_hh_fore(l2b_size), s0_hh_aft(l2b_size);
    std::vector<float> s0_vv_fore(l2b_size), s0_vv_aft(l2b_size);
    std::vector<float> var_s0_hh_fore(l2b_size), var_s0_hh_aft(l2b_size);
    std::vector<float> var_s0_vv_fore(l2b_size), var_s0_vv_aft(l2b_size);
    std::vector<float> inc_hh_fore(l2b_size), inc_hh_aft(l2b_size);
    std::vector<float> inc_vv_fore(l2b_size), inc_vv_aft(l2b_size);
    std::vector<float> relazi_hh_fore(l2b_size), relazi_hh_aft(l2b_size);
    std::vector<float> relazi_vv_fore(l2b_size), relazi_vv_aft(l2b_size);


    std::vector<float> s0_fore(l2b_size), s0_mid(l2b_size), s0_aft(l2b_size);
    std::vector<float>
        var_s0_fore(l2b_size), var_s0_mid(l2b_size), var_s0_aft(l2b_size);
    std::vector<float> inc_fore(l2b_size), inc_mid(l2b_size), inc_aft(l2b_size);
    std::vector<float>
        relazi_fore(l2b_size), relazi_mid(l2b_size), relazi_aft(l2b_size);


    int num_dirs = 90;
    std::vector<float> best_spd(l2b_size*num_dirs), best_obj(l2b_size*num_dirs);

    for(int ati=0; ati<nati; ++ati) {
        if(ati%200 == 0)
            fprintf(stdout, "%d of %d\n", ati, nati);
        for(int cti=0; cti<ncti; ++cti) {

            int l2bidx = ati + nati*cti;

            // init with fill values
            lat[l2bidx] = FILL_VALUE;
            lon[l2bidx] = FILL_VALUE;
            s0_hh_fore[l2bidx] = FILL_VALUE;
            s0_hh_aft[l2bidx] = FILL_VALUE;
            s0_vv_fore[l2bidx] = FILL_VALUE;
            s0_vv_aft[l2bidx] = FILL_VALUE;
            var_s0_hh_fore[l2bidx] = FILL_VALUE;
            var_s0_hh_aft[l2bidx] = FILL_VALUE;
            var_s0_vv_fore[l2bidx] = FILL_VALUE;
            var_s0_vv_aft[l2bidx] = FILL_VALUE;
            inc_hh_fore[l2bidx] = FILL_VALUE;
            inc_hh_aft[l2bidx] = FILL_VALUE;
            inc_vv_fore[l2bidx] = FILL_VALUE;
            inc_vv_aft[l2bidx] = FILL_VALUE;
            relazi_hh_fore[l2bidx] = FILL_VALUE;
            relazi_hh_aft[l2bidx] = FILL_VALUE;
            relazi_vv_fore[l2bidx] = FILL_VALUE;
            relazi_vv_aft[l2bidx] = FILL_VALUE;
            s0_fore[l2bidx] = FILL_VALUE;
            s0_mid[l2bidx] = FILL_VALUE;
            s0_aft[l2bidx] = FILL_VALUE;
            var_s0_fore[l2bidx] = FILL_VALUE;
            var_s0_mid[l2bidx] = FILL_VALUE;
            var_s0_aft[l2bidx] = FILL_VALUE;
            inc_fore[l2bidx] = FILL_VALUE;
            inc_mid[l2bidx] = FILL_VALUE;
            inc_aft[l2bidx] = FILL_VALUE;
            relazi_fore[l2bidx] = FILL_VALUE;
            relazi_mid[l2bidx] = FILL_VALUE;
            relazi_aft[l2bidx] = FILL_VALUE;

            for(int iamb=0; iamb<4; ++iamb) {
                int ambidx = iamb + 4*l2bidx;
                amb_spd[ambidx] = FILL_VALUE;
                amb_dir[ambidx] = FILL_VALUE;
                amb_obj[ambidx] = FILL_VALUE;
            }

            for(int jj=0; jj<num_dirs; ++jj){
                int ridge_idx = jj + 90*l2bidx;
                best_spd[ridge_idx] = FILL_VALUE;
                best_obj[ridge_idx] = FILL_VALUE;
            }

            WVC* ambig_wvc = l2b_nofilt.frame.swath.GetWVC(cti, ati);
            WVC* dirth_wvc = l2b.frame.swath.GetWVC(cti, ati);

            if(!l2a_swath[cti][ati] || !ambig_wvc || !dirth_wvc)
                continue;

            if(ambig_wvc->directionRanges.dirIdx.GetBins() != num_dirs) {
                printf("Error: wrong number of bins for best spd ridge!\n");
                exit(1);
            }

            int npols = (is_ascat==1) ? 1 : 2;
            int nlks = (is_ascat==1) ? 3 : 2;

            float sum_s0[nlks][npols], sum_s02[nlks][npols], sum_tb[2];
            float sum_cos_azi[nlks][npols], sum_sin_azi[nlks][npols];
            float sum_inc[nlks][npols];
            int cnts[nlks][npols];

            for(int ipol=0; ipol < npols; ++ipol) {
                sum_tb[ipol] = 0;
                for(int ilook=0; ilook < nlks; ++ilook) {
                    sum_s0[ilook][ipol] = 0;
                    sum_s02[ilook][ipol] = 0;
                    sum_cos_azi[ilook][ipol] = 0;
                    sum_sin_azi[ilook][ipol] = 0;
                    sum_inc[ilook][ipol] = 0;
                    cnts[ilook][ipol] = 0;
                }
            }

            MeasList* ml = &(l2a_swath[cti][ati]->measList);

            // compute average values per look
            LonLat lonlat = ml->AverageLonLat();
            lon[l2bidx] = rtd * lonlat.longitude;
            lat[l2bidx] = rtd * lonlat.latitude;

            // wrap to [-180, 180) interval
            while(lon[l2bidx]>=180) lon[l2bidx] -= 360;
            while(lon[l2bidx]<-180) lon[l2bidx] += 360;

            for(Meas* meas = ml->GetHead(); meas; meas = ml->GetNext()) {

                int ilook, ipol;
                if(is_ascat) {
                    ipol = 0;
                    if(meas->beamIdx < 3) {
                        ilook = meas->beamIdx;
                    } else {
                        ilook = meas->beamIdx - 3;
                    }
                } else {
                    ipol = meas->beamIdx;
                    ilook = 0;
                    if(meas->scanAngle > pi/2 && meas->scanAngle < 3*pi/2)
                        ilook = 1;
                }

                sum_tb[ipol] += meas->txPulseWidth;
                sum_s0[ilook][ipol] += meas->value;
                sum_s02[ilook][ipol] += pow(meas->value, 2);
                sum_cos_azi[ilook][ipol] += cos(meas->eastAzimuth);
                sum_sin_azi[ilook][ipol] += sin(meas->eastAzimuth);
                sum_inc[ilook][ipol] += meas->incidenceAngle * 180/M_PI;;
                cnts[ilook][ipol]++;
            }

            float s0[nlks][npols], var_s0[nlks][npols];
            float relazi[nlks][npols], inc[nlks][npols];
            for(int ipol=0; ipol < npols; ++ipol) {
                for(int ilook=0; ilook < nlks; ++ilook) {

                    if(cnts[ilook][ipol] < 2) {
                        s0[ilook][ipol] = FILL_VALUE;
                        var_s0[ilook][ipol] = FILL_VALUE;
                        inc[ilook][ipol] = FILL_VALUE;
                        relazi[ilook][ipol] = FILL_VALUE;
                        continue;
                    }

                    s0[ilook][ipol] =
                        sum_s0[ilook][ipol]/(float)cnts[ilook][ipol];

                    var_s0[ilook][ipol] =
                        sum_s02[ilook][ipol]/(float)cnts[ilook][ipol] -
                        pow(s0[ilook][ipol], 2);

                    var_s0[ilook][ipol] *= (float)cnts[ilook][ipol] /
                        (float)(cnts[ilook][ipol]-1);

                    inc[ilook][ipol] =
                        sum_inc[ilook][ipol]/(float)cnts[ilook][ipol];

                    relazi[ilook][ipol] = pe_rad_to_gs_deg(atan2(
                        sum_sin_azi[ilook][ipol], sum_cos_azi[ilook][ipol]));
                    while(relazi[ilook][ipol]>=180) relazi[ilook][ipol] -= 360;
                    while(relazi[ilook][ipol]<-180) relazi[ilook][ipol] += 360;
                }
            }

            // assign output values
            if(is_ascat) {
                s0_fore[l2bidx] = s0[0][0];
                var_s0_fore[l2bidx] = var_s0[0][0];
                inc_fore[l2bidx] = inc[0][0];
                relazi_fore[l2bidx] = relazi[0][0];

                s0_mid[l2bidx] = s0[1][0];
                var_s0_mid[l2bidx] = var_s0[1][0];
                inc_mid[l2bidx] = inc[1][0];
                relazi_mid[l2bidx] = relazi[1][0];

                s0_aft[l2bidx] = s0[2][0];
                var_s0_aft[l2bidx] = var_s0[2][0];
                inc_aft[l2bidx] = inc[2][0];
                relazi_aft[l2bidx] = relazi[2][0];
            } else {
                s0_hh_fore[l2bidx] = s0[0][0];
                var_s0_hh_fore[l2bidx] = var_s0[0][0];
                inc_hh_fore[l2bidx] = inc[0][0];
                relazi_hh_fore[l2bidx] = relazi[0][0];

                s0_hh_aft[l2bidx] = s0[1][0];
                var_s0_hh_aft[l2bidx] = var_s0[1][0];
                inc_hh_aft[l2bidx] = inc[1][0];
                relazi_hh_aft[l2bidx] = relazi[1][0];

                s0_vv_fore[l2bidx] = s0[0][1];
                var_s0_vv_fore[l2bidx] = var_s0[0][1];
                inc_vv_fore[l2bidx] = inc[0][1];
                relazi_vv_fore[l2bidx] = relazi[0][1];

                s0_vv_aft[l2bidx] = s0[1][1];
                var_s0_vv_aft[l2bidx] = var_s0[1][1];
                inc_vv_aft[l2bidx] = inc[1][1];
                relazi_vv_aft[l2bidx] = relazi[1][1];
            }

            // assign best speed obj function ridges
            for(int ibin = 0; ibin < num_dirs; ++ibin) {
                int ridge_idx = ibin + 90*l2bidx;
                best_spd[ridge_idx] = ambig_wvc->directionRanges.bestSpd[ibin];
                best_obj[ridge_idx] = ambig_wvc->directionRanges.bestObj[ibin];
            }

            // assign l2b ambig related quantities
            num_ambiguities[l2bidx] = ambig_wvc->ambiguities.NodeCount();
            int j_amb = 0;
            for(WindVectorPlus* wvp = ambig_wvc->ambiguities.GetHead(); wvp;
                wvp = ambig_wvc->ambiguities.GetNext()){

                int ambidx = j_amb + 4*l2bidx;
                amb_spd[ambidx] = wvp->spd;
                amb_dir[ambidx] = pe_rad_to_gs_deg(wvp->dir);
                amb_obj[ambidx] = wvp->obj;
                if(amb_dir[ambidx]>180) amb_dir[ambidx] -= 360;
                ++j_amb;
            }
        }
    }

    hid_t file_id = H5Fcreate(
        out_file, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

    float valid_max, valid_min;
    hsize_t dims[3] = {ncti, nati, 4};
    hsize_t dims_amb[3] = {ncti, nati, 4};
    hsize_t dims_ridge[3] = {ncti, nati, 90};

    valid_max = 0; valid_min = -199;
    H5LTmake_dataset(
        file_id, "obj_ridge", 3, dims_ridge, H5T_NATIVE_FLOAT, &best_obj[0]);
    H5LTset_attribute_float(file_id, "obj_ridge", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "obj_ridge", "valid_min", &valid_min, 1);

    valid_max = 100; valid_min = 0;
    H5LTmake_dataset(
        file_id, "spd_ridge", 3, dims_ridge, H5T_NATIVE_FLOAT, &best_spd[0]);
    H5LTset_attribute_float(file_id, "spd_ridge", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "spd_ridge", "valid_min", &valid_min, 1);


    valid_max = 90; valid_min = -90;
    H5LTmake_dataset(file_id, "lat", 2, dims, H5T_NATIVE_FLOAT, &lat[0]);
    H5LTset_attribute_float(file_id, "lat", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "lat", "valid_min", &valid_min, 1);
    H5LTset_attribute_string(file_id, "lat", "units", "degrees_north");

    valid_max = 180; valid_min = -180;
    H5LTmake_dataset(file_id, "lon", 2, dims, H5T_NATIVE_FLOAT, &lon[0]);
    H5LTset_attribute_float(file_id, "lon", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "lon", "valid_min", &valid_min, 1);
    H5LTset_attribute_string(file_id, "lon", "units", "degrees_east");

    valid_max = 100; valid_min = 0;
    H5LTmake_dataset(
        file_id, "amb_spd", 3, dims_amb, H5T_NATIVE_FLOAT, &amb_spd[0]);
    H5LTset_attribute_float(file_id, "amb_spd", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "amb_spd", "valid_min", &valid_min, 1);
    H5LTset_attribute_string(file_id, "amb_spd", "units", "m s-1");

    valid_max = 180; valid_min = -180;
    H5LTmake_dataset(
        file_id, "amb_dir", 3, dims_amb, H5T_NATIVE_FLOAT, &amb_dir[0]);
    H5LTset_attribute_float(file_id, "amb_dir", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "amb_dir", "valid_min", &valid_min, 1);
    H5LTset_attribute_string(file_id, "amb_dir", "units", "degrees");

    valid_max = 0; valid_min = -199;
    H5LTmake_dataset(
        file_id, "amb_obj", 3, dims_amb, H5T_NATIVE_FLOAT, &amb_obj[0]);
    H5LTset_attribute_float(file_id, "amb_obj", "valid_max", &valid_max, 1);
    H5LTset_attribute_float(file_id, "amb_obj", "valid_min", &valid_min, 1);

    if(is_ascat) {
        valid_max = 10; valid_min = -0.01;
        H5LTmake_dataset(
            file_id, "s0_fore", 2, dims, H5T_NATIVE_FLOAT, &s0_fore[0]);
        H5LTset_attribute_float(
            file_id, "s0_fore", "valid_max", &valid_max, 1);
        H5LTset_attribute_float(
            file_id, "s0_fore", "valid_min", &valid_min, 1);

        H5LTmake_dataset(
            file_id, "s0_mid", 2, dims, H5T_NATIVE_FLOAT, &s0_mid[0]);
        H5LTset_attribute_float(
            file_id, "s0_mid", "valid_max", &valid_max, 1);
        H5LTset_attribute_float(
            file_id, "s0_mid", "valid_min", &valid_min, 1);

        H5LTmake_dataset(
            file_id, "s0_aft", 2, dims, H5T_NATIVE_FLOAT, &s0_aft[0]);
        H5LTset_attribute_float(
            file_id, "s0_aft", "valid_max", &valid_max, 1);
        H5LTset_attribute_float(
            file_id, "s0_aft", "valid_min", &valid_min, 1);

        H5LTmake_dataset(
            file_id, "var_s0_fore", 2, dims, H5T_NATIVE_FLOAT, &var_s0_fore[0]);
        H5LTset_attribute_float(
            file_id, "var_s0_fore", "valid_max", &valid_max, 1);
        H5LTset_attribute_float(
            file_id, "var_s0_fore", "valid_min", &valid_min, 1);

        H5LTmake_dataset(
            file_id, "var_s0_mid", 2, dims, H5T_NATIVE_FLOAT, &var_s0_mid[0]);
        H5LTset_attribute_float(
            file_id, "var_s0_mid", "valid_max", &valid_max, 1);
        H5LTset_attribute_float(
            file_id, "var_s0_mid", "valid_min", &valid_min, 1);

        H5LTmake_dataset(
            file_id, "var_s0_aft", 2, dims, H5T_NATIVE_FLOAT, &var_s0_aft[0]);
        H5LTset_attribute_float(
            file_id, "var_s0_aft", "valid_max", &valid_max, 1);
        H5LTset_attribute_float(
            file_id, "var_s0_aft", "valid_min", &valid_min, 1);

        valid_max = 60; valid_min = 20;
        H5LTmake_dataset(
            file_id, "inc_fore", 2, dims, H5T_NATIVE_FLOAT, &inc_fore[0]);
        H5LTset_attribute_float(
            file_id, "inc_fore", "valid_max", &valid_max, 1);
        H5LTset_attribute_float(
            file_id, "inc_fore", "valid_min", &valid_min, 1);

        H5LTmake_dataset(
            file_id, "inc_mid", 2, dims, H5T_NATIVE_FLOAT, &inc_mid[0]);
        H5LTset_attribute_float(
            file_id, "inc_mid", "valid_max", &valid_max, 1);
        H5LTset_attribute_float(
            file_id, "inc_mid", "valid_min", &valid_min, 1);

        H5LTmake_dataset(
            file_id, "inc_aft", 2, dims, H5T_NATIVE_FLOAT, &inc_aft[0]);
        H5LTset_attribute_float(
            file_id, "inc_aft", "valid_max", &valid_max, 1);
        H5LTset_attribute_float(
            file_id, "inc_aft", "valid_min", &valid_min, 1);

        valid_max = 180; valid_min = -180;
        H5LTmake_dataset(
            file_id, "relazi_fore", 2, dims, H5T_NATIVE_FLOAT, &relazi_fore[0]);
        H5LTset_attribute_float(
            file_id, "relazi_fore", "valid_max", &valid_max, 1);
        H5LTset_attribute_float(
            file_id, "relazi_fore", "valid_min", &valid_min, 1);

        H5LTmake_dataset(
            file_id, "relazi_mid", 2, dims, H5T_NATIVE_FLOAT, &relazi_mid[0]);
        H5LTset_attribute_float(
            file_id, "relazi_mid", "valid_max", &valid_max, 1);
        H5LTset_attribute_float(
            file_id, "relazi_mid", "valid_min", &valid_min, 1);

        H5LTmake_dataset(
            file_id, "relazi_aft", 2, dims, H5T_NATIVE_FLOAT, &relazi_aft[0]);
        H5LTset_attribute_float(
            file_id, "relazi_aft", "valid_max", &valid_max, 1);
        H5LTset_attribute_float(
            file_id, "relazi_aft", "valid_min", &valid_min, 1);

    } else {
        valid_max = 10; valid_min = -0.01;
        H5LTmake_dataset(
            file_id, "s0_hh_fore", 2, dims, H5T_NATIVE_FLOAT, &s0_hh_fore[0]);
        H5LTset_attribute_float(
            file_id, "s0_hh_fore", "valid_max", &valid_max, 1);
        H5LTset_attribute_float(
            file_id, "s0_hh_fore", "valid_min", &valid_min, 1);

        H5LTmake_dataset(
            file_id, "s0_hh_aft", 2, dims, H5T_NATIVE_FLOAT, &s0_hh_aft[0]);
        H5LTset_attribute_float(
            file_id, "s0_hh_aft", "valid_max", &valid_max, 1);
        H5LTset_attribute_float(
            file_id, "s0_hh_aft", "valid_min", &valid_min, 1);

        H5LTmake_dataset(
            file_id, "s0_vv_fore", 2, dims, H5T_NATIVE_FLOAT, &s0_vv_fore[0]);
        H5LTset_attribute_float(
            file_id, "s0_vv_fore", "valid_max", &valid_max, 1);
        H5LTset_attribute_float(
            file_id, "s0_vv_fore", "valid_min", &valid_min, 1);

        H5LTmake_dataset(
            file_id, "s0_vv_aft", 2, dims, H5T_NATIVE_FLOAT, &s0_vv_aft[0]);
        H5LTset_attribute_float(
            file_id, "s0_vv_aft", "valid_max", &valid_max, 1);
        H5LTset_attribute_float(
            file_id, "s0_vv_aft", "valid_min", &valid_min, 1);

        H5LTmake_dataset(
            file_id, "var_s0_hh_fore", 2, dims, H5T_NATIVE_FLOAT,
            &var_s0_hh_fore[0]);
        H5LTset_attribute_float(
            file_id, "var_s0_hh_fore", "valid_max", &valid_max, 1);
        H5LTset_attribute_float(
            file_id, "var_s0_hh_fore", "valid_min", &valid_min, 1);

        H5LTmake_dataset(
            file_id, "var_s0_hh_aft", 2, dims, H5T_NATIVE_FLOAT,
            &var_s0_hh_aft[0]);
        H5LTset_attribute_float(
            file_id, "var_s0_hh_aft", "valid_max", &valid_max, 1);
        H5LTset_attribute_float(
            file_id, "var_s0_hh_aft", "valid_min", &valid_min, 1);

        H5LTmake_dataset(
            file_id, "var_s0_vv_fore", 2, dims, H5T_NATIVE_FLOAT,
            &var_s0_vv_fore[0]);
        H5LTset_attribute_float(
            file_id, "var_s0_vv_fore", "valid_max", &valid_max, 1);
        H5LTset_attribute_float(
            file_id, "var_s0_vv_fore", "valid_min", &valid_min, 1);

        H5LTmake_dataset(
            file_id, "var_s0_vv_aft", 2, dims, H5T_NATIVE_FLOAT,
            &var_s0_vv_aft[0]);
        H5LTset_attribute_float(
            file_id, "var_s0_vv_aft", "valid_max", &valid_max, 1);
        H5LTset_attribute_float(
            file_id, "var_s0_vv_aft", "valid_min", &valid_min, 1);

        valid_max = 180; valid_min = -180;
        H5LTmake_dataset(
            file_id, "relazi_hh_fore", 2, dims, H5T_NATIVE_FLOAT,
            &relazi_hh_fore[0]);
        H5LTset_attribute_float(
            file_id, "relazi_hh_fore", "valid_max", &valid_max, 1);
        H5LTset_attribute_float(
            file_id, "relazi_hh_fore", "valid_min", &valid_min, 1);

        H5LTmake_dataset(
            file_id, "relazi_hh_aft", 2, dims, H5T_NATIVE_FLOAT,
            &relazi_hh_aft[0]);
        H5LTset_attribute_float(
            file_id, "relazi_hh_aft", "valid_max", &valid_max, 1);
        H5LTset_attribute_float(
            file_id, "relazi_hh_aft", "valid_min", &valid_min, 1);

        H5LTmake_dataset(
            file_id, "relazi_vv_fore", 2, dims, H5T_NATIVE_FLOAT,
            &relazi_vv_fore[0]);
        H5LTset_attribute_float(
            file_id, "relazi_vv_fore", "valid_max", &valid_max, 1);
        H5LTset_attribute_float(
            file_id, "relazi_vv_fore", "valid_min", &valid_min, 1);

        H5LTmake_dataset(
            file_id, "relazi_vv_aft", 2, dims, H5T_NATIVE_FLOAT,
            &relazi_vv_aft[0]);
        H5LTset_attribute_float(
            file_id, "relazi_vv_aft", "valid_max", &valid_max, 1);
        H5LTset_attribute_float(
            file_id, "relazi_vv_aft", "valid_min", &valid_min, 1);

        valid_max = 60; valid_min = 20;
        H5LTmake_dataset(
            file_id, "inc_hh_fore", 2, dims, H5T_NATIVE_FLOAT,
            &inc_hh_fore[0]);
        H5LTset_attribute_float(
            file_id, "inc_hh_fore", "valid_max", &valid_max, 1);
        H5LTset_attribute_float(
            file_id, "inc_hh_fore", "valid_min", &valid_min, 1);

        H5LTmake_dataset(
            file_id, "inc_hh_aft", 2, dims, H5T_NATIVE_FLOAT,
            &inc_hh_aft[0]);
        H5LTset_attribute_float(
            file_id, "inc_hh_aft", "valid_max", &valid_max, 1);
        H5LTset_attribute_float(
            file_id, "inc_hh_aft", "valid_min", &valid_min, 1);

        H5LTmake_dataset(
            file_id, "inc_vv_fore", 2, dims, H5T_NATIVE_FLOAT,
            &inc_vv_fore[0]);
        H5LTset_attribute_float(
            file_id, "inc_vv_fore", "valid_max", &valid_max, 1);
        H5LTset_attribute_float(
            file_id, "inc_vv_fore", "valid_min", &valid_min, 1);

        H5LTmake_dataset(
            file_id, "inc_vv_aft", 2, dims, H5T_NATIVE_FLOAT,
            &inc_vv_aft[0]);
        H5LTset_attribute_float(
            file_id, "inc_vv_aft", "valid_max", &valid_max, 1);
        H5LTset_attribute_float(
            file_id, "inc_vv_aft", "valid_min", &valid_min, 1);

    }

    return(0);
}

