//=========================================================//
// Copyright (C) 2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

static const char rcs_id_flower_c[] =
    "@(#) $Id$";

#include <stdio.h>
#include <math.h>
#include "Flower.h"

//========//
// Flower //
//========//

Flower::Flower()
:  cti(0), ati(0), selectedDirIdx(-1)
{
    return;
}

Flower::~Flower()
{
    return;
}

//---------------------------//
// Flower::FillProbabilities //
//---------------------------//

void
Flower::FillProbabilities(
    float  value)
{
    for (int i = 0; i < DIR_BINS; i++)
    {
        probabilityArray[i] = value;
    }
    return;
}

//--------------------//
// Flower::CopySpeeds //
//--------------------//

void
Flower::CopySpeeds(
    Flower*  other_op)
{
    for (int i = 0; i < DIR_BINS; i++)
    {
        speedArray[i] = other_op->speedArray[i];
    }
    return;
}

//---------------//
// Flower::Write //
//---------------//

int
Flower::Write(
    FILE*  fp)
{
    if (fwrite(&cti, sizeof(short), 1, fp) == 1 &&
        fwrite(&ati, sizeof(short), 1, fp) == 1 &&
        fwrite(probabilityArray, sizeof(float), DIR_BINS, fp) == DIR_BINS &&
        fwrite(speedArray, sizeof(unsigned short), DIR_BINS, fp) == DIR_BINS)
    {
        return(1);
    }
    return(0);
}

//--------------//
// Flower::Read //
//--------------//

int
Flower::Read(
    FILE*  fp)
{
    if (fread(&cti, sizeof(short), 1, fp) == 1 &&
        fread(&ati, sizeof(short), 1, fp) == 1 &&
        fread(probabilityArray, sizeof(float), DIR_BINS, fp) == DIR_BINS &&
        fread(speedArray, sizeof(unsigned short), DIR_BINS, fp) == DIR_BINS)
    {
        return(1);
    }
    return(0);
}

//------------------//
// Flower::GetSpeed //
//------------------//

float
Flower::GetSpeed(
    int  dir_idx)
{
    return(speedArray[dir_idx] * SPD_SCALE);
}

//-------------------------//
// Flower::GetAverageSpeed //
//-------------------------//

float
Flower::GetAverageSpeed()
{
    double spd_sum = 0.0;
    for (int i = 0; i < DIR_BINS; i++)
        spd_sum += speedArray[i] * SPD_SCALE;
    float avg_spd = spd_sum / (double)DIR_BINS;
    return(avg_spd);
}

//----------------------//
// Flower::GetDirection //
//----------------------//
// in radians

float
Flower::GetDirection(
    int  dir_idx)
{
    return((float)(two_pi * (double)dir_idx / (double)DIR_BINS));
}

//------------------------//
// Flower::GetProbability //
//------------------------//

float
Flower::GetProbability(
    int  dir_idx)
{
    return(probabilityArray[dir_idx]);
}

//---------------------------//
// Flower::SpeedIndexToSpeed //
//---------------------------//

float
Flower::SpeedIndexToSpeed(
    int  spd_idx)
{
    return((float)((double)MAX_SPD * (double)spd_idx / (double)SPD_BINS));
}

//-------------------------------//
// Flower::RetrieveProbabilities //
//-------------------------------//

