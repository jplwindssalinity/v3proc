//==============================================================//
// Copyright (C) 1997-1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_gmf_c[] =
    "@(#) $Id$";

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <malloc.h>
#include <math.h>
#ifdef INTEL86
#include <ieee754.h>
#else
#include <ieeefp.h>
#endif
#include "GMF.h"
#include "GSparameters.h"
#include "Meas.h"
#include "Misc.h"
#include "Interpolate.h"
#include "Constants.h"
#include "Beam.h"
#include "List.h"
#include "AngleInterval.h"
#include "Array.h"

//=====//
// GMF //
//=====//

GMF::GMF()
:   retrieveUsingKpcFlag(1), retrieveUsingKpmFlag(1), retrieveUsingKpriFlag(1),
    retrieveUsingKprsFlag(1), retrieveUsingLogVar(0), retrieveOverIce(0),
    smartNudgeFlag(0), minimumAzimuthDiversity(20.0*dtr), _phiCount(0),
    _phiStepSize(0.0), _spdTol(DEFAULT_SPD_TOL), _sepAngle(DEFAULT_SEP_ANGLE),
    _smoothAngle(DEFAULT_SMOOTH_ANGLE), _maxSolutions(DEFAULT_MAX_SOLUTIONS),
    _bestSpd(NULL), _bestObj(NULL), _copyObj(NULL), _speed_buffer(NULL),
    _objective_buffer(NULL), _dir_mle_maxima(NULL)
{
    SetPhiCount(DEFAULT_PHI_COUNT);

    _speed_buffer = (float*)malloc((MAX_DIR_SAMPLES+1)*sizeof(float));
    _objective_buffer = (float*)malloc((MAX_DIR_SAMPLES+1)*sizeof(float));
    _dir_mle_maxima = (int*)malloc((MAX_DIR_SAMPLES+1)*sizeof(int));
    if (_speed_buffer == NULL || _objective_buffer == NULL ||
        _dir_mle_maxima == NULL)
    {
        fprintf(stderr,"GMF::GMF: Error allocating memory\n");
        exit(1);
    }

    return;
}

GMF::~GMF()
{
    free(_bestSpd);
    free(_bestObj);
    free(_copyObj);

    if (_speed_buffer)
        free(_speed_buffer);
    if (_objective_buffer)
        free(_objective_buffer);
    if (_dir_mle_maxima)
        free(_dir_mle_maxima);

    return;
}

//------------------//
// GMF::SetPhiCount //
//------------------//

int
GMF::SetPhiCount(
    int  phi_count)
{
    if (!_bestSpd && _phiCount == phi_count)
    {
        // No adjustment needed
        return(1);
    }

    _phiCount = phi_count;
    _phiStepSize = two_pi / _phiCount;

    if (_bestSpd)
        free(_bestSpd);
    _bestSpd = (float *)malloc(_phiCount * sizeof(float));
    if (_bestSpd == NULL)
        return(0);

    if (_bestObj)
        free(_bestObj);
    _bestObj = (float *)malloc(_phiCount * sizeof(float));
    if (_bestObj == NULL)
        return(0);

    if (_copyObj)
        free(_copyObj);
    _copyObj = (float *)malloc(_phiCount * sizeof(float));
    if (_copyObj == NULL)
        return(0);

    return(1);
}

//----------------//
// GMF::SetSpdTol //
//----------------//

int
GMF::SetSpdTol(
    float  spd_tol)
{
    _spdTol = spd_tol;
    return(1);
}

//------------------//
// GMF:ReadOldStyle //
//------------------//

int GMF::ReadOldStyle(
    const char*  filename)
{
    int fd = open(filename, O_RDONLY);
    if (fd == -1)
        return(0);

    int dummy;
    read(fd, &dummy, sizeof(int));

    _metCount = 2;

    _incCount = 26;
    _incMin = 16.0 * dtr;
    _incMax = 66.0 * dtr;
    _incStep = 2.0 * dtr;

    _spdCount = 51;
    _spdMin = 0.0;
    _spdMax = 50.0;
    _spdStep = 1.0;

    int file_chi_count = 37;
    _chiCount = 72;
    _chiStep = two_pi / _chiCount;    // 5 degrees

    if (! _Allocate())
        return(0);

    float value;
    for (int met_idx = 0; met_idx < _metCount; met_idx++)
    {
        for (int chi_idx = 0; chi_idx < file_chi_count; chi_idx++)
        {
            for (int spd_idx = 1; spd_idx < _spdCount; spd_idx++)
            {
                for (int inc_idx = 0; inc_idx < _incCount; inc_idx++)
                {
                    if (read(fd, &value, sizeof(float)) != sizeof(float))
                    {
                        close(fd);
                        return(0);
                    }
                    *(*(*(*(_value+met_idx)+inc_idx)+spd_idx)+chi_idx) =
                        (float)value;

                    int chi_idx_2 = (_chiCount - chi_idx) % _chiCount;
                    *(*(*(*(_value+met_idx)+inc_idx)+spd_idx)+chi_idx_2) =
                        (float)value;
                }
            }
        }
    }

    //----------------------//
    // zero the 0 m/s model //
    //----------------------//

    int spd_idx = 0;
    for (int met_idx = 0; met_idx < _metCount; met_idx++)
    {
        for (int chi_idx = 0; chi_idx < _chiCount; chi_idx++)
        {
            for (int inc_idx = 0; inc_idx < _incCount; inc_idx++)
            {
                *(*(*(*(_value+met_idx)+inc_idx)+spd_idx)+chi_idx) = 0.0;
            }
        }
    }

    close(fd);
    return(1);
}

//----------------------//
// GMF:ReadPolarimetric //
//----------------------//

int GMF::ReadPolarimetric(
    const char*  filename)
{
    FILE* ifp = fopen(filename, "r");
    if (! ifp)
        return(0);

    _metCount = 5;

    _incCount = 26;
    _incMin = 14.0 * dtr;
    _incMax = 64.0 * dtr;
    _incStep = 2.0 * dtr;

    _spdCount = 51;
    _spdMin = 0.0;
    _spdMax = 50.0;
    _spdStep = 1.0;

    int file_min_spd_idx = 0;
    _chiCount = 72;
    _chiStep = two_pi / _chiCount;    // 5 degrees

    if (! _Allocate())
        return(0);

    int first_line_of_file=1;
    while(!feof(ifp))
    {
        char string[40];

        float spd, theta, chi, s0hh, s0vv, s0hv, s0vvhv, pvvhv, s0hhvh, phhvh;

        // Read ASCII line of ipnuts and outputs to model function.
        fscanf(ifp,"%s",string);
        if (feof(ifp))
            break;
        spd = atof(string);
        fscanf(ifp,"%s",string);
        theta = atof(string)*dtr;
        fscanf(ifp,"%s",string);
        chi = atof(string)*dtr;
        fscanf(ifp,"%s",string);
        s0hh = atof(string);
        fscanf(ifp,"%s",string);
        s0vv = atof(string);
        fscanf(ifp,"%s",string);
        s0hv = atof(string);
        fscanf(ifp,"%s",string);
        s0hhvh = atof(string);
        fscanf(ifp,"%s",string);
        phhvh = atof(string);
        fscanf(ifp,"%s",string);
        s0vvhv = atof(string);
        fscanf(ifp,"%s",string);
        pvvhv = atof(string);

        // Compute indexes into table
        int ispd, ichi, itheta;
        ispd=_SpdToIndex(spd);
        itheta=_IncToIndex(theta);
        ichi=_ChiToIndex(chi);

        // Compute table values
        s0vv=pow(10.0,0.1*s0vv);
        s0hv=pow(10.0,0.1*s0hv);
        s0hh=pow(10.0,0.1*s0hh);
        if(s0vvhv==-99.0) s0vvhv=0.0;
          else s0vvhv=pow(10.0,0.1*s0vvhv)*SGN(pvvhv);
        if(s0hhvh==-99.0) s0hhvh=0.0;
          else s0hhvh=pow(10.0,0.1*s0hhvh)*SGN(phhvh);

          // Store values in table
          int imet;
          imet=_MetToIndex(Meas::VV_MEAS_TYPE);
          *(*(*(*(_value + imet) + itheta) + ispd) + ichi)=s0vv;
          imet=_MetToIndex(Meas::HH_MEAS_TYPE);
          *(*(*(*(_value + imet) + itheta) + ispd) + ichi)=s0hh;
          imet=_MetToIndex(Meas::HV_MEAS_TYPE);
          *(*(*(*(_value + imet) + itheta) + ispd) + ichi)=s0hv;
          imet=_MetToIndex(Meas::VV_HV_CORR_MEAS_TYPE);
          *(*(*(*(_value + imet) + itheta) + ispd) + ichi)=s0vvhv;
          imet=_MetToIndex(Meas::HH_VH_CORR_MEAS_TYPE);
          *(*(*(*(_value + imet) + itheta) + ispd) + ichi)=s0hhvh;

        if (first_line_of_file)
            file_min_spd_idx = ispd;

        first_line_of_file = 0;
    }

    //----------------------//
    // zero the 0 m/s model //
    //----------------------//

    for (int spd_idx =0; spd_idx < file_min_spd_idx; spd_idx++)
    {
        for (int met_idx = 0; met_idx < _metCount; met_idx++)
        {
            for (int chi_idx = 0; chi_idx < _chiCount; chi_idx++)
            {
                for (int inc_idx = 0; inc_idx < _incCount; inc_idx++)
                {
                    float tmp = *(*(*(*(_value+met_idx) + inc_idx) +
                        file_min_spd_idx)+chi_idx);
                    tmp *= (float)spd_idx/(float)file_min_spd_idx;
                    *(*(*(*(_value+met_idx)+inc_idx)+spd_idx)+chi_idx) = tmp;
                }
            }
        }
    }
    fclose(ifp);
    return(1);
}

//---------------//
// GMF::GetCoefs //
//---------------//

int
GMF::GetCoefs(
    Meas::MeasTypeE  met,
    float            inc,
    float            spd,
    float*           A0,
    float*           A1,
    float*           A1_phase,
    float*           A2,
    float*           A2_phase,
    float*           A3,
    float*           A3_phase,
    float*           A4,
    float*           A4_phase)
{
    float real[5], imag[5];
    int n = _chiCount - 1;
    float wn = two_pi / n;

    for (int i = 0; i < 5; i++)
    {
        real[i] = 0.0;
        imag[i] = 0.0;

        // assumes single point overlap in chi
        for (int chi_idx = 0; chi_idx < n; chi_idx++)
        {
            float arg = wn * (float)i * (float)chi_idx;
            float c = cos(arg);
            float s = sin(arg);
            float chi = (float)chi_idx * _chiStep;
            float val;
            GetInterpolatedValue(met, inc, spd, chi, &val);
            real[i] += val * c;
            imag[i] += val * s;
        }
    }

    *A0 = real[0] / (float)n;
    *A1 = 2.0 * sqrt(real[1] * real[1] + imag[1] * imag[1]) / (float)n;
    *A1_phase = -atan2(imag[1], real[1]);
    *A2 = 2.0 * sqrt(real[2] * real[2] + imag[2] * imag[2]) / (float)n;
    *A2_phase = -atan2(imag[2], real[2]);
    *A3 = 2.0 * sqrt(real[3] * real[3] + imag[3] * imag[3]) / (float)n;
    *A3_phase = -atan2(imag[3], real[3]);
    *A4 = 2.0 * sqrt(real[4] * real[4] + imag[4] * imag[4]) / (float)n;
    *A4_phase = -atan2(imag[4], real[4]);

    return(1);
}

//---------------//
// GMF::WritePdf //
//---------------//

int
GMF::WritePdf(
    FILE*      ofp,
    MeasList*  meas_list,
    Kp*        kp)
{
    //-------------------------------//
    // Set objective function values //
    //-------------------------------//

    WVC* wvc = new WVC();
    if (! RetrieveWinds_PE(meas_list, kp, wvc))
        return(0);

    //--------------------------------------------//
    // Determine Maximum Objective Function value //
    //--------------------------------------------//

    double scale = 0;
    WindVectorPlus* head = wvc->ambiguities.GetHead();
    if (head != NULL)
        scale=head->obj;
    else
        return(0);

    //---------------------------------------//
    // Calculate and print out probabilities //
    //---------------------------------------//

    float sum = 0;
    for (int i = 0; i < _phiCount; i++)
    {
        _bestObj[i] = exp((_bestObj[i] - scale) / 2);
        sum += _bestObj[i];
    }
    for (int i = 0; i < _phiCount; i++)
    {
        fprintf(ofp, "%g %g\n", (float)i * _phiStepSize * rtd,
            _bestObj[i] / sum);
    }

    fprintf(ofp, "&\n");
    delete wvc;
    return(1);
}

//--------------------------//
// GMF::WriteSolutionCurves //
//--------------------------//

int
GMF::WriteSolutionCurves(
    FILE*      ofp,
    MeasList*  meas_list,
    Kp*        kp)
{
    //------------------------------//
    // generate each solution curve //
    //------------------------------//

    MeasList new_list;
    for (Meas* meas = meas_list->GetHead(); meas; meas = meas_list->GetNext())
    {
        new_list.Append(meas);
        SolutionCurve(&new_list, kp);
        for (int i = 0; i < _phiCount; i++)
        {
            fprintf(ofp, "%g %g\n", (float)i * _phiStepSize * rtd,
                _bestSpd[i]);
        }
        fprintf(ofp, "&\n");
        new_list.GetHead();
        new_list.RemoveCurrent();
    }

    //----------------------------------//
    // generate combined solution curve //
    //----------------------------------//

    SolutionCurve(meas_list, kp);
    float min_obj = _bestObj[0];
    float max_obj = _bestObj[0];
    for (int i = 0; i < _phiCount; i++)
    {
        if (_bestObj[i] < min_obj)
            min_obj = _bestObj[i];
        if (_bestObj[i] > max_obj)
            max_obj = _bestObj[i];
    }
    float scale = 1.0 / (max_obj - min_obj);
    for (int i = 0; i < _phiCount; i++)
    {
        fprintf(ofp, "%g %g %g\n", (float)i * _phiStepSize * rtd,
            _bestSpd[i], (_bestObj[i] - min_obj) * scale);
    }

    //--------------------------------------//
    // generate smoothed objective function //
    //--------------------------------------//

    Smooth();
    min_obj = _bestObj[0];
    max_obj = _bestObj[0];
    for (int i = 0; i < _phiCount; i++)
    {
        if (_bestObj[i] < min_obj)
            min_obj = _bestObj[i];
        if (_bestObj[i] > max_obj)
            max_obj = _bestObj[i];
    }
    scale = 1.0 / (max_obj - min_obj);
    fprintf(ofp, "&\n");
    for (int i = 0; i < _phiCount; i++)
    {
        fprintf(ofp, "%g %g\n", (float)i * _phiStepSize * rtd,
            (_bestObj[i] - min_obj) * scale);
    }

    //----------------------------//
    // write individual solutions //
    //----------------------------//

    WVC* wvc = new WVC();
    RetrieveWinds_PE(meas_list, kp, wvc);
    for (WindVectorPlus* wvp = wvc->ambiguities.GetHead(); wvp;
        wvp = wvc->ambiguities.GetNext())
    {
        fprintf(ofp, "&\n");
        fprintf(ofp, "%g %g\n", wvp->dir * rtd, wvp->spd);
    }

    //---------------//
    // delete things //
    //---------------//

    delete wvc;

    return(1);
}

//-------------------//
// GMF::GetObjLimits //
//-------------------//

int
GMF::GetObjLimits(
    float*  min_obj,
    float*  max_obj)
{
    *min_obj = _bestObj[0];
    *max_obj = _bestObj[0];
    for (int i = 0; i < _phiCount; i++)
    {
        if (_bestObj[i] < *min_obj)
            *min_obj = _bestObj[i];
        if (_bestObj[i] > *max_obj)
            *max_obj = _bestObj[i];
    }

    return(1);
}

//--------------------------//
// GMF::WriteObjectiveCurve //
//--------------------------//

int
GMF::WriteObjectiveCurve(
    FILE*  ofp,
    float  min_obj,
    float  max_obj)
{

    float scale;
    if (min_obj == 0.0 && max_obj == 0.0)
    {
        scale = 1.0;
    }
    else if (min_obj == max_obj)
    {
        fprintf(stderr,"GMF::WriteObjectiveCurve: Invalid obj limits\n");
        exit(1);
    }
    else
    {
        scale = 1.0 / (max_obj - min_obj);
    }
    for (int i = 0; i < _phiCount; i++)
    {
        fprintf(ofp, "%g %g\n", (float)i * _phiStepSize * rtd,
            (_bestObj[i] - min_obj) * scale);
    }

    return(1);
}

//----------------------------//
// GMF::WriteGSObjectiveCurve //
//----------------------------//

int
GMF::WriteGSObjectiveCurve(
    FILE*        ofp,
    float        min_obj,
    float        max_obj)
{

    float dir_spacing =  wind_dir_intv_init;
    int num_dir_samples = (int)(360. / dir_spacing) + 2 ;

    float scale;
    if (min_obj == 0.0 && max_obj == 0.0)
    {
        scale = 1.0;
    }
    else if (min_obj == max_obj)
    {
        fprintf(stderr,"GMF::WriteGSObjectiveCurve: Invalid obj limits\n");
        exit(1);
    }
    else
    {
        scale = 1.0 / (max_obj - min_obj);
    }

    for (int i = 2; i <= num_dir_samples; i++)
    {
        float angle = dir_spacing * (float)(i - 1) - dir_spacing;
        fprintf(ofp, "%g %g\n", angle,
            (_objective_buffer[i] - min_obj) * scale);
    }

    return(1);
}

