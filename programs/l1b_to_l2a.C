//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

//----------------------------------------------------------------------
// NAME
//		l1b_to_l2a
//
// SYNOPSIS
//		l1b_to_l2a <sim_config_file> [indices_file]
//
// DESCRIPTION
//		Simulates the SeaWinds 1b ground processing of Level 1B to
//		Level 2A data.  This program groups the sigma0 data onto
//		a grid.
//
// OPTIONS
//		None.
//
// OPERANDS
//		The following operand is supported:
//		<sim_config_file>		The sim_config_file needed listing
//								all input parameters, input files, and
//								output files.
//              [indices_file}     Optional ASCII output file for each meaurement
//                                 Format is:  spot_id pixel_num num_wvcs_gridded_in atd ctd
//
// EXAMPLES
//		An example of a command line is:
//			% l1b_to_l2a sws1b.cfg
//
// ENVIRONMENT
//		Not environment dependent.
//
// EXIT STATUS
//		The following exit values are returned:
//		1	Program executed successfully
//		>0	Program had an error
//
// NOTES
//		None.
//
// AUTHOR
//		James N. Huddleston
//		hudd@acid.jpl.nasa.gov
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
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Misc.h"
#include "ConfigList.h"
#include "ConfigSim.h"
#include "Ephemeris.h"
#include "L1B.h"
#include "L2A.h"
#include "L1BToL2A.h"
#include "Tracking.h"
#include "Tracking.C"
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

#define	USE_COMPOSITING_KEYWORD		"USE_COMPOSITING"

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

const char* usage_array[] = { "<sim_config_file>", "[indices_file]",0};

//--------------//
// MAIN PROGRAM //
//--------------//

