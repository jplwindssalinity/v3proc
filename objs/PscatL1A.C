//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_pscatl1a_c[] =
    "@(#) $Id$";

#include <memory.h>
#include <malloc.h>
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
    loadSlices(NULL), loadNoise(0), antennaPosition(NULL), event(NULL),
    science(NULL), spotNoise(NULL), antennaCyclesPerFrame(0),
    spotsPerFrame(0), slicesPerSpot(0), measPerSlice(0), measPerSpot(0),
    measPerFrame(0)
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
    measPerSlice = 2;
    measPerSpot = measPerSlice * slicesPerSpot;
    measPerFrame = measPerSpot * spotsPerFrame;

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

    event = (unsigned char *)malloc(spotsPerFrame * sizeof(unsigned char));
    if (event == NULL)
        return(0);

    //-------------------------------//
    // allocate science measurements //
    //-------------------------------//

    science = (unsigned int *)malloc(measPerFrame * sizeof(unsigned int));
    if (science == NULL)
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
        free(antennaPosition);
    if (loopbackSlices)
        free(loopbackSlices);
    if (loadSlices)
        free(loadSlices);
    if (event)
        free(event);
    if (science)
        free(science);
    if (spotNoise)
        free(spotNoise);
    antennaPosition = NULL;
    loopbackSlices = NULL;
    loadSlices = NULL;
    event = NULL;
    science = NULL;
    spotNoise = NULL;
    antennaCyclesPerFrame = 0;
    spotsPerFrame = 0;
    slicesPerSpot = 0;
    measPerSlice = 0;
    measPerFrame = 0;
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
    size += sizeof(float) * measPerFrame;  // science data
    size += sizeof(float) * spotsPerFrame;   // spot noise

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
    memcpy((void *)(buffer + idx), (void *)event, size);
    idx += size;

    size = sizeof(unsigned int) * measPerFrame;
    memcpy((void *)(buffer + idx), (void *)science, size);
    idx += size;

    size = sizeof(float) * spotsPerFrame;
    memcpy((void *)(buffer + idx), (void *)spotNoise, size);
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
    memcpy((void *)event, (void *)(buffer + idx), size);
    idx += size;

    size = sizeof(unsigned int) * measPerFrame;
    memcpy((void *)science, (void *)(buffer + idx), size);
    idx += size;

    size = sizeof(float) * spotsPerFrame;
    memcpy((void *)spotNoise, (void *)(buffer + idx), size);
    idx += size;

    return(idx);
}

//---------------------------//
// PscatL1AFrame::WriteAscii //
//---------------------------//

int PscatL1AFrame::WriteAscii(
    FILE*  ofp)
{
    fprintf(ofp,
        "\n########################Frame Info#####################\n\n");
    fprintf(ofp,
        "Time: %g InstrumentTicks: %d OrbitTicks %d PriOfOrbitStepChange %d\n",
        time, instrumentTicks, orbitTicks, (int)priOfOrbitStepChange);
    fprintf(ofp, "GCAlt: %g GCLon: %g GCLat: %g GCX: %g GCY: %g GCZ: %g\n",
        gcAltitude, gcLongitude*rtd, gcLatitude*rtd, gcX, gcY, gcZ);
    fprintf(ofp,
        "VelX: %g VelY: %g VelZ: %g Roll: %g Pitch: %g Yaw: %g\n",
        velX, velY, velZ, attitude.GetRoll()*rtd, attitude.GetPitch()*rtd,
        attitude.GetYaw()*rtd);
    int offset = 0;
    for (int c = 0; c < spotsPerFrame; c++)
    {
        fprintf(ofp,
            "\n    :::::::::::::::: Spot Info :::::::::::::::::::  \n\n");
        fprintf(ofp, "AntennaPos: %d SpotNoise: %d Beam:%d\n",
            (int)antennaPosition[c], spotNoise[c], c%2);
        fprintf(ofp, "E(S+N) Slices(1-%d): ", slicesPerSpot);
        for (int s = 0; s < slicesPerSpot; s++)
        {
            fprintf(ofp, "%d ", science[offset]);
            offset++;
        }
        fprintf(ofp,"\n");
    }
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
PscatL1A::WriteDataRecAscii()
{
    if (_outputFp == NULL)
        return(0);
    if (! frame.WriteAscii(_outputFp))
        return(0);
    return(1);
}
