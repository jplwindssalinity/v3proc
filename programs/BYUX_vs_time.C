//==============================================================//
// Copyright (C) 1998-1999, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

//----------------------------------------------------------------------
// NAME
//		BYUX_vs_time
//
// SYNOPSIS
//		BYUX_vs_time <sim_config_file> <azimuth> <beam> > output_file
//
// DESCRIPTION
//              Writes data suitable for plotting in xmgr read from the
//              BYUXTable for a particular azimuth and beam. 
//              Columns in output file
//              1     time
//              2-13  X for slices 1-12
//              14    Egg X
//
// OPTIONS
//		None.
//
// OPERANDS
//		The following operands are supported:
//		<sim_config_file>	The sim configuration file.
//              <azimuth>               Ground Impact Azimuth
//              <beam>                  0=inner 1=outer
//              
//
// EXAMPLES
//		An example of a command line is:
//			% BYUX_vs_time file.cfg 90.0 0 > file.out
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
//		Bryan Stiles
//		bstiles@acid.jpl.nasa.gov
//----------------------------------------------------------------------
//-----------------------//
// Configuration Control //
//-----------------------//

static const char rcs_id[] =
	"@(#) $Id$";

//----------//
// INCLUDES //
//----------//
#include<stdio.h>
#include<math.h>
#include"BYUXTable.h"
#include "Misc.h"
#include "ConfigSim.h"
#include "ConfigList.h"
#include "List.h"
#include "List.C"
#include "Meas.h"
#include "Wind.h"
#include "Tracking.h"
#include "Tracking.C"
#include "Spacecraft.h"
#include "ConfigSim.h"
#include "QscatConfig.h"
#include "QscatSim.h"
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
//-----------//
// TEMPLATES //
//-----------//
class AngleInterval;

template class List<AngleInterval>;
template class List<StringPair>;
template class List<Meas>;
template class List<EarthPosition>;
template class List<MeasSpot>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<WindVectorPlus>;
template class List<long>;
template class List<OffsetList>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "<sim_config_file>", "<azimuth>", "<beam>", 0};

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
	if (argc != 4)
		usage(command, usage_array, 1);
        int clidx=1;
	const char* config_file = argv[clidx++];        
        float azimuth=atof(argv[clidx++])*dtr;
        int beam=atoi(argv[clidx++]);
	//--------------------------------//
	// read in simulation config file //
	//--------------------------------//

	ConfigList config_list;
	if (! config_list.Read(config_file))
	{
		fprintf(stderr, "%s: error reading sim config file %s\n",
			command, config_file);
		exit(1);
	}

	//-----------------------------//
        // Configure BYUXTable         //
        //-----------------------------//
        BYUXTable byux;
        if(!ConfigBYUXTable(&byux,&config_list)){ 
	  fprintf(stderr,"%s: Error configuring  BYUXTable.\n",command);
	  exit(1);
	}
	

	//------------------------------//
        // Output data                  //
        //------------------------------//
	for(float t=0; t<BYU_NOMINAL_ORBIT_PERIOD;
	    t+=BYU_TIME_INTERVAL_BETWEEN_STEPS){
	  printf("%g ",t);
	  for(int s=0;s<12;s++){
	    int rel_idx;
            abs_to_rel_idx(s,12,&rel_idx);
	    float X=byux.GetX(beam,azimuth,t/BYU_NOMINAL_ORBIT_PERIOD,rel_idx,
			      0);
	    X=10.0*log10(X);
	    printf("%g ",X);	    
	  }
	  printf("\n");
	}
	exit(0);
}
