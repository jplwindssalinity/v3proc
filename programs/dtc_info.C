//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    dtc_info
//
// SYNOPSIS
//    dtc_info <dtc_file> <info_base>
//
// DESCRIPTION
//    Reads in a set of Doppler Tracking Constants and produces some
//    information files.
//
// OPTIONS
//    None.
//
// OPERANDS
//    The following operands are supported:
//        <dtc_file>   The DTC input file.
//        <info_base>  The basename for output information files.
//
// EXAMPLES
//    Examples of command lines are:
//      % dtc_info beam1.dtc beam1_info
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
#include <unistd.h>
#include "Misc.h"
#include "Tracking.h"
#include "Tracking.C"
#include "Qscat.h"

//-----------//
// TEMPLATES //
//-----------//

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

int  basic_info(const char* filename, const char* dtc_file,
         DopplerTracker* doppler_tracker);
int  freq_scan(const char* filename, const char* dtc_file,
         DopplerTracker* doppler_tracker);
int  coef_step(const char* filename, const char* dtc_file,
         DopplerTracker* doppler_tracker);

double  ds_evaluate_amp(double* x, void* ptr);
double  evaluate_amp(int* good, double* coef, double* x);

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "<dtc_file>", "<info_base>", 0 };

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
    if (argc != 3)
    {
        usage(command, usage_array, 1);
        exit(1);
    }

    int arg_idx = 1;
    const char* dtc_file = argv[arg_idx++];
    const char* info_base = argv[arg_idx++];

    //--------------//
    // read the DTC //
    //--------------//

    DopplerTracker doppler_tracker;
    if (! doppler_tracker.ReadBinary(dtc_file))
    {
        fprintf(stderr, "Error reading binary DTC file %s\n", dtc_file);
        exit(1);
    }

    //--------------------------//
    // generate some basic info //
    //--------------------------//

    char filename[1024];
    sprintf(filename, "%s.basic", info_base);
    basic_info(filename, dtc_file, &doppler_tracker);

    sprintf(filename, "%s.freq", info_base);
    freq_scan(filename, dtc_file, &doppler_tracker);

    sprintf(filename, "%s.coef", info_base);
    coef_step(filename, dtc_file, &doppler_tracker);

    return(0);
}

//------------//
// basic_info //
//------------//

int
basic_info(
    const char*      filename,
    const char*      dtc_file,
    DopplerTracker*  doppler_tracker)
{
    FILE* ofp = fopen(filename, "w");
    if (ofp == NULL)
        return(0);

    fprintf(ofp, "DTC File: %s\n", dtc_file);
    unsigned short id = doppler_tracker->GetTableId();
    fprintf(ofp, "Table ID: %d (%x)\n", id, id);

    fclose(ofp);
    return(1);
}

//-----------//
// freq_scan //
//-----------//

int
freq_scan(
    const char*      filename,
    const char*      dtc_file,
    DopplerTracker*  doppler_tracker)
{
    FILE* ofp = fopen(filename, "w");
    if (ofp == NULL)
        return(0);

    for (unsigned short orbit_step = 0; orbit_step < ORBIT_STEPS; orbit_step++)
    {
        for (unsigned short azimuth_step = 0; azimuth_step < 32768;
            azimuth_step += 91)
        {
            float x_value = (float)orbit_step + (float)azimuth_step / 32768.0;
            short dop_dn;
            doppler_tracker->GetCommandedDoppler(orbit_step, azimuth_step,
                0, 0.0, &dop_dn);
            float freq = TX_FREQUENCY_CMD_RESOLUTION * dop_dn;
            fprintf(ofp, "%g %g\n", x_value, freq);
        }
    }

    fclose(ofp);
    return(1);
}

//-----------//
// coef_step //
//-----------//

