//=========================================================//
// Copyright (C) 2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    sws_look
//
// SYNOPSIS
//    sws_look <config_file> <output_base> <echo_file...>
//
// DESCRIPTION
//    Reads echo files and estimates the look angle for each
//    pulse.
//
// OPTIONS
//    None.
//
// OPERANDS
//    The following operands are supported:
//      <config_file>   The configuration file.
//      <output_base>   The output base name.
//      <echo_file...>  The input echo files.
//
// EXAMPLES
//    An example of a command line is:
//      % sws_look sws.cfg sws.look sws.echo1 sws.echo2
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
#include "Tracking.h"
#include "BufferedList.h"
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

#define OPTSTRING    ""

#define MIN_PEAK  -40000
#define MAX_PEAK   40000

#define LAMBDA      0.1
#define XTOL        0.001

/*
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
#define PLEX_STEPS  36
*/

//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

int     ds_optimize(double* look, double lambda, double xtol);
double  ds_evaluate(double* x, void* ptr);
double  evaluate(double* look);

/*
int     process_orbit_step(const char* output_base, Spacecraft* spacecraft,
            Qscat* qscat, FbbTable* fbb_table, int orbit_step, FILE* att_fp);
double  ds_evaluate(double* x, void* ptr);
double  evaluate(Spacecraft* spacecraft, Qscat* qscat, FbbTable* fbb_table,
            double att[3], FILE* ofp = NULL, int or_flag = 0);
int     prune();
*/

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "<config_file>", "<output_base>",
    "<echo_file...>", 0 };

Topo        g_topo;
Stable      g_stable;
EchoInfo    g_echo_info;
Spacecraft  g_spacecraft;
Qscat       g_qscat;
int         g_beam_idx;
int         g_spot_idx;
BYUXTable   g_byux;

double     g_ref_look[NUMBER_OF_QSCAT_BEAMS];

/*
int        g_frame_count = 0;
int        g_spots_per_frame = 0;
EchoInfo*  g_echo_info;

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
int        g_opt_bias = 0;
int        g_opt_minutes = 0;

int        g_land_is_there = 0;

extern int g_max_downhill_simplex_passes;
*/

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

/*
    g_max_downhill_simplex_passes = 50;
*/

    //------------------------//
    // parse the command line //
    //------------------------//

    const char* command = no_path(argv[0]);

    extern int optind;
//    extern char *optarg;
    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
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

    if (! ConfigSpacecraft(&g_spacecraft, &config_list))
    {
        fprintf(stderr, "%s: error configuring spacecraft\n", command);
        exit(1);
    }

    //----------------------------//
    // create and configure QSCAT //
    //----------------------------//

    if (! ConfigQscat(&g_qscat, &config_list))
    {
        fprintf(stderr, "%s: error configuring QSCAT\n", command);
        exit(1);
    }

    //----------------------------------//
    // create and configure BYU X Table //
    //----------------------------------//

    if (! ConfigBYUXTable(&g_byux, &config_list))
    {
        fprintf(stderr, "%s: error configuring BYUXTable\n", command);
        exit(1);
    }

    //-----------------------------//
    // store the reference vectors //
    //-----------------------------//

    for (int beam_idx = 0; beam_idx < NUMBER_OF_QSCAT_BEAMS; beam_idx++)
    {
        g_ref_look[beam_idx] = g_qscat.cds.xRefLook[beam_idx];
    }

    //---------------------------------------//
    // create and configure Level 1A product //
    //---------------------------------------//

