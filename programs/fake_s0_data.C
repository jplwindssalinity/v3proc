//=========================================================//
// Copyright (C) 1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    fake_s0_data
//
// SYNOPSIS
//    fake_s0_data <sim_config_file> <spd> <dir> <output_base>
//
// DESCRIPTION
//    Generates measurements for a given wind speed and direction and
//    sets of viewing geometries.  Input values are prompted from
//    standard input and are measurement type (VV, HH, VH, HV, VVHV,
//    HHVH), incidence angle (deg.), azimuth angle (deg), and sigma-0
//    (dB).  The sequence of input parameters is terminated by a line
//    that fails to conform to format.
//
// OPTIONS
//    None.
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
//      % fake_s0_data pol.cfg 3.0 30.0 data.out
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

const char* usage_array[] = { "<sim_config_file>", "<spd>", "<dir>",
    "<output_base>", 0};

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
    if (argc != 5)
        usage(command, usage_array, 1);

    int clidx = 1;
    const char* config_file = argv[clidx++];
    float spd = atof(argv[clidx++]);
    float dir = atof(argv[clidx++]) * dtr;
    const char* output_base = argv[clidx++];

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

    //------//
    // loop //
    //------//

    MeasList meas_list;
    char line[1024];
    char typestring[1024];
    float inc, azi, s0;
    for (int file_idx = 1; ; file_idx++)
    {
        meas_list.FreeContents();
        for (int meas_idx = 1; ; meas_idx++)
        {
            if (fgets(line, 1024, stdin) != line)
                break;

            if (line[0] == '#')
                continue;    // skip comments
            if (sscanf(line, " %s %f %f", typestring, &inc, &azi) != 3)
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

            //-----------------//
            // add measurement //
            //-----------------//

            meas_list.Append(new_meas);
        }

        //------------------//
        // open output file //
        //------------------//

        FILE* ofp = fopen(output_base, "w");
        if (ofp == NULL)
        {
            fprintf(stderr, "%s: error opening output file %s\n", command,
                output_base);
            exit(1);
        }

        //-------//
        // write //
        //-------//

        for (Meas* meas = meas_list.GetHead(); meas;
            meas = meas_list.GetNext())
        {
            char* typestring;
            switch(meas->measType)
            {
            case Meas::VV_MEAS_TYPE:
                typestring = "VV";
                break;
            case Meas::HH_MEAS_TYPE:
                typestring = "HH";
                break;
            case Meas::VH_MEAS_TYPE:
                typestring = "VH";
                break;
            case Meas::HV_MEAS_TYPE:
                typestring = "HV";
                break;
            case Meas::VV_HV_CORR_MEAS_TYPE:
                typestring = "VVHV";
                break;
            case Meas::HH_VH_CORR_MEAS_TYPE:
                typestring = "HHVH";
                break;
            default:
                break;
            }
            float s0;
            float chi = dir - meas->eastAzimuth + pi;
            gmf.GetInterpolatedValue(meas->measType, meas->incidenceAngle,
                spd, chi, &s0);
            fprintf(ofp, "%s %g %g %g\n", typestring,
                meas->incidenceAngle * rtd, meas->eastAzimuth * rtd, s0);
        }

        //-------------------//
        // close output file //
        //-------------------//

        fprintf(ofp, "END\nEND\n");

        fclose(ofp);
    }

    return (0);
}