int
coef_step(
    const char*      filename,
    const char*      dtc_file,
    DopplerTracker*  doppler_tracker)
{
    FILE* ofp = fopen(filename, "w");
    if (ofp == NULL)
        return(0);

    double** terms = (double **)make_array(sizeof(double), 2, ORBIT_STEPS, 3);
    doppler_tracker->GetTerms(terms);

    for (unsigned short orbit_step = 0; orbit_step < ORBIT_STEPS; orbit_step++)
    {
        double amplitude = *(*(terms + orbit_step) + 0);
        double phase = *(*(terms + orbit_step) + 1) * rtd;
        double bias = *(*(terms + orbit_step) + 2);

        fprintf(ofp, "%d %g %g %g\n", orbit_step, amplitude, phase, bias);
    }

    //------------------------------//
    // fit a curve to the amplitude //
    //------------------------------//

    double average_amp = 432000.0;
    double lambda_0 = 1000.0;
    double phi_coef = 500.0;
    double lambda_1 = 50.0;
    double phi_phase = -90.0 * dtr;
    double lambda_2 = 5.0 * dtr;
    double two_phi_coef = 500.0;
    double lambda_3 = 50.0;
    double two_phi_phase = 0.0 * dtr;
    double lambda_4 = 5.0 * dtr;

    int ndim = 5;
    double** p = (double**)make_array(sizeof(double), 2, ndim + 1, ndim);
    if (p == NULL)
        return(0);

    p[0][0] = average_amp;
    p[0][1] = phi_coef;
    p[0][2] = phi_phase;
    p[0][3] = two_phi_coef;
    p[0][4] = two_phi_phase;

    p[1][0] = average_amp + lambda_0;
    p[1][1] = phi_coef;
    p[1][2] = phi_phase;
    p[1][3] = two_phi_coef;
    p[1][4] = two_phi_phase;

    p[2][0] = average_amp;
    p[2][1] = phi_coef + lambda_1;
    p[2][2] = phi_phase;
    p[2][3] = two_phi_coef;
    p[2][4] = two_phi_phase;

    p[3][0] = average_amp;
    p[3][1] = phi_coef;
    p[3][2] = phi_phase + lambda_2;
    p[3][3] = two_phi_coef;
    p[3][4] = two_phi_phase;

    p[4][0] = average_amp;
    p[4][1] = phi_coef;
    p[4][2] = phi_phase;
    p[4][3] = two_phi_coef + lambda_3;
    p[4][4] = two_phi_phase;

    p[5][0] = average_amp;
    p[5][1] = phi_coef;
    p[5][2] = phi_phase;
    p[5][3] = two_phi_coef;
    p[5][4] = two_phi_phase + lambda_4;

    int good[ORBIT_STEPS];
    double coef[ORBIT_STEPS];
    for (int orbit_step = 0; orbit_step < ORBIT_STEPS; orbit_step++)
    {
        good[orbit_step] = 1;
        coef[orbit_step] = *(*(terms + orbit_step) + 0);
    }

    char* ptr[2];
    ptr[0] = (char *)good;
    ptr[1] = (char *)coef;

    downhill_simplex(p, ndim, ndim, 1E-6, ds_evaluate_amp, ptr);

    free_array(terms, 2, ORBIT_STEPS, 3);

    // show fit
fprintf(ofp, "&\n");
    for (int orbit_step = 0; orbit_step < ORBIT_STEPS; orbit_step++)
    {
        double angle = two_pi * (double)orbit_step / (double)ORBIT_STEPS;
        fprintf(ofp, "%d %g\n", orbit_step, p[0][0] +
            p[0][1] * cos(angle + p[0][2]) +
            p[0][3] * cos(2.0 * angle + p[0][4]));
    }

    //--------------------------//
    // fit a curve to the phase //
    //--------------------------//

    int phase_count = 3;
    double phase_init[] = { 0.0, 4.0, 0.0 * dtr };
    double phase_lambda[] = { 1.0, 1.0, 10.0 * dtr };

    double** p = make_p(phase_count, phase_init, phase_lambda);

    int ndim = 5;
    double** p = (double**)make_array(sizeof(double), 2, ndim + 1, ndim);
    if (p == NULL)
        return(0);

    p[0][0] = average_amp;
    p[0][1] = phi_coef;
    p[0][2] = phi_phase;
    p[0][3] = two_phi_coef;
    p[0][4] = two_phi_phase;

    p[1][0] = average_amp + lambda_0;
    p[1][1] = phi_coef;
    p[1][2] = phi_phase;
    p[1][3] = two_phi_coef;
    p[1][4] = two_phi_phase;

    p[2][0] = average_amp;
    p[2][1] = phi_coef + lambda_1;
    p[2][2] = phi_phase;
    p[2][3] = two_phi_coef;
    p[2][4] = two_phi_phase;

    p[3][0] = average_amp;
    p[3][1] = phi_coef;
    p[3][2] = phi_phase + lambda_2;
    p[3][3] = two_phi_coef;
    p[3][4] = two_phi_phase;

    p[4][0] = average_amp;
    p[4][1] = phi_coef;
    p[4][2] = phi_phase;
    p[4][3] = two_phi_coef + lambda_3;
    p[4][4] = two_phi_phase;

    p[5][0] = average_amp;
    p[5][1] = phi_coef;
    p[5][2] = phi_phase;
    p[5][3] = two_phi_coef;
    p[5][4] = two_phi_phase + lambda_4;

    int good[ORBIT_STEPS];
    double coef[ORBIT_STEPS];
    for (int orbit_step = 0; orbit_step < ORBIT_STEPS; orbit_step++)
    {
        good[orbit_step] = 1;
        coef[orbit_step] = *(*(terms + orbit_step) + 0);
    }

    char* ptr[2];
    ptr[0] = (char *)good;
    ptr[1] = (char *)coef;

    downhill_simplex(p, ndim, ndim, 1E-6, ds_evaluate_amp, ptr);

    free_array(terms, 2, ORBIT_STEPS, 3);

    // show fit
fprintf(ofp, "&\n");
    for (int orbit_step = 0; orbit_step < ORBIT_STEPS; orbit_step++)
    {
        double angle = two_pi * (double)orbit_step / (double)ORBIT_STEPS;
        fprintf(ofp, "%d %g\n", orbit_step, p[0][0] +
            p[0][1] * cos(angle + p[0][2]) +
            p[0][3] * cos(2.0 * angle + p[0][4]));
    }

    fclose(ofp);

    return(1);
}

