//=========================================================//
// Copyright (C) 2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

static const char rcs_id_obprob_c[] =
    "@(#) $Id$";

#include <stdio.h>
#include <math.h>
#include "ObProb.h"

//========//
// ObProb //
//========//

ObProb::ObProb()
:  cti(0), ati(0)
{
    return;
}

ObProb::~ObProb()
{
    return;
}

//---------------------------//
// ObProb::FillProbabilities //
//---------------------------//

void
ObProb::FillProbabilities(
    float  value)
{
    for (int i = 0; i < DIR_BINS; i++)
    {
        probabilityArray[i] = value;
    }
    return;
}

//--------------------//
// ObProb::CopySpeeds //
//--------------------//

void
ObProb::CopySpeeds(
    ObProb*  other_op)
{
    for (int i = 0; i < DIR_BINS; i++)
    {
        speedArray[i] = other_op->speedArray[i];
    }
    return;
}

//---------------//
// ObProb::Write //
//---------------//

int
ObProb::Write(
    FILE*  fp)
{
    if (fwrite(&cti, sizeof(short), 1, fp) == 1 &&
        fwrite(&ati, sizeof(short), 1, fp) == 1 &&
        fwrite(probabilityArray, sizeof(unsigned short), DIR_BINS, fp)
          == DIR_BINS &&
        fwrite(speedArray, sizeof(unsigned short), DIR_BINS, fp) == DIR_BINS)
    {
        return(1);
    }
    return(0);
}

//--------------//
// ObProb::Read //
//--------------//

int
ObProb::Read(
    FILE*  fp)
{
    if (fread(&cti, sizeof(short), 1, fp) == 1 &&
        fread(&ati, sizeof(short), 1, fp) == 1 &&
        fread(probabilityArray, sizeof(unsigned short), DIR_BINS, fp)
          == DIR_BINS &&
        fread(speedArray, sizeof(unsigned short), DIR_BINS, fp) == DIR_BINS)
    {
        return(1);
    }
    return(0);
}

//------------------//
// ObProb::GetSpeed //
//------------------//

float
ObProb::GetSpeed(
    int  dir_idx)
{
    return(speedArray[dir_idx] * SPD_SCALE);
}

//----------------------//
// ObProb::GetDirection //
//----------------------//
// in radians

float
ObProb::GetDirection(
    int  dir_idx)
{
    return((float)(two_pi * (double)dir_idx / (double)DIR_BINS));
}

//------------------------//
// ObProb::GetProbability //
//------------------------//

float
ObProb::GetProbability(
    int  dir_idx)
{
    return(probabilityArray[dir_idx] * PROB_SCALE);
}

//---------------------------//
// ObProb::SpeedIndexToSpeed //
//---------------------------//

float
ObProb::SpeedIndexToSpeed(
    int  spd_idx)
{
    return((float)((double)MAX_SPD * (double)spd_idx / (double)SPD_BINS));
}

//--------------------------------//
// ObProb::RetrieveProbabilities //
//--------------------------------//

int
ObProb::RetrieveProbabilities(
    GMF*       gmf,
    MeasList*  ml,
    Kp*        kp)
{
    //----------------//
    // get MLE values //
    //----------------//

    float tmp_array[DIR_BINS][SPD_BINS];
    float max_mle = -9e9;
    for (int dir_idx = 0; dir_idx < DIR_BINS; dir_idx++)
    {
        float dir = GetDirection(dir_idx);
        for (int spd_idx = 0; spd_idx < SPD_BINS; spd_idx++)
        {
            float spd = SpeedIndexToSpeed(spd_idx);
            float mle = gmf->_ObjectiveFunction(ml, spd, dir, kp);
            tmp_array[dir_idx][spd_idx] = mle;
            if (mle > max_mle)
                max_mle = mle;
        }
    }

    //--------------------------//
    // convert to probabilities //
    //--------------------------//

    double total_prob_sum = 0.0;
    for (int dir_idx = 0; dir_idx < DIR_BINS; dir_idx++)
    {
        for (int spd_idx = 0; spd_idx < SPD_BINS; spd_idx++)
        {
            tmp_array[dir_idx][spd_idx] =
                exp((tmp_array[dir_idx][spd_idx] - max_mle) / 2.0);
            total_prob_sum += tmp_array[dir_idx][spd_idx];
        }
    }

    //-----------------------//
    // collapse in direction //
    //-----------------------//
    // and find the best speed //

    for (int dir_idx = 0; dir_idx < DIR_BINS; dir_idx++)
    {
        double dir_prob_sum = 0.0;
        int best_spd_idx = 0;
        for (int spd_idx = 0; spd_idx < SPD_BINS; spd_idx++)
        {
            dir_prob_sum += tmp_array[dir_idx][spd_idx];
            if (tmp_array[dir_idx][spd_idx]
                > tmp_array[dir_idx][best_spd_idx])
            {
                best_spd_idx = spd_idx;
            }
        }
        dir_prob_sum /= total_prob_sum;    // normalize total prob to 1.0
        probabilityArray[dir_idx] = (int)(dir_prob_sum / PROB_SCALE + 0.5);
        float spd = MAX_SPD * (float)best_spd_idx / (float)SPD_BINS;
        speedArray[dir_idx] = (int)(spd / SPD_SCALE + 0.5);
    }

    return(1);
}

