//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.			//
//==============================================================//

static const char rcs_id_kpr_c[] =
	"@(#) $Id$";

#include <fcntl.h>
#include <unistd.h>
#include "Kpr.h"
#include "Constants.h"
#include "Misc.h"
#include "Array.h"
#include "GenericGeom.h"
#include "CoordinateSwitch.h"

//======//
// Kpri //
//======//

Kpri::Kpri()
{
	return;
}

Kpri::~Kpri()
{
	return;
}

//----------------//
// Kpri::GetKpri2 //
//----------------//

int
Kpri::GetKpri2(
	double*		kpri2)
{
	*kpri2 = _kpPtGr*_kpPtGr;
	return(1);
}

int
Kpri::SetKpPtGr(
	double kp_ptgr)
{
        _kpPtGr=kp_ptgr;
	return(1);
}

//======//
// Kprs //
//======//

//----------------------------//
// Constructors               //
//----------------------------//

Kprs::Kprs()
:  _numBeams(0), _numScienceSlices(0), _numGuardSlicesEachSide(0),
   _numSlices(0), _numAzimuths(0), _minNumSamples(1),
   _scienceBandwidth(0.0),_guardBandwidth(0.0), _numSamples(NULL), _value(NULL)
{
  return;
}

Kprs::Kprs(int num_beams, float science_bandwidth,
	   float guard_bandwidth,
	   int num_science_slices, 
           int num_guard_slices_each_side,
	   int number_of_azimuth_bins,
	   int min_num_samples
){
  _numBeams=num_beams;
  _minNumSamples=min_num_samples;
  _scienceBandwidth=science_bandwidth;
  _guardBandwidth=guard_bandwidth;
  _numScienceSlices=num_science_slices;
  _numGuardSlicesEachSide=num_guard_slices_each_side;
  _numAzimuths=number_of_azimuth_bins;
  _numSlices=_numScienceSlices + 2* _numGuardSlicesEachSide;
  _Allocate();
  return;
}

//------------------------//
// Kprs::~Kprs    //
//------------------------//

Kprs::~Kprs()
{
	if (_value == NULL)
		return;

	free_array((void *)_value, 4, _numBeams, _numSlices,
		   _numSlices, _numAzimuths);
	free_array((void *)_numSamples, 4, _numBeams, _numSlices,
		   _numSlices, _numAzimuths);

	_value = NULL;
	_numSamples = NULL;
	return;
}

//--------------------------//
// Kprs::Accumulate         //
//--------------------------//

int 
Kprs::Accumulate(MeasSpotList* quiet, MeasSpotList* noisy)
{
  MeasSpot* noisy_spot=noisy->GetHead();
  MeasSpot* quiet_spot=quiet->GetHead();
  int spot_number=0;


  

  //--------------------//
  // For each Spot      //
  //--------------------//
  while(noisy_spot && quiet_spot){


    Meas* noisy_slice=noisy_spot->GetHead();
    Meas* quiet_slice=quiet_spot->GetHead();


    int slice_number=0;
    

    //----------------------//
    // For each Slice       //
    //----------------------//
    while(noisy_slice && quiet_slice){

      if( slice_number >= _numSlices){   
	fprintf(stderr,"Kprs::Accumulate-- Too many slices.\n");
	return(0);
      }

      for(int n=1; n<= _numSlices - slice_number; n++){
            Meas comp_quiet, comp_noise;
            if(!comp_quiet.Composite(quiet_spot,n)) return(0);
            if(!comp_noise.Composite(noisy_spot,n)) return(0);
            if(!Accumulate(&comp_quiet,&comp_noise))  return(0);
      }
      noisy_slice=noisy_spot->GetNext();
      quiet_slice=quiet_spot->GetNext();
      slice_number++;
    }
    

    if(slice_number < _numSlices){   
      fprintf(stderr,"Kprs::Accumulate-- Too few slices.\n");
      return(0);
    }

    if(noisy_slice || quiet_slice){
      fprintf(stderr,"Kprs::Accumulate-- SlicesPerSpot don't match\n");
      return(0);
    }  
    noisy_spot=noisy->GetNext();
    quiet_spot=quiet->GetNext();
    spot_number++;
  }


  if(noisy_spot || quiet_spot){
    fprintf(stderr,"Kprs::Accumulate-- SpotsPerFrame don't match\n");
    return(0);
  }
 
  return(1);
}

