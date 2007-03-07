//==============================================================//
// Copyright (C) 1998-2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    l2b_extract_worst
//
// SYNOPSIS
//    l2b_diff_from_truth <config_file>
//
// DESCRIPTION
//    Locates worst wind vectors in the file
//    for each cross_track_index.
//
// OPTIONS
//    None.
//
// OPERANDS
//    The following operands are supported:
//      <config_file>  Simulation Configuration file
//
// EXAMPLES
//    An example of a command line is:
//      % l2b_extract_worst quikscat.cfg
//
// ENVIRONMENT
//    Not environment dependent.
//
// EXIT STATUS
//    The following exit values are returned:
//       0  Program executed successfully
//      >0  Program had an error
//
// NOTES
//    None.
//
// AUTHOR
//    James N. Huddleston (hudd@acid.jpl.nasa.gov)
//----------------------------------------------------------------------

//-----------------------//
// Configuration Control //
//-----------------------//

static const char rcs_id[] =
    "@(#) $Id$";

//----------//
// INCLUDES //
//----------//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Misc.h"
#include "ConfigList.h"
#include "L2A.h"
#include "ConfigSim.h"
#include "ConfigSimDefs.h"
#include "L2B.h"
#include "L2AToL2B.h"
#include "Tracking.h"
#include "Tracking.C"

using std::list;
using std::map; 

//-----------//
// TEMPLATES //
//-----------//

// Class declarations needed for templates
// eliminates need to include the entire header file
class AngleInterval;

template class List<AngleInterval>;
template class List<StringPair>;
template class List<Meas>;
template class List<EarthPosition>;
template class List<WindVectorPlus>;
template class List<MeasSpot>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<long>;
template class List<off_t>;
template class List<OffsetList>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;
template class std::list<string>;
template class std::map<string,string,Options::ltstr>;

//-----------//
// CONSTANTS //
//-----------//

//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "<config_file>", "<hdf_flag (1=use HDF, default=0)",
    "<compare_nudge_flag (1=compare to nudge, 0=default)>",  0};

//--------------//
// MAIN PROGRAM //
//--------------//

