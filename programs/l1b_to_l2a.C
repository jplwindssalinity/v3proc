//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		l15_to_l17
//
// SYNOPSIS
//		l15_to_l17 <sim_config_file>
//
// DESCRIPTION
//		Simulates the SeaWinds 1b ground processing of Level 1.5 to
//		Level 1.7 data.  This program groups the sigma0 data onto
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
//
// EXAMPLES
//		An example of a command line is:
//			% l15_to_l17 sws1b.cfg
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
#include "L15.h"
#include "L17.h"
#include "L15ToL17.h"

//-----------//
// TEMPLATES //
//-----------//

template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<StringPair>;
template class List<Meas>;
template class List<LonLat>;
template class List<MeasSpot>;

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
	config_list.LogErrors();
	if (! config_list.Read(config_file))
	{
		fprintf(stderr, "%s: error reading sim config file %s\n",
			command, config_file);
		exit(1);
	}

    //-------------------------------------//
    // create and configure level products //
    //-------------------------------------//

    L15 l15;
    if (! ConfigL15(&l15, &config_list))
    {
        fprintf(stderr, "%s: error configuring Level 1.5 Product\n", command);
        exit(1);
    }

    L17 l17;
    if (! ConfigL17(&l17, &config_list))
    {
        fprintf(stderr, "%s: error configuring Level 1.7 Product\n", command);
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
    ephemeris.SetMaxNodes(50);      // this should be calculated

    //---------------------------//
    // create and configure Grid //
    //---------------------------//

	double alongtrack_res,crosstrack_res;
	// alongtrack_size needs to be big enough to contain all the measurements
	// that might be colocated.  (in km)
	double alongtrack_size = 5000.0;
	double crosstrack_size = 1400.0;
    config_list.GetDouble("alongtrack_res",&alongtrack_res);
    config_list.GetDouble("crosstrack_res",&crosstrack_res);
	// start_time determines the start of the along track grid coordinates
	double start_time;
    config_list.GetDouble("ati_start_time",&start_time);

	Grid grid;
	grid.Allocate(crosstrack_res,alongtrack_res,
				  crosstrack_size,alongtrack_size);
	grid.l17 = &l17;
	grid.SetEphemeris(&ephemeris);
	grid.SetStartTime(start_time);

    //------------//
    // open files //
    //------------//

    l15.file.OpenForInput();
    l17.file.OpenForOutput();

    //-----------------//
    // conversion loop //
    //-----------------//
 
    L15ToL17 l15_to_l17;

    do
    {
        //------------------------------//
        // read a level 1.5 data record //
        //------------------------------//

        if (! l15.ReadDataRec())
        {
            switch (l15.GetStatus())
            {
            case L15::OK:       // end of file
                break;
            case L15::ERROR_READING_FRAME:
                fprintf(stderr, "%s: error reading Level 1.5 data\n", command);
                exit(1);
                break;
            case L15::ERROR_UNKNOWN:
                fprintf(stderr, "%s: unknown error reading Level 1.5 data\n",
                    command);
                exit(1);
                break;
            default:
                fprintf(stderr, "%s: unknown status (???)\n", command);
                exit(1);
            }
            break;      // done, exit do loop
        }

        //-------//
        // Group //
        //-------//

        if (! l15_to_l17.Group(&l15, &grid))
        {
            fprintf(stderr, "%s: error converting Level 1.0 to Level 1.5\n",
                command);
            exit(1);
        }

    } while (1);

    l15.file.Close();
    l17.file.Close();

    return (0);
}
