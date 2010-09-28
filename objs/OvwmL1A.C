//==============================================================//
// Copyright (C) 1997-2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_l1a_c[] =
    "@(#) $Id$";

#include <memory.h>
#include <malloc.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "OvwmL1A.h"
#include "Misc.h"

using namespace std;

#define GET_L1A_FIRST_PULSE(x) ((x & 0x00000004) >> 2)

//===============//
// OvwmL1AFrame //
//===============//

OvwmL1AFrame::OvwmL1AFrame()
:   time(0),
    spotTime(NULL), spotScanAngle(NULL), spotBeamIdx(NULL),
    gcAltitude(0.0), gcLongitude(0.0),
    gcLatitude(0.0), gcX(0.0), gcY(0.0), gcZ(0.0), velX(0.0), velY(0.0),
    velZ(0.0),
    loopbackSpots(NULL), loopbackNoise(0),
    loadSpots(NULL), loadNoise(0),
    science(NULL), spotNoise(NULL), dataCountSpots(NULL),
    spotsPerFrame(0), maxMeasPerSpot(0)
{
    return;
}

OvwmL1AFrame::~OvwmL1AFrame()
{
    Deallocate();
    return;
}

//-------------------//
// OvwmL1A::Allocate //
//-------------------//
    
int
OvwmL1AFrame::Allocate(
    int  number_of_beams,
    int  antenna_cycles_per_frame,
    int  max_meas_per_spot)
{
    spotsPerFrame = number_of_beams;

    //---------------------------//
    // allocate cal measurements //
    //---------------------------//

    loopbackSpots = (float *)malloc(spotsPerFrame*sizeof(float));
    if (loopbackSpots == NULL)
    {
        return(0);
    }
    loadSpots = (float *)malloc(spotsPerFrame*sizeof(float));
    if (loadSpots == NULL)
    {
        return(0);
    }

    //-------------------------------//
    // allocate science measurements //
    //-------------------------------//

    science = (float *)malloc(spotsPerFrame*max_meas_per_spot*sizeof(float));
    if (science == NULL)
        return(0);

    spotNoise = (float *)malloc(spotsPerFrame * sizeof(float));
    if (spotNoise == NULL)
        return(0);

    dataCountSpots = (int *)malloc(spotsPerFrame * sizeof(int));
    if (dataCountSpots == NULL)
        return(0);

    spotTime = (double *)malloc(spotsPerFrame * sizeof(double));
    if (spotTime == NULL)
        return(0);

    spotScanAngle = (float *)malloc(spotsPerFrame * sizeof(float));
    if (spotScanAngle == NULL)
        return(0);

    spotBeamIdx = (int *)malloc(spotsPerFrame * sizeof(int));
    if (spotBeamIdx == NULL)
        return(0);

    return(1);
}

//--------------------------//
// OvwmL1AFrame::Deallocate //
//--------------------------//

int
OvwmL1AFrame::Deallocate()
{
    if (loopbackSpots)
    {
        free(loopbackSpots);
        loopbackSpots = NULL;
    }
    if (loadSpots)
    {
        free(loadSpots);
        loadSpots = NULL;
    }
    if (science)
    {
        free(science);
        science = NULL;
    }
    if (spotNoise)
    {
        free(spotNoise);
        spotNoise = NULL;
    }
    if (dataCountSpots)
    {
        free(dataCountSpots);
        dataCountSpots = NULL;
    }
    if (spotTime)
    {
        free(spotTime);
        spotTime = NULL;
    }
    if (spotScanAngle)
    {
        free(spotScanAngle);
        spotScanAngle = NULL;
    }
    if (spotBeamIdx)
    {
        free(spotBeamIdx);
        spotBeamIdx = NULL;
    }
    spotsPerFrame = 0;

    return(1);

}

//-------------------------//
// OvwmL1AFrame::FrameSize //
//-------------------------//