//--------------------//
// ObProb::KmDistance //
//--------------------//

float
ObProb::KmDistance(
    ObProb*  other_op)
{
    int dati = other_op->ati - ati;
    int dcti = other_op->cti - cti;
    float distance = WVC_RESOLUTION
        * (float)sqrt((double)(dati*dati + dcti*dcti));
    return(distance);
}

//-------------//
// ObProb::Add //
//-------------//

void
ObProb::Add(
    int    dir_idx,
    float  probability)
{
    probabilityArray[dir_idx] += probability;
    return;
}

//------------------//
// ObProb::Multiply //
//------------------//

void
ObProb::Multiply(
    ObProb*  other_op)
{
    for (int i = 0; i < DIR_BINS; i++)
    {
        probabilityArray[i] *= other_op->probabilityArray[i];
    }
    return;
}

//-------------------//
// ObProb::Normalize //
//-------------------//

void
ObProb::Normalize()
{
    double sum = 0.0;
    for (int i = 0; i < DIR_BINS; i++)
    {
        sum += (double)probabilityArray[i];
    }
    if (sum == 0.0)
        return;
    for (int i = 0; i < DIR_BINS; i++)
    {
        probabilityArray[i] /= (sum);
    }
    return;
}

//---------------------//
// ObProb::WriteFlower //
//---------------------//

int
ObProb::WriteFlower(
    FILE*  ofp)
{
    //-----------------//
    // generate flower //
    //-----------------//

    float max_dist = 1.0;    // this corresponds to probability 1.0
    fprintf(ofp, "#\n");
    fprintf(ofp, "%d %d\n", cti, ati);
    for (int dir_idx = 0; dir_idx < DIR_BINS; dir_idx++)
    {
        float direction = GetDirection(dir_idx);
        float probability = GetProbability(dir_idx);
        float dx = max_dist * cos(direction);
        float dy = max_dist * sin(direction);
        fprintf(ofp, "%g %g\n", (float)cti + dx, (float)ati + dy);
    }
    return(1);
}

//=============//
// ObProbArray //
//=============//

ObProbArray::ObProbArray()
{
    for (int i = 0; i < CT_WIDTH; i++)
    {
        for (int j = 0; j < AT_WIDTH; j++)
        {
            array[i][j] = NULL;
        }
    }
    return;
}

ObProbArray::~ObProbArray()
{
    for (int i = 0; i < CT_WIDTH; i++)
    {
        for (int j = 0; j < AT_WIDTH; j++)
        {
            free(array[i][j]);
        }
    }
    return;
}

//-------------------//
// ObProbArray::Read //
//-------------------//

int
ObProbArray::Read(
    const char*  filename)
{
    FILE* ifp = fopen(filename, "r");
    if (ifp == NULL)
    {
        fprintf(stderr, "Error opening file %s\n", filename);
        return(0);
    }

    for (;;)
    {
        ObProb* op = new ObProb();
        if (op == NULL)
        {
            fprintf(stderr, "Error allocating obprob\n");
            return(0);
        }
        if (! op->Read(ifp))
        {
            if (feof(ifp))
                break;
            else
            {
                fprintf(stderr, "Error reading obprob\n");
                return(0);
            }
        }

        int cti = op->cti;
        if (cti < 0 || cti >= CT_WIDTH)
        {
            fprintf(stderr, "CTI out of range (%d)\n", cti);
            return(0);
        }
        int ati = op->ati;
        if (ati < 0 || ati >= AT_WIDTH)
        {
            fprintf(stderr, "ATI out of range (%d)\n", ati);
            return(0);
        }
        array[cti][ati] = op;
    }

    fclose(ifp);
    return(1);
}

