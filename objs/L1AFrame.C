//==============================================================//
// Copyright (C) 1997-1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_l1aframe_c[] =
    "@(#) $Id$";

#include <assert.h>
#include <memory.h>
#include <malloc.h>
#include <stdlib.h>

#include "Constants.h"
#include "EarthPosition.h"
#include "L1AHdf.h"
#include "L1AFrame.h"

#define COPY_FROM_HDF_VALUE(l1aHdf,param,paramIdE,unitE,x) \
    param = l1aHdf->GetParameter(paramIdE, unitE);\
    assert(param != 0);\
    memcpy((void *)&(x), (void *)(param->data), sizeof(x));

#define COPY_FROM_HDF_ADDRESS(l1aHdf,param,paramIdE,unitE,x,size) \
    param = l1aHdf->GetParameter(paramIdE, unitE);\
    assert(param != 0);\
    memcpy((void *)x, (void *)(param->data), size);


//==========//
// L1AFrame //
//==========//

L1AFrame::L1AFrame()
: time(0), instrumentTicks(0), orbitTicks(0), orbitStep(0),
  priOfOrbitStepChange(255), gcAltitude(0.0), gcLongitude(0.0),
  gcLatitude(0.0), gcX(0.0), gcY(0.0), gcZ(0.0), velX(0.0), velY(0.0),
  velZ(0.0), calPosition(255), loopbackSlices(NULL),
  loopbackNoise(0), loadSlices(NULL), loadNoise(0),
  antennaPosition(NULL), science(NULL), spotNoise(NULL),
  frame_inst_status(0),
  frame_err_status(0),
  frame_qual_flag(0),
  antennaCyclesPerFrame(0), spotsPerFrame(0), slicesPerSpot(0),
  slicesPerFrame(0)
{
  (void)memset(&frame_time, 0, 24);
  (void)memset(&status, 0, sizeof(GSL1AStatus));
  (void)memset(&engdata, 0, sizeof(GSL1AEngData));
  (void)memset(&in_eu, 0, sizeof(GSL1AEu));
  return;
}

L1AFrame::~L1AFrame()
{
    Deallocate();
    return;
}

//---------------//
// L1A::Allocate //
//---------------//

int
L1AFrame::Allocate(
    int  number_of_beams,
    int  antenna_cycles_per_frame,
    int  slices_per_spot)
{
    antennaCyclesPerFrame = antenna_cycles_per_frame;
    spotsPerFrame = number_of_beams * antennaCyclesPerFrame;
    slicesPerSpot = slices_per_spot;
    slicesPerFrame = spotsPerFrame * slicesPerSpot;

    //----------------------------//
    // allocate antenna positions //
    //----------------------------//

    antennaPosition = (unsigned short *)malloc(spotsPerFrame *
        sizeof(unsigned short));
    if (antennaPosition == NULL)
        return(0);

    //---------------------------//
    // allocate cal measurements //
    //---------------------------//

    loopbackSlices = (unsigned int *)malloc(slicesPerSpot*sizeof(unsigned int));
    if (loopbackSlices == NULL)
    {
        return(0);
    }
    loadSlices = (unsigned int *)malloc(slicesPerSpot * sizeof(unsigned int));
    if (loadSlices == NULL)
    {
        return(0);
    }

    //-------------------------------//
    // allocate science measurements //
    //-------------------------------//

    science = (unsigned int *)malloc(slicesPerFrame * sizeof(unsigned int));
    if (science == NULL)
    {
        return(0);
    }
    spotNoise = (unsigned int *)malloc(spotsPerFrame * sizeof(unsigned int));
    if (spotNoise == NULL)
    {
        return(0);
    }

    return(1);
}

//----------------------//
// L1AFrame::Deallocate //
//----------------------//

int
L1AFrame::Deallocate()
{
    if (antennaPosition)
        free(antennaPosition);
    if (loopbackSlices)
        free(loopbackSlices);
    if (loadSlices)
        free(loadSlices);
    if (science)
        free(science);
    if (spotNoise)
        free(spotNoise);
    antennaPosition = NULL;
    loopbackSlices = NULL;
    loadSlices = NULL;
    science = NULL;
    spotNoise = NULL;
    antennaCyclesPerFrame = 0;
    spotsPerFrame = 0;
    slicesPerSpot = 0;
    slicesPerFrame = 0;
    return(1);
}