int
OvwmL1AFrame::FrameSize()
{
    int size = 0;
    size += sizeof(double);                 // frame time
    size += sizeof(double) * spotsPerFrame; // spot time
    size += sizeof(float) * spotsPerFrame;  // spot scan angle
    size += sizeof(int) * spotsPerFrame;    // spot beam idx

    //size += sizeof(unsigned int);   // instrument ticks
    //size += sizeof(unsigned int);   // orbit ticks
    //size += sizeof(unsigned char);  // orbit step
    //size += sizeof(unsigned char);  // pri of orbit step change

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

    //size += sizeof(unsigned char);  // cal position

    size += sizeof(float) * spotsPerFrame;  // loopback slices
    size += sizeof(float);                  // loopback noise
    size += sizeof(float) * spotsPerFrame;  // load slices
    size += sizeof(float);                  // load noise

    //size += sizeof(unsigned short) * spotsPerFrame;  // antenna position
    //size += sizeof(unsigned char) * spotsPerFrame;  // event

    size += sizeof(float) * spotsPerFrame * maxMeasPerSpot; // science data
    size += sizeof(float) * spotsPerFrame;                  // spot noise
    size += sizeof(int) * spotsPerFrame;                    // data counts of spots

    //size += sizeof(unsigned int);   // instrument status

    return(size);
}

//--------------------//
// OvwmL1AFrame::Pack //
//--------------------//
    
int 
OvwmL1AFrame::Pack(
    char*  buffer)
{
    int idx = 0;
    int size;

    size = sizeof(double);
    memcpy((void *)(buffer + idx), (void *)&time, size);
    idx += size;

    size = sizeof(double) * spotsPerFrame;
    memcpy((void *)(buffer + idx), (void *)spotTime, size);
    idx += size;

    size = sizeof(float) * spotsPerFrame;
    memcpy((void *)(buffer + idx), (void *)spotScanAngle, size);
    idx += size;

    size = sizeof(int) * spotsPerFrame;
    memcpy((void *)(buffer + idx), (void *)spotBeamIdx, size);
    idx += size;

    //size = sizeof(unsigned int);
    //memcpy((void *)(buffer + idx), (void *)&instrumentTicks, size);
    //idx += size;

    //memcpy((void *)(buffer + idx), (void *)&orbitTicks, size);
    //idx += size;
    
    //size = sizeof(unsigned char);
    //memcpy((void *)(buffer + idx), (void *)&orbitStep, size);
    //idx += size;
    
    //memcpy((void *)(buffer + idx), (void *)&priOfOrbitStepChange, size);
    //idx += size;
    
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

    //size = sizeof(unsigned char);
    //memcpy((void *)(buffer + idx), (void *)&calPosition, size);
    //idx += size;

    size = sizeof(float) * spotsPerFrame;
    memcpy((void *)(buffer + idx), (void *)loopbackSpots, size);
    idx += size;

    size = sizeof(float);
    memcpy((void *)(buffer + idx), (void *)&loopbackNoise, size);
    idx += size;

    size = sizeof(float) * spotsPerFrame;
    memcpy((void *)(buffer + idx), (void *)loadSpots, size);
    idx += size;

    size = sizeof(float);
    memcpy((void *)(buffer + idx), (void *)&loadNoise, size);
    idx += size;

    size = sizeof(float) * spotsPerFrame * maxMeasPerSpot;
    memcpy((void *)(buffer + idx), (void *)science, size);
    idx += size;

    size = sizeof(float) * spotsPerFrame;
    memcpy((void *)(buffer + idx), (void *)spotNoise, size);
    idx += size;

    size = sizeof(int) * spotsPerFrame;
    memcpy((void *)(buffer + idx), (void *)dataCountSpots, size);
    idx += size;

    return(idx);
}

//----------------------//
// OvwmL1AFrame::Unpack //
//----------------------//
    
