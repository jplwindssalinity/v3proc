//==============================================================//
// Copyright (C) 1998-1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#ifndef ECHO_FUNCS_H
#define ECHO_FUNCS_H

static const char rcs_id_echo_funcs_h[] =
    "@(#) $Id$";

#include <stdio.h>
#include "Qscat.h"
#include "Array.h"
#include "Index.h"

#define SPOTS_PER_FRAME  100

//==========//
// EchoInfo //
//==========//

class EchoInfo
{
public:

    enum { OK, CAL_OR_LOAD_PULSE, LAND, BAD_PEAK };

    int            Write(int fd);
    int            Read(int fd);
    unsigned char  SpotOrbitStep(int spot_idx);

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
    unsigned char   beamIdx[SPOTS_PER_FRAME];
    unsigned short  idealEncoder[SPOTS_PER_FRAME];
    double          txCenterAzimuthAngle[SPOTS_PER_FRAME];
    float           txDoppler[SPOTS_PER_FRAME];
    float           rxGateDelay[SPOTS_PER_FRAME];
    unsigned char   flag[SPOTS_PER_FRAME];
    double          totalSignalEnergy[SPOTS_PER_FRAME];
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
            float* peak_slice, float* peak_freq, float* width_freq);
double  gfit_eval(double* x, void* ptr);

int     gaussian_fit2(Qscat* qscat, double* x, double* y, int points,
            float* peak_slice, float* peak_freq, float* width_freq);
double  gfit_eval2(double* x, void* ptr);

float   est_sigma0(int beam_idx, float incidence_angle);

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
    int frame_double_size = SPOTS_PER_FRAME * sizeof(double);
    int frame_float_size = SPOTS_PER_FRAME * sizeof(float);
    int frame_short_size = SPOTS_PER_FRAME * sizeof(short);
    int frame_char_size = SPOTS_PER_FRAME * sizeof(char);

    if ( write(fd, (void *)&gcX, float_size) != float_size ||
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
        write(fd, (void *)beamIdx, frame_char_size) != frame_char_size ||
        write(fd, (void *)idealEncoder, frame_short_size) !=
          frame_short_size ||
        write(fd, (void *)txCenterAzimuthAngle, frame_double_size) !=
          frame_double_size ||
        write(fd, (void *)txDoppler, frame_float_size) !=
          frame_float_size ||
        write(fd, (void *)rxGateDelay, frame_float_size) !=
          frame_float_size ||
        write(fd, (void *)flag, frame_char_size) != frame_char_size ||
        write(fd, (void *)totalSignalEnergy, frame_double_size) !=
          frame_double_size ||
        write(fd, (void *)measSpecPeakFreq, frame_float_size) !=
          frame_float_size)
    {
        return(0);
    }
    return(1);
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
    int frame_double_size = SPOTS_PER_FRAME * sizeof(double);
    int frame_float_size = SPOTS_PER_FRAME * sizeof(float);
    int frame_short_size = SPOTS_PER_FRAME * sizeof(short);
    int frame_char_size = SPOTS_PER_FRAME * sizeof(char);

    //------------------------------------//
    // use the first read to test for EOF //
    //------------------------------------//

    int retval = read(fd, (void *)&gcX, float_size);
    if (retval != float_size)
    {
        switch (retval)
        {
        case 0:    // EOF
            return(-1);
            break;
        default:   // error
            return(0);
            break;
        }
    }
    if (read(fd, (void *)&gcY, float_size) != float_size ||
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
        read(fd, (void *)beamIdx, frame_char_size) != frame_char_size ||
        read(fd, (void *)idealEncoder, frame_short_size) !=
          frame_short_size ||
        read(fd, (void *)txCenterAzimuthAngle, frame_double_size) !=
          frame_double_size ||
        read(fd, (void *)txDoppler, frame_float_size) !=
          frame_float_size ||
        read(fd, (void *)rxGateDelay, frame_float_size) !=
          frame_float_size ||
        read(fd, (void *)flag, frame_char_size) != frame_char_size ||
        read(fd, (void *)totalSignalEnergy, frame_double_size) !=
          frame_double_size ||
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

//--------------//
// gaussian_fit //
//--------------//

int
gaussian_fit(
    Qscat*   qscat,
    double*  x,
    double*  y,
    int      points,
    float*   peak_slice,
    float*   peak_freq,
    float*   width_freq)
{
    //----------------//
    // allocate array //
    //----------------//

    int ndim = 4;
    double** p = (double**)make_array(sizeof(double), 2, ndim + 1, ndim);
    if (p == NULL)
        return(0);

    //--------------------//
    // initialize simplex //
    //--------------------//

    int max_idx = 0;
    for (int i = 0; i < points; i++)
    {
        if (y[i] > y[max_idx])
            max_idx = i;
    }

    double amp = y[max_idx];
    double amp_lambda = amp * 0.5;
    double center = (double)max_idx;
    double center_lambda = 0.2;
    double width = 3.5;
    double width_lambda = 0.5;
    double bias = 0.0;
    double bias_lambda = amp_lambda;

    p[0][0] = amp;
    p[0][1] = center;
    p[0][2] = width;
    p[0][3] = bias;

    p[1][0] = amp + amp_lambda;
    p[1][1] = center;
    p[1][2] = width;
    p[1][3] = bias;

    p[2][0] = amp;
    p[2][1] = center + center_lambda;
    p[2][2] = width;
    p[2][3] = bias;

    p[3][0] = amp;
    p[3][1] = center;
    p[3][2] = width + width_lambda;
    p[3][3] = bias;

    p[4][0] = amp;
    p[4][1] = center;
    p[4][2] = width;
    p[4][3] = bias + bias_lambda;

    char* ptr[3];
    ptr[0] = (char *)x;
    ptr[1] = (char *)y;
    ptr[2] = (char *)&points;

    downhill_simplex(p, ndim, ndim, 1E-6, gfit_eval, ptr);

    //------------------------//
    // check for "bad" values //
    //------------------------//

    float fslice = p[0][1];
    if (fslice < 0.0 || fslice > points - 1)
        return(0);

    width = p[0][2];

    int near_slice_idx = (int)(fslice + 0.5);
    float f1, bw;
    qscat->ses.GetSliceFreqBw(near_slice_idx, &f1, &bw);
    *peak_slice = fslice;
    *peak_freq = f1 + bw * (fslice - (float)near_slice_idx + 0.5);
    *width_freq = bw * width;

    return(1);
}

//-----------//
// gfit_eval //
//-----------//
// gaussian fit evaluation function (MSE)

#define MIN_AMPLITUDE  0.0
#define MIN_CENTER     -1.0
#define MAX_CENTER     12.0
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

    if (c[0] < MIN_AMPLITUDE)
        c[0] = MIN_AMPLITUDE;

    if (c[1] < MIN_CENTER)
        c[1] = MIN_CENTER;
    if (c[1] > MAX_CENTER)
        c[1] = MAX_CENTER;

    if (c[2] > MAX_WIDTH)
        c[2] = MAX_WIDTH;

    //---------------------//
    // the real evaluation //
    //---------------------//

    double sum_dif = 0.0;
    for (int i = 0; i < points; i++)
    {
        double ex = (x[i] - c[1]) / c[2];
        double arg = -ex * ex;
        double val = c[0] * exp(arg) + c[3];
        double dif = val - y[i];
        sum_dif += ((dif * dif) / (double)points);
    }
    return(sum_dif);
}

