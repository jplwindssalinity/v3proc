//==============================================================//
// Copyright (C) 1997-2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    l2b_to_vctr
//
// SYNOPSIS
//    l2b_to_vctr <l2b_file> [ vctr_base ] [ hdf_flag] [ flag_file ] [ config_file ]
//
// DESCRIPTION
//    Converts a Level 2B file into multiple vctr (vector)
//    files for plotting in IDL.  Output filenames are created by
//    adding the rank number (0 for selected) to the base name.
//    If vctr_base is not provided, l2b_file is used as the base name.
//    If a config_file is specified, the truth is also written out.  
//       Need to specify: vctr_base, hdf_flag (can set to 0), and
//       flag_file (can set to '0', '[]', or 'null' (case insensitive) if no flag file
//       is desired).  
//
// OPTIONS
//    None.
//
// OPERANDS
//    The following operands are supported:
//      <l2b_file>        The input Level 2B wind field
//      [ vctr_base ]     The output vctr file basename
//      [ hdf_flag ]      1 to read hdf format, 0 to not
//      [ flag_file ]     external rain flag file.  Use '0', '[]', or 
//                        'null' (case insensitive) if no flag_file is desired
//      [ config_file ]   If and only if config_file is specified, truth is written out
//
// EXAMPLES
//    An example of a command line is:
//    % l2b_to_vctr l2b.dat l2b.vctr
//
//    % l2b_to_vctr l2b.dat l2b.vctr 0 [] sim_params.rdf
//    % l2b_to_vctr l2b.dat l2b.vctr 0 0 sim_params.rdf
//    % l2b_to_vctr l2b.dat l2b.vctr 0 null sim_params.rdf
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
//    Alexandra H. Chau, modified 9/24/09
//      Added truth output, updated comments
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
#include <unistd.h>
#include "ConfigList.h"
#include "ConfigSimDefs.h"
#include "Misc.h"
#include "Wind.h"
#include "L2B.h"
#include "List.h"

using std::list;
using std::map; 

//-----------//
// TEMPLATES //
//-----------//

