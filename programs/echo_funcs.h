//----------//
// INCLUDES //
//----------//

#include "Qscat.h"

//------------------//
// TYPE DEFINITIONS //
//------------------//

enum DataId { SPOT_ID, EPHEMERIS_ID, ORBIT_STEP_ID, ORBIT_TIME_ID };

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

int  write_spot(int fd, int beam_idx, float tx_doppler, float rx_gate_delay,
         int ideal_encoder, float tx_center_azimuth,
         float meas_spec_peak_freq, float total_signal_energy, int land_flag);
int  read_spot(int fd, int* beam_idx, float* tx_doppler, float* rx_gate_delay,
         int* ideal_encoder, float* tx_center_azimuth,
         float* meas_spec_peak_freq, float* total_signal_energy,
         int* land_flag);

int  write_ephemeris(int fd, float gcx, float gcy, float gcz, float velx,
         float vely, float velz, float roll, float pitch, float yaw);
int  read_ephemeris(int fd, float* gcx, float* gcy, float* gcz, float* velx,
         float* vely, float* velz, float* roll, float* pitch, float* yaw);

int  write_orbit_step(int fd, int orbit_step);
int  read_orbit_step(int fd, int* orbit_step);

int  write_orbit_time(int fd, unsigned int orbit_ticks);
int  read_orbit_time(int fd, unsigned int* orbit_ticks);

int  find_peak(Qscat* qscat, double* x, double* y, int points,
         float* peak_slice, float* peak_freq);
int  large_slope(double* x, double* y, double c[4], double step[4],
         int points, int* idx);
int  optimize(double* x, double* y, double c[4], double step[4],
         int points, double* sse);
double  evaluate(double* x, double* y, double c[4], int points);

//------------//
// write_spot //
//------------//

#define SPOT_RECORD_SIZE  32

int
write_spot(
    int    fd,
    int    beam_idx,
    float  tx_doppler,
    float  rx_gate_delay,
    int    ideal_encoder,
    float  tx_center_azimuth,
    float  meas_spec_peak_freq,
    float  total_signal_energy,
    int    land_flag)
{
    // form the buffer
    char buffer[SPOT_RECORD_SIZE + 4];
    int id = SPOT_ID;
    int count = 0;

    int size = sizeof(int);
    memcpy(buffer + count, &id, size);
    count += size;

    size = sizeof(int);
    memcpy(buffer + count, &beam_idx, size);
    count += size;

    size = sizeof(float);
    memcpy(buffer + count, &tx_doppler, size);
    count += size;

    memcpy(buffer + count, &rx_gate_delay, size);
    count += size;

    size = sizeof(int);
    memcpy(buffer + count, &ideal_encoder, size);
    count += size;

    size = sizeof(float);
    memcpy(buffer + count, &tx_center_azimuth, size);
    count += size;

    memcpy(buffer + count, &meas_spec_peak_freq, size);
    count += size;

    memcpy(buffer + count, &total_signal_energy, size);
    count += size;

    size = sizeof(int);
    memcpy(buffer + count, &land_flag, size);
    count += size;

    if (write(fd, (void *)buffer, count) != count)
        return(0);

    return(1);
}

//-----------//
// read_spot //
//-----------//

int
read_spot(
    int     fd,
    int*    beam_idx,
    float*  tx_doppler,
    float*  rx_gate_delay,
    int*    ideal_encoder,
    float*  tx_center_azimuth,
    float*  meas_spec_peak_freq,
    float*  total_signal_energy,
    int*    land_flag)
{
    // form the buffer
    char buffer[SPOT_RECORD_SIZE];

    if (read(fd, (void *)buffer, SPOT_RECORD_SIZE) != SPOT_RECORD_SIZE)
        return(0);

    int count = 0;

    int size = sizeof(int);
    memcpy(beam_idx, buffer + count, size);
    count += size;

    size = sizeof(float);
    memcpy(tx_doppler, buffer + count, size);
    count += size;

    memcpy(rx_gate_delay, buffer + count, size);
    count += size;

    size = sizeof(int);
    memcpy(ideal_encoder, buffer + count, size);
    count += size;

    size = sizeof(float);
    memcpy(tx_center_azimuth, buffer + count, size);
    count += size;

    memcpy(meas_spec_peak_freq, buffer + count, size);
    count += size;

    memcpy(total_signal_energy, buffer + count, size);
    count += size;

    size = sizeof(int);
    memcpy(land_flag, buffer + count, size);
    count += size;

    return(1);
}

//-----------------//
// write_ephemeris //
//-----------------//

