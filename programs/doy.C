//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    doy
//
// SYNOPSIS
//    doy <date_or_doy>
//
// DESCRIPTION
//    Converts between date and doy.  Available formats are
//    yyyy-mm-dd
//    dd/mm/yy
//    dd/mm/yyyy
//    doy/yyyy
//    doy/yy
//    yyyy-doy
//
// OPTIONS
//    None.
//
// OPERANDS
//    <date_or_doy>  A date or doy.  The programs figures it out.
//
// EXAMPLES
//    An example of a command line is:
//      % doy 1999-10-02
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
// AUTHOR
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
#include <time.h>
#include "Misc.h"

//-----------//
// CONSTANTS //
//-----------//

#define FORMAT_1  "%Y-%m-%d"
#define FORMAT_2  "%m/%d/%y"
#define FORMAT_3  "%m/%d/%Y"
#define FORMAT_4  "%Y-%j"
#define FORMAT_5  "%j/%y"
#define FORMAT_6  "%j/%Y"

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "<date_or_doy>", 0 };

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
    if (argc != 2)
        usage(command, usage_array, 1);

    const char* time_string = argv[1];

    //------------------------//
    // try a bunch of parsing //
    //------------------------//

    int from_doy = 0;
    int to_doy = 0;
    struct tm tm_time;

    if (strptime(time_string, FORMAT_1, &tm_time) != NULL)
        to_doy = 1;
    else if (strptime(time_string, FORMAT_2, &tm_time) != NULL)
        to_doy = 1;
    else if (strptime(time_string, FORMAT_3, &tm_time) != NULL)
        to_doy = 1;
    else if (strptime(time_string, FORMAT_4, &tm_time) != NULL)
        from_doy = 1;
    else if (strptime(time_string, FORMAT_5, &tm_time) != NULL)
        from_doy = 1;
    else if (strptime(time_string, FORMAT_6, &tm_time) != NULL)
        from_doy = 1;
    else
    {
        fprintf(stderr, "%s: error parsing input %s\n", command, time_string);
        exit(1);
    }

    //---------//
    // convert //
    //---------//
    
    if (to_doy)
    {
        mktime(&tm_time);
        printf("%04d-%02d-%02d -> %04d-%03d\n", tm_time.tm_year + 1900,
            tm_time.tm_mon + 1, tm_time.tm_mday, tm_time.tm_year + 1900,
            tm_time.tm_yday + 1);
    }
    if (from_doy)
    {
        printf("%04d-%03d -> %04d-%02d-%02d\n", tm_time.tm_year + 1900,
            tm_time.tm_yday + 1, tm_time.tm_year + 1900, tm_time.tm_mon + 1,
            tm_time.tm_mday);
    }

    return(0);
}
