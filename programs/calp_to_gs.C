//=========================================================//
// Copyright (C) 2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    calp_to_gs
//
// SYNOPSIS
//    calp_to_gs [ -l leap_seconds ] <calpulse_file>
//
// DESCRIPTION
//    Reads in a binary calpulse file and writes out the data in the
//    ground system approved form. The output filename is created
//    and written to standard output for reference.
//
// OPTIONS
//    The following options are supported:
//      [ -l leap_seconds ]  Add the specified number of leap seconds.
//
// EXAMPLES
//    An example of a command line is:
//      % calp_to_gs -l 5 calp.dat
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
#include "Misc.h"
#include "Ephemeris.h"
#include "ETime.h"
#include "L1A.h"
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

const char* usage_array[] = { "[ -l leap_seconds ]", "<calpulse_file>", 0};

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

    const char* calp_file = argv[optind++];

    //-----------------------------------//
    // determine the GS epoch in seconds //
    //-----------------------------------//

    ETime gs_epoch_etime;
    gs_epoch_etime.FromCodeA("1993-01-01T00:00:00.000");
    double gs_epoch = gs_epoch_etime.GetTime();

    //--------------------//
    // open the calp file //
    //--------------------//

    FILE* calp_fp = fopen(calp_file, "r");
    if (calp_fp == NULL)
    {
        fprintf(stderr, "%s: error opening calpulse file %s\n", command,
            calp_file);
        exit(1);
    }

    //----------------//
    // loop and write //
    //----------------//

    char output_file[1024];
    FILE* ofp = NULL;
    char buffer[154];
    int num_data_records = 0;

    ETime beginning_date, production_date;
    L1A_Calibration_Pulse_Type cpr;
    do
    {
        if (fread(&cpr, sizeof(L1A_Calibration_Pulse_Type), 1, calp_fp) != 1)
        {
            if (feof(calp_fp))
                break;
            else
            {
                 fprintf(stderr, "%s: error reading calpulse record from %s\n",
                     command, calp_file);
                 exit(1);
            }
        }

        if (num_data_records == 0)
        {
            // this is the first record, name and open output file
            beginning_date.SetTime(cpr.frame_time_cal_secs);
            char beginning_block_b[BLOCK_B_TIME_LENGTH];
            beginning_date.ToBlockB(beginning_block_b);

            production_date.CurrentTime();
            char production_block_b[BLOCK_B_TIME_LENGTH];
            production_date.ToBlockB(production_block_b);

            sprintf(output_file, "SW_SCAL%s.%s", beginning_block_b,
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
            for (int i = 0; i < 77; i++)
            {
                fwrite(blank_line, 80, 1, ofp);
            }
        }

        //-----------------------------------//
        // determine the adjusted frame time //
        //-----------------------------------//

        double gs_time = cpr.frame_time_cal_secs - gs_epoch
            + (double)leap_seconds;

        //--------------------//
        // transfer to buffer //
        //--------------------//

        char* ptr = buffer;
        memcpy(ptr, &(gs_time), sizeof(double));
        ptr += sizeof(double);
        memcpy(ptr, &(cpr.true_cal_pulse_pos), sizeof(char));
        ptr += sizeof(char);
        memcpy(ptr, &(cpr.beam_identifier), sizeof(unsigned char));
        ptr += sizeof(unsigned char);
        memcpy(ptr, &(cpr.loop_back_cal_power),
            SLICES_PER_PULSE * sizeof(unsigned int));
        ptr += SLICES_PER_PULSE * sizeof(unsigned int);
        memcpy(ptr, &(cpr.loop_back_cal_noise), sizeof(unsigned int));
        ptr += sizeof(unsigned int);
        memcpy(ptr, &(cpr.load_cal_power),
            SLICES_PER_PULSE * sizeof(unsigned int));
        ptr += SLICES_PER_PULSE * sizeof(unsigned int);
        memcpy(ptr, &(cpr.load_cal_noise), sizeof(unsigned int));
        ptr += sizeof(unsigned int);
        memcpy(ptr, &(cpr.rj_temp_eu), sizeof(float));
        ptr += sizeof(float);
        memcpy(ptr, &(cpr.precision_coupler_temp_eu), sizeof(float));
        ptr += sizeof(float);
        memcpy(ptr, &(cpr.rcv_protect_sw_temp_eu), sizeof(float));
        ptr += sizeof(float);
        memcpy(ptr, &(cpr.beam_select_sw_temp_eu), sizeof(float));
        ptr += sizeof(float);
        memcpy(ptr, &(cpr.receiver_temp_eu), sizeof(float));
        ptr += sizeof(float);
        memcpy(ptr, &(cpr.transmit_power_inner), sizeof(float));
        ptr += sizeof(float);
        memcpy(ptr, &(cpr.transmit_power_outer), sizeof(float));
        ptr += sizeof(float);
        memcpy(ptr, &(cpr.frame_inst_status), sizeof(unsigned int));
        ptr += sizeof(unsigned int);
        memcpy(ptr, &(cpr.frame_err_status), sizeof(unsigned int));
        ptr += sizeof(unsigned int);
        memcpy(ptr, &(cpr.frame_qual_flag), sizeof(short));
        ptr += sizeof(short);

        if (fwrite(buffer, 154, 1, ofp) != 1)
        {
             fprintf(stderr, "%s: error writing calpulse record to %s\n",
                 command, output_file);
             exit(1);
        }

        num_data_records++;
    } while (1);

    ETime ending_date;
    ending_date.SetTime(cpr.frame_time_cal_secs);

    //----------------------//
    // close the input file //
    //----------------------//

    fclose(calp_fp);

    //-------------------------------//
    // rewind and fill in the header //
    //-------------------------------//

    fseek(ofp, 0, SEEK_SET);
    fprintf(ofp, "num_header_records            = 77                                           ;\r\n");
    fprintf(ofp, "LongName                      = SeaWinds Calibration Pulse Data Product      ;\r\n");
    fprintf(ofp, "ShortName                     = SWSCAL                                       ;\r\n");
    fprintf(ofp, "producer_agency               = NASA                                         ;\r\n");
    fprintf(ofp, "producer_institution          = JPL                                          ;\r\n");
    fprintf(ofp, "project_id                    = SeaWinds                                     ;\r\n");
    fprintf(ofp, "data_format_type              = BINARY                                       ;\r\n");
    fprintf(ofp, "GranulePointer                = SW_SCAL05751.20002841242                     ;\r\n");
    fprintf(ofp, "num_input_files               = 1                                            ;\r\n");
    fprintf(ofp, "InputPointer1                 = SW_S0A20002090044.20002090329                ;\r\n");
    fprintf(ofp, "sis_id                        = 686-644-21/2000-01-01                        ;\r\n");
    fprintf(ofp, "build_id                      = 2.3.2-4/2000-09-26                           ;\r\n");

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
    fprintf(ofp, "num_data_records              = %-45d;\r\n",
        num_data_records);
    fprintf(ofp, "data_record_length            = 154                                          ;\r\n");
    for (int i = 0; i < 58; i++)
    {
        fprintf(ofp, "spare_metadata_element        =                                              ;\r\n");
    }

    fclose(ofp);
    return (0);
}
