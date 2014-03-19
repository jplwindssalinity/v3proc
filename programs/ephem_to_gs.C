//=========================================================//
// Copyright (C) 2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    ephem_to_gs
//
// SYNOPSIS
//    ephem_to_gs [ -l leap_seconds ] <ephem file>
//
// DESCRIPTION
//    Reads in a binary ephem file and writes out the data in the
//    ground system approved form. The output filename is created
//    and written to standard output for reference.
//
// OPTIONS
//    The following options are supported:
//      [ -l leap_seconds ]  Add the specified number of leap seconds.
//
// EXAMPLES
//    An example of a command line is:
//      % ephem_to_gs -l 5 ephem.dat
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
#include <stdlib.h>
#include <unistd.h>
#include "Misc.h"
#include "Ephemeris.h"
#include "ETime.h"
#include "BufferedList.h"
#include "List.h"

//-----------//
// TEMPLATES //
//-----------//

template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<EarthPosition>;

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING  "o:l:d:"

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

const char* usage_array[] = { "[-o outfile] || [-d outdir] [ -l leap_seconds ]", "<ephem_file>", 0};

//--------------//
// MAIN PROGRAM //
//--------------//

int
main(
    int    argc,
    char*  argv[])
{
    int leap_seconds = 0;

    //------------------------//
    // parse the command line //
    //------------------------//

    const char* command = no_path(argv[0]);
    extern char* optarg;
    char* outfile = NULL;
    char* outdir = NULL;
    extern int optind;

    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
        case 'o':
            outfile = optarg;
            break;
        case 'd':
            outdir = optarg;
            break;
        case 'l':
            leap_seconds = atoi(optarg);
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }
    
    if( !outfile == !outdir )
      usage(command, usage_array, 1);
    
    if (argc != optind + 1)
        usage(command, usage_array, 1);

    const char* ephem_file = argv[optind++];

    //-----------------------------------//
    // determine the GS epoch in seconds //
    //-----------------------------------//

    ETime gs_epoch_etime;
    gs_epoch_etime.FromCodeA("1993-01-01T00:00:00.000");
    double gs_epoch = gs_epoch_etime.GetTime();

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
    // loop and write //
    //----------------//

    FILE* ofp = NULL;
    int num_data_records = 0;

    double last_time = 0.0;
    ETime beginning_date, production_date;

    do
    {
        OrbitState os;
        if (! os.Read(ephem_fp))
        {
            if (feof(ephem_fp))
                break;
            else
            {
                fprintf(stderr, "%s: error reading ephemeris record from %s\n",
                    command, ephem_file);
                exit(1);
            }
        }

        if (num_data_records == 0)
        {
            // this is the first record, name and open output file
            beginning_date.SetTime(os.time);
            char beginning_block_b[BLOCK_B_TIME_LENGTH];
            beginning_date.ToBlockB(beginning_block_b);

            production_date.CurrentTime();
            char production_block_b[BLOCK_B_TIME_LENGTH];
            production_date.ToBlockB(production_block_b);

            if( outdir ) {
              outfile = new char[1024];
              sprintf(outfile, "SW_SEPHG%s.%s", beginning_block_b,
                      production_block_b);
            }
            
            ofp = fopen( outfile, "w");
            if (ofp == NULL)
            {
                fprintf(stderr, "%s: error opening output file %s\n", command,
                    outfile);
                exit(1);
            }

            // write an empty header
            char blank_line[80];
            sprintf(blank_line, "%80s", ";\r\n");
            for (int i = 0; i < 21; i++)
            {
                fwrite(blank_line, 80, 1, ofp);
            }
        }

        //-----------------------------------//
        // determine the adjusted frame time //
        //-----------------------------------//

        last_time = os.time;
        os.time = os.time - gs_epoch + (double)leap_seconds;

        if (! os.Write(ofp))
        {
            fprintf(stderr, "%s: error writing ephemeris record\n", command);
            exit(1);
        }

        num_data_records++;
    } while (1);

    ETime ending_date;
    ending_date.SetTime(last_time);

    //----------------------//
    // close the input file //
    //----------------------//

    fclose(ephem_fp);

    //-------------------------------//
    // rewind and fill in the header //
    //-------------------------------//

    fseek(ofp, 0, SEEK_SET);

    fprintf(ofp, "num_header_records            = 21                                           ;\r\n");
    fprintf(ofp, "LongName                      = ISS GPS Ephemeris Data                       ;\r\n");
    fprintf(ofp, "ShortName                     = SWSEPHG                                      ;\r\n");
    fprintf(ofp, "producer_agency               = NASA                                         ;\r\n");
    fprintf(ofp, "producer_institution          = JPL                                          ;\r\n");
    fprintf(ofp, "project_id                    = RapidSCAT                                    ;\r\n");
    fprintf(ofp, "data_format_type              = BINARY                                       ;\r\n");
    fprintf(ofp, "GranulePointer                = %-45.45s;\r\n",no_path(outfile));
    fprintf(ofp, "InputPointer                  = %-45.45s;\r\n",no_path(ephem_file));
    fprintf(ofp, "sis_id                        = 686-644-15/1998-08-24                        ;\r\n");
    fprintf(ofp, "build_id                      = 2.3.2/2000-04-07                             ;\r\n");

    char beginning_code_b[CODE_B_TIME_LENGTH];
    beginning_date.ToCodeB(beginning_code_b);
    beginning_code_b[8] = '\0';
    fprintf(ofp, "RangeBeginningDate            = %-45.45s;\r\n", beginning_code_b);
    fprintf(ofp, "RangeBeginningTime            = %-45.45s;\r\n", beginning_code_b + 9);

    char ending_code_b[CODE_B_TIME_LENGTH];
    ending_date.ToCodeB(ending_code_b);
    ending_code_b[8] = '\0';
    fprintf(ofp, "RangeEndingDate               = %-45.45s;\r\n", ending_code_b);
    fprintf(ofp, "RangeEndingTime               = %-45.45s;\r\n", ending_code_b + 9);

    char production_code_b[CODE_B_TIME_LENGTH];
    production_date.ToCodeB(production_code_b);
    fprintf(ofp, "ProductionDateTime            = %-45.45s;\r\n", production_code_b);
    fprintf(ofp, "ephemeris_type                = GPS                                          ;\r\n");
    fprintf(ofp, "num_data_records              = %-45d;\r\n",
        num_data_records);
    fprintf(ofp, "data_record_length            = 56                                           ;\r\n");
    fprintf(ofp, "spare_metadata_element        =                                              ;\r\n");
    fprintf(ofp, "spare_metadata_element        =                                              ;\r\n");

    fclose(ofp);
    if ( outdir != NULL ) delete[] outfile;
    return (0);
}