//---------------------//
// L1AFrame::FrameSize //
//---------------------//

int
L1AFrame::FrameSize()
{
    int size = 0;
    size += sizeof(double);         // time
    size += sizeof(unsigned int);   // instrument ticks
    size += sizeof(unsigned int);   // orbit ticks
    size += sizeof(unsigned char);  // orbit step
    size += sizeof(unsigned char);  // pri of orbit step change
    size += sizeof(float);          // altitude
    size += sizeof(float);          // longitude
    size += sizeof(float);          // latitude
    size += sizeof(float);          // x
    size += sizeof(float);          // y
    size += sizeof(float);          // z
    size += sizeof(float);          // vx
    size += sizeof(float);          // vy
    size += sizeof(float);          // vz
    size += sizeof(float);          // roll
    size += sizeof(float);          // pitch
    size += sizeof(float);          // yaw
//    size += sizeof(float);          // PtGr
    size += sizeof(unsigned char);  // cal position
    size += sizeof(unsigned int) * slicesPerSpot;  // loopback slices
    size += sizeof(unsigned int);                  // loopback noise
    size += sizeof(unsigned int) * slicesPerSpot;  // load slices
    size += sizeof(unsigned int);                  // load noise
    size += sizeof(unsigned short) * spotsPerFrame;// antenna position
    size += sizeof(unsigned int) * slicesPerFrame; // science data
    size += sizeof(unsigned int) * spotsPerFrame;  // spot noise

    size += sizeof(GSL1AStatus);  // status structure
    size += sizeof(GSL1AEngData); // engdata structure
    size += sizeof(GSL1AEu);      // in_eu structure

    size += sizeof(unsigned int);    // frame_inst_status
    size += sizeof(unsigned int);    // frame_err_status
    size += sizeof(unsigned short);  // frame_qual_flag
    size += 13 * sizeof(unsigned char);   // pulse_qual_flag

    size += 24;  // frame_time

    return(size);
}

//----------------//
// L1AFrame::Pack //
//----------------//

int
L1AFrame::Pack(
    char*  buffer)
{
    int idx = 0;
    int size;

    size = sizeof(double);
    memcpy((void *)(buffer + idx), (void *)&time, size);
    idx += size;

    size = sizeof(unsigned int);
    memcpy((void *)(buffer + idx), (void *)&instrumentTicks, size);
    idx += size;

    memcpy((void *)(buffer + idx), (void *)&orbitTicks, size);
    idx += size;

    size = sizeof(unsigned char);
    memcpy((void *)(buffer + idx), (void *)&orbitStep, size);
    idx += size;

    memcpy((void *)(buffer + idx), (void *)&priOfOrbitStepChange, size);
    idx += size;

    size = sizeof(float);
    memcpy((void *)(buffer + idx), (void *)&gcAltitude, size);
    idx += size;

    memcpy((void *)(buffer + idx), (void *)&gcLongitude, size);
    idx += size;

    memcpy((void *)(buffer + idx), (void *)&gcLatitude, size);
    idx += size;

    memcpy((void *)(buffer + idx), (void *)&gcX, size);
    idx += size;

    memcpy((void *)(buffer + idx), (void *)&gcY, size);
    idx += size;

    memcpy((void *)(buffer + idx), (void *)&gcZ, size);
    idx += size;

    memcpy((void *)(buffer + idx), (void *)&velX, size);
    idx += size;

    memcpy((void *)(buffer + idx), (void *)&velY, size);
    idx += size;

    memcpy((void *)(buffer + idx), (void *)&velZ, size);
    idx += size;

    float tmp_float;
    tmp_float = attitude.GetRoll();
    memcpy((void *)(buffer + idx), (void *)&tmp_float, size);
    idx += size;

    tmp_float = attitude.GetPitch();
    memcpy((void *)(buffer + idx), (void *)&tmp_float, size);
    idx += size;

    tmp_float = attitude.GetYaw();
    memcpy((void *)(buffer + idx), (void *)&tmp_float, size);
    idx += size;

    size = sizeof(unsigned char);
    memcpy((void *)(buffer + idx), (void *)&calPosition, size);
    idx += size;

    size = sizeof(unsigned int) * slicesPerSpot;
    memcpy((void *)(buffer + idx), (void *)loopbackSlices, size);
    idx += size;

    size = sizeof(unsigned int);
    memcpy((void *)(buffer + idx), (void *)&loopbackNoise, size);
    idx += size;

    size = sizeof(unsigned int) * slicesPerSpot;
    memcpy((void *)(buffer + idx), (void *)loadSlices, size);
    idx += size;

    size = sizeof(unsigned int);
    memcpy((void *)(buffer + idx), (void *)&loadNoise, size);
    idx += size;

    size = sizeof(unsigned short) * spotsPerFrame;
    memcpy((void *)(buffer + idx), (void *)antennaPosition, size);
    idx += size;

    size = sizeof(unsigned int) * slicesPerFrame;
    memcpy((void *)(buffer + idx), (void *)science, size);
    idx += size;

    size = sizeof(unsigned int) * spotsPerFrame;
    memcpy((void *)(buffer + idx), (void *)spotNoise, size);
    idx += size;

    size = sizeof(GSL1AStatus);
    memcpy((void *)(buffer + idx), (void *)&status, size);
    idx += size;

    size = sizeof(GSL1AEngData);
    memcpy((void *)(buffer + idx), (void *)&engdata, size);
    idx += size;

    size = sizeof(GSL1AEu);
    memcpy((void *)(buffer + idx), (void *)&in_eu, size);
    idx += size;

    size = sizeof(unsigned int);
    memcpy((void *)(buffer + idx), (void *)&frame_inst_status, size);
    idx += size;

    size = sizeof(unsigned int);
    memcpy((void *)(buffer + idx), (void *)&frame_err_status, size);
    idx += size;

    size = sizeof(unsigned short);
    memcpy((void *)(buffer + idx), (void *)&frame_qual_flag, size);
    idx += size;

    size = 13*sizeof(unsigned char);
    memcpy((void *)(buffer + idx), (void *)&pulse_qual_flag, size);
    idx += size;

    size = 24;
    memcpy((void *)(buffer + idx), (void *)frame_time, size);
    idx += size;

    return(idx);
}

