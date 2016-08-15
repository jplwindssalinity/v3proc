//==============================================================//
// Copyright (C) 2016, California Institute of Technology.	    //
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

#define QS_LANDMAP_FILE_KEYWORD "QS_LANDMAP_FILE"
#define QS_ICEMAP_FILE_KEYWORD  "QS_ICEMAP_FILE"
#define REV_START_TIME_KEYWORD "REV_START_TIME"
#define REV_STOP_TIME_KEYWORD "REV_STOP_TIME"
#define COASTAL_DISTANCE_FILE_KEYWORD "COASTAL_DISTANCE_FILE"

#include <stdio.h>
#include <stdlib.h>make -
#include <string>
#include "List.h"
#include "CoastDistance.h"
#include "BufferedList.h"
#include "Misc.h"
#include "ConfigList.h"
#include "ConfigSim.h"
#include "L1B.h"
#include "Meas.h"
#include "Tracking.h"
#include "SpacecraftSim.h"
#include "AscatL1bReader.h"
#include "ETime.h"

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

int main(int argc, char* argv[]) {

    const char* command = no_path(argv[0]);
    char* config_file = argv[1];

    ConfigList config_list;
    if(!config_list.Read(config_file)) {
        fprintf(
            stderr, "%s: error reading config file %s\n", command, config_file);
        exit(1);
    }

    config_list.ExitForMissingKeywords();

    QSLandMap qs_landmap;
    qs_landmap.Read(config_list.Get(QS_LANDMAP_FILE_KEYWORD));

    QSIceMap qs_icemap;
    qs_icemap.Read(config_list.Get(QS_ICEMAP_FILE_KEYWORD));

    CoastDistance coast_dist;
    coast_dist.Read(config_list.Get(COASTAL_DISTANCE_FILE_KEYWORD));

    Ephemeris ephem(config_list.Get(EPHEMERIS_FILE_KEYWORD), 10000);

    int do_composite = 0;
    config_list.GetInt("USE_COMPOSITING", &do_composite);

    L1B l1b;
    if(!l1b.OpenForWriting(config_list.Get(L1B_FILE_KEYWORD))) {
        fprintf(stderr, "%s: cannot open l1b file for output\n", command);
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

    etime.FromCodeA("2000-01-01T00:00:00.000");
    double ascat_base = (double)etime.GetSec()+(double)etime.GetMs()/1000;

    AscatFile ascat_file;
    int number_records, number_nodes;
    if(ascat_file.open(
        config_list.Get("L1B_SZF_FILE"), &number_records, &number_nodes)) {
        fprintf(stderr, "%s: ERROR opening ascat szf file\n", command); 
        exit(1);
    }

    printf(
        "version, n_recs, n_nodes: %d %d %d\n", ascat_file.fmt_maj,
        number_records, number_nodes);

    if(ascat_file.fmt_maj < 12) {
        fprintf(stderr, "%s only for format >= 12\n", command);
        exit(1);
    }

    l1b.frame.spotList.FreeContents();
    for(int irecord = 0; irecord < number_records; ++irecord) {
        // Read the next measurement data record from the SZF file, skip on
        // errors or dummy MDRs.
        int is_dummy_mdr = 0;
        if(ascat_file.read_mdr(&is_dummy_mdr) || is_dummy_mdr)
            continue;

        AscatSZFNodeNew ascat_szf_node_first;
        ascat_file.get_node_new(0, &ascat_szf_node_first);

        double time = ascat_szf_node_first.tm*86400 + ascat_base;

        MeasSpot* new_meas_spot = new MeasSpot();

        new_meas_spot->time = time;
        new_meas_spot->scOrbitState.time = time;
        ephem.GetOrbitState(
            time, EPHEMERIS_INTERP_ORDER,
            &new_meas_spot->scOrbitState);
        new_meas_spot->scAttitude.SetRPY(0, 0, 0);

        // Iterate over nudes in this MDR
        for(int inode = 0; inode < number_nodes; ++inode) {

            // Get this SZF node from the MDR
            AscatSZFNodeNew ascat_szf_node;
            ascat_file.get_node_new(inode, &ascat_szf_node);

            // Check quality
            if(ascat_szf_node.is_bad)
                continue;

            Meas* new_meas = new Meas();

            new_meas->measType = Meas::C_BAND_VV_MEAS_TYPE;
            new_meas->value = ascat_szf_node.s0;
            new_meas->XK = 1.0;

            double tmp_lon = dtr*ascat_szf_node.lon;
            double tmp_lat = dtr*ascat_szf_node.lat;
            if(tmp_lon<0) tmp_lon += two_pi;

            new_meas->centroid.SetAltLonGDLat(0.0, tmp_lon, tmp_lat);
            new_meas->incidenceAngle = dtr*ascat_szf_node.t0;
//             new_meas->eastAzimuth = (450.0*dtr - dtr*ascat_szf_node.a0);
            new_meas->eastAzimuth = gs_deg_to_pe_rad(ascat_szf_node.a0);
            new_meas->numSlices = 1;
            new_meas->startSliceIdx = inode;
            new_meas->beamIdx = ascat_szf_node.beam - 1;
            new_meas->azimuth_width = 20; // [km]; from documentation
            new_meas->range_width = 10; // [km]; from documentation
            new_meas->A = 1;
            new_meas->B = 0;
            new_meas->C = 0;
            new_meas->landFlag = 0;

            double distance;
            coast_dist.Get(tmp_lon, tmp_lat, &distance);

            if(distance < 30)
                new_meas->landFlag += 1; // bit 0 for land

            if(qs_icemap.IsIce(tmp_lon, tmp_lat, 0))
                new_meas->landFlag += 2; // bit 1 for ice

            new_meas_spot->Append(new_meas);
        }

        if(new_meas_spot->NodeCount() > 0)
            l1b.frame.spotList.Append(new_meas_spot);
        else
            delete new_meas_spot;

        if(ascat_szf_node_first.beam == 5) {

            // Dont write out empty L1B data records
            if(l1b.frame.spotList.NodeCount() > 0)
                l1b.WriteDataRec();

            l1b.frame.spotList.FreeContents();
        }

        if(irecord % 1000 == 0) {
            printf(
                "Wrote %d of %d pulses; beam %d\n", irecord, number_records,
                ascat_szf_node_first.beam);
        }
    }
    ascat_file.close();
    return(0);
}
