//==========================================================//
// Copyright (C) 1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		l1bhdf_sigma0_histo
//
// SYNOPSIS
//	      l1bhdf_sigma0_histo <input_file> <output_file> <min> <max> <step>
//
// DESCRIPTION
//    Generates a histogram of sigma0's.  The
//    values outside of the specified range are dropped (not clipped).
//    The program will calculate an appropriate step size based on the
//    range and the approximate step size given.
//      OPTIONS
//		Last 3 arguments are optional
// AUTHOR
//		Richard West
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
#include <assert.h>
#include "Misc.h"
#include "L1B.h"
#include "L1BHdf.h"
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

//------------------//
// GLOBAL VARIABLES //
//------------------//

int debug_flag = 0;
const char* usage_array[] = {
  "<input_file>",
  "<output_file>",
  "<min>", "<max>", "<step>",
  0 };

//------------------//
// OPTION VARIABLES //
//------------------//

#define OPTSTRING               "p"

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
    int start_frame=-1, end_frame=2;
    char* input_file = NULL;
    char* output_file = NULL;
    float min = -50;
    float max = 10;
    float step = 2;

    int p_flag = 0;
    while (1)
    {
      int c = getopt(argc, argv, OPTSTRING);
      if (c == 'p') p_flag = 1;
      else if (c == -1) break;
    }

    int clidx = optind;
    if (argc-optind == 5)
    {
	  input_file = argv[clidx++];
	  output_file = argv[clidx++];
	  min = atof(argv[clidx++]);
	  max = atof(argv[clidx++]);
	  step = atof(argv[clidx++]);
//	  start_frame=atoi(argv[clidx++]);
//	  end_frame=atoi(argv[clidx++]);
	}
    else if (argc-optind == 2)
    {
	  input_file = argv[clidx++];
	  output_file = argv[clidx++];
    }
    else
    {
      usage(command, usage_array, 1);
    }

    //----------------------//
    // create needed arrays //
    //----------------------//

    unsigned int bins = (unsigned int)((max - min) / step + 0.5);
    step = (max - min) / (float)bins;
    unsigned int* count = new unsigned int[bins];
    for (unsigned int i = 0; i < bins; i++)
        count[i] = 0;

    //-----------------------//
    // read in HDF 1B file   //
    //-----------------------//

    HdfFile::StatusE rc;
    L1BHdf  l1bHdf(input_file, rc);
    if (rc != HdfFile::OK)
    {
        fprintf(stderr, "%s: cannot open HDF %s for input\n",
                               argv[0], input_file);
        exit(1);
    }
    fprintf(stdout, "%s: %s has %d records\n",
                               argv[0], input_file, l1bHdf.GetDataLength());

	//------------------------//
	// open the output file   //
	//------------------------//

    FILE* output = fopen(output_file,"w");
 	if (! output)
	{
		fprintf(stderr, "%s: error opening output file %s\n", command,
			output_file);
		exit(1);
	}       

	//------------------------//
	// process desired frames //
	//------------------------//

    while (1)
    {
      // get next index if more data
      int32 index;
      if (l1bHdf.GetNextIndex(index) != HdfFile::OK) break;
      if (! l1bHdf.ReadL1BHdfDataRec(index)) break;
      if (index < start_frame) continue; // skip until index in range
      if (start_frame > 0 && index > end_frame) break;  // done
    
      Parameter* param=NULL;
      param = l1bHdf.GetParameter(NUM_PULSES, UNIT_DN);
      assert(param != 0);
      int Npulse = (int)(*((char*)(param->data)));

      //--------------------//
      // For each pulse ... //
      //--------------------//

      for (int ipulse = 0; ipulse < Npulse; ipulse++)
      {
        //-------------------------------------------------------
        // the bit 0-1 in sigma0_mode_flag must be 0 - meas pulse
        // otherwise, skip this pulse
        //-------------------------------------------------------
        param = l1bHdf.GetParameter(SIGMA0_MODE_FLAG, UNIT_DN);
        assert(param != 0);
        unsigned short* ushortP = (unsigned short*) (param->data);
        unsigned short sliceModeFlags = *(ushortP + ipulse);
        if ((sliceModeFlags & 0x0003) != 0)
            break;

        param = l1bHdf.GetParameter(SLICE_QUAL_FLAG, UNIT_DN);
        assert(param != 0);
        unsigned int* uintP = (unsigned int*)param->data;
        unsigned int sliceQualFlags = *(uintP + ipulse);

        //--------------------//
        // For each slice ... //
        //--------------------//

        for (int islice = 0; islice < MAX_L1BHDF_NUM_SLICES; islice++)
        {
          param = l1bHdf.GetParameter(SLICE_SIGMA0, UNIT_DB);
          assert(param != 0);
          float* floatP = (float*)param->data;
          floatP += ipulse * MAX_L1BHDF_NUM_SLICES + islice;
          // do i need to put negative sign on explicitely here?
          double sigma0dB = (double)(*floatP);
          int ii = (int)((sigma0dB - min) / step + 0.5);
          if (ii >= 0 && ii <= bins) count[ii]++;
        }
      }

    }

    //--------//
    // output //
    //--------//

    unsigned long total_count = 0;
    for (unsigned int i = 0; i < bins; i++)
    {
        total_count += count[i];
    }
    float scale = 1.0 / (step * float(total_count));

    for (unsigned int i = 0; i < bins; i++)
    {
        fprintf(output, "%g %g\n", i * step + min, count[i] * scale);
    }

    rc = l1bHdf.HdfFile::GetStatus();
    if (rc != HdfFile::OK && rc != HdfFile::NO_MORE_DATA)
    {
        fprintf(stderr, "%s: reading HDF %s failed before EOF is reached\n",
                           argv[0], input_file);
            exit(1);
    }

    //----------------------//
    // close files and exit //
    //----------------------//

	l1bHdf.Close();
    fclose(output);
    return(0);
}

