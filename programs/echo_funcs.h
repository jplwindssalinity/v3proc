//------------------//
// TYPE DEFINITIONS //
//------------------//

enum DataId { SPOT_ID, EPHEMERIS_ID, ORBIT_STEP_ID, ORBIT_TIME_ID };

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

int  write_spot(int fd, int beam_idx, float tx_doppler, float rx_gate_delay,
         int ideal_encoder, int held_encoder, float meas_spect_peak,
         float exp_spect_peak, float total_signal_energy, int land_flag);
int  read_spot(int fd, int* beam_idx, float* tx_doppler, float* rx_gate_delay,
         int* ideal_encoder, int* held_encoder, float* meas_spec_peak,
         float* exp_spec_peak, float* total_signal_energy, int* land_flag);

int  write_ephemeris(int fd, float gcx, float gcy, float gcz, float velx,
         float vely, float velz, float roll, float pitch, float yaw);
int  read_ephemeris(int fd, float* gcx, float* gcy, float* gcz, float* velx,
         float* vely, float* velz, float* roll, float* pitch, float* yaw);

int  write_orbit_step(int fd, int orbit_step);
int  read_orbit_step(int fd, int* orbit_step);

int  write_orbit_time(int fd, unsigned int orbit_ticks);
int  read_orbit_time(int fd, unsigned int* orbit_ticks);

//------------//
// write_spot //
//------------//

#define SPOT_RECORD_SIZE  36

int
write_spot(
    int    fd,
    int    beam_idx,
    float  tx_doppler,
    float  rx_gate_delay,
    int    ideal_encoder,
    int    held_encoder,
    float  meas_spect_peak,
    float  exp_spect_peak,
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

    memcpy(buffer + count, &held_encoder, size);
    count += size;

    size = sizeof(float);
    memcpy(buffer + count, &meas_spect_peak, size);
    count += size;

    memcpy(buffer + count, &exp_spect_peak, size);
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
    int*    held_encoder,
    float*  meas_spec_peak,
    float*  exp_spec_peak,
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

    memcpy(held_encoder, buffer + count, size);
    count += size;

    size = sizeof(float);
    memcpy(meas_spec_peak, buffer + count, size);
    count += size;

    memcpy(exp_spec_peak, buffer + count, size);
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
