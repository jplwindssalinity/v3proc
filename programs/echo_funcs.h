//=========================================================//
// Copyright (C) 1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef ECHO_FUNCS_H
#define ECHO_FUNCS_H

static const char rcs_id_echo_funcs_h[] =
    "@(#) $Id$";

#define SPOTS_PER_FRAME  100

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
    unsigned char   beamIdx[SPOTS_PER_FRAME];
    unsigned short  idealEncoder[SPOTS_PER_FRAME];
    double          txCenterAzimuthAngle[SPOTS_PER_FRAME];
    float           txDoppler[SPOTS_PER_FRAME];
    float           rxGateDelay[SPOTS_PER_FRAME];
    unsigned char   flag[SPOTS_PER_FRAME];
    double          totalSignalEnergy[SPOTS_PER_FRAME];
    float           measSpecPeakFreq[SPOTS_PER_FRAME];
};

int  gaussian_fit(Qscat* qscat, double* x, double* y, int points,
         float* peak_slice, float* peak_freq);
double  gfit_eval(double* x, void* ptr);

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

//--------------//
// gaussian_fit //
//--------------//

#define CENTER_ACCY 0.0001

int
gaussian_fit(
    Qscat*   qscat,
    double*  x,
    double*  y,
    int      points,
    float*   peak_slice,
    float*   peak_freq)
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
    double amp_lambda = amp / 50.0;
    double center = (double)max_idx;
    double center_lambda = 0.5;
    double width = 3.0;
    double width_lambda = 0.5;
    double bias = 0.0;
    double bias_lambda = amp_lambda;

    p[0][0] = amp;
    p[0][1] = center;
    p[0][2] = width;
    p[0][3] = bias;

    p[1][0] = amp - amp_lambda;
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

    float fslice = p[0][1];
    if (fslice < 0.0 || fslice > points - 1)
        return(0);

    int near_slice_idx = (int)(fslice + 0.5);
    float f1, bw;
    qscat->ses.GetSliceFreqBw(near_slice_idx, &f1, &bw);
    *peak_slice = fslice;
    *peak_freq = f1 + bw * (fslice - (float)near_slice_idx + 0.5);

    return(1);
}

//-----------//
// gfit_eval //
//-----------//

double
gfit_eval(
    double*  c,
    void*    ptr)
{
    char** ptr2 = (char**)ptr;
    double* x = (double *)ptr2[0];
    double* y = (double *)ptr2[1];
    int points = *(int *)ptr2[2];

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

#endif
