//==============================================================//
// Copyright (C) 1998-2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    get_att_otf
//
// SYNOPSIS
//    get_att_otf [ -it ] [ -y yaw ] [ -f type:windfield ]
//      [ -s start:end:step ] <sim_config_file> <output_base>
//      <echo_file...>
//
// DESCRIPTION
//    Reads echo files and estimates the roll, pitch, and yaw
//    of the instrument and the look angle of each antenna beam.
//
// OPTIONS
//    [ -i ]                 Eliminate ice orbit steps.
//    [ -t ]                 Use topography map.
//    [ -y yaw ]             Fix yaw at given value.
//    [ -f type:windfield ]  Use windfield for energy profile
//                             correction.
//    [ -s step ]            The step size for orbit steps.
//
// OPERANDS
//    The following operands are supported:
//      <sim_config_file>  The simulation configuration file.
//      <output_base>      The output base name.
//      <echo_file...>     The input echo files.
//
// EXAMPLES
//    An example of a command line is:
//      % get_att_otf -s 2 qscat.cfg qscat.know qscat.echo1
//            qscat.echo2
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
#include <fcntl.h>
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

#define OPTSTRING    "f:is:ty:"

#define PLOT_OFFSET               40000
#define DIR_STEPS                 36    // for data reduction
#define MIN_POINTS_PER_DIR_STEP   10
#define MIN_DIR_STEP_DATA_POINTS  36

#define QUOTES    '"'

#define LINE_SIZE  1024

#define MAX_SPOTS  1200000
#define MAX_EPHEM  15000

#define SIGNAL_ENERGY_THRESHOLD       0E4

#define MIN_VAR_DATA_COUNT  2

// simplex search parameters
#define LAMBDA      0.05
#define XTOL        0.01
#define PLEX_STEPS  36

//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

int     process_orbit_step(const char* output_base, Spacecraft* spacecraft,
            Qscat* qscat, FbbTable* fbb_table, int orbit_step, FILE* att_fp);
int     ds_optimize(Spacecraft* spacecraft, Qscat* qscat, FbbTable* fbb_table,
            double att[3], double lambda, double xtol);
double  ds_evaluate(double* x, void* ptr);
double  evaluate(Spacecraft* spacecraft, Qscat* qscat, FbbTable* fbb_table,
            double att[3], FILE* ofp = NULL, int or_flag = 0);
int     prune();

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -it ]", "[ -y yaw ]", "[ -f type:windfield ]",
    "[ -s start:end:step ] ", "<sim_config_file>", "<output_base>",
    "<echo_file...>", 0 };

int        g_frame_count = 0;
int        g_spots_per_frame = 0;
EchoInfo*  g_echo_info;

double     g_ref_look[NUMBER_OF_QSCAT_BEAMS];
double     g_att[3];
double     g_time;
double     g_first_time = -1.0;
int        g_opt_windfield = 0;
char*      g_windfield_type = NULL;
char*      g_windfield_file = NULL;
WindField  g_windfield;
GMF        g_gmf;
int        g_step = 1;
int        g_range_opt = 0;
int        g_opt_ice = 0;
int        g_opt_fix_yaw = 0;
double     g_fixed_yaw = 0.0;
int        g_opt_topo = 0;
Topo       g_topo;
Stable     g_stable;