int
Flower::RetrieveProbabilities(
    GMF*       gmf,
    MeasList*  ml,
    Kp*        kp)
{
    //-----------------------------------//
    // find best speed at each direction //
    //-----------------------------------//

    float spd_min = gmf->GetMinSpd();
    float spd_max = gmf->GetMaxSpd();

    float max_mle = -9e9;
    float ax = spd_min;
    float cx = spd_max;
    for (int dir_idx = 0; dir_idx < DIR_BINS; dir_idx++)
    {
        float bx = ax + (cx - ax) * golden_r;
        float phi = (float)(two_pi * (double)dir_idx / (double)DIR_BINS);
        if (gmf->_ObjectiveFunction(ml, bx, phi, kp) <
            gmf->_ObjectiveFunction(ml, ax, phi, kp) )
        {
            ax = spd_min;
        }
        if (gmf->_ObjectiveFunction(ml, bx, phi, kp) <
            gmf->_ObjectiveFunction(ml, cx, phi, kp) )
        {
            cx = spd_max;
        }
        float x0, x1, x2, x3;
        x0 = ax;
        x3 = cx;
        if (cx - bx > bx - ax)
        {
            x1 = bx;
            x2 = bx + golden_c * (cx - bx);
        }
        else
        {
            x2 = bx;
            x1 = bx - golden_c * (bx - ax);
        }
        float f1 = gmf->_ObjectiveFunction(ml, x1, phi, kp);
        float f2 = gmf->_ObjectiveFunction(ml, x2, phi, kp);
        while (x3 - x0 > SPD_SCALE)
        {
            if (f2 > f1)
            {
                x0 = x1;
                x1 = x2;
                x2 = x2 + golden_c * (x3 - x2);
                f1 = f2;
                f2 = gmf->_ObjectiveFunction(ml, x2, phi, kp);
                if (f2 > max_mle)
                    max_mle = f2;
            }
            else
            {
                x3 = x2;
                x2 = x1;
                x1 = x1 - golden_c * (x1 - x0);
                f2 = f1;
                f1 = gmf->_ObjectiveFunction(ml, x1, phi, kp);
                if (f1 > max_mle)
                    max_mle = f1;
            }
        }
        if (f1 > f2)
        {
            speedArray[dir_idx] = (unsigned short)(x1 / SPD_SCALE + 0.5);
        }
        else
        {
            speedArray[dir_idx] = (unsigned short)(x2 / SPD_SCALE + 0.5);
        }
        ax = x1 - (x1 * 0.05) - 0.5;
        if (ax < spd_min)
            ax = spd_min;
        cx = x1 + (x2 * 0.05) + 0.5;
        if (cx > spd_max)
            cx = spd_max;
    }

    //-----------------//
    // sum up in speed //
    //-----------------//

    for (int dir_idx = 0; dir_idx < DIR_BINS; dir_idx++)
    {
        float phi = (float)(two_pi * (double)dir_idx / (double)DIR_BINS);
        float spd = speedArray[dir_idx] * SPD_SCALE;
        double sum_prob = exp((gmf->_ObjectiveFunction(ml, spd, phi, kp)
            - max_mle) / 2.0);
        for(;;)
        {
            spd -= SPD_SUM_STEP_SIZE;
            if (spd < spd_min)
                break;
            double prob = exp((gmf->_ObjectiveFunction(ml, spd, phi, kp)
                - max_mle) / 2.0);
            sum_prob += prob;
            if (prob < MIN_PROB)
                break;
        }
        spd = speedArray[dir_idx] * SPD_SCALE;
        for(;;)
        {
            spd += SPD_SUM_STEP_SIZE;
            if (spd < spd_min)
                break;
            double prob = exp((gmf->_ObjectiveFunction(ml, spd, phi, kp)
                - max_mle) / 2.0);
            sum_prob += prob;
            if (prob < MIN_PROB)
                break;
        }
        probabilityArray[dir_idx] = sum_prob;
    }

    //-----------------------//
    // normalize probability //
    //-----------------------//

    Normalize();

    return(1);
}

//--------------------//
// Flower::KmDistance //
//--------------------//

float
Flower::KmDistance(
    Flower*  other_op)
{
    int dati = other_op->ati - ati;
    int dcti = other_op->cti - cti;
    float distance = WVC_RESOLUTION
        * (float)sqrt((double)(dati*dati + dcti*dcti));
    return(distance);
}

//-------------//
// Flower::Add //
//-------------//

void
Flower::Add(
    int    dir_idx,
    float  probability)
{
    probabilityArray[dir_idx] += probability;
    return;
}

//-------------//
// Flower::Add //
//-------------//

void
Flower::Add(
    Flower*  other_op)
{
    for (int i = 0; i < DIR_BINS; i++)
    {
        probabilityArray[i] += other_op->probabilityArray[i];
    }
    return;
}

//------------------//
// Flower::Multiply //
//------------------//

void
Flower::Multiply(
    Flower*  other_op)
{
    for (int i = 0; i < DIR_BINS; i++)
    {
        probabilityArray[i] *= other_op->probabilityArray[i];
    }
    return;
}

//-------------------//
// Flower::Normalize //
//-------------------//

void
Flower::Normalize()
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
        probabilityArray[i] /= sum;
    }
    return;
}

//-------------------//
// Flower::Normalize //
//-------------------//

