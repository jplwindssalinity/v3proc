//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#ifndef PSCATL1A_H
#define PSCATL1A_H

static const char rcs_id_pscatl1a_h[] =
    "@(#) $Id$";

#include "BaseFile.h"
#include "Attitude.h"

//======================================================================
// CLASSES
//    PscatL1AFrame, PscatL1A
//======================================================================

//======================================================================
// CLASS
//    PscatL1AFrame
//
// DESCRIPTION
//    The PscatL1AFrame object contains the contents of a PSCAT Level
//    1A frame as a structure.
//======================================================================

class PscatL1AFrame
{
public:

    //--------------//
    // construction //
    //--------------//

    PscatL1AFrame();
    ~PscatL1AFrame();

    int  Allocate(int number_of_beams, int antenna_cycles_per_frame,
             int slices_per_spot);
    int  Deallocate();
    int  FrameSize();

    //-------------------//
    // data manipulation //
    //-------------------//

    int  Pack(char* buffer);
    int  Unpack(char* buffer);
    int  WriteAscii(FILE* ofp);

    //-------------------//
    // product variables //
    //-------------------//

    double         time;
    unsigned int   instrumentTicks;
    unsigned int   orbitTicks;
    unsigned char  orbitStep;
    unsigned char  priOfOrbitStepChange;

    //-----------------//
    // S/C information //
    //-----------------//

    float          gcAltitude;
    float          gcLongitude;
    float          gcLatitude;
    float          gcX;
    float          gcY;
    float          gcZ;
    float          velX;
    float          velY;
    float          velZ;
    Attitude       attitude;
    unsigned char  calPosition;

    //------------------//
    // calibration data //
    //------------------//

    unsigned int*  loopbackSlices;
    unsigned int   loopbackNoise;
    unsigned int*  loadSlices;
    unsigned int   loadNoise;

    //------------------//
    // antenna position //
    //------------------//

    unsigned short*  antennaPosition;

    //----------------------//
    // science measurements //
    //----------------------//

    unsigned char*   event;
    unsigned int*    science;
    unsigned int*    spotNoise;

    //-------------------------//
    // informational variables //
    //-------------------------//

    int  antennaCyclesPerFrame;
    int  spotsPerFrame;
    int  slicesPerSpot;
    int  measPerSlice;
    int  measPerSpot;
    int  measPerFrame;
};

//======================================================================
// CLASS
//    PscatL1A
//
// DESCRIPTION
//    The PscatL1A object allows for the easy writing, reading, and
//    manipulating of Level 1A data.
//======================================================================

class PscatL1A : public BaseFile
{
public:

    //------//
    // enum //
    //------//

    enum StatusE { OK, ERROR_READING_FRAME, ERROR_UNKNOWN };

    //--------------//
    // construction //
    //--------------//

    PscatL1A();
    ~PscatL1A();

    int  AllocateBuffer();
    int  DeallocateBuffer();

	//---------------------//
    // setting and getting //
    //---------------------//

    StatusE    GetStatus() { return(_status); };

    //--------------//
    // input/output //
    //--------------//

    int  ReadDataRec();
    int  WriteDataRec();
    int  WriteDataRecAscii();

    //-----------//
    // variables //
    //-----------//

    char*          buffer;
    int            bufferSize;
    PscatL1AFrame  frame;

protected:

    //-----------//
    // variables //
    //-----------//

    StatusE  _status;
};

#endif