//------------------//
// L1AFrame::Unpack //
//------------------//

int
L1AFrame::Unpack(
    char*    buffer)
{
    int idx = 0;
    int size;

    size = sizeof(double);
    memcpy((void *)&time, (void *)(buffer + idx), size);
    idx += size;

    size = sizeof(unsigned int);
    memcpy((void *)&instrumentTicks, (void *)(buffer + idx), size);
    idx += size;

    memcpy((void *)&orbitTicks, (void *)(buffer + idx), size);
    idx += size;

    size = sizeof(unsigned char);
    memcpy((void *)&orbitStep, (void *)(buffer + idx), size);
    idx += size;

    memcpy((void *)&priOfOrbitStepChange, (void *)(buffer + idx), size);
    idx += size;

    size = sizeof(float);
    memcpy((void *)&gcAltitude, (void *)(buffer + idx), size);
    idx += size;

    memcpy((void *)&gcLongitude, (void *)(buffer + idx), size);
    idx += size;

    memcpy((void *)&gcLatitude, (void *)(buffer + idx), size);
    idx += size;

    memcpy((void *)&gcX, (void *)(buffer + idx), size);
    idx += size;

    memcpy((void *)&gcY, (void *)(buffer + idx), size);
    idx += size;

    memcpy((void *)&gcZ, (void *)(buffer + idx), size);
    idx += size;

    memcpy((void *)&velX, (void *)(buffer + idx), size);
    idx += size;

    memcpy((void *)&velY, (void *)(buffer + idx), size);
    idx += size;

    memcpy((void *)&velZ, (void *)(buffer + idx), size);
    idx += size;

    float tmp_float;
    memcpy((void *)&tmp_float, (void *)(buffer + idx), size);
    attitude.SetRoll(tmp_float);
    idx += size;

    memcpy((void *)&tmp_float, (void *)(buffer + idx), size);
    attitude.SetPitch(tmp_float);
    idx += size;

    memcpy((void *)&tmp_float, (void *)(buffer + idx), size);
    attitude.SetYaw(tmp_float);
    idx += size;

    size = sizeof(unsigned char);
    memcpy((void *)&calPosition, (void *)(buffer + idx), size);
    idx += size;

    size = sizeof(unsigned int) * slicesPerSpot;
    memcpy((void *)loopbackSlices, (void *)(buffer + idx), size);
    idx += size;

    size = sizeof(unsigned int);
    memcpy((void *)&loopbackNoise, (void *)(buffer + idx), size);
    idx += size;

    size = sizeof(unsigned int) * slicesPerSpot;
    memcpy((void *)loadSlices, (void *)(buffer + idx), size);
    idx += size;

    size = sizeof(unsigned int);
    memcpy((void *)&loadNoise, (void *)(buffer + idx), size);
    idx += size;

    size = sizeof(unsigned short) * spotsPerFrame;
    memcpy((void *)antennaPosition, (void *)(buffer + idx), size);
    idx += size;

    size = sizeof(unsigned int) * slicesPerFrame;
    memcpy((void *)science, (void *)(buffer + idx), size);
    idx += size;

    size = sizeof(unsigned int) * spotsPerFrame;
    memcpy((void *)spotNoise, (void *)(buffer + idx), size);
    idx += size;

    size = sizeof(GSL1AStatus);
    memcpy((void *)&status, (void *)(buffer + idx), size);
    idx += size;

    size = sizeof(GSL1AEngData);
    memcpy((void *)&engdata, (void *)(buffer + idx), size);
    idx += size;

    size = sizeof(GSL1AEu);
    memcpy((void *)&in_eu, (void *)(buffer + idx), size);
    idx += size;

    size = sizeof(unsigned int);
    memcpy((void *)&frame_inst_status, (void *)(buffer + idx), size);
    idx += size;

    size = sizeof(unsigned int);
    memcpy((void *)&frame_err_status, (void *)(buffer + idx), size);
    idx += size;

    size = sizeof(unsigned short);
    memcpy((void *)&frame_qual_flag, (void *)(buffer + idx), size);
    idx += size;

    size = 13*sizeof(unsigned char);
    memcpy((void *)&pulse_qual_flag, (void *)(buffer + idx), size);
    idx += size;

    size = 24;
    memcpy((void *)frame_time, (void *)(buffer + idx), size);
    idx += size;

    return(idx);
}


