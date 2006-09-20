//==============================================================//
// Copyright (C) 1997-1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#ifndef OVWML1A_H
#define OVWML1A_H

static const char rcs_id_pscatl1a_h[] =
    "@(#) $Id$";

#include "Constants.h"
#include "BaseFile.h"
#include "Attitude.h"

//======================================================================
// CLASSES
//    OvwmL1AFrame, OvwmL1A
//======================================================================

//======================================================================
// CLASS
//    OvwmL1AFrame
//
// DESCRIPTION
//    The OvwmL1AFrame object contains the contents of a OVWM Level
//    1A frame as a structure.
//======================================================================

class OvwmL1AFrame
{
public:

    //--------------//
    // construction //
    //--------------//

    OvwmL1AFrame();
    ~OvwmL1AFrame();

    int  Allocate(int number_of_beams, int antenna_cycles_per_frame, int max_meas_per_spot);
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
    double* spotTime;      // array of time for spots in a frame
    float*  spotScanAngle; // array of azimuth angle for spots in a frame
    int*    spotBeamIdx;   // array of beam idx for spots in a frame

    //unsigned int   instrumentTicks;
    //unsigned int   orbitTicks;
    //unsigned char  orbitStep;
    //unsigned char  priOfOrbitStepChange;

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
    //unsigned char  calPosition;

    //------------------//
    // calibration data //
    //------------------//

    float*  loopbackSpots;
    float   loopbackNoise;
    float*  loadSpots;
    float   loadNoise;

    //------------------//
    // antenna position //
    //------------------//

    //unsigned short*  antennaPosition;

    //----------------------//
    // science measurements //
    //----------------------//

    float*    science;
    float*    spotNoise;
    int*      dataCountSpots;

    //----------------------------//
    // L1A Status and Error Flags //
    //----------------------------//

    //unsigned int    frame_inst_status;

    //-------------------------//
    // informational variables //
    //-------------------------//

    //int  antennaCyclesPerFrame;
    int  spotsPerFrame;
    int  maxMeasPerSpot;
    //int  slicesPerFrame;
};

//======================================================================
// CLASS
//    OvwmL1A
//
// DESCRIPTION
//    The OvwmL1A object allows for the easy writing, reading, and
//    manipulating of Level 1A data.
//======================================================================

class OvwmL1A : public BaseFile
{
public:

    //------//
    // enum //
    //------//

    enum StatusE { OK, ERROR_READING_FRAME, ERROR_UNKNOWN };

    //--------------//
    // construction //
    //--------------//

    OvwmL1A();
    ~OvwmL1A();

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
    int  WriteDataRecAscii(FILE* fp);

    //-----------//
    // variables //
    //-----------//

    char*          buffer;
    int            bufferSize;
    OvwmL1AFrame   frame;

protected:

    //-----------//
    // variables //
    //-----------//

    StatusE  _status;
};

#endif
