//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		make_kfactor
//
// SYNOPSIS
//		make_kfactor <true_X_file> <est_X_file> <kfactor_file>
//
// DESCRIPTION
//		Computes a Kfactor table form a true and estimated X tables.
//
// OPTIONS
//		None.
//
// OPERANDS
//		The following operand is supported:
//        <true_X_table> File of true X values produced by sim_accurate.
//        <est_X_table> File of estimated X values produced by sim.
//        <kfactor_table> Kfactor output file. 
//
// EXAMPLES
//		An example of a command line is:
//			% make_kfactor truex.tab estx.tab kfactor.tab
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
//		Slices in trueX table may be smaller than those in estX
//
// AUTHOR
//		Bryan Stiles
//		bstiles@warukaze.jpl.nasa.gov
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
#include "L1A.h"
#include "ConfigSim.h"
#include "L1B.h"
#include "L1AToL1B.h"
#include "ConfigList.h"
#include "XTable.h"

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

const char* usage_array[] = { "<trueX_file>","<estX_file>", "<kfactor_file>", 0};

//--------------//
// MAIN PROGRAM //
//--------------//

int
main(
	int		argc,
	char*	argv[])
{
	const char* command = no_path(argv[0]);
	if (argc != 4)
		usage(command, usage_array, 1);

        XTable trueX, estX, kfactor;

	/**** Read Xtables ***/
	if(!trueX.SetFilename(argv[1]))
	  {
	    fprintf(stderr,"%s :Bad filename for trueX table\n",command);
	    exit(1);
	  }

	if(!estX.SetFilename(argv[2]))
	  {
	    fprintf(stderr,"%s :Bad filename for estX table\n",command);
	    exit(1);
	  }

	if(!trueX.Read())
	  {
	    fprintf(stderr,"%s :Error reading trueX table\n",command);
	    exit(1);
	  }
        if(!estX.Read())
	  {
	    fprintf(stderr,"%s :Error reading estX table\n",command);
	    exit(1);
	  }

        /**** Calculate Kfactor table ***/
        
        if(!MakeKfactorTable(&trueX,&estX,&kfactor)){
	    fprintf(stderr,"%s :Error calculating kfactor table\n",command);
	    exit(1);
	}

        /**** Write Kfactor Table ***/
        if(!kfactor.SetFilename(argv[3])){
	   exit(1); 
	}

	if(!kfactor.Write()){
	    fprintf(stderr,"%s :Error writing Kfactor table\n",command);
	    exit(1);
	}
	
	exit(0);
}