extern int g_max_downhill_simplex_passes;

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

    g_max_downhill_simplex_passes = 50;

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
        case 'i':
            g_opt_ice = 1;
            break;
        case 'f':
            if (sscanf(optarg, "%s:%s", g_windfield_type,
                g_windfield_file) != 2)
            {
                fprintf(stderr, "%s: error parsing windfield %s\n", command,
                    optarg);
                exit(1);
            }
            g_opt_windfield = 1;
            break;
        case 's':
            if (sscanf(optarg, "%d", &g_step) != 1)
            {
                fprintf(stderr, "%s: error parsing step %s\n", command,
                    optarg);
                exit(1);
            }
            break;
        case 't':
            g_opt_topo = 1;
            break;
        case 'y':
            g_opt_fix_yaw = 1;
            if (sscanf(optarg, "%lg", &g_fixed_yaw) != 1)
            {
                fprintf(stderr, "%s: error parsing yaw %s\n", command,
                    optarg);
                exit(1);
            }
            g_fixed_yaw *= dtr;
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    if (argc < optind + 3)
        usage(command, usage_array, 1);

    const char* config_file = argv[optind++];
    const char* output_base = argv[optind++];
    int start_idx = optind;
    int end_idx = argc;

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

    //---------------------------------//
    // create and configure spacecraft //
    //---------------------------------//

    Spacecraft spacecraft;
    if (! ConfigSpacecraft(&spacecraft, &config_list))
    {
        fprintf(stderr, "%s: error configuring spacecraft\n", command);
        exit(1);
    }

    //----------------------------//
    // create and configure QSCAT //
    //----------------------------//

    Qscat qscat;
    if (! ConfigQscat(&qscat, &config_list))
    {
        fprintf(stderr, "%s: error configuring QSCAT\n", command);
        exit(1);
    }

    //-----------------------------//
    // store the reference vectors //
    //-----------------------------//

    for (int beam_idx = 0; beam_idx < NUMBER_OF_QSCAT_BEAMS; beam_idx++)
    {
        g_ref_look[beam_idx] = qscat.cds.xRefLook[beam_idx];
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
    g_spots_per_frame = l1a.frame.spotsPerFrame;

    //--------------------------------//
    // create and configure Fbb table //
    //--------------------------------//

    FbbTable fbb_table;
    if (! fbb_table.Read("/home/svt/data/BYUX/CN61",
        "/home/svt/data/BYUX/CN62"))
    {
        fprintf(stderr, "%s: error reading Fbb tables\n", command);
        exit(1);
    }

    //-----------------------------------------------------//
    // configure topographic map and frequency shift table //
    //-----------------------------------------------------//

    if (g_opt_topo)
    {
        char* topomap_file = config_list.Get(TOPOMAP_FILE_KEYWORD);
        char* stable_file = config_list.Get(STABLE_FILE_KEYWORD);
        if (topomap_file && stable_file)
        {
            if (! g_topo.Read(topomap_file))
            {
                fprintf(stderr, "%s: error reading topographic map %s\n",
                    command, topomap_file);
                exit(1);
            }
            if (! g_stable.Read(stable_file))
            {
                fprintf(stderr, "%s: error reading S Table %s\n", command,
                    stable_file);
                exit(1);
            }
        }
    }

    //-------------------//
    // read in windfield //
    //-------------------//

    if (g_opt_windfield)
    {
        if (! g_windfield.ReadType(g_windfield_file, g_windfield_type))
        {
            fprintf(stderr, "%s: error reading windfield %s of type %s\n",
                command, g_windfield_file, g_windfield_type);
            exit(1);
        }

        if (! ConfigGMF(&g_gmf, &config_list))
        {
            fprintf(stderr, "%s: error configuring GMF\n", command);
            exit(1);
        }
    }

    //----------------------------//
    // set up max frames per step //
    //----------------------------//

    int frame_count[ORBIT_STEPS];
    for (int i = 0; i < ORBIT_STEPS; i++)
    {
        frame_count[i] = 0;
    }

    for (int file_idx = start_idx; file_idx < end_idx; file_idx++)
    {
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

        int orbit_step_count[2][ORBIT_STEPS];
        for (int i = 0; i < 2; i++)
        {
            for (int j = 0; j < ORBIT_STEPS; j++)
            {
                orbit_step_count[i][j] = 0;
            }
        }

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

            int orbit_step = echo_info.SpotOrbitStep(0);
            frame_count[orbit_step]++;
        } while (1);

        //----------------//
        // close the file //
        //----------------//

        close(ifd);
    }

    //--------------//
    // find the max //
    //--------------//

    int max_count = 0;
    for (int orbit_step = 0; orbit_step < ORBIT_STEPS; orbit_step++)
    {
        if (frame_count[orbit_step] > max_count)
        {
            max_count = frame_count[orbit_step];
        }
    }
    printf("Maximum frames per orbit step = %d\n", max_count);
    max_count *= g_step;
    printf("Maximum frames per attitude step = %d\n", max_count);

    //--------------------------//
    // allocate echo info array //
    //--------------------------//

    g_echo_info = (EchoInfo *)malloc(max_count * sizeof(EchoInfo));
    if (g_echo_info == NULL)
    {
        fprintf(stderr, "%s: error allocating %d Echo Info\n", command,
            max_count);
        exit(1);
    }

    //---------------------------//
    // open attitude output file //
    //---------------------------//

    char filename[1024];
    sprintf(filename, "%s.att", output_base);
    FILE* att_fp = fopen(filename, "w");
    if (att_fp == NULL)
    {
        fprintf(stderr,
            "%s: error opening attitude output file %s\n", command,
            filename);
        exit(1);
    }

    //------------------------------------------------//
    // read and process data orbit step by orbit step //
    //------------------------------------------------//

    double sum_time = 0.0;
    int starting_orbit_step = -1;
    g_frame_count = 0;
    for (int file_idx = start_idx; file_idx < end_idx; file_idx++)
    {
        //----------------//
        // open each file //
        //----------------//

        char* echo_file = argv[file_idx];
        printf("  %s\n", echo_file);

        int ifd = open(echo_file, O_RDONLY);
        if (ifd == -1)
        {
            fprintf(stderr, "%s: error opening echo data file %s\n",
                command, echo_file);
            exit(1);
        }

        //-----------//
        // read data //
        //-----------//

        do
        {
            int retval = g_echo_info[g_frame_count].Read(ifd);
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

            int first_orbit_step =
                g_echo_info[g_frame_count].SpotOrbitStep(0);
            int last_orbit_step =
                g_echo_info[g_frame_count].SpotOrbitStep(99);

            if (first_orbit_step < starting_orbit_step || 
                last_orbit_step >= starting_orbit_step + g_step)
            {
                if (starting_orbit_step != -1 && g_frame_count > 0)
                {
                    g_time = sum_time / (double)g_frame_count;
                    printf("Process %d\n", starting_orbit_step);
                    if (g_first_time == -1.0)
                        g_first_time = g_time;
                    process_orbit_step(output_base, &spacecraft, &qscat,
                        &fbb_table, starting_orbit_step, att_fp);
                }
                starting_orbit_step = (int)(last_orbit_step / g_step) * g_step;
                sum_time = 0.0;
                g_frame_count = 0;
            }
            else
            {
                sum_time +=
                    (double)(g_echo_info[g_frame_count].frameTime.GetSec());
                g_frame_count++;
            }
        } while (1);

        //----------------//
        // close the file //
        //----------------//

        close(ifd);
    }
    fclose(att_fp);

    return (0);
}

