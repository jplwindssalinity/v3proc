//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    fit_dtc
//
// SYNOPSIS
//    fit_dtc [ -fi ] [ -h terms ] <raw_dtc_file> <dtc_base>
//
// DESCRIPTION
//    Reads the raw DTC file and generates a fit to the data.
//
// OPTIONS
//    [ -f ]        Filter out outliers.
//    [ -h terms ]  Use more terms (higher order).
//    [ -i ]        Filter out ice.
//
// OPERANDS
//    The following operands are supported:
//      <raw_dtc_file>  The input raw DTC file.
//      <dtc_base>      The base name of the output DTC files.
//
// EXAMPLES
//    An example of a command line is:
//      % fit_dtc -f dtc.raw newdtc
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
#include <stdlib.h>
#include <math.h>
#include "Misc.h"
#include "Array.h"
#include "Tracking.h"
#include "Tracking.C"

//-----------//
// TEMPLATES //
//-----------//

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING  "fh:i"
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

const char* usage_array[] = { "[ -fhi ]", "<raw_dtc_file>", "<dtc_base>", 0 };

double**  g_terms[NUMBER_OF_QSCAT_BEAMS];
char      g_good[NUMBER_OF_QSCAT_BEAMS][ORBIT_STEPS];
char      g_good_too[ORBIT_STEPS];
int       g_high_terms;

int       g_opt_filter = 0;
int       g_opt_ice = 0;
int       g_opt_high = 0;

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
    extern int optind;
    extern char *optarg;
    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
        case 'f':
            g_opt_filter = 1;
            printf("3-Sigma filtering\n");
            break;
        case 'h':
            g_opt_high = 1;
            g_high_terms = atoi(optarg);
            printf("High order fit\n");
            break;
        case 'i':
            g_opt_ice = 1;
            printf("Eliminating ice\n");
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

    for (int beam_idx = 0; beam_idx < NUMBER_OF_QSCAT_BEAMS; beam_idx++)
    {
        fit_terms_plus(dtc_base, beam_idx, g_terms[beam_idx]);
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

    //---------------------------//
    // initialize good_too array //
    //---------------------------//

    for (int i = 0; i < ORBIT_STEPS; i++)
    {
        g_good_too[i] = 1;
        if (g_opt_ice)
        {
            if ((i > 46 && i < 82) || (i > 167 && i < 217))
                g_good_too[i] = 0;
        }
    }

    double* azimuth = (double *)make_array(sizeof(double), 1, good_count);
    double* data = (double *)make_array(sizeof(double), 1, good_count);

    int high_order_max_term[3];    // gets filled in later
//    double high_order_threshold[3] = { 100.0, 0.0002, 100.0 };
    double high_order_threshold[3] = { 0.0, 0.0, 0.0 };

    int low_order_max_term[3] = { 3, 2, 2 };    // amp, phase, bias
    double low_order_threshold[3] = { 0.0, 0.0, 0.0 };

    int* max_term;
    double* threshold;
    if (g_opt_high)
    {
        for (int i = 0; i < 3; i++)
            high_order_max_term[i] = g_high_terms;

        max_term = high_order_max_term;
        threshold = high_order_threshold;
    }
    else
    {
        max_term = low_order_max_term;
        threshold = low_order_threshold;
    }
    double new_coefs[3][ORBIT_STEPS];

    //----------------------//
    // for each coefficient //
    //----------------------//

    for (int coef_idx = 0; coef_idx < 3; coef_idx++)
    {
        //------------------//
        // fill in the data //
        //------------------//

        int sample_count = 0;
        for (int orbit_step = 0; orbit_step < ORBIT_STEPS; orbit_step++)
        {
            if (! g_good[beam_idx][orbit_step] || ! g_good_too[orbit_step])
                continue;

            azimuth[sample_count] = two_pi * (double)orbit_step /
                (double)ORBIT_STEPS;
            data[sample_count] = *(*(terms + orbit_step) + coef_idx);
            sample_count++;
        }

        //----------------------//
        // start filtering loop //
        //----------------------//

        double amplitude[TERM_COUNT], phase[TERM_COUNT];
        do
        {
            //---------------------------//
            // solve low frequency terms //
            //---------------------------//

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

            if (! g_opt_filter)
                break;    // one pass is enough

            //-----------------------------------//
            // calculate threshold for next pass //
            //-----------------------------------//

            int sample_count = 0;
            double std_dev = 0.0;
            for (int orbit_step = 0; orbit_step < ORBIT_STEPS; orbit_step++)
            {
                if (! g_good[beam_idx][orbit_step] ||
                    ! g_good_too[orbit_step])
                {
                    continue;
                }

                double angle = two_pi * (double)orbit_step /
                    (double)ORBIT_STEPS;
                double fit_value = 0.0;
                for (int term_idx = 0; term_idx < TERM_COUNT; term_idx++)
                {
                    fit_value += amplitude[term_idx] *
                        cos((double)term_idx * angle + phase[term_idx]);
                }
                double dif = fit_value - *(*(terms + orbit_step) + coef_idx);
                std_dev += (dif*dif);
                sample_count++;
            }
            std_dev /= (double)(sample_count - 1);
            std_dev = sqrt(std_dev);

            //-------------//
            // filter data //
            //-------------//

            int toss_count = 0;
            double thresh = 5.0 * std_dev;
            sample_count = 0;
            for (int orbit_step = 0; orbit_step < ORBIT_STEPS; orbit_step++)
            {
                if (! g_good[beam_idx][orbit_step] ||
                    ! g_good_too[orbit_step])
                {
                    continue;
                }

                double angle = two_pi * (double)orbit_step /
                    (double)ORBIT_STEPS;
                double fit_value = 0.0;
                for (int term_idx = 0; term_idx < TERM_COUNT; term_idx++)
                {
                    fit_value += amplitude[term_idx] *
                        cos((double)term_idx * angle + phase[term_idx]);
                }
                double dif = fit_value - *(*(terms + orbit_step) + coef_idx);
                if (fabs(dif) > thresh)
                {
                    g_good_too[orbit_step] = 0;
                    toss_count++;
                    continue;
                }

                azimuth[sample_count] = angle;
                data[sample_count] = *(*(terms + orbit_step) + coef_idx);
                sample_count++;
            }

            if (toss_count == 0.0)
                break;

        } while (1);

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

    if (g_opt_filter || g_opt_ice)
    {
        fprintf(ofp, "&\n");
        for (int orbit_step = 0; orbit_step < ORBIT_STEPS; orbit_step++)
        {
            if (! g_good[beam_idx][orbit_step] || ! g_good_too[orbit_step])
                continue;

            fprintf(ofp, "%d %g\n", orbit_step,
                *(*(terms + orbit_step) + term_idx));
        }
    }
    fclose(ofp);
    return(1);
}