int 
OvwmL1AFrame::Unpack(
    char*  buffer)
{
    int idx = 0;
    int size;

    size = sizeof(double);
    memcpy((void *)&time, (void *)(buffer + idx), size);
    idx += size;

    size = sizeof(double) * spotsPerFrame;
    memcpy((void *)spotTime, (void *)(buffer + idx), size);
    idx += size;

    size = sizeof(float) * spotsPerFrame;
    memcpy((void *)spotScanAngle, (void *)(buffer + idx), size);
    idx += size;

    size = sizeof(int) * spotsPerFrame;
    memcpy((void *)spotBeamIdx, (void *)(buffer + idx), size);
    idx += size;

    //size = sizeof(unsigned int);
    //memcpy((void *)&instrumentTicks, (void *)(buffer + idx), size);
    //idx += size;

    //memcpy((void *)&orbitTicks, (void *)(buffer + idx), size);
    //idx += size;
    
    //size = sizeof(unsigned char);
    //memcpy((void *)&orbitStep, (void *)(buffer + idx), size);
    //idx += size;
    
    //memcpy((void *)&priOfOrbitStepChange, (void *)(buffer + idx), size);
    //idx += size;
    
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

    //size = sizeof(unsigned char);
    //memcpy((void *)&calPosition, (void *)(buffer + idx), size);
    //idx += size;

    size = sizeof(float) * spotsPerFrame;
    memcpy((void *)loopbackSpots, (void *)(buffer + idx), size);
    idx += size;

    size = sizeof(float);
    memcpy((void *)&loopbackNoise, (void *)(buffer + idx), size);
    idx += size;

    size = sizeof(float) * spotsPerFrame;
    memcpy((void *)loadSpots, (void *)(buffer + idx), size);
    idx += size;

    size = sizeof(float); 
    memcpy((void *)&loadNoise, (void *)(buffer + idx), size);
    idx += size;

    //size = sizeof(unsigned short) * spotsPerFrame;
    //memcpy((void *)antennaPosition, (void *)(buffer + idx), size);
    //idx += size; 
    
    size = sizeof(float) * spotsPerFrame * maxMeasPerSpot;
    memcpy((void *)science, (void *)(buffer + idx), size);
    idx += size;

    size = sizeof(float) * spotsPerFrame;
    memcpy((void *)spotNoise, (void *)(buffer + idx), size);
    idx += size;

    size = sizeof(int) * spotsPerFrame;
    memcpy((void *)dataCountSpots, (void *)(buffer + idx), size);
    idx += size;

    //size = sizeof(unsigned int);
    //memcpy((void *)&frame_inst_status, (void *)(buffer + idx), size);
    //idx += size;
    
    return(idx);
}

//--------------------------//
// OvwmL1AFrame::WriteAscii //
//--------------------------//
    
int 
OvwmL1AFrame::WriteAscii(
    FILE*  ofp)
{   
    fprintf(ofp, "\n");
    fprintf(ofp, "*** Frame ***\n\n");

    //fprintf(ofp, "Time: %g   Instrument Ticks: %d   Orbit Ticks: %d\n",
    //    time, instrumentTicks, orbitTicks);

    fprintf(ofp, "Time: %18.6lf \n", time);

    //fprintf(ofp, "Orbit Step: %d   Pri Of Orbit Step Change: %d\n",
    //    orbitStep, priOfOrbitStepChange);

    fprintf(ofp, "GC Altitude: %g   GC Longitude: %g   GC Latitude: %g\n",
        gcAltitude, gcLongitude, gcLatitude);
    fprintf(ofp, "X: %g   Y: %g   Z: %g\n", gcX, gcY, gcZ);
    fprintf(ofp, "VelX: %g   VelY: %g   VelZ: %g\n", velX, velY, velZ);
    fprintf(ofp, "Roll: %g   Pitch: %g   Yaw: %g\n", attitude.GetRoll() * rtd,
        attitude.GetPitch() * rtd, attitude.GetYaw() * rtd);

    //fprintf(ofp, "Cal Position: %d\n", calPosition);

    fprintf(ofp, "--- Calibration Data ---\n");
    fprintf(ofp, "loop back: %g, noise: %g\n", loopbackSpots[0], loopbackNoise);
    fprintf(ofp, "load: %g, noise: %g\n", loadSpots[0], loadNoise);

    fprintf(ofp, "--- Science Data ---\n");
    for (int spot_idx = 0; spot_idx < spotsPerFrame; spot_idx++)
    {

        fprintf(ofp, "Spot Time: %12.6lf\n", spotTime[spot_idx]);
        fprintf(ofp, "Spot Scan Angle: %12.6f\n", spotScanAngle[spot_idx]);
        fprintf(ofp, "Spot Beam Idx: %2d\n", spotBeamIdx[spot_idx]);

        int spot_slice_offset = spot_idx * maxMeasPerSpot;

        //fprintf(ofp, "Spot %d (%s) AntennaPos: %d SpotNoise: %d\n", spot_idx,
        //pscat_event_map[(int)eventId[spot_idx]],
        //antennaPosition[spot_idx],spotNoise[spot_idx]);

        fprintf(ofp, "Spot: %d, meas count: %d, spotNoise: %g\n", spot_idx,
                     dataCountSpots[spot_idx], spotNoise[spot_idx]);

        //fprintf(ofp, "meas count: %d, loop back: %d, load: %d\n", dataCountSpots[spot_idx],
        //             loopbackSpots[spot_idx], loadSpots[spot_idx]);

        for (int meas_idx = 0; meas_idx < dataCountSpots[spot_idx]; meas_idx++)
        {
            fprintf(ofp, "  %g\n", science[spot_slice_offset + meas_idx]);
        }
    }

    fprintf(ofp, "\n");
    return(1);
}