//--------------------//
// process_orbit_step //
//--------------------//

int
process_orbit_step(
    const char*  output_base,
    Spacecraft*  spacecraft,
    Qscat*       qscat,
    FbbTable*    fbb_table,
    int          orbit_step,
    FILE*        att_fp)
{
    //--------------------------//
    // search for best attitude //
    //--------------------------//

    double att[3];
    att[0] = 0.0;    // roll
    att[1] = 0.0;    // pitch
    if (g_opt_fix_yaw)
        att[2] = g_fixed_yaw;
    else
        att[2] = 0.0;    // yaw
    ds_optimize(spacecraft, qscat, fbb_table, att, LAMBDA * dtr, XTOL * dtr);

    //---------------------------//
    // generate a knowledge plot //
    //---------------------------//

    char filename[1024];
    char title[1024];
    sprintf(filename, "%s.att.%03d.out", output_base, orbit_step);
    sprintf(title, "Orbit Step = %03d", orbit_step);

    FILE* ofp = fopen(filename, "w");
    if (ofp == NULL)
        return(0);

    fprintf(ofp, "@ title %c%s%c\n", QUOTES, title, QUOTES);
    fprintf(ofp, "@ subtitle %cDelta Roll=%.3f, Delta Pitch = %.3f, Delta Yaw = %.3f%c\n", QUOTES,
        att[0] * rtd, att[1] * rtd, att[2] * rtd, QUOTES);

    evaluate(spacecraft, qscat, fbb_table, att, ofp, 0);

    fclose(ofp);

    //---------------------------------------//
    // add roll, pitch, yaw to attitude file //
    //---------------------------------------//

    double delta_time = (g_time - g_first_time) / 60.0;
    fprintf(att_fp, "%g %g %g %g\n", delta_time, att[0] * rtd,
        att[1] * rtd, att[2] * rtd);
    fflush(att_fp);

    return(1);
}