template class List<StringPair>;
template class List<EarthPosition>;
template class List<WindVectorPlus>;
template class List<AngleInterval>;
template class std::list<string>;
template class std::map<string,string,Options::ltstr>;

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
			      "[ hdf_flag (1=HDF, 0=default) ]",
			      "[flag_file (use '0', '[]', or 'null' for empty flag_file)]", 
			      "[config_file (add a config_File if you want to output truth)]", 0};

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
    if (argc < 2 || argc > 6)
      usage(command, usage_array, 1);
  
    int clidx = 1;
    const char* l2b_file = argv[clidx++];
    const char* vctr_base = l2b_file;
    char* flag_file = NULL;
    int hdf_flag = 0;
    char* config_file = NULL;
    ConfigList config_list;
  
    //    int rm_rain = 0;
    if (argc >= 3)
      vctr_base = argv[clidx++];
    if (argc >= 4)
      hdf_flag = atoi(argv[clidx++]);
    if (argc >= 5)
      flag_file=argv[clidx++];
    if (argc >= 6)
      config_file = argv[clidx++];
    // Can have the case where you don't want a flag_file, but you do want a config_file (to get truth), 
    // so set flag_file back to NULL if certain keywords were input
    if (flag_file)
      if (!strcasecmp(flag_file,"0") || !strcasecmp(flag_file,"[]") || !strcasecmp(flag_file,"NULL"))
	{
	  flag_file = NULL;
	  fprintf(stderr,"Setting flag_file to null\n");
	};
       

    //------------------//
    // read in l2b file //
    //------------------//

    L2B l2b;
    if (hdf_flag)
    {
        if (! l2b.ReadPureHdf(l2b_file))
        {
            fprintf(stderr, "%s: error reading HDF L2B file %s\n", command,
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

    //--------------------------------------------------//
    // read in config_file if passed in, and read truth //
    //--------------------------------------------------//
    WindField truth;
    if(config_file)
    {
      if (! config_list.Read(config_file)) // Read in the file
      {
	fprintf(stderr, "%s: error reading config file %s\n", command, config_file);
	exit(1);
      }
      // fprintf(stderr, "%s: config_file is %s\n", command, config_file); // for debugging
      char* truth_type = NULL;
      char* truth_file = NULL;
      // read truth type
      truth_type = config_list.Get(TRUTH_WIND_TYPE_KEYWORD);
      if (truth_type == NULL)
      {
	fprintf(stderr, "%s: must specify truth windfield type \n", command);
	exit(1);
      }
      // read truth file name
      truth_file = config_list.Get(TRUTH_WIND_FILE_KEYWORD);
      if (truth_file == NULL)
      {
	fprintf(stderr, "%s: must specify truth windfield file in %s \n", command, config_file);
	exit(1);
      }
      // read truth file
      // Check boundaries
      if (strcasecmp(truth_type,"SV") == 0)
      {
	if (!config_list.GetFloat(WIND_FIELD_LAT_MIN_KEYWORD, &truth.lat_min) ||
	    !config_list.GetFloat(WIND_FIELD_LAT_MAX_KEYWORD, &truth.lat_max) ||
	    !config_list.GetFloat(WIND_FIELD_LON_MIN_KEYWORD, &truth.lon_min) ||
	    !config_list.GetFloat(WIND_FIELD_LON_MAX_KEYWORD, &truth.lon_max))
	{
	  fprintf(stderr, "ConfigWindField: SV can't determine range of lat and lon\n");
	  return(0);
	}
      }
      // Read truth
      if (! truth.ReadType(truth_file, truth_type))
      {
	fprintf(stderr, "%s: error reading true wind field from file %s\n", command, truth_file);
	exit(1);
      }
      // Scale wind speeds
      config_list.DoNothingForMissingKeywords();
      float scale;
      if (config_list.GetFloat(TRUTH_WIND_SPEED_MULTIPLIER_KEYWORD, &scale))
      {
	truth.ScaleSpeed(scale);
	fprintf(stderr, "Warning: scaling all wind speeds by %g\n", scale);
      }
      config_list.ExitForMissingKeywords();
      // use as fixed wind speed? 
      config_list.DoNothingForMissingKeywords();
      float fixed_speed;
      if (config_list.GetFloat(TRUTH_WIND_FIXED_SPEED_KEYWORD, &fixed_speed))
      {
	truth.FixSpeed(fixed_speed);
      }
      float fixed_direction;
      if (config_list.GetFloat(TRUTH_WIND_FIXED_DIRECTION_KEYWORD, &fixed_direction))
      {
	fixed_direction *= dtr;
	truth.FixDirection(fixed_direction);
      }
      config_list.ExitForMissingKeywords(); 
    }

    //-----------------------------------------//
    // use external rain flag file  if desired //
    //-----------------------------------------//

    if(flag_file) l2b.frame.swath.ReadFlagFile(flag_file);

    //----------------------//
    // write out vctr files //
    //----------------------//
    
    // Write ambiguities (.0, .1, .2, etc.)
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
    
    // Delete rain flagged data and write .norain
    l2b.frame.swath.DeleteFlaggedData();
    sprintf(filename, "%s.norain", vctr_base);
    if (! l2b.WriteVctr(filename, 0))
    {
        fprintf(stderr, "%s: error writing vctr file %s\n", command, filename);
        exit(1);
    }

    // Write .nudge
    l2b.frame.swath.SelectNudge();
    sprintf(filename, "%s.nudge", vctr_base);
    if (! l2b.WriteVctr(filename, 0))
    {
        fprintf(stderr, "%s: error writing vctr file %s\n", command, filename);
        exit(1);
    }

    // Write .near
    l2b.frame.swath.SelectNearest(NULL);
    sprintf(filename, "%s.near", vctr_base);
    if (! l2b.WriteVctr(filename, 0))
    {
        fprintf(stderr, "%s: error writing vctr file %s\n", command, filename);
        exit(1);
    }

    // Write .truth
    if (config_file)
    {
      l2b.frame.swath.SelectTruth(&truth);
      sprintf(filename, "%s.truth", vctr_base);
      if (! l2b.WriteVctr(filename,0))
      {
	fprintf(stderr, "%s: error writing vctr file %s\n", command, filename);
	exit(1);
      }
    }

    // Write .init.
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


