//=========================================================//
// Copyright (C) 2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    echo_deltaf
//
// SYNOPSIS
//    echo_deltaf [ -o ] [ -f fit_base ] <output_base>
//        <echo_file...>
//
// DESCRIPTION
//    Reads the echo data files and fits a sinusoid to each orbit
//    step.
//
// OPTIONS
//    [ -o ]           Ocean only.
//    [ -f fit_base ]  Generate fit output with the given filename base.
//
// OPERANDS
//    The following operands are supported:
//      <echo_file...>  The echo data files.
//
// EXAMPLES
//    An example of a command line is:
//      % echo_deltaf -f fit qscat.cfg deltaf echo1.dat echo2.dat
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
#include <ieeefp.h>
#include <math.h>
#include "Misc.h"
#include "L1A.h"
#include "InstrumentGeom.h"
#include "List.h"
#include "List.C"
#include "Tracking.h"
#include "Tracking.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "AngleInterval.h"
#include "echo_funcs.h"

//-----------//
// TEMPLATES //
//-----------//

template class List<Meas>;
template class List<long>;
template class List<WindVectorPlus>;
template class List<MeasSpot>;
template class List<OffsetList>;
template class TrackerBase<unsigned char>;
template class List<OrbitState>;
template class BufferedList<OrbitState>;
template class List<EarthPosition>;
template class TrackerBase<unsigned short>;
template class List<AngleInterval>;

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING  "of:"

#define QUOTE  '"'

#define SIGNAL_ENERGY_THRESHOLD  0
#define ORBIT_STEPS              256
#define AZIMUTH_STEPS            256
#define SECTOR_COUNT             4
#define MIN_POINTS_PER_SECTOR    20
#define MIN_SECTORS              8

#define SPOTS_PER_FRAME          100

//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

int  process_orbit_step(int beam_idx, int orbit_step,
         const char* fit_base);
int  accumulate(int beam_idx, double azimuth, double delta_f);

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -o ]", "[ -f fit_base ]", "<output_base>",
    "<echo_file...>", 0 };

int       g_count[NUMBER_OF_QSCAT_BEAMS];
char      g_good[NUMBER_OF_QSCAT_BEAMS][ORBIT_STEPS];
int       g_good_count;
int       g_sector_count[NUMBER_OF_QSCAT_BEAMS][ORBIT_STEPS];

float     g_percentage[NUMBER_OF_QSCAT_BEAMS][AZIMUTH_STEPS][ORBIT_STEPS];
float     g_max_delta_f[NUMBER_OF_QSCAT_BEAMS][AZIMUTH_STEPS][ORBIT_STEPS];
float     g_max_raw_delta_f[NUMBER_OF_QSCAT_BEAMS][AZIMUTH_STEPS][ORBIT_STEPS];

