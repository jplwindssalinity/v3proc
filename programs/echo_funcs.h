//==============================================================//
// Copyright (C) 1998-2003, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#ifndef ECHO_FUNCS_H
#define ECHO_FUNCS_H

static const char rcs_id_echo_funcs_h[] =
    "@(#) $Id$";

#include <stdio.h>
#include <unistd.h>
#include "Qscat.h"
#include "Array.h"
#include "Index.h"
#include "ETime.h"

#define SPOTS_PER_FRAME  100

//==========//
// EchoInfo //
//==========//

class EchoInfo
{
public:

    enum { OK, CAL_OR_LOAD_PULSE, BAD_PEAK, BAD_EPHEMERIS };
    enum { OCEAN, NOT_OCEAN };

    int             Write(int fd);
    void            WriteAscii(FILE* ofp = stdout);
    int             Read(int fd);
    unsigned char   SpotOrbitStep(int spot_idx);
    int             SpotBeamIdx(int spot_idx);

    ETime           frameTime;
    float           gcX;
    float           gcY;
    float           gcZ;
    float           velX;
    float           velY;
    float           velZ;
    float           roll;
    float           pitch;
    float           yaw;
    unsigned int    orbitTicks;
    unsigned char   orbitStep;
    unsigned char   priOfOrbitStepChange;
    float           spinRate;
    unsigned short  idealEncoder[SPOTS_PER_FRAME];
    float           txCenterAzimuthAngle[SPOTS_PER_FRAME];
    float           txDoppler[SPOTS_PER_FRAME];
    float           rxGateDelay[SPOTS_PER_FRAME];
    unsigned char   surface_flag[SPOTS_PER_FRAME];
    unsigned char   quality_flag[SPOTS_PER_FRAME];
    float           totalSignalEnergy[SPOTS_PER_FRAME];
    float           deltaF[SPOTS_PER_FRAME];
    float           measSpecPeakFreq[SPOTS_PER_FRAME];
};

//==============//
// EchoInfoPlus //
//==============//

class EchoInfoPlus : public EchoInfo
{
public:
    int  WritePlus(int fd);
    int  ReadPlus(int fd);

    float  width[SPOTS_PER_FRAME];
    float  slices[SPOTS_PER_FRAME][10];
};

//===========//
// Functions //
//===========//

int     gaussian_fit(Qscat* qscat, double* x, double* y, int points,
            float* peak_slice, float* peak_freq, float* width_freq,
            int use_precalc = 0);
double  gfit_eval(double* x, void* ptr);
double  gfit_eval_precalc(double* x, void* ptr);

float     est_sigma0(int beam_idx, float incidence_angle);
double**  make_p(int p_count, double* p_init, double* p_lambda);
int       free_p(double** p, int p_count);

//-----------------//
// EchoInfo::Write //
//-----------------//

int
EchoInfo::Write(
    int  fd)
{
    int float_size = sizeof(float);
    int int_size = sizeof(int);
    int char_size = sizeof(char);
    int frame_float_size = SPOTS_PER_FRAME * sizeof(float);
    int frame_short_size = SPOTS_PER_FRAME * sizeof(short);
    int frame_char_size = SPOTS_PER_FRAME * sizeof(char);

    if (! frameTime.Write(fd) ||
        write(fd, (void *)&gcX, float_size) != float_size ||
        write(fd, (void *)&gcY, float_size) != float_size ||
        write(fd, (void *)&gcZ, float_size) != float_size ||
        write(fd, (void *)&velX, float_size) != float_size ||
        write(fd, (void *)&velY, float_size) != float_size ||
        write(fd, (void *)&velZ, float_size) != float_size ||
        write(fd, (void *)&roll, float_size) != float_size ||
        write(fd, (void *)&pitch, float_size) != float_size ||
        write(fd, (void *)&yaw, float_size) != float_size ||
        write(fd, (void *)&orbitTicks, int_size) != int_size ||
        write(fd, (void *)&orbitStep, char_size) != char_size ||
        write(fd, (void *)&priOfOrbitStepChange, char_size) !=
          char_size ||
        write(fd, (void *)&spinRate, float_size) != float_size ||
        write(fd, (void *)idealEncoder, frame_short_size) !=
          frame_short_size ||
        write(fd, (void *)txCenterAzimuthAngle, frame_float_size) !=
          frame_float_size ||
        write(fd, (void *)txDoppler, frame_float_size) !=
          frame_float_size ||
        write(fd, (void *)rxGateDelay, frame_float_size) !=
          frame_float_size ||
        write(fd, (void *)surface_flag, frame_char_size) != frame_char_size ||
        write(fd, (void *)quality_flag, frame_char_size) != frame_char_size ||
        write(fd, (void *)totalSignalEnergy, frame_float_size) !=
          frame_float_size ||
        write(fd, (void *)deltaF, frame_float_size) !=
          frame_float_size ||
        write(fd, (void *)measSpecPeakFreq, frame_float_size) !=
          frame_float_size)
    {
        return(0);
    }
    return(1);
}

