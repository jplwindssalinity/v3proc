//=========================================================//
// Copyright (C) 2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    hdf_l1a_writer
//
// SYNOPSIS
//    hdf_l1a_writer [ -l leap_seconds ] <config_file> [ PE_l1a_file ]
//        <HDF_l1a_file>
//
// DESCRIPTION
//    Reads in a PE Level 1A file (or standard input, if a PE L1A file
//    is not specified) and converts it to an HDF L1A file.
//
// OPTIONS
//    The following options are supported:
//      [ -l leap_seconds ]  Add the specified number of leap seconds.
//
// OPERANDS
//    The following operands are supported:
//      <config_file>    The configuration file.
//      [ PE_l1a_file ]  A PE level 1a input file. If this is not
//                       specified, standard input is read.
//      <HDF_l1a_file>   The output HDF L1A file.
//
// EXAMPLES
//    An example of a command line is:
//      % hdf_l1a_writer qscat.cfg l1a.dat QS_S1A08068.20010060320
//    or
//      % sim qscat.cfg | hdf_l1a_writer qscat.cfg QS_S1A08068.20010060320
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
#include <string.h>
#include "Misc.h"
#include "L1AH.h"
#include "ConfigList.h"
#include "ConfigSim.h"
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Tracking.h"
#include "Tracking.C"

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

//-----------//
// TEMPLATES //
//-----------//

template List<Meas>;
template BufferedList<OrbitState>;
template List<OrbitState>;
template List<AngleInterval>;
template TrackerBase<unsigned short>;
template TrackerBase<unsigned char>;
template List<EarthPosition>;
template List<WindVectorPlus>;
template List<MeasSpot>;
template List<OffsetList>;
template List<long>;
template List<StringPair>;

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -l leap_seconds ]", "<config_file>",
    "[ PE_l1a_file ]", "<HDF_l1a_file>", 0 };

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

    const char* config_file = argv[optind++];

    const char* pe_l1a_file = NULL;
    const char* hdf_l1a_file = NULL;
    if (argc == optind + 1)
    {
        hdf_l1a_file = argv[optind++];
    }
    else if (argc == optind + 2)
    {
        pe_l1a_file = argv[optind++];
        hdf_l1a_file = argv[optind++];
    }
    else
    {
        usage(command, usage_array, 1);
    }

    //---------------------//
    // read in config file //
    //---------------------//

    ConfigList config_list;
    if (! config_list.Read(config_file))
    {
        fprintf(stderr, "%s: error reading sim config file %s\n",
            command, config_file);
        exit(1);
    }

    //-----------------------------//
    // determine orbit information //
    //-----------------------------//

    SpacecraftSim spacecraft_sim;
    if (! ConfigSpacecraftSim(&spacecraft_sim, &config_list))
    {
        fprintf(stderr, "%s: error configuring spacecraft simulator\n",
            command);
        exit(1);
    }
    double period = spacecraft_sim.GetPeriod();
    double inclination = spacecraft_sim.GetInclination();
    double sma = spacecraft_sim.GetSemiMajorAxis();
    double eccentricity = spacecraft_sim.GetEccentricity();

    //---------------------------//
    // configure the L1AH object //
    //---------------------------//

    L1AH l1a;
    if (! ConfigL1A(&l1a, &config_list))
    {
        fprintf(stderr, "%s: error configuring Level 1A\n", command);
        exit(1);
    }
    if (pe_l1a_file == NULL)
    {
        l1a.SetInputFp(stdin);
    }
    else
    {
        l1a.SetInputFilename(pe_l1a_file);
        if (! l1a.OpenForReading())
        {
            fprintf(stderr, "%s: error opening L1A file %s for reading\n",
                command, l1a.GetInputFilename());
            exit(1);
        }
    }

    //-----------------------------------//
    // determine the GS epoch in seconds //
    //-----------------------------------//

    ETime gs_epoch_etime;
    gs_epoch_etime.FromCodeA("1993-01-01T00:00:00.000");
    double gs_epoch = gs_epoch_etime.GetTime();

    //----------------------------//
    // create the output HDF file //
    //----------------------------//

    l1a.SetOutputFilename(hdf_l1a_file);
    l1a.OpenHdfForWriting();
    l1a.OpenSDSForWriting();

    //------------------------------//
    // create the Vdata's and SDS's //
    //------------------------------//

    if (! l1a.CreateVdatas())
    {
        fprintf(stderr, "%s: error creating Vdatas\n", command);
        exit(1);
    }
    if (! l1a.CreateSDSs())
    {
        fprintf(stderr, "%s: error creating SDSs\n", command);
        exit(1);
    }

    //------------------//
    // loop over frames //
    //------------------//

    do
    {
        //---------------//
        // read PE frame //
        //---------------//

        if (! l1a.ReadDataRec())
        {
            switch (l1a.GetStatus())
            {
            case L1A::OK:        // end of file
                break;
            case L1A::ERROR_READING_FRAME:
                fprintf(stderr, "%s: error reading Level 1A data\n", command);
                exit(1);
                break;
            case L1A::ERROR_UNKNOWN:
                fprintf(stderr, "%s: unknown error reading Level 1A data\n",
                    command);
                exit(1);
                break;
            default:
                fprintf(stderr, "%s: unknown status (???)\n", command);
                exit(1);
            }
            break;        // done, exit do loop
        }
        l1a.frame.Unpack(l1a.buffer);

        //------------------------//
        // adjust the frame times //
        //------------------------//

        double time = l1a.frame.time - gs_epoch + (double)leap_seconds;
        l1a.frame.time = time;

        //-----------------//
        // write HDF frame //
        //-----------------//

        if (! l1a.WriteHDFFrame())
        {
            fprintf(stderr, "%s: error writing HDF frame\n", command);
            exit(1);
        }
        l1a.NextRecord();

    } while (1);

    //----------------------//
    // write the HDF header //
    //----------------------//

    if (! l1a.WriteHDFHeader(period, inclination, sma, eccentricity))
    {
        fprintf(stderr, "%s: error writing HDF header\n", command);
//        exit(1);
    }

    //------------------//
    // end Vdata access //
    //------------------//

    l1a.EndVdataOutput();

    //----------------//
    // end SDS access //
    //----------------//

    l1a.EndSDSOutput();

    //----------------//
    // close HDF file //
    //----------------//

    l1a.CloseHdfOutputFile();
//    l1a.CloseSDSOutputFile();

    //---------------//
    // close PE file //
    //---------------//

    l1a.CloseInputFile();

    return (0);
}
