//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_gmf_c[] =
    "@(#) $Id$";

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <malloc.h>
#include <math.h>
#include "GMF.h"
#include "GSparameters.h"
#include "Meas.h"
#include "Interpolate.h"
#include "Constants.h"
#include "Beam.h"
#include "List.h"

//=====//
// GMF //
//=====//

GMF::GMF()
:   retrieveUsingKpcFlag(1), retrieveUsingKpmFlag(1), retrieveUsingKpriFlag(1),
    retrieveUsingKprsFlag(1), retrieveUsingLogVar(0), smartNudgeFlag(0),
    _spdTol(DEFAULT_SPD_TOL), _sepAngle(DEFAULT_SEP_ANGLE),
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
		exit(-1);
	}

	return;
}

GMF::~GMF()
{
	free(_bestSpd);
	free(_bestObj);
	free(_copyObj);

	if (! _speed_buffer) free(_speed_buffer);
	if (! _objective_buffer) free(_objective_buffer);
	if (! _dir_mle_maxima) free(_dir_mle_maxima);

	return;
}

//------------------//
// GMF::SetPhiCount //
//------------------//

int
GMF::SetPhiCount(
	int		phi_count)
{
	if (!_bestSpd && _phiCount == phi_count)
	{	// No adjustment needed
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

//------------------//
// GMF:ReadOldStyle //
//------------------//

int GMF::ReadOldStyle(
	const char*		filename)
{
	int fd = open(filename, O_RDONLY);
	if (fd == -1)
		return(0);

	int dummy;
	read(fd, &dummy, sizeof(int));

	_polCount = 2;

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
	_chiStep = two_pi / _chiCount;		// 5 degrees

	if (! _Allocate())
		return(0);

	float value;
	for (int pol_idx = 0; pol_idx < _polCount; pol_idx++)
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
					*(*(*(*(_value+pol_idx)+inc_idx)+spd_idx)+chi_idx) =
						(float)value;

					int chi_idx_2 = (_chiCount - chi_idx) % _chiCount;
					*(*(*(*(_value+pol_idx)+inc_idx)+spd_idx)+chi_idx_2) =
						(float)value;
				}
			}
		}
	}

	//----------------------//
	// zero the 0 m/s model //
	//----------------------//

	int spd_idx = 0;
	for (int pol_idx = 0; pol_idx < _polCount; pol_idx++)
	{
		for (int chi_idx = 0; chi_idx < _chiCount; chi_idx++)
		{
			for (int inc_idx = 0; inc_idx < _incCount; inc_idx++)
			{
				*(*(*(*(_value+pol_idx)+inc_idx)+spd_idx)+chi_idx) = 0.0;
			}
		}
	}

	close(fd);
	return(1);
}

//---------------//
// GMF::GetCoefs //
//---------------//

int
GMF::GetCoefs(
	PolE		pol,
	float		inc,
	float		spd,
	float*		A0,
	float*		A1,
	float*		A1_phase,
	float*		A2,
	float*		A2_phase,
	float*		A3,
	float*		A3_phase,
	float*		A4,
	float*		A4_phase)
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
			GetInterpolatedValue(pol, inc, spd, chi, &val);
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
	FILE*		ofp,
	MeasList*	meas_list,
	Kp*			kp)
{
  //---------------------------------//
  // Set objective function values   //
  //---------------------------------//

  WVC* wvc = new WVC();
  if(! RetrieveWinds(meas_list, kp, wvc)) return(0);

  //----------------------------------------------//
  // Determine Maximum Objective Function value   //
  //----------------------------------------------//

  double scale=0;
  WindVectorPlus* head=wvc->ambiguities.GetHead();
  if(head!=NULL) scale=head->obj;
  else return(0);

  //---------------------------------------------//
  // Calculate and print out probabilities       //
  //---------------------------------------------//
  float sum=0;
  for (int i = 0; i < _phiCount; i++)
    { _bestObj[i]=exp((_bestObj[i]-scale)/2);
      sum+=_bestObj[i];
    }
  for (int i = 0; i < _phiCount; i++)
    {
      fprintf(ofp, "%g %g\n", (float)i * _phiStepSize * rtd,
	      _bestObj[i]/sum);
    }

  fprintf(ofp,"&\n");
  delete wvc;
  return(1);
}

//--------------------------//
// GMF::WriteSolutionCurves //
//--------------------------//