// the following are allocated dynamically
double**  g_azimuth;
double**  g_delta_f;
off_t***  g_offsets;

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

    int opt_ocean_only = 0;    // default is to include land
    int opt_fit = 0;
    const char* fit_base = NULL;

    //------------------------//
    // parse the command line //
    //------------------------//

    const char* command = no_path(argv[0]);
    extern int optind;
    extern char *optarg;
    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
        case 'o':
            // this flag means ocean only
            opt_ocean_only = 1;
            break;
        case 'f':
            opt_fit = 1;
            fit_base = optarg;
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    if (argc < optind + 2)
        usage(command, usage_array, 1);

    const char* output_base = argv[optind++];
    int start_idx = optind;
    int end_idx = argc;
    int file_count = end_idx - start_idx;

    //-----------------------//
    // allocate offset table //
    //-----------------------//

    g_offsets = (off_t ***)make_array(sizeof(off_t), 3, file_count,
        ORBIT_STEPS, 2);
    if (g_offsets == NULL)
    {
        fprintf(stderr,
            "%s: error allocating offset table array (%d x %d x %d)\n",
            command, file_count, ORBIT_STEPS, 2);
        exit(1);
    }
    for (int i = 0; i < file_count; i++)
    {
        for (int j = 0; j < ORBIT_STEPS; j++)
        {
            for (int k = 0; k < 2; k++)
            {
                g_offsets[i][j][k] = 1;  // why 1?  it's an invalid offset
            }
        }
    }

    //--------------------------------------------//
    // set up offset table and max spots per step //
    //--------------------------------------------//

    printf("Setting up offset table\n");
    for (int file_idx = start_idx; file_idx < end_idx; file_idx++)
    {
        int file_idx_zero = file_idx - start_idx;

        //----------------//
        // open each file //
        //----------------//

        char* echo_file = argv[file_idx];
        printf("  %s\n", echo_file);

        int ifd = open(echo_file, O_RDONLY);
        if (ifd == -1)
        {
            fprintf(stderr, "%s: error opening echo data file %s\n", command,
                echo_file);
            exit(1);
        }

        //---------------------------------------//
        // read and accumulate orbit step counts //
        //---------------------------------------//
        // use the sector count array temporarily

        do
        {
            // get the offset before reading
            off_t current_offset = lseek(ifd, 0, SEEK_CUR);

            EchoInfo echo_info;
            int retval = echo_info.Read(ifd);
            if (retval != 1)
            {
                if (retval == -1)    // EOF
                    break;
                else
                {
                    fprintf(stderr, "%s: error reading echo file %s\n", command,
                        echo_file);
                    exit(1);
                }
            }
            for (int spot_idx = 0; spot_idx < SPOTS_PER_FRAME; spot_idx++)
            {
                int beam_idx = echo_info.SpotBeamIdx(spot_idx);
                int orbit_step = echo_info.SpotOrbitStep(spot_idx);

                if (echo_info.quality_flag[spot_idx] ==
                    EchoInfo::CAL_OR_LOAD_PULSE ||
                    echo_info.quality_flag[spot_idx] ==
                    EchoInfo::BAD_EPHEMERIS) { 
                    continue;
                }

                if (opt_ocean_only &&
                    echo_info.surface_flag[spot_idx] != EchoInfo::OCEAN)
                {
                    continue;
                }

                if (g_offsets[file_idx_zero][orbit_step][0] == 1)
                {
                    // start offset not set yet...so set it!
                    g_offsets[file_idx_zero][orbit_step][0] =
                        current_offset;
                }
                // might as well always set the ending offset
                g_offsets[file_idx_zero][orbit_step][1] = current_offset;
                g_sector_count[beam_idx][orbit_step]++;
            }
        } while (1);

        //----------------//
        // close the file //
        //----------------//

        close(ifd);
    }

    //-----------------------------------------------//
    // find the max and clear the sector count array //
    //-----------------------------------------------//

    int max_count = 0;
    for (int beam_idx = 0; beam_idx < NUMBER_OF_QSCAT_BEAMS; beam_idx++)
    {
        for (int orbit_step = 0; orbit_step < ORBIT_STEPS; orbit_step++)
        {
            if (g_sector_count[beam_idx][orbit_step] > max_count)
            {
                max_count = g_sector_count[beam_idx][orbit_step];
            }
            g_sector_count[beam_idx][orbit_step] = 0;
        }
    }

    printf("Maximum spots per orbit step = %d\n", max_count);

    //-----------------//
    // make the arrays //
    //-----------------//

    g_azimuth = (double **)make_array(sizeof(double), 2,
        NUMBER_OF_QSCAT_BEAMS, max_count);
    if (g_azimuth == NULL)
    {
        fprintf(stderr, "%s: error allocating azimuth array (%d x %d)\n",
            command, NUMBER_OF_QSCAT_BEAMS, max_count);
        exit(1);
    }

    g_delta_f = (double **)make_array(sizeof(double), 2,
        NUMBER_OF_QSCAT_BEAMS, max_count);
    if (g_delta_f == NULL)
    {
        fprintf(stderr,
            "%s: error allocating delta_f array (%d x %d)\n", command,
            NUMBER_OF_QSCAT_BEAMS, max_count);
        exit(1);
    }

    //------------------------------------------------//
    // read and process data orbit step by orbit step //
    //------------------------------------------------//

    for (int target_orbit_step = 0; target_orbit_step < ORBIT_STEPS;
        target_orbit_step++)
    {
        printf("Processing orbit step %d...\n", target_orbit_step);
        for (int file_idx = start_idx; file_idx < end_idx; file_idx++)
        {
            //----------------//
            // check for data //
            //----------------//

            int file_idx_zero = file_idx - start_idx;

            char* echo_file = argv[file_idx];
            printf("  %s\n", echo_file);

            if (g_offsets[file_idx_zero][target_orbit_step][0] == 1) {
                printf("    No data.\n");
                continue;
            }

            //----------------//
            // open each file //
            //----------------//

            int ifd = open(echo_file, O_RDONLY);
            if (ifd == -1)
            {
                fprintf(stderr, "%s: error opening echo data file %s\n",
                    command, echo_file);
                exit(1);
            }

            //-------------------------//
            // seek to starting offset //
            //-------------------------//

            lseek(ifd, g_offsets[file_idx_zero][target_orbit_step][0],
                SEEK_SET);

            //--------------------------------------------------//
            // read and process data from the target orbit step //
            //--------------------------------------------------//

            do
            {
                EchoInfo echo_info;
                int retval = echo_info.Read(ifd);
                if (retval != 1)
                {
                    if (retval == -1)    // EOF
                        break;
                    else
                    {
                        fprintf(stderr, "%s: error reading echo file %s\n",
                            command, echo_file);
                        exit(1);
                    }
                }

                //------------------//
                // check orbit step //
                //------------------//

                int first_orbit_step = echo_info.SpotOrbitStep(0);
                int last_orbit_step = echo_info.SpotOrbitStep(99);
                if (first_orbit_step != target_orbit_step &&
                    last_orbit_step != target_orbit_step)
                {
                    continue;
                }

                //------------------------------------------//
                // accumulate pulses with target orbit step //
                //------------------------------------------//
                // pulses must be OK and have a minimum energy

                for (int spot_idx = 0; spot_idx < SPOTS_PER_FRAME; spot_idx++)
                {
                    if (echo_info.quality_flag[spot_idx] ==
                        EchoInfo::CAL_OR_LOAD_PULSE ||
                        echo_info.quality_flag[spot_idx] ==
                        EchoInfo::BAD_EPHEMERIS) {
                        continue;
                    }

                    if (opt_ocean_only &&
                        echo_info.surface_flag[spot_idx] != EchoInfo::OCEAN)
                    {
                        continue;
                    }

                    int orbit_step = echo_info.SpotOrbitStep(spot_idx);
                    if (orbit_step != target_orbit_step)
                        continue;

                    double azimuth = two_pi *
                        (double)echo_info.idealEncoder[spot_idx] /
                        (double)ENCODER_N;
                    int beam_idx = echo_info.SpotBeamIdx(spot_idx);
                    accumulate(beam_idx, azimuth,
                        echo_info.deltaF[spot_idx]);
                }

            } while (lseek(ifd, 0, SEEK_CUR) <=
                g_offsets[file_idx_zero][target_orbit_step][1]);

            //----------------//
            // close the file //
            //----------------//

            close(ifd);
        }

        for (int beam_idx = 0; beam_idx < NUMBER_OF_QSCAT_BEAMS; beam_idx++)
        {
            process_orbit_step(beam_idx, target_orbit_step, fit_base);
        }
    }

    //--------//
    // output //
    //--------//

    char filename[1024];
    int x_size = AZIMUTH_STEPS;
    int y_size = ORBIT_STEPS;
    FILE* ofp = NULL;
    for (int beam_idx = 0; beam_idx < NUMBER_OF_QSCAT_BEAMS; beam_idx++)
    {
        //--------------------//
        // the fitted delta f //
        //--------------------//

        sprintf(filename, "%s.fit.%d", output_base, beam_idx + 1);
        ofp = fopen(filename, "w");
        if (ofp == NULL) {
            fprintf(stderr, "%s: error opening output file %s\n", command,
                filename);
            exit(1);
        }
        fwrite(&x_size, sizeof(int), 1, ofp);
        fwrite(&y_size, sizeof(int), 1, ofp);
        if (fwrite(g_max_delta_f[beam_idx], sizeof(float), x_size * y_size,
            ofp) != (unsigned int)(x_size * y_size))
        {
            fprintf(stderr, "%s: error writing output file %s\n", command,
                filename);
            exit(1);
        }
        fclose(ofp);

        //-----------------//
        // the raw delta f //
        //-----------------//

        sprintf(filename, "%s.raw.%d", output_base, beam_idx + 1);
        ofp = fopen(filename, "w");
        if (ofp == NULL) {
            fprintf(stderr, "%s: error opening output file %s\n", command,
                filename);
            exit(1);
        }
        fwrite(&x_size, sizeof(int), 1, ofp);
        fwrite(&y_size, sizeof(int), 1, ofp);
        if (fwrite(g_max_raw_delta_f[beam_idx], sizeof(float),
            x_size * y_size, ofp) != (unsigned int)(x_size * y_size))
        {
            fprintf(stderr, "%s: error writing output file %s\n", command,
                filename);
            exit(1);
        }
        fclose(ofp);
    }

    free_array(g_delta_f, 2, NUMBER_OF_QSCAT_BEAMS, max_count);
    free_array(g_offsets, 3, file_count, ORBIT_STEPS, 2);
    free_array(g_azimuth, 2, NUMBER_OF_QSCAT_BEAMS, max_count);

    return(0);
}

