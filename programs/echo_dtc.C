//==============================================================//
// Copyright (C) 1999-2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    echo_dtc
//
// SYNOPSIS
//    echo_dtc [ -o ] [ -f fit_base ] [ -s scale ] <config_file>
//      <dtc_base> <echo_file...>
//
// DESCRIPTION
//    Reads the echo data files and generates corrected Doppler
//    tracking constants which should center the echo.  It gets
//    a bit more complicated than that.  Once the coefficients
//    are calculated for each orbit step (if the orbit step has
//    enough points) a parametric form is fitted to smooth the
//    coefficients and fill in missing gaps.
//
// OPTIONS
//    [ -o ]           Ocean only.
//    [ -f fit_base ]  Generate fit output with the given filename base.
//    [ -s scale ]     Scale the frequency offset by this factor.
//                       This allows constants to "predict" the future
//                       (assuming the current trend). The default is 1.
//
// OPERANDS
//    The following operands are supported:
//      <config_file>   The simulation configuration file.
//      <dtc_base>      The base name of the output DTC files.
//      <echo_file...>  The echo data files.
//
// EXAMPLES
//    An example of a command line is:
//      % echo_dtc -f fit qscat.cfg newdtc echo1.dat echo2.dat
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
#include "Misc.h"
#include "ConfigList.h"
#include "L1A.h"
#include "ConfigSim.h"
#include "QscatConfig.h"
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
template class List<StringPair>;
template class TrackerBase<unsigned char>;
template class List<OrbitState>;
template class BufferedList<OrbitState>;
template class List<EarthPosition>;
template class TrackerBase<unsigned short>;
template class List<AngleInterval>;

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING  "morf:s:"

#define QUOTE  '"'

#define SIGNAL_ENERGY_THRESHOLD  0
#define ORBIT_STEPS              256
#define SECTOR_COUNT             4
#define MIN_POINTS_PER_SECTOR    20
#define MIN_SECTORS              8

//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

int     process_orbit_step(int beam_idx, int orbit_step,
            const char* fit_base);
int     accumulate(int beam_idx, double azimuth, double meas_spec_peak);
int     plot_fit(const char* base, int beam_idx, int term_idx, double** terms,
            double** p, int term_count);

int     plot_fit_spec(const char* base, int beam_idx, int term_idx,
            double** terms, double* new_term);

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -or ]", "[ -f fit_base ]", "[ -s scale ]",
    "<config_file>", "<dtc_base>", "<echo_file...>", 0 };

int       g_count[NUMBER_OF_QSCAT_BEAMS];
double**  g_terms[NUMBER_OF_QSCAT_BEAMS];
char      g_good[NUMBER_OF_QSCAT_BEAMS][ORBIT_STEPS];
int       g_good_count;
int       g_sector_count[NUMBER_OF_QSCAT_BEAMS][ORBIT_STEPS];
double    g_min_offset[NUMBER_OF_QSCAT_BEAMS][ORBIT_STEPS];
double    g_max_offset[NUMBER_OF_QSCAT_BEAMS][ORBIT_STEPS];

// the following are allocated dynamically
double**  g_azimuth;
double**  g_meas_spec_peak;
off_t***  g_offsets;

