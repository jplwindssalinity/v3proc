//=========================================================//
// Copyright (C) 2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef OBPROB_H
#define OBPROB_H

static const char rcs_id_obprob_h[] =
    "@(#) $Id$";

#include <stdio.h>
#include "GMF.h"
#include "L2AH.h"

//======================================================================
// CLASSES
//    ObProb, ObProbArray, DistProb
//======================================================================

class DistProb;

//======================================================================
// CLASS
//    ObProb
//
// DESCRIPTION
//    The ObProb object holds an array of probabilities and wind
//    speeds for multiple directions.
//======================================================================

#define DIR_BINS    72
#define SPD_BINS    30

#define MAX_SPD     30.0

#define PROB_SCALE  0.0001   // 0.0 - 6.5
#define SPD_SCALE   0.001    // 0.0 - 65

#define WVC_RESOLUTION  25.0    // km

class ObProb
{
public:

    //--------------//
    // construction //
    //--------------//

    ObProb();
    ~ObProb();

    void  FillProbabilities(float value);
    void  CopySpeeds(ObProb* other_op);

    //--------------//
    // input/output //
    //--------------//

    int  Write(FILE* fp);
    int  Read(FILE* fp);

    float  GetSpeed(int dir_idx);
    float  GetDirection(int dir_idx);
    float  GetProbability(int dir_idx);
    float  SpeedIndexToSpeed(int spd_idx);

    //------------//
    // processing //
    //------------//

    int    RetrieveProbabilities(GMF* gmf, MeasList* ml, Kp* kp);
    float  KmDistance(ObProb* other_op);
    void   Add(int dir_idx, float probability);
    void   Multiply(ObProb* other_op);
    void   Normalize();
    int    WriteFlower(FILE* ofp);

    //-----------//
    // variables //
    //-----------//

    short cti;
    short ati;
    float           probabilityArray[DIR_BINS];
    unsigned short  speedArray[DIR_BINS];
};

//======================================================================
// CLASS
//    ObProbArray
//
// DESCRIPTION
//    A fixed size array of ObProbs.
//======================================================================

class ObProbArray
{
public:

    //--------------//
    // construction //
    //--------------//

    ObProbArray();
    ~ObProbArray();

    //--------------//
    // input/output //
    //--------------//

    int  Read(const char* filename);

    ObProb* GetObProb(int cti, int ati);

    //------------//
    // processing //
    //------------//

    ObProb*  LocalProb(DistProb* dp, int window_size, int center_cti,
                 int center_ati);

    //-----------//
    // variables //
    //-----------//

    ObProb* array[CT_WIDTH][AT_WIDTH];
};

//======================================================================
// CLASS
//    DistProb
//
// DESCRIPTION
//    The DistProb object is used to hold/calculate the probabilities
//    of wind speed and direction deltas given distance and speed.
//======================================================================

#define DISTANCE_BINS    41
#define MIN_DISTANCE     0.0
#define MAX_DISTANCE     1000.0

#define SPEED_BINS       31
#define MIN_SPEED        0.0
#define MAX_SPEED        30.0

#define DSPEED_BINS      21
#define MIN_DSPEED       -10.0
#define MAX_DSPEED       10.0

#define DDIRECTION_BINS  37
#define MIN_DDIRECTION   0.0
#define MAX_DDIRECTION   M_PI

class DistProb
{
public:

    //--------------//
    // construction //
    //--------------//

    DistProb();
    ~DistProb();

    //--------------//
    // input/output //
    //--------------//

    int  Write(const char* filename);
    int  Read(const char* filename);

    //-----//
    // use //
    //-----//

    void   SetSum();
    float  Probability(float distance, float speed0, float dspeed,
               float ddirection);

    int    DistanceToIndex(float distance);
    int    SpeedToIndex(float speed);
    int    DeltaSpeedToIndex(float dspeed);
    int    DeltaDirectionToIndex(float ddirection);

    float  IndexToDistance(int distance_idx);
    float  IndexToSpeed(int speed_idx);
    float  IndexToDeltaSpeed(int dspeed_idx);

    //-----------//
    // variables //
    //-----------//

    Index  _distanceIndex;
    Index  _speedIndex;
    Index  _dspeedIndex;
    Index  _ddirectionIndex;

    unsigned long
        count[DISTANCE_BINS][SPEED_BINS][DSPEED_BINS][DDIRECTION_BINS];
    unsigned long  sum;    // the normalization factor
};

#endif