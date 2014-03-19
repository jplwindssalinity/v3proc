//----------//
// INCLUDES //
//----------//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "List.h"
#include "BufferedList.h"
#include "Misc.h"
#include "ConfigList.h"
#include "L2A.h"
#include "ConfigSim.h"
#include "L2B.h"
#include "L2AToL2B.h"
#include "Tracking.h"
#include "Array.h"

using std::list;
using std::map; 

//-----------//
// TEMPLATES //
//-----------//

// Class declarations needed for templates
// eliminates need to include the entire header file
class AngleInterval;

template class List<StringPair>;
template class List<Meas>;
template class List<EarthPosition>;
template class List<WindVectorPlus>;
template class List<MeasSpot>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<off_t>;
template class List<OffsetList>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;
template class List<AngleInterval>;
template class std::list<string>;
template class std::map<string,string,Options::ltstr>;

//-----------//
// CONSTANTS //
//-----------//

#define HIRES12
#define HDF_NUM_AMBIGUITIES   4

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

int
main(
    int    argc,
    char*  argv[])
{
  const char* command = no_path(argv[0]);
  
  int hdf_input        = 0;
  int svt_input        = 0;
  int out_file_entered = 0;
  int print_usage      = 0;

  char* file_in         = NULL;
  char* retdat_filename = NULL;
  
 // Parse Command-Line arguments
  int optind = 1;
  while ( (optind < argc) && (argv[optind][0]=='-') )
    {
      std::string sw = argv[optind];
      if( sw == "-hdf_input" )
	{
	  ++optind;
	  hdf_input = 1;
	  file_in   = argv[optind];
	}
      else if( sw == "-svt_input" )
	{
	  ++optind;
	  svt_input = 1;
	  file_in   = argv[optind];
	}
      else if( sw == "-o" )
	{
	  ++optind;
	  out_file_entered = 1;
	  retdat_filename = argv[optind];
	}
      else
	print_usage = 1;
      ++optind;
    }
  
  if(       svt_input && hdf_input  ) print_usage = 1;
  else if( !svt_input && !hdf_input ) print_usage = 1;
  
  if( print_usage )
    {
      printf("Usasge: %s [-hdf_input hdf_file] or [-svt_input svt_file] -o output_retdat_file\n",command);
      return (1);  
    }

  L2B l2b;
  l2b.SetInputFilename( file_in );
  
  if( svt_input )
  {
    if (! l2b.OpenForReading() )
      {
	fprintf(stderr, "%s: error opening L2B file for reading\n",
                command);
	exit(1);
      }
    if (! l2b.ReadHeader())
    {
      fprintf(stderr, "%s: error reading Level 2B header\n", command);
      exit(1);
    }
    if (! l2b.ReadDataRec())
      {
	switch (l2b.GetStatus())
	  {
	  case L2B::OK:
	    fprintf(stderr, "%s: Unexpected EOF Level 2B data\n", command);
	    exit(1);
	    break;    // end of file
	  case L2B::ERROR_READING_FRAME:
	    fprintf(stderr, "%s: error reading Level 2B data\n", command);
	    exit(1);
	    break;
	  case L2B::ERROR_UNKNOWN:
	    fprintf(stderr, "%s: unknown error reading Level 2B data\n",
		    command);
	    exit(1);
	    break;
	  default:
	    fprintf(stderr, "%s: unknown status\n", command);
	    exit(1);
	  }
      }  
  }
  else if( hdf_input )
    {
      if ( l2b.ReadPureHdf(file_in,1) == 0 )
	{
	  fprintf(stderr, "%s: error reading HDF L2B file %s\n", command, file_in);
	  exit(1);
	}
      l2b.header.crossTrackResolution = 25.0;
      l2b.header.alongTrackResolution = 25.0;
      l2b.header.zeroIndex = 38;    
    }
  else
    {
      fprintf(stderr,"%s, we shouldn't be here.\n",command);
    }
  
  l2b.WriteRETDAT( retdat_filename );
  l2b.Close();
  
  return (0);
}