//------------------------//
// ObProbArray::GetObProb //
//------------------------//

ObProb*
ObProbArray::GetObProb(
    int  cti,
    int  ati)
{
    if (cti < 0 || cti >= CT_WIDTH || ati < 0 || ati >= AT_WIDTH)
        return(NULL);
    return(array[cti][ati]);
}

//------------------------//
// ObProbArray::LocalProb //
//------------------------//

ObProb*
ObProbArray::LocalProb(
    DistProb*     dp,
    int           window_size,
    int           center_cti,
    int           center_ati)
{
    //----------------//
    // get the center //
    //----------------//

    ObProb* op0 = GetObProb(center_cti, center_ati);
    if (op0 == NULL)
        return(NULL);

    //-------------------------------//
    // this is where the output goes //
    //-------------------------------//

    ObProb* dest_op = new ObProb();
    if (dest_op == NULL)
        return(NULL);
    dest_op->cti = center_cti;
    dest_op->ati = center_ati;
    dest_op->FillProbabilities(1.0);    // we'll be multiplying
    dest_op->CopySpeeds(op0);    // use the original speeds

    //--------------------------//
    // temporary ObProb storage //
    //--------------------------//

    ObProb tmp_op;

    //-----------------------------//
    // determine window boundaries //
    //-----------------------------//

    int half_window = window_size / 2;

    int min_cti = center_cti - half_window;
    if (min_cti < 0)
        min_cti = 0;
    int max_cti = center_cti + half_window + 1;
    if (max_cti > CT_WIDTH)
        max_cti = CT_WIDTH;

    int min_ati = center_ati - half_window;
    if (min_ati < 0)
        min_ati = 0;
    int max_ati = center_ati + half_window + 1;
    if (max_ati > AT_WIDTH)
        max_ati = AT_WIDTH;

    for (int cti = min_cti; cti < max_cti; cti++)
    {
        for (int ati = min_ati; ati < max_ati; ati++)
        {
            ObProb* op1 = GetObProb(cti, ati);
            if (op1 == NULL)
                continue;

            float distance = op0->KmDistance(op1);

            //----------------------------------------//
            // first, clear the tmp probability array //
            //----------------------------------------//

            tmp_op.FillProbabilities(0.0);

            //------------//
            // accumulate //
            //------------//

            for (int dir_idx = 0; dir_idx < DIR_BINS; dir_idx++)
            {
                float speed0 = op0->GetSpeed(dir_idx);
                float direction0 = op0->GetDirection(dir_idx);

                for (int other_dir_idx = 0; other_dir_idx < DIR_BINS;
                    other_dir_idx++)
                {
                    float speed1 = op1->GetSpeed(other_dir_idx);
                    float direction1 = op1->GetDirection(other_dir_idx);
                    float probability1 = op1->GetProbability(other_dir_idx);

                    float dspeed = speed1 - speed0;
                    float ddirection = ANGDIF(direction1, direction0);

                    float dist_prob = dp->Probability(distance, speed0,
                        dspeed, ddirection);

                    tmp_op.Add(dir_idx, dist_prob * probability1);
                }
            }

            //-------------------------------------------------//
            // normalize, just to keep the values large enough //
            //-------------------------------------------------//

            tmp_op.Normalize();

            //----------------------------------//
            // multiply it into the destination //
            //----------------------------------//

            dest_op->Multiply(&tmp_op);
        }
    }
    return(dest_op);
}

//==========//
// DistProb //
//==========//

DistProb::DistProb()
:   sum(0)
{
    //-----------------------//
    // clear the count array //
    //-----------------------//

    for (int i = 0; i < DISTANCE_BINS; i++)
    {
        for (int j = 0; j < SPEED_BINS; j++)
        {
            for (int k = 0; k < DSPEED_BINS; k++)
            {
                for (int l = 0; l < DDIRECTION_BINS; l++)
                {
                    count[i][j][k][l] = 0;
                }
            }
        }
    }

    //------------------//
    // set the indicies //
    //------------------//

    _distanceIndex.SpecifyCenters(MIN_DISTANCE, MAX_DISTANCE, DISTANCE_BINS);
    _speedIndex.SpecifyCenters(MIN_SPEED, MAX_SPEED, SPEED_BINS);
    _dspeedIndex.SpecifyCenters(MIN_DSPEED, MAX_DSPEED, DSPEED_BINS);
    _ddirectionIndex.SpecifyCenters(MIN_DDIRECTION, MAX_DDIRECTION,
        DDIRECTION_BINS);

    return;
}