//--------------------//
// process_orbit_step //
//--------------------//

int
process_orbit_step(
    int          beam_idx,
    int          orbit_step,
    const char*  fit_base)
{
    //----------------//
    // check for data //
    //----------------//

    if (g_count[beam_idx] == 0) {
        return(1);
    }

    //----------------------------//
    // fit a sinusoid to the data //
    //----------------------------//

    double a, p, c;
    sinfit(g_azimuth[beam_idx], g_delta_f[beam_idx], NULL,
        g_count[beam_idx], &a, &p, &c);

    //-------------------------//
    // new align/overlap check //
    //-------------------------//

/*
    int align[SECTOR_COUNT], offset[SECTOR_COUNT];
    for (int sector_idx = 0; sector_idx < SECTOR_COUNT; sector_idx++)
    {
        align[sector_idx] = 0;
        offset[sector_idx] = 0;
    }

    for (int i = 0; i < g_count[beam_idx]; i++)
    {
        int align_idx = (int)(SECTOR_COUNT * g_azimuth[beam_idx][i] / two_pi);
        align[align_idx]++;

        int offset_idx = (int)(SECTOR_COUNT * g_azimuth[beam_idx][i] / two_pi +
            0.5) % SECTOR_COUNT;
        offset[offset_idx]++;
    }
    g_sector_count[beam_idx][orbit_step] = 0;
    for (int sector_idx = 0; sector_idx < SECTOR_COUNT; sector_idx++)
    {
        if (align[sector_idx] >= MIN_POINTS_PER_SECTOR)
            g_sector_count[beam_idx][orbit_step]++;
        if (offset[sector_idx] >= MIN_POINTS_PER_SECTOR)
            g_sector_count[beam_idx][orbit_step]++;
    }
*/

    //-----------------------//
    // calculate percentages //
    //-----------------------//

    int total_azimuth[AZIMUTH_STEPS];
    int bad_azimuth[AZIMUTH_STEPS];
    for (int i = 0; i < AZIMUTH_STEPS; i++) {
        total_azimuth[i] = 0;
        bad_azimuth[i] = 0;
    }
    for (int i = 0; i < g_count[beam_idx]; i++)
    {
        float delta_f = g_delta_f[beam_idx][i];
        float abs_df = fabs(delta_f);
        float fit_peak = c + a * cos(g_azimuth[beam_idx][i] + p);
        float abs_dif = fabs(delta_f - fit_peak);
        int az_idx = (int)(g_azimuth[beam_idx][i] * 256.0 / two_pi);

        if (abs_dif > g_max_delta_f[beam_idx][az_idx][orbit_step]) {
            g_max_delta_f[beam_idx][az_idx][orbit_step] = abs_dif;
        }
        if (abs_df > g_max_raw_delta_f[beam_idx][az_idx][orbit_step]) {
            g_max_raw_delta_f[beam_idx][az_idx][orbit_step] = abs_df;
        }

/*
        total_azimuth[az_idx]++;
        if (dif > DELTA_F_THRESHOLD) {
            bad_azimuth[az_idx]++;
        }
*/
    }

/*
    for (int az_idx = 0; az_idx < AZIMUTH_STEPS; az_idx++) {
        if (total_azimuth[az_idx] > 0) {
            g_percentage[beam_idx][az_idx][orbit_step] = 100.0 *
                bad_azimuth[az_idx] / total_azimuth[az_idx];
        } else {
            g_percentage[beam_idx][az_idx][orbit_step] = -1.0;
        }
    }
*/

    //--------------------//
    // output diagnostics //
    //--------------------//

    if (fit_base)
    {
        char filename[1024];
        sprintf(filename, "%s.b%1d.s%03d", fit_base, beam_idx + 1,
            orbit_step);
        FILE* ofp = fopen(filename, "w");
        if (ofp == NULL)
        {
            return(0);
        }
        fprintf(ofp, "@ subtitle %cBeam %d, Orbit Step %d%c\n", QUOTE,
            beam_idx + 1, orbit_step, QUOTE);
        fprintf(ofp, "@ xaxis label %cAzimuth Angle (deg)%c\n", QUOTE, QUOTE);
        fprintf(ofp, "@ yaxis label %cBaseband Frequency (kHz)%c\n", QUOTE,
            QUOTE);
        for (int i = 0; i < g_count[beam_idx]; i++)
        {
float delta_f = g_delta_f[beam_idx][i];
float fit_peak = c + a * cos(g_azimuth[beam_idx][i] + p);
float dif = delta_f - fit_peak;
        
            fprintf(ofp, "%g %g %g %g\n", g_azimuth[beam_idx][i] * rtd,
                g_delta_f[beam_idx][i], fit_peak, dif);
        }
        fprintf(ofp, "&\n");

        fclose(ofp);
    }

    //-----------------------------------------------------//
    // calcaulte the worse case errors (bias +- amplitude) //
    //-----------------------------------------------------//

    g_count[beam_idx] = 0;

    return(1);
}

//------------//
// accumulate //
//------------//

int
accumulate(
    int     beam_idx,
    double  azimuth,
    double  delta_f)
{
    int idx = g_count[beam_idx];
    g_azimuth[beam_idx][idx] = azimuth;
    g_delta_f[beam_idx][idx] = delta_f / 1000.0;
    g_count[beam_idx]++;

    return(1);
}