//----------------------//
// L1AFrame::WriteAscii //
//----------------------//

int
L1AFrame::WriteAscii(
    FILE*  ofp)
{
    fprintf(ofp,
        "\n########################Frame Info#####################\n\n");
    fprintf(ofp,
        "Time: %g InstrumentTicks: %d OrbitTicks %d PriOfOrbitStepChange %d\n",
        time, instrumentTicks, orbitTicks, (int)priOfOrbitStepChange);
    fprintf(ofp, "GCAlt: %g GCLon: %g GCLat: %g GCX: %g GCY: %g GCZ: %g\n",
        gcAltitude, gcLongitude*rtd, gcLatitude*rtd, gcX, gcY, gcZ);
    fprintf(ofp, "VelX: %g VelY: %g VelZ: %g Roll: %g Pitch: %g Yaw: %g\n",
        velX, velY, velZ, attitude.GetRoll()*rtd, attitude.GetPitch()*rtd,
        attitude.GetYaw()*rtd);
    int offset=0;
    for (int c=0; c<spotsPerFrame; c++)
    {
        fprintf(ofp,
            "\n    :::::::::::::::: Spot Info :::::::::::::::::::  \n\n");
        fprintf(ofp, "AntennaPos: %d SpotNoise: %d Beam:%d\n",
            (int)antennaPosition[c], spotNoise[c], c%2);
        fprintf(ofp, "E(S+N) Slices(1-%d): ", slicesPerSpot);
        for (int s=0; s<slicesPerSpot; s++)
        {
            fprintf(ofp, "%d ", science[offset]);
            offset++;
        }
        fprintf(ofp, "\n");
    }
    return(1);
}

//---------------------//
// L1AFrame::UnpackHdf //
//---------------------//