int
Flower::Normalize(
    float  min_prob)
{
    Normalize();
    int count = 0;
    for (int i = 0; i < DIR_BINS; i++)
    {
        if (probabilityArray[i] < min_prob)
        {
            probabilityArray[i] = 0.0;
            count++;
        }
    }

    // renomalize if necessary
    if (count > 0)
        Normalize();

    return(count);
}

//---------------------//
// Flower::WriteFlower //
//---------------------//

int
Flower::WriteFlower(
    FILE*  ofp,
    float  scale,
    float  max_range)  // ignore the scale and make this the maximum distance
{
    //-----------------//
    // generate flower //
    //-----------------//

    if (max_range != 0.0)
    {
        // calculate the scale factor
        double max_prob = 0.0;
        for (int dir_idx = 0; dir_idx < DIR_BINS; dir_idx++)
        {
            double probability = GetProbability(dir_idx);
            if (probability > max_prob)
                max_prob = probability;
        }
        if (max_prob == 0)
            return(0);
        scale = max_range / max_prob;
    }

    fprintf(ofp, "#\n");
    fprintf(ofp, "%d %d\n", cti, ati);
    for (int dir_idx = 0; dir_idx <= DIR_BINS; dir_idx++)
    {
        int use_dir_idx = dir_idx % DIR_BINS;
        double direction = GetDirection(use_dir_idx);
        double probability = GetProbability(use_dir_idx);
        double dx = probability * scale * cos(direction);
        double dy = probability * scale * sin(direction);
        fprintf(ofp, "%g %g\n", (double)cti + dx, (double)ati + dy);
    }
    fprintf(ofp, "%d %d\n", cti, ati);
    return(1);
}

//-------------------------//
// Flower::WriteBestVector //
//-------------------------//

int
Flower::WriteBestVector(
    FILE*  ofp)
{
    fprintf(ofp, "#\n");
    double max_prob = 0.0;
    int max_dir_idx = 0;
    for (int dir_idx = 0; dir_idx < DIR_BINS; dir_idx++)
    {
        double probability = GetProbability(dir_idx);
        if (probability > max_prob)
        {
            max_prob = probability;
            max_dir_idx = dir_idx;
        }
    }
    if (max_prob == 0.0)
        return(0);

    double x[6], y[6];
    double direction = GetDirection(max_dir_idx);
    double spd = GetSpeed(max_dir_idx);
    x[0] = cti;
    y[0] = ati;
    x[1] = x[0] + VECTOR_SPEED_SCALE * spd * cos(direction);
    y[1] = y[0] + VECTOR_SPEED_SCALE * spd * sin(direction);
    x[2] = x[1] + VECTOR_HEAD_SCALE * cos(direction + M_PI
        - VECTOR_HEAD_ANGLE);
    y[2] = y[1] + VECTOR_HEAD_SCALE * sin(direction + M_PI
        - VECTOR_HEAD_ANGLE);
    x[3] = x[1] + VECTOR_HEAD_SCALE * cos(direction - M_PI
        + VECTOR_HEAD_ANGLE);
    y[3] = y[1] + VECTOR_HEAD_SCALE * sin(direction - M_PI
        + VECTOR_HEAD_ANGLE);
    x[4] = x[1];
    y[4] = y[1];
    x[5] = x[0];
    y[5] = y[0];
    for (int i = 0; i < 6; i++)
    {
        fprintf(ofp, "%g %g\n", x[i], y[i]);
    }
    return(1);
}

//-----------------------//
// Flower::WriteBestProb //
//-----------------------//

int
Flower::WriteBestProb(
    FILE*  ofp)
{
    fprintf(ofp, "#\n");
    int best_idx = FindBestDirIdx();
    if (best_idx == -1)
        return(0);

    double probability = GetProbability(best_idx);

    double x[7], y[7];
    x[0] = cti;
    y[0] = ati;
    x[1] = x[0] + PROB_DIAMOND_SCALE * probability;
    y[1] = y[0];
    x[2] = x[0];
    y[2] = y[0] + PROB_DIAMOND_SCALE * probability;
    x[3] = x[0] - PROB_DIAMOND_SCALE * probability;
    y[3] = y[0];
    x[4] = x[0];
    y[4] = y[0] - PROB_DIAMOND_SCALE * probability;
    x[5] = x[0] + PROB_DIAMOND_SCALE * probability;
    y[5] = y[0];
    x[6] = cti;
    y[6] = ati;
    for (int i = 0; i < 7; i++)
    {
        fprintf(ofp, "%g %g\n", x[i], y[i]);
    }
    return(1);
}