//----------------------//
// EchoInfo::WriteAscii //
//----------------------//

void
EchoInfo::WriteAscii(
    FILE*  ofp)
{
    frameTime.WriteAscii(ofp);
    fprintf(ofp, "\n");
    fprintf(ofp, "<index> <txDoppler> <rxGateDelay> <deltaF>\n");
    for (int i = 0; i < SPOTS_PER_FRAME; i++) {
        fprintf(ofp, "%d %g %g %g\n", i, txDoppler[i], rxGateDelay[i],
            deltaF[i]);
    }
    return;
}

//----------------//
// EchoInfo::Read //
//----------------//

int
EchoInfo::Read(
    int  fd)
{
    int float_size = sizeof(float);
    int int_size = sizeof(int);
    int char_size = sizeof(char);
    int frame_float_size = SPOTS_PER_FRAME * sizeof(float);
    int frame_short_size = SPOTS_PER_FRAME * sizeof(short);
    int frame_char_size = SPOTS_PER_FRAME * sizeof(char);

    //------------------------------------//
    // use the first read to test for EOF //
    //------------------------------------//

    if (! frameTime.Read(fd))
    {
        off_t current = lseek(fd, 0, SEEK_CUR);
        off_t end_of_file = lseek(fd, 0, SEEK_END);
        if (current == end_of_file)
            return(-1);    // EOF
        else
            return(0);     // error
    }
    if (read(fd, (void *)&gcX, float_size) != float_size ||
        read(fd, (void *)&gcY, float_size) != float_size ||
        read(fd, (void *)&gcZ, float_size) != float_size ||
        read(fd, (void *)&velX, float_size) != float_size ||
        read(fd, (void *)&velY, float_size) != float_size ||
        read(fd, (void *)&velZ, float_size) != float_size ||
        read(fd, (void *)&roll, float_size) != float_size ||
        read(fd, (void *)&pitch, float_size) != float_size ||
        read(fd, (void *)&yaw, float_size) != float_size ||
        read(fd, (void *)&orbitTicks, int_size) != int_size ||
        read(fd, (void *)&orbitStep, char_size) != char_size ||
        read(fd, (void *)&priOfOrbitStepChange, char_size) !=
          char_size ||
        read(fd, (void *)&spinRate, float_size) != float_size ||
        read(fd, (void *)idealEncoder, frame_short_size) !=
          frame_short_size ||
        read(fd, (void *)txCenterAzimuthAngle, frame_float_size) !=
          frame_float_size ||
        read(fd, (void *)txDoppler, frame_float_size) !=
          frame_float_size ||
        read(fd, (void *)rxGateDelay, frame_float_size) !=
          frame_float_size ||
        read(fd, (void *)surface_flag, frame_char_size) != frame_char_size ||
        read(fd, (void *)quality_flag, frame_char_size) != frame_char_size ||
        read(fd, (void *)totalSignalEnergy, frame_float_size) !=
          frame_float_size ||
        read(fd, (void *)deltaF, frame_float_size) !=
          frame_float_size ||
        read(fd, (void *)measSpecPeakFreq, frame_float_size) !=
          frame_float_size)
    {
        return(0);
    }
    return(1);
}

//-------------------------//
// EchoInfoPlus::WritePlus //
//-------------------------//

int
EchoInfoPlus::WritePlus(
    int  fd)
{
    if (! Write(fd))
        return(0);

    int frame_float_size = SPOTS_PER_FRAME * sizeof(float);
    if (write(fd, (void *)width, frame_float_size) != frame_float_size)
        return(0);

    int slices_size = sizeof(float) * 10;
    for (int i = 0; i < SPOTS_PER_FRAME; i++)
    {
        if (write(fd, (void *)slices[i], slices_size) != slices_size)
            return(0);
    }
    return(1);
}

//------------------------//
// EchoInfoPlus::ReadPlus //
//------------------------//

int
EchoInfoPlus::ReadPlus(
    int  fd)
{
    if (! Read(fd))
        return(0);

    int frame_float_size = SPOTS_PER_FRAME * sizeof(float);
    if (read(fd, (void *)width, frame_float_size) != frame_float_size)
        return(0);

    int slices_size = sizeof(float) * 10;
    for (int i = 0; i < SPOTS_PER_FRAME; i++)
    {
        if (read(fd, (void *)slices[i], slices_size) != slices_size)
            return(0);
    }
    return(1);
}

//-------------------------//
// EchoInfo::SpotOrbitStep //
//-------------------------//

