//==============================================================//
// Copyright (C) 2015, California Institute of Technology.      //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//
#define L1B_TB_LORES_ASC_FILE_KEYWORD "L1B_TB_LORES_ASC_FILE"
#define L1B_TB_LORES_DEC_FILE_KEYWORD "L1B_TB_LORES_DEC_FILE"
#define TB_FLAT_MODEL_FILE_KEYWORD "TB_FLAT_MODEL_FILE"
#define TB_ROUGH_MODEL_FILE_KEYWORD "TB_ROUGH_MODEL_FILE"
#define QS_LANDMAP_FILE_KEYWORD "QS_LANDMAP_FILE"
#define QS_ICEMAP_FILE_KEYWORD  "QS_ICEMAP_FILE"
#define L1B_TB_ASC_ANC_U10_FILE_KEYWORD "L1B_TB_ASC_ANC_U10_FILE"
#define L1B_TB_ASC_ANC_V10_FILE_KEYWORD "L1B_TB_ASC_ANC_V10_FILE"
#define L1B_TB_ASC_ANC_SSS_FILE_KEYWORD "L1B_TB_ASC_ANC_SSS_FILE"
#define L1B_TB_ASC_ANC_SST_FILE_KEYWORD "L1B_TB_ASC_ANC_SST_FILE"
#define L1B_TB_ASC_ANC_SWH_FILE_KEYWORD "L1B_TB_ASC_ANC_SWH_FILE"
#define L1B_TB_DEC_ANC_U10_FILE_KEYWORD "L1B_TB_DEC_ANC_U10_FILE"
#define L1B_TB_DEC_ANC_V10_FILE_KEYWORD "L1B_TB_DEC_ANC_V10_FILE"
#define L1B_TB_DEC_ANC_SSS_FILE_KEYWORD "L1B_TB_DEC_ANC_SSS_FILE"
#define L1B_TB_DEC_ANC_SST_FILE_KEYWORD "L1B_TB_DEC_ANC_SST_FILE"
#define L1B_TB_DEC_ANC_SWH_FILE_KEYWORD "L1B_TB_DEC_ANC_SWH_FILE"
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
#include "Misc.h"
#include "Constants.h"
#include "Array.h"
#include "Meas.h"
#include "CAPGMF.h"
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
    char* out_file = argv[2];

    int do_lr = 0;
    if(argc > 3) {
        std::string sw = argv[3];
        if(sw == "--do-left-right") do_lr = 1;
    }

    printf("do lr: %d\n", do_lr);

    ConfigList config_list;
    if(!config_list.Read(config_file)) {
        fprintf(
            stderr, "%s: error reading config file %s\n", command, config_file);
        exit(1);
    }

    QSLandMap qs_landmap;
    char* qslandmap_file = config_list.Get(QS_LANDMAP_FILE_KEYWORD);
    qs_landmap.Read(qslandmap_file);

    QSIceMap qs_icemap;
    char* qsicemap_file  = config_list.Get(QS_ICEMAP_FILE_KEYWORD);
    qs_icemap.Read(qsicemap_file);

    CoastDistance coast_dist;
    coast_dist.Read(config_list.Get(COASTAL_DISTANCE_FILE_KEYWORD));

    char* l1b_tbfiles[2] = {NULL, NULL};

    // These ones are required
    config_list.ExitForMissingKeywords();
    l1b_tbfiles[0] = config_list.Get(L1B_TB_LORES_ASC_FILE_KEYWORD);
    l1b_tbfiles[1] = config_list.Get(L1B_TB_LORES_DEC_FILE_KEYWORD);

    char* anc_u10_files[2] = {NULL, NULL};
    anc_u10_files[0] = config_list.Get(L1B_TB_ASC_ANC_U10_FILE_KEYWORD);
    anc_u10_files[1] = config_list.Get(L1B_TB_DEC_ANC_U10_FILE_KEYWORD);

    char* anc_v10_files[2] = {NULL, NULL};
    anc_v10_files[0] = config_list.Get(L1B_TB_ASC_ANC_V10_FILE_KEYWORD);
    anc_v10_files[1] = config_list.Get(L1B_TB_DEC_ANC_V10_FILE_KEYWORD);

    char* anc_sss_files[2] = {NULL, NULL};
    anc_sss_files[0] = config_list.Get(L1B_TB_ASC_ANC_SSS_FILE_KEYWORD);
    anc_sss_files[1] = config_list.Get(L1B_TB_DEC_ANC_SSS_FILE_KEYWORD);

    char* anc_sst_files[2] = {NULL, NULL};
    anc_sst_files[0] = config_list.Get(L1B_TB_ASC_ANC_SST_FILE_KEYWORD);
    anc_sst_files[1] = config_list.Get(L1B_TB_DEC_ANC_SST_FILE_KEYWORD);

    char* anc_swh_files[2] = {NULL, NULL};
    anc_swh_files[0] = config_list.Get(L1B_TB_ASC_ANC_SWH_FILE_KEYWORD);
    anc_swh_files[1] = config_list.Get(L1B_TB_DEC_ANC_SWH_FILE_KEYWORD);

    char* tb_flat_file = config_list.Get(TB_FLAT_MODEL_FILE_KEYWORD);
    char* tb_rough_file = config_list.Get(TB_ROUGH_MODEL_FILE_KEYWORD);

    // Determine number of frames in both portions of orbit
    int nframes[2] = {0, 0};
    int nfootprints[2] = {0, 0};

    if(!determine_l1b_sizes(l1b_tbfiles, nframes, nfootprints)) {
        fprintf(stderr, "Unable to determine array sizes.");
        exit(1);
    }

    CAPGMF cap_gmf;
    cap_gmf.ReadFlat(tb_flat_file);
    cap_gmf.ReadRough(tb_rough_file);

    float sum_dtb[2] = {0, 0};
    unsigned int counts[2] = {0, 0};

    float sum_dtb_fore_asc[2] = {0, 0};
    unsigned int counts_fore_asc[2] = {0, 0};
    float sum_dtb_fore_dec[2] = {0, 0};
    unsigned int counts_fore_dec[2] = {0, 0};

    float sum_dtb_aft_asc[2] = {0, 0};
    unsigned int counts_aft_asc[2] = {0, 0};
    float sum_dtb_aft_dec[2] = {0, 0};
    unsigned int counts_aft_dec[2] = {0, 0};

    // Iterate over ascending / decending portions of orbit
    for(int ipart = 0; ipart < 2; ++ipart){
        hid_t id = H5Fopen(l1b_tbfiles[ipart], H5F_ACC_RDONLY, H5P_DEFAULT);

        std::vector<float> lat;
        std::vector<float> lon;
        std::vector<float> azi;
        std::vector<float> antazi;
        std::vector<float> inc;
        std::vector<float> solar_spec_theta;

        std::vector< std::vector<float> > tb(2);
        std::vector< std::vector<float> > ta(2);
        std::vector< std::vector<float> > ta_f(2);
        std::vector< std::vector<uint16> > tb_flag(2);

        int data_size = nframes[ipart]*nfootprints[ipart];
        azi.resize(data_size);
        antazi.resize(data_size);
        lat.resize(data_size);
        lon.resize(data_size);
        inc.resize(data_size);
        solar_spec_theta.resize(data_size);

        // resize arrays for data dimensions
        for(int ipol = 0; ipol < 2; ++ipol) {
            tb[ipol].resize(data_size);
            ta[ipol].resize(data_size);
            ta_f[ipol].resize(data_size);
            tb_flag[ipol].resize(data_size);
        }

        // These data only have footprint versions
        read_SDS_h5(id, "/Brightness_Temperature/tb_lat", &lat[0]);
        read_SDS_h5(id, "/Brightness_Temperature/tb_lon", &lon[0]);
        read_SDS_h5(id, "/Brightness_Temperature/earth_boresight_azimuth", &azi[0]);
        read_SDS_h5(id, "/Brightness_Temperature/antenna_scan_angle", &antazi[0]);
        read_SDS_h5(id, "/Brightness_Temperature/earth_boresight_incidence", &inc[0]);
        read_SDS_h5(
            id, "/Brightness_Temperature/solar_specular_theta",
            &solar_spec_theta[0]);

        read_SDS_h5(id, "/Brightness_Temperature/tb_v", &tb[0][0]);
        read_SDS_h5(id, "/Brightness_Temperature/tb_h", &tb[1][0]);

        read_SDS_h5(id, "/Brightness_Temperature/ta_v", &ta[0][0]);
        read_SDS_h5(id, "/Brightness_Temperature/ta_h", &ta[1][0]);

        read_SDS_h5(id, "/Brightness_Temperature/ta_filtered_v", &ta_f[0][0]);
        read_SDS_h5(id, "/Brightness_Temperature/ta_filtered_h", &ta_f[1][0]);

        read_SDS_h5(id, "/Brightness_Temperature/tb_qual_flag_v", &tb_flag[0][0]);
        read_SDS_h5(id, "/Brightness_Temperature/tb_qual_flag_h", &tb_flag[1][0]);



        CAP_ANC_L1B anc_u10(anc_u10_files[ipart]);
        CAP_ANC_L1B anc_v10(anc_v10_files[ipart]);
        CAP_ANC_L1B anc_sss(anc_sss_files[ipart]);
        CAP_ANC_L1B anc_sst(anc_sst_files[ipart]);
        CAP_ANC_L1B anc_swh(anc_swh_files[ipart]);

        // ensure use same array size
        if(anc_u10.nframes != nframes[ipart] ||
           anc_u10.nfootprints != nfootprints[ipart] ||
           anc_v10.nframes != nframes[ipart] ||
           anc_v10.nfootprints != nfootprints[ipart] ||
           anc_sst.nframes != nframes[ipart] ||
           anc_sst.nfootprints != nfootprints[ipart] ||
           anc_sss.nframes != nframes[ipart] ||
           anc_sss.nfootprints != nfootprints[ipart] ||
           anc_swh.nframes != nframes[ipart] ||
           anc_swh.nfootprints != nfootprints[ipart])
            continue;

        // Iterate over scans
        for(int iframe = 0; iframe < nframes[ipart]; ++iframe) {
            // Iterate over low-res footprints
            for(int ifootprint = 0; ifootprint < nfootprints[ipart];
                ++ifootprint) {
                // Index into footprint-sized arrays
                int fp_idx = iframe * nfootprints[ipart] + ifootprint;
                for(int ipol=0; ipol<2; ++ipol){

                    // check flags
                    if(0x1 & tb_flag[ipol][fp_idx])
                        continue;

                    if(lat[fp_idx] < -90)
                        continue;

                    double tmp_lon = dtr*lon[fp_idx];
                    double tmp_lat = dtr*lat[fp_idx];
                    if(tmp_lon<0) tmp_lon += two_pi;

                    if(qs_landmap.IsLand(tmp_lon, tmp_lat, 0) ||
                       qs_icemap.IsIce(tmp_lon, tmp_lat, 0))
                        continue;

                    double distance;
                    coast_dist.Get(tmp_lon, tmp_lat, &distance);

                    Meas::MeasTypeE met;
                    if(ipol==0)
                        met = Meas::L_BAND_TBV_MEAS_TYPE;
                    else
                        met = Meas::L_BAND_TBH_MEAS_TYPE;

                    float u10 = anc_u10.data[0][iframe][ifootprint];
                    float v10 = anc_v10.data[0][iframe][ifootprint];

                    float spd = 1.03 * sqrt(u10*u10 + v10*v10);

                    // Met convention
                    float dir = atan2(-u10, -v10);

                    float sss = anc_sss.data[0][iframe][ifootprint];
                    float sst = anc_sst.data[0][iframe][ifootprint] + 273.16;
                    float swh = anc_swh.data[0][iframe][ifootprint];
                    if(swh > 10)
                        swh = -99999;

                    float this_inc = dtr*inc[fp_idx];

                    // Compute relative azimuth angle
                    float relazi = azi[fp_idx] - dir;
                    relazi *= pi/180.0;
                    relazi -= pi;
                    while(relazi<0) relazi += two_pi;
                    while(relazi>=two_pi) relazi -= two_pi;

                    float model_tb, tb_flat, dtb;
                    cap_gmf.GetTB(
                        met, this_inc, sst, sss, spd, relazi, swh, &tb_flat,
                        &dtb);

                    model_tb = tb_flat + dtb;

                    float thresh_tb = (ipol == 0) ? 200 : 150;

                    // Check things before adding to delta TB accumulation
                    if(qs_landmap.IsLand(tmp_lon, tmp_lat, 0) ||
                       qs_icemap.IsIce(tmp_lon, tmp_lat, 0) ||
                       distance < 500 ||
                       spd < 0 || spd > 15 ||
                       sst < 278.16 || sst > 373 ||
                       fabs(inc[fp_idx]-40) > 0.5 ||
                       fabs(ta[ipol][fp_idx]-ta_f[ipol][fp_idx]) > 1 ||
                       fabs(lat[fp_idx]) > 50 ||
                       model_tb < 65 || model_tb > 125 ||
                       tb[ipol][fp_idx] > thresh_tb ||
                       solar_spec_theta[fp_idx] < 25)
                        continue;

                    float this_delta = tb[ipol][fp_idx] - model_tb;

                    // Accumulate delta tb
                    sum_dtb[ipol] += this_delta;
                    counts[ipol] += 1;

                    int is_fore = (antazi[fp_idx] < 90 || antazi[fp_idx] > 270) ? 1 : 0;

                    if(do_lr)
                        is_fore = (
                            antazi[fp_idx] >= 0 && antazi[fp_idx] < 180) ? 1 : 0;

                    int is_asc = (ipart == 0) ? 1 : 0;

                    // Accumulate into fore/aft x asc/dec seperatly also
                    if(is_asc) {
                        if(is_fore) {
                            sum_dtb_fore_asc[ipol] += this_delta;
                            counts_fore_asc[ipol] += 1;
                        } else {
                            sum_dtb_aft_asc[ipol] += this_delta;
                            counts_aft_asc[ipol] += 1;
                        }
                    } else {
                        if(is_fore) {
                            sum_dtb_fore_dec[ipol] += this_delta;
                            counts_fore_dec[ipol] += 1;
                        } else {
                            sum_dtb_aft_dec[ipol] += this_delta;
                            counts_aft_dec[ipol] += 1;
                        }
                    }
                }
            }
        }
        H5Fclose(id);
    }

    int revno;
    config_list.GetInt("REVNO", &revno);

    FILE* ofp = fopen(out_file, "w");
    fprintf(
        ofp, "%d, %d, %f, %d, %f, %d, %f, %d, %f, %d, %f, %d, %f, %d, %f, %d, %f, %d, %f, %d, %f\n",
        revno,
        counts[0], sum_dtb[0]/(float)counts[0],
        counts[1], sum_dtb[1]/(float)counts[1],
        counts_fore_asc[0], sum_dtb_fore_asc[0]/(float)counts_fore_asc[0],
        counts_fore_asc[1], sum_dtb_fore_asc[1]/(float)counts_fore_asc[1],
        counts_aft_asc[0], sum_dtb_aft_asc[0]/(float)counts_aft_asc[0],
        counts_aft_asc[1], sum_dtb_aft_asc[1]/(float)counts_aft_asc[1],
        counts_fore_dec[0], sum_dtb_fore_dec[0]/(float)counts_fore_dec[0],
        counts_fore_dec[1], sum_dtb_fore_dec[1]/(float)counts_fore_dec[1],
        counts_aft_dec[0], sum_dtb_aft_dec[0]/(float)counts_aft_dec[0],
        counts_aft_dec[1], sum_dtb_aft_dec[1]/(float)counts_aft_dec[1]);
    fclose(ofp);
    return(0);
}
