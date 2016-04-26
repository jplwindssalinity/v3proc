#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <math.h>
#include "hdf5.h"
#include "hdf5_hl.h"
#include "Constants.h"

int read_SDS_h5(hid_t obj_id, const char* sds_name, void* data_buffer) {
    hid_t sds_id = H5Dopen1(obj_id,sds_name);
    if( sds_id < 0 ) return(0);
    
	hid_t type_id = H5Dget_type(sds_id);
	if( type_id < 0 ) return(0);
	
	if( H5Dread( sds_id, type_id, H5S_ALL, H5S_ALL,H5P_DEFAULT, data_buffer) < 0 ||
	    H5Dclose( sds_id ) < 0 ) return(0);
	
	return(1);
}

int main(int argc, char* argv[]){

    char* l1bfile = argv[1];
    char* l1cfile = argv[2];
    char* outfile = argv[3];

    FILE* fp = fopen(outfile, "r");
    if(fp!=NULL) {
        exit(1);
    }

    hid_t l1b_id = H5Fopen(l1bfile, H5F_ACC_RDONLY, H5P_DEFAULT);
    hsize_t dims[2];
    H5T_class_t class_id;
    size_t type_size;

    if(H5LTget_dataset_info(l1b_id, "/Sigma0_Data/sigma0_hh",
        dims, &class_id, &type_size))
        return(0);

    int nscans = dims[0];
    int nlris = dims[1];

    printf("nscans: %d; nlris: %d\n", nscans, nlris);

    int l1b_size = nscans * nlris;

    std::vector<float> l1b_lat(l1b_size);
    std::vector<float> l1b_lon(l1b_size);
    std::vector<float> l1b_antazi(l1b_size);

    read_SDS_h5(l1b_id, "/Sigma0_Data/center_lat_v", &l1b_lat[0]);
    read_SDS_h5(l1b_id, "/Sigma0_Data/center_lon_v", &l1b_lon[0]);
    read_SDS_h5(l1b_id, "/Sigma0_Data/antenna_scan_angle", &l1b_antazi[0]);

    H5Fclose(l1b_id);

    std::vector<float> match_hh(l1b_size);
    std::vector<float> match_vv(l1b_size);
    std::vector<float> match_xpol(l1b_size);
    std::vector<float> match_hh_rms(l1b_size);
    std::vector<float> match_vv_rms(l1b_size);
    std::vector<float> match_xpol_rms(l1b_size);

    hid_t l1c_id = H5Fopen(l1cfile, H5F_ACC_RDONLY, H5P_DEFAULT);
    if(H5LTget_dataset_info(l1c_id, "/Sigma0_Data/cell_sigma0_hh_fore",
        dims, &class_id, &type_size))
        return(0);

    int natis = dims[0];
    int nctis = dims[1];

    printf("natis: %d; nctis: %d\n", natis, nctis);

    int l1c_size = natis * nctis;

    std::vector<float> l1c_lat(l1c_size);
    std::vector<float> l1c_lon(l1c_size);

    std::vector<float> l1c_hh_fore(l1c_size);
    std::vector<float> l1c_hh_aft(l1c_size);
    std::vector<float> cell_kp_hh_fore(l1c_size);
    std::vector<float> cell_kp_hh_aft(l1c_size);
    std::vector<uint16_t> l1c_hh_flg(l1c_size);
    
    std::vector<float> l1c_vv_fore(l1c_size);
    std::vector<float> l1c_vv_aft(l1c_size);
    std::vector<float> cell_kp_vv_fore(l1c_size);
    std::vector<float> cell_kp_vv_aft(l1c_size);
    std::vector<uint16_t> l1c_vv_flg(l1c_size);

    std::vector<float> l1c_xpol_fore(l1c_size);
    std::vector<float> l1c_xpol_aft(l1c_size);
    std::vector<float> cell_kp_xpol_fore(l1c_size);
    std::vector<float> cell_kp_xpol_aft(l1c_size);
    std::vector<uint16_t> l1c_xpol_flg(l1c_size);

    read_SDS_h5(l1c_id, "/Sigma0_Data/cell_lat", &l1c_lat[0]);
    read_SDS_h5(l1c_id, "/Sigma0_Data/cell_lon", &l1c_lon[0]);

    read_SDS_h5(l1c_id, "/Sigma0_Data/cell_sigma0_hh_fore", &l1c_hh_fore[0]);
    read_SDS_h5(l1c_id, "/Sigma0_Data/cell_sigma0_hh_aft", &l1c_hh_aft[0]);
    read_SDS_h5(l1c_id, "/Sigma0_Data/cell_kp_hh_fore", &cell_kp_hh_fore[0]);
    read_SDS_h5(l1c_id, "/Sigma0_Data/cell_kp_hh_aft", &cell_kp_hh_aft[0]);
    read_SDS_h5(l1c_id, "/Sigma0_Data/cell_sigma0_qual_flag_hh", &l1c_hh_flg[0]);

    read_SDS_h5(l1c_id, "/Sigma0_Data/cell_sigma0_vv_fore", &l1c_vv_fore[0]);
    read_SDS_h5(l1c_id, "/Sigma0_Data/cell_sigma0_vv_aft", &l1c_vv_aft[0]);
    read_SDS_h5(l1c_id, "/Sigma0_Data/cell_kp_vv_fore", &cell_kp_vv_fore[0]);
    read_SDS_h5(l1c_id, "/Sigma0_Data/cell_kp_vv_aft", &cell_kp_vv_aft[0]);
    read_SDS_h5(l1c_id, "/Sigma0_Data/cell_sigma0_qual_flag_vv", &l1c_vv_flg[0]);

    read_SDS_h5(l1c_id, "/Sigma0_Data/cell_sigma0_xpol_fore", &l1c_xpol_fore[0]);
    read_SDS_h5(l1c_id, "/Sigma0_Data/cell_sigma0_xpol_aft", &l1c_xpol_aft[0]);
    read_SDS_h5(l1c_id, "/Sigma0_Data/cell_kp_xpol_fore", &cell_kp_xpol_fore[0]);
    read_SDS_h5(l1c_id, "/Sigma0_Data/cell_kp_xpol_aft", &cell_kp_xpol_aft[0]);
    read_SDS_h5(l1c_id, "/Sigma0_Data/cell_sigma0_qual_flag_xpol", &l1c_xpol_flg[0]);

    H5Fclose(l1c_id);

    float delta_threshold = pow(15.0/111.0, 2);

    std::vector<int> l1c_useable_fore(l1c_size);
    std::vector<int> l1c_useable_aft(l1c_size);

    for(int l1c_idx = 0; l1c_idx < l1c_size; ++l1c_idx) {

        float kp_threshold = 1.78;

        uint16_t bit_mask_fore = 0x5024;
        uint16_t bit_mask_aft = 0xa088;

        l1c_useable_fore[l1c_idx] = 1;
        l1c_useable_aft[l1c_idx] = 1;

        // Check L1C quality flag bits
        if((l1c_hh_flg[l1c_idx] & bit_mask_fore) || 
           (l1c_vv_flg[l1c_idx] & bit_mask_fore) ||
           (l1c_xpol_flg[l1c_idx] & bit_mask_fore) ||
           (cell_kp_hh_fore[l1c_idx] > kp_threshold) ||
           (cell_kp_vv_fore[l1c_idx] > kp_threshold) ||
           (cell_kp_xpol_fore[l1c_idx] > kp_threshold))
            l1c_useable_fore[l1c_idx] = 0;

        if((l1c_hh_flg[l1c_idx] & bit_mask_aft) || 
           (l1c_vv_flg[l1c_idx] & bit_mask_aft) ||
           (l1c_xpol_flg[l1c_idx] & bit_mask_aft) ||
           (cell_kp_hh_aft[l1c_idx] > kp_threshold) ||
           (cell_kp_vv_aft[l1c_idx] > kp_threshold) ||
           (cell_kp_xpol_aft[l1c_idx] > kp_threshold))

            l1c_useable_aft[l1c_idx] = 0;

    }

    for(int l1b_idx = 0; l1b_idx < l1b_size; ++l1b_idx) {

        if(l1b_idx % 1000 == 0) {
            printf("%d %d\n", l1b_idx, l1b_size);
        }

        match_hh[l1b_idx] = -9999;
        match_vv[l1b_idx] = -9999;
        match_xpol[l1b_idx] = -9999;

        match_hh_rms[l1b_idx] = -9999;
        match_vv_rms[l1b_idx] = -9999;
        match_xpol_rms[l1b_idx] = -9999;

        // Check for fill value
        if(l1b_lat[l1b_idx] < -90)
            continue;

        float this_l1b_lon = l1b_lon[l1b_idx];
        float this_l1b_lat = l1b_lat[l1b_idx];
        float this_cos_lat = cos(this_l1b_lat*dtr);
        float this_antazi = l1b_antazi[l1b_idx];

        int cnts = 0;
        float sum_l1c_hh = 0;
        float sum_l1c_vv = 0;
        float sum_l1c_xpol = 0;
        float sum_l1c_hh2 = 0;
        float sum_l1c_vv2 = 0;
        float sum_l1c_xpol2 = 0;

        int is_fore = (this_antazi < 90 || this_antazi > 270) ? 1 : 0;

        for(int l1c_idx = 0; l1c_idx < l1c_size; ++l1c_idx) {

            if((is_fore && !l1c_useable_fore[l1c_idx]) ||
               (!is_fore && !l1c_useable_aft[l1c_idx]))
                continue;

            float dlat = l1c_lat[l1c_idx] - this_l1b_lat;

            if(fabs(dlat) > 0.15)
                continue;

            float dlon = l1c_lon[l1c_idx] - this_l1b_lon;
            if(dlon >= 180) dlon -= 360;
            if(dlon < -180) dlon += 360;

            dlon *= this_cos_lat;

            if(dlon*dlon + dlat*dlat < delta_threshold) {
                cnts++;
                if(is_fore) {
                    sum_l1c_hh += l1c_hh_fore[l1c_idx];
                    sum_l1c_vv += l1c_vv_fore[l1c_idx];
                    sum_l1c_xpol += l1c_xpol_fore[l1c_idx];
                    sum_l1c_hh2 += pow(l1c_hh_fore[l1c_idx], 2);
                    sum_l1c_vv2 += pow(l1c_vv_fore[l1c_idx], 2);
                    sum_l1c_xpol2 += pow(l1c_xpol_fore[l1c_idx], 2);
                } else {
                    sum_l1c_hh += l1c_hh_aft[l1c_idx];
                    sum_l1c_vv += l1c_vv_aft[l1c_idx];
                    sum_l1c_xpol += l1c_xpol_aft[l1c_idx];
                    sum_l1c_hh2 += pow(l1c_hh_aft[l1c_idx], 2);
                    sum_l1c_vv2 += pow(l1c_vv_aft[l1c_idx], 2);
                    sum_l1c_xpol2 += pow(l1c_xpol_aft[l1c_idx], 2);
                }
            }
        }

        if(cnts > 0) {
            match_hh[l1b_idx] = sum_l1c_hh/float(cnts);
            match_vv[l1b_idx] = sum_l1c_vv/float(cnts);
            match_xpol[l1b_idx] = sum_l1c_xpol/float(cnts);
            match_hh_rms[l1b_idx] = sqrt(sum_l1c_hh2/float(cnts));
            match_vv_rms[l1b_idx] = sqrt(sum_l1c_vv2/float(cnts));
            match_xpol_rms[l1b_idx] = sqrt(sum_l1c_xpol2/float(cnts));
        }
    }

    FILE* ofp = fopen(outfile, "w");
    fwrite(&nscans, sizeof(int), 1, ofp);
    fwrite(&nlris, sizeof(int), 1, ofp);
    fwrite(&match_hh[0], sizeof(float), l1b_size, ofp);
    fwrite(&match_vv[0], sizeof(float), l1b_size, ofp);
    fwrite(&match_xpol[0], sizeof(float), l1b_size, ofp);
    fwrite(&match_hh_rms[0], sizeof(float), l1b_size, ofp);
    fwrite(&match_vv_rms[0], sizeof(float), l1b_size, ofp);
    fwrite(&match_xpol_rms[0], sizeof(float), l1b_size, ofp);
    fclose(ofp);

    return(0);
}