unsigned char
EchoInfo::SpotOrbitStep(
    int  spot_idx)
{
    if (priOfOrbitStepChange != 255 && spot_idx < priOfOrbitStepChange - 1)
    {
        if (orbitStep == 0)
            return(ORBIT_STEPS - 1);
        else
            return(orbitStep - 1);
    }
    else
        return(orbitStep);
}

//-----------------------//
// EchoInfo::SpotBeamIdx //
//-----------------------//

int
EchoInfo::SpotBeamIdx(
    int  spot_idx)
{
    return(spot_idx % NUMBER_OF_QSCAT_BEAMS);
}

//--------------//
// gaussian_fit //
//--------------//
// solves for the best amplitude and bias, thus reducing a 4D problem to 2D
// reduction equations courtesy of B. Stiles

#define TOO_WIDE  11.0

int
gaussian_fit(
    Qscat*   qscat,
    double*  x,
    double*  y,
    int      points,
    float*   peak_slice,
    float*   peak_freq,
    float*   width_freq,
    int      use_precalc)
{
    //------------------------------//
    // estimate gaussian parameters //
    //------------------------------//

    int window_width = 4;
    double window_sum = 0.0;
    for (int i = 0; i < window_width; i++)
    {
        window_sum += y[i];
    }
    int max_window_idx = 0;
    double max_window_sum = window_sum;

    for (int i = 0; i < points - window_width; i++)
    {
        window_sum -= y[i];
        window_sum += y[i + window_width];
        if (window_sum > max_window_sum)
        {
            max_window_sum = window_sum;
            max_window_idx = i + 1;    // starting index of window
        }
    }
    double est_center = (double)max_window_idx + (double)window_width / 2.0;
    double est_width = 3.5;
    double lambda_center = 0.2;
    double lambda_width = 0.5;

    //--------------//
    // make p array //
    //--------------//

    int ndim = 2;
    double gauss_init[] = { est_center, est_width };
    double gauss_lambda[] = { lambda_center, lambda_width };
    double** p = make_p(ndim, gauss_init, gauss_lambda);
    if (p == NULL)
        return(0);

    char* ptr[3];
    ptr[0] = (char *)x;
    ptr[1] = (char *)y;
    ptr[2] = (char *)&points;

    if (use_precalc)
    {
        if (! downhill_simplex(p, ndim, ndim, 1E-6, gfit_eval_precalc, ptr))
        {
            free_p(p, ndim);
            return(0);
        }
    }
    else
    {
        if (! downhill_simplex(p, ndim, ndim, 1E-6, gfit_eval, ptr))
        {
            free_p(p, ndim);
            return(0);
        }
    }

    //------------------------//
    // check for "bad" values //
    //------------------------//

    float fslice = p[0][0];
    if (fslice < x[0] || fslice > x[points-1])
    {
        free_p(p, ndim);
        return(0);
    }

    float width = p[0][1];
    if (width > TOO_WIDE)
    {
        free_p(p, ndim);
        return(0);
    }

    //----------------------//
    // transfer information //
    //----------------------//

    int near_slice_idx = (int)(fslice + 0.5);
    float f1, bw, dummy;
    qscat->ses.GetSliceFreqBw(near_slice_idx, &f1, &dummy);
    qscat->ses.GetSliceFreqBw(5, &dummy, &bw);
    *peak_slice = fslice;
    *peak_freq = f1 + bw * (fslice - (float)near_slice_idx + 0.5);
    *width_freq = bw * width;

    free_p(p, ndim);

    return(1);
}

//-----------//
// gfit_eval //
//-----------//
// gaussian fit evaluation function (MSE)

#define MIN_CENTER     -1.0
#define MAX_CENTER     12.0
#define MIN_WIDTH      1.0
#define MAX_WIDTH      12.0

double
gfit_eval(
    double*  c,
    void*    ptr)
{
    char** ptr2 = (char**)ptr;
    double* x = (double *)ptr2[0];
    double* y = (double *)ptr2[1];
    int points = *(int *)ptr2[2];

    //----------------------------------------//
    // this is a hack to restrict the simplex //
    //----------------------------------------//

    if (c[0] < MIN_CENTER)
        c[0] = MIN_CENTER;
    if (c[0] > MAX_CENTER)
        c[0] = MAX_CENTER;

    if (c[1] > MAX_WIDTH)
        c[1] = MAX_WIDTH;
    if (c[1] < MIN_WIDTH)
        c[1] = MIN_WIDTH;

    //---------------------------------//
    // estimate the amplitude and bias //
    //---------------------------------//
    // store computationally intensive terms

    static double hold[12];

    double f_sum = 0.0;
    double f_sqr_sum = 0.0;
    double y_sum = 0.0;
    double fy_sum = 0.0;

    for (int i = 0; i < points; i++)
    {
        double ex = (x[i] - c[0]) / c[1];
        double val = exp(-ex * ex);
        hold[i] = val;
        f_sum += val;
        f_sqr_sum += (val * val);
        y_sum += y[i];
        fy_sum += (y[i] * val);
    }
    double n = (double)points;
    double denom = (n * f_sqr_sum - f_sum * f_sum);
    double amp = (n * fy_sum - y_sum * f_sum) / denom;
    double bias = (y_sum * f_sqr_sum - f_sum * fy_sum) / denom;

    //---------------------//
    // the real evaluation //
    //---------------------//

    double sum_dif = 0.0;
    for (int i = 0; i < points; i++)
    {
        double val = amp * hold[i] + bias;
        double dif = val - y[i];
        sum_dif += ((dif * dif) / (double)points);
    }
    return(sum_dif);
}