//-----------------//
// ds_evaluate_amp //
//-----------------//

double
ds_evaluate_amp(
    double*  x,
    void*    ptr)
{
    char** ptr2 = (char **)ptr;
    int* good = (int *)ptr2[0];   // 0 = ignore, 1 = use
    double* coef = (double *)ptr2[1];    // the coefficients

    return (evaluate_amp(good, coef, x));
}

//--------------//
// evaluate_amp //
//--------------//

double
evaluate_amp(
    int*     good,
    double*  coef,
    double*  x)
{
    double sse = 0.0;
    for (int orbit_step = 0; orbit_step < ORBIT_STEPS; orbit_step++)
    {
        if (! good[orbit_step])
            continue;

        double angle = two_pi * (double)orbit_step / (double)ORBIT_STEPS;
        double fit = x[0] + x[1] * cos(angle + x[2]) +
            x[3] * cos(2.0 * angle + x[4]);
        double dif = fit - coef[orbit_step];
        sse += (dif * dif);
    }
    return(sse);
}

//-----------------//
// ds_evaluate_amp //
//-----------------//

double
ds_evaluate_amp(
    double*  x,
    void*    ptr)
{
    char** ptr2 = (char **)ptr;
    int* good = (int *)ptr2[0];   // 0 = ignore, 1 = use
    double* coef = (double *)ptr2[1];    // the coefficients

    return (evaluate_amp(good, coef, x));
}

//--------------//
// evaluate_amp //
//--------------//

double
evaluate_amp(
    int*     good,
    double*  coef,
    double*  x)
{
    double sse = 0.0;
    for (int orbit_step = 0; orbit_step < ORBIT_STEPS; orbit_step++)
    {
        if (! good[orbit_step])
            continue;

        double angle = two_pi * (double)orbit_step / (double)ORBIT_STEPS;
        double fit = x[0] + x[1] * cos(angle + x[2]) +
            x[3] * cos(2.0 * angle + x[4]);
        double dif = fit - coef[orbit_step];
        sse += (dif * dif);
    }
    return(sse);
}

//-----------------//
// ds_evaluate_amp //
//-----------------//

double
ds_evaluate_amp(
    double*  x,
    void*    ptr)
{
    char** ptr2 = (char **)ptr;
    int* good = (int *)ptr2[0];   // 0 = ignore, 1 = use
    double* coef = (double *)ptr2[1];    // the coefficients

    return (evaluate_amp(good, coef, x));
}

//--------------//
// evaluate_amp //
//--------------//

double
evaluate_amp(
    int*     good,
    double*  coef,
    double*  x)
{
    double sse = 0.0;
    for (int orbit_step = 0; orbit_step < ORBIT_STEPS; orbit_step++)
    {
        if (! good[orbit_step])
            continue;

        double angle = two_pi * (double)orbit_step / (double)ORBIT_STEPS;
        double fit = x[0] + x[1] * cos(angle + x[2]) +
            x[3] * cos(2.0 * angle + x[4]);
        double dif = fit - coef[orbit_step];
        sse += (dif * dif);
    }
    return(sse);
}

//--------//
// make_p //
//--------//
// contruct the p array

double**
make_p(
    int      p_count,
    double*  p_init,
    double*  p_lambda)
{
    double** p = (double**)make_array(sizeof(double), 2, p_count + 1, p_count);
    if (p == NULL)
        return(NULL);

    for (int i = 0; i < p_count + 1; i++)
    {
        for (int j = 0; j < p_count; j++)
        {
            p[i][j] = p_init[j] + (i == (j - 1) ? p_lambda[j] : 0.0);
        }
    }
        p[0][1] = phi_coef;
        p[0][2] = phi_phase;
        p[0][3] = two_phi_coef;
        p[0][4] = two_phi_phase;

