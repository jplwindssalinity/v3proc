//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

//----------------------------------------------------------------------
// NAME
//		l1a_to_l1b
//
// SYNOPSIS
//		l1a_to_l1b <sim_config_file>
//
// DESCRIPTION
//		Simulates the SeaWinds 1b ground processing of Level 1A to
//		Level 1B data.  This program converts the received power
//		into sigma-0.
//
// OPTIONS
//		None.
//
// OPERANDS
//		The following operand is supported:
//		<sim_config_file>		The sim_config_file needed listing
//								all input parameters, input files, and
//								output files.
//
// EXAMPLES
//		An example of a command line is:
//			% l1a_to_l1b sws1b.cfg
//
// ENVIRONMENT
//		Not environment dependent.
//
// EXIT STATUS
//		The following exit values are returned:
//		0	Program executed successfully
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
#include "BufferedList.h"
#include "BufferedList.C"
#include "List.h"
#include "List.C"
#include "Misc.h"
#include "Ephemeris.h"
#include "ConfigList.h"
#include "L1A.h"
#include "ConfigSim.h"
#include "L1B.h"
#include "L1AToL1B.h"
#include "Tracking.h"
#include "Tracking.C"

//-----------//
// TEMPLATES //
//-----------//

template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<StringPair>;
template class List<Meas>;
template class List<EarthPosition>;
template class List<MeasSpot>;
template class List<WindVectorPlus>;
template class List<long>;
template class List<OffsetList>;

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

const char* usage_array[] = { "<sim_config_file>", 0};

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
	if (argc != 2)
		usage(command, usage_array, 1);

	int clidx = 1;
	const char* config_file = argv[clidx++];

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

	L1A l1a;
	if (! ConfigL1A(&l1a, &config_list))
	{
		fprintf(stderr, "%s: error configuring Level 1A Product\n", command);
		exit(1);
	}

	L1B l1b;
	if (! ConfigL1B(&l1b, &config_list))
	{
		fprintf(stderr, "%s: error configuring Level 1B Product\n", command);
		exit(1);
	}

	//---------------------------------//
	// create and configure spacecraft //
	//---------------------------------//

	Spacecraft spacecraft;
	if (! ConfigSpacecraft(&spacecraft, &config_list))
	{
		fprintf(stderr, "%s: error configuring spacecraft\n", command);
		exit(1);
	}

	//---------------------------------//
	// create and configure instrument //
	//---------------------------------//

	Instrument instrument;
	if (! ConfigInstrument(&instrument, &config_list))
	{
		fprintf(stderr, "%s: error configuring instrument\n", command);
		exit(1);
	}

	//--------------------------------//
	// create and configure ephemeris //
	//--------------------------------//

	Ephemeris ephemeris;
	if (! ConfigEphemeris(&ephemeris, &config_list))
	{
		fprintf(stderr, "%s: error configuring ephemeris\n", command);
		exit(1);
	}

	//------------//
	// open files //
	//------------//

	l1a.OpenForReading();
	l1b.OpenForWriting();


	//-----------------//
	// conversion loop //
	//-----------------//

	int data_record_number=1;
	L1AToL1B l1a_to_l1b;
	if (! ConfigL1AToL1B(&l1a_to_l1b, &config_list))
	{
		fprintf(stderr,
			"%s: error configuring Level 1A to Level 1B converter.\n",
			command);
		exit(1);
	}

        int top_of_file=1;
	do
	{


		//-----------------------------//
		// read a level 1A data record //
		//-----------------------------//

		if (! l1a.ReadDataRec())
		{
			switch (l1a.GetStatus())
			{
			case L1A::OK:		// end of file
				break;
			case L1A::ERROR_READING_FRAME:
				fprintf(stderr, "%s: error reading Level 1A data\n", command);
				exit(1);
				break;
			case L1A::ERROR_UNKNOWN:
				fprintf(stderr, "%s: unknown error reading Level 1A data\n",
					command);
				exit(1);
				break;
			default:
				fprintf(stderr, "%s: unknown status (???)\n", command);
				exit(1);
			}
			break;		// done, exit do loop
		}

                //=======================//
                // FOR FIRST RECORD ONLY // 
		//=======================//
                //===========================================================//
		// This part may be omitted if the prev_eqx_time is included //
                // in the l1a file                                           //
		//===========================================================//
	        if(top_of_file==1){
		  top_of_file=0;
		  //----------------------------------//
		  // Quickly simulate the spacecraft  //
		  // to get previous equator crossing //
		  // time.                            //
		  //----------------------------------//
		  SpacecraftSim spacecraft_sim;
		  if (! ConfigSpacecraftSim(&spacecraft_sim, &config_list)){
		      fprintf(stderr, "%s: error configuring spacecraft simulator\n",
			command);
		      exit(1);
		  }
		  //---------------------------//
		  // set the previous Eqx time //
		  //---------------------------//
                  l1a.frame.Unpack(l1a.buffer);
		  
		  double eqx_time =
		    spacecraft_sim.FindPrevArgOfLatTime(l1a.frame.time,
				      EQX_ARG_OF_LAT, EQX_TIME_TOLERANCE);
		    instrument.SetEqxTime(eqx_time);
		}

		//---------//
		// convert //
		//---------//

		if (! l1a_to_l1b.Convert(&l1a, &spacecraft, &instrument,
			&ephemeris, &l1b))
		{
		       fprintf(stderr, "%s: error converting data record %d\n",
				command, data_record_number);
		}

		//------------------------------//
		// write a level 1B data record //
		//------------------------------//

		else if (! l1b.WriteDataRec())
		{
			fprintf(stderr, "%s: error writing Level 1B data\n", command);
			exit(1);
		}

		data_record_number++;
	} while (1);

	l1a.Close();
	l1b.Close();

	return (0);
}