int    g_opt_scale = 0;
double g_scale_factor = 1.0;

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
        case 's':
            g_opt_scale = 1;
            g_scale_factor = atof(optarg);
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    if (argc < optind + 3)
        usage(command, usage_array, 1);

    const char* config_file = argv[optind++];
    const char* dtc_base = argv[optind++];
    int start_idx = optind;
    int end_idx = argc;
    int file_count = end_idx - start_idx;

    //---------------------//
    // read in config file //
    //---------------------//

    ConfigList config_list;
    if (! config_list.Read(config_file))
    {
        fprintf(stderr, "%s: error reading configuration file %s\n",
            command, config_file);
        exit(1);
    }
    config_list.StompOrAppend(USE_RGC_KEYWORD, "1");
    config_list.StompOrAppend(USE_DTC_KEYWORD, "1");

    //----------------//
    // create a QSCAT //
    //----------------//

    Qscat qscat;
    if (! ConfigQscat(&qscat, &config_list))
    {
        fprintf(stderr, "%s: error configuring QSCAT\n", command);
        exit(1);
    }

    //---------------------------------------//
    // create and configure Level 1A product //
    //---------------------------------------//

    L1A l1a;
    if (! ConfigL1A(&l1a, &config_list))
    {
        fprintf(stderr, "%s: error configuring Level 1A Product\n", command);
        exit(1);
    }
    int spots_per_frame = l1a.frame.spotsPerFrame;

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
            for (int spot_idx = 0; spot_idx < spots_per_frame; spot_idx++)
            {
                int beam_idx = echo_info.SpotBeamIdx(spot_idx);
                int orbit_step = echo_info.SpotOrbitStep(spot_idx);

                if (echo_info.quality_flag[spot_idx] != EchoInfo::OK)
                    continue;

                if (opt_ocean_only &&
                    echo_info.surface_flag[spot_idx] != EchoInfo::OCEAN)
                {
                    continue;
                }

                if (echo_info.totalSignalEnergy[spot_idx] <
                    SIGNAL_ENERGY_THRESHOLD)
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

    g_meas_spec_peak = (double **)make_array(sizeof(double), 2,
        NUMBER_OF_QSCAT_BEAMS, max_count);
    if (g_meas_spec_peak == NULL)
    {
        fprintf(stderr,
            "%s: error allocating meas_spec_peak array (%d x %d)\n", command,
            NUMBER_OF_QSCAT_BEAMS, max_count);
        exit(1);
    }

    for (int beam_idx = 0; beam_idx < NUMBER_OF_QSCAT_BEAMS; beam_idx++)
    {
        // terms are [0] = amplitude, [1] = phase, [2] = bias
        g_terms[beam_idx] = (double **)make_array(sizeof(double), 2,
            ORBIT_STEPS, 3);
        if (g_terms[beam_idx] == NULL)
        {
            fprintf(stderr, "%s: error allocating DTC term arrays\n", command);
            exit(1);
        }
        g_count[beam_idx] = 0;
        qscat.cds.beamInfo[beam_idx].dopplerTracker.GetTerms(
            g_terms[beam_idx]);
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

                for (int spot_idx = 0; spot_idx < spots_per_frame; spot_idx++)
                {
                    if (echo_info.quality_flag[spot_idx] != EchoInfo::OK)
                        continue;

                    if (opt_ocean_only &&
                        echo_info.surface_flag[spot_idx] != EchoInfo::OCEAN)
                    {
                        continue;
                    }

                    if (echo_info.totalSignalEnergy[spot_idx] <
                        SIGNAL_ENERGY_THRESHOLD)
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
                        echo_info.measSpecPeakFreq[spot_idx]);
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

    free_array(g_offsets, 3, file_count, ORBIT_STEPS, 2);
    free_array(g_azimuth, 2, NUMBER_OF_QSCAT_BEAMS, max_count);
    free_array(g_meas_spec_peak, 2, NUMBER_OF_QSCAT_BEAMS, max_count);

    //-------------------//
    // set up good array //
    //-------------------//
    // this indicates whether to use the coefficient for this orbit step

    g_good_count = 0;
    for (int beam_idx = 0; beam_idx < NUMBER_OF_QSCAT_BEAMS; beam_idx++)
    {
        for (int orbit_step = 0; orbit_step < ORBIT_STEPS; orbit_step++)
        {
            if (g_sector_count[beam_idx][orbit_step] >= MIN_SECTORS)
            {
                g_good[beam_idx][orbit_step] = 1;
                g_good_count++;
            }
            else
                g_good[beam_idx][orbit_step] = 0;
        }
    }

    //---------------------------------------------------------//
    // write out raw terms and good array for later processing //
    //---------------------------------------------------------//

    char filename[1024];
    sprintf(filename, "%s.raw", dtc_base);
    FILE* term_fp = fopen(filename, "w");
    if (term_fp == NULL)
    {
        fprintf(stderr, "%s: error opening raw term file %s\n", command,
            filename);
        exit(1);
    }
    for (int beam_idx = 0; beam_idx < NUMBER_OF_QSCAT_BEAMS; beam_idx++)
    {
        for (int orbit_step = 0; orbit_step < ORBIT_STEPS; orbit_step++)
        {
            fwrite((void *)g_terms[beam_idx][orbit_step], sizeof(double), 3,
                term_fp);
            fwrite((void *)&(g_good[beam_idx][orbit_step]), sizeof(char), 1,
                term_fp);
        }
    }
    fclose(term_fp);

    //---------------------------//
    // write out min/max offsets //
    //---------------------------//

    sprintf(filename, "%s.maxoff", dtc_base);
    FILE* offset_fp = fopen(filename, "w");
    if (offset_fp == NULL)
    {
        fprintf(stderr, "%s: error opening offsets file %s\n", command,
            filename);
        exit(1);
    }
    fprintf(offset_fp, "@ subtitle %c%s%c\n", QUOTE, "Min and Max Offsets",
        QUOTE);
    fprintf(offset_fp, "@ xaxis label %cOrbit Step%c\n", QUOTE, QUOTE);
    fprintf(offset_fp, "@ yaxis label %cBaseband Frequency (kHz)%c\n", QUOTE,
        QUOTE);
    fprintf(offset_fp, "@ world xmin 0\n");
    fprintf(offset_fp, "@ world xmax 256\n");
    fprintf(offset_fp, "@ xaxis tick major 64\n");
    fprintf(offset_fp, "@ xaxis tick minor 16\n");
    fprintf(offset_fp, "@ legend on\n");
    fprintf(offset_fp, "@ legend x1 0.67\n");
    fprintf(offset_fp, "@ legend y1 0.75\n");
    fprintf(offset_fp, "@ legend string 0 %cInner Beam Minimum%c\n", QUOTE,
        QUOTE);
    fprintf(offset_fp, "@ legend string 1 %cInner Beam Maximum%c\n", QUOTE,
        QUOTE);
    fprintf(offset_fp, "@ legend string 2 %cOuter Beam Minimum%c\n", QUOTE,
        QUOTE);
    fprintf(offset_fp, "@ legend string 3 %cOuter Beam Maximum%c\n", QUOTE,
        QUOTE);

    for (int beam_idx = 0; beam_idx < NUMBER_OF_QSCAT_BEAMS; beam_idx++)
    {
        for (int orbit_step = 0; orbit_step < ORBIT_STEPS; orbit_step++)
        {
            if (g_good[beam_idx][orbit_step] &&
                fabs(g_min_offset[beam_idx][orbit_step]) < 80000.0 &&
                fabs(g_max_offset[beam_idx][orbit_step]) < 80000.0)
            {
                fprintf(offset_fp, "%d %g %g\n", orbit_step,
                    g_min_offset[beam_idx][orbit_step] / 1000.0,
                    g_max_offset[beam_idx][orbit_step] / 1000.0);
            }
        }
        if (beam_idx != NUMBER_OF_QSCAT_BEAMS - 1) {
            fprintf(offset_fp, "&\n");
        }
    }
    fclose(offset_fp);

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

    //-------------------------//
    // first, scale the offset //
    //-------------------------//

    if (g_opt_scale)
    {
        for (int i = 0; i < g_count[beam_idx]; i++)
        {
            g_meas_spec_peak[beam_idx][i] *= g_scale_factor;
        }
    }

    //----------------------------//
    // fit a sinusoid to the data //
    //----------------------------//

    double a, p, c;
    sinfit(g_azimuth[beam_idx], g_meas_spec_peak[beam_idx], NULL,
        g_count[beam_idx], &a, &p, &c);

    //-------------------------//
    // new align/overlap check //
    //-------------------------//

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

    //------------------------------------------//
    // subtract the sinusoid from the constants //
    //------------------------------------------//

    double** terms = g_terms[beam_idx];

    double A = *(*(terms + orbit_step) + 0);
    double P = *(*(terms + orbit_step) + 1);
    double C = *(*(terms + orbit_step) + 2);

//    double newA = sqrt(A*A*cos(2.0*P) + a*a*cos(2.0*p) - cos(P+p));  // (1)
//    double newA = sqrt(A*A - 2.0*A*a*cos(P+p) + a*a);  // (2)
    double minus_a = -a;   // change the sign before adding
    double minus_c = -c;
    double newA = sqrt(A*A + 2.0*A*minus_a*cos(P-p) + minus_a*minus_a);  // (3)
    double newC = C + minus_c;
    double y = A*sin(P) + minus_a*sin(p);
    double x = A*cos(P) + minus_a*cos(p);
    double newP = atan2(y, x);

    *(*(terms + orbit_step) + 0) = newA;
    *(*(terms + orbit_step) + 1) = newP;
    *(*(terms + orbit_step) + 2) = newC;

    //-------------------------//
    // check for resonableness //
    //-------------------------//

    if (isnand(newA) || isnand(newC) || isnand(newP))
        g_sector_count[beam_idx][orbit_step] = 0;

    if (fabs(newA) > 600000.0)
        g_sector_count[beam_idx][orbit_step] = 0;

    if (fabs(newC) > 600000.0)
        g_sector_count[beam_idx][orbit_step] = 0;

    //--------------------//
    // output diagnostics //
    //--------------------//

    if (fit_base)
    {
        //---------------------------------------------//
        // determine if orbit step will be used in fit //
        //---------------------------------------------//

        char* used_string = "DISCARDED";
        if (g_sector_count[beam_idx][orbit_step] >= MIN_SECTORS)
        {
            used_string = "USED";
        }

        char filename[1024];
        sprintf(filename, "%s.b%1d.s%03d", fit_base, beam_idx + 1,
            orbit_step);
        FILE* ofp = fopen(filename, "w");
        if (ofp == NULL)
        {
            return(0);
        }
        fprintf(ofp, "@ title %cBeam %d, Orbit Step %d%c\n", QUOTE,
            beam_idx + 1, orbit_step, QUOTE);
        fprintf(ofp, "@ subtitle %c%s%c\n", QUOTE, used_string, QUOTE);
        fprintf(ofp, "@ xaxis label %cAzimuth Angle (deg)%c\n", QUOTE, QUOTE);
        fprintf(ofp, "@ yaxis label %cBaseband Frequency (kHz)%c\n", QUOTE,
            QUOTE);
        for (int i = 0; i < g_count[beam_idx]; i++)
        {
            fprintf(ofp, "%g %g\n", g_azimuth[beam_idx][i] * rtd,
                g_meas_spec_peak[beam_idx][i] / 1000.0);
        }
        fprintf(ofp, "&\n");

        //-----------------------------//
        // estimate the standard error //
        //-----------------------------//

        double sum_sqr_dif = 0.0;
        for (int i = 0; i < g_count[beam_idx]; i++)
        {
            double dif = g_meas_spec_peak[beam_idx][i] -
                (a * cos(g_azimuth[beam_idx][i] + p) + c);
            sum_sqr_dif += dif*dif;
        }
        double std_dev = sqrt(sum_sqr_dif / (double)g_count[beam_idx]);
        double std_err = std_dev / sqrt((double)g_count[beam_idx]);

        fprintf(ofp, "# standard error = %g Hz\n", std_err);

        //--------------------------------------------//
        // report the fit sinusoid and standard error //
        //--------------------------------------------//

        double step = two_pi / 360.0;
        for (double azim = 0; azim < two_pi + step / 2.0; azim += step)
        {
            float value = a * cos(azim + p) + c;
            fprintf(ofp, "%g %g\n", azim * rtd, value / 1000.0);
        }

        fclose(ofp);
    }

    //-----------------------------------------------------//
    // calcaulte the worse case errors (bias +- amplitude) //
    //-----------------------------------------------------//

    double plus = c + a;
    double minus = c - a;
    g_max_offset[beam_idx][orbit_step] = MAX(plus, minus);
    g_min_offset[beam_idx][orbit_step] = MIN(plus, minus);

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
    double  meas_spec_peak)
{
    int idx = g_count[beam_idx];
    g_azimuth[beam_idx][idx] = azimuth;
    g_meas_spec_peak[beam_idx][idx] = meas_spec_peak;
    g_count[beam_idx]++;

    return(1);
}

//----------//
// plot_fit //
//----------//

int
plot_fit(
    const char*  base,
    int          beam_idx,
    int          term_idx,
    double**     terms,
    double**     p,
    int          term_count)
{
    static char* term_string[3] = { "amp", "phase", "bias" };
    static char* title_string[3] = { "Amplitude", "Phase", "Bias" };
    static char* unit_string[3] = { "Frequency (Hz)", "Angle (radians)",
        "Frequency (Hz)" };

    char filename[1024];
    sprintf(filename, "%s.b%1d.%s", base, beam_idx + 1,
        term_string[term_idx]);
    FILE* ofp = fopen(filename, "w");
    if (ofp == NULL)
        return(0);

    fprintf(ofp, "@ subtitle %cBeam %d, %s%c\n", QUOTE,
        beam_idx + 1, title_string[term_idx], QUOTE);
    fprintf(ofp, "@ xaxis label %cOrbit Step%c\n", QUOTE, QUOTE);
    fprintf(ofp, "@ yaxis label %c%s%c\n", QUOTE, unit_string[term_idx],
        QUOTE);

    for (int orbit_step = 0; orbit_step < ORBIT_STEPS; orbit_step++)
    {
        if (g_sector_count[beam_idx][orbit_step] < MIN_SECTORS)
            continue;

        fprintf(ofp, "%d %g\n", orbit_step,
            *(*(terms + orbit_step) + term_idx));
    }
    fprintf(ofp, "&\n");

    for (int orbit_step = 0; orbit_step < ORBIT_STEPS; orbit_step++)
    {
        double angle = two_pi * (double)orbit_step / (double)ORBIT_STEPS;
        double value = p[0][0] + p[0][1] * cos(angle + p[0][2]);
        if (term_count == 5)
            value += p[0][3] * cos(2.0 * angle + p[0][4]);
        fprintf(ofp, "%d %g\n", orbit_step, value);
    }
    fclose(ofp);
    return(1);
}