int
L1AFrame::UnpackHdf(
L1AHdf*     l1aHdf)
{
    assert(l1aHdf != 0);

    Parameter* param=0;

    param = l1aHdf->GetParameter(FRAME_TIME, UNIT_L1ATIME);
    assert(param != 0);
    memcpy((void*)frame_time, (void*) param->data, L1_TIME_LEN -1);
    frame_time[L1_TIME_LEN-1] = '\0';

    COPY_FROM_HDF_VALUE(l1aHdf, param, TAI_TIME, UNIT_TAI_SECONDS, time);

    double instrument_time;
    COPY_FROM_HDF_VALUE(l1aHdf, param, INSTRUMENT_TIME, UNIT_COUNTS,
                                instrument_time);
    instrumentTicks = (unsigned int) instrument_time;

    COPY_FROM_HDF_VALUE(l1aHdf, param, ORBIT_TIME, UNIT_COUNTS, orbitTicks);
    COPY_FROM_HDF_VALUE(l1aHdf, param, DOPPLER_ORBIT_STEP, UNIT_COUNTS,
                                orbitStep);
    COPY_FROM_HDF_VALUE(l1aHdf, param, PRF_ORBIT_STEP_CHANGE, UNIT_COUNTS,
                                priOfOrbitStepChange);

    COPY_FROM_HDF_VALUE(l1aHdf, param, X_POS, UNIT_KILOMETERS, gcX);
    COPY_FROM_HDF_VALUE(l1aHdf, param, Y_POS, UNIT_KILOMETERS, gcY);
    COPY_FROM_HDF_VALUE(l1aHdf, param, Z_POS, UNIT_KILOMETERS, gcZ);

    EarthPosition v(gcX, gcY, gcZ);
    double alt, lon, lat;
    v.GetAltLonGDLat(&alt, &lon, &lat);
    gcAltitude = (float) alt;
    gcLongitude = (float) lon;
    gcLatitude = (float) lat;

    COPY_FROM_HDF_VALUE(l1aHdf, param, X_VEL, UNIT_KMPS, velX);
    COPY_FROM_HDF_VALUE(l1aHdf, param, Y_VEL, UNIT_KMPS, velY);
    COPY_FROM_HDF_VALUE(l1aHdf, param, Z_VEL, UNIT_KMPS, velZ);

    float roll, pitch, yaw;
    COPY_FROM_HDF_VALUE(l1aHdf, param, ROLL, UNIT_RADIANS, roll);
    COPY_FROM_HDF_VALUE(l1aHdf, param, PITCH, UNIT_RADIANS, pitch);
    COPY_FROM_HDF_VALUE(l1aHdf, param, YAW, UNIT_RADIANS, yaw);
    attitude.SetRPY(roll, pitch, yaw);

    COPY_FROM_HDF_VALUE(l1aHdf, param, TRUE_CAL_PULSE_POS, UNIT_DN,
                                in_eu.true_cal_pulse_pos);
    in_eu.true_cal_pulse_pos -= 1;   // HDF's offset is 1, SVT's is 0

    UnpackL1AStatus(l1aHdf);
    UnpackL1AEngData(l1aHdf);
    UnpackL1AEu(l1aHdf);

    COPY_FROM_HDF_VALUE(l1aHdf, param, FRAME_INST_STATUS, UNIT_DN,
                                frame_inst_status);
    COPY_FROM_HDF_VALUE(l1aHdf, param, FRAME_ERR_STATUS, UNIT_DN,
                                frame_err_status);
    COPY_FROM_HDF_VALUE(l1aHdf, param, FRAME_QUALITY_FLAG, UNIT_DN,
                                frame_qual_flag);
    COPY_FROM_HDF_VALUE(l1aHdf, param, PULSE_QUALITY_FLAG, UNIT_HEX_BYTES,
                                pulse_qual_flag);

//    unsigned int uint4_12[slicesPerSpot];
    COPY_FROM_HDF_ADDRESS(l1aHdf, param, LOOP_BACK_CAL_A_POWER, UNIT_DN,
                                loopbackSlices, sizeof(int) * slicesPerSpot);
//    float* floatP = loopbackSlices;
//    for (int i=0; i < slicesPerSpot; i++, floatP++)
//        *floatP = (float) (uint4_12[i]);

//    unsigned int uint4Num;
    COPY_FROM_HDF_VALUE(l1aHdf, param, LOOP_BACK_CAL_NOISE, UNIT_DN,
                        loopbackNoise);
//    loopbackNoise = (float) uint4Num;

    COPY_FROM_HDF_VALUE(l1aHdf, param, LOAD_CAL_NOISE, UNIT_DN, loadNoise);
//    loadNoise = (float) uint4Num;

//    unsigned int uint4_1200[slicesPerFrame];
    COPY_FROM_HDF_ADDRESS(l1aHdf, param, POWER_DN, UNIT_DN,
                   science, slicesPerFrame * sizeof(unsigned int));
//    floatP = science;
//    for (int i=0; i < slicesPerFrame; i++, floatP++)
//        *floatP = (float) (uint4_1200[i]);

//    unsigned int uint4_100[spotsPerFrame];
    COPY_FROM_HDF_ADDRESS(l1aHdf, param, NOISE_DN, UNIT_DN,
                   spotNoise, spotsPerFrame * sizeof(unsigned int));
//    floatP = spotNoise;
//    for (int i=0; i < spotsPerFrame; i++, floatP++)
//        *floatP = (float) (uint4_100[i]);

    COPY_FROM_HDF_ADDRESS(l1aHdf, param, ANTENNA_POS, UNIT_DN,
                   antennaPosition, sizeof(unsigned short) * spotsPerFrame);

    return 1;

} //L1AFrame::UnpackHdf

