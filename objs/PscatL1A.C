//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_pscatl1a_c[] =
    "@(#) $Id$";

#include <memory.h>
#include <malloc.h>
#include "Pscat.h"
#include "PscatL1A.h"
#include "Constants.h"
#include "Misc.h"

#define GET_L1A_FIRST_PULSE(x) ((x & 0x00000004) >> 2)

//===============//
// PscatL1AFrame //
//===============//

PscatL1AFrame::PscatL1AFrame()
:   time(0), instrumentTicks(0), orbitTicks(0), orbitStep(0),
    priOfOrbitStepChange(255), gcAltitude(0.0), gcLongitude(0.0),
    gcLatitude(0.0), gcX(0.0), gcY(0.0), gcZ(0.0), velX(0.0), velY(0.0),
    velZ(0.0), calPosition(0), loopbackSlices(NULL), loopbackNoise(0),
    loadSlices(NULL), loadNoise(0), antennaPosition(NULL), eventId(NULL),
    copol(NULL), corr(NULL), spotNoise(NULL), frame_inst_status(0),
    antennaCyclesPerFrame(0), spotsPerFrame(0), slicesPerSpot(0),
    slicesPerFrame(0)
{
    return;
}

PscatL1AFrame::~PscatL1AFrame()
{
    Deallocate();
    return;
}

//--------------------//
// PscatL1A::Allocate //
//--------------------//

int
PscatL1AFrame::Allocate(
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

    //-----------------//
    // allocate events //
    //-----------------//

    eventId = (unsigned char *)malloc(spotsPerFrame * sizeof(unsigned char));
    if (eventId == NULL)
        return(0);

    //-------------------------------//
    // allocate science measurements //
    //-------------------------------//

    copol = (unsigned int *)malloc(slicesPerFrame * sizeof(unsigned int));
    if (copol == NULL)
        return(0);

    corr = (float *)malloc(slicesPerFrame * sizeof(unsigned int));
    if (corr == NULL)
        return(0);

    spotNoise = (unsigned int *)malloc(spotsPerFrame * sizeof(unsigned int));
    if (spotNoise == NULL)
        return(0);

    return(1);
}

//---------------------------//
// PscatL1AFrame::Deallocate //
//---------------------------//

int
PscatL1AFrame::Deallocate()
{
    if (antennaPosition)
    {
        free(antennaPosition);
        antennaPosition = NULL;
    }
    if (loopbackSlices)
    {
        free(loopbackSlices);
        loopbackSlices = NULL;
    }
    if (loadSlices)
    {
        free(loadSlices);
        loadSlices = NULL;
    }
    if (eventId)
    {
        free(eventId);
        eventId = NULL;
    }
    if (copol)
    {
        free(copol);
        copol = NULL;
    }
    if (corr)
    {
        free(corr);
        corr = NULL;
    }
    if (spotNoise)
    {
        free(spotNoise);
        spotNoise = NULL;
    }
    antennaCyclesPerFrame = 0;
    spotsPerFrame = 0;
    slicesPerSpot = 0;
    slicesPerFrame = 0;

    return(1);
}

//--------------------------//
// PscatL1AFrame::FrameSize //
//--------------------------//

int
PscatL1AFrame::FrameSize()
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
    size += sizeof(unsigned char);  // cal position
    size += sizeof(unsigned int) * slicesPerSpot;  // loopback slices
    size += sizeof(unsigned int);                  // loopback noise
    size += sizeof(unsigned int) * slicesPerSpot;  // load slices
    size += sizeof(unsigned int);                  // load noise
    size += sizeof(unsigned short) * spotsPerFrame;  // antenna position
    size += sizeof(unsigned char) * spotsPerFrame;  // event
    size += sizeof(float) * slicesPerFrame;  // copol data
    size += sizeof(float) * slicesPerFrame;  // corr data
    size += sizeof(float) * spotsPerFrame;   // spot noise
    size += sizeof(unsigned int);   // instrument status

    return(size);
}

//---------------------//
// PscatL1AFrame::Pack //
//---------------------//

int
PscatL1AFrame::Pack(
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

    size = sizeof(unsigned char) * spotsPerFrame;
    memcpy((void *)(buffer + idx), (void *)eventId, size);
    idx += size;

    size = sizeof(unsigned int) * slicesPerFrame;
    memcpy((void *)(buffer + idx), (void *)copol, size);
    idx += size;

    size = sizeof(float) * slicesPerFrame;
    memcpy((void *)(buffer + idx), (void *)corr, size);
    idx += size;

    size = sizeof(float) * spotsPerFrame;
    memcpy((void *)(buffer + idx), (void *)spotNoise, size);
    idx += size;

    size = sizeof(unsigned int);
    memcpy((void *)(buffer + idx), (void *)&frame_inst_status, size);
    idx += size;

    return(idx);
}