int
Kprs::Accumulate(Meas* quiet, Meas* noisy){
  float azi, sqrdif;
  int azi_idx;
  
  //---------------------------------//
  // Get Azimuth Index               //
  //---------------------------------//

  azi=quiet->scanAngle;
  if(azi < -(2*M_PI)){
    fprintf(stderr,"Kprs::Accumulate: Azimuth < -2PI\n");
    return(0);
  }
  azi=azi*_numAzimuths/(2*M_PI);
  if(azi<0) azi+=2*_numAzimuths;
  azi_idx=(int)azi;
  azi_idx%=_numAzimuths;

  int beam_idx = (int)(quiet->beamIdx);
  int start_slice_rel_idx = (int)(quiet->startSliceIdx);
  int start_slice_abs_idx;
  if(!rel_to_abs_idx(start_slice_rel_idx,_numSlices,&start_slice_abs_idx)){
   fprintf(stderr,"Kprs::Accumulate: rel_to_abs_idx failed.\n");
    return(0);
  }
  int num_slices_per_comp = (int)(quiet->numSlices);

  //-------------------------------------//
  // Increments number of samples in bin //
  //-------------------------------------//

  _numSamples[beam_idx][num_slices_per_comp-1][start_slice_abs_idx][azi_idx]++;

  //---------------------------------//
  // Add square of normalized Sigma0 //
  // difference to _value            //
  //---------------------------------//
  sqrdif=(quiet->value - noisy->value)/(quiet->value);
  sqrdif*=sqrdif;
  _value[beam_idx][num_slices_per_comp-1][start_slice_abs_idx][azi_idx]+=
    sqrdif;
  return(1);
}


int Kprs::Smooth(int filter_size){
  if (filter_size % 2 != 1){
    fprintf(stderr,"Kprs::Smooth: filter_size must be an odd number\n");
    return(0);
  }
  // Allocate buffer for mean filtering
  float* buffer=new float(_numAzimuths+filter_size-1);
  for(int b=0;b<_numBeams;b++){
   for(int n=0;n<_numSlices;n++){
    for(int s=0;s<_numSlices;s++){
 

      //-------------------------------------------//
      // copy kpr values to buffer with wraparound //
      //-------------------------------------------//
      for(int c=0; c< filter_size/2; c++){ 
        int a=_numAzimuths-filter_size/2+c;
	buffer[c]=_value[b][n][s][a];
      }
      for(int c=0; c< _numAzimuths; c++){ 
	buffer[c+filter_size/2]=_value[b][n][s][c];
      }
      for(int c=0; c< filter_size/2; c++){ 
        int a=_numAzimuths+filter_size/2+c;
	buffer[a]=_value[b][n][s][c];
      }
      
      //-- Perform Mean Filtering in order to Smooth --//
      for(int c=0; c<_numAzimuths; c++){
	_value[b][n][s][c]=mean(&(buffer[c]), filter_size);
      }
    }
   }
  }
  delete buffer;
  return(1);
}

//-------------------------------//
//  Kprs::Interpolate        //
//-------------------------------//
float Kprs::Interpolate(
			int beam_number, 
			int num_slices_in_comp, 
                        int start_slice_rel_idx,
			float azimuth){
  float azi, kpr;
  int az1, az2;

  if(azimuth< -(2*M_PI)){
    fprintf(stderr,"Error Kprs::Interpolate: Azimuth < -2PI rads\n");
    exit(1);
  }
  azi=azimuth*_numAzimuths/(2*M_PI);
  if(azi<0) azi+=2*_numAzimuths;
  az1=(int)azi;
  az1%=_numAzimuths;
  az2=az1+1;

  //=============================================//
  // Get Absolute Slice Index                    //
  //=============================================//

  int start_slice_abs_idx;
  if(!rel_to_abs_idx(start_slice_rel_idx,_numSlices,&start_slice_abs_idx)){
    fprintf(stderr,"Error Kprs::Interpolate: rel_to_abs_idx failed.\n");
    exit(1);
  }  

  if(start_slice_abs_idx < 0 || start_slice_abs_idx >= _numSlices){
    fprintf(stderr,"Error Kprs::Interpolate: Bad slice number\n");
    exit(1);
  }

  float w=azi-(float)az1;

  /******* In case of wraparound ****/
  if(az2== _numAzimuths) az2=0;

  kpr=(1-w)*_value[beam_number][num_slices_in_comp-1][start_slice_abs_idx][az1]
         + w*_value[beam_number][num_slices_in_comp-1][start_slice_abs_idx][az2];
  return(kpr);
}

//-------------------------------//
//  Kprs::Normalize          //
//-------------------------------//   
int Kprs::Normalize(){
  for(int b=0;b<_numBeams;b++){
    for(int n=0;n<_numSlices;n++){
      for(int s=0;s<_numSlices;s++){
	for(int a=0;a<_numAzimuths;a++){
	  if(_numSamples[b][n][s][a] < _minNumSamples){
	    fprintf(stderr,"Too few samples per bin.\n");
	    return(0);
	  }
	  else{
	    _value[b][n][s][a]/=_numSamples[b][n][s][a];
	    _value[b][n][s][a]=sqrt(_value[b][n][s][a]);
	  }
	}
      }
    }
  }
  return(1);
}


//----------------------------------------------//
// Checks to see if a Kpr Table is empty or not //
//----------------------------------------------//
int Kprs::Empty(){
  if(_numBeams==0) return(1);
  return(0);
}

//--------------------------------//
// Kprs::Write                //
//--------------------------------//   
int Kprs::Write(const char* filename){
      if (filename == NULL)
      return(0);

      FILE* fp = fopen(filename, "w");
      if (fp == NULL)
        return(0);

      if (! _WriteHeader(fp))
	return(0);

      if (! _WriteTable(fp))
	return(0);

      return(1);							     
}