//----------------------//
// GMF::AppendSolutions //
//----------------------//

int
GMF::AppendSolutions(
    FILE*  ofp,
    WVC*   wvc,
    float  min_obj,
    float  max_obj)
{
    float scale;
    if (min_obj == 0.0 && max_obj == 0.0)
    {
        scale = 1.0;
    }
    else if (min_obj == max_obj)
    {
        fprintf(stderr,"GMF::AppendSolutions: Invalid obj limits\n");
        exit(1);
    }
    else
    {
        scale = 1.0 / (max_obj - min_obj);
    }

    //----------------------------//
    // write individual solutions //
    //----------------------------//

    for (WindVectorPlus* wvp = wvc->ambiguities.GetHead(); wvp;
        wvp = wvc->ambiguities.GetNext())
    {
        fprintf(ofp, "%g %g\n", wvp->dir * rtd, (wvp->obj - min_obj) * scale);
    }

    return(1);
}

//----------------------------//
// GMF::CheckRetrieveCriteria //
//----------------------------//
// Returns 1 if wind retrieval should be performed, Otherwise 0

int
GMF::CheckRetrieveCriteria(
    MeasList*    meas_list)
{
    //------------------------------------------//
    // check for minimum number of measurements //
    //------------------------------------------//

    if (meas_list->NodeCount() < MINIMUM_WVC_MEASUREMENTS)
        return(0);

    //-------------------------------------//
    // check for land contamination        //
    //-------------------------------------//
        for (Meas* meas=meas_list->GetHead(); meas;
             meas=meas_list->GetNext()){
      if(!retrieveOverIce && meas->landFlag!=0) return(0);
          else if(meas->landFlag==1 || meas->landFlag==3) return(0);
          // For SeaIce landFlag=2
    }

    Node<Meas>* current;

    for (Meas* meas1 = meas_list->GetHead(); meas1;
        meas1 = meas_list->GetNext())
    {
        // remember the current
        current = meas_list->GetCurrentNode();

        for (Meas* meas2 = meas_list->GetHead(); meas2;
            meas2 = meas_list->GetNext())
        {
            float azdiv = ANGDIF(meas1->eastAzimuth, meas2->eastAzimuth);
            if (azdiv >= minimumAzimuthDiversity)
            {
                goto passed;
                break;
            }
        }

        // restore the current
        meas_list->SetCurrentNode(current);
    }
    return(0);        // failed diversity check

    passed:            // passed diversity check

    return(1);
}

//-----------------------//
// GMF::RetrieveWinds_PE //
//-----------------------//

int
GMF::RetrieveWinds_PE(
    MeasList*    meas_list,
    Kp*            kp,
    WVC*        wvc)
{
    //-------------------------//
    // find the solution curve //
    //-------------------------//

    if (! SolutionCurve(meas_list, kp))
    {
        fprintf(stderr, "can't find solution curve\n");
        return(0);
    }

    //---------------------------//
    // smooth the solution curve //
    //---------------------------//

    if (! Smooth())
    {
//        fprintf(stderr, "can't smooth solution curve\n");
        return(0);
    }

    //--------------------------------//
    // find maxima and add to the wvc //
    //--------------------------------//

    if (! FindMaxima(wvc))
    {
        fprintf(stderr, "can't find maxima\n");
        return(0);
    }

    //------------------------------------------//
    // sort the solutions by objective function //
    //------------------------------------------//

    wvc->SortByObj();

    return(1);
}

//------------------------//
// GMF::RetrieveManyWinds //
//------------------------//

int
GMF::RetrieveManyWinds(
    MeasList*  meas_list,
    Kp*        kp,
    WVC*       wvc)
{
    //-------------------------//
    // find the solution curve //
    //-------------------------//

    if (! SolutionCurve(meas_list, kp))
    {
        fprintf(stderr, "can't find solution curve\n");
        return(0);
    }

    //---------------------------//
    // smooth the solution curve //
    //---------------------------//

    if (! Smooth())
        return(0);

    //--------------------------------------//
    // find many vectors and add to the wvc //
    //--------------------------------------//

    if (! FindMany(wvc))
        return(0);

    //------------------------------------------//
    // sort the solutions by objective function //
    //------------------------------------------//

    wvc->SortByObj();

    return(1);
}

/********* Commented out old version
//-------------------------------------//
// GMF::RetrieveWindsWithPeakSplitting //
//-------------------------------------//
int
GMF::RetrieveWindsWithPeakSplitting(
    MeasList*         meas_list,
        Kp*                      kp,
        WVC*                    wvc,
        float                 one_peak_width,
        float                 two_peak_separation_threshold,
    float                 threshold)

{
        if(! RetrieveWinds(meas_list,kp,wvc)) return(0);
        float two_peak_width=one_peak_width/3;
        float one_peak_separation_threshold=2.0*two_peak_width;

    //--------------------------------------//
    // determine number of ambiguities with  //
    // scaled probability values greater than //
    // threshold                              //
    //----------------------------------------//

    int num_peaks=0;
    float top_peaks_dir[2];

    float obj,scale=0;
        WindVectorPlus* head=wvc->ambiguities.GetHead();
        if(head!=NULL) scale=head->obj;
    for(WindVectorPlus* wvp=wvc->ambiguities.GetHead();
        wvp; wvp=wvc->ambiguities.GetNext()){
      obj=wvp->obj;
      float prob=exp((obj-scale)/2);
      if (prob > threshold){
        if(num_peaks<2) top_peaks_dir[num_peaks]=wvp->dir;
        num_peaks++;
      }
    }

    //-------------------------------------------------------------//
        // Case 1: Two peaks closer together than one peak_separation_ //
        // threshold. They are combined to form a single peak and Case //
        // 2 is applied.                                               //
        //-------------------------------------------------------------//

    if(num_peaks==2 && fabs(ANGDIF(top_peaks_dir[0],top_peaks_dir[1]))<
       one_peak_separation_threshold){
      top_peaks_dir[0]=(top_peaks_dir[0]+top_peaks_dir[1])/2;
      if(fabs(ANGDIF(top_peaks_dir[1],top_peaks_dir[0])) > pi/2)
        top_peaks_dir[0]+=pi;
      if(top_peaks_dir[0]>two_pi) top_peaks_dir[0]-=two_pi;
      num_peaks=1;
    }

        //------------------------------------------------------------//
        // Case 2: One Peak                                           //
        //------------------------------------------------------------//
    if(num_peaks==1){
      float dir_start= top_peaks_dir[0] - 0.5*one_peak_width;
          float dir_step = one_peak_width/3.0;
          WindVectorPlus* wvp=wvc->ambiguities.GetHead();
      for(int c=0;c<4;c++){
        float dir=dir_start+dir_step*c;
        while(dir<0) dir+=two_pi;
        while(dir>two_pi) dir-=two_pi;
            int phi_idx=(int)(dir/two_pi*_phiCount+0.5);
            if(phi_idx==_phiCount) phi_idx=0;
        if(wvp){
          wvp->spd=_bestSpd[phi_idx];
          wvp->dir=dir;
              wvp->obj=_bestObj[phi_idx];
          wvp=wvc->ambiguities.GetNext();
        }
        else{
          wvp= new WindVectorPlus();
          wvp->spd=_bestSpd[phi_idx];
          wvp->dir=dir;
              wvp->obj=_bestObj[phi_idx];
          wvc->ambiguities.Append(wvp);
          wvp=NULL;
        }
      }
    }

    //-------------------------------------------------//
        // Case 3: two peaks less than two peak separation //
        // threshold apart which do not fit Case 1:        //
        //-------------------------------------------------//
        if(num_peaks==2 &&
           fabs(ANGDIF(top_peaks_dir[0],top_peaks_dir[1]))<
           two_peak_separation_threshold){

                  WindVectorPlus* wvp=wvc->ambiguities.GetHead();

                  float dir_start= top_peaks_dir[0] - 0.5*two_peak_width;
                  float dir_step = two_peak_width;
          for(int c=0;c<2;c++){
            float dir=dir_start+dir_step*c;
                    while(dir<0) dir+=two_pi;
                    while(dir>two_pi) dir-=two_pi;
            int phi_idx=(int)(dir/two_pi*_phiCount+0.5);
            if(phi_idx==_phiCount) phi_idx=0;
                    if(wvp){
              wvp->spd=_bestSpd[phi_idx];
              wvp->dir=dir;
              wvp->obj=_bestObj[phi_idx];
              wvp=wvc->ambiguities.GetNext();
            }
            else{
              wvp= new WindVectorPlus();
              wvp->spd=_bestSpd[phi_idx];
              wvp->dir=dir;
              wvp->obj=_bestObj[phi_idx];
                      wvc->ambiguities.Append(wvp);
                      wvp=NULL;
            }
          }

                  dir_start= top_peaks_dir[1] - 0.5*two_peak_width;
                  dir_step = two_peak_width;
          for(int c=0;c<2;c++){
            float dir=dir_start+dir_step*c;
                    while(dir<0) dir+=two_pi;
                    while(dir>two_pi) dir-=two_pi;
            int phi_idx=(int)(dir/two_pi*_phiCount+0.5);
            if(phi_idx==_phiCount) phi_idx=0;
                    if(wvp){
              wvp->spd=_bestSpd[phi_idx];
              wvp->dir=dir;
              wvp->obj=_bestObj[phi_idx];
              wvp=wvc->ambiguities.GetNext();
            }
            else{
              wvp= new WindVectorPlus();
              wvp->spd=_bestSpd[phi_idx];
              wvp->dir=dir;
              wvp->obj=_bestObj[phi_idx];
                      wvc->ambiguities.Append(wvp);
                      wvp=NULL;
            }
          }
        }

        //----------------------------------//
                // Case 4: Everything else          //
                // Do not change ambiguities.       //
                //----------------------------------//

        //------------------------------------------//
        // sort the solutions by objective function //
        //------------------------------------------//

        wvc->SortByObj();
        return(1);
}
Commented out old version       ********/

//------------------------------//
// GMF::_ObjectiveToProbability //
//------------------------------//

int
GMF::ConvertObjToPdf()
{
    float sum = 0.0;
    float scale = _bestObj[0];
    for (int c = 1; c < _phiCount; c++)
    {
        if (scale < _bestObj[c])
            scale = _bestObj[c];
    }
    for (int c = 0; c < _phiCount; c++)
    {
        _bestObj[c] = exp((_bestObj[c] - scale) / 2);
        sum += _bestObj[c];
    }
    for (int c = 0; c < _phiCount; c++)
    {
        _bestObj[c] /= sum;
    }
    return(1);
}

//------------------------------//
// GMF::_ObjectiveToProbability //
//------------------------------//

int
GMF::_ObjectiveToProbability(
    float  scale,
    int    radius)
{
  float sum=0;
  for(int c=0;c<_phiCount;c++){
    *(_bestObj+c)=exp((*(_bestObj+c)-scale)/2);
    sum+=*(_bestObj+c);
  }
  for(int c=0;c<_phiCount;c++){
    *(_bestObj+c)/=sum;
  }
  float* tmp_buffer=new float[_phiCount];
  for(int c=0;c<_phiCount;c++){
    *(tmp_buffer+c)=0;
    for(int r=-radius;r<=radius;r++){
      int offset=c+r;
      if(offset<0) offset+=_phiCount;
      if(offset>=_phiCount) offset-=_phiCount;
      *(tmp_buffer+c)+=*(_bestObj+offset);
    }
  }
  for(int c=0;c<_phiCount;c++){
    *(_bestObj+c)=*(tmp_buffer+c);
  }
  delete(tmp_buffer);
  return(1);
}

//-------------------------------------//
// GMF::RetrieveWindsWithPeakSplitting //
//-------------------------------------//

int
GMF::RetrieveWindsWithPeakSplitting(
    MeasList*  meas_list,
    Kp*        kp,
    WVC*       wvc,
    float      one_peak_width,
    float      two_peak_separation_threshold,
    float      threshold,
    int        max_num_ambigs)
{
        if(! RetrieveWinds_PE(meas_list,kp,wvc)) return(0);
        int one_peak_radius=(int)(one_peak_width/(2*_phiStepSize) +0.5);

        //----------------------------------------------------------------//
        // Convert Objective Function Values to Scaled Probability Values //
    //----------------------------------------------------------------//
    float scale=0;
        WindVectorPlus* head=wvc->ambiguities.GetHead();
        if(head!=NULL) scale=head->obj;
    _ObjectiveToProbability(scale,one_peak_radius);

    //--------------------------------------//
    // determine number of ambiguities with  //
    // scaled probability values greater than //
    // threshold and remove ambiguities that  //
    // are below the threshold                //
    //----------------------------------------//

    int num_peaks=0;


        WindVectorPlus* wvp=wvc->ambiguities.GetHead();

    while(wvp){
      int phi_idx=(int)(wvp->dir/_phiStepSize +0.5);
      float prob=*(_bestObj+phi_idx);
      if (prob > threshold){
        num_peaks++;
        wvp->obj=prob;
        wvp=wvc->ambiguities.GetNext();
      }
      else{
        wvp=wvc->ambiguities.RemoveCurrent();
        delete wvp;
        wvp=wvc->ambiguities.GetCurrent();
      }
    }

    //------------------------------------------//
        // Add ambiguities until the maximum number //
        // of ambiguities is reached.               //
        //------------------------------------------//

        while(num_peaks<max_num_ambigs){
      float max_prob=0;
          int max_offset=0;
          for(int c=0;c<_phiCount;c++){

            // check to see if the direction is already represented by
            // a peak
        int available=1;
            float dir=(float)c*_phiStepSize;
        for(wvp=wvc->ambiguities.GetHead();wvp;
        wvp=wvc->ambiguities.GetNext()){
          if(fabs(ANGDIF(wvp->dir,dir))<two_peak_separation_threshold){
        available=0;
          }
        }
            // determine the available direction with the maximum probability
        if(available==1 && *(_bestObj+c)>max_prob){
          max_prob=*(_bestObj+c);
          max_offset=c;
        }
      }
      //------------//
      // add to wvc //
      //------------//

      wvp = new WindVectorPlus();
      if (! wvp)
        return(0);
      wvp->spd = _bestSpd[max_offset];
      wvp->dir = (float)max_offset * _phiStepSize;
      wvp->obj = _bestObj[max_offset];
      if (! wvc->ambiguities.Append(wvp))
        {
          delete wvp;
          return(0);
        }
      num_peaks++;
    }


    //--------------------------------------------//
    // sort the solutions by probability          //
    //--------------------------------------------//

    wvc->SortByObj();
    return(1);
}

//--------------------//
// GMF::SolutionCurve //
//--------------------//
// For each of phi_count directions, find the speed that maximizes the
// objective function.  Fills in the _bestSpd and _bestObj arrays

int
GMF::SolutionCurve(
    MeasList*    meas_list,
    Kp*            kp)
{
    //-------------------------------//
    // bracket maxima with certainty //
    //-------------------------------//

    float ax = _spdMin;
    float cx = _spdMax;

    //-----------------------//
    // for each direction... //
    //-----------------------//

    for (int phi_idx = 0; phi_idx < _phiCount; phi_idx++)
    {
        float bx = ax + (cx - ax) * golden_r;
        float phi = (float)phi_idx * _phiStepSize;

        //------------------------------------------//
        // ...widen so that the maxima is bracketed //
        //------------------------------------------//

        if (_ObjectiveFunction(meas_list, bx, phi, kp) <
            _ObjectiveFunction(meas_list, ax, phi, kp) )
        {
            ax = _spdMin;
        }
        if (_ObjectiveFunction(meas_list, bx, phi, kp) <
            _ObjectiveFunction(meas_list, cx, phi, kp) )
        {
            cx = _spdMax;
        }

        //------------------------//
        // ...find the best speed //
        //------------------------//

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
        float f1 = _ObjectiveFunction(meas_list, x1, phi, kp);
        float f2 = _ObjectiveFunction(meas_list, x2, phi, kp);

        while (x3 - x0 > _spdTol)
        {
            if (f2 > f1)
            {
                x0 = x1;
                x1 = x2;
                x2 = x2 + golden_c * (x3 - x2);
                f1 = f2;
                f2 = _ObjectiveFunction(meas_list, x2, phi, kp);
            }
            else
            {
                x3 = x2;
                x2 = x1;
                x1 = x1 - golden_c * (x1 - x0);
                f2 = f1;
                f1 = _ObjectiveFunction(meas_list, x1, phi, kp);
            }
        }

        if (f1 > f2)
        {
            _bestSpd[phi_idx] = x1;
            _bestObj[phi_idx] = f1;
        }
        else
        {
            _bestSpd[phi_idx] = x2;
            _bestObj[phi_idx] = f2;
        }
        // the additional 0.5 prevents near zero speeds from having nearly
        // identical ax and cx
        ax = x1 - (x1 * 0.05) - 0.5;
        if (ax < _spdMin)
            ax = _spdMin;
        cx = x1 + (x2 * 0.05) + 0.5;
        if (cx > _spdMax)
            cx = _spdMax;
    }
    return(1);
}

