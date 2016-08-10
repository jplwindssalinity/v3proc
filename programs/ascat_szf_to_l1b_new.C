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
#include <stdlib.h>
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

    L1B l1b;
    if(!l1b.OpenForWriting(config_list.Get(L1B_TB_FILE_KEYWORD))) {
        fprintf(stderr, "%s: cannot open l1b file %s for output\n", command,
                output_file);
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

    ascat_szf_filename = config_list.Get("L1B_SZF_FILE");
    AscatFile ascat_file();
    int number_records, number_nodes;
    if(!ascat_file.open(scat_szf_filename, &number_records, &number_nodes)) {
        fprintf(
            stderr, "%s: ERROR opening ascat szf file %s\n", command,
            ascat_szf_filename); 
        exit(1);
    }

    for(int irecord = 0; irecord < number_recoreds; ++irecords) {

        // Read the next measurement data record from the SZF file, skip on
        // errors or dummy MDRs.
        int is_dummy_mdr;
        if(ascat_file.read_mdr(&is_dummy_mdr) || is_dummy_mdr)
            continue;

        // Iterate over nudes in this MDR
        for(int inode = 0; inode < number_nodes; ++inode) {

            // Get this SZF node from the MDR
            AscatSZFNodeNew ascat_szf_node();
            ascat_file.get_node_new(inode, &ascat_szf_node);

            printf(
                "%d %d %f %f %f\n", irecord, inode, ascat_szf_node.lat,
                ascat_szf_node.lon, ascat_szf_node.s0);

        }
    }

    // do stuff here!

    return(0);
}