//-------------//
// ds_optimize //
//-------------//

int
ds_optimize(
    Spacecraft*  spacecraft,
    Qscat*       qscat,
    FbbTable*    fbb_table,
    double       att[3],
    double       lambda,
    double       xtol)
{
    int ndim = 3;
    double** p = (double**)make_array(sizeof(double), 2, ndim + 1, ndim);
    if (p == NULL)
        return(0);

    for (int i = 0; i < 3; i++)
    {
        p[0][i] = att[i];
    }

    for (int i = 1; i < ndim + 1; i++)
    {
        for (int j = 0; j < ndim; j++)
        {
            p[i][j] = p[0][j] + (j+1 == i ? lambda : 0.0);
        }
    }

    char* ptr[3];
    ptr[0] = (char *)spacecraft;
    ptr[1] = (char *)qscat;
    ptr[2] = (char *)fbb_table;

    int unknowns_plus_constants = ndim;
    int unknowns = ndim;
    if (g_opt_fix_yaw)
        unknowns = ndim - 1;

    if (! downhill_simplex(p, unknowns, unknowns_plus_constants, 0.0,
        ds_evaluate, ptr, xtol))
    {
        printf("Did not converge.  Trying again.\n");

        // average points to get new starting point
        for (int j = 0; j < ndim; j++)
        {
            p[0][j] /= (float)(ndim + 1);
            for (int i = 1; i < ndim + 1; i++)
            {
                p[0][j] += p[i][j] / (float)(ndim + 1);
            }
        }

        // relambda-ize
        for (int i = 1; i < ndim + 1; i++)
        {
            for (int j = 0; j < ndim; j++)
            {
                p[i][j] = p[0][j] + (j+1 == i ? lambda : 0.0);
            }
        }
        downhill_simplex(p, unknowns, unknowns_plus_constants, 0.0,
            ds_evaluate, ptr, xtol);
    }

    for (int i = 0; i < 3; i++)
    {
        att[i] = p[0][i];
    }

    free_array(p, 2, ndim + 1, ndim);

    return(1);
}

//-------------//
// ds_evaluate //
//-------------//

double
ds_evaluate(
    double*  x,
    void*    ptr)
{
    char** ptr2 = (char**)ptr;
    Spacecraft* spacecraft = (Spacecraft *)ptr2[0];
    Qscat* qscat = (Qscat *)ptr2[1];
    FbbTable* fbb_table = (FbbTable *)ptr2[2];

    return (evaluate(spacecraft, qscat, fbb_table, x));
}

//----------//
// evaluate //
//----------//