//-------------//
// GMF::Smooth //
//-------------//
// Smooths the solution curve until the desired number (or less) of
// solutions are obtained.  Returns the number of solutions found.
// phi_buffer is one sided (i.e. 5 degrees means 5 degrees to either side)

int
GMF::Smooth()
{
    //---------------------------//
    // calculate some parameters //
    //---------------------------//

    int max_delta_phi = (int)(_sepAngle / _phiStepSize + 0.5);
    int max_smoothing_phi = (int)(_smoothAngle / _phiStepSize + 0.5);
    int delta_phi = 0;        // start with no smoothing

    int flag = 0;
    int return_value = 0;

    //----------------------------------------------------//
    // loop until desired number of solutions is obtained //
    //----------------------------------------------------//

    for (;;)
    {
        //-------------------------------//
        // count the number of solutions //
        //-------------------------------//

        int maxima_count = 0;
        int local_maxima_count = 0;
        for (int phi_idx = 0; phi_idx < _phiCount; phi_idx++)
        {
            //--------------------------//
            // check for 3 point maxima //
            //--------------------------//

            int idx_minus = (phi_idx - 1 + _phiCount) % _phiCount;
            int idx_plus = (phi_idx + 1) % _phiCount;
            if (_bestObj[phi_idx] > _bestObj[idx_minus] &&
                _bestObj[phi_idx] > _bestObj[idx_plus])
            {
                maxima_count++;

                //------------------------//
                // check for local maxima //
                //------------------------//

                int isa_max = 1;
                for (int offset = 2; offset <= max_delta_phi; offset++)
                {
                    idx_minus = (phi_idx - offset + _phiCount) % _phiCount;
                    idx_plus = (phi_idx + offset) % _phiCount;
                    if (_bestObj[phi_idx] < _bestObj[idx_minus] ||
                        _bestObj[phi_idx] < _bestObj[idx_plus])
                    {
                        // not a maxima
                        isa_max = 0;
                        break;
                    }
                }
                if (isa_max)
                    local_maxima_count++;
            }
        }

        if (maxima_count == local_maxima_count &&
            maxima_count <= _maxSolutions)
        {
            //----------------------------------//
            // done -- scale objective function //
            //----------------------------------//

            if (flag)
            {
                // scaling was performed
                float scale = (float)(1 + delta_phi * 2);
                for (int phi_idx = 0; phi_idx < _phiCount; phi_idx++)
                {
                    _bestObj[phi_idx] /= scale;
                }
            }
            return_value = 1;
            break;
        }
        else
        {
            //-----------------------------//
            // check if too much smoothing //
            //-----------------------------//

            delta_phi++;
            if (delta_phi > max_smoothing_phi)
                break;

            //--------//
            // smooth //
            //--------//

            if (! flag)
            {
                memcpy(_copyObj, _bestObj, _phiCount * sizeof(float));
                flag = 1;
            }

            for (int phi_idx = 0; phi_idx < _phiCount; phi_idx++)
            {
                int phi_idx_plus = (phi_idx + delta_phi) % _phiCount;
                int phi_idx_minus = (phi_idx - delta_phi + _phiCount) %
                    _phiCount;
                _bestObj[phi_idx] += _copyObj[phi_idx_minus];
                _bestObj[phi_idx] += _copyObj[phi_idx_plus];
            }
        }
    }
    return(return_value);
}

//-----------------//
// GMF::FindMaxima //
//-----------------//

int
GMF::FindMaxima(
    WVC*        wvc)
{
    for (int phi_idx = 0; phi_idx < _phiCount; phi_idx++)
    {
        //--------------------------//
        // check for 3 point maxima //
        //--------------------------//

        int idx_minus = (phi_idx - 1 + _phiCount) % _phiCount;
        int idx_plus = (phi_idx + 1) % _phiCount;
        if (_bestObj[phi_idx] > _bestObj[idx_minus] &&
            _bestObj[phi_idx] > _bestObj[idx_plus])
        {
            //------------//
            // add to wvc //
            //------------//

            WindVectorPlus* wvp = new WindVectorPlus();
            if (! wvp)
                return(0);
            wvp->spd = _bestSpd[phi_idx];
            wvp->dir = (float)phi_idx * _phiStepSize;
            wvp->obj = _bestObj[phi_idx];
            if (! wvc->ambiguities.Append(wvp))
            {
                delete wvp;
                return(0);
            }
        }
    }
    return(1);
}

//---------------//
// GMF::FindMany //
//---------------//

int
GMF::FindMany(
    WVC*        wvc)
{
    //---------------------------------------//
    // find maximum objective function value //
    //---------------------------------------//
    // this is used to prevent overflow

    double max_obj = _bestObj[0];
    for (int phi_idx = 0; phi_idx < _phiCount; phi_idx++)
    {
        if (_bestObj[phi_idx] > max_obj)
            max_obj = _bestObj[phi_idx];
    }

    //-----------------------------------//
    // convert objective function to pdf //
    //-----------------------------------//

    double sum = 0.0;
    for (int phi_idx = 0; phi_idx < _phiCount; phi_idx++)
    {
        double expo = (_bestObj[phi_idx] - max_obj) / 2.0;
        _bestObj[phi_idx] = exp(expo);
        sum += _bestObj[phi_idx];
    }

    //---------------------------------------//
    // "integrate" over bin width by scaling //
    //---------------------------------------//

    for (int phi_idx = 0; phi_idx < _phiCount; phi_idx++)
    {
        if (sum > 0.0)
            _bestObj[phi_idx] /= sum;
        else
            _bestObj[phi_idx] = 0.0;
    }

    //------------//
    // add to wvc //
    //------------//

    for (int phi_idx = 0; phi_idx < _phiCount; phi_idx++)
    {
        //------------//
        // add to wvc //
        //------------//

        WindVectorPlus* wvp = new WindVectorPlus();
        if (! wvp)
            return(0);
        wvp->spd = _bestSpd[phi_idx];
        wvp->dir = (float)phi_idx * _phiStepSize;
        wvp->obj = _bestObj[phi_idx];

        if (! wvc->ambiguities.Append(wvp))
        {
            delete wvp;
            return(0);
        }
    }
    return(1);
}
//------------------//
// GMF::GetVariance //
//------------------//

int global_debug=0;  // GLOBAL DEBUG FLAG

float
GMF::GetVariance(
    Meas*  meas,
    float  spd,
    float  chi,
    float  trial_sigma0,
    Kp*    kp)
{
    //-------------------------------------------------------//
    // calculate the expected variance for the trial sigma-0 //
    //-------------------------------------------------------//

    if ( !kp)
        return(0.0); // If kp is NULL returns 0.0

    //--------------//
    // Kpc variance //
    //--------------//

    double vpc = 0.0;
    float s0_co, s0_x;
    if (retrieveUsingKpcFlag && meas->numSlices != -1)
    {
        switch (meas->measType)
        {
        case Meas::VV_MEAS_TYPE:
        case Meas::HH_MEAS_TYPE:
        case Meas::VH_MEAS_TYPE:
        case Meas::HV_MEAS_TYPE:
            if (! kp->GetVpc(meas, trial_sigma0, &vpc))
            {
                fprintf(stderr, "GMF::GetVariance: Error computing Vpc\n");
                vpc = 0.0;
            }
            break;
        case Meas::VV_HV_CORR_MEAS_TYPE:
            GetInterpolatedValue(Meas::VV_MEAS_TYPE, meas->incidenceAngle,
                spd, chi, &s0_co);
            GetInterpolatedValue(Meas::HV_MEAS_TYPE, meas->incidenceAngle,
                spd, chi, &s0_x);
            if (! kp->GetVpc(meas, trial_sigma0, s0_co, s0_x, &vpc))
            {
                fprintf(stderr, "GMF::GetVariance: Error computing Vpc\n");
                vpc = 0.0;
            }
            break;
        case Meas::HH_VH_CORR_MEAS_TYPE:
            GetInterpolatedValue(Meas::HH_MEAS_TYPE, meas->incidenceAngle,
                spd, chi, &s0_co);
            GetInterpolatedValue(Meas::VH_MEAS_TYPE, meas->incidenceAngle,
                spd, chi, &s0_x);
            if (! kp->GetVpc(meas, trial_sigma0, s0_co, s0_x, &vpc))
            {
                fprintf(stderr, "GMF::GetVariance: Error computing Vpc\n");
                vpc = 0.0;
            }
            break;
        default:
            fprintf(stderr, "GMF::GetVariance: Bad Measurement Type\n");
            exit(1);
            break;
        }
    }

    //-----//
    // Kpm //
    //-----//

    double vpm = 0.0;
    if (retrieveUsingKpmFlag && meas->numSlices!=-1)
    {
        double kpm2 = 0.0;
        if (! kp->GetKpm2(meas->measType, spd, &kpm2))
        {
            fprintf(stderr,"GMF::GetVariance: Error computing Kpm\n");
            kpm2 = 0.0;
        }
        switch(meas->measType)
        {
        case Meas::VV_HV_CORR_MEAS_TYPE:
        case Meas::HH_VH_CORR_MEAS_TYPE:
            float kpm_sig0;
            GetMaxValueForSpeed(meas->measType, meas->incidenceAngle, spd,
                &kpm_sig0);
            vpm = kpm2 * kpm_sig0*kpm_sig0;
            break;
        default:
            vpm = kpm2 * trial_sigma0 * trial_sigma0;
            break;
        }
    }

    //------//
    // Kpri //
    //------//

    double kpri2 = 0.0;
    if (retrieveUsingKpriFlag && meas->numSlices!=-1)
    {
        if (! kp->GetKpri2(&kpri2))
        {
            fprintf(stderr,"GMF::GetVariance: Error computing Kpri2\n");
            kpri2=0.0;
        }
    }

    //------//
    // Kprs //
    //------//

    double kprs2 = 0.0;
    if (retrieveUsingKprsFlag && meas->numSlices!=-1)
    {
        if (! kp->GetKprs2(meas, &kprs2))
        {
            fprintf(stderr, "GMF::GetVariance: Error computing Kprs2\n");
            kprs2 = 0.0;
        }
    }

    //------------------------//
    // calculate the variance //
    //------------------------//

    double var;
    if (meas->numSlices == -1)
    {
        float kpm = 0.16;
        float alpha = (1.0 + kpm*kpm) * meas->A - 1.0;
        var = (alpha*trial_sigma0 + meas->B) * trial_sigma0 + meas->C;
    }
    else
    {
        var = (trial_sigma0*trial_sigma0 + vpc + vpm) * (1.0 + kpri2) *
            (1.0 + kprs2) - trial_sigma0*trial_sigma0;
    }
    if (global_debug)
    {
        printf("%g %g %g %g %g %g ", trial_sigma0, vpc, kpri2, kprs2, vpm,
            var);
    }
    return(var);
}

//-------------------------//
// GMF::_ObjectiveFunction //
//-------------------------//

float
GMF::_ObjectiveFunction(
    MeasList*  meas_list,
    float      spd,
    float      phi,
    Kp*        kp)
{
    //-----------------------------------------//
    // initialize the objective function value //
    //-----------------------------------------//

    float fv = 0.0;

    //-------------------------//
    // for each measurement... //
    //-------------------------//

    for (Meas* meas = meas_list->GetHead(); meas; meas = meas_list->GetNext())
    {
        //---------------------------------------//
        // get sigma-0 for the trial wind vector //
        //---------------------------------------//

        float chi = phi - meas->eastAzimuth + pi;
        float trial_value;
        GetInterpolatedValue(meas->measType, meas->incidenceAngle, spd, chi,
            &trial_value);

        //------------------------------------------------------------//
        // find the difference between the trial and measured sigma-0 //
        //------------------------------------------------------------//

        // Sanity check on measurement
        double tmp=meas->value;
        if (! finite(tmp))
            continue;

        float s = trial_value - meas->value;

        //-------------------------------------------------------//
        // calculate the expected variance for the trial sigma-0 //
        //-------------------------------------------------------//

        float var = GetVariance(meas, spd, chi, trial_value, kp);
        // returns 0 if kp is NULL

        if (var == 0.0)
        {
            // variances all turned off, so use uniform weighting.
            fv += s*s;
        }
        else if (retrieveUsingLogVar)
        {
            fv += s*s / var + log(var);
        }
        else
        {
            fv += s*s / var;
        }
    }
    return(-fv);
}

//-----------------------//
// GMF::RetrieveWinds_GS //
//-----------------------//

int
GMF::RetrieveWinds_GS(
    MeasList*    meas_list,
    Kp*            kp,
    WVC*        wvc,
    int         polar_special=0)
{
//
//  Step 1:  Find an initial set of coarse wind solutions.
//

    Calculate_Init_Wind_Solutions(meas_list, kp, wvc, polar_special);

    WindVectorPlus* wvp = NULL;

//    for (wvp=wvc->ambiguities.GetHead(); wvp;
//         wvp = wvc->ambiguities.GetNext())
//    {
//        printf("init obj,spd,dir= %g %g %g\n",wvp->obj,wvp->spd,rtd*wvp->dir);
//    }

//
//  Step 2:  Iterate to find an optimized set of wind solutions.
//

    Optimize_Wind_Solutions(meas_list,kp,wvc);

//  Step 3:  Rank the optimized wind solutions for use in ambiguity
//           removal.

    wvc->Rank_Wind_Solutions();

    //------------------------------------------//
    // sort the solutions by objective function //
    //------------------------------------------//

    wvc->SortByObj();

    //----------------------------------------//
    // keep only the 4 highest rank solutions //
    //----------------------------------------//

    if (wvc->ambiguities.NodeCount() > 4)
    {
        wvp = NULL;
        wvc->ambiguities.GotoHead();
        for (int i=1; i <= 4; i++)
        {
            wvp = wvc->ambiguities.GetNext();
        }

        while (wvp != NULL)
        {
            wvp = wvc->ambiguities.RemoveCurrent();
            delete(wvp);
        }
    }

//    for (wvp=wvc->ambiguities.GetHead(); wvp;
//         wvp = wvc->ambiguities.GetNext())
//    {
//        printf("final obj,spd,dir= %g %g %g\n",wvp->obj,wvp->spd,rtd*wvp->dir);
//    }
//    exit(0);

    return(1);
}

//------------------------------------//
// GMF::Calculate_Init_Wind_Solutions //
//------------------------------------//

