//==============================================================//
// Copyright (C) 1998-1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    echo_split
//
// SYNOPSIS
//    echo_split <echo_data_file> [ output_base ]
//
// DESCRIPTION
//    Splits the echo data file into orbit steps.  Adds the extension:
//        .orbit_step.codeBtime
//        .023.1999-193T23:35:01.232
//
// OPTIONS
//    None.
//
// OPERANDS
//    The following operands are supported:
//      <echo_data_file>   The echo data input file.
//      [ output_base ]    The output file base name.
//
// EXAMPLES
//    An example of a command line is:
//      % echo_split qscat.echo qe
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
#include <fcntl.h>
#include "Misc.h"
#include "List.h"
#include "List.C"
#include "Tracking.h"
#include "Tracking.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "echo_funcs.h"

//-----------//
// TEMPLATES //
//-----------//

template class List<Meas>;
template class List<EarthPosition>;
template class TrackerBase<unsigned short>;
template class TrackerBase<unsigned char>;
template class List<OrbitState>;
template class BufferedList<OrbitState>;
template class List<MeasSpot>;
template class List<OffsetList>;
template class List<long>;

//-----------//
// CONSTANTS //
//-----------//

#define MAX_SPOTS  10000
#define MAX_EPHEM  200

//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

int  make_file(const char* command, const char* output_base, int orbit_step,
         ETime time, char* output_file);

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "<echo_data_file>", "[ output_base ]", 0};

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

    if (argc != 2 && argc != 3)
        usage(command, usage_array, 1);

    int argidx = 1;
    const char* echo_data_file = argv[argidx++];
    const char* output_base;
    if (argc == 3)
        output_base = argv[argidx++];
    else
        output_base = echo_data_file;

    //---------------------------------//
    // open echo data file for reading //
    //---------------------------------//

    int ifd = open(echo_data_file, O_RDONLY);
    if (ifd == -1)
    {
        fprintf(stderr, "%s: error opening echo data file %s\n", command,
            echo_data_file);
        exit(1);
    }

    //------------//
    // initialize //
    //------------//

    int last_orbit_step = -1;
    char output_file[1024];
    int ofd = -1;

    //-----------------------//
    // read and process data //
    //-----------------------//

    do
    {
        //------//
        // read //
        //------//

        EchoInfo echo_info;
        int retval = echo_info.Read(ifd);
        if (retval != 1)
        {
            if (retval == -1)    // EOF
                break;
            else
            {
                fprintf(stderr, "%s: error reading echo file %s\n", command,
                    echo_data_file);
                exit(1);
            }
        }

        //--------------------//
        // deal with start-up //
        //--------------------//

        if (last_orbit_step == -1)
        {
            int use_orbit_step;
            if (echo_info.priOfOrbitStepChange == 255)
                use_orbit_step = echo_info.orbitStep;
            else
                use_orbit_step = (echo_info.orbitStep + 256 - 1) % 256;
 
            ofd = make_file(command, output_base, use_orbit_step,
                echo_info.frameTime, output_file);

            last_orbit_step = use_orbit_step;
        }

        //--------------------------------//
        // deal with changing orbit steps //
        //--------------------------------//

        if (echo_info.orbitStep != last_orbit_step)
        {
            unsigned char save_orbit_step = echo_info.orbitStep;
            unsigned char save_pri_of_orbit_step_change =
                echo_info.priOfOrbitStepChange;

            //---------------------//
            // save original flags //
            //---------------------//

            unsigned char save_flags[SPOTS_PER_FRAME];
            for (int i = 0; i < SPOTS_PER_FRAME; i++)
                save_flags[i] = echo_info.flag[i];

            //---------------------------------//
            // mark new orbit step data as bad //
            //---------------------------------//

            for (int i = save_pri_of_orbit_step_change;
                i < SPOTS_PER_FRAME; i++)
            {
                echo_info.flag[i] = EchoInfo::BAD_PEAK;
            }

            //------------------------------------//
            // "correct" the orbit step variables //
            //------------------------------------//
            // this makes it look like the orbit step didn't change

            echo_info.orbitStep = (save_orbit_step + 256 - 1) % 256;
            echo_info.priOfOrbitStepChange = 255;

            //------------------------------------------------//
            // write out the last frame of the old orbit step //
            //------------------------------------------------//

            if (! echo_info.Write(ofd))
            {
                fprintf(stderr,
                    "%s: error writing frame to output echo file %s\n",
                    command, output_file);
                exit(1);
            }

            //----------------//
            // close old file //
            //----------------//

            close(ofd);

            //------------------------//
            // restore original flags //
            //------------------------//

            for (int i = 0; i < SPOTS_PER_FRAME; i++)
                echo_info.flag[i] = save_flags[i];
    
            //---------------------------------//
            // mark old orbit step data as bad //
            //---------------------------------//

            for (int i = 0; i < save_pri_of_orbit_step_change; i++)
                echo_info.flag[i] = EchoInfo::BAD_PEAK;

            //------------------------------------//
            // "correct" the orbit step variables //
            //------------------------------------//
            // this makes it look like the orbit step didn't change

            echo_info.orbitStep = save_orbit_step;
            echo_info.priOfOrbitStepChange = 255;

            //----------------------//
            // open new output file //
            //----------------------//

            ofd = make_file(command, output_base, save_orbit_step,
                echo_info.frameTime, output_file);

            last_orbit_step = echo_info.orbitStep;
        }

        //-----------------//
        // write echo info //
        //-----------------//

        if (! echo_info.Write(ofd))
        {
            fprintf(stderr,
                "%s: error writing frame to output echo file %s\n",
                command, output_file);
            exit(1);
        }

    } while (1);

    close(ofd);
    close(ifd);

    return (0);
}

//-----------//
// make_file //
//-----------//

int
make_file(
    const char*  command,
    const char*  output_base,
    int          orbit_step,
    ETime        time,
    char*        output_file)
{
    char time_stamp[CODE_B_TIME_LENGTH];
    time.ToCodeB(time_stamp);

    if (output_base == NULL)
    {
        sprintf(output_file, "%03d.%s", orbit_step, time_stamp);
    }
    else
    {
        sprintf(output_file, "%s.%03d.%s", output_base, orbit_step,
            time_stamp);
    }
    int output_fd = creat(output_file, 0644);
    if (output_fd == -1)
    {
        fprintf(stderr, "%s: error opening output file %s\n",
            command, output_file);
        exit(1);
    }

    return(output_fd);
}
