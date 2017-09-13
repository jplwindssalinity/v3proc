//==============================================================//
// Copyright (C) 2017, California Institute of Technology.      //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//      l1b_to_l2a_collocate
//
//      Does the L1B to L2A processing of an L1B file from another platform
//      onto the L2A swath grid of a first platform.
//
// USEAGE
//      l1b_to_l2a_collocate config_file first_platform_ephemfile
//
// NOTES
//      config_file must have "EPHEMERIS_FILE" keyword set so it points to
//      the other platforms ephemeris file.
//
//----------------------------------------------------------------------

//----------//
// INCLUDES //
//----------//

#include <stdio.h>
#include <stdlib.h>
#include "List.h"
#include "BufferedList.h"
#include "Misc.h"
#include "ConfigList.h"
#include "ConfigSim.h"
#include "Ephemeris.h"
#include "L1B.h"
#include "L2A.h"
#include "L1BToL2A.h"
#include "Tracking.h"
#include "ETime.h"

using std::list;
using std::map; 

//-----------//
// TEMPLATES //
//-----------//

// Class declarations needed for templates
// eliminates need to include the entire header file
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

int main(int argc, char* argv[]) {

    const char* command = no_path(argv[0]);
    const char* config_file = argv[1];
    const char* this_ephem_file = argv[2];

    ConfigList config_list;
    config_list.Read(config_file);
    config_list.ExitForMissingKeywords();

    const char* other_ephem_file = config_list.Get("EPHEMERIS_FILE");
    Ephemeris this_ephem(this_ephem_file, 50000);
    Ephemeris other_ephem(other_ephem_file, 50000);

    int use_compositing;
    config_list.GetInt("USE_COMPOSITING", &use_compositing);

    Grid grid;
    ConfigGrid(&grid, &config_list);

    config_list.WarnForMissingKeywords();

    if(grid.algorithm != Grid::SOM) {
        fprintf(stderr, "Program only for use with SOM gridding!\n");
        exit(1);
    }

    ETime etime;
    etime.FromCodeB("1970-001T00:00:00.000");
    double time_base = (double)etime.GetSec() + (double)etime.GetMs()/1000;

    etime.FromCodeB(config_list.Get("REV_START_TIME"));
    double other_rev_start =
        (double)etime.GetSec() + (double)etime.GetMs()/1000 - time_base;

    etime.FromCodeB(config_list.Get("REV_STOP_TIME"));
    double other_rev_stop =
        (double)etime.GetSec() + (double)etime.GetMs()/1000 - time_base;


    grid.SetStartTime(other_rev_start);
    grid.SetEndTime(other_rev_stop);
    grid.lat_end_time = other_rev_stop;

    grid.l1b.OpenForReading();
    grid.l2a.OpenForWriting();


    int nSpotSize = 4;
    int timeSize = 8;
    int scOSSize = 56;
    int scAttSize = 15; // 3 float and 3 char (order of rpy)
    int nMeasSize = 4;
    int spotSize = timeSize + scOSSize + scAttSize;
    int nSpot = 0;
    int nm = 0;
    off_t tmp = 0;

    /* find byte length of a measurement from a spot list with */
    /* positive number of measurements                         */
    while (nm == 0) {
        tmp = ftello(grid.l1b.GetInputFp());
        if (!grid.l1b.ReadDataRec()) {
            fprintf(stderr, "%s: error reading Level 1B data\n", command);
            exit(1);
        }

        /* find out number of spots in the spotlist */
        nSpot = grid.l1b.frame.spotList.NodeCount();

          /* find out total number of measurements for all spots */
        for (MeasSpot* meas_spot = grid.l1b.frame.spotList.GetHead();
             meas_spot; meas_spot = grid.l1b.frame.spotList.GetNext()) {
            nm += meas_spot->NodeCount();
        }
    }
    int sumSize = nSpotSize + nSpot*(spotSize+nMeasSize);
    grid.meas_length = (int)((ftello(grid.l1b.GetInputFp())-tmp-sumSize)/nm);
    fseeko(grid.l1b.GetInputFp(), 0, SEEK_SET);

    int counter = 0;
    int spot_counter = 0;
    int meas_counter = 0;

    double other_subtrack_start_time = (other_rev_start + other_rev_stop)/2;
    double other_time = other_subtrack_start_time;

    OrbitState other_os;
    other_ephem.GetOrbitState(
        other_subtrack_start_time, EPHEMERIS_INTERP_ORDER, &other_os);
    EarthPosition other_subtrack_start = other_os.rsat.Nadir();

    while (grid.l1b.ReadDataRec()) {

        if(counter % 100 == 0)
            fprintf(stderr,"L1B record count = %ld\n", counter);

        MeasSpotList* meas_spot_list = &grid.l1b.frame.spotList;
        for(MeasSpot* meas_spot = meas_spot_list->GetHead(); meas_spot;
            meas_spot = meas_spot_list->GetNext()) {

            double spot_time = meas_spot->time;
            double other_ephem_time;

            // don't collocate more than 3000 seconds apart
            if (spot_time < other_rev_start - 50*60 ||
                spot_time > other_rev_stop + 50*60)
                continue;

            // Interpolate this_ephem to spot_time
            OrbitState this_os;
            this_ephem.GetOrbitState(
                spot_time, EPHEMERIS_INTERP_ORDER, &this_os);

            // Compute nadir point of platform
            EarthPosition this_nadir_point = this_os.rsat.Nadir();

            // Find time in other_ephem such that the nadir point of other_ephem
            // at that time is nearest to this_nadir_point.
            // Iterativly solve for best time via minizing difference between
            // along-track coordinate of other_nadir_point and this_nadir_point
            // using the subtrack coordinate methods in the other_ephem class
            // instance.
            int num_iters = 0;
            float atd, ctd, atd_nadir, ctd_nadir;
            while(num_iters < 10 && fabs(atd_nadir - atd) > 1) {

                double vground = other_os.vsat.Magnitude() *
                    r1_earth/other_os.rsat.Magnitude();

                // solution increment (not first time through)
                if(num_iters != 0) {
                    other_time -= (atd - atd_nadir) / vground;

                    if(other_time > other_rev_stop) {
                        other_time = other_rev_stop;
                        break;
                    }

                    if(other_time < other_rev_start) {
                        other_time = other_rev_start;
                        break;
                    }
                }

                // Compute other_ephem nadir point at that time
                other_ephem.GetOrbitState(
                    other_time, EPHEMERIS_INTERP_ORDER, &other_os);
                EarthPosition other_nadir_point = other_os.rsat.Nadir();

                // Compute the cross-track / along-track coordinates of
                // this_nadir_point and other_nadir_point.
                other_ephem.GetSubtrackCoordinates(
                    this_nadir_point, other_subtrack_start,
                    other_subtrack_start_time, other_time, &ctd, &atd);

                other_ephem.GetSubtrackCoordinates(
                    other_nadir_point, other_subtrack_start,
                    other_subtrack_start_time, other_time, &ctd_nadir,
                    &atd_nadir);

//                 printf(
//                     "ii, atd, atd_nadir, other_time: %d %f %f %f\n",
//                     num_iters, atd, atd_nadir, other_time);

                num_iters++;
            }

            for(Meas* meas = meas_spot->GetHead(); meas;
                meas = meas_spot->GetNext()) {

                // Add it if it aint crazy
                if (isnan(meas->value) || (meas->range_width > 1000) ||
                    meas->azimuth_width > 1000)
                    continue;

                grid.Add(meas, other_time, spot_counter, use_compositing, 1);
                meas_counter++;
            }
            spot_counter++;
        }
        counter++;
    }

    grid.Flush(use_compositing);
    grid.l1b.Close();
    grid.l2a.Close();
    return(0);
}
