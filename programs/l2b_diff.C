//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

//----------------------------------------------------------------------
// NAME
//		l2b_diff
//
// SYNOPSIS
//		l2b_diff <l2b_file1> <l2b_file2> [ vctr_base ] [hdfflag]
//
// DESCRIPTION
//	        Computes difference between wind vectors in two l2b files and
//              convert it to vector               
//		files for plotting in IDL.  Output filenames are created by
//		adding the rank number (0 for selected) to the base name.
//		If vctr_base is not provided, l2b_file is used as the base name.
//
// OPTIONS
//		None.
//
// OPERANDS
//		The following operands are supported:
//		<l2b_filex>		The input Level 2B wind fields
//		[ vctr_base ]	The output vctr file basename
//              [ hdfflag ] 1=HDF 0=default 
// EXAMPLES
//		An example of a command line is:
//			% l2b_to_vctr l2b.dat1 l2b.dat2 l2b.vctr
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
#include "Misc.h"
#include "Wind.h"
#include "L2B.h"
#include "List.h"
#include "List.C"

//-----------//
// TEMPLATES //
//-----------//

template class List<EarthPosition>;
template class List<WindVectorPlus>;
template class List<AngleInterval>;

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

const char* usage_array[] = { "<l2b_file1>", "<l2b_file2>", "[ vctr_base ]",
"[hdfflag] 1=HDF 0=default", 0};

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
	if (argc != 4 && argc!=5 )
		usage(command, usage_array, 1);

	int clidx = 1;
	const char* l2b_file1 = argv[clidx++];
        const char* l2b_file2 = argv[clidx++];
	const char* vctr_base = argv[clidx++];
        int hdf_flag=0;
        if(argc==5) hdf_flag = atoi(argv[clidx++]);

	//------------------//
	// read in l2b file //
	//------------------//

	L2B l2b1, l2b2;        
        l2b1.SetInputFilename(l2b_file1);
        l2b2.SetInputFilename(l2b_file2);
        if(hdf_flag){ 
	  if(l2b1.ReadHDF()==0){	    
	    fprintf(stderr, "%s: error opening HDF L2B file %s\n", command,
		    l2b_file1);
	    
	     exit(1);
	  }
	  if(l2b2.ReadHDF()==0){	    
	    fprintf(stderr, "%s: error opening HDF L2B file %s\n", command,
		    l2b_file2);
	     exit(1);
	  }

	}
        else {
	  if (! l2b1.OpenForReading(l2b_file1))
	    {
	      fprintf(stderr, "%s: error opening L2B file %s\n", command, l2b_file1);
	      exit(1);
	    }
	  if (! l2b1.ReadHeader())
	    {
	      fprintf(stderr, "%s: error reading L2B header from file %s\n",
		      command, l2b_file1);
	      exit(1);
	    }
	  
	  if (! l2b1.ReadDataRec())
	    {
	      fprintf(stderr, "%s: error reading L2B data record from file %s\n",
		      command, l2b_file1);
	      exit(1);
	    }
	  
	  if (! l2b2.OpenForReading(l2b_file2))
	    {
	      fprintf(stderr, "%s: error opening L2B file %s\n", command, l2b_file2);
	      exit(1);
	    }
	  if (! l2b2.ReadHeader())
	    {
	      fprintf(stderr, "%s: error reading L2B header from file %s\n",
		      command, l2b_file2);
	      exit(1);
	    }
	  
	  if (! l2b2.ReadDataRec())
	    {
	      fprintf(stderr, "%s: error reading L2B data record from file %s\n",
		      command, l2b_file2);
	      exit(1);
	    }
	}
	//----------------------//
	// write out vctr files //
	//----------------------//
        l2b1.frame.swath-=l2b2.frame.swath;
	int max_rank = l2b1.frame.swath.GetMaxAmbiguityCount();
	for (int i = 0; i <= max_rank; i++)
	{
		char filename[1024];
		sprintf(filename, "%s.%d", vctr_base, i);
		if (! l2b1.WriteVctr(filename, i))
		{
			fprintf(stderr, "%s: error writing vctr file %s\n", command,
				filename);
			exit(1);
		}
	}

	return (0);
}