int
GMF::WriteSolutionCurves(
	FILE*		ofp,
	MeasList*	meas_list,
	Kp*			kp)
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
	RetrieveWinds(meas_list, kp, wvc);
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
	float*		min_obj,
	float*		max_obj)
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
	FILE*		ofp,
	float		min_obj,
	float		max_obj)
{

	if (min_obj == max_obj)
	{
		fprintf(stderr,"GMF::WriteObjectiveCurve: Invalid obj limits\n");
		exit(-1);
	}
	float scale = 1.0 / (max_obj - min_obj);
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
	FILE*		ofp,
	float		min_obj,
	float		max_obj)
{

	float dir_spacing =  wind_dir_intv_init;
	int num_dir_samples = (int)(360. / dir_spacing) + 2 ;

	if (min_obj == max_obj)
	{
		fprintf(stderr,"GMF::WriteGSObjectiveCurve: Invalid obj limits\n");
		exit(-1);
	}

	float scale = 1.0 / (max_obj - min_obj);

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
	FILE*		ofp,
	WVC*		wvc,
	float		min_obj,
	float		max_obj)
{

	if (min_obj == max_obj)
	{
		fprintf(stderr,"GMF::AppendSolutions: Invalid obj limits\n");
		exit(-1);
	}

	float scale = 1.0 / (max_obj - min_obj);

	//----------------------------//
	// write individual solutions //
	//----------------------------//

	for (WindVectorPlus* wvp = wvc->ambiguities.GetHead(); wvp;
		wvp = wvc->ambiguities.GetNext())
	{
		fprintf(ofp, "%g %g\n", wvp->dir * rtd, (wvp->obj-min_obj)*scale);
	}

	return(1);
}

//----------------------------//
// GMF::CheckRetrieveCriteria //
//----------------------------//
// Returns 1 if wind retrieval should be performed, Otherwise 0

int
GMF::CheckRetrieveCriteria(
	MeasList*	meas_list)
{
	//------------------------------------------//
	// check for minimum number of measurements //
	//------------------------------------------//

	if (meas_list->NodeCount() < MINIMUM_WVC_MEASUREMENTS)
		return(0);

	//-------------------------------------//
	// check for minimum azimuth diversity //
	//-------------------------------------//

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
			if (azdiv > MINIMUM_AZIMUTH_DIVERSITY)
			{
				goto passed;
				break;
			}
		}

		// restore the current
		meas_list->SetCurrentNode(current);
	}
	return(0);		// failed diversity check

	passed:			// passed diversity check

	return(1);
}

//--------------------//
// GMF::RetrieveWinds //
//--------------------//

int
GMF::RetrieveWinds(
	MeasList*	meas_list,
	Kp*			kp,
	WVC*		wvc)
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
//		fprintf(stderr, "can't smooth solution curve\n");
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
	MeasList*	meas_list,
	Kp*			kp,
	WVC*		wvc)
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

//-------------------------------------//
// GMF::_ObjectiveToProbability         //
//-------------------------------------//
int
GMF::_ObjectiveToProbability(float scale, int radius)
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
	MeasList*         meas_list,
        Kp*                      kp,
        WVC*                    wvc,
        float                 one_peak_width,
        float                 two_peak_separation_threshold,
	float                 threshold,
	int                   max_num_ambigs)

{
        if(! RetrieveWinds(meas_list,kp,wvc)) return(0);
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
	MeasList*	meas_list,
	Kp*			kp)
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
		ax = x1 - (x1 * 0.05);
		if (ax < _spdMin)
			ax = _spdMin;
		cx = x1 + (x2 * 0.05);
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
	int delta_phi = 0;		// start with no smoothing

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
	WVC*		wvc)
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
	WVC*		wvc)
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

//-------------------------//
// GMF::_ObjectiveFunction //
//-------------------------//

float
GMF::_ObjectiveFunction(
	MeasList*	meas_list,
	float		spd,
	float		phi,
	Kp*			kp)
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
		GetInterpolatedValue(meas->pol, meas->incidenceAngle, spd, chi,
			&trial_value);

		//------------------------------------------------------------//
		// find the difference between the trial and measured sigma-0 //
		//------------------------------------------------------------//

		float s = trial_value - meas->value;

		//-------------------------------------------------------//
		// calculate the expected variance for the trial sigma-0 //
		//-------------------------------------------------------//

		if (kp)
		{
			//--------------//
			// Kpc variance //
			//--------------//

			double vpc = 0.0;
			if (retrieveUsingKpcFlag)
			{
				if (! kp->GetVpc(meas, trial_value, &vpc))
					return(0);
			}

			//-----//
			// Kpm //
			//-----//

			double kpm2 = 0.0;
			if (retrieveUsingKpmFlag)
			{
				if (! kp->GetKpm2(meas->pol, spd, &kpm2))
					return(0);
			}

			//------//
			// Kpri //
			//------//

			double kpri2 = 0.0;
			if (retrieveUsingKpriFlag)
			{
				if (! kp->GetKpri2(&kpri2))
					return(0);
			}

			//------//
			// Kprs //
			//------//

			double kprs2 = 0.0;
			if (retrieveUsingKprsFlag)
			{
				if (! kp->GetKprs2(meas, &kprs2))
					return(0);
			}

			//------------------------//
			// calculate the variance //
			//------------------------//

			double var = vpc +
				(kpm2 + kpri2 + kprs2) * trial_value * trial_value;

			if (var == 0.0)
			{	// variances all turned off, so use uniform weighting.
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
		else
		{
			//--------------//
			// no kp at all //
			//--------------//

			fv += s*s;
		}
	}
	return(-fv);
}