//=========//
// OvwmL1A //
//=========//

OvwmL1A::OvwmL1A()
:    buffer(NULL), bufferSize(0), _status(OK)
{
    return;
}

OvwmL1A::~OvwmL1A()
{
    DeallocateBuffer();
    return;
}

//-------------------------//
// OvwmL1A::AllocateBuffer //
//-------------------------//

int
OvwmL1A::AllocateBuffer()
{
    bufferSize = frame.FrameSize();
    buffer = (char *)malloc(bufferSize);
    if (buffer == NULL)
    {
        bufferSize = 0;
        return(0);
    }
    //gsBuffer = (char*) malloc(GS_L1A_FRAME_SIZE);
    //if (gsBuffer == NULL) return 0;
    return(1);
}

//---------------------------//
// OvwmL1A::DeallocateBuffer //
//---------------------------//

int
OvwmL1A::DeallocateBuffer()
{
    if (buffer)
    {
        free(buffer);
        buffer = 0;
    }
    bufferSize = 0;
    //if (gsBuffer)
    //{
    //    free(gsBuffer);
    //    gsBuffer = 0;
    //}
    return(1);
}

//----------------------//
// OvwmL1A::ReadDataRec //
//----------------------//

int
OvwmL1A::ReadDataRec()
{
    if (! Read(buffer, bufferSize))
    {
        if (EndOfFile())
        {
            // end of file, leave status alone (typically status is OK)
            return(0);
        }
        else
        {
            // an error occurred
            _status = ERROR_READING_FRAME;
            return(0);
        }
    }
    return(1);
}

//-----------------------//
// OvwmL1A::WriteDataRec //
//-----------------------//

int
OvwmL1A::WriteDataRec()
{
    return(Write(buffer, bufferSize));
}

//----------------------------//
// OvwmL1A::WriteDataRecAscii //
//----------------------------//

int
OvwmL1A::WriteDataRecAscii(
    FILE*  ofp)
{
    if (ofp == NULL || ! frame.WriteAscii(ofp))
        return(0);

    return(1);
}

//--------------------//
// L1A::ReadGSDataRec //
//--------------------//

/*
int
L1A::ReadGSDataRec()
{
    if (! Read(gsBuffer, GS_L1A_FRAME_SIZE))
    {
        if (EndOfFile())
        {
            // end of file, leave status alone (typically status is OK)
            return(0);
        }
        else
        {
            // an error occurred
            _status = ERROR_READING_FRAME;
            return(0);
        }
    }
    return(1);
}
*/

//--------------------------//
// L1A::WriteGSDataRecAscii //
//--------------------------//

/*
int
L1A::WriteGSDataRecAscii(
    FILE*  ofp)
{
    if (ofp == NULL)
        return(0);

    return(gsFrame.WriteAscii(ofp));
}
*/

//---------------------//
// L1A::WriteGSDataRec //
//---------------------//

/*
int
L1A::WriteGSDataRec()
{
    if (_outputFp == NULL)
        return(0);
    FillGSFrame();
    if (gsFrame.Pack(gsBuffer) == 0)
        return(0);
    return(Write(gsBuffer, GS_L1A_FRAME_SIZE));
}
*/

//------------------//
// L1A::FillGSFrame //
//------------------//