int
GMF::Calculate_Init_Wind_Solutions(
    MeasList*  meas_list,
    Kp*        kp,
    WVC*       wvc,
    int        polar_special=0)
{
//!Description:
//             This routine calculates an initial set of wind solutions
//               using an iterative "coarse search" mechanism based on MLE
//               computations.
//

//
//        Remove Bad Copol Measurements
//
      RemoveBadCopol(meas_list,kp);
//
// Local Declarations
//

      int   i;
      int   j;
      int   k;
      int   ii;
      int   jj;
      float      center_speed;
      float      minus_speed;
      float      plus_speed;
      int   good_speed;
      float      center_objective;
      float      minus_objective;
      float      plus_objective;
      float      dir_spacing;
      int   num_dir_samples;
      float      angle;
      float      diff_objective_1;
      float      diff_objective_2;
      float      current_objective;
      int   num_mle_maxima;
//      float      speed_buffer [MAX_DIR_SAMPLES+1];
//      float      objective_buffer [MAX_DIR_SAMPLES+1] ;
//      int   dir_mle_maxima [MAX_DIR_SAMPLES+1];

    for (i=0; i <= MAX_DIR_SAMPLES+1; i++)
    {
        _speed_buffer[i] = 0.0;
        _objective_buffer[i] = 0.0;
        _dir_mle_maxima[i] = 0;
    }

      center_speed = (int)(wind_start_speed);

      if (center_speed < (lower_speed_bound + 2))
         center_speed = (lower_speed_bound + 2);

      if (center_speed > (upper_speed_bound - 1))
         center_speed = (upper_speed_bound -1);

//
//   Calculate number of wind direction samples.
//

      dir_spacing =  wind_dir_intv_init;
      num_dir_samples = (int)(360. / dir_spacing) + 2 ;

//
//   Loop through directional space to find local MLE maximas.
//

      int multiridge=0;
      int tmp=0;
      float tmp2,tmp3, max_multiridge_sep=0, ave_multiridge_sep=0;
      float multiridge_width=0.0, min_multiridge_sep=HUGE_VAL;
    for (k=2; k <= num_dir_samples-1; k++)
    {
      if(polar_special){
        tmp=FindMultiSpeedRidge(meas_list, kp,k,&tmp2,&tmp3);
        if(tmp>multiridge) multiridge=tmp;
            if(tmp>1){
          multiridge_width+=dir_spacing;
          ave_multiridge_sep+=tmp2;
          if(tmp2>max_multiridge_sep) max_multiridge_sep=tmp2;
              if(tmp3<min_multiridge_sep) min_multiridge_sep=tmp3;
        }
            continue;
      }
         angle = dir_spacing * (float)(k - 1) - dir_spacing;

//
//   Compute MLE at 3 points centered about center speed.
//

         minus_speed  =  center_speed -  wind_speed_intv_init;
         plus_speed   =  center_speed +  wind_speed_intv_init;

        minus_objective=_ObjectiveFunction(meas_list,minus_speed,dtr*angle,kp);
        center_objective=_ObjectiveFunction(meas_list,center_speed,
            dtr*angle,kp);
        plus_objective=_ObjectiveFunction(meas_list,plus_speed,dtr*angle,kp);

//
//   Move the triplet in the speed dimension until center_objective
//   reaches maximum.   Clip search at 0 m/sec and 50 m/sec.
//

         good_speed = 1;

         while (good_speed &&
            (center_objective < minus_objective  ||
             center_objective < plus_objective) )
        {

//
//  If "minus speed" has the largest objective value, shift the speed
//  "downward".
//

            if (minus_objective >= center_objective  &&
               minus_objective >= plus_objective    &&
               good_speed )
            {
               center_speed = center_speed
                           - wind_speed_intv_init ;
               plus_objective = center_objective;
               center_objective = minus_objective;
               minus_speed = center_speed
                          - wind_speed_intv_init;

//
//   Clip search at null (0) speed.
//

               if (minus_speed < (float)(lower_speed_bound))
                {
                  _speed_buffer[k] = center_speed;
                 _objective_buffer [k] = center_objective;
                  center_speed = center_speed + wind_speed_intv_init;
                  good_speed = 0;
                }

//
//   Re-Evaluate objective function with shifted minus speed.
//

               if (good_speed)
                {
                    minus_objective = _ObjectiveFunction(meas_list,
                                        minus_speed,dtr*angle,kp);
                }
            }

//
//  If "plus speed" has the largest objective value, shift the speed
//  "upward".
//

            if (plus_objective > center_objective  &&
               plus_objective > minus_objective   &&
               good_speed  )
            {
               center_speed = center_speed + wind_speed_intv_init;
               minus_objective = center_objective;
               center_objective = plus_objective;
               plus_speed = center_speed + wind_speed_intv_init;

//
//   Clip search at maximum (50) speed.
//

               if  (plus_speed > (float)(upper_speed_bound))
                {
                  _speed_buffer [k] = center_speed;
                  _objective_buffer [k] = center_objective;
                  center_speed = center_speed - wind_speed_intv_init;
                  good_speed = 0;
                }

//
//   Re-Evaluate objective function with shifted plus speed.
//

               if   (good_speed)
                {
                    plus_objective = _ObjectiveFunction(meas_list,
                                        plus_speed,dtr*angle,kp);
                }
            }
        }    // while loop


         if (center_objective >= minus_objective  &&
            center_objective >= plus_objective   &&
            good_speed)
        {
            diff_objective_1 = plus_objective - minus_objective;
            diff_objective_2 = (plus_objective + minus_objective)
                            - 2.0 * center_objective;

            if(diff_objective_2==0) _speed_buffer[k]=center_speed;
            else _speed_buffer [k] = center_speed  - 0.5
                            * (diff_objective_1 / diff_objective_2)
                            * wind_speed_intv_init;

#define GSFIXED
#ifdef GSFIXED
            // Re-evaluate objective function to avoid interpolation bumps
            // that introduce artificial peaks.
            _objective_buffer[k] = _ObjectiveFunction(meas_list,
                                        _speed_buffer[k],dtr*angle,kp);
#else
            _objective_buffer [k] = center_objective
                                - (diff_objective_1*diff_objective_1
                                  /diff_objective_2) / 8.0;
#endif

        }
    }    // end of angular k loop

      if(polar_special){
    ave_multiridge_sep/=multiridge_width/dir_spacing;
    printf("%d %g %g %g %g ",multiridge, multiridge_width,
                   ave_multiridge_sep, max_multiridge_sep,
                           min_multiridge_sep);
      }
//
//   Make speed/objective buffers "circularly continuous".
//

      _speed_buffer [1] = _speed_buffer [num_dir_samples - 1];
      _speed_buffer [num_dir_samples] = _speed_buffer [2];

      _objective_buffer [1]  = _objective_buffer [num_dir_samples - 1];
      _objective_buffer [num_dir_samples]  = _objective_buffer [2];

//
//   Find local MLE maximas over the angular intervals.
//

      num_mle_maxima = 0;

    for (k=2; k <= num_dir_samples-1; k++)
    {

      // Added >= to eliminate missed peaks
        if (_objective_buffer[k] > _objective_buffer[k-1] &&
            _objective_buffer[k] >= _objective_buffer[k+1])
        {
            num_mle_maxima = num_mle_maxima + 1;
            _dir_mle_maxima [num_mle_maxima] = k;
        }
    }

//
//   If the number of local MLE maximas exceeds the desired maximum
//   wind solutions (= wind_max_solutions), sort and select the
//   (wind_max_solutions) highest.
//

    if (num_mle_maxima > wind_max_solutions)
    {
//        printf("number greater than 10 = %d\n",num_mle_maxima);
//         write(99,*) objective_buffer

        for (i=1; i <= num_mle_maxima; i++)
        {
            k = 0;
            current_objective = _objective_buffer [_dir_mle_maxima[i]];
            ii = i + num_mle_maxima;
            for (j=1; j <= num_mle_maxima; j++)
            {
               jj = _dir_mle_maxima [j];
               if (current_objective < _objective_buffer [jj] )
                  k = k + 1;
            }

//
//    Tag current maxima index as -2 if it is not in the highest
//    "wind_max_solutions" rank.  Otherwise, tag as -1.
//

            if (k >= wind_max_solutions)
                _dir_mle_maxima [ii] = -2;
            else
                _dir_mle_maxima [ii] = -1;
        }

//
//    Select and store "highest rank" directional indices into
//    "latter" dimensions of the array.
//

        ii = 0;
        for (i=1; i <= num_mle_maxima; i++)
        {
            if (_dir_mle_maxima[i + num_mle_maxima] == -1)
            {
               ii = ii + 1;
               _dir_mle_maxima [ii + 2 * num_mle_maxima] = _dir_mle_maxima [i];
            }
        }

//
//    Move "highest rank" directions back to the front.
//

        for (k=1; k <= wind_max_solutions; k++)
        {
            _dir_mle_maxima [k] = _dir_mle_maxima [k + 2 * num_mle_maxima];
        }
        num_mle_maxima = wind_max_solutions;
//        printf("dir_mle_maxima = %d\n",dir_mle_maxima);

    }

//
//   Finally, fill in the output WR variables/arrays.
//

    int wr_num_ambigs = num_mle_maxima;
    for (i=1; i <= wr_num_ambigs; i++)
    {
        jj = _dir_mle_maxima [i];
//        wr_wind_dir [i] = dir_spacing * (float)(jj - 1) - dir_spacing;
//        wr_wind_speed [i] = speed_buffer [jj];
//        wr_mle [i] = objective_buffer [jj];

        WindVectorPlus* wvp = new WindVectorPlus();
        if (! wvp)
            return(0);
        wvp->spd = _speed_buffer [jj];
        wvp->dir = dtr * (dir_spacing * (float)(jj - 1) - dir_spacing);
        wvp->obj = _objective_buffer [jj];
//        printf("init: obj,spd,dir= %g %g %g\n",wvp->obj,wvp->spd,rtd*wvp->dir);
        if (! wvc->ambiguities.Append(wvp))
        {
            delete wvp;
            return(0);
        }
    }


    return(1);
}
//------------------------------//
// GMF::FindMultiSpeedRidge     //
//------------------------------//
int
GMF::FindMultiSpeedRidge(
    MeasList*       meas_list,
        Kp*                    kp,
        int               dir_idx,
    float*            max_sep,
    float*            min_sep){
      float      center_speed;
      float      minus_speed;
      float      plus_speed;
      int        min_speed_best=0;
      float      max_speed_best=0;
      int        ridge_count=0;
      float      best_center_speed=0;
      float      best_minus_speed;
      float      best_plus_speed;
      float      center_objective;
      float      minus_objective;
      float      plus_objective;
      float      best_center_objective=-HUGE_VAL;
      float      best_minus_objective=-HUGE_VAL;
      float      best_plus_objective=-HUGE_VAL;
      float      dir_spacing;
      float      spd_spacing;
      float      angle;
      float      diff_objective_1;
      float      diff_objective_2;
      float      speed_peaks[30];

      *max_sep=0;
      *min_sep=HUGE_VAL;
      //
      // compute angle
      //
      dir_spacing =  wind_dir_intv_init;
      angle = dir_spacing * (float)(dir_idx - 1) - dir_spacing;
      angle=angle*dtr;

      //
      // check for peak at lower speed bound
      //

      minus_speed = lower_speed_bound;
      spd_spacing = wind_speed_intv_init;
      center_speed = lower_speed_bound + spd_spacing;
      plus_speed=lower_speed_bound + 2*spd_spacing;
      minus_objective=_ObjectiveFunction(meas_list,minus_speed,angle,kp);
      center_objective=_ObjectiveFunction(meas_list,center_speed,angle,kp);

      if( minus_objective>center_objective ){
    min_speed_best=1;
        speed_peaks[ridge_count]=minus_speed;
        ridge_count++;
        best_center_speed=minus_speed;
        best_center_objective=minus_objective;
      }
      int offset=1;
      while(plus_speed <= upper_speed_bound){
    plus_objective=_ObjectiveFunction(meas_list,plus_speed,angle,kp);
    if(plus_objective <= center_objective &&
           minus_objective <= center_objective){
      speed_peaks[ridge_count]=center_speed;
      ridge_count++;
          if(center_objective > best_center_objective){

        best_center_objective=center_objective;
            best_center_speed=center_speed;

            best_plus_objective=plus_objective;
            best_plus_speed=plus_speed;

            best_minus_objective=minus_objective;
            best_minus_speed=minus_speed;

            min_speed_best=0;
      }
    }
    offset++;
        center_speed = offset*spd_spacing+lower_speed_bound;
    minus_speed = center_speed - spd_spacing;
        plus_speed =  center_speed + spd_spacing;
    minus_objective=center_objective;
        center_objective=plus_objective;
      }

      //
      // Check for peak at upper speed bound;
      //
      if(center_objective>minus_objective){
    speed_peaks[ridge_count]=upper_speed_bound;
    ridge_count++;
        // recompute exactly at boundary
        center_objective=_ObjectiveFunction(meas_list,upper_speed_bound,angle,kp);
    if(center_objective >= best_center_objective){
      best_center_objective=center_objective;
          best_center_speed=upper_speed_bound;
      max_speed_best=1;
          min_speed_best=0;
    }
      }

      if(max_speed_best || min_speed_best){
          _speed_buffer[dir_idx] = best_center_speed;
          _objective_buffer[dir_idx] = best_center_objective;
      }
      else{
            diff_objective_1 = best_plus_objective - best_minus_objective;
            diff_objective_2 = (best_plus_objective + best_minus_objective)
                            - 2.0 * best_center_objective;

            _speed_buffer [dir_idx] = best_center_speed  - 0.5
                            * (diff_objective_1 / diff_objective_2)
                            * spd_spacing;
        _objective_buffer[dir_idx] = _ObjectiveFunction(meas_list,
                     _speed_buffer[dir_idx],angle,kp);
      }

      for(int c=0;c<ridge_count;c++){
        float sep=fabs(best_center_speed - speed_peaks[c]);
    if(sep>*max_sep) *max_sep=sep;
    if(sep != 0.0 && sep<*min_sep) *min_sep=sep;
      }
      return(ridge_count);
}

//------------------------------//
// GMF::Optimize_Wind_Solutions //
//------------------------------//

