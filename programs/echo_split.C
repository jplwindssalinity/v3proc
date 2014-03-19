//==============================================================//
// Copyright (C) 1998-1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    echo_split
//
// SYNOPSIS
//    echo_split [ -o output_base ] <echo_data_file...>
//
// DESCRIPTION
//    Splits the echo data files into orbit step files named:
//        output_base.orbitstep or echo.orbitstep
//
// OPTIONS
//    The following options are supported:
//      [ -o output_base ]  Use output_base to create output filesnames.
//
// OPERANDS
//    The following operands are supported:
//      <echo_data_file..>   The echo data input files.
//
// EXAMPLES
//    An example of a command line is:
//      % echo_split -o day1.echo echo.*
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
#include <unistd.h>
#include "Misc.h"
#include "List.h"
#include "Tracking.h"
#include "BufferedList.h"
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

#define OPTSTRING            "o:"

#define ORBIT_STEPS          256
#define MAX_SPOTS            10000
#define MAX_EPHEM            200
#define DEFAULT_OUTPUT_BASE  "echo"

//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

int  put_in_file(const char* output_base, EchoInfo* echo_info);

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -o output_base ]", "<echo_data_file..>", 0 };

int g_created[ORBIT_STEPS];
int g_fd = -1;
int g_fd_orbit_step = -1;

//--------------//
// MAIN PROGRAM //
//--------------//

int
main(
    int    argc,
    char*  argv[])
{
    //------------//
    // initialize //
    //------------//

    char* output_base = DEFAULT_OUTPUT_BASE;

    //------------------------//
    // parse the command line //
    //------------------------//

    const char* command = no_path(argv[0]);

    if (argc == 1)
    {
        usage(command, usage_array, 1);
        exit(1);
    }

    extern char *optarg;
    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
        case 'o':
            output_base = optarg;
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    char** echo_data_files = (argv + optind);
    int file_count = argc - optind;

    //----------------------------//
    // for each echo data file... //
    //----------------------------//

	for (int file_idx = 0; file_idx < file_count; file_idx++)
    {
        char* echo_data_file = echo_data_files[file_idx];

        //---------------------------------//
        // open echo data file for reading //
        //---------------------------------//

printf("%s\n", echo_data_file);
        int ifd = open(echo_data_file, O_RDONLY);
        if (ifd == -1)
        {
            fprintf(stderr, "%s: error opening echo data file %s\n", command,
                echo_data_file);
            exit(1);
        }

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
                    fprintf(stderr, "%s: error reading echo file %s\n",
                        command, echo_data_file);
                    exit(1);
                }
            }

            //----------------------------------------//
            // if entire frame is from one orbit step //
            // than simply put it in the right file   //
            //----------------------------------------//

            if (echo_info.priOfOrbitStepChange == 255)
            {
                if (! put_in_file(output_base, &echo_info))
                {
                    fprintf(stderr, "%s: error writing echo info\n", command);
                    exit(1);
                }
            }
            else
            {
                //--------------------------------//
                // deal with changing orbit steps //
                //--------------------------------//

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

                //-----------------//
                // write echo info //
                //-----------------//

                if (! put_in_file(output_base, &echo_info))
                {
                    fprintf(stderr, "%s: error writing echo info\n", command);
                    exit(1);
                }

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

                //-----------------//
                // write echo info //
                //-----------------//

                if (! put_in_file(output_base, &echo_info))
                {
                    fprintf(stderr, "%s: error writing echo info\n", command);
                    exit(1);
                }
            }
        } while (1);

        close(ifd);
    }

    close(g_fd);

    return (0);
}

//-------------//
// put_in_file //
//-------------//

int
put_in_file(
    const char*  output_base,
    EchoInfo*    echo_info)
{
    //----------------//
    // determine file //
    //----------------//

    int orbit_step = echo_info->orbitStep;
    if (g_fd != -1 && g_fd_orbit_step != orbit_step)
    {
        // file is open, but is the wrong orbit step
        close(g_fd);
        g_fd = -1;
        g_fd_orbit_step = -1;
    }

    if (g_fd == -1)
    {
        // file is closed, open it
        char filename[2048];
        sprintf(filename, "%s.%03d", output_base, orbit_step);
        int oflag;
        if (g_created[orbit_step])
        {
            // file already created, just append
            oflag = O_APPEND | O_WRONLY;
        }
        else
        {
            // need to create file
            oflag = O_CREAT | O_WRONLY;
        }
        g_fd = open(filename, oflag, 0644);
        if (g_fd == -1)
        {
            fprintf(stderr, "Error opening file %s\n", filename);
            return(0);
        }
        g_fd_orbit_step = orbit_step;
        g_created[orbit_step] = 1;
    }

    //---------------------//
    // write the echo info //
    //---------------------//

    if (! echo_info->Write(g_fd))
        return(0);

    return(1);
}