//------------------------//
// Flower::FindBestDirIdx //
//------------------------//

int
Flower::FindBestDirIdx()
{
    double max_prob = 0.0;
    int max_idx = -1;
    for (int dir_idx = 0; dir_idx < DIR_BINS; dir_idx++)
    {
        double probability = GetProbability(dir_idx);
        if (probability > max_prob)
        {
            max_prob = probability;
            max_idx = dir_idx;
        }
    }
    return(max_idx);
}

//=============//
// FlowerArray //
//=============//

FlowerArray::FlowerArray()
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

FlowerArray::~FlowerArray()
{
    FreeContents();
    return;
}

//-------------------//
// FlowerArray::Read //
//-------------------//

int
FlowerArray::Read(
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
        Flower* op = new Flower();
        if (op == NULL)
        {
            fprintf(stderr, "Error allocating Flower\n");
            return(0);
        }
        if (! op->Read(ifp))
        {
            if (feof(ifp))
                break;
            else
            {
                fprintf(stderr, "Error reading Flower\n");
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
// FlowerArray::GetFlower //
//------------------------//

Flower*
FlowerArray::GetFlower(
    int  cti,
    int  ati)
{
    if (cti < 0 || cti >= CT_WIDTH || ati < 0 || ati >= AT_WIDTH)
        return(NULL);
    return(array[cti][ati]);
}

//---------------------------//
// FlowerArray::FreeContents //
//---------------------------//

void
FlowerArray::FreeContents()
{
    for (int i = 0; i < CT_WIDTH; i++)
    {
        for (int j = 0; j < AT_WIDTH; j++)
        {
            free(array[i][j]);
            array[i][j] = NULL;
        }
    }
    return;
}

//------------------------------//
// FlowerArray::LocalFlowerProb //
//------------------------------//
// gamma controls the assumptions about error correlation. When
// gamma is 0.0, the errors are assumed to be uncorrelated and the
// probabilities are multiplied. When gamma is 1.0, the errors
// are assumed to be completely correlated and the probabilities
// are added. Thanks to Bryan Stiles for helping me with the
// formulation. This one uses the whole flower.

Flower*
FlowerArray::LocalFlowerProb(
    DistProb*  dp,
    int        window_size,
    int        center_cti,
    int        center_ati,
    float      gamma)
{
    //----------------//
    // get the center //
    //----------------//

    Flower* op0 = GetFlower(center_cti, center_ati);
    if (op0 == NULL)
        return(NULL);

    //-------------------------------//
    // this is where the output goes //
    //-------------------------------//

    Flower* dest_op = new Flower();
    if (dest_op == NULL)
        return(NULL);
    dest_op->cti = center_cti;
    dest_op->ati = center_ati;
    dest_op->CopySpeeds(op0);    // use the original speeds

    //--------------------------//
    // temporary Flower storage //
    //--------------------------//

    Flower cor_op;
    cor_op.FillProbabilities(0.0);    // we'll be adding

    Flower uncor_op;
    uncor_op.FillProbabilities(1.0);    // we'll be multiplying

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

    //----------------------------------------------//
    // calculate the probability for each direction //
    //----------------------------------------------//

    for (int cti = min_cti; cti < max_cti; cti++)
    {
        for (int ati = min_ati; ati < max_ati; ati++)
        {
            //--------------------------//
            // don't use the center WVC //
            //--------------------------//

            if (cti == center_cti && ati == center_ati)
                continue;

            // assume the information from this wvc will be good
            int bad_wvc = 0;

            Flower* op1 = GetFlower(cti, ati);
            if (op1 == NULL)
                continue;

            float distance = op0->KmDistance(op1);

            //------------------------------------//
            // first, clear the probability array //
            //------------------------------------//

            Flower tmp_op;
            tmp_op.FillProbabilities(0.0);

            for (int dir_idx = 0; dir_idx < DIR_BINS; dir_idx++)
            {
                float speed0 = op0->GetSpeed(dir_idx);
                float direction0 = op0->GetDirection(dir_idx);

                //------------//
                // accumulate //
                //------------//

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
                    if (dist_prob < 0.0)
                    {
                        // can't estimate probs: eliminate this wvc
                        bad_wvc = 1;
                        break;
                    }

                    tmp_op.Add(dir_idx, dist_prob * probability1);
                }
                if (bad_wvc)
                    break;
            }

            if (bad_wvc)
                continue;

            //----------------------------------------------------//
            // form the correlated and uncorrelated probabilities //
            //----------------------------------------------------//

            tmp_op.Normalize();
            cor_op.Add(&tmp_op);
            uncor_op.Multiply(&tmp_op);
            uncor_op.Normalize(MIN_NORM_PROB);   // this should help
        }
    }

    //----------------------------------------//
    // form the combination weighted by gamma //
    //----------------------------------------//

    cor_op.Normalize();
    uncor_op.Normalize();    // just to be sure

    for (int i = 0; i < DIR_BINS; i++)
    {
        dest_op->probabilityArray[i] =
            (1.0 - gamma) * uncor_op.probabilityArray[i] +
            gamma * cor_op.probabilityArray[i];
    }

    dest_op->Normalize();
    return(dest_op);
}

//------------------------------//
// FlowerArray::LocalVectorProb //
//------------------------------//
// gamma controls the assumptions about error correlation. When
// gamma is 0.0, the errors are assumed to be uncorrelated and the
// probabilities are multiplied. When gamma is 1.0, the errors
// are assumed to be completely correlated and the probabilities
// are added. Thanks to Bryan Stiles for helping me with the
// formulation. This one uses the neighbors vectors.

Flower*
FlowerArray::LocalVectorProb(
    DistProb*  dp,
    int        window_size,
    int        center_cti,
    int        center_ati,
    float      gamma)
{
    //----------------//
    // get the center //
    //----------------//

    Flower* op0 = GetFlower(center_cti, center_ati);
    if (op0 == NULL)
        return(NULL);

    //-------------------------------//
    // this is where the output goes //
    //-------------------------------//

    Flower* dest_op = new Flower();
    if (dest_op == NULL)
        return(NULL);
    dest_op->cti = center_cti;
    dest_op->ati = center_ati;
    dest_op->CopySpeeds(op0);    // use the original speeds

    //--------------------------//
    // temporary Flower storage //
    //--------------------------//

    Flower cor_op;
    cor_op.FillProbabilities(0.0);    // we'll be adding

    Flower uncor_op;
    uncor_op.FillProbabilities(1.0);    // we'll be multiplying

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

    //----------------------------------------------//
    // calculate the probability for each direction //
    //----------------------------------------------//

    for (int cti = min_cti; cti < max_cti; cti++)
    {
        for (int ati = min_ati; ati < max_ati; ati++)
        {
            //--------------------------//
            // don't use the center WVC //
            //--------------------------//

            if (cti == center_cti && ati == center_ati)
                continue;

            // assume the information from this wvc will be good
            int bad_wvc = 0;

            Flower* op1 = GetFlower(cti, ati);
            if (op1 == NULL)
                continue;

            float distance = op0->KmDistance(op1);

            //------------------------------------//
            // first, clear the probability array //
            //------------------------------------//

            Flower tmp_op;
            tmp_op.FillProbabilities(0.0);

            for (int dir_idx = 0; dir_idx < DIR_BINS; dir_idx++)
            {
                float speed0 = op0->GetSpeed(dir_idx);
                float direction0 = op0->GetDirection(dir_idx);

                //------------//
                // accumulate //
                //------------//

                int other_dir_idx = op1->GetSelectedDirIdx();
                if (other_dir_idx == -1)
                {
                    // can't estimate probs: eliminate this wvc
                    bad_wvc = 1;
                    break;
                }

                float speed1 = op1->GetSpeed(other_dir_idx);
                float direction1 = op1->GetDirection(other_dir_idx);
                float probability1 = op1->GetProbability(other_dir_idx);

                float dspeed = speed1 - speed0;
                float ddirection = ANGDIF(direction1, direction0);

                float dist_prob = dp->Probability(distance, speed0,
                    dspeed, ddirection);
                if (dist_prob < 0.0)
                {
                    // can't estimate probs: eliminate this wvc
                    bad_wvc = 1;
                    break;
                }

                tmp_op.Add(dir_idx, dist_prob * probability1);
            }

            if (bad_wvc)
                continue;

            //----------------------------------------------------//
            // form the correlated and uncorrelated probabilities //
            //----------------------------------------------------//

            tmp_op.Normalize();
            cor_op.Add(&tmp_op);
            uncor_op.Multiply(&tmp_op);
            uncor_op.Normalize(MIN_NORM_PROB);   // this should help
        }
    }

    //----------------------------------------//
    // form the combination weighted by gamma //
    //----------------------------------------//

    cor_op.Normalize();
    uncor_op.Normalize();    // just to be sure

    for (int i = 0; i < DIR_BINS; i++)
    {
        dest_op->probabilityArray[i] =
            (1.0 - gamma) * uncor_op.probabilityArray[i] +
            gamma * cor_op.probabilityArray[i];
    }

    dest_op->Normalize();

    return(dest_op);
}

//==========//
// DistProb //
//==========//

DistProb::DistProb()
{
    //-------------------------------//
    // clear the count and sum array //
    //-------------------------------//

    for (int i = 0; i < DISTANCE_BINS; i++)
    {
        for (int j = 0; j < SPEED_BINS; j++)
        {
            sum[i][j] = 0;
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
                    sum[i][j] += count[i][j][k][l];
                }
            }
        }
    }
    return;
}