/*
int
L1A::FillGSFrame()
{
    //----------------------------------------------//
    // Transfer data from standard frame to gsFrame //
    //----------------------------------------------//

    GSL1APcd* in_pcdP = &(gsFrame.in_pcd);
    (void)memcpy(in_pcdP->frame_time, frame.frame_time, 24);
    in_pcdP->time = frame.time;
    // assumes fractional part of instrument_time (5th byte) is zero
    in_pcdP->instrument_time = (double)frame.instrumentTicks;
    in_pcdP->orbit_time = (int)frame.orbitTicks;
    in_pcdP->x_pos = frame.gcX*1000;
    in_pcdP->y_pos = frame.gcY*1000;
    in_pcdP->z_pos = frame.gcZ*1000;
    in_pcdP->x_vel = frame.velX*1000;
    in_pcdP->y_vel = frame.velY*1000;
    in_pcdP->z_vel = frame.velZ*1000;
    in_pcdP->roll = frame.attitude.GetRoll();
    in_pcdP->pitch = frame.attitude.GetPitch();
    in_pcdP->yaw = frame.attitude.GetYaw();

    (void)memcpy(&(gsFrame.status), &(frame.status), sizeof(GSL1AStatus));

    (void)memcpy(&(gsFrame.engdata), &(frame.engdata), sizeof(GSL1AEngData));

    (void)memset(&(gsFrame.SESerrflags), 0, 13);
    (void)memset(&(gsFrame.memro), 0, 26);
    (void)memset(&(gsFrame.pcd), 0, 32);

    (void)memcpy(&(gsFrame.in_eu), &(frame.in_eu), sizeof(GSL1AEu));

    (void)memcpy(gsFrame.in_science.antenna_position,
                 frame.antennaPosition, sizeof(short)*frame.spotsPerFrame);

    // Cal measurements need to be converted to floats (but this isn't used!)
    for (int i = 0; i < frame.slicesPerSpot; i++)
    {  // convert floats to ints
      gsFrame.in_science.loop_back_cal_A_power[i]=(int)frame.loopbackSlices[i];
      gsFrame.in_science.loop_back_cal_B_power[i]=(int)frame.loopbackSlices[i];
      gsFrame.in_science.load_cal_A_power[i]=(int)frame.loadSlices[i];
      gsFrame.in_science.load_cal_B_power[i]=(int)frame.loadSlices[i];
    }
    gsFrame.in_science.loop_back_cal_noise = (int)frame.loopbackNoise;
    gsFrame.in_science.load_cal_noise = (int)frame.loadNoise;

*/
/*
    (void)memcpy(gsFrame.in_science.loop_back_cal_A_power,
                 frame.loopbackSlices, sizeof(float)*frame.slicesPerSpot);
    (void)memcpy(gsFrame.in_science.loop_back_cal_B_power,
                 frame.loopbackSlices, sizeof(float)*frame.slicesPerSpot);
    (void)memcpy(&gsFrame.in_science.loop_back_cal_noise,
                 &frame.loopbackNoise, sizeof(float));
    (void)memcpy(gsFrame.in_science.load_cal_A_power,
                 frame.loadSlices, sizeof(float)*frame.slicesPerSpot);
    (void)memcpy(gsFrame.in_science.load_cal_B_power,
                 frame.loadSlices, sizeof(float)*frame.slicesPerSpot);
    (void)memcpy(&gsFrame.in_science.load_cal_noise,
                 &frame.loadNoise, sizeof(float));
*/
/*
    (void)memcpy(gsFrame.in_science.power_dn,
                 frame.science, sizeof(unsigned int)*frame.slicesPerFrame);
    (void)memcpy(gsFrame.in_science.noise_dn,
                 frame.spotNoise, sizeof(unsigned int)*frame.spotsPerFrame);

*/
/*
    for (int i = 0; i < frame.slicesPerFrame; i++)
    {  // convert floats to ints
      *(gsFrame.in_science.power_dn[0] + i) = (int)frame.science[i];
    }
    for (int i = 0; i < frame.spotsPerFrame; i++)
    {  // convert floats to ints
      gsFrame.in_science.noise_dn[i] = (int)frame.spotNoise[i];
    }
*/
/*
    // la1_frame_inst_status
    (void)memcpy(&(gsFrame.l1a_frame_inst_status),
                   &(frame.frame_inst_status), sizeof(int));

    // l1a_frame_err_status
    (void)memcpy(&(gsFrame.l1a_frame_err_status),
                   &(frame.frame_err_status), sizeof(int));

    // l1a_frame_qual_flag
    (void)memcpy(&(gsFrame.l1a_frame_qual_flag),
                   &(frame.frame_qual_flag), sizeof(short));

    // l1a_pulse_qual_flag[13]
    (void)memcpy(&(gsFrame.l1a_pulse_qual_flag), &(frame.pulse_qual_flag), 13);

    return(1);
} // L1A::FillGSFrame
*/