int
GMF::Optimize_Wind_Solutions(
    MeasList*    meas_list,
    Kp*            kp,
    WVC*        wvc)
{

//
//!Description:
//             This routine does optimization for the initial set of
//               wind solutions.  It also computes the errors associated
//               with optimized wind speeds and directions.
//

//
// Local Declarations
//

      int   i;
      int   j;
      int   ambig ;
      int   i_spd;
      int   i_spd_max;
      int   i_spd_old;
      int   ii_spd_old ;
      int   i_dir;
      int   i_dir_max;
      int   i_dir_old;
      int   ii_dir_old;
      int   shift_pattern;
      int   max_shift = 0;
      int   coord_sum;
      int   new_coord_sum;
      int   old_coord_sum;

      int   points_in_search;
      float      current_objective[4][4];
      float      saved_objective [4][4];
      float      maximum_objective;
      int   number_iterations;
      float      center_speed;
      float      center_dir;
      float      speed;
      float      direction;

      float      q00;
      float      q01;
      float      q10;
      float      q02;
      float      q20;
      float      q11;
      float      determinant;
      float      final_speed;
      float      final_dir;

    float    wr_wind_speed[wind_max_solutions];
    float    wr_wind_dir[wind_max_solutions];
    float    wr_mle[wind_max_solutions];

    // Copy data into wr_ arrays. (convert radians to degrees)
    i = 0;
    for (WindVectorPlus* wvp = wvc->ambiguities.GetHead();
         wvp; wvp = wvc->ambiguities.GetNext())
    {
        wr_wind_speed[i] = wvp->spd;
        wr_wind_dir[i] = rtd*wvp->dir;
        wr_mle[i] = wvp->obj;
        i++;
        if (i >= wind_max_solutions) break;
    }

//
//  Initialization
//

    for (ambig=0; ambig < wvc->ambiguities.NodeCount(); ambig++)
    {
//
//   Initialization for each ambiguity.
//

         center_speed = wr_wind_speed [ambig];
         center_dir = wr_wind_dir [ambig];
         number_iterations =  0;
         maximum_objective = -1000000.;
         points_in_search = 5;
         i_spd_max = 0;
         i_dir_max = 0;
         shift_pattern = 0;

//
//   Compute initial objecitve function values at 5 non-corner grid
//   points.   Find maximum location and its shift vector.
//


        for (i=1; i <= 3; i++)
        {
            i_spd = i - 2;
            speed = center_speed + (float)(i_spd) * wind_speed_intv_opti;
            for (j=1; j <= 3; j++)
            {
               i_dir = j - 2;
               direction = center_dir + (float)(i_dir) * wind_dir_intv_opti;
               if (i_dir == 0  ||  i_spd == 0)
                {
                    current_objective[i][j] = _ObjectiveFunction(meas_list,
                                        speed,dtr*direction,kp);

                    if (current_objective [i][j] > maximum_objective)
                    {
                      maximum_objective = current_objective [i][j];
                      i_spd_max = i_spd;
                      i_dir_max = i_dir;
                    }
                }
            }
        }

//
//   Check if the maximum point has shifted from the original center
//   point.
//

         shift_pattern = abs (i_spd_max) + abs (i_dir_max);
         number_iterations = number_iterations + 1;

//
//   Continuous search until "9-points" AND "no shift" detected.
//


        while (shift_pattern != 0  || points_in_search  != 9 )
        {
//
//    If "5-point" AND "no shift" is reached, turn on "9-point"
//    search option and compute the 4 corners.
//

            if (shift_pattern == 0  && points_in_search == 5)
            {
               points_in_search = 9;
                for (i=1; i <= 3; i+=2)
                {
                    speed = center_speed + (float)(i - 2) *
                        wind_speed_intv_opti;
                    for (j=1; j <= 3; j+=2)
                    {
                        direction = center_dir + (float)(j - 2) *
                            wind_dir_intv_opti;

                        current_objective[i][j] = _ObjectiveFunction(meas_list,
                                            speed,dtr*direction,kp);

                           if (current_objective [i][j] > maximum_objective)
                        {
                               maximum_objective = current_objective [i][j];
                               i_spd_max = i - 2;
                               i_dir_max = j - 2;
                        }
                    }
                }

               shift_pattern = abs (i_spd_max) + abs (i_dir_max);
               number_iterations = number_iterations + 1;
            }

//
//   If center point is not the maximum point, do shift.
//

            if (shift_pattern != 0)
            {
               center_speed = center_speed + (float)(i_spd_max)
                                * wind_speed_intv_opti;
               center_dir = center_dir + (float)(i_dir_max)
                                * wind_dir_intv_opti;

//
//   Loop through  3 x 3  phase space.
//

                for (i=1; i <= 3; i++)
                {
                    i_spd = i - 2;
                    i_spd_old = i + i_spd_max;
                    ii_spd_old = i_spd_old - 2;
                    for (j=1; j <= 3; j++)
                    {
                        i_dir = j - 2;
                        i_dir_old = j + i_dir_max;
                        ii_dir_old = i_dir_old - 2;
                        new_coord_sum = abs (i_spd) + abs(i_dir);
                        old_coord_sum = abs (ii_spd_old) + abs(ii_dir_old);

//
//   Continue do-loop processing unless "points_in_search = 5" AND
//   "coordinate sum = 2".
//

                        if (points_in_search != 5  || new_coord_sum != 2)
                        {
                            if (points_in_search == 5)
                                max_shift = 1;

                            if (points_in_search == 9)
                                max_shift = 2;

                            if (abs(ii_spd_old) == 2  || abs(ii_dir_old) == 2)
                                max_shift = 1;

//
//    If "old" coordinate is outside the "new" boundary, then do shift
//    and compute a new MLE.
//

                            if (old_coord_sum > max_shift)
                            {
                                speed = center_speed + (float)(i_spd) *
                                  wind_speed_intv_opti;
                                direction = center_dir + (float)(i_dir) *
                                  wind_dir_intv_opti;
                                saved_objective[i][j] = _ObjectiveFunction(
                                    meas_list,speed,dtr*direction,kp);
                            }
                            else
                            {
                                saved_objective [i][j] =
                                    current_objective [i_spd_old][i_dir_old];
                            }

                        }
                    }
                }


//
//   Find maximum objective value and its shift vector.
//

//
//    Initialization: assume maximum is at new center point.
//

               maximum_objective = saved_objective [2][2];
               i_spd_max  = 0;
               i_dir_max  = 0;

                for (i=1; i <= 3; i++)
                {
                    i_spd = i - 2;
                    for (j=1; j <= 3; j++)
                    {
                        i_dir = j - 2;
                        coord_sum = abs(i_spd) + abs(i_dir);

//
//    Skip corners for 5-points search.
//

                        if (points_in_search != 5  || coord_sum  != 2)
                        {
                            current_objective [i][j] =
                                saved_objective [i][j];

                            if (current_objective [i][j] > maximum_objective)
                            {
                                maximum_objective = current_objective [i][j];
                                i_spd_max = i_spd;
                                i_dir_max = i_dir;
                            }
                        }
                    }
                }

               shift_pattern = abs(i_spd_max) + abs (i_dir_max);
               number_iterations = number_iterations + 1;

            }    // if (shift_pattern /= 0)

        }    // while loop

//
//   Now the 9-point search is complete, store "new" speed and
//   direction back to WR array.
//

         wr_wind_speed [ambig]  = center_speed;
         wr_wind_dir [ambig] = center_dir;

//
//   Interpolate to find optimized wind solutions.
//

//
//    Compute various partial derivatives and Jacobian determinant.
//
//      q00:  constant term
//      q10:  1st partial  w.r.t.  speed
//      q01:  1st partial  w.r.t.  direction
//      q20:  2nd parital  w.r.t.  speed
//      q02:  2nd partial  w.r.t.  direction
//      q11:  2nd partial  w.r.t.  speed and direction
//      determinant:  Jacobian determinant.
//

         q00 =  - current_objective [2][2];
         q10 =  - (current_objective [3][2] -
                  current_objective [1][2] ) / 2.;
         q01 =  - (current_objective [2][3] -
                  current_objective [2][1] ) / 2.;
         q20 =  - (current_objective [3][2] +
                  current_objective [1][2] -
                  2.0 * current_objective [2][2] );
         q02 =  - (current_objective [2][3] +
                  current_objective [2][1] -
                  2.0 * current_objective [2][2] );
         q11 =  - (current_objective [3][3] -
                  current_objective [3][1] -
                  current_objective [1][3] +
                  current_objective [1][1] ) / 4.;

         determinant = q20 * q02 - q11 * q11;


//
//   Skip to next ambiguity if determinant happens to be 0.
//

        if (determinant == 0)
        {
            wr_mle [ambig] = - q00;
//            wr_wind_speed_err [ambig] =
//                wind_speed_intv_opti / sqrt (float(wr_count));
//            wr_wind_dir_err [ambig] =
//                wind_dir_intv_opti /sqrt ((float)(wr_count));
        }
        else
        {
//
//   Compute final wind speed solution.
//

            final_speed = wr_wind_speed [ambig] -
                         wind_speed_intv_opti *
                        (q10 * q02 - q01 * q11) /
                        determinant;

            if (fabs (final_speed - wr_wind_speed [ambig])
               > 2. * wind_speed_intv_opti   ||
               final_speed < 0.0)
            {
                wr_mle [ambig] = -q00;
//                wr_wind_speed_err [ambig] = wind_speed_intv_opti
//                    * sqrt (fabs (0.5 * q20 / determinant));
//                wr_wind_dir_err [ambig] = wind_dir_intv_opti
//                    * sqrt (fabs (0.5 * q02 / determinant));
            }
            else
            {
//
//   Compute final wind direction.
//
               final_dir = wr_wind_dir [ambig] -
                          wind_dir_intv_opti *
                          (q20 * q01 - q10 * q11) /
                          determinant;

               if (fabs (final_dir - wr_wind_dir [ambig])
                    > 2. * wind_dir_intv_opti)
                {

                  wr_mle [ambig] = -q00;
//                  wr_wind_speed_err [ambig]
//                    = wind_speed_intv_opti
//                    * sqrt (fabs (0.5 * q20 / determinant));
//                  wr_wind_dir_err [ambig] = wind_dir_intv_opti
//                    * sqrt (fabs (0.5 * q02 / determinant));
                }
               else
                  {
//
//   Constrain final direction between 0 and 360 degrees.
//

                  if  (final_dir < 0.)
                     final_dir = final_dir + 360.;
                  if  (final_dir > 360.)
                     final_dir = final_dir - 360.;

//
//   Move interpolated speed & direction into WR arrays.
//

                  wr_wind_speed [ambig] = final_speed;
                  wr_wind_dir [ambig] = final_dir;

//
//   Estimate the final value of likelihood.
//
                wr_mle[ambig] = _ObjectiveFunction(meas_list,
                    wr_wind_speed[ambig],dtr*wr_wind_dir[ambig],kp);

//
//   Estimate the RMS speed and direction errors.
//

//                  wr_wind_speed_err [ambig] = wind_speed_intv_opti *
//                    sqrt (fabs (0.5 * q20 / determinant));

//                  wr_wind_dir_err [ambig] = wind_dir_intv_opti *
//                    sqrt (fabs (0.5 * q02 / determinant));
            }
            }
        }
    }    //  The big ambiguity loop

    // Copy data from wr_ arrays. (convert to radians)
    i = 0;
    for (WindVectorPlus* wvp = wvc->ambiguities.GetHead();
         wvp; wvp = wvc->ambiguities.GetNext())
    {
        wvp->spd = wr_wind_speed[i];
        wvp->dir = wr_wind_dir[i]*dtr;
        wvp->obj = wr_mle[i];
//        printf("opti: obj,spd,dir= %g %g %g\n",wvp->obj,wvp->spd,rtd*wvp->dir);
        i++;
// #define VPC_DEBUG  // Uncomment this line to get Vpc debugging output
#ifdef VPC_DEBUG
        global_debug=1;
        for(Meas*meas=meas_list->GetHead();meas;meas=meas_list->GetNext()){
          printf("%d %d %d %d %g %g %g %g ",i,meas->startSliceIdx,meas->numSlices,(int)(meas->measType),meas->value,wvp->obj,wvp->spd,wvp->dir*rtd);
                  // calculate partial objective function
          MeasList ml;
          ml.Append(meas);
                  float partial_obj=_ObjectiveFunction(&ml,wvp->spd,wvp->dir,kp);
          float sigma0_over_snr=meas->EnSlice / meas->XK / meas->txPulseWidth;
          printf("%g %g %g %g %g %g %g %g\n",partial_obj,sigma0_over_snr,
             meas->EnSlice, meas->XK, meas->txPulseWidth, meas->A, meas->B, meas->C);
                  ml.GetHead();
                  ml.RemoveCurrent();
        }
                  global_debug=0;
#endif
        if (i >= wind_max_solutions) break;
    }


    return(1);

}
//---------------------//
// CopyBuffersGSToPE   //
//---------------------//
int
GMF::CopyBuffersGSToPE(){
  if(wind_dir_intv_init!=360.0/_phiCount) return(0);
  for(int c=0;c<_phiCount;c++){
    _bestObj[c]=_objective_buffer[c+2];
    _bestSpd[c]=_speed_buffer[c+2];
  }
  return(1);
}

//-------------------//
// GMF::WriteObjXmgr //
//-------------------//

int
GMF::WriteObjXmgr(
    char*        basename,
    int            panelcount,
    WVC*        wvc)
{

  static FILE* objxmgr_file = NULL;
  static char filename[256];
  static int count = 0;
  static int graphnum = 0;

  sprintf(filename,"%s.%d",basename,count);
  objxmgr_file = fopen(filename,"a");
  if (objxmgr_file == NULL)
  {
    fprintf(stderr,"GMF::WriteObjXmgr: Error opening %s\n",filename);
    exit(-1);
  }

  if (graphnum != 0)
  {
    fprintf(objxmgr_file, "&\n");
  }

  fprintf(objxmgr_file,"@WITH G%d\n",graphnum);
  fprintf(objxmgr_file,"@G%d ON\n",graphnum);
  float min_obj,max_obj;
  GetObjLimits(&min_obj,&max_obj);
  WriteObjectiveCurve(objxmgr_file,min_obj,max_obj);
  fprintf(objxmgr_file, "&\n");
  AppendSolutions(objxmgr_file,wvc,min_obj,max_obj);

  if (graphnum == panelcount-1)
  {
    graphnum = 0;
    count++;
  }
  else
  {
    graphnum++;
  }

  fclose(objxmgr_file);
  return(1);

}


//-----------------------//
// GMF::SolutionCurve_H1 //
//-----------------------//
// For each of the directions, find the speed that maximizes the
// objective function.  Fills in the _bestSpd and _bestObj arrays
// The H1 version uses adaptive thresholding and a sub-function

#define H1_DELTA_SPEED_FRACTION    0.2
#define H1_THRESH_FRACTION         0.1   // for S1 should be 0.9

int
GMF::SolutionCurve_H1(
    MeasList*  meas_list,
    Kp*        kp)
{
    //-------------------------------//
    // bracket maxima with certainty //
    //-------------------------------//

    float low_speed = _spdMin;
    float high_speed = _spdMax;

    //-----------------------//
    // for each direction... //
    //-----------------------//

    for (int phi_idx = 0; phi_idx < _phiCount; phi_idx++)
    {
        float dir = phi_idx * _phiStepSize;
        FindBestSpeed(meas_list, kp, dir, low_speed, high_speed,
            &(_bestSpd[phi_idx]), &(_bestObj[phi_idx]));

        //----------------------------------------------------//
        // adjust the low and high speeds for the next search //
        //----------------------------------------------------//

        low_speed = _bestSpd[phi_idx] * (1.0 - H1_DELTA_SPEED_FRACTION);
        high_speed = _bestSpd[phi_idx] * (1.0 + H1_DELTA_SPEED_FRACTION);
    }
    return(1);
}

//--------------------//
// GMF::FindBestSpeed //
//--------------------//

int
GMF::FindBestSpeed(
    MeasList*  meas_list,
    Kp*        kp,
    float      dir,
    float      low_speed,
    float      high_speed,
    float*     best_speed,
    float*     best_obj)
{
    float ax = low_speed;
    float cx = high_speed;
    float bx = ax + (cx - ax) * golden_r;
    float phi = dir;

    //-----------------------------------//
    // make sure the maxima is bracketed //
    //-----------------------------------//

    if (_ObjectiveFunction(meas_list, bx, phi, kp) <
        _ObjectiveFunction(meas_list, ax, phi, kp) )
    {
        ax = _spdMin;
    }
    if (_ObjectiveFunction(meas_list, bx, phi, kp) <
        _ObjectiveFunction(meas_list, cx, phi, kp) )
    {
        cx = _spdMax;
    }

    //---------------------//
    // find the best speed //
    //---------------------//

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
    float f1 = _ObjectiveFunction(meas_list, x1, phi, kp);
    float f2 = _ObjectiveFunction(meas_list, x2, phi, kp);

    while (x3 - x0 > _spdTol)
    {
        if (f2 > f1)
        {
            x0 = x1;
            x1 = x2;
            x2 = x2 + golden_c * (x3 - x2);
            f1 = f2;
            f2 = _ObjectiveFunction(meas_list, x2, phi, kp);
        }
        else
        {
            x3 = x2;
            x2 = x1;
            x1 = x1 - golden_c * (x1 - x0);
            f2 = f1;
            f1 = _ObjectiveFunction(meas_list, x1, phi, kp);
        }
    }

    if (f1 > f2)
    {
        *best_speed = x1;
        *best_obj = f1;
    }
    else
    {
        *best_speed = x2;
        *best_obj = f2;
    }

    return(1);
}


//-----------------------//
// GMF::RetrieveWinds_H1 //
//-----------------------//

