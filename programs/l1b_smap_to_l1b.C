//==============================================================//
// Copyright (C) 2015, California Institute of Technology.      //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#define L1B_S0_LORES_ASC_FILE_KEYWORD "L1B_S0_LORES_ASC_FILE"
#define L1B_S0_LORES_DEC_FILE_KEYWORD "L1B_S0_LORES_DEC_FILE"
#define L1C_S0_HIRES_ASC_FILE_KEYWORD "L1C_S0_HIRES_ASC_FILE"
#define L1C_S0_HIRES_DEC_FILE_KEYWORD "L1C_S0_HIRES_DEC_FILE"
#define L1B_TB_LORES_ASC_FILE_KEYWORD "L1B_TB_LORES_ASC_FILE"
#define L1B_TB_LORES_DEC_FILE_KEYWORD "L1B_TB_LORES_DEC_FILE"
#define DO_QUADPOL_PROCESSING_KEYWORD "DO_QUADPOL_PROCESSING"

//----------//
// INCLUDES //
//----------//

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <vector>
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

int read_SDS_h5( hid_t obj_id, const char* sds_name, void* data_buffer )
{
    hid_t sds_id = H5Dopen1(obj_id,sds_name);
    if( sds_id < 0 ) return(0);
    
	hid_t type_id = H5Dget_type(sds_id);
	if( type_id < 0 ) return(0);
	
	if( H5Dread( sds_id, type_id, H5S_ALL, H5S_ALL,H5P_DEFAULT, data_buffer) < 0 ||
	    H5Dclose( sds_id ) < 0 ) return(0);
	
	return(1);
}