//-------------------------//
// EchoInfo::SpotOrbitStep //
//-------------------------//

unsigned char
EchoInfo::SpotOrbitStep(
    int  spot_idx)
{
    if (priOfOrbitStepChange != 255 && spot_idx < priOfOrbitStepChange)
    {
        if (orbitStep == 0)
            return(ORBIT_STEPS - 1);
        else
            return(orbitStep - 1);
    }
    else
        return(orbitStep);
}

//---------------//
// gaussian_fit2 //
//---------------//
// like gaussian_fit, but solves for the best amplitude and bias, thus
// reducing a 4D problem to 2D
// reduction equations courtesy of B. Stiles

#define TOO_WIDE  11.0

int
gaussian_fit2(
    Qscat*   qscat,
    double*  x,
    double*  y,
    int      points,
    float*   peak_slice,
    float*   peak_freq,
    float*   width_freq)
{
    //----------------//
    // allocate array //
    //----------------//

    int ndim = 2;
    double** p = (double**)make_array(sizeof(double), 2, ndim + 1, ndim);
    if (p == NULL)
        return(0);

    //--------------------//
    // initialize simplex //
    //--------------------//

    int max_idx = 0;
    for (int i = 0; i < points; i++)
    {
        if (y[i] > y[max_idx])
            max_idx = i;
    }

    double center = (double)max_idx;
    double center_lambda = 0.2;
    double width = 3.5;
    double width_lambda = 0.5;

    p[0][0] = center;
    p[0][1] = width;

    p[1][0] = center + center_lambda;
    p[1][1] = width;

    p[2][0] = center;
    p[2][1] = width + width_lambda;

    char* ptr[3];
    ptr[0] = (char *)x;
    ptr[1] = (char *)y;
    ptr[2] = (char *)&points;

    if (! downhill_simplex(p, ndim, ndim, 1E-6, gfit_eval2, ptr))
        return(0);

    //------------------------//
    // check for "bad" values //
    //------------------------//

    float fslice = p[0][0];
    if (fslice < 0.0 || fslice > points - 1)
        return(0);

    width = p[0][1];
    if (width > TOO_WIDE)
        return(0);

    //----------------------//
    // transfer information //
    //----------------------//

    int near_slice_idx = (int)(fslice + 0.5);
    float f1, bw;
    qscat->ses.GetSliceFreqBw(near_slice_idx, &f1, &bw);
    *peak_slice = fslice;
    *peak_freq = f1 + bw * (fslice - (float)near_slice_idx + 0.5);
    *width_freq = bw * width;

    return(1);
}

//------------//
// gfit_eval2 //
//------------//
// gaussian fit evaluation function (MSE)

#define MIN_CENTER     -1.0
#define MAX_CENTER     12.0
#define MAX_WIDTH      12.0

double
gfit_eval2(
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

#endif