//--------------------------------//
// Kprs::WriteXmgr                //
//--------------------------------//   
int Kprs::WriteXmgr(const char* filename, int num_slices_per_comp){
      if (filename == NULL)
      return(0);

      FILE* fp = fopen(filename, "w");
      if (fp == NULL)
        return(0);

      for(int a=0;a<_numAzimuths; a++){
	float azi=360.0*(float(a)/(float)_numAzimuths);
	for(int b=0; b<_numBeams; b++){
	  fprintf(fp,"%g ", azi);
	  for(int s=0; s<_numSlices; s++){
	    float db=10*log10(1+_value[b][num_slices_per_comp-1][s][a]);
	    fprintf(fp,"%g ",db);
	  }
	}
	fprintf(fp,"\n");
      }
      return(1);							     
}

//--------------------------------//
// Kprs::Read                 //
//--------------------------------//   
int Kprs::Read(const char* filename){
      if (filename == NULL)
      return(0);

      FILE* fp = fopen(filename, "r");
      if (fp == NULL)
	return(0);
      if (! _ReadHeader(fp))
	return(0);

      if (! _Allocate())
	return(0);
      
      if (! _ReadTable(fp))
	return(0);

      return(1);
}

//----------------------//
// Kprs::_Allocate  //
//----------------------//

int
Kprs::_Allocate()
{
	//------------------------------------//
	// allocate the pointer arrays        //
	//------------------------------------//

	_value = (float****)make_array(sizeof(float), 4, _numBeams,
		_numSlices, _numSlices, _numAzimuths);

	if (_value == NULL)
		return(0);


	_numSamples = (int****)make_array(sizeof(int), 4, _numBeams,
		_numSlices, _numSlices, _numAzimuths);

	if (_numSamples == NULL)
		return(0);

        //-------------------------------------//
        // Initialize values to zero           //
        //-------------------------------------//
	for(int b=0; b<_numBeams; b++){
	  for(int n=0; n<_numSlices; n++){
	    for(int s=0; s<_numSlices; s++){
	      for(int a=0;a<_numAzimuths; a++){
		_numSamples[b][n][s][a]=0;
		_value[b][n][s][a]=0.0;
	      }
	    }
	  }
	}

	return(1);
}


//----------------------------------//
// Kprs::_ReadHeader           //
//----------------------------------//

int 
Kprs::_ReadHeader(FILE* ifp)
{
  if (fread((void*)&_numBeams, sizeof(int),1,ifp)!=1) return(0); 
  if (fread((void*)&_numScienceSlices, sizeof(int),1,ifp)!=1) return(0); 
  if (fread((void*)&_numGuardSlicesEachSide, sizeof(int),1,ifp)!=1) return(0); 
  if (fread((void*)&_scienceBandwidth, sizeof(float),1,ifp)!=1) return(0); 
  if (fread((void*)&_guardBandwidth, sizeof(float),1,ifp)!=1) return(0); 
  if (fread((void*)&_numAzimuths, sizeof(int),1,ifp)!=1) return(0);

  _numSlices=2*_numGuardSlicesEachSide + _numScienceSlices;
  return(1);     
}

//----------------------------------//
// Kprs::_ReadTable                 //
//----------------------------------//

int 
Kprs::_ReadTable(FILE* ifp)
{
  for(int b=0;b<_numBeams;b++){
    for(int n=0;n<_numSlices;n++){
      for(int s=0;s<_numSlices;s++){
	if(fread((void*)&_value[b][n][s][0], sizeof(float), _numAzimuths,ifp)
	   != (unsigned int)_numAzimuths) return(0);
      }
    }
  }
  return(1);
}
//----------------------------------//
// Kprs::_WriteHeader           //
//----------------------------------//

int 
Kprs::_WriteHeader(FILE* ofp)
{
  if (fwrite((void*)&_numBeams, sizeof(int),1,ofp)!=1) return(0); 
  if (fwrite((void*)&_numScienceSlices, sizeof(int),1,ofp)!=1) return(0); 
  if (fwrite((void*)&_numGuardSlicesEachSide, sizeof(int),1,ofp)!=1) 
    return(0);  
  if (fwrite((void*)&_scienceBandwidth, sizeof(float),1,ofp)!=1) return(0); 
  if (fwrite((void*)&_guardBandwidth, sizeof(float),1,ofp)!=1) return(0); 
  if (fwrite((void*)&_numAzimuths, sizeof(int),1,ofp)!=1) return(0);  
  return(1);   
}

//----------------------------------//
// Kprs::_WriteTable           //
//----------------------------------//

int 
Kprs::_WriteTable(FILE* ofp)
{
  for(int b=0;b<_numBeams;b++){
    for(int n=0;n<_numSlices;n++){
      for(int s=0;s<_numSlices;s++){
	if(fwrite((void*)&_value[b][n][s][0], sizeof(float), _numAzimuths,ofp)
	   != (unsigned int)_numAzimuths) return(0);
      }
    }
  }
  return(1);
}



