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
    std::vector<float> l1b_hh(l1b_size);
    std::vector<float> l1b_vv(l1b_size);
    std::vector<float> l1b_hv(l1b_size);
    std::vector<uint16_t> l1b_hh_flg(l1b_size);
    std::vector<uint16_t> l1b_vv_flg(l1b_size);
    std::vector<uint16_t> l1b_hv_flg(l1b_size);

    read_SDS_h5(l1b_id, "/Sigma0_Data/center_lat_v", &l1b_lat[0]);
    read_SDS_h5(l1b_id, "/Sigma0_Data/center_lon_v", &l1b_lon[0]);
    read_SDS_h5(l1b_id, "/Sigma0_Data/antenna_scan_angle", &l1b_antazi[0]);
    read_SDS_h5(l1b_id, "/Sigma0_Data/sigma0_hh", &l1b_hh[0]);
    read_SDS_h5(l1b_id, "/Sigma0_Data/sigma0_vv", &l1b_vv[0]);
    read_SDS_h5(l1b_id, "/Sigma0_Data/sigma0_hv", &l1b_hv[0]);
    read_SDS_h5(l1b_id, "/Sigma0_Data/sigma0_qual_flag_hh", &l1b_hh_flg[0]);
    read_SDS_h5(l1b_id, "/Sigma0_Data/sigma0_qual_flag_vv", &l1b_vv_flg[0]);
    read_SDS_h5(l1b_id, "/Sigma0_Data/sigma0_qual_flag_hv", &l1b_hv_flg[0]);

    H5Fclose(l1b_id);

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
    std::vector<uint16_t> l1c_hh_flg(l1c_size);
    std::vector<uint16_t> l1c_vv_flg(l1c_size);
    std::vector<uint16_t> l1c_xpol_flg(l1c_size);

    read_SDS_h5(l1c_id, "/Sigma0_Data/cell_lat", &l1c_lat[0]);
    read_SDS_h5(l1c_id, "/Sigma0_Data/cell_lon", &l1c_lon[0]);
    read_SDS_h5(l1c_id, "/Sigma0_Data/cell_sigma0_qual_flag_hh", &l1c_hh_flg[0]);
    read_SDS_h5(l1c_id, "/Sigma0_Data/cell_sigma0_qual_flag_vv", &l1c_vv_flg[0]);
    read_SDS_h5(l1c_id, "/Sigma0_Data/cell_sigma0_qual_flag_xpol", &l1c_xpol_flg[0]);
    H5Fclose(l1c_id);

    // outputs
    std::vector<float> match_hh_fore(l1c_size);
    std::vector<float> match_vv_fore(l1c_size);
    std::vector<float> match_hv_fore(l1c_size);
    std::vector<float> match_fore_scan(l1c_size);

    std::vector<float> match_hh_aft(l1c_size);
    std::vector<float> match_vv_aft(l1c_size);
    std::vector<float> match_hv_aft(l1c_size);
    std::vector<float> match_aft_scan(l1c_size);

    std::vector<int> l1b_useable(l1b_size);
    for(int l1b_idx = 0; l1b_idx < l1b_size; ++l1b_idx) {
        l1b_useable[l1b_idx] = 1;

        if(l1b_hh_flg[l1b_idx] & 0x1 || 
           l1b_vv_flg[l1b_idx] & 0x1 ||
           l1b_hv_flg[l1b_idx]) l1b_useable[l1b_idx] = 0;
    }

    float delta_threshold = pow(15.0/111.0, 2);

    for(int l1c_idx = 0; l1c_idx < l1c_size; ++l1c_idx) {

        int ati = l1c_idx / nctis;
        int cti = l1c_idx - nctis * ati;

        match_hh_fore[l1c_idx] = -9999;
        match_vv_fore[l1c_idx] = -9999;
        match_hv_fore[l1c_idx] = -9999;
        match_fore_scan[l1c_idx] = -9999;

        match_hh_aft[l1c_idx] = -9999;
        match_vv_aft[l1c_idx] = -9999;
        match_hv_aft[l1c_idx] = -9999;
        match_aft_scan[l1c_idx] = -9999;

//         if(cti == 0 && ati % 50 == 0) printf("ati: %d\n", ati);
//         if(ati > 3000) break;

        // Check if L1C is useable
        if(l1c_lat[l1c_idx] < -90)
            continue;

        float this_l1c_lat = l1c_lat[l1c_idx];
        float this_l1c_lon = l1c_lon[l1c_idx];
        float this_cos_lat = cos(l1c_lat[l1c_idx]*dtr);

        float nearest_delta2_fore = delta_threshold;
        float nearest_delta2_aft = delta_threshold;

        // loop over L1B
        for(int l1b_idx = 0; l1b_idx < l1b_size; ++l1b_idx) {

            // scan index
            int iscan = l1b_idx / nlris;

            if(l1b_useable[l1b_idx] == 0)
                continue;

            float dlat = l1b_lat[l1b_idx] - this_l1c_lat;

            if(fabs(dlat) > 0.15)
                continue;

            float dlon = l1b_lon[l1b_idx] - this_l1c_lon;
            if(dlon >= 180) dlon -= 360;
            if(dlon < -180) dlon += 360;

            dlon *= this_cos_lat;
            float this_delta2 = dlon*dlon + dlat*dlat;

            int is_fore = (
                l1b_antazi[l1b_idx] < 90 || l1b_antazi[l1b_idx] > 270
                ) ? 1 : 0;

            if(is_fore == 1 && this_delta2 < nearest_delta2_fore) {
                nearest_delta2_fore = this_delta2;
                match_hh_fore[l1c_idx] = l1b_hh[l1b_idx];
                match_vv_fore[l1c_idx] = l1b_vv[l1b_idx];
                match_hv_fore[l1c_idx] = l1b_hv[l1b_idx];
                match_fore_scan[l1c_idx] = (float)iscan;

            } else if(is_fore == 0 && this_delta2 < nearest_delta2_aft) {
                nearest_delta2_aft = this_delta2;
                match_hh_aft[l1c_idx] = l1b_hh[l1b_idx];
                match_vv_aft[l1c_idx] = l1b_vv[l1b_idx];
                match_hv_aft[l1c_idx] = l1b_hv[l1b_idx];
                match_aft_scan[l1c_idx] = (float)iscan;
            }
        }

    }

    FILE* ofp = fopen(outfile, "w");
    fwrite(&natis, sizeof(int), 1, ofp);
    fwrite(&nctis, sizeof(int), 1, ofp);
    fwrite(&match_hh_fore[0], sizeof(float), l1c_size, ofp);
    fwrite(&match_vv_fore[0], sizeof(float), l1c_size, ofp);
    fwrite(&match_hv_fore[0], sizeof(float), l1c_size, ofp);
    fwrite(&match_fore_scan[0], sizeof(float), l1c_size, ofp);
    fwrite(&match_hh_aft[0], sizeof(float), l1c_size, ofp);
    fwrite(&match_vv_aft[0], sizeof(float), l1c_size, ofp);
    fwrite(&match_hv_aft[0], sizeof(float), l1c_size, ofp);
    fwrite(&match_aft_scan[0], sizeof(float), l1c_size, ofp);
    fclose(ofp);

    return(0);
}





