//==============================================================//
// Copyright (C) 2015-2017, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#define L1B_TB_LORES_ASC_FILE_KEYWORD "L1B_TB_LORES_ASC_FILE"
#define L1B_TB_LORES_DEC_FILE_KEYWORD "L1B_TB_LORES_DEC_FILE"
#define L1B_TB_FILE_KEYWORD "L1B_TB_FILE"
#define QS_LANDMAP_FILE_KEYWORD "QS_LANDMAP_FILE"
#define QS_ICEMAP_FILE_KEYWORD  "QS_ICEMAP_FILE"
#define REV_START_TIME_KEYWORD "REV_START_TIME"
#define REV_STOP_TIME_KEYWORD "REV_STOP_TIME"
#define COASTAL_DISTANCE_FILE_KEYWORD "COASTAL_DISTANCE_FILE"
#define DO_LAND_CORRECTION_KEYWORD "DO_LAND_CORRECTION"
#define SMAP_LAND_FRAC_MAP_FILE_KEYWORD "SMAP_LAND_FRAC_MAP_FILE"
#define SMAP_LAND_NEAR_MAP_FILE_KEYWORD "SMAP_LAND_NEAR_MAP_FILE"
#define DO_ICE_CORRECTION_KEYWORD "DO_ICE_CORRECTION"
#define ICE_CONCECTRATION_FILE_KEYWORD "ICE_CONCECTRATION_FILE"
#define SMAP_ICE_NEAR_MAP_FILE_KEYWORD "SMAP_ICE_NEAR_MAP_FILE"
#define DO_GAL_CORR_KEYWORD "DO_SMAP_TB_GAL_CORR"
#define L1B_TB_ASC_ANC_U10_FILE_KEYWORD "L1B_TB_ASC_ANC_U10_FILE"
#define L1B_TB_DEC_ANC_U10_FILE_KEYWORD "L1B_TB_DEC_ANC_U10_FILE"
#define L1B_TB_ASC_ANC_V10_FILE_KEYWORD "L1B_TB_ASC_ANC_V10_FILE"
#define L1B_TB_DEC_ANC_V10_FILE_KEYWORD "L1B_TB_DEC_ANC_V10_FILE"
#define GAL_CORR_FILE_KEYWORD "SMAP_TB_GAL_CORR_FILE"


//----------//
// INCLUDES //
//----------//

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include "CAPGMF.h"
#include "Misc.h"
#include "Ephemeris.h"
#include "ConfigList.h"
#include "L1A.h"
#include "ConfigSim.h"
#include "L1AToL1B.h"
#include "Tracking.h"
#include "QscatConfig.h"
#include "List.h"
#include "BufferedList.h"
#include "SeaPac.h"
#include "AttenMap.h"
#include "OS2XFix.h"
#include "SMAPLandFracMap.h"
#include "CoastDistance.h"
/* hdf5 include */
#include "hdf5.h"
#include "hdf5_hl.h"


//-----------//
// TEMPLATES //
//-----------//

class AngleInterval;

template class List<AngleInterval>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<StringPair>;
template class List<Meas>;
template class List<EarthPosition>;
template class List<MeasSpot>;
template class List<WindVectorPlus>;
template class List<off_t>;
template class List<OffsetList>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;
template class std::list<string>;
template class std::map<string,string,Options::ltstr>;

//-----------//
// CONSTANTS //
//-----------//

const char* usage_array[] = {"config_file", NULL};

int determine_l1b_sizes(char* l1b_tbfiles[], int nframes[], int nfootprints[]){

    for(int ipart=0; ipart<2; ++ipart) {
        hid_t id = H5Fopen(l1b_tbfiles[ipart], H5F_ACC_RDONLY, H5P_DEFAULT);
        if(id<0) return(0);

        hsize_t dims[2];
        H5T_class_t class_id;
        size_t type_size;

        if(H5LTget_dataset_info(id, "/Brightness_Temperature/tb_h",
            dims, &class_id, &type_size))
            return(0);

        nframes[ipart] = dims[0];
        nfootprints[ipart] = dims[1];
    }
    return(1);
}

