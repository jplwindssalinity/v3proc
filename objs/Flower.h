//=========================================================//
// Copyright (C) 2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef OBPROB_H
#define OBPROB_H

static const char rcs_id_flower_h[] =
    "@(#) $Id$";

#include <stdio.h>
#include "GMF.h"
#include "L2AH.h"

//======================================================================
// CLASSES
//    Flower, FlowerArray, DistProb
//======================================================================

class DistProb;

//======================================================================
// CLASS
//    Flower
//
// DESCRIPTION
//    The Flower object holds an array of probabilities and wind
//    speeds for multiple directions.
//======================================================================

#define DIR_BINS    72
#define SPD_BINS    60

#define MAX_SPD     30.0

#define PROB_SCALE  0.0001   // 0.0 - 6.5
#define SPD_SCALE   0.001    // 0.0 - 65

#define SPD_SUM_STEP_SIZE  0.1    // for summing up probability

#define MIN_PROB    1E-5

#define WVC_RESOLUTION  25.0    // km

#define VECTOR_SPEED_SCALE  0.05
#define VECTOR_HEAD_SCALE   0.15
#define VECTOR_HEAD_ANGLE   15.0*dtr
#define RAIN_VECTOR_HEAD_SCALE   0.50
#define RAIN_VECTOR_HEAD_ANGLE   10.0*dtr

#define PROB_DIAMOND_SCALE  1.5

#define MIN_NORM_PROB  1E-16

class Flower
{
public:

    //--------------//
    // construction //
    //--------------//

    Flower();
    ~Flower();

    void  FillProbabilities(float value);
    void  CopySpeeds(Flower* other_op);

    //--------------//
    // input/output //
    //--------------//

    int  Write(FILE* fp);
    int  Read(FILE* fp);

    float  GetSpeed(int dir_idx);
    float  GetAverageSpeed();
    float  GetDirection(int dir_idx);
    float  GetProbability(int dir_idx);
    float  SpeedIndexToSpeed(int spd_idx);
    int    GetSelectedDirIdx() { return(selectedDirIdx); };

    //------------//
    // processing //
    //------------//

    int    RetrieveProbabilities(GMF* gmf, MeasList* ml, Kp* kp);
    float  KmDistance(Flower* other_op);
    void   Add(int dir_idx, float probability);
    void   Add(Flower* other_op);
    void   Multiply(float value);
    void   Multiply(Flower* other_op);
    void   Power(float value);
    void   Normalize();
    int    Normalize(float min_prob);
    int    WriteFlower(FILE* ofp, float scale = 2.0, float max_range = 0.0);
    int    WriteSpeedBall(FILE* ofp, float scale = 0.03);
    int    WriteBestVector(FILE* ofp);
    int    WriteBestProb(FILE* ofp);
    int    FindBestDirIdx();
    void   SetSelectedDirIdx(int dir_idx) { selectedDirIdx = dir_idx; return; };
    void   ApplyPointFlower(float gamma, float alpha, Flower* point_flower);

    //-----------//
    // variables //
    //-----------//

    short           cti;
    short           ati;
    unsigned char   rainFlag;
    float           probabilityArray[DIR_BINS];
    unsigned short  speedArray[DIR_BINS];

    //---------------------//
    // temporary variables //
    //---------------------//

    int  selectedDirIdx;
};

//======================================================================
// CLASS
//    FlowerArray
//
// DESCRIPTION
//    A fixed size array of Flowers.
//======================================================================

class FlowerArray
{
public:

    //--------------//
    // construction //
    //--------------//

    FlowerArray();
    ~FlowerArray();

    //--------------//
    // input/output //
    //--------------//

    int       Read(const char* filename);
    Flower*   GetFlower(int cti, int ati);
    int       AttachFlower(int cti, int ati, Flower* flower);
    Flower*   DetachFlower(int cti, int ati);
    void      DeleteFlower(int cti, int ati);
    void      FreeContents();

    //------------//
    // processing //
    //------------//

    Flower*  LocalFlowerProb(DistProb* dp, int window_size, int center_cti,
                 int center_ati, float gamma, int use_rain_flag);
    Flower*  LocalVectorProb(DistProb* dp, int window_size, int center_cti,
                 int center_ati, float gamma, int use_rain_flag);
    int      SelectBestDirections();

    //-----------//
    // variables //
    //-----------//

    Flower* array[CT_WIDTH][AT_WIDTH];
};

//======================================================================
// CLASS
//    DistProb
//
// DESCRIPTION
//    The DistProb object is used to hold/calculate the probabilities
//    of wind speed and direction deltas given distance and speed.
//======================================================================

#define DISTANCE_BINS    26
#define MIN_DISTANCE     0.0
#define MAX_DISTANCE     250.0

#define SPEED_BINS       31
#define MIN_SPEED        0.0
#define MAX_SPEED        30.0

#define DSPEED_BINS      21
#define MIN_DSPEED       -10.0
#define MAX_DSPEED       10.0

#define DDIRECTION_BINS  37
#define MIN_DDIRECTION   0.0
#define MAX_DDIRECTION   M_PI

#define MINIMUM_SAMPLES  10

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
    float  Probability(float distance, float speed, float dspeed,
               float ddirection);
    float  Probability(int distance_idx, int speed_idx, int dspeed_idx,
               int ddirection_idx);

    int    DistanceToIndex(float distance);
    int    SpeedToIndex(float speed);
    int    DeltaSpeedToIndex(float dspeed);
    int    DeltaDirectionToIndex(float ddirection);

    float  IndexToDistance(int distance_idx);
    float  IndexToSpeed(int speed_idx);
    float  IndexToDeltaSpeed(int dspeed_idx);
    float  IndexToDeltaDirection(int ddirection_idx);

    //-----------//
    // variables //
    //-----------//

    Index  _distanceIndex;
    Index  _speedIndex;
    Index  _dspeedIndex;
    Index  _ddirectionIndex;

    unsigned long
        count[DISTANCE_BINS][SPEED_BINS][DSPEED_BINS][DDIRECTION_BINS];
    unsigned long
        sum[DISTANCE_BINS][SPEED_BINS];
};

#endif