//----------------------//
// GMF::GSRetrieveWinds //
//----------------------//

int
GMF::GSRetrieveWinds(
	MeasList*	meas_list,
	Kp*			kp,
	WVC*		wvc)
{

//
//  Step 1:  Find an initial set of coarse wind solutions.
//

	Calculate_Init_Wind_Solutions(meas_list,kp,wvc);

	WindVectorPlus* wvp = NULL;

//	for (wvp=wvc->ambiguities.GetHead(); wvp;
//		 wvp = wvc->ambiguities.GetNext())
//	{
//		printf("init obj,spd,dir= %g %g %g\n",wvp->obj,wvp->spd,rtd*wvp->dir);
//	}

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

//	for (wvp=wvc->ambiguities.GetHead(); wvp;
//		 wvp = wvc->ambiguities.GetNext())
//	{
//		printf("final obj,spd,dir= %g %g %g\n",wvp->obj,wvp->spd,rtd*wvp->dir);
//	}
//	exit(0);

	return(1);
}

//------------------------------------//
// GMF::Calculate_Init_Wind_Solutions //
//------------------------------------//

int
GMF::Calculate_Init_Wind_Solutions(
	MeasList*	meas_list,
	Kp*			kp,
	WVC*		wvc)
{

//
//!Description:
// 	        This routine calculates an initial set of wind solutions
//               using an iterative "coarse search" mechanism based on MLE
//               computations.
//

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

	for (k=2; k <= num_dir_samples-1; k++)
	{
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

            if (minus_objective > center_objective  &&
               minus_objective > plus_objective    &&
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
		}	// while loop


         if (center_objective >= minus_objective  &&
            center_objective >= plus_objective   &&
            good_speed)
		{
            diff_objective_1 = plus_objective - minus_objective;
            diff_objective_2 = (plus_objective + minus_objective)
                            - 2.0 * center_objective;

            _speed_buffer [k] = center_speed  - 0.5
                            * (diff_objective_1 / diff_objective_2)
                            * wind_speed_intv_init;

			// Re-evaluate objective function to avoid interpolation bumps
			// that introduce artificial peaks.
//			_objective_buffer[k] = _ObjectiveFunction(meas_list,
//										_speed_buffer[k],dtr*angle,kp);
            _objective_buffer [k] = center_objective
                                - (diff_objective_1*diff_objective_1
                                  /diff_objective_2) / 8.0;
		}
	}	// end of angular k loop


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
		if (_objective_buffer[k] > _objective_buffer[k-1] &&
			_objective_buffer[k] > _objective_buffer[k+1])
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
//		printf("number greater than 10 = %d\n",num_mle_maxima);
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
//		printf("dir_mle_maxima = %d\n",dir_mle_maxima);

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
//		printf("init: obj,spd,dir= %g %g %g\n",wvp->obj,wvp->spd,rtd*wvp->dir);
		if (! wvc->ambiguities.Append(wvp))
		{
			delete wvp;
			return(0);
		}
	}


	return(1);
}

//------------------------------//
// GMF::Optimize_Wind_Solutions //
//------------------------------//

int
GMF::Optimize_Wind_Solutions(
	MeasList*	meas_list,
	Kp*			kp,
	WVC*		wvc)
{

//
//!Description:
// 	        This routine does optimization for the initial set of
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

	float	wr_wind_speed[wind_max_solutions];
	float	wr_wind_dir[wind_max_solutions];
	float	wr_mle[wind_max_solutions];

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

			}	// if (shift_pattern /= 0)

		}	// while loop

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
//				wind_speed_intv_opti / sqrt (float(wr_count));
//            wr_wind_dir_err [ambig] =
//				wind_dir_intv_opti /sqrt ((float)(wr_count));
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
//					* sqrt (fabs (0.5 * q20 / determinant));
//                wr_wind_dir_err [ambig] = wind_dir_intv_opti
//					* sqrt (fabs (0.5 * q02 / determinant));
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
//					= wind_speed_intv_opti
//					* sqrt (fabs (0.5 * q20 / determinant));
//                  wr_wind_dir_err [ambig] = wind_dir_intv_opti
//					* sqrt (fabs (0.5 * q02 / determinant));
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
	}	//  The big ambiguity loop

	// Copy data from wr_ arrays. (convert to radians)
	i = 0;
	for (WindVectorPlus* wvp = wvc->ambiguities.GetHead();
		 wvp; wvp = wvc->ambiguities.GetNext())
	{
		wvp->spd = wr_wind_speed[i];
		wvp->dir = wr_wind_dir[i]*dtr;
		wvp->obj = wr_mle[i];
//		printf("opti: obj,spd,dir= %g %g %g\n",wvp->obj,wvp->spd,rtd*wvp->dir);
		i++;
        if (i >= wind_max_solutions) break;
	}


	return(1);

}

