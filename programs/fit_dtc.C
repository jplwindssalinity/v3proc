//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    fit_dtc
//
// SYNOPSIS
//    fit_dtc [ -n ] <raw_dtc_file> <dtc_base>
//
// DESCRIPTION
//    Reads the raw DTC file and generates a fit to the data.
//
// OPTIONS
//    [ -n ]  Natural.  No fit.
//
// OPERANDS
//    The following operands are supported:
//      <raw_dtc_file>  The input raw DTC file.
//      <dtc_base>      The base name of the output DTC files.
//
// EXAMPLES
//    An example of a command line is:
//      % fit_dtc dtc.raw newdtc
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
#include <math.h>
#include "Misc.h"
#include "Array.h"
#include "Tracking.h"
#include "Tracking.C"
/*
#include <fcntl.h>
#include "ConfigList.h"
#include "L1A.h"
#include "ConfigSim.h"
#include "QscatConfig.h"
#include "InstrumentGeom.h"
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "AngleInterval.h"
#include "echo_funcs.h"
*/

//-----------//
// TEMPLATES //
//-----------//

/*
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
*/

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING  "n"
#define QUOTE      '"'

#define ORBIT_STEPS            256
#define NUMBER_OF_QSCAT_BEAMS  2

//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

int     fit_terms_plus(const char* fit_base, int beam_idx, double** terms);
int     plot_fit_spec(const char* base, int beam_idx, int term_idx,
            double** terms, double* new_term);

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -n ]", "<raw_dtc_file>", "<dtc_base>", 0 };

double**  g_terms[NUMBER_OF_QSCAT_BEAMS];
char      g_good[NUMBER_OF_QSCAT_BEAMS][ORBIT_STEPS];

/*
// the following are allocated dynamically
double**  g_azimuth;
double**  g_meas_spec_peak;
off_t***  g_offsets;
*/

//--------------//
// MAIN PROGRAM //
//--------------//

int
main(
    int    argc,
    char*  argv[])
{
    int opt_natural = 0;

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
        case 'n':
            opt_natural = 1;
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    if (argc < optind + 2)
        usage(command, usage_array, 1);

    const char* raw_dtc_file = argv[optind++];
    const char* dtc_base = argv[optind++];

    //--------------------//
    // allocate for terms //
    //--------------------//

    for (int beam_idx = 0; beam_idx < NUMBER_OF_QSCAT_BEAMS; beam_idx++)
    {
        g_terms[beam_idx] = (double **)make_array(sizeof(double), 2,
            ORBIT_STEPS, 3);
        if (g_terms[beam_idx] == NULL)
        {
            fprintf(stderr, "%s: error allocating DTC term arrays\n", command);
            exit(1);
        }
    }

    //----------------------//
    // read in raw DTC file //
    //----------------------//

    FILE* term_fp = fopen(raw_dtc_file, "r");
    if (term_fp == NULL)
    {
        fprintf(stderr, "%s: error opening raw term file %s\n", command,
            raw_dtc_file);
        exit(1);
    }
    for (int beam_idx = 0; beam_idx < NUMBER_OF_QSCAT_BEAMS; beam_idx++)
    {
        for (int orbit_step = 0; orbit_step < ORBIT_STEPS; orbit_step++)
        {
            fread((void *)g_terms[beam_idx][orbit_step], sizeof(double), 3,
                term_fp);
            fread((void *)&(g_good[beam_idx][orbit_step]), sizeof(char), 1,
                term_fp);
        }
    }
    fclose(term_fp);

    //-----//
    // fit //
    //-----//

if (! opt_natural)
{
    for (int beam_idx = 0; beam_idx < NUMBER_OF_QSCAT_BEAMS; beam_idx++)
    {
        fit_terms_plus(dtc_base, beam_idx, g_terms[beam_idx]);
    }
}

    //-------------//
    // set Doppler //
    //-------------//

    DopplerTracker doppler_tracker;
    if (! doppler_tracker.Allocate(ORBIT_STEPS))
    {
        fprintf(stderr, "%s: error allocating Doppler tracker\n", command);
        exit(1);
    }

    for (int beam_idx = 0; beam_idx < NUMBER_OF_QSCAT_BEAMS; beam_idx++)
    {
        doppler_tracker.SetTerms(g_terms[beam_idx]);
        char filename[1024];
        sprintf(filename, "%s.%d", dtc_base, beam_idx + 1);
        if (! doppler_tracker.WriteBinary(filename))
        {
            fprintf(stderr, "%s: error writing DTC file %s\n", command,
                filename);
            exit(1);
        }
    }

    for (int beam_idx = 0; beam_idx < NUMBER_OF_QSCAT_BEAMS; beam_idx++)
    {
        free_array(g_terms[beam_idx], 2, ORBIT_STEPS, 3);
    }

    return(0);
}

//----------------//
// fit_terms_plus //
//----------------//
// uses spectral analysis

#define TERM_COUNT  32