//-----------------------//
// PscatL1AFrame::Unpack //
//-----------------------//

int
PscatL1AFrame::Unpack(
    char*  buffer)
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

    size = sizeof(unsigned char) * spotsPerFrame;
    memcpy((void *)eventId, (void *)(buffer + idx), size);
    idx += size;

    size = sizeof(unsigned int) * slicesPerFrame;
    memcpy((void *)copol, (void *)(buffer + idx), size);
    idx += size;

    size = sizeof(float) * slicesPerFrame;
    memcpy((void *)corr, (void *)(buffer + idx), size);
    idx += size;

    size = sizeof(float) * spotsPerFrame;
    memcpy((void *)spotNoise, (void *)(buffer + idx), size);
    idx += size;

    size = sizeof(unsigned int);
    memcpy((void *)&frame_inst_status, (void *)(buffer + idx), size);
    idx += size;

    return(idx);
}

//---------------------------//
// PscatL1AFrame::WriteAscii //
//---------------------------//

int
PscatL1AFrame::WriteAscii(
    FILE*  ofp)
{
    fprintf(ofp, "\n");
    fprintf(ofp, "*** Frame ***\n\n");
    fprintf(ofp, "Time: %g   Instrument Ticks: %d   Orbit Ticks: %d\n",
        time, instrumentTicks, orbitTicks);
    fprintf(ofp, "Orbit Step: %d   Pri Of Orbit Step Change: %d\n",
        orbitStep, priOfOrbitStepChange);
    fprintf(ofp, "GC Altitude: %g   GC Longitude: %g   GC Latitude: %g\n",
        gcAltitude, gcLongitude, gcLatitude);
    fprintf(ofp, "X: %g   Y: %g   Z: %g\n", gcX, gcY, gcZ);
    fprintf(ofp, "VelX: %g   VelY: %g   VelZ: %g\n", velX, velY, velZ);
    fprintf(ofp, "Roll: %g   Pitch: %g   Yaw: %g\n", attitude.GetRoll() * dtr,
        attitude.GetPitch() * dtr, attitude.GetYaw() * dtr);
    fprintf(ofp, "Cal Position: %d\n", calPosition);

    fprintf(ofp, "--- Calibration Data ---\n");
    fprintf(ofp, "--- Science Data ---\n");
    for (int spot_idx = 0; spot_idx < spotsPerFrame; spot_idx++)
    {
        int spot_slice_offset = spot_idx * slicesPerSpot;
        fprintf(ofp, "Spot %d (%s) AntennaPos: %d SpotNoise: %d\n", spot_idx,
		pscat_event_map[(int)eventId[spot_idx]],
		antennaPosition[spot_idx],spotNoise[spot_idx]);
        for (int slice_idx = 0; slice_idx < slicesPerSpot; slice_idx++)
        {
            fprintf(ofp, "  %d %g\n", copol[spot_slice_offset + slice_idx],
                corr[spot_slice_offset + slice_idx]);
        }
    }
    
    fprintf(ofp, "\n");
    return(1);
}

//==========//
// PscatL1A //
//==========//

PscatL1A::PscatL1A()
:   buffer(NULL), bufferSize(0), _status(OK)
{
    return;
}

PscatL1A::~PscatL1A()
{
    DeallocateBuffer();
    return;
}

//--------------------------//
// PscatL1A::AllocateBuffer //
//--------------------------//

int
PscatL1A::AllocateBuffer()
{
    bufferSize = frame.FrameSize();
    buffer = (char *)malloc(bufferSize);
    if (buffer == NULL)
    {
        bufferSize = 0;
        return(0);
    }
    return(1);
}

//----------------------------//
// PscatL1A::DeallocateBuffer //
//----------------------------//

int
PscatL1A::DeallocateBuffer()
{
    if (buffer)
    {
        free(buffer);
        buffer = 0;
    }
    bufferSize = 0;
    return(1);
}

//-----------------------//
// PscatL1A::ReadDataRec //
//-----------------------//

int
PscatL1A::ReadDataRec()
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

//------------------------//
// PscatL1A::WriteDataRec //
//------------------------//

int
PscatL1A::WriteDataRec()
{
    return(Write(buffer, bufferSize));
}

//-----------------------------//
// PscatL1A::WriteDataRecAscii //
//-----------------------------//

int
PscatL1A::WriteDataRecAscii(
    FILE*  fp)
{
    FILE* use_fp;
    if (fp)
        use_fp = fp;
    else if (_outputFp)
        use_fp = _outputFp;
    else
        return(0);

    if (! frame.WriteAscii(use_fp))
        return(0);

    return(1);
}