//-----------------------------//
// L1A::OpenCalPulseForWriting //
//-----------------------------//

/*
int
L1A::OpenCalPulseForWriting(
    const char*  filename)
{
    if (filename == NULL)
        return(0);
    _calPulseFP = fopen(filename, "w");
    if (_calPulseFP == NULL)
        return(0);
    return(1);
}
*/

//-----------------------------//
// L1A::OpenCalPulseForReading //
//-----------------------------//

/*
int
L1A::OpenCalPulseForReading(
    const char*  filename)
{
    if (filename == NULL)
        return(0);
    _calPulseFP = fopen(filename, "r");
    if (_calPulseFP == NULL)
        return(0);
    return(1);
}
*/

//------------------------//
// L1A::CloseCalPulseFile //
//------------------------//

/*
int
L1A::CloseCalPulseFile()
{
    if (_calPulseFP == NULL)
        return(0);
    if (! fclose(_calPulseFP))
    {
        _calPulseFP = NULL;
        return(0);
    }
    return(1);
}
*/

//-------------------------//
// L1A::WriteGSCalPulseRec //
//-------------------------//
// totally rewritten by JNH 2001.03.29 (untested too!)

/*
int
L1A::WriteGSCalPulseRec()
{
    if (_calPulseFP == NULL)
        return(0);

    L1A_Calibration_Pulse_Type cpr;

    // frame time
    cpr.frame_time_cal_secs = frame.time;

    // true cal pulse position
    cpr.true_cal_pulse_pos = (char)frame.in_eu.true_cal_pulse_pos;

    // beam identifier (I don't quite understand the code that follows)
    if (IS_EVEN(frame.in_eu.true_cal_pulse_pos))
    {
        // different
        if (GET_L1A_FIRST_PULSE(frame.frame_inst_status) == 0)
            cpr.beam_identifier = 1;
        else
            cpr.beam_identifier = 0;
    }
    else
    {
        // same
        if (GET_L1A_FIRST_PULSE(frame.frame_inst_status) == 0)
            cpr.beam_identifier = 0;
        else
            cpr.beam_identifier = 1;
    }

    // loop back cal slice powers
    // load cal slice powers
    for (int i = 0; i < SLICES_PER_PULSE; i++)
    {
        cpr.loop_back_cal_power[i] = *(frame.loopbackSlices + i);
        cpr.load_cal_power[i] = *(frame.loadSlices + i);
    }

    // loop back cal noise
    // load cal noise
    cpr.loop_back_cal_noise = frame.loopbackNoise;
    cpr.load_cal_noise = frame.loadNoise;

    // we don't have the rotary joint temp, i'll just use the coupler
    cpr.rj_temp_eu = frame.in_eu.precision_coupler_temp_eu;
    cpr.precision_coupler_temp_eu = frame.in_eu.precision_coupler_temp_eu;
    cpr.rcv_protect_sw_temp_eu = frame.in_eu.rcv_protect_sw_temp_eu;
    cpr.beam_select_sw_temp_eu = frame.in_eu.beam_select_sw_temp_eu;
    cpr.receiver_temp_eu = frame.in_eu.receiver_temp_eu;
    cpr.transmit_power_inner = frame.in_eu.transmit_power_inner;
    cpr.transmit_power_outer = frame.in_eu.transmit_power_outer;
    cpr.frame_inst_status = frame.frame_inst_status;
    cpr.frame_err_status = frame.frame_err_status;
    cpr.frame_qual_flag = frame.frame_qual_flag;

    return(fwrite(&cpr, sizeof(L1A_Calibration_Pulse_Type), 1, _calPulseFP));
}
*/

//------------------------//
// L1A::ReadGSCalPulseRec //
//------------------------//
// This may actually work.