int
GMF::RetrieveWinds_H1(
    MeasList*  meas_list,
    Kp*        kp,
    WVC*       wvc)
{
    //--------------------------------//
    // generate coarse solution curve //
    //--------------------------------//

    if (_phiCount != 45)
        SetPhiCount(45);
    SolutionCurve_H1(meas_list, kp);

    //----------------------------//
    // determine maxima threshold //
    //----------------------------//

    float min_obj = _bestObj[0];
    float max_obj = _bestObj[0];
    for (int phi_idx = 0; phi_idx < _phiCount; phi_idx++)
    {
        if (_bestObj[phi_idx] > max_obj)
            max_obj = _bestObj[phi_idx];
        if (_bestObj[phi_idx] < min_obj)
            min_obj = _bestObj[phi_idx];
    }
    float threshold_delta = (max_obj - min_obj) * H1_THRESH_FRACTION;

    //-------------//
    // find maxima //
    //-------------//

    int peak[10];            // phi index of peak
    float left_rad[10];      // leftmost angle (radians) of peak
    float right_rad[10];     // rightmost angle (radians) of peak
    float width_rad[10];     // width of peak (radians)
    int number_ambigs[10];   // number of ambiguities assigned to peak
    int valid_peak[10];      // process this peak

    int peak_count = 0;
    int peak_idx = 0;
    for (int phi_idx = 0; phi_idx < _phiCount; phi_idx++)
    {
        if (_bestObj[phi_idx] > _bestObj[(phi_idx + 1) % _phiCount] &&
            _bestObj[phi_idx] > _bestObj[(phi_idx + _phiCount - 1) % _phiCount])
        {
            //------------------------------------//
            // maxima found, determine peak index //
            //------------------------------------//

            if (peak_count >= 10)
            {
                //----------------------------------------//
                // too many peaks, replace the worst peak //
                //----------------------------------------//

                fprintf(stderr, "More than 10 peaks: replacing worst\n");
                peak_idx = 0;
                for (int i = 0; i < peak_count; i++)
                {
                    if (_bestObj[i] < _bestObj[peak_idx])
                        peak_idx = i;
                }
                peak_count = 10;
            }
            else
            {
                peak_idx = peak_count;
                peak_count++;
            }

            //----------------------------------------------//
            // maxima found, determine left and right edges //
            //----------------------------------------------//

            float thresh = _bestObj[phi_idx] - threshold_delta;
            int left_idx = (phi_idx + _phiCount - 1) % _phiCount;
            while (_bestObj[left_idx] < _bestObj[phi_idx] &&
                   _bestObj[left_idx] > thresh)
            {
                left_idx = (left_idx + _phiCount - 1) % _phiCount;
                if (left_idx == phi_idx)    // full loop
                    break;
            }

            int right_idx = (phi_idx + 1) % _phiCount;
            while (_bestObj[right_idx] < _bestObj[phi_idx] &&
                   _bestObj[right_idx] > thresh)
            {
                right_idx = (right_idx + 1) % _phiCount;
                if (right_idx == phi_idx)    // full loop
                    break;
            }

            //------------------------------//
            // check the nature of the peak //
            //------------------------------//

            if (left_idx == phi_idx && right_idx == phi_idx)
            {
                //-------------------------------------------------//
                // I think this is impossible, but just in case... //
                //-------------------------------------------------//

                left_rad[peak_idx] = (float)phi_idx * _phiStepSize;
                right_rad[peak_idx] = left_rad[peak_idx];
            }
            else if (_bestObj[left_idx] > _bestObj[phi_idx] ||
                _bestObj[right_idx] > _bestObj[phi_idx])
            {
                //--------------------------------------------------//
                // just a blip on the side of mountain, but keep it //
                //--------------------------------------------------//

                left_rad[peak_idx] = (float)phi_idx * _phiStepSize;
                right_rad[peak_idx] = left_rad[peak_idx];
            }
            else
            {
                //----------------------------------//
                // a real peak, determine the edges //
                //----------------------------------//

                float m, b;

                int left_plus_idx = (left_idx + 1) % _phiCount;
                m = _bestObj[left_plus_idx] - _bestObj[left_idx];
                b = _bestObj[left_idx] - m * (float)left_idx;
                left_rad[peak_idx] = _phiStepSize * (thresh - b) / m;

                int right_minus_idx = (right_idx + _phiCount - 1) % _phiCount;
                m = _bestObj[right_idx] - _bestObj[right_minus_idx];
                b = _bestObj[right_minus_idx] - m * (float)right_minus_idx;
                right_rad[peak_idx] = _phiStepSize * (thresh - b) / m;
            }

            //---------------------//
            // calculate the width //
            //---------------------//

            while (left_rad[peak_idx] < 0.0)
                left_rad[peak_idx] += two_pi;

            while (left_rad[peak_idx] > two_pi)
                left_rad[peak_idx] -= two_pi;

            while (right_rad[peak_idx] - left_rad[peak_idx] < 0.0)
                right_rad[peak_idx] += two_pi;

            while (right_rad[peak_idx] - left_rad[peak_idx] > two_pi)
                right_rad[peak_idx] -= two_pi;

            width_rad[peak_idx] = right_rad[peak_idx] -
                left_rad[peak_idx];

            //----------------------//
            // set other parameters //
            //----------------------//

            peak[peak_idx] = phi_idx;
            number_ambigs[peak_idx] = 1;    // assign it one ambiguity
            valid_peak[peak_idx] = 1;       // starts as a valid peak
        }
    }

    //-------------------//
    // check for overlap //
    //-------------------//

    for (int this_idx = 0; this_idx < peak_count; this_idx++)
    {
        if (! valid_peak[this_idx])    // erased peak
            continue;

        if (width_rad[this_idx] == 0.0)    // too thin to overlap
            continue;

        float center = (left_rad[this_idx] + right_rad[this_idx]) / 2.0;

        for (int other_idx = 0; other_idx < peak_count; other_idx++)
        {
            if (! valid_peak[other_idx])    // erased peak
                continue;

            if (other_idx == this_idx)      // the same peak
                continue;

            float other_center = (left_rad[other_idx] +
                right_rad[other_idx]) / 2.0;
            float center_dif = ANGDIF(center, other_center);
            float range = (width_rad[this_idx] + width_rad[other_idx]) / 2.0;

            if (center_dif >= range)    // no overlap
                continue;

            //------------------------------------------//
            // overlap: get the centers near each other //
            //------------------------------------------//

            while (other_center > center + pi)
                other_center -= two_pi;
            while (other_center < center - pi)
                other_center += two_pi;

            float this_left = center - width_rad[this_idx] / 2.0;
            float this_right = center + width_rad[this_idx] / 2.0;

            float other_left = other_center - width_rad[other_idx] / 2.0;
            float other_right = other_center + width_rad[other_idx] / 2.0;

            if (width_rad[other_idx] == 0.0)
            {
                //--------------------------------//
                // wide overlaps point: absorb it //
                //--------------------------------//

                valid_peak[other_idx] = 0;     // absorb peak
                number_ambigs[this_idx] = 0;   // flag as having no ambigs
            }
            else
            {
                //----------------------------------//
                // wide overlaps wide: combine them //
                //----------------------------------//

                left_rad[this_idx] = MIN(other_left, this_left);
                right_rad[this_idx] = MAX(other_right, this_right);
                width_rad[this_idx] = right_rad[this_idx] - left_rad[this_idx];
                valid_peak[other_idx] = 0;    // erase other peak
                number_ambigs[this_idx] = 0;  // flag as having no ambigs
            }
        }
    }

    //-----------------------------------------//
    // determine how many ambiguities are used //
    //-----------------------------------------//

    int ambiguities = 0;
    for (int this_idx = 0; this_idx < peak_count; this_idx++)
    {
        if (! valid_peak[this_idx])
            continue;

        ambiguities += number_ambigs[this_idx];
    }

    //--------------------------//
    // assign extra ambiguities //
    //--------------------------//

    while (ambiguities < DEFAULT_MAX_SOLUTIONS)
    {
        int max_idx = -1;
        float max_use_width = 0.0;
        for (peak_idx = 0; peak_idx < peak_count; peak_idx++)
        {
            if (! valid_peak[peak_idx])
                continue;

            // determine number of ambiguities
            float use_width;
            if (number_ambigs[peak_idx])
            {
                use_width = (float)width_rad[peak_idx] /
                    (float)(number_ambigs[peak_idx] + 1);
            }
            else
            {
                // artificially inflate widths of peaks with no ambiguities
                use_width = width_rad[peak_idx] + two_pi;
            }

            if (use_width > max_use_width)
            {
                max_use_width = use_width;
                max_idx = peak_idx;
            }
        }

        if (max_idx != -1)
        {
            number_ambigs[max_idx]++;
            ambiguities++;
        }
        else
        {
            break;
        }
    }

    //-------------------------------//
    // refine single ambiguity peaks //
    //-------------------------------//

    WVC tmp_wvc;
    for (peak_idx = 0; peak_idx < peak_count; peak_idx++)
    {
        if (! valid_peak[peak_idx])
            continue;

        if (number_ambigs[peak_idx] == 1)
        {
            WindVectorPlus* wvp = new WindVectorPlus();
            if (! wvp)
                return(0);
            wvp->spd = _bestSpd[peak[peak_idx]];
            wvp->dir = (float)peak[peak_idx] * _phiStepSize;
            wvp->obj = _bestObj[peak[peak_idx]];

            // put in temporary wvc
            if (! tmp_wvc.ambiguities.Append(wvp))
            {
                delete wvp;
                return(0);
            }

            // refine
            Optimize_Wind_Solutions(meas_list, kp, &tmp_wvc);

            // transfer to real wvc
            tmp_wvc.ambiguities.GotoHead();
            wvp = tmp_wvc.ambiguities.RemoveCurrent();
            if (! wvc->ambiguities.Append(wvp))
            {
                delete wvp;
                return(0);
            }
        }
    }

    //-------------------------------------//
    // peak split multiple ambiguity peaks //
    //-------------------------------------//

    for (peak_idx = 0; peak_idx < peak_count; peak_idx++)
    {
        if (! valid_peak[peak_idx])
            continue;

        float max_peak_obj = -9E50;
        WindVectorPlus* max_wvp = NULL;
        if (number_ambigs[peak_idx] > 1)
        {
            for (int i = 0; i < number_ambigs[peak_idx]; i++)
            {
                float dir = left_rad[peak_idx] +
                        width_rad[peak_idx] /
                        (2.0 * (float)number_ambigs[peak_idx]) +
                        (float)i * width_rad[peak_idx] /
                        (float)number_ambigs[peak_idx];

                dir = fmod(dir, two_pi);

                float spd, obj;
                FindBestSpeed(meas_list, kp, dir, 0.0, 50.0, &spd, &obj);

                WindVectorPlus* wvp = new WindVectorPlus();
                if (! wvp)
                    return(0);
                wvp->spd = spd;
                wvp->dir = dir;
                wvp->obj = obj;

                if (wvp->obj > max_peak_obj)
                {
                    max_peak_obj = wvp->obj;
                    max_wvp = wvp;
                }

                // put in wvc
                if (! tmp_wvc.ambiguities.Append(wvp))
                {
                    delete wvp;
                    return(0);
                }
            }
        }

        //------------------------------------//
        // reassign objective function values //
        //------------------------------------//

        tmp_wvc.ambiguities.GotoHead();
        while (WindVectorPlus* wvp = tmp_wvc.ambiguities.RemoveCurrent())
        {
            if (smartNudgeFlag)
            {
                // smart nudge
                if (wvp->obj > (max_obj - min_obj) * 0.8 + min_obj)
                    wvp->obj = 1.0;
            }
            else
            {
                // get one ambiguity per peak
                if (wvp != max_wvp)
                    wvp->obj = min_obj;
            }
            if (! wvc->ambiguities.Append(wvp))
            {
                delete wvp;
                return(0);
            }
        }
    }

    //------//
    // sort //
    //------//

    wvc->SortByObj();

    //-------------------------//
    // limit to four solutions //
    //-------------------------//

    int delete_count = wvc->ambiguities.NodeCount() - DEFAULT_MAX_SOLUTIONS;
    if (delete_count > 0)
    {
//        WriteObjXmgr("toomany",10,wvc);
        fprintf(stderr, "Too many solutions: deleting %d\n", delete_count);
        for (int i = 0; i < delete_count; i++)
        {
            wvc->ambiguities.GotoTail();
            WindVectorPlus* wvp = wvc->ambiguities.RemoveCurrent();
            delete wvp;
        }
    }

    return(1);
}

//-----------------------//
// GMF::RetrieveWinds_H2 //
//-----------------------//

#define H2_PHI_COUNT    45
#define H2_RANGES        45            // way more than needed (unless buggy)

#define H3_MIN_RAD_WIDTH  0.7854    // 45 degrees

// h3_and_s1_flag
// 0:=  H2
// 1:=  H3
// 2:=  S1
int
GMF::RetrieveWinds_H2(
    MeasList*  meas_list,
    Kp*        kp,
    WVC*       wvc,
    int        h3_and_s1_flag)
{
    //--------------------------------//
    // generate coarse solution curve //
    //--------------------------------//

    if (_phiCount != H2_PHI_COUNT)
        SetPhiCount(H2_PHI_COUNT);
    SolutionCurve_H1(meas_list, kp);
    if(h3_and_s1_flag==2) ConvertObjToPdf();

    //----------------------------//
    // determine maxima threshold //
    //----------------------------//

    float min_obj = _bestObj[0];
    float max_obj = _bestObj[0];
    for (int phi_idx = 0; phi_idx < H2_PHI_COUNT; phi_idx++)
    {
        if (_bestObj[phi_idx] > max_obj)
            max_obj = _bestObj[phi_idx];
        if (_bestObj[phi_idx] < min_obj)
            min_obj = _bestObj[phi_idx];
    }
    float threshold_delta;
    if(h3_and_s1_flag==2){    // S1 uses a peak-specific threshold
      min_obj=0;
      threshold_delta=1.0-H1_THRESH_FRACTION;
    }
    else threshold_delta = (max_obj - min_obj) * H1_THRESH_FRACTION;

    //----------------//
    // initialize map //
    //----------------//

    enum MapE { NOTHING, EXTENT, PEAK };
    MapE map[H2_PHI_COUNT];
    for (int i = 0; i < H2_PHI_COUNT; i++)
    {
        map[i] = NOTHING;
    }

    //------------------------//
    // find peaks and extents //
    //------------------------//

    float refined_peak_direction[H2_PHI_COUNT];
    float refined_peak_obj[H2_PHI_COUNT];
    WVC tmp_wvc;
    for (int phi_idx = 0; phi_idx < H2_PHI_COUNT; phi_idx++)
    {
        if (_bestObj[phi_idx] > _bestObj[(phi_idx + 1) % H2_PHI_COUNT] &&
            _bestObj[phi_idx] > _bestObj[(phi_idx + H2_PHI_COUNT - 1) %
                H2_PHI_COUNT])
        {
            //-----------------------//
            // maxima found...map it //
            //-----------------------//

            map[phi_idx] = PEAK;    // mark as peak

            //--------------//
            // ...refine it //
            //--------------//

            WindVectorPlus* wvp = new WindVectorPlus();
            if (! wvp)
                return(0);
            wvp->spd = _bestSpd[phi_idx];
            wvp->dir = (float)phi_idx * _phiStepSize;
        if(h3_and_s1_flag==2)
          wvp->obj = _ObjectiveFunction(meas_list,wvp->spd,wvp->dir,kp);
        else wvp->obj = _bestObj[phi_idx];

            // put in temporary wvc
            if (! tmp_wvc.ambiguities.Append(wvp))
            {
                delete wvp;
                return(0);
            }

            // refine
            Optimize_Wind_Solutions(meas_list, kp, &tmp_wvc);

            // transfer to real wvc
            tmp_wvc.ambiguities.GotoHead();
            wvp = tmp_wvc.ambiguities.RemoveCurrent();
            if (! wvc->ambiguities.Append(wvp))
            {
                delete wvp;
                return(0);
            }
            while (wvp->dir < 0.0)
                wvp->dir += two_pi;
            while (wvp->dir > two_pi)
                wvp->dir -= two_pi;

            //------------------------//
            // remember the direction //
            //------------------------//

            refined_peak_direction[phi_idx] = wvp->dir;
            refined_peak_obj[phi_idx] = _bestObj[phi_idx];

            //--------------------//
            // ...map peak extent //
            //--------------------//

            float thresh; // S1 uses a threshold relative to the peak height
        if(h3_and_s1_flag==2) thresh =_bestObj[phi_idx]*threshold_delta;
            else thresh = _bestObj[phi_idx] - threshold_delta;
            int left_idx = (phi_idx + H2_PHI_COUNT - 1) % H2_PHI_COUNT;
            while (_bestObj[left_idx] < _bestObj[phi_idx] &&
                   _bestObj[left_idx] > thresh)
            {
                if (map[left_idx] == NOTHING)
                {
                    map[left_idx] = EXTENT;
                    refined_peak_obj[left_idx] = _bestObj[phi_idx];
                }

                left_idx = (left_idx + H2_PHI_COUNT - 1) % H2_PHI_COUNT;
                if (left_idx == phi_idx)    // full loop
                    break;
            }

            int right_idx = (phi_idx + 1) % H2_PHI_COUNT;
            while (_bestObj[right_idx] < _bestObj[phi_idx] &&
                   _bestObj[right_idx] > thresh)
            {
                if (map[right_idx] == NOTHING)
                {
                    map[right_idx] = EXTENT;
                    refined_peak_obj[right_idx] = _bestObj[phi_idx];
                }

                right_idx = (right_idx + 1) % H2_PHI_COUNT;
                if (right_idx == phi_idx)    // full loop
                    break;
            }
        }
    }
    int range_idx = 0;
    int range_count = 0;
    int range_status = 0;    // 0 = outside extent, 1 = inside extent
    int start_idx = 0;
    int phi_idx = 0;

    int ambiguities = wvc->ambiguities.NodeCount();
    if (ambiguities >= DEFAULT_MAX_SOLUTIONS)
        goto wrap_it_up;

    //----------------------//
    // determine the ranges //
    //----------------------//

    float left_edge[H2_RANGES];
    int left_edge_type[H2_RANGES];
    float right_edge[H2_RANGES];
    int right_edge_type[H2_RANGES];

    //----------------------------//
    // move to empty place in map //
    //----------------------------//

    while (map[start_idx] != NOTHING)
    {
        start_idx = (start_idx + 1) % H2_PHI_COUNT;
        if (start_idx == 0)
        {
            fprintf(stderr, "RetrieveWinds_H2: it's all peak!\n");
            return(0);
        }
        continue;
    }

    //-------------//
    // search loop //
    //-------------//

    phi_idx = start_idx;
    do
    {
        int next_idx = (phi_idx + 1) % H2_PHI_COUNT;
        if (range_status == 0)
        {
            //---------------------------------------//
            // outside of extent, look for left edge //
            //---------------------------------------//

            if (map[phi_idx] == NOTHING && map[next_idx] != NOTHING)
            {
                //-------------------------------------------------------//
                // found left edge, calculate interpolation coefficients //
                //-------------------------------------------------------//

                float m = _bestObj[next_idx] - _bestObj[phi_idx];
                float b = _bestObj[phi_idx] - m * (float)phi_idx;

                // try extent
                float thresh; // S1 uses a threshold relative to peak height
                if(h3_and_s1_flag==2) thresh=refined_peak_obj[next_idx]*threshold_delta;
        else thresh=refined_peak_obj[next_idx] - threshold_delta;
                left_edge[range_idx] = _phiStepSize * (thresh - b) / m;
                if (left_edge[range_idx] < phi_idx * _phiStepSize ||
                    left_edge[range_idx] > (phi_idx + 1) * _phiStepSize)
                {
                    // extent is extrapolating, try rise
                    left_edge[range_idx] = _phiStepSize *
                        (refined_peak_obj[next_idx] - b) / m;
                    if (left_edge[range_idx] < phi_idx * _phiStepSize ||
                        left_edge[range_idx] > (phi_idx + 1) * _phiStepSize)
                    {
                        // give up
                        fprintf(stderr, "RetrieveWinds_H2: giving up!\n");
                        left_edge[range_idx] = _phiStepSize * phi_idx;
                    }
                }

                left_edge_type[range_idx] = 1;
                range_status = 1;    // inside extent now
            }
        }
        else
        {
            //---------------------------------------//
            // inside of extent, look for right edge //
            //---------------------------------------//

            if (map[phi_idx] != NOTHING && map[next_idx] == NOTHING)
            {
                //-------------------------//
                // found extent right edge //
                //-------------------------//

                float m = _bestObj[next_idx] - _bestObj[phi_idx];
                float b = _bestObj[phi_idx] - m * (float)phi_idx;
                float thresh;
        if(h3_and_s1_flag==2) thresh=refined_peak_obj[phi_idx]*threshold_delta;
        else thresh=refined_peak_obj[phi_idx]-threshold_delta;
                // try extent
                right_edge[range_idx] = _phiStepSize * (thresh - b) / m;
                if (right_edge[range_idx] < phi_idx * _phiStepSize ||
                    right_edge[range_idx] > (phi_idx + 1) * _phiStepSize)
                {
                    // extent is extrapolating, try rise
                    right_edge[range_idx] = _phiStepSize *
                        (refined_peak_obj[phi_idx] - b) / m;
                    if (right_edge[range_idx] < phi_idx * _phiStepSize ||
                        right_edge[range_idx] > (phi_idx + 1) * _phiStepSize)
                    {
                        // give up
                        fprintf(stderr, "RetrieveWinds_H2: giving up!\n");
                        right_edge[range_idx] = _phiStepSize * phi_idx;
                    }
                }
                range_idx++;
                range_status = 0;    // outside extent now
            }
            else if (map[phi_idx] == PEAK)
            {
                //-----------------//
                // found peak edge //
                //-----------------//

                right_edge[range_idx] = refined_peak_direction[phi_idx];
                right_edge_type[range_idx] = 2;
                range_idx++;

                left_edge[range_idx] = refined_peak_direction[phi_idx];
                left_edge_type[range_idx] = 2;
            }
        }
        phi_idx = (phi_idx + 1) % H2_PHI_COUNT;
        if (phi_idx == start_idx)
            break;
    } while (1);

    range_count = range_idx;

    //------------------//
    // calculate widths //
    //------------------//

    float width[H2_RANGES];
    int number_of_peaks[H2_RANGES];
    for (range_idx = 0; range_idx < range_count; range_idx++)
    {
        while (right_edge[range_idx] < left_edge[range_idx])
            right_edge[range_idx] += two_pi;
        width[range_idx] = right_edge[range_idx] - left_edge[range_idx];

        // ...and number of peaks
        number_of_peaks[range_idx] = 0;

        if (left_edge_type[range_idx] == 2)
            number_of_peaks[range_idx]++;

        if (right_edge_type[range_idx] == 2)
            number_of_peaks[range_idx]++;
    }

    //--------------------------//
    // assign extra ambiguities //
    //--------------------------//

    int ambigs[H2_RANGES];
    for (range_idx = 0; range_idx < range_count; range_idx++)
    {
        ambigs[range_idx] = 0;
    }

    while (ambiguities < DEFAULT_MAX_SOLUTIONS)
    {
        int max_idx = -1;
        float max_width = 0.0;
        for (range_idx = 0; range_idx < range_count; range_idx++)
        {
            float use_width, use_ambigs;
            use_ambigs = 0.5 * (float)number_of_peaks[range_idx] +
                ambigs[range_idx] + 1;    // + 1 is for potential added ambig
            use_width = width[range_idx] / use_ambigs;
            if (use_width > max_width)
            {
                // if using h3 or s1, don't split narrow peaks
                if (h3_and_s1_flag && width[range_idx] < H3_MIN_RAD_WIDTH)
                {
                    continue;
                }
                else
                {
                    max_width = use_width;
                    max_idx = range_idx;
                }
            }
        }

        if (max_idx != -1)
        {
            ambigs[max_idx]++;
            ambiguities++;
        }
        else
        {
            break;
        }
    }

    //---------------------------//
    // put ambiguities in ranges //
    //---------------------------//

    for (range_idx = 0; range_idx < range_count; range_idx++)
    {
        if (! ambigs[range_idx])
            continue;

        for (int i = 0; i < ambigs[range_idx]; i++)
        {
            float use_width, use_ambigs;
            use_ambigs = 0.5 * (float)number_of_peaks[range_idx] +
                ambigs[range_idx];
            use_width = width[range_idx] / use_ambigs;

            float dir;
            if (left_edge_type[range_idx] == 2)
                dir = left_edge[range_idx] + (float)(i + 1) * use_width;
            else
                dir = left_edge[range_idx] + ((float)i + 0.5) * use_width;

            dir = fmod(dir, two_pi);

            float spd, obj;
            FindBestSpeed(meas_list, kp, dir, 0.0, 50.0, &spd, &obj);

            WindVectorPlus* wvp = new WindVectorPlus();
            if (! wvp)
                return(0);
            wvp->spd = spd;
            wvp->dir = dir;
            if (h3_and_s1_flag)
                wvp->obj = obj;        // h3 is set up for threshold nudging
            else
                wvp->obj = min_obj;    // h2 is set up for 12 nudging

            // put in wvc
            if (! wvc->ambiguities.Append(wvp))
            {
                delete wvp;
                return(0);
            }
        }
    }

    //------------//
    // goto label //
    //------------//

    wrap_it_up:

    //------//
    // sort //
    //------//

    wvc->SortByObj();

    //-------------------------//
    // limit to four solutions //
    //-------------------------//

    int delete_count = wvc->ambiguities.NodeCount() - DEFAULT_MAX_SOLUTIONS;
    if (delete_count > 0)
    {
        fprintf(stderr, "Too many solutions: deleting %d\n", delete_count);
        for (int i = 0; i < delete_count; i++)
        {
            wvc->ambiguities.GotoTail();
            WindVectorPlus* wvp = wvc->ambiguities.RemoveCurrent();
            delete wvp;
        }
    }

    return(1);
}