//-----------------------//
// DistProb::Probability //
//-----------------------//
// returns a probability of -1.0 if a real one can't be calculated

float
DistProb::Probability(
    float  distance,
    float  speed,
    float  dspeed,
    float  ddirection)
{
    //-----------------------------------//
    // if you are looking at yourself... //
    //-----------------------------------//

    if (distance == 0.0)
        return(1.0);

    //------------------//
    // get coefficients //
    //------------------//

    int distance_idx[2];
    float distance_coef[2];
    if (! _distanceIndex.GetLinearCoefsStrict(distance, distance_idx,
        distance_coef))
    {
        return(-1.0);
    }

    int speed_idx[2];
    float speed_coef[2];
    if (! _speedIndex.GetLinearCoefsStrict(speed, speed_idx,
        speed_coef))
    {
        return(-1.0);
    }

    int dspeed_idx[2];
    float dspeed_coef[2];
    if (! _dspeedIndex.GetLinearCoefsStrict(dspeed, dspeed_idx,
        dspeed_coef))
    {
        return(-1.0);
    }

    int ddirection_idx[2];
    float ddirection_coef[2];
    if (! _ddirectionIndex.GetLinearCoefsStrict(ddirection, ddirection_idx,
        ddirection_coef))
    {
        return(-1.0);
    }

    //-----------------------//
    // get the probabilities //
    //-----------------------//

    double prob_sum = 0.0;
    for (int di = 0; di < 2; di++)
    {
        int di_idx = distance_idx[di];
        double di_coef = distance_coef[di];
        for (int sp = 0; sp < 2; sp++)
        {
            int sp_idx = speed_idx[sp];
            double sp_coef = speed_coef[sp];
            for (int ds = 0; ds < 2; ds++)
            {
                int ds_idx = dspeed_idx[ds];
                double ds_coef = dspeed_coef[ds];
                for (int dd = 0; dd < 2; dd++)
                {
                    int dd_idx = ddirection_idx[dd];
                    double dd_coef = ddirection_coef[dd];

                    double prob = Probability(di_idx, sp_idx, ds_idx, dd_idx);
                    if (prob < 0.0)
                        return(-1.0);
                    prob_sum += di_coef * sp_coef * ds_coef * dd_coef * prob;
                }
            }
        }
    }

    return(prob_sum);
}

//-----------------------//
// DistProb::Probability //
//-----------------------//

float
DistProb::Probability(
    int  distance_idx,
    int  speed_idx,
    int  dspeed_idx,
    int  ddirection_idx)
{
    double probability;
    if (sum[distance_idx][speed_idx] < MINIMUM_SAMPLES)
        return(-1.0);
    else
        probability =
          (double)count[distance_idx][speed_idx][dspeed_idx][ddirection_idx] /
          (double)sum[distance_idx][speed_idx];

    // since the absolute value of the angular difference was calculated,
    // the probability in the table represents the probability of + and -
    // the delta direction. since we only have the + OR - case, the
    // probability needs to be cut in half. this doesn't affect the zero
    // bin or the 180 bin. symmetry is assumed.
    if (ddirection_idx != 0 && ddirection_idx != DDIRECTION_BINS - 1)
        probability /= 2.0;

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

//---------------------------------//
// DistProb::IndexToDeltaDirection //
//---------------------------------//

float
DistProb::IndexToDeltaDirection(
    int  ddirection_idx)
{
    float ddirection;
    _ddirectionIndex.IndexToValue(ddirection_idx, &ddirection);
    return(ddirection);
}