int read_SDS_h5(hid_t obj_id, const char* sds_name, void* data_buffer) {
    hid_t sds_id = H5Dopen1(obj_id,sds_name);
    if( sds_id < 0 ) return(0);

    hid_t type_id = H5Dget_type(sds_id);
    if( type_id < 0 ) return(0);

    if( H5Dread( sds_id, type_id, H5S_ALL, H5S_ALL,H5P_DEFAULT, data_buffer) < 0 ||
        H5Dclose( sds_id ) < 0 ) return(0);
    return(1);
}

int init_string( char* string, int length ) {
    for( int ii = 0; ii < length; ++ii )
        string[ii] = NULL;
    return(1);
}


int main(int argc, char* argv[]){
    //-----------//
    // variables //
    //-----------//
    const char* command = no_path(argv[0]);
    char* config_file = argv[1];

    ConfigList config_list;
    if(!config_list.Read(config_file)) {
        fprintf(
            stderr, "%s: error reading config file %s\n", command, config_file);
        exit(1);
    }

    char* l1b_tbfiles[2] = {NULL, NULL};
    // These ones are required
    config_list.ExitForMissingKeywords();
    l1b_tbfiles[0] = config_list.Get(L1B_TB_LORES_ASC_FILE_KEYWORD);
    l1b_tbfiles[1] = config_list.Get(L1B_TB_LORES_DEC_FILE_KEYWORD);
    printf("l1b_tbfiles: %s %s\n", l1b_tbfiles[0], l1b_tbfiles[1]);

    QSLandMap qs_landmap;
    char* qslandmap_file = config_list.Get(QS_LANDMAP_FILE_KEYWORD);
    qs_landmap.Read(qslandmap_file);

    QSIceMap qs_icemap;
    char* qsicemap_file  = config_list.Get(QS_ICEMAP_FILE_KEYWORD);
    qs_icemap.Read(qsicemap_file);

    CoastDistance coast_dist;
    coast_dist.Read(config_list.Get(COASTAL_DISTANCE_FILE_KEYWORD));

    char* ephem_file = config_list.Get(EPHEMERIS_FILE_KEYWORD);
    Ephemeris ephem(ephem_file, 10000);

    int do_land_correction = 0;
    int do_smap_tb_gal_corr = 0;
    int do_ice_correction = 0;
    config_list.DoNothingForMissingKeywords();
    config_list.GetInt(DO_LAND_CORRECTION_KEYWORD, &do_land_correction);
    config_list.GetInt(DO_GAL_CORR_KEYWORD, &do_smap_tb_gal_corr);
    config_list.GetInt(DO_ICE_CORRECTION_KEYWORD, &do_ice_correction);
    config_list.ExitForMissingKeywords();

    SMAPLandTBNearMap smap_land_tb_near_map;
    SMAPLandFracMap smap_land_frac_map;
    if(do_land_correction) {
        smap_land_frac_map.Read(config_list.Get(SMAP_LAND_FRAC_MAP_FILE_KEYWORD));
        smap_land_tb_near_map.Read(
            config_list.Get(SMAP_LAND_NEAR_MAP_FILE_KEYWORD));
    }

    SMAPLandTBNearMap smap_ice_tb_near_map;
    ICECMap icec_map;
    if(do_ice_correction) {
        smap_ice_tb_near_map.Read(
            config_list.Get(SMAP_ICE_NEAR_MAP_FILE_KEYWORD));
        if(!icec_map.Read(
            config_list.Get(ICE_CONCECTRATION_FILE_KEYWORD))) {
            fprintf(stderr, "error reading icecmap\n");
        }
    }

    char* anc_u10_files[2] = {NULL, NULL};
    char* anc_v10_files[2] = {NULL, NULL};
    TBGalCorr tb_gal_corr_map;
    if(do_smap_tb_gal_corr) {
        char* gal_corr_filename = config_list.Get(GAL_CORR_FILE_KEYWORD);
        tb_gal_corr_map.Read(gal_corr_filename);
        anc_u10_files[0] = config_list.Get(L1B_TB_ASC_ANC_U10_FILE_KEYWORD);
        anc_u10_files[1] = config_list.Get(L1B_TB_DEC_ANC_U10_FILE_KEYWORD);
        anc_v10_files[0] = config_list.Get(L1B_TB_ASC_ANC_V10_FILE_KEYWORD);
        anc_v10_files[1] = config_list.Get(L1B_TB_DEC_ANC_V10_FILE_KEYWORD);
    }

    L1B l1b;
    char* output_file = config_list.Get(L1B_TB_FILE_KEYWORD);
    if (l1b.OpenForWriting(output_file) == 0) {
        fprintf(stderr, "%s: cannot open l1b file %s for output\n", command,
            output_file);
        exit(1);
    }

    // Determine number of frames in both portions of orbit
    int nframes[2] = {0, 0};
    int nfootprints[2] = {0, 0};

    if(!determine_l1b_sizes(l1b_tbfiles, nframes, nfootprints)) {
        fprintf(stderr, "Unable to determine array sizes.");
        exit(1);
    }

    ETime etime;
    etime.FromCodeB("1970-001T00:00:00.000");
    char* time_string;
    time_string = config_list.Get(REV_START_TIME_KEYWORD);
    double time_base = (
        (double)etime.GetSec() + (double)etime.GetMs()/1000);
    etime.FromCodeB(time_string);

    double rev_start_time = (
        (double)etime.GetSec()+(double)etime.GetMs()/1000 - time_base);

    time_string = config_list.Get(REV_STOP_TIME_KEYWORD);
    etime.FromCodeB(time_string);
    double rev_stop_time = (
        (double)etime.GetSec()+(double)etime.GetMs()/1000 - time_base);

    double last_frame_time = 0;

    // Iterate over ascending / decending portions of orbit
    for(int ipart = 0; ipart < 2; ++ipart){
        hid_t id = H5Fopen(l1b_tbfiles[ipart], H5F_ACC_RDONLY, H5P_DEFAULT);

        CAP_ANC_L1B cap_anc_u10;
        CAP_ANC_L1B cap_anc_v10;

        if(do_smap_tb_gal_corr) {
            cap_anc_u10.Read(anc_u10_files[ipart]);
            cap_anc_v10.Read(anc_v10_files[ipart]);
        }

        char antenna_scan_time_utc[nframes[ipart]][24];

        std::vector<float> yaw(nframes[ipart]);
        std::vector<float> pitch(nframes[ipart]);
        std::vector<float> roll(nframes[ipart]);

        read_SDS_h5(
            id, "/Spacecraft_Data/antenna_scan_time_utc", &antenna_scan_time_utc[0][0]);
        H5LTread_dataset_float(id, "/Spacecraft_Data/yaw", &yaw[0]);
        H5LTread_dataset_float(id, "/Spacecraft_Data/pitch", &pitch[0]);
        H5LTread_dataset_float(id, "/Spacecraft_Data/roll", &roll[0]);

        std::vector<float> lat;
        std::vector<float> lon;
        std::vector<float> azi;
        std::vector<float> antazi;
        std::vector<float> inc;
        std::vector<float> solar_spec_theta;
        std::vector<float> surface_water_fraction_mb;
        std::vector<float> ra;
        std::vector<float> dec;

        std::vector< std::vector<float> > tb(2);
        std::vector< std::vector<float> > nedt(2);
        std::vector< std::vector<uint16> > tb_flag(2);
        std::vector< std::vector<float> > tb_gal_corr(2);

        int data_size = nframes[ipart]*nfootprints[ipart];
        azi.resize(data_size);
        antazi.resize(data_size);
        lat.resize(data_size);
        lon.resize(data_size);
        ra.resize(data_size);
        dec.resize(data_size);
        inc.resize(data_size);
        solar_spec_theta.resize(data_size);
        surface_water_fraction_mb.resize(data_size);

        // resize arrays for data dimensions
        for(int ipol = 0; ipol < 2; ++ipol) {
            tb[ipol].resize(data_size);
            nedt[ipol].resize(data_size);
            tb_flag[ipol].resize(data_size);
            tb_gal_corr[ipol].resize(data_size);
        }

        // These data only have footprint versions
        read_SDS_h5(id, "/Brightness_Temperature/tb_lat", &lat[0]);
        read_SDS_h5(id, "/Brightness_Temperature/tb_lon", &lon[0]);
        read_SDS_h5(id, "/Brightness_Temperature/specular_right_ascension", &ra[0]);
        read_SDS_h5(id, "/Brightness_Temperature/specular_declination", &dec[0]);
        read_SDS_h5(id, "/Brightness_Temperature/earth_boresight_azimuth", &azi[0]);
        read_SDS_h5(id, "/Brightness_Temperature/antenna_scan_angle", &antazi[0]);
        read_SDS_h5(id, "/Brightness_Temperature/earth_boresight_incidence", &inc[0]);
        read_SDS_h5(
            id, "/Brightness_Temperature/solar_specular_theta",
            &solar_spec_theta[0]);
        read_SDS_h5(
            id, "/Brightness_Temperature/surface_water_fraction_mb_h",
            &surface_water_fraction_mb[0]);

        read_SDS_h5(id, "/Brightness_Temperature/tb_v", &tb[0][0]);
        read_SDS_h5(id, "/Brightness_Temperature/tb_h", &tb[1][0]);

        read_SDS_h5(id, "/Brightness_Temperature/nedt_v", &nedt[0][0]);
        read_SDS_h5(id, "/Brightness_Temperature/nedt_h", &nedt[1][0]);

        read_SDS_h5(id, "/Brightness_Temperature/tb_qual_flag_v", &tb_flag[0][0]);
        read_SDS_h5(id, "/Brightness_Temperature/tb_qual_flag_h", &tb_flag[1][0]);

        read_SDS_h5(
            id, "/Brightness_Temperature/galactic_reflected_correction_v",
            &tb_gal_corr[0][0]);
        read_SDS_h5(
            id, "/Brightness_Temperature/galactic_reflected_correction_h",
            &tb_gal_corr[1][0]);

        // Iterate over scans
        for(int iframe = 0; iframe < nframes[ipart]; ++iframe) {
            l1b.frame.spotList.FreeContents();

            // Copy over time-strings, null terminate and parse
            char time_str[24];
            init_string(time_str, 24);

            // only copy first 23 chars (last one is Z)
            strncpy(time_str, antenna_scan_time_utc[iframe], 23);

            etime.FromCodeA(time_str);

            char* str_month = strtok(time_str, "-");
            str_month = strtok(NULL, "-");
            int this_month = atoi(str_month);

            // Result
            double time = (
                (double)etime.GetSec()+(double)etime.GetMs()/1000 - time_base);

            if(time<rev_start_time || time>rev_stop_time)
                continue;

            // skip overlapping frames from descending side
            if(ipart==1 && time<=last_frame_time)
                continue;

            // Iterate over low-res footprints
            for(int ifootprint = 0; ifootprint < nfootprints[ipart];
                ++ifootprint) {

                // Index into footprint-sized arrays
                int fp_idx = iframe * nfootprints[ipart] + ifootprint;

                if(do_smap_tb_gal_corr == 0 && tb_gal_corr[1][fp_idx] > 3.5)
                    continue;

                if(solar_spec_theta[fp_idx] < 25)
                    continue;

                float this_land_frac = 1 - surface_water_fraction_mb[fp_idx];

                for(int ipol=0; ipol<2; ++ipol){

                    // check flags
                    if(0x1 & tb_flag[ipol][fp_idx])
                        continue;

                   MeasSpot* new_meas_spot = new MeasSpot();

                   // Is this good enough (interpolate within each scan)
                   new_meas_spot->time = time;

                   new_meas_spot->scOrbitState.time = time;

                   ephem.GetOrbitState(
                       time, EPHEMERIS_INTERP_ORDER,
                       &new_meas_spot->scOrbitState);

                   new_meas_spot->scAttitude.SetRPY(
                       dtr*roll[iframe], dtr*pitch[iframe], dtr*yaw[iframe]);

                    Meas* new_meas = new Meas();

                    if(ipol==0)
                        new_meas->measType = Meas::L_BAND_TBV_MEAS_TYPE;
                    else
                        new_meas->measType = Meas::L_BAND_TBH_MEAS_TYPE;

                    new_meas->value = tb[ipol][fp_idx];
                    new_meas->XK = 1.0;

                    double tmp_lon = dtr*lon[fp_idx];
                    double tmp_lat = dtr*lat[fp_idx];
                    if(tmp_lon<0) tmp_lon += two_pi;

                    new_meas->centroid.SetAltLonGDLat(0.0, tmp_lon, tmp_lat);

                    new_meas->incidenceAngle = dtr*inc[fp_idx];
                    new_meas->eastAzimuth = (450.0*dtr - dtr*azi[fp_idx]);
                    new_meas->scanAngle = dtr * (360-antazi[fp_idx]);
                    new_meas->beamIdx = 0;
                    new_meas->numSlices = -1;
                    new_meas->startSliceIdx = -1;
                    new_meas->landFlag = 0;

                    while(new_meas->eastAzimuth>two_pi)
                        new_meas->eastAzimuth-=two_pi;

                    // WAG based on radiometer 3dB fp of 39x47 km
                    new_meas->azimuth_width = 40;
                    new_meas->range_width = 40;

                    // Set land fraction
                    new_meas->bandwidth = this_land_frac;

                    double distance;
                    coast_dist.Get(tmp_lon, tmp_lat, &distance);

                    if(distance < 35)
                        new_meas->landFlag += 1; // bit 0 for land

                    if(qs_icemap.IsIce(tmp_lon, tmp_lat, 0))
                        new_meas->landFlag += 2; // bit 1 for ice


                    // Need to figure out the KP (a, b, c) terms.
                    new_meas->A = pow(nedt[ipol][fp_idx], 2);
                    new_meas->B = 0;
                    new_meas->C = 0;

                    if(do_smap_tb_gal_corr) {

                        float this_anc_spd = 1.03 * sqrt(
                            pow(cap_anc_u10.data[0][iframe][ifootprint], 2) +
                            pow(cap_anc_v10.data[0][iframe][ifootprint], 2));

                        float this_dtg;
                        tb_gal_corr_map.Get(
                            ra[fp_idx], dec[fp_idx], this_anc_spd,
                            new_meas->measType, &this_dtg);

                        // corrected TB (add back in SDS correction, remove my
                        // own estimate).
                        new_meas->value =
                            tb[ipol][fp_idx] + tb_gal_corr[ipol][fp_idx] -
                            this_dtg;

                        // Store galaxy correction value for flagging later
                        new_meas->B = this_dtg;
                    }

                    if(do_land_correction && distance < 1000 & distance > 0) {

                        float land_frac, land_near_value;
                        if(!smap_land_tb_near_map.Get(new_meas, this_month, &land_near_value)||
                           !smap_land_frac_map.Get(new_meas, &land_frac)) {
                            land_frac = 0;
                            land_near_value = 0;
                        }

                        // linearly scale land_frac to exactly zero at 1000
                        if(distance >= 800 & distance <= 1000) {
                            land_frac = land_frac + (
                                (distance-800)/200) * (0-land_frac);
                        }


                        float Tc = 
                            (new_meas->value - land_frac*land_near_value) /
                            (1-land_frac);

                        float dTc_dF = 
                            (new_meas->value - land_near_value) / 
                            pow(1-land_frac, 2);

                        float dTc_dT = 1/(1-land_frac);
                        float dTc_dTland = land_frac/(1-land_frac);

                        float var_T = new_meas->A;
                        float var_Tland = pow(10, 2);
                        float var_F = pow(land_frac/4, 2);

                        float var_Tc = 
                            pow(dTc_dF, 2) * var_F + pow(dTc_dT, 2) * var_T + 
                            pow(dTc_dTland, 2) * var_Tland;

                        // Set corrected values for TB and variance of TB
                        new_meas->bandwidth = land_frac;

                        // only apply correction if non-empty land near value,
                        // otherwise will increase TB not decrease it.
                        if(land_near_value > 0) {
                            new_meas->EnSlice = new_meas->value - Tc;
                            new_meas->value = Tc;
                            new_meas->A = var_Tc;
                        }
                    }

                    if(do_ice_correction) {
                        float ice_frac, ice_near_value;
                        if(!smap_ice_tb_near_map.Get(new_meas, this_month, &ice_near_value)||
                           !icec_map.Get(tmp_lon, tmp_lat, &ice_frac)||
                           ice_frac > 0.5) {
                            ice_frac = 0;
                            ice_near_value = 0;
                        }

                        float Tc = 
                            (new_meas->value - ice_frac*ice_near_value) /
                            (1-ice_frac);

                        float dTc_dF = 
                            (new_meas->value - ice_near_value) / 
                            pow(1-ice_frac, 2);

                        float dTc_dT = 1/(1-ice_frac);
                        float dTc_dTice = ice_frac/(1-ice_frac);

                        float var_T = new_meas->A;
                        float var_Tice = pow(10, 2);
                        float var_F = pow(ice_frac/4, 2);

                        float var_Tc = 
                            pow(dTc_dF, 2) * var_F + pow(dTc_dT, 2) * var_T + 
                            pow(dTc_dTice, 2) * var_Tice;

                        // Set corrected values for TB and variance of TB
                        new_meas->txPulseWidth = ice_frac;

                        // only apply correction if non-empty ice near value,
                        // otherwise will increase TB not decrease it.
                        if(ice_near_value > 0) {
                            new_meas->value = Tc;
                            new_meas->A = var_Tc;
                        }
                    }

                    if(do_land_correction && new_meas->bandwidth > 0.5)
                        delete new_meas;
                    else
                        new_meas_spot->Append(new_meas);

                    if(new_meas_spot->NodeCount() > 0)
                        l1b.frame.spotList.Append(new_meas_spot);
                    else
                        delete new_meas_spot;
                }
            }

            if(l1b.frame.spotList.NodeCount() == 0)
                continue;

            int this_frame = iframe;
            if(ipart==1) this_frame += nframes[0];

            l1b.frame.frame_i              = this_frame;
            l1b.frame.num_l1b_frames       = nframes[0] + nframes[1];
            l1b.frame.num_pulses_per_frame = nfootprints[ipart];
            l1b.frame.num_slices_per_pulse = -1;

            // Write this L1BFrame
            if(!l1b.WriteDataRec()) {
                fprintf(
                    stderr, "%s: writing to %s failed.\n", command, output_file);
                exit(1);
            }

            if(this_frame % 100 == 0){
                printf("Wrote %d of %d frames\n", this_frame,
                    nframes[0] + nframes[1]);
            }
            last_frame_time = time;
        }
        H5Fclose(id);
    }
    return(0);
}