//-----------------------//
// GMF::RetrieveWinds_S2 //
//-----------------------//
#define S2_INIT_BISECT  2
#define S2_USE_BRUTE_FORCE 0
#define S2_REPLACE_BAD_PEAKS 0
#define S2_DIR_MSE_THRESHOLD 100.0     // degrees squared
int
GMF::RetrieveWinds_S2(
    MeasList*  meas_list,
    Kp*        kp,
    WVC*       wvc)
{

    static int num=1;
    //--------------------------------//
    // generate coarse solution curve //
    //--------------------------------//

    if (_phiCount != H2_PHI_COUNT)
        SetPhiCount(H2_PHI_COUNT);
    if(!RetrieveWinds_GS(meas_list, kp,wvc)) return(0);
    if(!CopyBuffersGSToPE()) return(0);
    ConvertObjToPdf();
    wvc->SortByDir();

    float peak_dir[DEFAULT_MAX_SOLUTIONS];
    int ambiguities=wvc->ambiguities.NodeCount();

    //------------------------//
    // copy peak_dirs         //
    //------------------------//

    WindVectorPlus* wvp=wvc->ambiguities.GetHead();
    for(int c=0;c<ambiguities;c++){
      peak_dir[c]=wvp->dir;
      wvp=wvc->ambiguities.GetNext();
    }

    int final_num_peaks=ambiguities;

    // For now assume that extra ambiguities implies
    // DeleteBadPeaks is not needed but only deleting the lowest obj.
    if (ambiguities > DEFAULT_MAX_SOLUTIONS){
        goto wrap_it_up_S2;
    }


    float mse_est;
    mse_est=EstimateDirMSE(peak_dir, ambiguities);
    if(mse_est<S2_DIR_MSE_THRESHOLD*dtr*dtr)
        goto wrap_it_up_S2;
    if(ambiguities==0) return(0);

    //----------------------------------------//
    //  If enabled delete peaks thst result in//
    // lower estimated mse values when replaced//
    //-----------------------------------------//
    if(S2_REPLACE_BAD_PEAKS){
      if(!DeleteBadPeaks(wvc,peak_dir,&ambiguities,mse_est))return(0);
    }

    final_num_peaks=ambiguities;
    if (ambiguities >= DEFAULT_MAX_SOLUTIONS){
        goto wrap_it_up_S2;
    }

    //-----------------------------------------//
    // Select extra ambiguities to minimize    //
    // Estimated Direction Mean Square Error   //
    //-----------------------------------------//
    if(S2_USE_BRUTE_FORCE){
      if(!BruteForceGetMinEstimateMSE(peak_dir, ambiguities,&mse_est))
                  return(0);
    }
    else{
      if(!GetMinEstimateMSE(peak_dir, ambiguities,&mse_est,num)) return(0);
    }
    for(int c=ambiguities;c<DEFAULT_MAX_SOLUTIONS;c++){

      WindVectorPlus* wvp = new WindVectorPlus();
      if (! wvp)
    return(0);
      int peak_idx = int(0.5+peak_dir[c]/_phiStepSize)%_phiCount;
      wvp->spd = _bestSpd[peak_idx];
      wvp->dir = peak_dir[c];
      wvp->obj = _ObjectiveFunction(meas_list,wvp->spd,wvp->dir,kp);
      if (! wvc->ambiguities.Append(wvp))
    {
      delete wvp;
      return(0);
    }
    }
    final_num_peaks=DEFAULT_MAX_SOLUTIONS;
    //------------//
    // goto label //
    //------------//

    wrap_it_up_S2:

    //------//
    // sort //
    //------//
    float mse_est_new;
    mse_est=EstimateDirMSE(peak_dir, ambiguities);
    mse_est_new=EstimateDirMSE(peak_dir, final_num_peaks);

    wvc->SortByObj();


#ifdef S2_DEBUG_INTERVAL
    char file[50];
    int initial_num_peaks=ambiguities;
    if(num%S2_DEBUG_INTERVAL==0){
      sprintf(file,"examples/exam%d",num/S2_DEBUG_INTERVAL);
      FILE* ofpp = fopen(file,"w");
      for(int c=0;c<_phiCount;c++){
    fprintf(ofpp,"%g %g\n",c*_phiStepSize*rtd,_bestObj[c]);
      }
      fprintf(ofpp,"&\n");
      for(WindVectorPlus* wvp=wvc->ambiguities.GetHead(); wvp;
      wvp=wvc->ambiguities.GetNext()){
    fprintf(ofpp,"%g %g\n",wvp->dir*rtd,_bestObj[int((wvp->dir)/_phiStepSize)]);
      }
      fprintf(ofpp,"&\n");
      fprintf(ofpp,"%g 0\n&\n%g 0\n&\n%d\n&\n%d\n&\n",sqrt(mse_est*rtd*rtd),
          sqrt(mse_est_new*rtd*rtd),initial_num_peaks,ambiguities);
      fclose(ofpp);
      printf("%s ",file);
    }
#endif
    num++;
    return(1);
}


// Method for determining the directions of extra ambiguities required
// to minimize the estimated MSE
// ConvertObjToPdf must have been previously run for this to work.
int
GMF::BruteForceGetMinEstimateMSE(
       float* peak_dir,
       int    num_peaks,
       float* mse,
       int    level=0,
       float* tmp_peak_dir=NULL){
  // Check to see if we are at the bottom level
  if(level+num_peaks == DEFAULT_MAX_SOLUTIONS){
    // if so compute MSE and compare to input value
    float tmp=EstimateDirMSE(tmp_peak_dir,DEFAULT_MAX_SOLUTIONS);
    if(tmp<*mse){
      *mse=tmp;
      for(int c=num_peaks;c<DEFAULT_MAX_SOLUTIONS;c++){
    peak_dir[c]=tmp_peak_dir[c];
      }
    }
  }

  else{
    // special case for top level
    if(level==0){
      tmp_peak_dir=new float[DEFAULT_MAX_SOLUTIONS];
      for(int c=0;c<num_peaks;c++) tmp_peak_dir[c]=peak_dir[c];
    }
    for(int c=0;c<_phiCount;c++){
      tmp_peak_dir[num_peaks+level]=c*_phiStepSize;
      BruteForceGetMinEstimateMSE(peak_dir,num_peaks,mse,level+1,tmp_peak_dir);
    }
    // special case for top level
    if(level==0) delete tmp_peak_dir;
  }
  return(1);
}
// Method for determining the directions of extra ambiguities required
// to minimize the estimated MSE
// ConvertObjToPdf must have been previously run for this to work.
int
GMF::GetMinEstimateMSE(
       float* peak_dir,
       int    num_peaks,
       float* mse,
       int num=0){
  int finished=0;
  int debug=0;
    FILE* ofpp = NULL;
#ifdef S2_DETAILED_DEBUG
    if (num % S2_DEBUG_INTERVAL == 0 &&
        num / S2_DEBUG_INTERVAL == S2_DETAILED_DEBUG)
    {
        ofpp = fopen("detailed_debug", "w");
        debug = 1;
    }
#endif
  int num_available_ambiguities=DEFAULT_MAX_SOLUTIONS - num_peaks;
  float* tmp_peak_dir = new float[DEFAULT_MAX_SOLUTIONS];

  if (debug){
    fprintf(ofpp,"Initial Peaks: %d Ambiguities Remaining: %d\n",num_peaks,
        num_available_ambiguities);
    fprintf(ofpp,"Peak Directions: ");
    for(int c=0;c<num_peaks;c++) fprintf(ofpp,"%g ",peak_dir[c]*rtd);
    fprintf(ofpp,"Original Estimated RSS: %g\n\n\n",sqrt(*mse*rtd*rtd));
  }

  //------------------------------------//
  // Calculate Initial Search Intervals //
  //------------------------------------//
  AngleIntervalList intervals;
  if(num_peaks==1){
    AngleInterval* interval = new AngleInterval;
    float left=peak_dir[0]+_phiStepSize;
    float right=peak_dir[0]-_phiStepSize;
    interval->SetLeftRight(left,right);
    if(!intervals.Append(interval)) return(0);
  }
  else{
    // Sort Peaks
    sort_increasing(peak_dir,num_peaks);
    for(int p=0;p<num_peaks;p++){
      AngleInterval* interval = new AngleInterval;
      float left=peak_dir[p];
      float right=peak_dir[(p+1)%num_peaks];
      interval->SetLeftRight(left,right);
      if(!intervals.Append(interval))return(0);
    }
  }
  if(debug){
    fprintf(ofpp,"Initial Intervals Before Bisection: %d\n", intervals.NodeCount());
    for(AngleInterval* interval=intervals.GetHead();interval;
    interval=intervals.GetNext()){
      fprintf(ofpp,"[%g,%g],",(interval->left)*rtd,(interval->right)*rtd);
    }
    fprintf(ofpp,"\n");
  }
  for(int c=0;c<S2_INIT_BISECT;c++){
    if(!intervals.Bisect()) return(0);
  }
  if(debug){
    fprintf(ofpp,"Initial Intervals After %d Bisections: %d\n",S2_INIT_BISECT,
        intervals.NodeCount());
    for(AngleInterval* interval=intervals.GetHead();interval;
    interval=intervals.GetNext()){
      fprintf(ofpp,"[%g,%g]\n",(interval->left)*rtd,(interval->right)*rtd);
    }
    fprintf(ofpp,"\n\n\n");
  }

  while(!finished){
    //------------------------------------//
    // Compute trial ambiguity sets       //
    //------------------------------------//
    int num_intervals=intervals.NodeCount();
    int num_trials;
    int** num_trial_ambigs;
    if (debug){
      fprintf(ofpp,"\n\n###########Level %d Calculations############\n\n\n",debug);
      debug++;
    }
    if(! intervals.GetPossiblePlacings(num_available_ambiguities,&num_trials,
                  &num_trial_ambigs)) return(0);
    if(debug){
      for(int t=0; t< num_trials; t++){
    fprintf(ofpp,"Trial #%d  Spacing ",t);
        for(int i=0;i<intervals.NodeCount();i++)
      fprintf(ofpp,"%d ",num_trial_ambigs[t][i]);
    fprintf(ofpp,"\n");
      }
      fprintf(ofpp,"\n\n");
    }
    // Choose best among trial ambiguity sets
    int min_idx=-1;
    *mse=two_pi*two_pi;
    for(int t=0;t<num_trials;t++){
      if(debug) fprintf(ofpp,"Trial #%d Peaks: ",t);
      for(int p=0;p<num_peaks;p++){
    tmp_peak_dir[p]=peak_dir[p];
      }
      int offset=num_peaks;
      int i=0;
      for(AngleInterval* interval=intervals.GetHead();interval;
            interval=intervals.GetNext()){
    interval->GetEquallySpacedAngles(num_trial_ambigs[t][i],
                     &tmp_peak_dir[offset]);
        offset+=num_trial_ambigs[t][i];
    i++;
      }

      float tmp=EstimateDirMSE(tmp_peak_dir,DEFAULT_MAX_SOLUTIONS);

      if(debug){
        for(int p=0;p<DEFAULT_MAX_SOLUTIONS;p++){
      fprintf(ofpp,"%g ", tmp_peak_dir[p]*rtd);
    }
        fprintf(ofpp, "RSS=%g\n",sqrt(tmp*rtd*rtd));
      }
      if(tmp<*mse){
    *mse=tmp;
    min_idx=t;
        for(int p=num_peaks;p<DEFAULT_MAX_SOLUTIONS;p++){
      peak_dir[p]=tmp_peak_dir[p];
    }
      }
    }
    if(debug) fprintf(ofpp,"Best trial is %d, RSS=%g\n\n\n", min_idx,
              sqrt((*mse)*rtd*rtd));

    //----------------------------------------//
    // Compute new Search Intervals           //
    //----------------------------------------//

    // Delete Unpromising Search Intervals
    AngleInterval* interval=intervals.GetHead();
    int i=0;
    while(interval){
      if(num_trial_ambigs[min_idx][i]==0){
    interval=intervals.RemoveCurrent();
    delete interval;
        interval=intervals.GetCurrent();
      }
      else interval=intervals.GetNext();
      i++;
    }

    if(debug){
      fprintf(ofpp,"Intervals After Deletion: %d\n", intervals.NodeCount());
      for(interval=intervals.GetHead();interval;
      interval=intervals.GetNext()){
    fprintf(ofpp,"[%g,%g],",(interval->left)*rtd,(interval->right)*rtd);
      }
    }
    // Check If Done
    float max_interval_width=0.0;
    interval=intervals.GetHead();
    while(interval){
      float tmp=interval->GetWidth();
      if(tmp>max_interval_width) max_interval_width=tmp;
      interval=intervals.GetNext();
    }
    if(max_interval_width<_phiStepSize/2) finished=1;
    // If Not Bisect Promising Search Intervals
    if(!finished){
      if(!intervals.Bisect()) return(0);
      if(debug){
    fprintf(ofpp,"Intervals After Bisection: %d\n", intervals.NodeCount());
    for(interval=intervals.GetHead();interval;
        interval=intervals.GetNext()){
      fprintf(ofpp,"[%g,%g],",(interval->left)*rtd,(interval->right)*rtd);
    }
      }
    }

    free_array((void*)num_trial_ambigs, 2, num_trials, num_intervals);
  }
  if(debug) fclose(ofpp);
  delete tmp_peak_dir;
  num++;
  return(1);
}

//----------------------------------//
// DeleteBadPeaks                   //
//----------------------------------//
int
GMF::DeleteBadPeaks(
     WVC* wvc,
     float* peak_dir,
     int* num_peaks,
     float mse){
  float tmp_dir[DEFAULT_MAX_SOLUTIONS];
  float tmp_mse[DEFAULT_MAX_SOLUTIONS];
  float repl_dir[DEFAULT_MAX_SOLUTIONS];
  float max_mse=0;
  int max_idx=-1;
  if(*num_peaks==1) return(1); // for now always keep one original peak
  int num_avail=DEFAULT_MAX_SOLUTIONS-*num_peaks;
  WindVectorPlus* wvp=wvc->ambiguities.GetHead();
  for(int idx=0;idx<*num_peaks;idx++){
    for(int c=0;c<*num_peaks-1;c++){
      if(c<idx) tmp_dir[c+num_avail]=peak_dir[c];
      else tmp_dir[c+num_avail]=peak_dir[c+1];
    }
    for(int c=0;c<num_avail;c++){
      tmp_dir[c]=tmp_dir[num_avail];
    }
    tmp_mse[idx]=mse;
    if(!GetMinEstimateMSE(tmp_dir,DEFAULT_MAX_SOLUTIONS-1,&tmp_mse[idx]))
      return(0);

    // calculate best peak and keep it no matter what
    // best is the one with the highest MSE when removed
    if(tmp_mse[idx]>max_mse){
      max_mse=tmp_mse[idx];
      max_idx=idx;
    }
    repl_dir[idx]=tmp_dir[DEFAULT_MAX_SOLUTIONS-1];
    wvp=wvc->ambiguities.GetNext();
  }
  wvp=wvc->ambiguities.GetHead();
  int num_removed=0;
  for(int idx=0;idx<*num_peaks;idx++){
    if(idx!=max_idx && mse>tmp_mse[idx] && ANGDIF(repl_dir[idx],peak_dir[idx])>_phiStepSize){
      wvp=wvc->ambiguities.RemoveCurrent();
      delete wvp;
      (*num_peaks)--;
    }
    else{
      peak_dir[idx-num_removed]=peak_dir[idx];
      wvp=wvc->ambiguities.GetNext();
    }
  }
  return(1);
}