int
main(
	int		argc,
	char*	argv[])
{
	//------------------------//
	// parse the command line //
	//------------------------//

	const char* command = no_path(argv[0]);
	if (argc != 2 && argc!=3 )
		usage(command, usage_array, 1);

	int clidx = 1;
	const char* config_file = argv[clidx++];

        // OLD optional argument commented out
	long int max_record_no=0;
        //if(argc==3){
	//  max_record_no=atoi(argv[clidx++]);
        //	}
        char* indfile;
        if(argc==3) indfile=argv[clidx++];
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

	//----------------------//
	// Get compositing flag //
	//----------------------//

	int use_compositing;
	if (! config_list.GetInt(USE_COMPOSITING_KEYWORD, &use_compositing))
		return(0);

	//-----------------------//
	// create spacecraft sim //
	//-----------------------//

	SpacecraftSim spacecraft_sim;
	if (! ConfigSpacecraftSim(&spacecraft_sim, &config_list))
	{
		fprintf(stderr, "%s: error configuring spacecraft simulator\n",
			command);
		exit(1);
	}

	//---------------------------//
	// create and configure Grid //
	//---------------------------//

	Grid grid;
	if (! ConfigGrid(&grid, &config_list))
	{
		fprintf(stderr, "%s: error configuring grid\n", command);
		exit(1);
	}

        if(argc==3) grid.CreateIndicesFile(indfile);

	//-------------------------------//
	// configure the grid start time //
	//-------------------------------//

	double grid_start_time, grid_end_time;
	double instrument_start_time, instrument_end_time;
	double spacecraft_start_time, spacecraft_end_time;
        
    int gen_grid_times_from_L1B = 0;
    
    // Don't quit just because the config file doesn't contain 
    // the GEN_GRID_TIMES_FROM_L1B_KEYWORD string...
    config_list.WarnForMissingKeywords();
    
    if( config_list.GetInt(GEN_GRID_TIMES_FROM_L1B_KEYWORD,
                           &gen_grid_times_from_L1B) == 0  || gen_grid_times_from_L1B == 0 )
    {
        printf("Reading grid/instrument start/stop times from config file.\n");
  		if (! ConfigControl(&spacecraft_sim, &config_list,
			&grid_start_time, &grid_end_time,
			&instrument_start_time, &instrument_end_time,
			&spacecraft_start_time, &spacecraft_end_time))
		{
			fprintf(stderr, "%s: error configuring simulation times\n", command);
			exit(1);
		}
	}
	else // Generate the grid start/stop times from the L1B file itself.
	{
	  printf("Generating grid/instrument start/stop times from L1B file.\n");
	  grid.l1b.OpenForReading();
	  
	  int i_rec = 0;
	  double t_min, t_max;
	  
	  while ( grid.l1b.ReadDataRec() )
	  {
	    i_rec++;
	    
	    MeasSpot* meas_spot = grid.l1b.frame.spotList.GetHead();
	    double t_now        = meas_spot->time;
	    
	    if( i_rec == 1 )
	    {
	      t_min = t_now;
	      t_max = t_now;
	    }
	    if( t_now > t_max ) t_max = t_now;
	  }
	  
	  // First and last measurement times
	  instrument_start_time = t_min;
	  instrument_end_time   = t_max;

	  if( grid.algorithm == Grid::SUBTRACK )
	  {
	    // 5 min boundary around measurement times for grid times.
		grid_start_time = t_min - 5 * 60;
		grid_end_time   = t_max + 5 * 60;
	  }
	  else if( grid.algorithm == Grid::SOM )
	  { // SOM binning uses this for setting up the SOM coordinate
	    // to grid index conversion; SUBTRACK binning does not.
		grid_start_time = t_min;
		grid_end_time   = t_max;
	  }
	  else
	  {
	  	fprintf(stderr,"%s: Unknown gridding algo; quitting\n",command);
	  	exit(1);
	  }
	  
	  
      
      ETime dummy_time;
      char  codeA_time_str[CODE_A_TIME_LENGTH];
      
      printf("\n");
      printf("Generated grid/instrument start/end times: \n");
      
      dummy_time.SetTime(instrument_start_time);
      dummy_time.ToCodeA(&codeA_time_str[0]);
      printf("INSTRUMENT_START_TIME = %s\n",codeA_time_str);

      dummy_time.SetTime(instrument_end_time);
      dummy_time.ToCodeA(&codeA_time_str[0]);
      printf("INSTRUMENT_END_TIME   = %s\n",codeA_time_str);

      dummy_time.SetTime(grid_start_time);
      dummy_time.ToCodeA(&codeA_time_str[0]);
      printf("GRID_START_TIME       = %s\n",codeA_time_str);

      dummy_time.SetTime(grid_end_time);
      dummy_time.ToCodeA(&codeA_time_str[0]);
      printf("GRID_END_TIME         = %s\n",codeA_time_str);
      printf("\n");
      
	  grid.l1b.Close();
	}
	
	grid.SetStartTime(grid_start_time);
	grid.SetEndTime(grid_end_time);

        if (instrument_end_time>grid_end_time) {
          grid.lat_end_time = instrument_end_time;
        } else {
          grid.lat_end_time = grid_end_time;
        }

	//------------//
	// open files //
	//------------//

	grid.l1b.OpenForReading();
	grid.l2a.OpenForWriting();

        /* all variable ended with Size are in byte */

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

        while (nm==0) {

          tmp = ftello(grid.l1b.GetInputFp());
          if (! grid.l1b.ReadDataRec()) {
            fprintf(stderr, "%s: error reading Level 1B data\n", command);
            exit(1);
          }

          /* find out number of spots in the spotlist */

          nSpot = grid.l1b.frame.spotList.NodeCount();
          //cout << "num of spots: " << nSpot << endl;

          /* find out total number of measurements for all spots */

          for (MeasSpot* meas_spot = grid.l1b.frame.spotList.GetHead();
               meas_spot; meas_spot = grid.l1b.frame.spotList.GetNext()) {
            nm += meas_spot->NodeCount();
          }

        }

        cout << "number of meas: " << nm << endl;

        int sumSize = nSpotSize + nSpot*(spotSize+nMeasSize);
        grid.meas_length = (int)((ftello(grid.l1b.GetInputFp())-tmp-sumSize)/nm);
        cout << "meas length in byte: " << grid.meas_length << endl;

        /* get number of bytes in l1b file */

        fseeko(grid.l1b.GetInputFp(),0,SEEK_END);
        off_t end_byte = ftello(grid.l1b.GetInputFp());

        /* go back to the beginning of l1b file */

        fseeko(grid.l1b.GetInputFp(),0,SEEK_SET);

        double spotTime;

        Meas* meas = new Meas;

        long counter = 0;

        for (;;) {
          
          counter++;
          if (max_record_no>0 && counter>max_record_no) break;
          if (counter % 100 == 0) {
            fprintf(stderr,"L1B record count = %ld bytes count =%g\n",
                    counter, (double)ftello(grid.l1b.GetInputFp()));
          }

          if (fread(&nSpot, sizeof(int), 1, grid.l1b.GetInputFp()) != 1) break; // find number of spots
          //cout << "num of spots: " << nSpot << endl;

          for (long ss=0; ss<nSpot; ss++) {
            if (fread(&spotTime, sizeof(double), 1, grid.l1b.GetInputFp()) != 1) break; // find spot time
            if (fseeko(grid.l1b.GetInputFp(), spotSize-timeSize, SEEK_CUR) == -1) break;
            if (fread(&nm, sizeof(int), 1, grid.l1b.GetInputFp()) != 1) break; // find number of meas

            for (int mm=0; mm<nm; mm++) {
              if (meas->Read(grid.l1b.GetInputFp()) != 1) {
                fprintf(stderr, "Error in Read of Meas in l1b frame %d  spot %d meas %d!\n",counter-1,ss,mm);
                break;
                //exit(1);
              }
              if(argc==3) fprintf(grid.GetIndFp(),"%d ",(int)counter-1);
              if (grid.Add(meas, spotTime, ss, use_compositing) != 1) {
                fprintf(stderr, "Error in Add of Grid!\n");
                break;
                //exit(1);
              }
            } // meas loop

          } // spot loop

          if (ftello(grid.l1b.GetInputFp()) >= end_byte) break;

        }

        delete meas;

	//
	// Write out data in the grid that hasn't been written yet.
	//

        grid.Flush(use_compositing);

	grid.l1b.Close();
	grid.l2a.Close();

        return (0);

}
