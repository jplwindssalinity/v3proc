//==============================================================//
// Copyright (C) 2017, California Institute of Technology.      //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//
#define QS_LANDMAP_FILE_KEYWORD "QS_LANDMAP_FILE"
#define QS_ICEMAP_FILE_KEYWORD  "QS_ICEMAP_FILE"
#define S1L1B_ASC_FILE_KEYWORD "S1L1B_ASC_FILE"
#define S1L1B_DEC_FILE_KEYWORD "S1L1B_DEC_FILE"
#define HH_BIAS_ADJ_KEYWORD "HH_BIAS_ADJ"
#define VV_BIAS_ADJ_KEYWORD "VV_BIAS_ADJ"

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
/* hdf5 include */
#include "hdf5.h"

using namespace std;
using std::list;
using std::map;

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

const char* usage_array[] = { "config_file", 0 };

int read_SDS_h5(hid_t obj_id, const char* sds_name, void* data_buffer) {
    hid_t sds_id = H5Dopen1(obj_id,sds_name);
    if( sds_id < 0 ) return(0);

    hid_t type_id = H5Dget_type(sds_id);
    if( type_id < 0 ) return(0);

    if( H5Dread( sds_id, type_id, H5S_ALL, H5S_ALL,H5P_DEFAULT, data_buffer) < 0 ||
        H5Dclose( sds_id ) < 0 ) return(0);
    return(1);
}

int read_attr_h5(hid_t obj_id, char* attr_name, void* data_buffer) {
    hid_t attr_id = H5Aopen(obj_id, attr_name, H5P_DEFAULT);
    if(attr_id < 0) return(0);

    hid_t attr_type = H5Aget_type(attr_id);
    if(attr_type < 0) return(0);

    if(H5Aread(attr_id, attr_type, data_buffer) < 0 || 
       H5Aclose(attr_id ) < 0) return(0);

    return(1);
}

int init_string( char* string, int length ) {
    for( int ii = 0; ii < length; ++ii )
        string[ii] = NULL;
    return(1);
}

int determine_n_scans(hid_t obj_id) {
    hid_t sds_id = H5Dopen1(obj_id, "Scan_number");
    if( sds_id < 0 ) return(0);
    hid_t space_id = H5Dget_space(sds_id);
    hsize_t dims[2], maxdims[2];
    hsize_t n_dims = H5Sget_simple_extent_dims(space_id, &dims[0], &maxdims[0]);
    if( n_dims == 0 ) return(0);
    return(dims[0]);
}

//--------------//
// MAIN PROGRAM //
//--------------//