double
evaluate(
    Spacecraft*  spacecraft,
    Qscat*       qscat,
    FbbTable*    fbb_table,
    double       att[3],
    FILE*        ofp,
    int          or_flag)
{
    double mse = 0.0;
    float plot_offset_array[] = { 0.0, PLOT_OFFSET };

    for (int frame_idx = 0; frame_idx < g_frame_count; frame_idx++)
    {
        // set the spacecraft attitude
        spacecraft->orbitState.rsat.Set(g_echo_info[frame_idx].gcX,
            g_echo_info[frame_idx].gcY, g_echo_info[frame_idx].gcZ);
        spacecraft->orbitState.vsat.Set(g_echo_info[frame_idx].velX,
            g_echo_info[frame_idx].velY, g_echo_info[frame_idx].velZ);

// this would be used to estimate the attitude biases
/*
        spacecraft->attitude.SetRPY(g_echo_info[frame_idx].roll + att[0],
            g_echo_info[frame_idx].pitch + att[1],
            g_echo_info[frame_idx].yaw + att[2]);
*/
        spacecraft->attitude.SetRPY(att[0], att[1], att[2]);

        for (int spot_idx = 0; spot_idx < g_spots_per_frame; spot_idx++)
        {
            int beam_idx = g_echo_info[frame_idx].SpotBeamIdx(spot_idx);
            int orbit_time = g_echo_info[frame_idx].orbitTicks;
//            int orbit_step = g_echo_info[frame_idx].SpotOrbitStep(spot_idx);

            qscat->cds.currentBeamIdx = beam_idx;
//            Beam* beam = qscat->GetCurrentBeam();
            qscat->cds.orbitTime = orbit_time;
//            Meas::MeasTypeE meas_type = PolToMeasType(beam->polarization);

            float plot_offset = plot_offset_array[beam_idx];

            // only use data to be used
            if (g_echo_info[frame_idx].quality_flag[spot_idx] !=
                EchoInfo::OK && ! or_flag)
            {
                continue;
            }

            // set the antenna
            qscat->sas.antenna.txCenterAzimuthAngle =
                g_echo_info[frame_idx].txCenterAzimuthAngle[spot_idx];
            qscat->TxCenterToGroundImpactAzimuth(spacecraft);

            // set the instrument
            qscat->ses.CmdRxGateDelayEu(
                g_echo_info[frame_idx].rxGateDelay[spot_idx]);
            qscat->ses.CmdTxDopplerEu(
                g_echo_info[frame_idx].txDoppler[spot_idx]);

            //------------------------------//
            // locate slices (if necessary) //
            //------------------------------//

            MeasSpot meas_spot;
            if (g_opt_windfield)
            {
                // create measurements
                if (! qscat->MakeSlices(&meas_spot))
                    return(0);
                qscat->LocateSliceCentroids(spacecraft, &meas_spot);
            }

            //---------------//
            // determine fbb //
            //---------------//

            float fbb;
            if (g_opt_topo)
            {
                fbb = fbb_table->GetFbb(spacecraft, qscat, NULL, NULL, &g_topo,
                    &g_stable);
            }
            else
            {
                fbb = fbb_table->GetFbb(spacecraft, qscat, NULL, NULL, NULL,
                    NULL);
            }

            double dif = fbb -
                g_echo_info[frame_idx].measSpecPeakFreq[spot_idx];
            if (ofp)
            {
                fprintf(ofp, "%g %g %g\n",
                    g_echo_info[frame_idx].txCenterAzimuthAngle[spot_idx] *
                      rtd,
                    g_echo_info[frame_idx].measSpecPeakFreq[spot_idx] +
                      plot_offset,
                    fbb + plot_offset);
            }
            dif = dif * dif;
            mse += dif;
        }
//        if (ofp && beam_idx != NUMBER_OF_QSCAT_BEAMS - 1)
//            fprintf(ofp, "&\n");
    }
    printf("%g %g %g %g\n", att[0] * rtd, att[1] * rtd,
        att[2] * rtd, mse);
    return(mse);
}

//-------//
// prune //
//-------//