int determine_l1b_sizes(char* l1b_s0files[], char* l1b_tbfiles[], int nframes[],
                        int nfootprints[], int nslices[]) {

    for(int ipart=0; ipart<2; ++ipart) {
        hid_t id = H5Fopen(l1b_s0files[ipart], H5F_ACC_RDONLY, H5P_DEFAULT);
        if(id<0) return(0);

        hsize_t dims[3];
        H5T_class_t class_id;
        size_t type_size;

        if(H5LTget_dataset_info(id, "/Sigma0_Slice_Data/slice_sigma0_hh",
            dims, &class_id, &type_size))
            return(0);

        nframes[ipart] = dims[0];
        nfootprints[ipart] = dims[1];
        nslices[ipart] = dims[2];
    }

    // Check that TB files have same dims as s0 files
    if(l1b_tbfiles[0]&&l1b_tbfiles[1]){
        for(int ipart=0; ipart<2; ++ipart) {
            hid_t id = H5Fopen(l1b_tbfiles[ipart], H5F_ACC_RDONLY, H5P_DEFAULT);
            if(id<0) return(0);
            int rank[2];
            if(H5LTget_dataset_ndims(id, "/Brightness_Temperature/tb_h",
                rank)<0)
                return(0);

            if(nframes[ipart] != rank[0] || nfootprints[ipart] != rank[1])
                return(0);
        }
    }
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
    const char* command = NULL;

    char* config_file = NULL;
    int do_footprint = 0;

    command = no_path(argv[0]);

    //------------------------//
    // parse the command line //
    //------------------------//
    while ((optind < argc) && (argv[optind][0]=='-')) {
        std::string sw = argv[optind];
        if( sw == "-c" ) {
            config_file = argv[++optind];
        } else if ( sw == "-fp" ) {
            do_footprint = 1;
        } else {
            fprintf(stderr,"%s: Unknown option\n", command);
            exit(1);
        }
        ++optind;
    }

    if(config_file == NULL) {
        fprintf(stderr,"%s: Must specify -c config_file\n", command );
        exit(1);
    }

    ConfigList config_list;
    if(!config_list.Read(config_file)) {
        fprintf(
            stderr, "%s: error reading config file %s\n", command, config_file);
        exit(1);
    }

    //----------------------------------//
    // check for config file parameters //
    //----------------------------------//
    char* l1b_s0files[2] = {NULL, NULL};
    char* l1b_tbfiles[2] = {NULL, NULL};
    char* l1c_s0files[2] = {NULL, NULL};

    // These ones are required
    config_list.ExitForMissingKeywords();
    l1b_s0files[0] = config_list.Get(L1B_S0_LORES_ASC_FILE_KEYWORD);
    l1b_s0files[1] = config_list.Get(L1B_S0_LORES_DEC_FILE_KEYWORD);

    // Not required
    config_list.DoNothingForMissingKeywords();
    l1b_tbfiles[0] = config_list.Get(L1B_TB_LORES_ASC_FILE_KEYWORD);
    l1b_tbfiles[1] = config_list.Get(L1B_TB_LORES_DEC_FILE_KEYWORD);
    l1c_s0files[0] = config_list.Get(L1C_S0_HIRES_ASC_FILE_KEYWORD);
    l1c_s0files[1] = config_list.Get(L1C_S0_HIRES_DEC_FILE_KEYWORD);

    printf("l1b_s0files: %s %s\n", l1b_s0files[0], l1b_s0files[1]);
    printf("l1c_s0files: %s %s\n", l1c_s0files[0], l1c_s0files[1]);
    printf("l1b_tbfiles: %s %s\n", l1b_tbfiles[0], l1b_tbfiles[1]);

    // back to making keywords required
    config_list.ExitForMissingKeywords();
    int do_quadpol = 0;
    config_list.GetInt(DO_QUADPOL_PROCESSING_KEYWORD, &do_quadpol);

    // Check for L1B_FILE keyword
    char* output_file = config_list.Get(L1B_FILE_KEYWORD);
    if (output_file == NULL) {
        fprintf(stderr, "%s: config file must specify L1B_FILE\n", command);
        exit(1);
    }

    L1B l1b;
    if (l1b.OpenForWriting(output_file) == 0) {
        fprintf(
            stderr, "%s: cannot open l1b file %s for output\n", command,
            output_file);
        exit(1);
    }

    // Determine number of frames in both portions of orbit
    int nframes[2] = {0, 0};
    int nfootprints[2] = {0, 0};
    int nslices[2] = {0, 0};

    if(!determine_l1b_sizes(
        l1b_s0files, l1b_tbfiles, nframes, nfootprints, nslices)) {
        fprintf(stderr, "Non-matching sizes found in L1B S0 and TB files!");
        exit(1);
    }

    // Iterate over ascending / decending portions of orbit
    for(int ipart = 0; ipart < 2; ++ipart){

        hid_t id = H5Fopen(l1b_s0files[ipart], H5F_ACC_RDONLY, H5P_DEFAULT);

        char antenna_scan_time_utc[nframes[ipart]][24];

        std::vector<float> x_pos(nframes[ipart]);
        std::vector<float> y_pos(nframes[ipart]);
        std::vector<float> z_pos(nframes[ipart]);
        std::vector<float> x_vel(nframes[ipart]);
        std::vector<float> y_vel(nframes[ipart]);
        std::vector<float> z_vel(nframes[ipart]);
        std::vector<float> yaw(nframes[ipart]);
        std::vector<float> pitch(nframes[ipart]);
        std::vector<float> roll(nframes[ipart]);

        read_SDS_h5(
            id, "/Spacecraft_Data/antenna_scan_time_utc", &antenna_scan_time_utc[0][0]);
        H5LTread_dataset_float(id, "/Spacecraft_Data/x_pos", &x_pos[0]);
        H5LTread_dataset_float(id, "/Spacecraft_Data/y_pos", &y_pos[0]);
        H5LTread_dataset_float(id, "/Spacecraft_Data/z_pos", &z_pos[0]);
        H5LTread_dataset_float(id, "/Spacecraft_Data/x_vel", &x_vel[0]);
        H5LTread_dataset_float(id, "/Spacecraft_Data/y_vel", &y_vel[0]);
        H5LTread_dataset_float(id, "/Spacecraft_Data/z_vel", &z_vel[0]);
        H5LTread_dataset_float(id, "/Spacecraft_Data/yaw", &yaw[0]);
        H5LTread_dataset_float(id, "/Spacecraft_Data/pitch", &pitch[0]);
        H5LTread_dataset_float(id, "/Spacecraft_Data/roll", &roll[0]);

        std::vector<float> lat;
        std::vector<float> lon;
        std::vector<float> azi;
        std::vector<float> antazi;
        std::vector<float> inc;

        std::vector< std::vector<float> > kp(4);
        std::vector< std::vector<float> > snr(4);
        std::vector< std::vector<float> > xf(4);
        std::vector< std::vector<float> > s0(4);
        std::vector< std::vector<uint16> > s0_flag(4);

        int data_size = nframes[ipart]*nfootprints[ipart];
        azi.resize(data_size);
        antazi.resize(data_size);

        if(!do_footprint)
            data_size *= nslices[ipart];

        lat.resize(data_size);
        lon.resize(data_size);
        inc.resize(data_size);

        // resize arrays for data dimensions
        for(int ipol = 0; ipol < 4; ++ipol) {
            kp[ipol].resize(data_size);
            snr[ipol].resize(data_size);
            xf[ipol].resize(data_size);
            s0[ipol].resize(data_size);
            s0_flag[ipol].resize(data_size);
        }

        // These data only have footprint versions
        read_SDS_h5(id, "/Sigma0_Data/earth_boresight_azimuth", &azi[0]);
        read_SDS_h5(id, "/Sigma0_Data/antenna_scan_angle", &antazi[0]);

        // Read in the L1B data depending on do_footprint flag
        // polarization order to match Meas::MeasTypeE (VV, HH, VH, HV)
        if(do_footprint) {
            read_SDS_h5(id, "/Sigma0_Data/center_lat", &lat[0]);
            read_SDS_h5(id, "/Sigma0_Data/center_lon", &lon[0]);
            read_SDS_h5(id, "/Sigma0_Data/earth_boresight_incidence", &inc[0]);

            read_SDS_h5(id, "/Sigma0_Data/kp_vv", &kp[0][0]);
            read_SDS_h5(id, "/Sigma0_Data/kp_hh", &kp[1][0]);
            read_SDS_h5(id, "/Sigma0_Data/kp_vh", &kp[2][0]);
            read_SDS_h5(id, "/Sigma0_Data/kp_hv", &kp[3][0]);

            read_SDS_h5(id, "/Sigma0_Data/snr_vv", &snr[0][0]);
            read_SDS_h5(id, "/Sigma0_Data/snr_hh", &snr[1][0]);
            read_SDS_h5(id, "/Sigma0_Data/snr_vh", &snr[2][0]);
            read_SDS_h5(id, "/Sigma0_Data/snr_hv", &snr[3][0]);

            read_SDS_h5(id, "/Sigma0_Data/x_factor_vv", &xf[0][0]);
            read_SDS_h5(id, "/Sigma0_Data/x_factor_hh", &xf[1][0]);
            read_SDS_h5(id, "/Sigma0_Data/x_factor_vh", &xf[2][0]);
            read_SDS_h5(id, "/Sigma0_Data/x_factor_hv", &xf[3][0]);

            // ...etc
        } else {
            read_SDS_h5(id, "/Sigma0_Slice_Data/slice_lat", &lat[0]);
            read_SDS_h5(id, "/Sigma0_Slice_Data/slice_lon", &lon[0]);
            read_SDS_h5(id, "/Sigma0_Slice_Data/slice_earth_incidence", &inc[0]);

            read_SDS_h5(id, "/Sigma0_Slice_Data/slice_kp_vv", &kp[0][0]);
            read_SDS_h5(id, "/Sigma0_Slice_Data/slice_kp_hh", &kp[1][0]);
            read_SDS_h5(id, "/Sigma0_Slice_Data/slice_kp_vh", &kp[2][0]);
            read_SDS_h5(id, "/Sigma0_Slice_Data/slice_kp_hv", &kp[3][0]);

            read_SDS_h5(id, "/Sigma0_Slice_Data/slice_snr_vv", &snr[0][0]);
            read_SDS_h5(id, "/Sigma0_Slice_Data/slice_snr_hh", &snr[1][0]);
            read_SDS_h5(id, "/Sigma0_Slice_Data/slice_snr_vh", &snr[2][0]);
            read_SDS_h5(id, "/Sigma0_Slice_Data/slice_snr_hv", &snr[3][0]);

            read_SDS_h5(id, "/Sigma0_Slice_Data/slice_x_factor_vv", &xf[0][0]);
            read_SDS_h5(id, "/Sigma0_Slice_Data/slice_x_factor_hh", &xf[1][0]);
            read_SDS_h5(id, "/Sigma0_Slice_Data/slice_x_factor_vh", &xf[2][0]);
            read_SDS_h5(id, "/Sigma0_Slice_Data/slice_x_factor_hv", &xf[3][0]);


            // ...etc
        }

        // Iterate over scans
        for(int iframe = 0; iframe < nframes[ipart]; ++iframe) {
            l1b.frame.spotList.FreeContents();

            // Copy over time-strings, null terminate and parse
            char time_str[24];
            init_string(time_str, 24);

            // only copy first 23 chars (last one is Z)
            strncpy(time_str, antenna_scan_time_utc[iframe], 23);

            ETime etime;
            etime.FromCodeB("1970-001T00:00:00.000");
            double time_base = (
                (double)etime.GetSec() + (double)etime.GetMs()/1000);
            etime.FromCodeA(time_str);

            // Result
            double time = (
                (double)etime.GetSec()+(double)etime.GetMs()/1000 - time_base);

            int nfootprints = 0;
            int nslices = 0;

            // Iterate over low-res footprints
            for(int ifootprint = 0; ifootprint < nfootprints; ++ifootprint) {
                MeasSpot* new_meas_spot = new MeasSpot();

                // Is this good enough (interpolate within each scan)
                new_meas_spot->time = time;

                new_meas_spot->scOrbitState.time = time;
                new_meas_spot->scOrbitState.rsat.Set(
                    (double)x_pos[iframe]*0.001, (double)y_pos[iframe]*0.001,
                    (double)z_pos[iframe]*0.001);

                new_meas_spot->scOrbitState.vsat.Set(
                    (double)x_vel[iframe]*0.001, (double)y_vel[iframe]*0.001,
                    (double)z_vel[iframe]*0.001);

                new_meas_spot->scAttitude.SetRPY(
                    roll[iframe], pitch[iframe], yaw[iframe]);

                if(do_footprint) {
                    Meas* new_meas = new Meas();
                    //...
                    new_meas_spot->Append(new_meas);
                } else {
                    for(int islice = 0; islice < nslices; ++islice) {
                        Meas* new_meas = new Meas();
                        //...
                        new_meas_spot->Append(new_meas);
                    }
                }
                l1b.frame.spotList.Append(new_meas_spot);
            }

            int this_frame = iframe;
            if(ipart==1) this_frame += nframes[0];

            l1b.frame.frame_i              = this_frame;
            l1b.frame.num_l1b_frames       = nframes[0] + nframes[1];
            l1b.frame.num_pulses_per_frame = nfootprints;
            l1b.frame.num_slices_per_pulse = -1;

            // Write this L1BFrame
            if(!l1b.WriteDataRec()) {
                fprintf(
                    stderr, "%s: writing to %s failed.\n", command, output_file);
                exit(1);
            }
        }
    }
    
    



    // Iterate 

    return(0);
}



