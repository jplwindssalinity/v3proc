//==========================================================//
// Copyright (C) 1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		checkframe_to_ascii
//
// SYNOPSIS
//		checkframe_to_ascii <checkfile> <output_file>
//
// DESCRIPTION
//		Reads in a checkframe file and writes out the data in ascii form
//
// OPTIONS
//		None.
//
// EXAMPLES
//		An example of a command line is:
//			% checkframe_to_ascii simcheck.dat simcheck.txt
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
#include "Misc.h"
#include "CheckFrame.h"
#include "Meas.h"
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Tracking.h"
#include "Tracking.C"

//-----------//
// TEMPLATES //
//-----------//

template class List<EarthPosition>;
template class List<Meas>;
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

#define OPTSTRING               "f"

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -f ]", "<checkfile>", "<output_file>", 
			      "<start_frame>(OPT)",
			      "<end_frame>(OPT)",0};
extern int optind;

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

    int f_flag = 0;
    int c = getopt(argc, argv, OPTSTRING);
    if (c == 'f') f_flag = 1;

	if (argc-optind != 4 && argc-optind != 2)
		usage(command, usage_array, 1);

	int clidx = optind;
	const char* checkfile = argv[clidx++];
	const char* output_file = argv[clidx++];
    int start_frame=-1, end_frame=2;
    if (argc-optind == 4)
    {
	  start_frame=atoi(argv[clidx++]);
	  end_frame=atoi(argv[clidx++]);
	}

	//---------------------//
	// open the check file //
	//---------------------//

	FILE* check_fp = fopen(checkfile,"r");
	if (check_fp == NULL)
	{
		fprintf(stderr, "%s: error opening check file %s\n", command,
			checkfile);
		exit(1);
	}

	//------------------//
	// open output file //
	//------------------//

	FILE* output_fp = fopen(output_file, "w");
	if (output_fp == NULL)
	{
		fprintf(stderr, "%s: error opening output file %s\n", command,
			output_file);
		exit(1);
	}

/*
	//----------------------------//
	// Determine slices per frame //
	//----------------------------//

    int idx;
    int slices_per_frame;
    for (slices_per_frame=0; slices_per_frame < 200; slices_per_frame++)
    {
      if (fseek(check_fp,slices_per_frame*72,SEEK_SET) != 0)
      {
        fprintf(stderr, "%s: error seeking to the beginning in %s\n", command,
          checkfile);
	      exit(1);
      }
      if (fread((void *)&idx,sizeof(int),1,check_fp) != 1)
	  {
	  	fprintf(stderr, "%s: error reading slice idx\n", command);
		exit(1);
	  }
      printf("idx=%d\n",idx);
      if (idx == 0) break; 
    }

    if (fseek(check_fp,0,SEEK_SET) != 0)
    {
      fprintf(stderr, "%s: error seeking to the beginning in %s\n", command,
        checkfile);
	    exit(1);
    }

    if (slices_per_frame < 200)
    {
      printf("Found %d slices per frame\n",slices_per_frame);
    }
    else
    {
      fprintf(stderr, "%s: exceeded 200 slices per frame in %s\n", command,
        checkfile);
	    exit(1);
    }
    slices_per_frame = 10;
*/
 
	//-------------------//
	// Setup check frame //
	//-------------------//

	CheckFrame cf;

	//----------------//
	// loop and write //
	//----------------//

    int frame_count = 1;
    if (f_flag == 0)
    {
	  while (cf.ReadDataRec(check_fp) && frame_count < end_frame)
	  {
          if (frame_count > start_frame)
          {
		    cf.WriteDataRecAscii(output_fp);
		    fprintf(output_fp,"\n");
          }
          if (start_frame >= 0) frame_count++;
	  }
	}
    else
    {
	  while (cf.ReadFortranStructure(check_fp) && frame_count < end_frame)
	  {
          if (frame_count > start_frame)
          {
		    cf.WriteDataRecAscii(output_fp);
		    fprintf(output_fp,"\n");
          }
          if (start_frame >= 0) frame_count++;
	  }
	}

	//-----------------//
	// close the files //
	//-----------------//

	fclose(output_fp);
	fclose(check_fp);

	return (0);
}