DistProb::~DistProb()
{
    return;
}

//-----------------//
// DistProb::Write //
//-----------------//

int
DistProb::Write(
    const char*  filename)
{
    FILE* ofp = fopen(filename, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "Error opening output file %s\n", filename);
        return(0);
    }
    unsigned long number = DISTANCE_BINS * SPEED_BINS * DSPEED_BINS
        * DDIRECTION_BINS;
    if (fwrite(count, sizeof(unsigned long), number, ofp) != number)
    {
        fprintf(stderr, "Error writing DistProb file %s\n", filename);
        return(0);
    }
    fclose(ofp);
    return(1);
}

//----------------//
// DistProb::Read //
//----------------//

int
DistProb::Read(
    const char*  filename)
{
    FILE* ifp = fopen(filename, "r");
    if (ifp == NULL)
    {
        fprintf(stderr, "Error opening DistProb file %s\n", filename);
        return(0);
    }
    unsigned long number = DISTANCE_BINS * SPEED_BINS * DSPEED_BINS
        * DDIRECTION_BINS;
    if (fread(count, sizeof(unsigned long), number, ifp) != number)
    {
        fprintf(stderr, "Error reading DistProb file %s\n", filename);
        return(0);
    }
    fclose(ifp);

    //--------------------------//
    // set the sum scale factor //
    //--------------------------//

    SetSum();

    return(1);
}

//------------------//
// DistProb::SetSum //
//------------------//

void
DistProb::SetSum()
{
    for (int i = 0; i < DISTANCE_BINS; i++)
    {
        for (int j = 0; j < SPEED_BINS; j++)
        {
            for (int k = 0; k < DSPEED_BINS; k++)
            {
                for (int l = 0; l < DDIRECTION_BINS; l++)
                {
                    sum += count[i][j][k][l];
                }
            }
        }
    }
    return;
}

//-----------------------//
// DistProb::Probability //
//-----------------------//

float
DistProb::Probability(
    float  distance,
    float  speed,
    float  dspeed,
    float  ddirection)
{
    //-------------------------------//
    // convert everything to indices //
    //-------------------------------//

    int distance_idx = DistanceToIndex(distance);
    int speed_idx = SpeedToIndex(speed);
    int dspeed_idx = DeltaSpeedToIndex(dspeed);
    int ddirection_idx = DeltaDirectionToIndex(ddirection);

    double probability =
        (double)count[distance_idx][speed_idx][dspeed_idx][ddirection_idx] /
        (double)sum;

    return((float)probability);
}

//---------------------------//
// DistProb::DistanceToIndex //
//---------------------------//

int
DistProb::DistanceToIndex(
    float  distance)
{
    int idx = -1;
    _distanceIndex.GetNearestIndexClipped(distance, &idx);
    return(idx);
}

//------------------------//
// DistProb::SpeedToIndex //
//------------------------//

int
DistProb::SpeedToIndex(
    float  speed)
{
    int idx = -1;
    _speedIndex.GetNearestIndexClipped(speed, &idx);
    return(idx);
}

//-----------------------------//
// DistProb::DeltaSpeedToIndex //
//-----------------------------//

int
DistProb::DeltaSpeedToIndex(
    float  dspeed)
{
    int idx = -1;
    _dspeedIndex.GetNearestIndexClipped(dspeed, &idx);
    return(idx);
}

//---------------------------------//
// DistProb::DeltaDirectionToIndex //
//---------------------------------//

int
DistProb::DeltaDirectionToIndex(
    float  ddirection)
{
    int idx = -1;
    _ddirectionIndex.GetNearestIndexClipped(ddirection, &idx);
    return(idx);
}

//---------------------------//
// DistProb::IndexToDistance //
//---------------------------//

float
DistProb::IndexToDistance(
    int  distance_idx)
{
    float distance;
    _distanceIndex.IndexToValue(distance_idx, &distance);
    return(distance);
}

//------------------------//
// DistProb::IndexToSpeed //
//------------------------//

float
DistProb::IndexToSpeed(
    int  speed_idx)
{
    float speed;
    _speedIndex.IndexToValue(speed_idx, &speed);
    return(speed);
}

//-----------------------------//
// DistProb::IndexToDeltaSpeed //
//-----------------------------//

float
DistProb::IndexToDeltaSpeed(
    int  dspeed_idx)
{
    float dspeed;
    _dspeedIndex.IndexToValue(dspeed_idx, &dspeed);
    return(dspeed);
}