int
main(int argc, char* argv[]) {

    const char* command = no_path(argv[0]);
    char* config_file = argv[1];
    int do_footprint = 0;

    optind = 2;
    while((optind < argc) && (argv[optind][0]=='-')) {
        std::string sw = argv[optind];
        if(sw == "--do-footprint" || sw == "-fp") {
            do_footprint = 1;
        } else {
            fprintf(stderr,"%s: Unknow option\n", command);
            exit(1);
        }
        ++optind;
    }

    ConfigList config_list;
    if(!config_list.Read(config_file)) {
        fprintf(
            stderr, "%s: error reading config file %s\n", command, config_file);
        exit(1);
    }

    // These ones are required
    config_list.ExitForMissingKeywords();

    char* l1b_files[2] = {NULL, NULL};
    l1b_files[0] = config_list.Get(S1L1B_ASC_FILE_KEYWORD);
    l1b_files[1] = config_list.Get(S1L1B_DEC_FILE_KEYWORD);
    printf("l1b_files: %s %s\n", l1b_files[0], l1b_files[1]);

    float bias_adj[2];
    config_list.GetFloat(VV_BIAS_ADJ_KEYWORD, &bias_adj[0]);
    config_list.GetFloat(HH_BIAS_ADJ_KEYWORD, &bias_adj[1]);

    Kp kp;
    int use_kprs = 0;
    int use_kpri = 0;
    ConfigKp(&kp, &config_list);
    config_list.GetInt(RETRIEVE_USING_KPRS_FLAG_KEYWORD, &use_kprs);
    config_list.GetInt(RETRIEVE_USING_KPRI_FLAG_KEYWORD, &use_kpri);

    QSLandMap qs_landmap;
    char* qslandmap_file = config_list.Get(QS_LANDMAP_FILE_KEYWORD);
    qs_landmap.Read(qslandmap_file);

    QSIceMap qs_icemap;
    char* qsicemap_file  = config_list.Get(QS_ICEMAP_FILE_KEYWORD);
    qs_icemap.Read(qsicemap_file);

    char* ephem_file = config_list.Get(EPHEMERIS_FILE_KEYWORD);
    Ephemeris ephem(ephem_file, 10000);

    L1B l1b;
    char* output_file = config_list.Get(L1B_FILE_KEYWORD);
    if (l1b.OpenForWriting(output_file) == 0) {
        fprintf(stderr, "%s: cannot open l1b file %s for output\n", command,
            output_file);
        exit(1);
    }

    // set up attenuation map
    char* attenmap_file = config_list.Get(ATTEN_MAP_FILE_KEYWORD);
    AttenMap attenmap;
    attenmap.ReadWentzAttenMap(attenmap_file);

    int nfootprints[2] = {281, 282};
    int nslices[2] = {9, 15};
    int nframes[2];

    for(int ipart=0; ipart<2; ++ipart) {

        hid_t h_id = H5Fopen(l1b_files[ipart], H5F_ACC_RDONLY, H5P_DEFAULT);
        hid_t g_id = H5Gopen(h_id, "science_data", H5P_DEFAULT);

        char attr_text[128];
        if(!read_attr_h5(g_id, "L1B Actual Scans", &attr_text)) {
            fprintf(stderr, "Error obtaining # of L1B Frames from HDF file!\n");
            exit(1);
        }
        nframes[ipart] = atoi(attr_text);

        float s0_scale;
        read_attr_h5(g_id, "Sigma0 Scale", &attr_text);
        s0_scale = atof(attr_text);

        char sst[nframes[ipart]][22];
        read_SDS_h5(g_id,"Scan_start_time", &sst[0]);

        std::vector< std::vector<uint16> > s0(2);
        std::vector< std::vector<uint16> > s0_flg(2);
        std::vector< std::vector<uint16> > snr(2);
        std::vector< std::vector<uint16> > xf(2);
        std::vector< std::vector<uint16> > kpa(2);
        std::vector< std::vector<uint16> > kpb(2);
        std::vector< std::vector<uint16> > kpc(2);
        std::vector< std::vector<uint16> > lat(2);
        std::vector< std::vector<uint16> > lon(2);
        std::vector< std::vector<uint16> > azi(2);
        std::vector< std::vector<uint16> > inc(2);
        std::vector< std::vector<uint16> > antazi(2);

        if(!do_footprint) {
            int fp_size = nframes[ipart] * nfootprints[0];
            // resize and load in data for inner beam footprints
            s0[0].resize(fp_size);
            s0_flg[0].resize(fp_size);
            snr[0].resize(fp_size);
            xf[0].resize(fp_size);
            kpa[0].resize(fp_size);
            kpb[0].resize(fp_size);
            kpc[0].resize(fp_size);
            lat[0].resize(fp_size);
            lon[0].resize(fp_size);
            azi[0].resize(fp_size);
            inc[0].resize(fp_size);
            antazi[0].resize(fp_size);
            read_SDS_h5(g_id, "Inner_beam_footprint_sigma0", &s0[0][0]);
            read_SDS_h5(
                g_id, "Inner_beam_footprint_sigma0_flag", &s0_flg[0][0]);
            read_SDS_h5(g_id, "Inner_beam_footprint_SNR", &snr[0][0]);
            read_SDS_h5(g_id, "Inner_beam_footprint_Xfactor", &xf[0][0]);
            read_SDS_h5(g_id, "Inner_beam_footprint_Kpa", &kpa[0][0]);
            read_SDS_h5(g_id, "Inner_beam_footprint_Kpb", &kpb[0][0]);
            read_SDS_h5(g_id, "Inner_beam_footprint_Kpc", &kpc[0][0]);
            read_SDS_h5(g_id, "Inner_beam_footprint_latitude", &lat[0][0]);
            read_SDS_h5(g_id, "Inner_beam_footprint_longitude", &lon[0][0]);
            read_SDS_h5(g_id, "Inner_beam_footprint_azimuth_angle", &azi[0][0]);
            read_SDS_h5(
                g_id, "Inner_beam_footprint_incidence_angle", &inc[0][0]);
            read_SDS_h5(
                g_id, "Inner_beam_footprint_antenna_azimuth_angle",
                &antazi[0][0]);


            fp_size = nframes[ipart] * nfootprints[1];
            // resize and load in data for outer beam footprints
            s0[1].resize(fp_size);
            s0_flg[1].resize(fp_size);
            snr[1].resize(fp_size);
            xf[1].resize(fp_size);
            kpa[1].resize(fp_size);
            kpb[1].resize(fp_size);
            kpc[1].resize(fp_size);
            lat[1].resize(fp_size);
            lon[1].resize(fp_size);
            azi[1].resize(fp_size);
            inc[1].resize(fp_size);
            antazi[1].resize(fp_size);
            read_SDS_h5(g_id, "Outer_beam_footprint_sigma0", &s0[1][0]);
            read_SDS_h5(
                g_id, "Outer_beam_footprint_sigma0_flag", &s0_flg[1][0]);
            read_SDS_h5(g_id, "Outer_beam_footprint_SNR", &snr[1][0]);
            read_SDS_h5(g_id, "Outer_beam_footprint_Xfactor", &xf[1][0]);
            read_SDS_h5(g_id, "Outer_beam_footprint_Kpa", &kpa[1][0]);
            read_SDS_h5(g_id, "Outer_beam_footprint_Kpb", &kpb[1][0]);
            read_SDS_h5(g_id, "Outer_beam_footprint_Kpc", &kpc[1][0]);
            read_SDS_h5(g_id, "Outer_beam_footprint_latitude", &lat[1][0]);
            read_SDS_h5(g_id, "Outer_beam_footprint_longitude", &lon[1][0]);
            read_SDS_h5(g_id, "Outer_beam_footprint_azimuth_angle", &azi[1][0]);
            read_SDS_h5(
                g_id, "Outer_beam_footprint_incidence_angle", &inc[1][0]);
            read_SDS_h5(
                g_id, "Outer_beam_footprint_antenna_azimuth_angle",
                &antazi[1][0]);

        } else {
            // resize and load in data for inner beam slices
            int slice_size = nslices[0]*nframes[ipart]*nfootprints[0];
            s0[0].resize(slice_size);
            s0_flg[0].resize(slice_size);
            snr[0].resize(slice_size);
            xf[0].resize(slice_size);
            kpa[0].resize(slice_size);
            kpb[0].resize(slice_size);
            kpc[0].resize(slice_size);
            lat[0].resize(slice_size);
            lon[0].resize(slice_size);
            azi[0].resize(slice_size);
            inc[0].resize(slice_size);
            antazi[0].resize(slice_size);
            read_SDS_h5(g_id, "Inner_beam_slice_sigma0", &s0[0][0]);
            read_SDS_h5(
                g_id, "Inner_beam_slice_sigma0_flag", &s0_flg[0][0]);
            read_SDS_h5(g_id, "Inner_beam_slice_SNR", &snr[0][0]);
            read_SDS_h5(g_id, "Inner_beam_slice_Xfactor", &xf[0][0]);
            read_SDS_h5(g_id, "Inner_beam_slice_Kpa", &kpa[0][0]);
            read_SDS_h5(g_id, "Inner_beam_slice_Kpb", &kpb[0][0]);
            read_SDS_h5(g_id, "Inner_beam_slice_Kpc", &kpc[0][0]);
            read_SDS_h5(g_id, "Inner_beam_slice_latitude", &lat[0][0]);
            read_SDS_h5(g_id, "Inner_beam_slice_longitude", &lon[0][0]);
            read_SDS_h5(g_id, "Inner_beam_slice_azimuth_angle", &azi[0][0]);
            read_SDS_h5(
                g_id, "Inner_beam_slice_incidence_angle", &inc[0][0]);
            read_SDS_h5(
                g_id, "Inner_beam_slice_antenna_azimuth_angle",
                &antazi[0][0]);

            // resize and load in data for outer beam slices
            slice_size = nslices[1]*nframes[ipart]*nfootprints[1];
            s0[1].resize(slice_size);
            s0_flg[1].resize(slice_size);
            snr[1].resize(slice_size);
            xf[1].resize(slice_size);
            kpa[1].resize(slice_size);
            kpb[1].resize(slice_size);
            kpc[1].resize(slice_size);
            lat[1].resize(slice_size);
            lon[1].resize(slice_size);
            azi[1].resize(slice_size);
            inc[1].resize(slice_size);
            antazi[1].resize(slice_size);
            read_SDS_h5(g_id, "Outer_beam_slice_sigma0", &s0[1][0]);
            read_SDS_h5(
                g_id, "Outer_beam_slice_sigma0_flag", &s0_flg[1][0]);
            read_SDS_h5(g_id, "Outer_beam_slice_SNR", &snr[1][0]);
            read_SDS_h5(g_id, "Outer_beam_slice_Xfactor", &xf[1][0]);
            read_SDS_h5(g_id, "Outer_beam_slice_Kpa", &kpa[1][0]);
            read_SDS_h5(g_id, "Outer_beam_slice_Kpb", &kpb[1][0]);
            read_SDS_h5(g_id, "Outer_beam_slice_Kpc", &kpc[1][0]);
            read_SDS_h5(g_id, "Outer_beam_slice_latitude", &lat[1][0]);
            read_SDS_h5(g_id, "Outer_beam_slice_longitude", &lon[1][0]);
            read_SDS_h5(g_id, "Outer_beam_slice_azimuth_angle", &azi[1][0]);
            read_SDS_h5(
                g_id, "Outer_beam_slice_incidence_angle", &inc[1][0]);
            read_SDS_h5(
                g_id, "Outer_beam_slice_antenna_azimuth_angle",
                &antazi[1][0]);
        }
        H5Gclose(g_id);
        H5Fclose(h_id);

        for(int iframe=0; iframe < nframes[ipart]; ++iframe) {

            char frame_time_str[64];
            init_string(frame_time_str, 64);
            strcpy(frame_time_str, sst[iframe]);

            // Unix time is the epoch time for QSCATsim software.
            ETime etime;
            etime.FromCodeB("1970-001T00:00:00.000");
            double time_base = 
                (double)etime.GetSec() + (double)etime.GetMs()/1000;
            if(!etime.FromCodeB(frame_time_str)) {
              fprintf(stderr, 
                "l1b_hdf_to_l1b: Error: could not parse time string: %s\n",
                frame_time_str);
              exit(1);
            }

            double frame_time = 
                (double)etime.GetSec() + (double)etime.GetMs()/1000 - time_base;

            // Get seconds of year for attenuation map.
            int year = atoi(strtok(&frame_time_str[0], "-"));
            int doy = atoi(strtok(NULL, "T"));
            int hour = atoi(strtok(NULL, ":"));
            int minute = atoi(strtok(NULL, ":"));
            float seconds = atof(strtok(NULL, "\0"));

            double sec_year = seconds+60.0*(
                float(minute)+60.0*(float(hour)+24.0*float(doy-1)));

            l1b.frame.spotList.FreeContents();
            for(int ifootprint=0; ifootprint < 282; ++ifootprint) {

                for(int ipol = 0; ipol < 2; ++ipol) {

                    // HH only has 281 footprints
                    if(ifootprint == nfootprints[ipol]-1)
                        continue;

                    MeasSpot* new_meas_spot = new MeasSpot();
                    new_meas_spot->time = frame_time;
                    new_meas_spot->scOrbitState.time = frame_time;

                    ephem.GetOrbitState(
                        frame_time, EPHEMERIS_INTERP_ORDER,
                        &new_meas_spot->scOrbitState);

                    new_meas_spot->scAttitude.SetRPY(0, 0, 0);

                    uint16 FLAG_MASK = uint16(0xF0); // bits 4,5,6,7
                    uint16 ICE_MASK = uint16(0x2000); // bit 13
                    uint16 ICE_VALID = uint16(0x4000); // bit 14
                    uint16 LAND_MASK = uint16(0x8); // bit 3
                    uint16 NEG_S0_MASK = uint16(0x200); // bit 9
                    uint16 ASC_MASK = uint16(0x1); // bit 0
                    uint16 FORE_MASK = uint16(0x4); // bit 2
                    uint16 MIXED_MASK = uint16(0x100); // bit 8

                    for(int islice = 0; islice < nslices[ipart]; ++islice) {

                        // islice is a dummy index if do_footprint is true
                        if(do_footprint && islice > 0)
                            continue;

                        int data_idx;
                        if(do_footprint) {
                            data_idx = iframe * nfootprints[ipol] + ifootprint;
                        } else {
                            data_idx =
                                iframe * nfootprints[ipart] * nslices[ipart] +
                                ifootprint * nslices[ipart] + islice;
                        }

                        // check flags
                        if(FLAG_MASK & s0_flg[ipol][data_idx])
                            continue;

                        Meas* new_meas = new Meas();

                        double this_xf = -120+0.000613*double(xf[ipol][data_idx]);
                        double this_s0 = -96+s0_scale*double(s0[ipol][data_idx]);
                        double this_snr = -65+0.001547*double(snr[ipol][data_idx]);

                        new_meas->XK = pow(10.0, 0.1*this_xf);
                        new_meas->EnSlice = pow(
                            10.0, 0.1*(this_s0+this_xf-this_snr));

                        double tmp_lon = dtr*(
                            0.005515*double(lon[ipol][data_idx]));

                        double tmp_lat = dtr*(
                            -90+0.002757*double(lat[ipol][data_idx]));

                        // make longitude, latitude to be in range.
                        if(tmp_lon < 0) tmp_lon += two_pi;
                        if(tmp_lon >= two_pi) tmp_lon -= two_pi;
                        if(tmp_lat < -pi/2) tmp_lat  = -pi/2;
                        if(tmp_lat >  pi/2) tmp_lat  =  pi/2;
                        new_meas->centroid.SetAltLonGDLat(0.0, tmp_lon, tmp_lat);

                        new_meas->incidenceAngle = dtr*(
                            46+0.0002451*double(inc[ipol][data_idx]));

                        float atten_dB = 0;
                        atten_dB = attenmap.GetNadirAtten(
                            tmp_lon, tmp_lat, sec_year)/
                            cos(new_meas->incidenceAngle);
                        float atten_lin = pow(10.0,0.1*atten_dB);

                        float northAzimuth = dtr*0.005515*double(
                            azi[ipol][data_idx]);

                        new_meas->eastAzimuth = (450.0*dtr - northAzimuth);
                        if(new_meas->eastAzimuth >= two_pi)
                            new_meas->eastAzimuth -= two_pi;

                        new_meas->value = pow(10.0, 0.1*this_s0);
                        if(s0_flg[ipol][data_idx] & NEG_S0_MASK)
                            new_meas->value *= -1;

                        new_meas->scanAngle = dtr*0.005515*double(
                            antazi[ipol][data_idx]);

                        new_meas->measType = (ipol == 0) ?
                            Meas::HH_MEAS_TYPE : Meas::VV_MEAS_TYPE;

                        new_meas->beamIdx = ipol;

                        new_meas->landFlag = 0;
                        if(qs_landmap.IsLand(tmp_lon, tmp_lat, 0))
                            new_meas->landFlag += 1; // bit 0 for land

                        if(qs_icemap.IsIce(tmp_lon, tmp_lat, 0))
                            new_meas->landFlag += 2; // bit 1 for ice

                        if(do_footprint) {
                            // do footprint specific stuff
                            new_meas->numSlices = -1;
                            new_meas->startSliceIdx = -1;
                            if(ipol == 0) {
                                new_meas->azimuth_width = 27;
                                new_meas->range_width   = 45;
                            } else {
                                new_meas->azimuth_width = 30;
                                new_meas->range_width   = 68;
                            }

                            double sos = pow( 10.0, 0.1*(this_s0-this_snr) );

                            double kprs2 = 0.0;
                            if(use_kprs && !kp.GetKprs2(new_meas, &kprs2)) {
                                fprintf(stderr, "%s: Error computing Kprs2\n",
                                    command);
                                exit(1);
                            }

                            double kpri2 = 0.0;
                            if(use_kpri && !kp.GetKpri2(&kpri2) ) {
                                fprintf(stderr,"%s: Error computing Kpri2\n",
                                    command);
                                exit(1);
                            }

                            double kpr2 = 1 + kprs2 + kpri2;

                            double kpA = 0.0000154*double(kpa[ipol][data_idx]);
                            double kpB = 0.0000154*double(kpb[ipol][data_idx]);
                            double kpC = 0.0000154*double(kpc[ipol][data_idx]);

                            double kp_alpha = (1+kpA) * kpr2;
                            double kp_beta  = kpB * kpr2 * sos;
                            double kp_gamma = kpC * kpr2 * sos * sos;

                            // Set kp alpha, beta, gamma and correct for attenuation
                            new_meas->A = 1 + (kp_alpha - 1)*atten_lin*atten_lin;
                            new_meas->B = kp_beta*atten_lin*atten_lin;
                            new_meas->C = kp_gamma*atten_lin*atten_lin;
                        
                        } else {
                            // do slice specific stuff
                            new_meas->numSlices = 1;
                            new_meas->startSliceIdx = islice;

                            if(ipol == 0) {
                                new_meas->azimuth_width = 27;
                                new_meas->range_width   = 45/9;
                            } else {
                                new_meas->azimuth_width = 30;
                                new_meas->range_width   = 68/15;
                            }

                            new_meas->A = 0.0000154*double(kpa[ipol][data_idx]);
                            new_meas->B = 0.0000154*double(kpb[ipol][data_idx]);
                            new_meas->C = 0.0000154*double(kpc[ipol][data_idx]);
                        }
 
                        // Stick this meas in the measSpot
                        new_meas_spot->Append(new_meas);
                    }
                    l1b.frame.spotList.Append(new_meas_spot);
                }  // ipol loop
            }  // ifootprint loop

            if(l1b.frame.spotList.NodeCount() == 0)
                continue;

            int this_frame = iframe;
            if(ipart==1) this_frame += nframes[0];

            l1b.frame.frame_i              = this_frame;
            l1b.frame.num_l1b_frames       = nframes[0] + nframes[1];
            l1b.frame.num_pulses_per_frame = 282 + 281;
            l1b.frame.num_slices_per_pulse = -1;
            l1b.WriteDataRec();
        }  // iframe loop
    } // ipart loop
}
