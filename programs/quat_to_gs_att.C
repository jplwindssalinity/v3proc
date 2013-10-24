//=========================================================//
// Copyright (C) 2013, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    quat_to_gs_att.C
//
// SYNOPSIS
//    quat_to_gs_att [ -l leap_seconds ] <quat file>
//
// DESCRIPTION
//    Reads in a binary quat file and writes out the data in the
//    ground system approved form. The output filename is created
//    and written to standard output for reference.
//
// OPTIONS
//    The following options are supported:
//      [ -l leap_seconds ]  Add the specified number of leap seconds.
//
// EXAMPLES
//    An example of a command line is:
//      % quat_to_gs_att -l 5 quats.dat
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
#include "ETime.h"
#include "Quat.h"
#include "BufferedList.h"
#include "BufferedList.C"
#include "List.h"
#include "List.C"
#include "Attitude.h"
#include "Constants.h"

//-----------//
// TEMPLATES //
//-----------//
template class BufferedList<QuatRec>;
template class List<QuatRec>;


//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING  "l:"

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

const char* usage_array[] = { "[ -l leap_seconds ]", "<quat_file>", 0};

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
    extern int optind;

    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
        case 'l':
            leap_seconds = atoi(optarg);
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    if (argc != optind + 1)
        usage(command, usage_array, 1);

    const char* quat_file = argv[optind++];

    //-----------------------------------//
    // determine the GS epoch in seconds //
    //-----------------------------------//

    ETime gs_epoch_etime;
    gs_epoch_etime.FromCodeA("1993-01-01T00:00:00.000");
    double gs_epoch = gs_epoch_etime.GetTime();

    //--------------------------//
    // open the quaternion file //
    //--------------------------//

    FILE* quat_fp = fopen(quat_file,"r");
    if (quat_fp == NULL)
    {
        fprintf(stderr, "%s: error opening ephem file %s\n", command,
            quat_file);
        exit(1);
    }
    

    //----------------//
    // loop and write //
    //----------------//

    FILE* ofp = NULL;
    int num_data_records = 0;

    double last_time = 0.0;
    ETime beginning_date, production_date;

    char output_file[1024];
    
    do
    {
        QuatRec quat_rec;
        if (! quat_rec.Read(quat_fp))
        {
            if (feof(quat_fp))
                break;
            else
            {
                fprintf(stderr, "%s: error reading quaternion record from %s\n",
                    command, quat_file);
                exit(1);
            }
        }

        if (num_data_records == 0)
        {
            // this is the first record, name and open output file
            beginning_date.SetTime(quat_rec.time);
            char beginning_block_b[BLOCK_B_TIME_LENGTH];
            beginning_date.ToBlockB(beginning_block_b);

            production_date.CurrentTime();
            char production_block_b[BLOCK_B_TIME_LENGTH];
            production_date.ToBlockB(production_block_b);

            sprintf(output_file, "SW_SATTG%s.%s", beginning_block_b,
                production_block_b);

            ofp = fopen(output_file, "w");
            if (ofp == NULL)
            {
                fprintf(stderr, "%s: error opening output file %s\n", command,
                    output_file);
                exit(1);
            }

            // write an empty header
            char blank_line[80];
            sprintf(blank_line, "%80s", ";\r\n");
            for (int i = 0; i < 20; i++)
            {
                fwrite(blank_line, 80, 1, ofp);
            }
        }

        //-----------------------------------//
        // determine the adjusted frame time //
        //-----------------------------------//
        
        Attitude this_att;
        
        last_time = quat_rec.time;
        double this_time = quat_rec.time - gs_epoch + (double)leap_seconds;
        
        quat_rec.GetAttitudeGS(&this_att);
        float yaw        = this_att.GetYaw()   * rtd;
        float pitch      = this_att.GetPitch() * rtd;
        float roll       = this_att.GetRoll()  * rtd;
        float yaw_rate   = 0;
        float pitch_rate = 0;
        float roll_rate  = 0;
        
        fwrite( &this_time,  sizeof(double), 1, ofp );
        fwrite( &roll,       sizeof(float),  1, ofp );
        fwrite( &pitch,      sizeof(float),  1, ofp );
        fwrite( &yaw,        sizeof(float),  1, ofp );
        fwrite( &roll_rate,  sizeof(float),  1, ofp );
        fwrite( &pitch_rate, sizeof(float),  1, ofp );
        fwrite( &yaw_rate,   sizeof(float),  1, ofp );

        num_data_records++;
    } while (1);

    ETime ending_date;
    ending_date.SetTime(last_time);

    //----------------------//
    // close the input file //
    //----------------------//

    fclose(quat_fp);

    //-------------------------------//
    // rewind and fill in the header //
    //-------------------------------//

    fseek(ofp, 0, SEEK_SET);

    fprintf(ofp, "num_header_records            = 20                                           ;\r\n");
    fprintf(ofp, "LongName                      = RapidSCAT Attitude Data                      ;\r\n");
    fprintf(ofp, "ShortName                     = RSCATATT                                     ;\r\n");
    fprintf(ofp, "producer_agency               = NASA                                         ;\r\n");
    fprintf(ofp, "producer_institution          = JPL                                          ;\r\n");
    fprintf(ofp, "project_id                    = RapidSCAT                                    ;\r\n");
    fprintf(ofp, "data_format_type              = BINARY                                       ;\r\n");
    fprintf(ofp, "GranulePointer                = %-45.45s;\r\n",output_file);
    fprintf(ofp, "InputPointer                  = %-45.45s;\r\n",no_path(quat_file));
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
    fprintf(ofp, "attitude_type                 = ISS/LVLH                                     ;\r\n");
    fprintf(ofp, "num_data_records              = %-45d;\r\n",num_data_records);
    fprintf(ofp, "data_record_length            = 32                                           ;\r\n");
    fprintf(ofp, "spare_metadata_element        =                                              ;\r\n");

    fclose(ofp);
    return (0);
}

    
    