int
prune()
{
    int data_count = 0;

/*
    //------------//
    // initialize //
    //------------//

    int count_f[NUMBER_OF_QSCAT_BEAMS][DIR_STEPS];
    double sum_f[NUMBER_OF_QSCAT_BEAMS][DIR_STEPS];
    double sum_az[NUMBER_OF_QSCAT_BEAMS][DIR_STEPS];
    double min_dif[NUMBER_OF_QSCAT_BEAMS][DIR_STEPS];
    int min_idx[NUMBER_OF_QSCAT_BEAMS][DIR_STEPS];
    for (int i = 0; i < NUMBER_OF_QSCAT_BEAMS; i++)
    {
        for (int j = 0; j < DIR_STEPS; j++)
        {
            count_f[i][j] = 0;
            sum_f[i][j] = 0.0;
            sum_az[i][j] = 0.0;
            min_dif[i][j] = 90000.0;
            min_idx[i][j] = -1;
        }
    }
    float dir_step_size = two_pi / DIR_STEPS;

    //-------//
    // prune //
    //-------//

    for (int beam_idx = 0; beam_idx < NUMBER_OF_QSCAT_BEAMS; beam_idx++)
    {
        //-----------------------------------------//
        // for each dir_step_size, calculate means //
        //-----------------------------------------//

        for (int idx = 0; idx < g_count[beam_idx]; idx++)
        {
            double azim = g_tx_center_azimuth[beam_idx][idx];
            int dir_idx = (int)(azim / dir_step_size);
            count_f[beam_idx][dir_idx]++;
            sum_f[beam_idx][dir_idx] += g_meas_spec_peak_freq[beam_idx][idx];
            sum_az[beam_idx][dir_idx] += g_tx_center_azimuth[beam_idx][idx];
        }

        for (int dir_idx = 0; dir_idx < DIR_STEPS; dir_idx++)
        {
            if (count_f[beam_idx][dir_idx])
            {
                sum_f[beam_idx][dir_idx] /= (double)count_f[beam_idx][dir_idx];
                sum_az[beam_idx][dir_idx] /= (double)count_f[beam_idx][dir_idx];
            }
        }

        //------------------------------------------------//
        // for each step, find the point nearest the mean //
        //------------------------------------------------//

        for (int idx = 0; idx < g_count[beam_idx]; idx++)
        {
            double azim = g_tx_center_azimuth[beam_idx][idx];
            int dir_idx = (int)(azim / dir_step_size);

            int idx_1, idx_2;
            if (azim < sum_az[beam_idx][dir_idx])
            {
                idx_1 = (dir_idx + DIR_STEPS - 1) % DIR_STEPS;
                idx_2 = dir_idx;
            }
            else
            {
                idx_1 = dir_idx;
                idx_2 = (dir_idx + 1) % DIR_STEPS;
            }

            if (count_f[beam_idx][idx_1] < MIN_POINTS_PER_DIR_STEP ||
                count_f[beam_idx][idx_2] < MIN_POINTS_PER_DIR_STEP)
            {
                continue;
            }

            double step_dif = sum_az[beam_idx][idx_2] -
                sum_az[beam_idx][idx_1];
            if (step_dif < 0.0)
                step_dif += two_pi;
            double point_dif = azim - sum_az[beam_idx][idx_1];
            if (point_dif < 0.0)
                point_dif += two_pi;

            double fidx = point_dif / step_dif;
            double coef1 = 1.0 - fidx;
            double coef2 = fidx;
            double value = coef1 * sum_f[beam_idx][idx_1] +
                coef2 * sum_f[beam_idx][idx_2];
            double dif = fabs(g_meas_spec_peak_freq[beam_idx][idx] - value);
            if (dif < min_dif[beam_idx][dir_idx])
            {
                min_dif[beam_idx][dir_idx] = dif;
                min_idx[beam_idx][dir_idx] = idx;
            }
        }

        //-------------------------//
        // remove all other points //
        //-------------------------//

        for (int idx = 0; idx < g_count[beam_idx]; idx++)
        {
            g_use_data[beam_idx][idx] = 0;
        }
        for (int dir_idx = 0; dir_idx < DIR_STEPS; dir_idx++)
        {
            if (min_idx[beam_idx][dir_idx] != -1)
            {
                g_use_data[beam_idx][min_idx[beam_idx][dir_idx]] = 1;
                data_count++;
            }
        }
    }
printf("%d data points\n", data_count);
*/
    return(data_count);
}