//-------------------//
// gfit_eval_precalc //
//-------------------//
// just like gfit_eval, but uses a look up table for exp

double
gfit_eval_precalc(
    double*  c,
    void*    ptr)
{
    char** ptr2 = (char**)ptr;
    double* x = (double *)ptr2[0];
    double* y = (double *)ptr2[1];
    int points = *(int *)ptr2[2];

    //----------------//
    // do the precalc //
    //----------------//

    static int precalc_done = 0;
    static double precalc_exp[1100];
    if (! precalc_done)
    {
        for (int idx = 0; idx < 1100; idx++)
        {
            double abs_ex = (double)idx / 100.0;
            precalc_exp[idx] = exp(-abs_ex * abs_ex);
        }
        precalc_done = 1;
    }

    //----------------------------------------//
    // this is a hack to restrict the simplex //
    //----------------------------------------//

    if (c[0] < MIN_CENTER)
        c[0] = MIN_CENTER;
    if (c[0] > MAX_CENTER)
        c[0] = MAX_CENTER;

    if (c[1] > MAX_WIDTH)
        c[1] = MAX_WIDTH;
    if (c[1] < MIN_WIDTH)
        c[1] = MIN_WIDTH;

    //---------------------------------//
    // estimate the amplitude and bias //
    //---------------------------------//
    // store computationally intensive terms

    static double hold[12];

    double f_sum = 0.0;
    double f_sqr_sum = 0.0;
    double y_sum = 0.0;
    double fy_sum = 0.0;

    for (int i = 0; i < points; i++)
    {
        double ex = (x[i] - c[0]) / c[1];
        int idx = (int)(fabs(ex) * 100.0 + 0.5);
        if (idx < 0)
            idx = 0;
        if (idx >= 1100)
            idx = 1100 - 1;
        double val = precalc_exp[idx];
        hold[i] = val;
        f_sum += val;
        f_sqr_sum += (val * val);
        y_sum += y[i];
        fy_sum += (y[i] * val);
    }
    double n = (double)points;
    double denom = (n * f_sqr_sum - f_sum * f_sum);
    double amp = (n * fy_sum - y_sum * f_sum) / denom;
    double bias = (y_sum * f_sqr_sum - f_sum * fy_sum) / denom;

    //---------------------//
    // the real evaluation //
    //---------------------//

    double sum_dif = 0.0;
    for (int i = 0; i < points; i++)
    {
        double val = amp * hold[i] + bias;
        double dif = val - y[i];
        sum_dif += ((dif * dif) / (double)points);
    }
    return(sum_dif);
}

//------------//
// est_sigma0 //
//------------//

float
est_sigma0(
    int    beam_idx,
    float  incidence_angle)
{
    float s0_table[2][11] = {
      { 0.0255338, 0.0220457, 0.0193465, 0.0171857, 0.0153964, 0.0138646,
        0.0125171, 0.0113071, 0.010215, 0.00923337, 0.00837293 },
      { 0.0147139, 0.01166, 0.00936597, 0.00758692, 0.00617038, 0.00502344,
        0.00409095, 0.00333365, 0.00271822, 0.00221762, 0.00181041 }
    };

    Index index;
    index.SpecifyCenters(40.0 * dtr, 60.0 * dtr, 11);

    int idx[2];
    float coef[2];
    if (! index.GetLinearCoefsStrict(incidence_angle, idx, coef))
    {
        fprintf(stderr, "Incidence angle out of range\n");
        return(0);
    }

    float sigma0 = s0_table[beam_idx][idx[0]] * coef[0] +
        s0_table[beam_idx][idx[1]] * coef[1];
    return(sigma0);
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
            p[i][j] = p_init[j] + (i == j ? p_lambda[j] : 0.0);
        }
    }

    return(p);
}

//--------//
// free_p //
//--------//
// free the p array

int
free_p(
    double** p,
    int      p_count)
{
    free_array(p, 2, p_count + 1, p_count);
    return(1);
}

#endif