/*
int
L1A::ReadGSCalPulseRec()
{
    if (_calPulseFP == NULL)
        return(0);

    L1A_Calibration_Pulse_Type cpr;
    fread(&cpr, sizeof(L1A_Calibration_Pulse_Type), 1, _calPulseFP);

    // frame time
    frame.time = cpr.frame_time_cal_secs;

    // true cal pulse position
    frame.in_eu.true_cal_pulse_pos = cpr.true_cal_pulse_pos;

    // I don't think the beam identifier is needed in our frames
    // We just use the frame instrument status

    // loop back cal slice powers
    // load cal slice powers
    for (int i = 0; i < SLICES_PER_PULSE; i++)
    {
        *(frame.loopbackSlices + i) = cpr.loop_back_cal_power[i];
        *(frame.loadSlices + i) = cpr.load_cal_power[i];
    }

    // loop back cal noise
    // load cal noise
    frame.loopbackNoise = cpr.loop_back_cal_noise;
    frame.loadNoise = cpr.load_cal_noise;

    // we don't have the rotary joint temp, i'll just use the coupler
    frame.in_eu.precision_coupler_temp_eu = cpr.rj_temp_eu;
    frame.in_eu.precision_coupler_temp_eu = cpr.precision_coupler_temp_eu;
    frame.in_eu.rcv_protect_sw_temp_eu = frame.in_eu.rcv_protect_sw_temp_eu;
    frame.in_eu.beam_select_sw_temp_eu = cpr.beam_select_sw_temp_eu;
    frame.in_eu.receiver_temp_eu = cpr.receiver_temp_eu;
    frame.in_eu.transmit_power_inner = cpr.transmit_power_inner;
    frame.in_eu.transmit_power_outer = cpr.transmit_power_outer;
    frame.frame_inst_status = cpr.frame_inst_status;
    frame.frame_err_status = cpr.frame_err_status;
    frame.frame_qual_flag = cpr.frame_qual_flag;

    return(1);
}
*/

/*
//------------------------//
// L1A::ReadGSCalPulseRec //
//------------------------//
// This poor method is now broken. :-(

int
L1A::ReadGSCalPulseRec(
    FILE*  calfile)
{
    if (calfile == NULL)
        return(0);

    //------------------------------------------------------------------
    // The record size of the cal pulse file is now 150 bytes, without
    // any leading or trailing words (i.e. not F77-unformatted).
    //------------------------------------------------------------------

    int s;
    if ((s = fread(&(frame.time), sizeof(double), 1, calfile)) != 1)
    {
        if (feof(calfile))
            return(0);
        fprintf(stderr, "Error reading calpulse frame time\n");
        return(0);
    }

    unsigned char tpos;
    if (fread(&tpos, sizeof(char), 1, calfile) != 1)
    {
        fprintf(stderr, "Error reading calpulse true_cal_pulse_pos \n");
        return(0);
    }
    frame.in_eu.true_cal_pulse_pos = tpos;

    unsigned char beam_num;
    if (fread(&beam_num, sizeof(char), 1, calfile) != 1)
    {
        fprintf(stderr, "Error reading calpulse beam_num\n");
        return(0);
    }
//    frame.frame_inst_status = need to set correct bit for first pulse;


    if ((s=fread((frame.loopbackSlices), sizeof(unsigned int),
              frame.slicesPerSpot, calfile)) != frame.slicesPerSpot)
    {
        fprintf(stderr, "Error reading calpulse loopbackSlices\n");
        return(0);
    }

    if (fread(&(frame.loopbackNoise), sizeof(int), 1, calfile) != 1)
    {
        fprintf(stderr, "Error reading calpulse loopbackNoise\n");
        return(0);
    }

    if ((s=fread((frame.loadSlices), sizeof(unsigned int),
              frame.slicesPerSpot, calfile)) != frame.slicesPerSpot)
    {
        fprintf(stderr, "Error reading calpulse loadSlices\n");
        return(0);
    }

    if (fread(&(frame.loadNoise), sizeof(int), 1, calfile) != 1)
    {
        fprintf(stderr, "Error reading calpulse loadNoise\n");
        return(0);
    }

    if (fread(&(frame.in_eu.precision_coupler_temp_eu), sizeof(float),
              1, calfile) != 1)
    {
        fprintf(stderr,
            "Error reading calpulse frame precision_coupler_temp\n");
        return(0);
    }
    if (fread(&(frame.in_eu.rcv_protect_sw_temp_eu), sizeof(float),
              1, calfile) != 1)
    {
        fprintf(stderr, "Error reading calpulse rcv_protect_sw_temp\n");
        return(0);
    }
    if (fread(&(frame.in_eu.beam_select_sw_temp_eu), sizeof(float),
              1, calfile) != 1)
    {
        fprintf(stderr, "Error reading calpulse beam_select_sw_temp\n");
        return(0);
    }
    if (fread(&(frame.in_eu.receiver_temp_eu), sizeof(float),
              1, calfile) != 1)
    {
        fprintf(stderr, "Error reading calpulse receiver_temp\n");
        return(0);
    }
    if (fread(&(frame.in_eu.transmit_power_inner), sizeof(float),
              1, calfile) != 1)
    {
        fprintf(stderr, "Error reading calpulse transmit_power_inner\n");
        return(0);
    }
    if (fread(&(frame.in_eu.transmit_power_outer), sizeof(float),
              1, calfile) != 1)
    {
        fprintf(stderr, "Error reading calpulse transmit_power_outer\n");
        return(0);
    }
    if (fread(&(frame.frame_inst_status), sizeof(int),
              1, calfile) != 1)
    {
        fprintf(stderr, "Error reading calpulse frame_inst_status\n");
        return(0);
    }
    if (fread(&(frame.frame_err_status), sizeof(int),
              1, calfile) != 1)
    {
        fprintf(stderr, "Error reading calpulse frame_err_status\n");
        return(0);
    }
    int dummy; // 4 bytes at end are not used.
    if (fread(&(dummy), sizeof(int),
              1, calfile) != 1)
    {
        fprintf(stderr, "Error reading calpulse dummy\n");
        return(0);
    }

    return(1);
}
*/