//---------------------------------------------------------------------//
// Estimated Direction MSE (only works after ConvertObjToPdf is run.)  //
//---------------------------------------------------------------------//
float
GMF::EstimateDirMSE(
       AngleIntervalListPlus* alp)
{
 AngleInterval* old=alp->GetCurrent();
 float MSE=0;
 for(int c=0;c<_phiCount;c++){
   float dir=c*_phiStepSize;
   float dir2=alp->GetNearestValue(dir);
   float tmp=ANGDIF(dir,dir2);
   MSE+=tmp*tmp*_bestObj[c];
 }

 for(AngleInterval*ai=alp->GetHead();ai!=old;ai=alp->GetNext());
 return(MSE);
}

//---------------------------------------------------------------------//
// Estimated Direction MSE (only works after ConvertObjToPdf is run.)  //
//---------------------------------------------------------------------//
float
GMF::EstimateDirMSE(
       float* peak_dir,
       int    num_peaks)

{
  float retval=0;
  for(int c=0;c<_phiCount;c++){
    float min_dist=pi;
    for(int p=0;p<num_peaks;p++){
      float tmp=ANGDIF(c*_phiStepSize,peak_dir[p]);
      if(tmp<min_dist) min_dist=tmp;
    }
    retval+=min_dist*min_dist*_bestObj[c];
  }
  return(retval);
}

//-----------------------//
// GMF::RetrieveWinds_S3 //
//-----------------------//

#define S3_PROB_THRESHOLD 0.8
#define S3_MSE_THRESHOLD 100.0*dtr*dtr
int
GMF::RetrieveWinds_S3(
    MeasList*  meas_list,
    Kp*        kp,
    WVC*       wvc,
    int        s4_flag=0)
{
    //--------------------------------//
    // generate coarse solution curve //
    //--------------------------------//

    if (_phiCount != H2_PHI_COUNT)
        SetPhiCount(H2_PHI_COUNT);
    Calculate_Init_Wind_Solutions(meas_list,kp,wvc);
    Optimize_Wind_Solutions(meas_list,kp,wvc);
    if(!CopyBuffersGSToPE()) return(0);
    wvc->Rank_Wind_Solutions();

    // Copy arrays to wvc
    wvc->directionRanges.dirIdx.SpecifyWrappedCenters(0,two_pi,_phiCount);
    wvc->directionRanges.bestObj=(float*)malloc(sizeof(float)*_phiCount);
    for(int c=0;c<_phiCount;c++) wvc->directionRanges.bestObj[c]=_bestObj[c];
    wvc->directionRanges.bestSpd=(float*)malloc(sizeof(float)*_phiCount);
    for(int c=0;c<_phiCount;c++) wvc->directionRanges.bestSpd[c]=_bestSpd[c];

    ConvertObjToPdf();

    //------//
    // sort //
    //------//

    wvc->SortByObj();


    //-------------------------------------------//
    // Determine Direction Intervals Comprising  //
    // (S3_PROB_THRESHOLD)*100% of the probability//
    //-------------------------------------------//

    if(!s4_flag){
      if(!BuildDirectionRanges(wvc,S3_PROB_THRESHOLD)) return(0);
    }

    //------------------------------------------//
    // limit to DEFAULT_MAX_SOLUTIONS solutions //
    //------------------------------------------//

    int delete_count = wvc->ambiguities.NodeCount() - DEFAULT_MAX_SOLUTIONS;
    if (delete_count > 0)
    {
        fprintf(stderr, "Too many solutions: deleting %d\n", delete_count);
        for (int i = 0; i < delete_count; i++)
        {
            wvc->ambiguities.GotoTail();
            wvc->directionRanges.GotoTail();
            WindVectorPlus* wvp = wvc->ambiguities.RemoveCurrent();
        AngleInterval* ai = wvc->directionRanges.RemoveCurrent();
            delete wvp;
        delete ai;
        }
    }

    return(1);
}

int
GMF::BuildDirectionRangesByMSE(
     WVC*   wvc,
     float threshold){

     int num=wvc->ambiguities.NodeCount();
     if(num==0) return(1);

     // Initialize Ranges to Width 0
     int offset=0, minoffset=-1;
     int right=0;
     for(WindVectorPlus* wvp=wvc->ambiguities.GetHead();wvp;
     wvp=wvc->ambiguities.GetNext(), offset++){
       AngleInterval* ai=new AngleInterval;
       ai->SetLeftRight(wvp->dir,wvp->dir);
       wvc->directionRanges.Append(ai);
     }
     wvc->directionRanges.dirIdx.SpecifyWrappedCenters(0,two_pi,_phiCount);
     wvc->directionRanges.bestSpd=(float*)malloc(sizeof(float)*_phiCount);
     for(int c=0;c<_phiCount;c++) wvc->directionRanges.bestSpd[c]=_bestSpd[c];

     float minMSE=180*180*dtr*dtr;
     while(minMSE>threshold){
       offset=0;
       for(AngleInterval* ai=wvc->directionRanges.GetHead();ai;
           ai=wvc->directionRanges.GetNext()){
     float oldright=ai->right;
         float oldleft=ai->left;
         ai->SetLeftRight(oldleft-_phiStepSize,oldright);
         float MSE=EstimateDirMSE(&(wvc->directionRanges));
         ai->left=oldleft;
         if(MSE<minMSE){
       minMSE=MSE;
           minoffset=offset;
           right=0;
     }
         ai->SetLeftRight(oldleft,oldright+_phiStepSize);
         MSE=EstimateDirMSE(&(wvc->directionRanges));
         ai->right=oldright;
         if(MSE<minMSE){
       minMSE=MSE;
           minoffset=offset;
           right=1;
     }
     offset++;
       }
       AngleInterval* ai=wvc->directionRanges.GetHead();
       for(int c=0;c<minoffset;c++) ai=wvc->directionRanges.GetNext();
       if(right) ai->SetLeftRight(ai->left,ai->right+_phiStepSize);
       else ai->SetLeftRight(ai->left-_phiStepSize,ai->right);
     }
     return(1);
}


#define INTERP_RATIO 8
#define MERGE_BORDERS 0
int
GMF::BuildDirectionRanges(
     WVC*   wvc,
     float threshold){
     //---------------->>>>>>>>>>>>>>>>>>>>>>>> debugging tools
     // static int wvcno=0;
     // static FILE* dbg=fopen("debugfile","w");
     // wvcno++;
     //---------------->>>>>>>>>>>>>>>>>>>>>>>> debugging tools
     int pdfarraysize=_phiCount;
     float pdfstepsize=_phiStepSize;
     float* pdf = _bestObj;

     // Method reduce size of probability bins by a factor of INTERP_RATIO
     // in order to eliminate quantization effects.
     // Cubic Spline Interpolation is used.

     if(INTERP_RATIO>1){

       pdfarraysize*=INTERP_RATIO;
       pdfstepsize/=INTERP_RATIO;
       pdf=new float[pdfarraysize];
       float sum=0.0;

       // Set up arrays for wraparound cubic spline
       // By including four extra wrapped points on each end
       double* xarray = new double[_phiCount+8];
       double* yarray = new double[_phiCount+8];
       for(int i = 0; i<4;i++){
     xarray[i]=_phiStepSize*(i-4);
     yarray[i]=_bestObj[i+_phiCount-4];
       }
       for(int i=4;i<_phiCount+4;i++){
     xarray[i]=_phiStepSize*(i-4);
     yarray[i]=_bestObj[i-4];
       }
       for(int i=_phiCount+4;i<_phiCount+8;i++){
     xarray[i]=_phiStepSize*(i-4);
     yarray[i]=_bestObj[i-_phiCount-4];
       }

       // Compute second derivates for cubic spline
       double* y2array = new double[_phiCount+8];
       cubic_spline(xarray,yarray,_phiCount+8,1e+40,1e+40,y2array);

       double xvalue, yvalue;
       for(int c=0;c<pdfarraysize;c++){

         // compute interpolation direction
     xvalue=c*pdfstepsize;

         //Interpolate
         interpolate_cubic_spline(xarray,yarray,y2array,_phiCount+8,xvalue,
                  &yvalue);

         // To eliminate gross errors from spline
         // bound below by 0.5*min two neighboring original samples
         // and above by 2.0* max two neighboring original samples

         int idxleft=int(xvalue/_phiStepSize)%_phiCount;
         int idxright=(idxleft+1)%_phiCount;
         float lower_bound=_bestObj[idxleft];
         float upper_bound=_bestObj[idxright];
         if(lower_bound > upper_bound){
       lower_bound=_bestObj[idxright];
       upper_bound=_bestObj[idxleft];
     }
         lower_bound*=0.5;
         upper_bound*=2;
         if(yvalue<lower_bound) yvalue=lower_bound;
         if(yvalue>upper_bound) yvalue=upper_bound;
         pdf[c]=yvalue;
         sum+=pdf[c];
       }

       for(int c=0;c<pdfarraysize;c++) pdf[c]/=sum;

       delete[] xarray;
       delete[] yarray;
       delete[] y2array;
     } // END If( INTERP_RATIO > 1)

     int num=wvc->ambiguities.NodeCount();
     if(num==0) return(1);
     AngleInterval* range=new AngleInterval[num];

     // Initialize Ranges to Width 0
     int offset=0;
     for(WindVectorPlus* wvp=wvc->ambiguities.GetHead();wvp;
     wvp=wvc->ambiguities.GetNext(), offset++){
       range[offset].SetLeftRight(wvp->dir,wvp->dir);
     }

     // Initialize intermediate variables
     float prob_sum=0;
     int* dir_include=new int[pdfarraysize];
     for(int c=0;c<pdfarraysize;c++) dir_include[c]=0;
     int* left_idx= new int[num];
     int* right_idx=new int[num];
     for(int c=0;c<num;c++){
        left_idx[c]=int(floor(range[c].left/pdfstepsize));
        right_idx[c]=(left_idx[c]+1)%pdfarraysize;
     }
     //Add the maximal probability among directions not included in range
     //to prob_sum, and expand ranges accordingly.
     float max=0.0;
     while(1){

       // Determine maximum available directions by sliding down the
       // peaks on both sides
       max=0.0;
       int max_idx=-1;
       int right=0;
       for(int c=0;c<num;c++){
     if(pdf[left_idx[c]]>max && !dir_include[left_idx[c]]){
       max=pdf[left_idx[c]];
       right=0;
       max_idx=c;
     }
     if(pdf[right_idx[c]]>max && !dir_include[right_idx[c]]){
       max=pdf[right_idx[c]];
       right=1;
       max_idx=c;
     }
       }
       if(max==0.0){
     fprintf(stderr,"GMF::BuildDirectionRanges() Max=0.0?????\n");
         fprintf(stderr,"max=%g prob_sum=%g thresh=%g\n",max,prob_sum,threshold);
         for(int c=0;c<num;c++)
       fprintf(stderr,"%d left %d right %d\n",c,left_idx[c],right_idx[c]);

         break;
       }
       // Add maximum to total included probability
       prob_sum+=max;

       // Break if threshold reached
       if(prob_sum>threshold) break;

       // Expand range to include best available direction
       //  and update intermediate variables
       if(right){
     float tmp_left=range[max_idx].left;
         float tmp_right=right_idx[max_idx]*pdfstepsize+0.5*pdfstepsize;
         dir_include[right_idx[max_idx]]=1;
     range[max_idx].SetLeftRight(tmp_left,tmp_right);
     right_idx[max_idx]++;
     right_idx[max_idx]%=pdfarraysize;
       }
       else{
     float tmp_right=range[max_idx].right;
         float tmp_left=left_idx[max_idx]*pdfstepsize-0.5*pdfstepsize;
         dir_include[left_idx[max_idx]]=1;
     range[max_idx].SetLeftRight(tmp_left,tmp_right);
     left_idx[max_idx]--;
     if(left_idx[max_idx]<0) left_idx[max_idx]+=pdfarraysize;
       }
     }


     //  2) Merge bordering intervals (for now but if this causes problems then
     //   eliminate this step and replace with trough finding.


     if(MERGE_BORDERS){

       // Merge bordering intervals.
       for(int c=0;c<num-1;c++){
     for(int d=c+1; d< num; d++){
       if(range[c].left == range[d].right){
         // incorporate range d into range c
              range[c].left = range[d].left;
          // eliminate range[d]
              for(int e=d;e<num-1;e++){
        range[e]=range[e+1];
              }
              num--;
              // eliminate ambiguity d
              wvc->ambiguities.GetByIndex(d);
              WindVectorPlus* wvp=wvc->ambiguities.RemoveCurrent();
              delete wvp;
       }
       else if(range[c].right == range[d].left){
          // incorporate range d into range c
              range[c].right = range[d].right;
          // eliminate range[d]
              for(int e=d;e<num-1;e++){
        range[e]=range[e+1];
              }
              num--;
              // eliminate ambiguity d
              wvc->ambiguities.GetByIndex(d);
              WindVectorPlus* wvp=wvc->ambiguities.RemoveCurrent();
              delete wvp;
       }
     }
       }

     }

     for(int c=0;c<num;c++){
       AngleInterval* ai=new AngleInterval;
       *ai=range[c];
       wvc->directionRanges.Append(ai);
     }

     //------------------------------------------>>>> debugging tools
     //for(int c=0;c<pdfarraysize;c++)
     //  fprintf(dbg,"%g %g PDF:WVC#%d\n",c*pdfstepsize*rtd,pdf[c],wvcno);
     //fprintf(dbg,"& PDF:WVC#%d\n",wvcno);
     //for(int c=0;c<_phiCount;c++)
     //  fprintf(dbg,"%g %g BestObj:WVC#%d\n",c*_phiStepSize*rtd,_bestObj[c],wvcno);
     //fprintf(dbg,"& BestObj:WVC#%d\n",wvcno);
     //for(int c=0;c<num;c++)
     //      fprintf(dbg,"%d %g Left:WVC#%d\n",c,rtd*range[c].left,wvcno);
     //fprintf(dbg,"& Left:WVC#%d\n",wvcno);
     //for(int c=0;c<num;c++)
     //      fprintf(dbg,"%d %g Right:WVC#%d\n",c,rtd*range[c].right,wvcno);
     //fprintf(dbg,"& Right:WVC#%d\n",wvcno);
     //------------------------------------------>>>> debugging tools

     if (INTERP_RATIO > 1) delete[] pdf;
     delete[] range;
     delete[] dir_include;
     delete[] left_idx;
     delete[] right_idx;
     return(1);
}

//---------------------//
// GMF::RemoveBadCopol //
//---------------------//

int
GMF::RemoveBadCopol(
    MeasList*  meas_list,
    Kp*        kp)
{
    // Compute look_indices, sums, and counts

    float sum[4] = {0.0, 0.0, 0.0, 0.0};
    int count[4] = {0, 0, 0, 0};
    int nc = meas_list->NodeCount();
    int* look_idx = new int[nc];
    Meas* meas = meas_list->GetHead();
    for (int c = 0; c < nc; c++)
    {
        switch (meas->measType)
        {
        case Meas::HH_MEAS_TYPE:
            if (meas->scanAngle < pi / 2 || meas->scanAngle > 3 * pi / 2)
                look_idx[c] = 0;
            else
                look_idx[c] = 1;
            break;
        case Meas::VV_MEAS_TYPE:
            if (meas->scanAngle < pi / 2 || meas->scanAngle > 3 * pi / 2)
                look_idx[c] = 2;
            else
                look_idx[c] = 3;
            break;
        default:
            look_idx[c] = -1;
            break;
        }
        if (look_idx[c] >= 0)
        {
            sum[look_idx[c]] += meas->value;
            count[look_idx[c]]++;
        }
        meas = meas_list->GetNext();
    }

    // Remove Bad Measurements
    meas = meas_list->GetHead();
    for (int c = 0; c < nc; c++)
    {
        int d = look_idx[c];
        if (d < 0)
            meas = meas_list->GetNext();
        else if (count[d] < 3)
            meas = meas_list->GetNext();
        else
        {
            double s0_ave = (sum[d] - meas->value) / (count[d] - 1);
            double vpc;
            kp->GetVpc(meas, s0_ave, &vpc);
            float std = sqrt(vpc);
            if (fabs(s0_ave-meas->value) > 10.0 * std)
            {
                meas = meas_list->RemoveCurrent();
                delete meas;
                meas = meas_list->GetCurrent();
            }
            else
                meas = meas_list->GetNext();
        }
    }
    delete[] look_idx;
    return(1);
}
