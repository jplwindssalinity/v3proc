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
#define L1B_TB_ASC_ANC_WSAT_FILE_KEYWORD "L1B_TB_ASC_ANC_WSAT_FILE"
#define L1B_TB_DEC_ANC_WSAT_FILE_KEYWORD "L1B_TB_DEC_ANC_WSAT_FILE"

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

    int use_wsat_matchups = 0;

    if(argc == 4 && strcmp(argv[3], "--wsat") == 0)
        use_wsat_matchups = 1;

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

    char* anc_wsat_files[2] = {NULL, NULL};

    if(use_wsat_matchups) {
        anc_wsat_files[0] = config_list.Get(L1B_TB_ASC_ANC_WSAT_FILE_KEYWORD);
        anc_wsat_files[1] = config_list.Get(L1B_TB_DEC_ANC_WSAT_FILE_KEYWORD);
    }

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

    int revno;
    config_list.GetInt("REVNO", &revno);

    // Iterate over ascending / decending portions of orbit
    for(int ipart = 0; ipart < 2; ++ipart){
        hid_t id = H5Fopen(l1b_tbfiles[ipart], H5F_ACC_RDONLY, H5P_DEFAULT);

        std::vector<float> lat;
        std::vector<float> lon;
        std::vector<float> azi;
        std::vector<float> antazi;
        std::vector<float> inc;

        std::vector< std::vector<float> > tb(2);
        std::vector< std::vector<float> > exp_tb(2);
        std::vector< std::vector<float> > nedt(2);
        std::vector< std::vector<uint16> > tb_flag(2);

        int data_size = nframes[ipart]*nfootprints[ipart];
        azi.resize(data_size);
        antazi.resize(data_size);
        lat.resize(data_size);
        lon.resize(data_size);
        inc.resize(data_size);

        // resize arrays for data dimensions
        for(int ipol = 0; ipol < 2; ++ipol) {
            tb[ipol].resize(data_size);
            exp_tb[ipol].resize(data_size);
            nedt[ipol].resize(data_size);
            tb_flag[ipol].resize(data_size);
        }

        // These data only have footprint versions
        read_SDS_h5(id, "/Brightness_Temperature/tb_lat", &lat[0]);
        read_SDS_h5(id, "/Brightness_Temperature/tb_lon", &lon[0]);
        read_SDS_h5(id, "/Brightness_Temperature/earth_boresight_azimuth", &azi[0]);
        read_SDS_h5(id, "/Brightness_Temperature/antenna_scan_angle", &antazi[0]);
        read_SDS_h5(id, "/Brightness_Temperature/earth_boresight_incidence", &inc[0]);

        read_SDS_h5(id, "/Brightness_Temperature/tb_v", &tb[0][0]);
        read_SDS_h5(id, "/Brightness_Temperature/tb_h", &tb[1][0]);

        read_SDS_h5(id, "/Brightness_Temperature/nedt_v", &nedt[0][0]);
        read_SDS_h5(id, "/Brightness_Temperature/nedt_h", &nedt[1][0]);

        read_SDS_h5(id, "/Brightness_Temperature/tb_qual_flag_v", &tb_flag[0][0]);
        read_SDS_h5(id, "/Brightness_Temperature/tb_qual_flag_h", &tb_flag[1][0]);

        CAP_ANC_L1B anc_u10(anc_u10_files[ipart]);
        CAP_ANC_L1B anc_v10(anc_v10_files[ipart]);
        CAP_ANC_L1B anc_sss(anc_sss_files[ipart]);
        CAP_ANC_L1B anc_sst(anc_sst_files[ipart]);
        CAP_ANC_L1B anc_swh(anc_swh_files[ipart]);
        CAP_ANC_L1B anc_wsat;
        
        if(use_wsat_matchups)
            anc_wsat.Read(anc_wsat_files[ipart]);

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
                    exp_tb[ipol][fp_idx] = -9999;

                    if(lat[fp_idx] < -90)
                        continue;

                    double tmp_lon = dtr*lon[fp_idx];
                    double tmp_lat = dtr*lat[fp_idx];
                    if(tmp_lon<0) tmp_lon += two_pi;

                    // TBD Filtering on location, land, ice flags
                    if(qs_landmap.IsLand(tmp_lon, tmp_lat, 0) ||
                       qs_icemap.IsIce(tmp_lon, tmp_lat, 0))
                        continue;

                    Meas::MeasTypeE met;
                    if(ipol==0)
                        met = Meas::L_BAND_TBV_MEAS_TYPE;
                    else
                        met = Meas::L_BAND_TBH_MEAS_TYPE;

                    float u10 = anc_u10.data[0][iframe][ifootprint];
                    float v10 = anc_v10.data[0][iframe][ifootprint];

                    // increase NCEP by 3%
                    float spd = 1.03 * sqrt(u10*u10 + v10*v10);
                    float rain = 0;
                    float dt = 0;

                    if(use_wsat_matchups) {
                        spd = anc_wsat.data[0][iframe][ifootprint];
                        rain = anc_wsat.data[2][iframe][ifootprint];
                        dt = anc_wsat.data[3][iframe][ifootprint];
                    }

                    if(spd < 0 || rain > 0 || dt > 45)
                        continue;

                    // Met convention
                    float dir = atan2(-u10, -v10);

                    float sss = anc_sss.data[0][iframe][ifootprint];
                    float sst = anc_sst.data[0][iframe][ifootprint];
                    sst += 273.16;
                    float swh = anc_swh.data[0][iframe][ifootprint];
                    float this_inc = dtr*inc[fp_idx];

                    // Compute relative azimuth angle
                    float relazi = azi[fp_idx] - dir;
                    relazi *= pi/180.0;
                    relazi -= pi;
                    while(relazi<0) relazi += two_pi;
                    while(relazi>=two_pi) relazi -= two_pi;

                    float model_tb, tb_flat, dtb;
                    float tb_flat_plus, dtb_plus;
                    float tb_flat_minus, dtb_minus;

                    float dsss = 1;
                    float sss_plus = sss + dsss;
                    float sss_minus = sss - dsss;

                    cap_gmf.GetTB(
                        met, this_inc, sst, sss, spd, relazi, swh, &tb_flat,
                        &dtb);

                    model_tb = tb_flat + dtb;

                    exp_tb[ipol][fp_idx] = model_tb;

                }
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
        fwrite(&exp_tb[0][0], sizeof(float), data_size, ofp);
        fwrite(&exp_tb[1][0], sizeof(float), data_size, ofp);
        fclose(ofp);
    }
    return(0);
}
