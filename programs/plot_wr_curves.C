//==============================================================//
// Copyright (C) 1998-1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    plot_wr_curves
//
// SYNOPSIS
//    plot_wr_curves [ -k ] <sim_config_file> <output_base>
//
// DESCRIPTION
//    Generates solution curve plots for a given set of measurements.
//    Input values are prompted from standard input and are
//    measurement type (VV, HH, VH, HV, VVHV, HHVH), incidence angle
//    (deg.), azimuth angle (deg), and sigma-0 (dB).  The sequence of
//    input parameters is terminated by a line that fails to conform
//    to format.
//
// OPTIONS
//    [ -k ]  Use additional columns for Kp information.
//
// OPERANDS
//    The following operand is supported:
//    <sim_config_file>  The sim_config_file needed listing all input
//                         parameters, input files, and output files.
//
//    <output_base>      The base for the output filenames.
//
// EXAMPLES
//    An example of a command line is:
//      % plot_wr_curves -k pol.cfg plot.out
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
//    James N. Huddleston (hudd@casket.jpl.nasa.gov)
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
#include "ConfigList.h"
#include "GMF.h"
#include "ConfigSim.h"
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Tracking.h"
#include "Tracking.C"

//-----------//
// TEMPLATES //
//-----------//

// Class declarations needed for templates
// eliminates need to include the entire header file
class AngleInterval;

template class List<AngleInterval>;
template class List<EarthPosition>;
template class List<StringPair>;
template class List<WindVectorPlus>;
template class List<Meas>;
template class List<MeasSpot>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<long>;
template class List<OffsetList>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING  "k"

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
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -k ]", "<sim_config_file>", "<output_base>",
    0};

//--------------//
// MAIN PROGRAM //
//--------------//

int
main(
    int    argc,
    char*  argv[])
{
    //------------------//
    // option variables //
    //------------------//

    int opt_kp = 0;

    //------------------------//
    // parse the command line //
    //------------------------//

    const char* command = no_path(argv[0]);

    extern int optind;
    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
        case 'k':
            opt_kp = 1;
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    if (argc != optind + 2)
        usage(command, usage_array, 1);

    const char* config_file = argv[optind++];
    const char* output_base = argv[optind++];

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

    //-------------------------------------//
    // read the geophysical model function //
    //-------------------------------------//

    GMF gmf;
    if (! ConfigGMF(&gmf, &config_list))
    {
        fprintf(stderr, "%s: error configuring GMF\n", command);
        exit(1);
    }
gmf.SetSpdTol(0.1);

    //--------------//
    // configure Kp //
    //--------------//

    Kp kp;
    Kp* kp_ptr = NULL;
    if (opt_kp)
    {
        //--------------//
        // configure Kp //
        //--------------//

        if (! ConfigKp(&kp, &config_list))
        {
            fprintf(stderr, "%s: error configuring Kp\n", command);
            exit(1);
        }
        kp_ptr = &kp;
    }

    //------//
    // loop //
    //------//

    MeasList meas_list;
    char filename[1024];
    char line[1024];
    char typestring[1024];
    float inc, azi, s0, xk, en_slice, bandwidth, pulse_width, kpa, kpb, kpc;
    for (int file_idx = 1; ; file_idx++)
    {
        meas_list.FreeContents();

        sprintf(filename, "%s.%05d", output_base, file_idx);
        printf("\nEnter information for output file %s\n", filename);
        for (int meas_idx = 1; ; meas_idx++)
        {
            if (fgets(line, 1024, stdin) != line)
                break;

            if (line[0] == '#')
                continue; // skip comments

            int bad_read = 0;
            if (opt_kp)
            {
                if (sscanf(line, " %s %f %f %f %f %f %f %f %f %f %f",
                    typestring, &inc, &azi, &s0, &xk, &en_slice, &bandwidth,
                    &pulse_width, &kpa, &kpb, &kpc) != 11)
                {
                    bad_read = 1;
                }
            }
            else
            {
                if (sscanf(line, " %s %f %f %f", typestring, &inc, &azi,
                    &s0) != 4)
                {
                    bad_read = 1;
                }
            }

            if (bad_read)
            {
                if (meas_idx == 1)
                    return(0);
                else
                    break;
            }

            Meas* new_meas = new Meas();
            if (strcasecmp(typestring, "VV") == 0)
            {
                new_meas->measType = Meas::VV_MEAS_TYPE;
            }
            else if (strcasecmp(typestring, "HH") == 0)
            {
                new_meas->measType = Meas::HH_MEAS_TYPE;
            }
            else if (strcasecmp(typestring, "VH") == 0)
            {
                new_meas->measType = Meas::VH_MEAS_TYPE;
            }
            else if (strcasecmp(typestring, "HV") == 0)
            {
                new_meas->measType = Meas::HV_MEAS_TYPE;
            }
            else if (strcasecmp(typestring, "VVHV") == 0)
            {
                new_meas->measType = Meas::VV_HV_CORR_MEAS_TYPE;
            }
            else if (strcasecmp(typestring, "HHVH") == 0)
            {
                new_meas->measType = Meas::HH_VH_CORR_MEAS_TYPE;
            }
            else
            {
                fprintf(stderr, "%s: error parsing measurement type %s\n",
                    command, typestring);
                exit(1);
            }
            new_meas->incidenceAngle = inc * dtr;
            new_meas->eastAzimuth = azi * dtr;
            new_meas->value = s0;

            if (opt_kp)
            {
                new_meas->XK = xk;
                new_meas->EnSlice = en_slice;
                new_meas->bandwidth = bandwidth;
                new_meas->txPulseWidth = pulse_width;
                new_meas->A = kpa;
                new_meas->B = kpb;
                new_meas->C = kpc;
            }

            //-----------------//
            // add measurement //
            //-----------------//

            meas_list.Append(new_meas);
        }

        printf("Writing to file %s\n", filename);

        //------------------//
        // open output file //
        //------------------//

        FILE* ofp = fopen(filename, "w");
        if (ofp == NULL)
        {
            fprintf(stderr, "%s: error opening output file %s\n", command,
                filename);
            exit(1);
        }

        //-----------------------//
        // write solution curves //
        //-----------------------//

        gmf.WriteSolutionCurves(ofp, &meas_list, kp_ptr);

        //-------------------//
        // close output file //
        //-------------------//

        fclose(ofp);
    }

    return (0);
}
