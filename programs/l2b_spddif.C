//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

//----------------------------------------------------------------------
// NAME
//		l2b_spddif.C
//
// SYNOPSIS
//		l2b_spddif.C <l2bfile1> <l2bfile2> <landmap>
//
// DESCRIPTION
//              Determines selected speed differences between two files.
//              also outputs whether or not cell is over land. 
// OPTIONS
//		None.
//
// OPERANDS     Self-explanatory
//
// EXAMPLES
//		An example of a command line is:
//			% l2b_extract_worst quikscat.cfg
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
#include <string.h>
#include <signal.h>
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Misc.h"
#include "ConfigList.h"
#include "L2A.h"
#include "LandMap.h"
#include "ConfigSim.h"
#include "ConfigSimDefs.h"
#include "L2B.h"
#include "L2AToL2B.h"
#include "Tracking.h"
#include "Tracking.C"


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
template class List<OffsetList>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;



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

const char* usage_array[] = { "<l2bfile1>,<l2bfile2>,<landmap>", 0};

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

	int clidx = 1;
	const char* l2b_file1 = argv[clidx++];
	const char* l2b_file2 = argv[clidx++];
        char* landmap_file = argv[clidx++];

	//-------------------------------------//
	// open L2B files                      //
	//-------------------------------------//

	L2B l2b1,l2b2;

 	if (! l2b1.OpenForReading(l2b_file1))
	{
		fprintf(stderr, "%s: error opening input file %s\n", command,
			l2b_file1);
		exit(1);
	}

 	if (! l2b2.OpenForReading(l2b_file2))
	{
		fprintf(stderr, "%s: error opening input file %s\n", command,
			l2b_file2);
		exit(1);
	}

	//------------------//
	// read in l2b files //
	//------------------//

	if (! l2b1.ReadHeader())
	{
		fprintf(stderr, "%s: error reading L2B header from file \n",
			command);
		exit(1);
	}

	if (! l2b1.ReadDataRec())
	{
		fprintf(stderr, "%s: error reading L2B data record from file \n",
			command);
		exit(1);
	}

	if (! l2b2.ReadHeader())
	{
		fprintf(stderr, "%s: error reading L2B header from file \n",
			command);
		exit(1);
	}

	if (! l2b2.ReadDataRec())
	{
		fprintf(stderr, "%s: error reading L2B data record from file \n",
			command);
		exit(1);
	}

        //-------------------------------------//
        // Open land map                       //
        //-------------------------------------//
	LandMap landmap;
        if (!landmap.Initialize(landmap_file,1)){
	  fprintf(stderr,"%s: error opening landmap file %s\n",command,
              landmap_file);
          exit(1);
	}

	//----------------------//
        // compute difference & //
	// write to stdout      //
	//----------------------//
	for(int ati=0;ati<1603;ati++){
	  WindVector true_wv;
	  for(int cti=0;cti<80;cti++){
	    WVC* wvc1=l2b1.frame.swath.swath[cti][ati];
            WVC* wvc2=l2b2.frame.swath.swath[cti][ati];
            float lon,lat;
            WindVectorPlus *wvp1=NULL, *wvp2=NULL;
	    if(wvc1) {
	      lon=wvc1->lonLat.longitude;
              lat=wvc1->lonLat.latitude;
              wvp1=wvc1->selected;
	    }
            else if(wvc2){
	      lon=wvc2->lonLat.longitude;
              lat=wvc2->lonLat.latitude;
	    }
	    else{
	      printf("NaN NaN NaN NaN NaN NaN ");
              continue;
	    }
	    if(wvc2) wvp2=wvc2->selected;
            int land=landmap.IsLand(lon,lat);
	    if(!wvp1 || !wvp2) printf("%g %g NaN NaN NaN %d ",lat*rtd,lon*rtd,land);
            else {
	      float spddif=wvp1->spd - wvp2->spd;
	      printf("%g %g %g %g %g %d ", lat*rtd, lon*rtd, spddif, wvp1->spd,
wvp1->obj, land);
	    }
            
	  }
	  printf("\n");
	}
	return (0);
}