void
L1AFrame::DoubleToVTCW(
double    vtcw_time,
char*     vtcw6Bytes)
{
    assert(vtcw6Bytes != 0);

    unsigned int vtcw_time_hi4 = (int)(vtcw_time/65536);
    unsigned short vtcw_time_lo2 =
               (unsigned short)(vtcw_time - (double)(vtcw_time_hi4) * 65536.0);
    memcpy((void *)vtcw6Bytes, (void *)(&vtcw_time_hi4), sizeof(unsigned int));
    memcpy((void *)(vtcw6Bytes + 4), (void *)(&vtcw_time_lo2),
                                     sizeof(unsigned short));
    return;

} // L1AFrame::DoubleToVTCW

double
L1AFrame::VTCWToDouble(
  char*     vtcw6Bytes)
{
  unsigned int vtcw_hi4 = 0;
  unsigned short vtcw_lo2 = 0;
  (void)memcpy(&vtcw_hi4, vtcw6Bytes, sizeof(unsigned int));
  (void)memcpy(&vtcw_lo2, vtcw6Bytes+4, sizeof(unsigned short));
  return((double)(vtcw_hi4)*65536.0 + vtcw_lo2);
   
} // L1AFrame::VTCWToDouble

void
L1AFrame::UnpackL1AStatus(
L1AHdf*     l1aHdf)
{
    Parameter* param=0;
    COPY_FROM_HDF_VALUE(l1aHdf, param, PRF_COUNT, UNIT_COUNTS,
                                       status.prf_count);
    COPY_FROM_HDF_VALUE(l1aHdf, param, PRF_CYCLE_TIME, UNIT_DN,
                                       status.prf_cycle_time);
    COPY_FROM_HDF_VALUE(l1aHdf, param, RANGE_GATE_A_DELAY, UNIT_DN,
                                       status.range_gate_a_delay);
    COPY_FROM_HDF_VALUE(l1aHdf, param, RANGE_GATE_A_WIDTH, UNIT_DN,
                                       status.range_gate_a_width);
    COPY_FROM_HDF_VALUE(l1aHdf, param, RANGE_GATE_B_DELAY, UNIT_DN,
                                       status.range_gate_b_delay);
    COPY_FROM_HDF_VALUE(l1aHdf, param, RANGE_GATE_B_WIDTH, UNIT_DN,
                                       status.range_gate_b_width);
    COPY_FROM_HDF_VALUE(l1aHdf, param, PULSE_WIDTH, UNIT_DN,
                                       status.pulse_width);
    COPY_FROM_HDF_VALUE(l1aHdf, param, PREDICT_ANT_POS_COUNT, UNIT_COUNTS,
                                       status.pred_antenna_pos_count);
    COPY_FROM_HDF_VALUE(l1aHdf, param, DOPPLER_ORBIT_STEP, UNIT_COUNTS,
                                       status.doppler_orbit_step);
    COPY_FROM_HDF_VALUE(l1aHdf, param, PRF_ORBIT_STEP_CHANGE, UNIT_COUNTS,
                                       status.prf_orbit_step_change);
    double vtcw_double;
    COPY_FROM_HDF_VALUE(l1aHdf, param, ADEOS_TIME, UNIT_TICKS, vtcw_double);
    DoubleToVTCW(vtcw_double, status.vtcw);

    char* corres_inst_time_ptr = status.corres_instr_time;
    *corres_inst_time_ptr = '\0';
    corres_inst_time_ptr++;
    double corres_inst_time;
    COPY_FROM_HDF_VALUE(l1aHdf, param, CORRES_INSTR_TIME, UNIT_TICKS,
                                corres_inst_time);
    unsigned int int_corres_inst_time = (unsigned int) corres_inst_time;
    (void) memcpy((void*) corres_inst_time_ptr, (void*) &int_corres_inst_time,
                                sizeof(int_corres_inst_time));
    return;

} // L1AFrame::UnpackL1AStatus