/*
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
    char* fbb_in_file = config_list.Get(FBB_INNER_BEAM_FILE_KEYWORD);
    char* fbb_out_file = config_list.Get(FBB_OUTER_BEAM_FILE_KEYWORD);
    if (! fbb_table.Read(fbb_in_file, fbb_out_file)) {
        fprintf(stderr, "%s: error reading Fbb tables (%s, %s)\n", command,
            fbb_in_file, fbb_out_file);
        exit(1);
    }
*/

    //-----------------------------------------------------//
    // configure topographic map and frequency shift table //
    //-----------------------------------------------------//

    char* topomap_file = config_list.Get(TOPOMAP_FILE_KEYWORD);
    char* stable_file = config_list.Get(STABLE_FILE_KEYWORD);
    int stable_mode_id;
    if (! config_list.GetInt(STABLE_MODE_ID_KEYWORD, &stable_mode_id))
    {
        fprintf(stderr, "%s: missing S-Table mode id\n", command);
        exit(1);
    }
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
        g_stable.SetModeId(stable_mode_id);
    }

    //-----------------------//
    // open the output files //
    //-----------------------//

    char filename[1024];
    sprintf(filename, "%s.look.inner", output_base);
    FILE* look_inner_fp = fopen_or_exit(filename, "w", command,
        "inner beam look file", 1);
    sprintf(filename, "%s.look.outer", output_base);
    FILE* look_outer_fp = fopen_or_exit(filename, "w", command,
        "outer beam look file", 1);

    //------------------//
    // for each file... //
    //------------------//

    int count = 0;
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

        //------------------------//
        // process frame by frame //
        //------------------------//

        do
        {
            int retval = g_echo_info.Read(ifd);

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

            //-----------------------//
            // set up the spacecraft //
            //-----------------------//

            g_spacecraft.orbitState.rsat.Set(g_echo_info.gcX, g_echo_info.gcY,
                g_echo_info.gcZ);
            g_spacecraft.orbitState.vsat.Set(g_echo_info.velX,
                g_echo_info.velY, g_echo_info.velZ);
            g_spacecraft.attitude.SetRPY(g_echo_info.roll, g_echo_info.pitch,
                g_echo_info.yaw);

            //----------------------//
            // process echo by echo //
            //----------------------//

            for (g_spot_idx = 0; g_spot_idx < 100; g_spot_idx++)
            {
                // only use good data
                if (g_echo_info.quality_flag[g_spot_idx] != EchoInfo::OK)
                    continue;

                // check for weirdness
                if (g_echo_info.measSpecPeakFreq[g_spot_idx] < MIN_PEAK ||
                    g_echo_info.measSpecPeakFreq[g_spot_idx] > MAX_PEAK)
                {
                    continue;
                }

                // only ocean
                if (g_echo_info.surface_flag[g_spot_idx] != EchoInfo::OCEAN)
                    continue;

                // configure
                g_beam_idx = g_echo_info.SpotBeamIdx(g_spot_idx);
                g_qscat.cds.currentBeamIdx = g_beam_idx;
                int orbit_time = g_echo_info.orbitTicks;
                g_qscat.cds.orbitTime = orbit_time;

                // set the antenna
                g_qscat.sas.antenna.txCenterAzimuthAngle =
                    g_echo_info.txCenterAzimuthAngle[g_spot_idx];
                g_qscat.TxCenterToGroundImpactAzimuth(&g_spacecraft);

                // set the instrument
                g_qscat.ses.CmdRxGateDelayEu(
                    g_echo_info.rxGateDelay[g_spot_idx]);
                g_qscat.ses.CmdTxDopplerEu(g_echo_info.txDoppler[g_spot_idx]);

                // get delta-f
/*
                float geom_delta_f = g_byux.GetDeltaFreq(&g_spacecraft,
                    &g_qscat, &g_topo, &g_stable, NULL);
*/

                // find the best look angle
                double look = g_qscat.cds.xRefLook[g_beam_idx];
                ds_optimize(&look, LAMBDA * dtr, XTOL * dtr);

                FILE* ofp = NULL;
                if (g_beam_idx == 0)
                    ofp = look_inner_fp;
                else
                    ofp = look_outer_fp;
                fprintf(ofp, "%d %g %g\n", count++, look * rtd,
                    g_echo_info.measSpecPeakFreq[g_spot_idx]);
            }
        } while (1);

        //----------------//
        // close the file //
        //----------------//

        close(ifd);
    }

    return (0);
}

//-------------//
// ds_optimize //
//-------------//

int
ds_optimize(
    double*      look,
    double       lambda,
    double       xtol)
{
    int ndim = 1;
    double** p = (double**)make_array(sizeof(double), 2, ndim + 1, ndim);
    if (p == NULL)
        return(0);

    p[0][0] = *look;

    for (int i = 1; i < ndim + 1; i++)
    {
        for (int j = 0; j < ndim; j++)
        {
            p[i][j] = p[0][j] + (j+1 == i ? lambda : 0.0);
        }
    }

    int unknowns_plus_constants = ndim;
    int unknowns = ndim;

    if (! downhill_simplex(p, unknowns, unknowns_plus_constants, 0.0,
        ds_evaluate, NULL, xtol))
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
            ds_evaluate, NULL, xtol);
    }

    *look = p[0][0];

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
    return (evaluate(x));
}

//----------//
// evaluate //
//----------//

double
evaluate(
    double*  look)
{
    // set the look angle
    g_qscat.cds.xRefLook[g_beam_idx] = *look;

    // get delta-f
    float delta_f = g_byux.GetDeltaFreq(&g_spacecraft, &g_qscat, &g_topo,
        &g_stable, NULL);

    double dif = delta_f - g_echo_info.measSpecPeakFreq[g_spot_idx];
    dif *= dif;

//    printf("%g %g\n", *look, dif);
    return(dif);
}