//-------------------//
// GMF::WriteObjXmgr //
//-------------------//

int
GMF::WriteObjXmgr(
	char*		basename,
	int			panelcount,
	WVC*		wvc)
{

  static FILE* objxmgr_file = NULL;
  static char filename[256];
  static count = 0;
  static graphnum = 0;

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

#define H1_THRESH_FRACTION    0.1

//----------------------//
// GMF::SolutionCurveH1 //
//----------------------//
// For each of the directions, find the speed that maximizes the
// objective function.  Fills in the _bestSpd and _bestObj arrays
// The H1 version uses adaptive thresholding and a sub-function

#define H1_DELTA_SPEED_FRACTION    0.2
#define H1_THRESH_FRACTION         0.1

int
GMF::SolutionCurveH1(
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

/*
//----------------------//
// GMF::RetrieveWindsH1 //
//----------------------//

int
GMF::RetrieveWindsH1(
    MeasList*  meas_list,
    Kp*        kp,
    WVC*       wvc)
{
    //--------------------------------//
    // generate coarse solution curve //
    //--------------------------------//

    if (_phiCount != 45)
        SetPhiCount(45);
    SolutionCurveH1(meas_list, kp);

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
    int overlap_idx[10];     // index of overlapping point peak

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
            overlap_idx[peak_idx] = -1;     // no overlaps yet
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
                //------------------------------//
                // wide overlaps point: mark it //
                //------------------------------//

                if (overlap_idx[this_idx] != -1)
                {
                    // already been marked, just absorb the old one and the new
                    fprintf(stderr, "Multiple overlaps: absorbing\n");
                    valid_peak[overlap_idx[this_idx]] = 0;  // absorb old
                    valid_peak[other_idx] = 0;  // absorb new
                    number_ambigs[this_idx] = 0;    // flag as having no peak
                }
                overlap_idx[this_idx] = other_idx;
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
                number_ambigs[this_idx] = 0;    // flag as having no peak
            }
        }
    }

    //-----------------------------------------//
    // determine how many ambiguities are left //
    //-----------------------------------------//

    int ambiguities = 0;
    for (int this_idx = 0; this_idx < peak_count; this_idx++)
    {
        if (! valid_peak[this_idx])
            continue;

        ambiguities += number_ambigs[this_idx];
    }
    int ambiguities_left = DEFAULT_MAX_SOLUTIONS - ambiguities;

    //--------------------------//
    // assign extra ambiguities //
    //--------------------------//

    while (ambiguities_left > 0)
    {
        int max_idx = -1;
        float max_use_width = 0.0;
        for (peak_idx = 0; peak_idx < peak_count; peak_idx++)
        {
            if (! valid_peak[peak_idx])
                continue;

            // determine number of amiguities (including overlap)
            int use_num = number_ambigs[peak_idx];
            if (overlap_idx[peak_idx] != -1 &&
                valid_peak[overlap_idx[peak_idx]])
            {
                use_num += number_ambigs[overlap_idx[peak_idx]];
            }

            float use_width;
            if (use_num)
            {
                use_width = (float)width_rad[peak_idx] / (float)(use_num + 1);
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
            ambiguities_left--;

            //-------------------------//
            // check for overlap index //
            //-------------------------//

            if (overlap_idx[max_idx] != -1)
            {
                //--------------//
                // absorb point //
                //--------------//

                number_ambigs[max_idx]++;                 // absorb ambiguity
                valid_peak[overlap_idx[max_idx]] = 0;  // erase point
                overlap_idx[max_idx] = -1;    // clear overlap index
            }
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
                else
                    wvp->obj = 0.0;
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
		WriteObjXmgr("toomany",10,wvc);
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
*/

//----------------------//
// GMF::RetrieveWindsH1 //
//----------------------//

int
GMF::RetrieveWindsH1(
    MeasList*  meas_list,
    Kp*        kp,
    WVC*       wvc)
{
    //--------------------------------//
    // generate coarse solution curve //
    //--------------------------------//

    if (_phiCount != 45)
        SetPhiCount(45);
    SolutionCurveH1(meas_list, kp);

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
//		WriteObjXmgr("toomany",10,wvc);
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