void
L1AFrame::UnpackL1AEngData(
L1AHdf*     l1aHdf)
{
    Parameter* param=0;
    COPY_FROM_HDF_VALUE(l1aHdf, param, PRECISION_COUPLER_TEMP, UNIT_DN,
                                       engdata.precision_coupler_temp);
    COPY_FROM_HDF_VALUE(l1aHdf, param, RCV_PROTECT_SW_TEMP, UNIT_DN,
                                       engdata.rcv_protect_sw_temp);
    COPY_FROM_HDF_VALUE(l1aHdf, param, BEAM_SELECT_SW_TEMP, UNIT_DN,
                                       engdata.beam_select_sw_temp);
    COPY_FROM_HDF_VALUE(l1aHdf, param, RECEIVER_TEMP, UNIT_DN,
                                       engdata.receiver_temp);
    return;

} // L1AFrame::UnpackL1AEngData

void
L1AFrame::UnpackL1AEu(
L1AHdf*     l1aHdf)
{
    Parameter* param=0;
    COPY_FROM_HDF_VALUE(l1aHdf, param, PRF_CYCLE_TIME_EU, UNIT_SECONDS,
                                       in_eu.prf_cycle_time_eu);
    COPY_FROM_HDF_VALUE(l1aHdf, param, RANGE_GATE_DELAY_INNER, UNIT_SECONDS,
                                       in_eu.range_gate_delay_inner);
    COPY_FROM_HDF_VALUE(l1aHdf, param, RANGE_GATE_DELAY_OUTER, UNIT_SECONDS,
                                       in_eu.range_gate_delay_outer);
    COPY_FROM_HDF_VALUE(l1aHdf, param, RANGE_GATE_WIDTH_INNER, UNIT_SECONDS,
                                       in_eu.range_gate_width_inner);
    COPY_FROM_HDF_VALUE(l1aHdf, param, RANGE_GATE_WIDTH_OUTER, UNIT_SECONDS,
                                       in_eu.range_gate_width_outer);
    COPY_FROM_HDF_VALUE(l1aHdf, param, TRANSMIT_PULSE_WIDTH, UNIT_SECONDS,
                                       in_eu.transmit_pulse_width);
    char ucharDN;
    COPY_FROM_HDF_VALUE(l1aHdf, param, TRUE_CAL_PULSE_POS, UNIT_DN, ucharDN);
    in_eu.true_cal_pulse_pos = (int) ucharDN;

    COPY_FROM_HDF_VALUE(l1aHdf, param, TRANSMIT_POWER_INNER, UNIT_DBM,
                                       in_eu.transmit_power_inner);
    COPY_FROM_HDF_VALUE(l1aHdf, param, TRANSMIT_POWER_OUTER, UNIT_DBM,
                                       in_eu.transmit_power_outer);
    COPY_FROM_HDF_VALUE(l1aHdf, param, PRECISION_COUPLER_TEMP_EU,
                       UNIT_DEGREES_C, in_eu.precision_coupler_temp_eu);
    COPY_FROM_HDF_VALUE(l1aHdf, param, RCV_PROTECT_SW_TEMP_EU,
                       UNIT_DEGREES_C, in_eu.rcv_protect_sw_temp_eu);
    COPY_FROM_HDF_VALUE(l1aHdf, param, BEAM_SELECT_SW_TEMP_EU,
                       UNIT_DEGREES_C, in_eu.beam_select_sw_temp_eu);
    COPY_FROM_HDF_VALUE(l1aHdf, param, RECEIVER_TEMP_EU,
                       UNIT_DEGREES_C, in_eu.receiver_temp_eu);

    return;

} // L1AFrame::UnpackL1AEu