int
fit_terms_plus(
    const char*  fit_base,
    int          beam_idx,
    double**     terms)
{
    //-------------------//
    // create data array //
    //-------------------//

    int good_count = 0;
    for (int i = 0; i < ORBIT_STEPS; i++)
    {
        if (g_good[beam_idx][i])
            good_count++;
    }

    double* azimuth = (double *)make_array(sizeof(double), 1, good_count);
    double* data = (double *)make_array(sizeof(double), 1, good_count);
//    double* residual = (double *)make_array(sizeof(double), 1, good_count);

    int max_term[3] = { 4, 3, 3 };
//    double threshold[3] = { 500.0, 0.01, 500.0 };    // amp, phase, bias
    double threshold[3] = { 0.0, 0.0, 0.0 };    // amp, phase, bias
    double new_coefs[3][ORBIT_STEPS];

    //----------------------//
    // for each coefficient //
    //----------------------//

    int coef_translate[3] = { 0, 2, 1};    // want phase the last index
    for (int coef_access_idx = 0; coef_access_idx < 3; coef_access_idx++)
    {
        int coef_idx = coef_translate[coef_access_idx];

        //------------------//
        // fill in the data //
        //------------------//

        int sample_count = 0;
        for (int orbit_step = 0; orbit_step < ORBIT_STEPS; orbit_step++)
        {
            if (g_good[beam_idx][orbit_step])
            {
                azimuth[sample_count] = two_pi * (double)orbit_step /
                    (double)ORBIT_STEPS;
                data[sample_count] = *(*(terms + orbit_step) + coef_idx);

/*
//-----------------------//
// calculate phase again //
//-----------------------//

if (coef_idx == 1)    // phase
{
    double orig_bias = *(*(terms + orbit_step) + 2);    // original bias
    double fit_bias = new_coefs[2][orbit_step];       // fit bias
    double orig_amp = *(*(terms + orbit_step) + 0);    // original a
    double fit_amp = new_coefs[0][orbit_step];       // fit a
    double orig_phase = *(*(terms + orbit_step) + 1);    // original p
    data[sample_count] = acos((orig_bias - fit_bias +
        orig_amp * cos(orig_phase)) / fit_amp);
}
*/
                sample_count++;
            }
        }

        //---------------------------//
        // solve low frequency terms //
        //---------------------------//

        double amplitude[TERM_COUNT], phase[TERM_COUNT];
        for (int i = 0; i < TERM_COUNT; i++)
        {
            amplitude[i] = 0.0;
            phase[i] = 0.0;
        }

        specfit(azimuth, data, NULL, sample_count, 0, max_term[coef_idx],
            amplitude, phase);

/*
        //--------------------------//
        // calculate residual error //
        //--------------------------//

        for (int i = 0; i < sample_count; i++)
        {
            residual[i] = data[i];
            for (int j = 0; j < term_count[coef_idx]; j++)
            {
                residual[i] -= (amplitude[j] * cos((double)j * azimuth[i] +
                    phase[j]));
            }
        }

        //--------------------//
        // solve for residual //
        //--------------------//

        double res_amplitude[TERM_COUNT], res_phase[TERM_COUNT];
        specfit(azimuth, data, NULL, sample_count, max_term[coef_idx],
            TERM_COUNT1, res_amplitude, res_phase);

        //---------------//
        // combine terms //
        //---------------//

        for (int term_idx = 0; term_idx < TERM_COUNT; term_idx++)
        {
            amplitude[term_idx] += 
        }
*/

        //-----------------//
        // threshold terms //
        //-----------------//

        for (int term_idx = 0; term_idx < TERM_COUNT; term_idx++)
        {
            if (amplitude[term_idx] < threshold[coef_idx])
                amplitude[term_idx] = 0.0;
        }

        //---------------//
        // calculate DTC //
        //---------------//

        for (int orbit_step = 0; orbit_step < ORBIT_STEPS; orbit_step++)
        {
            double angle = two_pi * (double)orbit_step / (double)ORBIT_STEPS;

            new_coefs[coef_idx][orbit_step] = 0.0;
            for (int i = 0; i < TERM_COUNT; i++)
            {
                new_coefs[coef_idx][orbit_step] += amplitude[i] *
                    cos((double)i * angle + phase[i]);
            }
        }

        //--------------------//
        // generate fit plots //
        //--------------------//

        if (fit_base)
        {
            plot_fit_spec(fit_base, beam_idx, coef_idx, terms,
                new_coefs[coef_idx]);
        }
    }

    //---------------//
    // plug in terms //
    //---------------//

    for (int coef_idx = 0; coef_idx < 3; coef_idx++)
    {
        for (int orbit_step = 0; orbit_step < ORBIT_STEPS; orbit_step++)
        {
            *(*(terms + orbit_step) + coef_idx) =
                new_coefs[coef_idx][orbit_step];
        }
    }

    free_array(data, 1, good_count);
    free_array(azimuth, 1, good_count);

    return(1);
}

//---------------//
// plot_fit_spec //
//---------------//

int
plot_fit_spec(
    const char*  base,
    int          beam_idx,
    int          term_idx,
    double**     terms,
    double*      new_term)
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
        if (! g_good[beam_idx][orbit_step])
            continue;

        fprintf(ofp, "%d %g\n", orbit_step,
            *(*(terms + orbit_step) + term_idx));
    }
    fprintf(ofp, "&\n");

    for (int orbit_step = 0; orbit_step < ORBIT_STEPS; orbit_step++)
    {
        fprintf(ofp, "%d %g\n", orbit_step, new_term[orbit_step]);
    }
    fclose(ofp);
    return(1);
}