#define EPHEMERIS_RECORD_SIZE  36

int
write_ephemeris(
    int    fd,
    float  gcx,
    float  gcy,
    float  gcz,
    float  velx,
    float  vely,
    float  velz,
    float  roll,
    float  pitch,
    float  yaw)
{
    // form the buffer
    char buffer[EPHEMERIS_RECORD_SIZE + 4];
    int id = EPHEMERIS_ID;
    int count = 0;

    int size = sizeof(int);
    memcpy(buffer + count, &id, size);
    count += size;

    size = sizeof(float);
    memcpy(buffer + count, &gcx, size);
    count += size;

    memcpy(buffer + count, &gcy, size);
    count += size;

    memcpy(buffer + count, &gcz, size);
    count += size;

    memcpy(buffer + count, &velx, size);
    count += size;

    memcpy(buffer + count, &vely, size);
    count += size;

    memcpy(buffer + count, &velz, size);
    count += size;

    memcpy(buffer + count, &roll, size);
    count += size;

    memcpy(buffer + count, &pitch, size);
    count += size;

    memcpy(buffer + count, &yaw, size);
    count += size;

    if (write(fd, (void *)buffer, count) != count)
        return(0);

    return(1);
}

//----------------//
// read_ephemeris //
//----------------//

int
read_ephemeris(
    int     fd,
    float*  gcx,
    float*  gcy,
    float*  gcz,
    float*  velx,
    float*  vely,
    float*  velz,
    float*  roll,
    float*  pitch,
    float*  yaw)
{
    // form the buffer
    char buffer[EPHEMERIS_RECORD_SIZE];

    if (read(fd, (void *)buffer, EPHEMERIS_RECORD_SIZE) !=
        EPHEMERIS_RECORD_SIZE)
    {
        return(0);
    }

    int count = 0;

    int size = sizeof(float);
    memcpy(gcx, buffer + count, size);
    count += size;

    memcpy(gcy, buffer + count, size);
    count += size;

    memcpy(gcz, buffer + count, size);
    count += size;

    memcpy(velx, buffer + count, size);
    count += size;

    memcpy(vely, buffer + count, size);
    count += size;

    memcpy(velz, buffer + count, size);
    count += size;

    memcpy(roll, buffer + count, size);
    count += size;

    memcpy(pitch, buffer + count, size);
    count += size;

    memcpy(yaw, buffer + count, size);
    count += size;

    return(1);
}

//------------------//
// write_orbit_step //
//------------------//

#define ORBIT_STEP_RECORD_SIZE  4

int
write_orbit_step(
    int  fd,
    int  orbit_step)
{
    // form the buffer
    char buffer[ORBIT_STEP_RECORD_SIZE + 4];
    int id = ORBIT_STEP_ID;
    int count = 0;

    int size = sizeof(int);
    memcpy(buffer + count, &id, size);
    count += size;

    size = sizeof(int);
    memcpy(buffer + count, &orbit_step, size);
    count += size;

    if (write(fd, (void *)buffer, count) != count)
        return(0);

    return(1);
}

//-----------------//
// read_orbit_step //
//-----------------//

int
read_orbit_step(
    int   fd,
    int*  orbit_step)
{
    // form the buffer
    char buffer[ORBIT_STEP_RECORD_SIZE];

    if (read(fd, (void *)buffer, ORBIT_STEP_RECORD_SIZE) !=
        ORBIT_STEP_RECORD_SIZE)
    {
        return(0);
    }

    int count = 0;

    int size = sizeof(int);
    memcpy(orbit_step, buffer + count, size);
    count += size;

    return(1);
}

//------------------//
// write_orbit_time //
//------------------//

#define ORBIT_TIME_RECORD_SIZE  4

int
write_orbit_time(
    int           fd,
    unsigned int  orbit_ticks)
{
    // form the buffer
    char buffer[ORBIT_TIME_RECORD_SIZE + 4];
    int id = ORBIT_TIME_ID;
    int count = 0;

    int size = sizeof(int);
    memcpy(buffer + count, &id, size);
    count += size;

    size = sizeof(unsigned int);
    memcpy(buffer + count, &orbit_ticks, size);
    count += size;

    if (write(fd, (void *)buffer, count) != count)
        return(0);

    return(1);
}

//-----------------//
// read_orbit_time //
//-----------------//

int
read_orbit_time(
    int            fd,
    unsigned int*  orbit_ticks)
{
    // form the buffer
    char buffer[ORBIT_TIME_RECORD_SIZE];

    if (read(fd, (void *)buffer, ORBIT_TIME_RECORD_SIZE) !=
        ORBIT_TIME_RECORD_SIZE)
    {
        return(0);
    }

    int count = 0;

    int size = sizeof(unsigned int);
    memcpy(orbit_ticks, buffer + count, size);
    count += size;

    return(1);
}

