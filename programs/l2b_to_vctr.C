//==============================================================//
// Copyright (C) 1997-2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    l2b_to_vctr
//
// SYNOPSIS
//    l2b_to_vctr <l2b_file> [ vctr_base ]
//
// DESCRIPTION
//    Converts a Level 2B file into multiple vctr (vector)
//    files for plotting in IDL.  Output filenames are created by
//    adding the rank number (0 for selected) to the base name.
//    If vctr_base is not provided, l2b_file is used as the base name.
//
// OPTIONS
//    None.
//
// OPERANDS
//    The following operands are supported:
//      <l2b_file>     The input Level 2B wind field
//      [ vctr_base ]  The output vctr file basename
//
// EXAMPLES
//    An example of a command line is:
//    % l2b_to_vctr l2b.dat l2b.vctr
//
// ENVIRONMENT
//    Not environment dependent.
//
// EXIT STATUS
//    The following exit values are returned:
//       0  Program executed successfully
//      >0  Program had an error
//
// NOTES
//    None.
//
// AUTHORS
//    James N. Huddleston (James.N.Huddleston@jpl.nasa.gov)
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

const char* usage_array[] = { "<l2b_file>", "[ vctr_base ]",
    "[ hdf_flag (1=HDF, 0=default) ] [flag_file]", 0};

//--------------//
// MAIN PROGRAM //
//--------------//

int
main(
    int    argc,
    char*  argv[])
{
    //------------------------//
    // parse the command line //
    //------------------------//

    const char* command = no_path(argv[0]);
    if (argc < 2 || argc > 5)
        usage(command, usage_array, 1);

    int clidx = 1;
    const char* l2b_file = argv[clidx++];
    const char* vctr_base = l2b_file;
    char* flag_file=NULL;
    int hdf_flag = 0;
//    int rm_rain = 0;
    if (argc >= 3)
        vctr_base = argv[clidx++];
    if (argc >= 4)
        hdf_flag = atoi(argv[clidx++]);
    if (argc == 5)
        flag_file=argv[clidx++];

    //------------------//
    // read in l2b file //
    //------------------//

    L2B l2b;
    if (hdf_flag)
    {
        if( l2b.ReadHDF(l2b_file) == 0)
        {
            fprintf(stderr, "%s: error opening HDF L2B file %s\n", command,
                l2b_file);
            exit(1);
        }
    }
    else
    {
        if (! l2b.OpenForReading(l2b_file))
        {
            fprintf(stderr, "%s: error opening L2B file %s\n", command,
                l2b_file);
            exit(1);
        }
        if (! l2b.ReadHeader())
        {
            fprintf(stderr, "%s: error reading L2B header from file %s\n",
                command, l2b_file);
            exit(1);
        }

        if (! l2b.ReadDataRec())
        {
            fprintf(stderr, "%s: error reading L2B data record from file %s\n",
                command, l2b_file);
            exit(1);
        }
    }
    //-----------------------------------------//
    // use extrenal rain flag file  if desired //
    //-----------------------------------------//

    if(flag_file) l2b.frame.swath.ReadFlagFile(flag_file);

    //----------------------//
    // write out vctr files //
    //----------------------//

    int max_rank = l2b.frame.swath.GetMaxAmbiguityCount();
    char filename[1024];
    for (int i = 0; i <= max_rank; i++)
    {
        sprintf(filename, "%s.%d", vctr_base, i);
        if (! l2b.WriteVctr(filename, i))
        {
            fprintf(stderr, "%s: error writing vctr file %s\n", command,
                filename);
            exit(1);
        }
    }
    l2b.frame.swath.DeleteFlaggedData();
    sprintf(filename, "%s.norain", vctr_base);
    if (! l2b.WriteVctr(filename, 0))
    {
        fprintf(stderr, "%s: error writing vctr file %s\n", command, filename);
        exit(1);
    }

    l2b.frame.swath.SelectNudge();
    sprintf(filename, "%s.nudge", vctr_base);
    if (! l2b.WriteVctr(filename, 0))
    {
        fprintf(stderr, "%s: error writing vctr file %s\n", command, filename);
        exit(1);
    }
    int c=170;
      l2b.frame.swath.StreamNudge(c);
      sprintf(filename, "%s.init.%d", vctr_base,c);
      if (! l2b.WriteVctr(filename, 0))
    {
        fprintf(stderr, "%s: error writing vctr file %s\n", command, filename);
        exit(1);
    }

    return (0);
}


