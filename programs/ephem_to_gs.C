//=========================================================//
// Copyright (C) 2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    ephem_to_gs
//
// SYNOPSIS
//    ephem_to_gs <ephem file>
//
// DESCRIPTION
//    Reads in a binary ephem file and writes out the data in the
//    ground system approved form. The output filename is created
//    and written to standard output for reference.
//
// OPTIONS
//    None.
//
// EXAMPLES
//    An example of a command line is:
//      % ephem_to_gs ephem.dat
//
// ENVIRONMENT
//    Not environment dependent.
//
// EXIT STATUS
//    The following exit values are returned:
//       1  Program executed successfully
//      >0  Program had an error
//
// NOTES
//    None.
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
#include "Ephemeris.h"
#include "ETime.h"
#include "BufferedList.h"
#include "BufferedList.C"
#include "List.h"
#include "List.C"

//-----------//
// TEMPLATES //
//-----------//

template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<EarthPosition>;

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

const char* usage_array[] = { "<ephem_file>", 0};

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

    int clidx = 1;
    const char* ephem_file = argv[clidx++];

    //---------------------//
    // open the ephem file //
    //---------------------//

    FILE* ephem_fp = fopen(ephem_file,"r");
    if (ephem_fp == NULL)
    {
        fprintf(stderr, "%s: error opening ephem file %s\n", command,
            ephem_file);
        exit(1);
    }

    //----------------//
    // read the first //
    //----------------//

    int num_data_records = 0;

    OrbitState os;
    if (! os.Read(ephem_fp))
    {
        fprintf(stderr, "%s: error reading ephemeris file %s\n", command,
            ephem_file);
        exit(1);
    }
    num_data_records++;

    //--------------------------------//
    // determine the output file name //
    //--------------------------------//

    ETime beginning_date;
    beginning_date.SetTime(os.time);
    char beginning_block_b[BLOCK_B_TIME_LENGTH];
    beginning_date.ToBlockB(beginning_block_b);

    ETime production_date;
    production_date.CurrentTime();
    char production_block_b[BLOCK_B_TIME_LENGTH];
    production_date.ToBlockB(production_block_b);

    char output_file[1024];
    sprintf(output_file, "SW_SEPHG%s.%s", beginning_block_b,
        production_block_b);

    //------------------//
    // open output file //
    //------------------//

    FILE* ofp = fopen(output_file, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            output_file);
        exit(1);
    }

    //-----------------------//
    // write an empty header //
    //-----------------------//

    char blank_line[80];
    sprintf(blank_line, "%80s", ";\r\n");
    for (int i = 0; i < 21; i++)
    {
        fwrite(blank_line, 80, 1, ofp);
    }

    //----------------//
    // loop and write //
    //----------------//

    do
    {
        if (! os.Write(ofp))
        {
            fprintf(stderr, "%s: error writing ephemeris record\n", command);
            exit(1);
        }

        if (! os.Read(ephem_fp))
        {
            break;    // just boldly assume that this means EOF
        }
        num_data_records++;
    } while (1);

    ETime ending_date;
    ending_date.SetTime(os.time);

    //----------------------//
    // close the input file //
    //----------------------//

    fclose(ephem_fp);

    //-------------------------------//
    // rewind and fill in the header //
    //-------------------------------//

    fseek(ofp, 0, SEEK_SET);
    fprintf(ofp, "num_header_records            = 21                                           ;\r\n");
    fprintf(ofp, "LongName                      = SeaWinds GPS Ephemeris Data                  ;\r\n");
    fprintf(ofp, "ShortName                     = SWSEPHG                                      ;\r\n");
    fprintf(ofp, "producer_agency               = NASA                                         ;\r\n");
    fprintf(ofp, "producer_institution          = JPL                                          ;\r\n");
    fprintf(ofp, "project_id                    = QuikSCAT                                     ;\r\n");
    fprintf(ofp, "data_format_type              = BINARY                                       ;\r\n");
    fprintf(ofp, "GranulePointer                = QS_SEPHG20002090602.20002090823              ;\r\n");
    fprintf(ofp, "InputPointer                  = qst20000727080447p01hk2.dat                  ;\r\n");
    fprintf(ofp, "sis_id                        = 686-644-15/1998-08-24                        ;\r\n");
    fprintf(ofp, "build_id                      = 2.3.2/2000-04-07                             ;\r\n");

    char beginning_code_b[CODE_B_TIME_LENGTH];
    beginning_date.ToCodeB(beginning_code_b);
    beginning_code_b[8] = '\0';
    fprintf(ofp, "RangeBeginningDate            = %s                                     ;\r\n", beginning_code_b);
    fprintf(ofp, "RangeBeginningTime            = %s                                 ;\r\n", beginning_code_b + 9);

    char ending_code_b[CODE_B_TIME_LENGTH];
    ending_date.ToCodeB(ending_code_b);
    ending_code_b[8] = '\0';
    fprintf(ofp, "RangeEndingDate               = %s                                     ;\r\n", ending_code_b);
    fprintf(ofp, "RangeEndingTime               = %s                                 ;\r\n", ending_code_b + 9);

    char production_code_b[CODE_B_TIME_LENGTH];
    production_date.ToCodeB(production_code_b);
    fprintf(ofp, "ProductionDateTime            = %s                        ;\r\n", production_code_b);
    fprintf(ofp, "ephemeris_type                = GPS                                          ;\r\n");
    fprintf(ofp, "num_data_records              = %-45d;\r\n",
        num_data_records);
    fprintf(ofp, "data_record_length            = 56                                           ;\r\n");
    fprintf(ofp, "spare_metadata_element        =                                              ;\r\n");
    fprintf(ofp, "spare_metadata_element        =                                              ;\r\n");

    fclose(ofp);
    return (0);
}