//-----------//
// find_peak //
//-----------//

#define CENTER_ACCY 0.0001

int
find_peak(
    Qscat*   qscat,
    double*  x,
    double*  y,
    int      points,
    float*   peak_slice,
    float*   peak_freq)
{
    int max_idx = 0;
    for (int i = 0; i < points; i++)
    {
        if (y[i] > y[max_idx])
            max_idx = i;
    }

    double c[4];
    c[0] = y[max_idx];
    c[1] = max_idx;
    c[2] = 5.0;
    c[3] = 0.0;

    double step[4];
    step[0] = c[0] / 10.0;
    step[1] = 1.0;
    step[2] = 1.0;
    step[3] = 0.0;

    double use_step[4] = { 0.0, 0.0, 0.0, 0.0 };

    do
    {
        int idx;
        large_slope(x, y, c, step, points, &idx);

        use_step[idx] = step[idx];
        double sse;
        if (! optimize(x, y, c, use_step, points, &sse))
            return(0);
        use_step[idx] = 0.0;

        if (idx == 1 && step[idx] < CENTER_ACCY)
            break;

        step[idx] /= 2.0;
    } while (1);

    float fslice = c[1];
    if (fslice < 0.0 || fslice > points - 1)
        return(0);

    int near_slice_idx = (int)(fslice + 0.5);
    float f1, bw;
    qscat->ses.GetSliceFreqBw(near_slice_idx, &f1, &bw);
    *peak_slice = fslice;
    *peak_freq = f1 + bw * (fslice - (float)near_slice_idx + 0.5);

    return(1);
}

//-------------//
// large_slope //
//-------------//

int
large_slope(
    double*  x,
    double*  y,
    double   c[4],
    double   step[4],
    int      points,
    int*     idx)
{
    double try_c_low[4];
    double try_c_high[4];

    double max_dif = 0.0;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            try_c_low[j] = c[j];
            try_c_high[j] = c[j];
        }
        try_c_low[i] -= step[i] / 2.0;
        try_c_high[i] += step[i] / 2.0;
        double low_sse = evaluate(x, y, try_c_low, points);
        double high_sse = evaluate(x, y, try_c_high, points);
        double dif = fabs(low_sse - high_sse);
        if (dif > max_dif)
        {
            max_dif = dif;
            *idx = i;
        }
    }
    return(1);
}

//----------//
// optimize //
//----------//

#define SWAP(a,b)  { swap = (a); (a) = (b); (b) = swap; }

int
optimize(
    double*  x,
    double*  y,
    double   c[4],
    double   step[4],
    int      points,
    double*  sse)
{
    //---------------------//
    // determine direction //
    //---------------------//

    // assume going forward
    double old_c[4];
    for (int i = 0; i < 4; i++)
    {
        old_c[i] = c[i] - step[i] / 2.0;
    }
    double old_sse = evaluate(x, y, old_c, points);

    double new_c[4];
    for (int i = 0; i < 4; i++)
    {
        new_c[i] = c[i] + step[i] / 2.0;
    }
    double new_sse = evaluate(x, y, new_c, points);

    float delta_sign = 1.0;

    double swap;
    if (old_sse < new_sse)
    {
        // reverse direction
        delta_sign = -1.0;
        for (int i = 0; i < 4; i++)
        {
            SWAP(new_c[i], old_c[i]);
        }
        SWAP(new_sse, old_sse);
    }

    //------------------------------//
    // search until minima is found //
    //------------------------------//

    for (;;)
    {
        if (new_sse < old_sse)
        {
            // continue moving forward
            for (int i = 0; i < 4; i++)
            {
                old_c[i] = new_c[i];
                new_c[i] += delta_sign * step[i];
            }
            if (new_c[2] > 10.0)    // too wide
                return(0);
            if (new_c[1] < 0.0 || new_c[1] > 9.0)    // off center
                return(0);
            old_sse = new_sse;
            new_sse = evaluate(x, y, new_c, points);
        }
        else
        {
            // stop (new value is worse than the old)
            for (int i = 0; i < 4; i++)
            {
                c[i] = old_c[i];
            }
            break;
        }
    }
    *sse = old_sse;
    return(1);
}

//----------//
// evaluate //
//----------//

double
evaluate(
    double*  x,
    double*  y,
    double   c[4],
    int      points)
{
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