//------------------------------//
// L1A::WriteGSCalPulseRecAscii //
//------------------------------//

/*
int
L1A::WriteGSCalPulseRecAscii(
    FILE*  ofp)
{
    if (ofp == NULL)
        return(0);

    fprintf(ofp, "Cal Pulse Record:\n");
    char ftime[25];
    (void)memcpy(ftime, frame.frame_time, 24);
    ftime[24] = '\0';
    fprintf(ofp, "frame time string (not in actual record): %s\n", ftime);
    fprintf(ofp, "frame_time_secs = %g\n",frame.time);
    for (int i = 0; i < 12; i++)
    {
        fprintf(ofp, "loopbackSlices[%d] = %d\n", i, frame.loopbackSlices[i]);
    }
    fprintf(ofp, "loopbackNoise = %d\n", frame.loopbackNoise);

    for (int i = 0; i < 12; i++)
    {
        fprintf(ofp, "loadSlices[%d] = %d\n", i, frame.loadSlices[i]);
    }
    fprintf(ofp, "loadNoise = %d\n",frame.loadNoise);
    fprintf(ofp, "precision_coupler_temp_eu = %g\n",
        frame.in_eu.precision_coupler_temp_eu);
    fprintf(ofp, "rcv_protect_sw_temp_eu = %g\n",
        frame.in_eu.rcv_protect_sw_temp_eu);
    fprintf(ofp, "beam_select_sw_temp_eu = %g\n",
        frame.in_eu.beam_select_sw_temp_eu);
    fprintf(ofp, "receiver_temp_eu = %g\n",
        frame.in_eu.receiver_temp_eu);
    fprintf(ofp, "transmit_power_inner = %g\n",
        frame.in_eu.transmit_power_inner);
    fprintf(ofp, "transmit_power_outer = %g\n",
        frame.in_eu.transmit_power_outer);
    fprintf(ofp, "frame_inst_status = %d\n",
        frame.frame_inst_status);
    fprintf(ofp, "frame_err_status = %d\n",
        frame.frame_err_status);
    fprintf(ofp, "true_cal_pulse_pos = %d\n",
        frame.in_eu.true_cal_pulse_pos);

    // Set beam identifier based on true unit offset position and first pulse
    // identity.
    if (IS_EVEN(frame.in_eu.true_cal_pulse_pos))
    {
        // different
        if (GET_L1A_FIRST_PULSE(frame.frame_inst_status) == 0)
            fprintf(ofp, "beam_identifier = %d\n", 1);
        else
            fprintf(ofp, "beam_identifier = %d\n", 0);
    }
    else
    {
        // same
        if (GET_L1A_FIRST_PULSE(frame.frame_inst_status) == 0)
            fprintf(ofp, "beam_identifier = %d\n", 0);
        else
            fprintf(ofp, "beam_identifier = %d\n", 1);
    }
    fprintf(ofp, "\n");

    return(1);
}
*/
