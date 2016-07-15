//==============================================================//
// Copyright (C) 2015, California Institute of Technology.      //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//
#define L1B_TB_LORES_ASC_FILE_KEYWORD "L1B_TB_LORES_ASC_FILE"
#define L1B_TB_LORES_DEC_FILE_KEYWORD "L1B_TB_LORES_DEC_FILE"
#define SMAP_ANTENNA_PATTERN_FILE_KEYWORD "SMAP_ANTENNA_PATTERN_FILE"
#define REV_START_TIME_KEYWORD "REV_START_TIME"
#define REV_STOP_TIME_KEYWORD "REV_STOP_TIME"
#define COASTAL_DISTANCE_FILE_KEYWORD "COASTAL_DISTANCE_FILE"

//----------//
// INCLUDES //
//----------//

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include "ConfigList.h"
#include "ConfigSim.h"
#include "Misc.h"
#include "Constants.h"
#include "Meas.h"
#include "SMAPBeam.h"
#include "CoastDistance.h"
#include "hdf5.h"
#include "hdf5_hl.h"

const char* usage_array[] = {"config_file", "out_file", NULL};

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
    char* out_file_base = argv[2];

    ConfigList config_list;
    if(!config_list.Read(config_file)) {
        fprintf(
            stderr, "%s: error reading config file %s\n", command, config_file);
        exit(1);
    }

    ETime etime;
    etime.FromCodeB("1970-001T00:00:00.000");
    double time_base = (
        (double)etime.GetSec() + (double)etime.GetMs()/1000);

    etime.FromCodeB(config_list.Get(REV_START_TIME_KEYWORD));
    double rev_start_time = (
        (double)etime.GetSec()+(double)etime.GetMs()/1000 - time_base);

    etime.FromCodeB(config_list.Get(REV_STOP_TIME_KEYWORD));
    double rev_stop_time = (
        (double)etime.GetSec()+(double)etime.GetMs()/1000 - time_base);


    char* l1b_tbfiles[2] = {NULL, NULL};

    // These ones are required
    config_list.ExitForMissingKeywords();
    l1b_tbfiles[0] = config_list.Get(L1B_TB_LORES_ASC_FILE_KEYWORD);
    l1b_tbfiles[1] = config_list.Get(L1B_TB_LORES_DEC_FILE_KEYWORD);

    CoastDistance coast_dist;
    coast_dist.Read(config_list.Get(COASTAL_DISTANCE_FILE_KEYWORD));

    char* smap_ant_patt_file = config_list.Get(SMAP_ANTENNA_PATTERN_FILE_KEYWORD);
    SMAPBeam smap_beam(smap_ant_patt_file);

    char* ephem_file = config_list.Get(EPHEMERIS_FILE_KEYWORD);
    Ephemeris ephem(ephem_file, 10000);

    LandMap land_map;
    if(!ConfigLandMap(&land_map, &config_list)) {
        fprintf(stderr,"%s: Error configuring LandMap\n",command);
        exit(1);
    }


    // Determine number of frames in both portions of orbit
    int nframes[2] = {0, 0};
    int nfootprints[2] = {0, 0};

    if(!determine_l1b_sizes(l1b_tbfiles, nframes, nfootprints)) {
        fprintf(stderr, "Unable to determine array sizes.");
        exit(1);
    }

    int revno;
    config_list.GetInt("REVNO", &revno);

    double delta_grid = 0.1 * dtr;
    int nlon = 360*dtr/delta_grid;
    int nlat = 180*dtr/delta_grid;

    std::vector<EarthPosition> ep_grid;
    std::vector<Vector3> ep_grid_normal;
    std::vector<char> is_land;

    ep_grid.resize(nlon * nlat);
    ep_grid_normal.resize(nlon * nlat);
    is_land.resize(nlon * nlat);

    for(int ilat = 0; ilat < nlat; ++ilat) {
        for(int ilon = 0; ilon < nlon; ++ilon) {

            int idx = ilon + ilat * nlon;
            float this_lat = -pi_over_two + ((float)ilat+0.5) * delta_grid;
            float this_lon = -pi + ((float)ilon+0.5) * delta_grid;

            ep_grid[idx].SetAltLonGDLat(0, this_lon, this_lat);
            ep_grid_normal[idx] = ep_grid[idx].Normal();

            is_land[idx] = land_map.IsLand(this_lon, this_lat);
        }
    }

    // Iterate over ascending / decending portions of orbit
    for(int ipart = 0; ipart < 2; ++ipart){
        hid_t id = H5Fopen(l1b_tbfiles[ipart], H5F_ACC_RDONLY, H5P_DEFAULT);

        char tb_time_utc[nframes[ipart]*nfootprints[ipart]][24];

        read_SDS_h5(
            id, "/Brightness_Temperature/tb_time_utc", &tb_time_utc[0][0]);

        std::vector<float> lat;
        std::vector<float> lon;
        std::vector<float> azi;
        std::vector<float> antazi;
        std::vector<float> antlook;
        std::vector<float> inc;
        std::vector<float> surface_water_fraction_mb;

        std::vector< std::vector<float> > tb(2);
        std::vector< std::vector<float> > land_frac(2);
        std::vector< std::vector<float> > nedt(2);
        std::vector< std::vector<uint16> > tb_flag(2);

        int data_size = nframes[ipart]*nfootprints[ipart];
        azi.resize(data_size);
        antazi.resize(data_size);
        antlook.resize(data_size);
        lat.resize(data_size);
        lon.resize(data_size);
        inc.resize(data_size);
        surface_water_fraction_mb.resize(data_size);

        // resize arrays for data dimensions
        for(int ipol = 0; ipol < 2; ++ipol) {
            tb[ipol].resize(data_size);
            land_frac[ipol].resize(data_size);
            nedt[ipol].resize(data_size);
            tb_flag[ipol].resize(data_size);
        }

        // These data only have footprint versions
        read_SDS_h5(id, "/Brightness_Temperature/tb_lat", &lat[0]);
        read_SDS_h5(id, "/Brightness_Temperature/tb_lon", &lon[0]);
        read_SDS_h5(id, "/Brightness_Temperature/antenna_scan_angle", &antazi[0]);
        read_SDS_h5(id, "/Brightness_Temperature/antenna_look_angle", &antlook[0]);
        read_SDS_h5(id, "/Brightness_Temperature/earth_boresight_incidence", &inc[0]);
        read_SDS_h5(
            id, "/Brightness_Temperature/surface_water_fraction_mb",
            &surface_water_fraction_mb[0]);

        // Iterate over scans
        for(int iframe = 0; iframe < nframes[ipart]; ++iframe) {
            // Iterate over low-res footprints
            for(int ifootprint = 0; ifootprint < nfootprints[ipart];
                ++ifootprint) {
                // Index into footprint-sized arrays
                int fp_idx = iframe * nfootprints[ipart] + ifootprint;

                land_frac[0][fp_idx] = -9999;
                land_frac[1][fp_idx] = -9999;

                if(lat[fp_idx] < -90)
                    continue;

                // Copy over time-strings, null terminate and parse
                char time_str[24];
                init_string(time_str, 24);

                // only copy first 23 chars (last one is Z)
                strncpy(time_str, tb_time_utc[fp_idx], 23);

                etime.FromCodeA(time_str);

                // Result
                double time = (
                    (double)etime.GetSec()+(double)etime.GetMs()/1000 -
                    time_base);

                if(time<rev_start_time || time>rev_stop_time)
                    continue;

                OrbitState orbit_state;
                ephem.GetOrbitState(
                    time, EPHEMERIS_INTERP_ORDER, &orbit_state);

                double tmp_lon = dtr*lon[fp_idx];
                double tmp_lat = dtr*lat[fp_idx];
                if(tmp_lon<0) tmp_lon += two_pi;

                EarthPosition centroid;
                centroid.SetAltLonGDLat(0.0, tmp_lon, tmp_lat);

                Vector3 beam_normal = centroid.Normal();

                // construct approx. antenna beam unit vectors
                Vector3 beam_zhat = centroid - orbit_state.rsat;
                beam_zhat /= beam_zhat.Magnitude();

                Vector3 beam_yhat = beam_zhat & beam_normal;
                beam_yhat /= beam_yhat.Magnitude();

                Vector3 beam_xhat = beam_yhat & beam_zhat;

                double sum_dX_h = 0;
                double land_sum_dX_h = 0;
                double sum_dX_v = 0;
                double land_sum_dX_v = 0;

                float max_dlat = 0;
                float max_dlon = 0;

                double distance;
                coast_dist.Get(tmp_lon, tmp_lat, &distance);

                // Only compute land fraction near coast and not over land.
                if(distance > 500 || distance < -100)
                    continue;

                for(int ilat = 0; ilat < nlat; ++ilat) {
                    float this_lat = 
                        -pi_over_two + ((float)ilat+0.5) * delta_grid;

                    if(fabs(this_lat-tmp_lat)>0.6)
                        continue;

                    float slat = sin(this_lat);
                    float clat = cos(this_lat);
                    float r_local_earth = 
                        (pow(r1_earth, 2) * r2_earth) / 
                        ( pow(r1_earth*clat, 2) + pow(r2_earth*slat, 2) );

                    for(int ilon = 0; ilon < nlon; ++ilon) {

                        float this_lon = -pi + ((float)ilon+0.5) * delta_grid;

                        float this_dlon = this_lon-tmp_lon;
                        if(this_dlon > pi) this_dlon -= two_pi;
                        if(this_dlon < -pi) this_dlon += two_pi;

                        if(clat*fabs(this_dlon)>2)
                            continue;

                        int idx = ilon + ilat * nlon;
                        EarthPosition* this_ep = &ep_grid[idx];
                        Vector3* this_normal = &ep_grid_normal[idx];

                        // x_e_minus_x_sc points to earth from s/c
                        Vector3 x_e_minus_x_sc = *this_ep - orbit_state.rsat;

                        double range = x_e_minus_x_sc.Magnitude();
                        Vector3 this_look = x_e_minus_x_sc;
                        this_look /= range;

                        double this_cos_inc = -(*this_normal % this_look);

                        // if not in visible disk of Earth skip
                        if(this_cos_inc <= 0)
                            continue;

                        if(fabs(this_lat-tmp_lat) > max_dlat)
                            max_dlat = fabs(this_lat-tmp_lat);

                        if(clat*fabs(this_dlon) > max_dlon)
                            max_dlon = clat*fabs(this_dlon);

                        double this_theta = Vector3::AngleBetween(
                            &beam_zhat, &x_e_minus_x_sc);

                        double this_phi = atan2(
                            beam_yhat % x_e_minus_x_sc,
                            beam_xhat % x_e_minus_x_sc);

                        // Compute area on earth
                        double pixel_area = 
                            pow(delta_grid*r_local_earth, 2) * clat;

                        // Compute unit of solid angle
                        double d_omega = pixel_area/pow(range, 2)*this_cos_inc;

                        // Compute gain
                        double gain_h, gain_v;
                        smap_beam.Get(this_theta, this_phi, &gain_h, &gain_v);

                        // Accumulate
                        sum_dX_h += gain_h * d_omega;
                        sum_dX_v += gain_v * d_omega;
                        land_sum_dX_h += (is_land[idx]) ? gain_h * d_omega : 0;
                        land_sum_dX_v += (is_land[idx]) ? gain_v * d_omega : 0;
                    }
                }

                land_frac[0][fp_idx] = land_sum_dX_v / sum_dX_v;
                land_frac[1][fp_idx] = land_sum_dX_h / sum_dX_h;

//                 float this_land_frac = 1 - surface_water_fraction_mb[fp_idx];
//                 printf(
//                     "%s %4.2f, %4.2f %f %f %f %4.2f %4.2f %4.3f %4.3f %4.0f\n",
//                     time_str, lat[fp_idx], lon[fp_idx], land_frac[0][fp_idx],
//                     land_frac[1][fp_idx], this_land_frac, sum_dX_h, sum_dX_v,
//                     max_dlat, max_dlon, distance);

            }
        }
        H5Fclose(id);

        char outfile[1024];
        if(ipart == 0) {
            sprintf(outfile, "%s-asc.dat", out_file_base);
        } else {
            sprintf(outfile, "%s-dec.dat", out_file_base);
        }

        FILE* ofp = fopen(outfile, "w");
        fwrite(&nframes[ipart], sizeof(int), 1, ofp);
        fwrite(&nfootprints[ipart], sizeof(int), 1, ofp);
        fwrite(&land_frac[0][0], sizeof(float), data_size, ofp);
        fwrite(&land_frac[1][0], sizeof(float), data_size, ofp);
        fclose(ofp);
    }
    return(0);
}