int
main(
    int    argc,
    char*  argv[])
{
    //------------------------//
    // parse the command line //
    //------------------------//

    const char* command = no_path(argv[0]);
    if (argc != 2 && argc !=3 && argc!=4)
        usage(command, usage_array, 1);

    int clidx = 1;
    const char* config_file = argv[clidx++];
        int use_hdf=0;
        int compare_nudge=0;
        if(argc>=3){
      use_hdf=atoi(argv[clidx++]);
      if(argc==4){
        compare_nudge=atoi(argv[clidx++]);
      }
    }

    //---------------------//
    // read in config file //
    //---------------------//

    ConfigList config_list;
    if (! config_list.Read(config_file))
    {
        fprintf(stderr, "%s: error reading sim config file %s\n",
            command, config_file);
        exit(1);
    }

    //-------------------------------------//
    // create and configure level products //
    //-------------------------------------//

    L2B l2b;
    if (! ConfigL2B(&l2b, &config_list))
    {
        fprintf(stderr, "%s: error configuring Level 2B Product\n", command);
        exit(1);
    }

    //-------------------------//
    // read in truth windfield //
    //-------------------------//

    WindField truth;
    if (! compare_nudge) {
        char* truth_type = config_list.Get(TRUTH_WIND_TYPE_KEYWORD);
        if (truth_type == NULL) {
            fprintf(stderr, "%s: must specify truth windfield type\n", command);
            exit(1);
        }

        if (strcasecmp(truth_type, "SV") == 0)
        {
            if (!config_list.GetFloat(WIND_FIELD_LAT_MIN_KEYWORD, &truth.lat_min) ||
                !config_list.GetFloat(WIND_FIELD_LAT_MAX_KEYWORD, &truth.lat_max) ||
                !config_list.GetFloat(WIND_FIELD_LON_MIN_KEYWORD, &truth.lon_min) ||
                !config_list.GetFloat(WIND_FIELD_LON_MAX_KEYWORD, &truth.lon_max))
            {
              fprintf(stderr, "%s: SV can't determine range of lat and lon\n", command);
              return(0);
            }
        }

        char* truth_file = config_list.Get(TRUTH_WIND_FILE_KEYWORD);
        if (truth_file == NULL) {
            fprintf(stderr, "%s: must specify truth windfield file\n", command);
            exit(1);
        }

        truth.ReadType(truth_file, truth_type);

       //--------------------------//
       // use as fixed wind speed? //
       //--------------------------//

       config_list.DoNothingForMissingKeywords();
       float fixed_speed; 
       if (config_list.GetFloat(TRUTH_WIND_FIXED_SPEED_KEYWORD, &fixed_speed))
       {
           truth.FixSpeed(fixed_speed);
       } 
       float fixed_direction;
       if (config_list.GetFloat(TRUTH_WIND_FIXED_DIRECTION_KEYWORD, &fixed_direction))
       { 
           fixed_direction *= dtr;
           truth.FixDirection(fixed_direction);
       }
       config_list.ExitForMissingKeywords();

    }

    //------------------//
    // read in l2b file //
    //------------------//

        if (use_hdf){
      if (l2b.ReadHDF()== 0)
        {
          fprintf(stderr, "%s: cannot open HDF file for input\n",
                                 argv[0]);
          exit(1);
        }
    }
        else{
      if (! l2b.OpenForReading())
        {
          fprintf(stderr, "%s: error opening L2B file\n",command);
          exit(1);
        }
      if (! l2b.ReadHeader())
        {
          fprintf(stderr, "%s: error reading L2B header from file \n",
              command);
          exit(1);
        }

      if (! l2b.ReadDataRec())
        {
          fprintf(stderr, "%s: error reading L2B data record from file \n",
              command);
          exit(1);
        }
    }

    //----------------------//
        // compute difference & //
    // write out vctr files //
    //----------------------//
        int ctbins=l2b.frame.swath.GetCrossTrackBins();
        int atbins=l2b.frame.swath.GetAlongTrackBins();
    for(int cti=0;cti<ctbins;cti++){
      WindVector true_wv;
      for(int ati=0;ati<atbins;ati++){
        WVC* wvc=l2b.frame.swath.swath[cti][ati];
        if(!wvc) continue;
            if (compare_nudge){
          if(! wvc->nudgeWV) continue;
              true_wv.dir=wvc->nudgeWV->dir;
              true_wv.spd=wvc->nudgeWV->spd;
        }
        else if (! truth.InterpolatedWindVector(wvc->lonLat, &true_wv))
                continue;
            while(true_wv.dir>two_pi) true_wv.dir=true_wv.dir-two_pi;
            while(true_wv.dir<0) true_wv.dir=true_wv.dir+two_pi;

        WindVectorPlus* wvp=wvc->selected;
            WindVectorPlus* nearest=wvc->GetNearestToDirection(true_wv.dir);
        if(!wvp) continue;
            while(wvp->dir>two_pi) wvp->dir=wvp->dir-two_pi;
            while(wvp->dir<0) wvp->dir=wvp->dir+two_pi;
            while(nearest->dir>two_pi) nearest->dir=nearest->dir-two_pi;
            while(nearest->dir<0) nearest->dir=nearest->dir+two_pi;

            float dirdif=ANGDIF(wvp->dir,true_wv.dir);
            float neardif=ANGDIF(nearest->dir,true_wv.dir);
            int nambig=wvc->ambiguities.NodeCount();
            float lat = wvc->lonLat.latitude*rtd;
        float lon = wvc->lonLat.longitude*rtd;
        printf("%d %d  LONLAT %g %g SPEED %g %g %g TRUE DIR %g SELECTED DIR %g %g  NEAREST DIR %g %g NAMBIG %d\n", cti+1,ati+1,lon,lat,
           true_wv.spd,wvp->spd,
           fabs(true_wv.spd - wvp->spd), true_wv.dir*rtd, wvp->dir*rtd,
           dirdif*rtd, nearest->dir*rtd, neardif*rtd, nambig);
      }
    }
    return (0);
}

