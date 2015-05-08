//==============================================================//
// Copyright (C) 2015, California Institute of Technology.      //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#define L1B_TB_LORES_ASC_FILE_KEYWORD "L1B_TB_LORES_ASC_FILE"
#define L1B_TB_LORES_DEC_FILE_KEYWORD "L1B_TB_LORES_DEC_FILE"
#define L1B_TB_FILE_KEYWORD "L1B_TB_FILE"
#define QS_LANDMAP_FILE_KEYWORD "QS_LANDMAP_FILE"
#define QS_ICEMAP_FILE_KEYWORD  "QS_ICEMAP_FILE"
#define REV_START_TIME_KEYWORD "REV_START_TIME"
#define REV_STOP_TIME_KEYWORD "REV_STOP_TIME"
#define TBH_ADJUST_KEYWORD "TBH_ADJUST"
#define TBV_ADJUST_KEYWORD "TBV_ADJUST"

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

    float tb_adjust[2] = {0, 0};
    config_list.GetFloat(TBV_ADJUST_KEYWORD, &tb_adjust[0]);
    config_list.GetFloat(TBH_ADJUST_KEYWORD, &tb_adjust[1]);

    QSLandMap qs_landmap;
    char* qslandmap_file = config_list.Get(QS_LANDMAP_FILE_KEYWORD);
    qs_landmap.Read(qslandmap_file);

    QSIceMap qs_icemap;
    char* qsicemap_file  = config_list.Get(QS_ICEMAP_FILE_KEYWORD);
    qs_icemap.Read(qsicemap_file);

    char* ephem_file = config_list.Get(EPHEMERIS_FILE_KEYWORD);
    Ephemeris ephem(ephem_file, 10000);

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

        std::vector< std::vector<float> > tb(2);
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

        // Iterate over scans
        for(int iframe = 0; iframe < nframes[ipart]; ++iframe) {
            l1b.frame.spotList.FreeContents();

            // Copy over time-strings, null terminate and parse
            char time_str[24];
            init_string(time_str, 24);

            // only copy first 23 chars (last one is Z)
            strncpy(time_str, antenna_scan_time_utc[iframe], 23);

            etime.FromCodeA(time_str);
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

                for(int ipol=0; ipol<2; ++ipol){

                   MeasSpot* new_meas_spot = new MeasSpot();

                   // Is this good enough (interpolate within each scan)
                   new_meas_spot->time = time;

                   new_meas_spot->scOrbitState.time = time;

                   ephem.GetOrbitState(
                       time, EPHEMERIS_INTERP_ORDER,
                       &new_meas_spot->scOrbitState);

                   new_meas_spot->scAttitude.SetRPY(
                       dtr*roll[iframe], dtr*pitch[iframe], dtr*yaw[iframe]);

                    // check flags
                    if(0x1 & tb_flag[ipol][fp_idx])
                        continue;

                    Meas* new_meas = new Meas();

                    if(ipol==0)
                        new_meas->measType = Meas::L_BAND_TBV_MEAS_TYPE;
                    else
                        new_meas->measType = Meas::L_BAND_TBH_MEAS_TYPE;

                    new_meas->value = tb[ipol][fp_idx] + tb_adjust[ipol];
                    new_meas->XK = 1.0;

                    // Placeholder for surface temperature
                    double ts = 300.0;
                    new_meas->EnSlice = ts;

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
                    new_meas->azimuth_width = 39;
                    new_meas->range_width = 47;

                    if(qs_landmap.IsLand(tmp_lon, tmp_lat, 0))
                        new_meas->landFlag += 1; // bit 0 for land

                    if( qs_icemap.IsIce(tmp_lon, tmp_lat, 0) )
                        new_meas->landFlag += 2; // bit 1 for ice

                    // Need to figure out the KP (a, b, c) terms.
                    new_meas->A = pow(nedt[ipol][fp_idx], 2);
                    new_meas->B = 0.0;
                    new_meas->C = 0.0;

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
