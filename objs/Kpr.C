//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
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
	// xxx, true Kpri needs to go here
	*kpri2 = 0.0;
	return(1);
}

//======//
// Kprs //
//======//

//----------------------------//
// Constructors               //
//----------------------------//

Kprs::Kprs()
:  _numBeams(0), _slicesPerSpot(0), _numAzimuths(0),
     _minNumSamples(1),
     _sliceBandwidth(8300.0), _numSamples(NULL), _value(NULL)
{
  return;
}

Kprs::Kprs(int number_of_beams, float slice_bandwidth,
		   int slices_per_spot, 
		   int number_of_azimuth_bins,
		   int min_num_samples
){
  _numBeams=number_of_beams;
  _minNumSamples=min_num_samples;
  _sliceBandwidth=slice_bandwidth;
  _slicesPerSpot=slices_per_spot;
  _numAzimuths=number_of_azimuth_bins;
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

	free_array((void *)_value, 3, _numBeams, _slicesPerSpot, _numAzimuths);
	free_array((void *)_numSamples, 3, _numBeams, _slicesPerSpot, _numAzimuths);

	_value = NULL;
	_numSamples = NULL;
	return;
}

//--------------------------//
// Kprs::Accumulate     //
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
    int beam_number=spot_number%_numBeams;
    int slice_number=0;

    // geocentric to spacecraft velocity //
    Vector3 sc_xv, sc_yv, sc_zv;
    velocity_frame(quiet_spot->scOrbitState.rsat, 
		   quiet_spot->scOrbitState.vsat,
		   &sc_xv, &sc_yv, &sc_zv);
  
    CoordinateSwitch gc_to_scv(sc_xv,sc_yv,sc_zv);


    
    Meas* noisy_slice=noisy_spot->GetHead();
    Meas* quiet_slice=quiet_spot->GetHead();


    

    //----------------------//
    // For each Slice       //
    //----------------------//
    while(noisy_slice && quiet_slice){
      if(slice_number >= _slicesPerSpot){   
	fprintf(stderr,"Kprs::Accumulate-- Too many slices.\n");
	return(0);
      }
      //------------------------------------------------------//
      // Calculate Azimuth (relative to spacecraft velocity   //
      //------------------------------------------------------//

      Vector3 gc_look_vector= quiet_slice->centroid - 
	                    quiet_spot->scOrbitState.rsat;

      Vector3 scv_look_vector=gc_to_scv.Forward(gc_look_vector);
      double r, look, azimuth;
      scv_look_vector.SphericalGet(&r, &look, &azimuth);

      if(!Accumulate(quiet_slice,noisy_slice,beam_number,slice_number,azimuth))
	return(0);
      noisy_slice=noisy_spot->GetNext();
      quiet_slice=quiet_spot->GetNext();
      slice_number++;
    }
    

    if(slice_number < _slicesPerSpot){   
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
Kprs::Accumulate(Meas* quiet, Meas* noisy, int beam_idx, int slice_idx,
		     double azimuth){
  float azi, sqrdif;
  int azi_idx;
  
  //---------------------------------//
  // Get Azimuth Index               //
  //---------------------------------//

  azi=azimuth;
  if(azimuth < -(2*M_PI)){
    fprintf(stderr,"Kprs::Accumulate: Azimuth < -2PI\n");
    return(0);
  }
  azi=azi*_numAzimuths/(2*M_PI);
  if(azi<0) azi+=2*_numAzimuths;
  azi_idx=(int)azi;
  azi_idx%=_numAzimuths;

  //--------------------------------//
  // Increment Number of Samples    //
  //--------------------------------//
  _numSamples[beam_idx][slice_idx][azi_idx]++;

  //---------------------------------//
  // Add square of normalized Sigma0 //
  // difference to _value            //
  //---------------------------------//
  sqrdif=(quiet->value - noisy->value)/(quiet->value);
  sqrdif*=sqrdif;
  _value[beam_idx][slice_idx][azi_idx]+=sqrdif;
  return(1);
}

//**********************************************
// DO NOT USE THIS METHOD WITHOUT INSTRUCTIONS
// FROM BRYAN STILES.
// IT MAKES USE OF DATA WHICH HAS BEEN GENERATED
// IN A VERY SPECIFIC AND NON-STANDARD MANNER
//**********************************************
int
Kprs::Accumulate(L00Frame* quiet , L00Frame* noisy){
  int slice_number=0;

  //----------------Check Error Conditions ---//

  if((quiet->spotsPerFrame != noisy->spotsPerFrame) ||
     (quiet->slicesPerFrame != noisy->slicesPerFrame)){
	fprintf(stderr,"Kprs::Accumulate-- Frame Sizes don't match.\n");
	return(0);
  }
  if(_slicesPerSpot != quiet->slicesPerSpot){   
	fprintf(stderr,"Kprs::Accumulate-- Wrong number of slices.\n");
	return(0);
  }
  
  //--------------------//
  // For each Spot      //
  //--------------------//
  for(int spot_idx=0; spot_idx<quiet->spotsPerFrame; spot_idx++){
    int beam_number=spot_idx%_numBeams;

    // Assume Number of Encoder Bits = 16 for Antenna Position //
    double angres=2*M_PI/pow(2.0,(double)16.0);

    // Calculate Azimuth //
    double azimuth= (double)quiet->antennaPosition[spot_idx]*angres;

    //----------------------//
    // For each Slice       //
    //----------------------//
    for(int slice_idx=0;slice_idx<quiet->slicesPerSpot; slice_idx++){
      Meas quiet_slice, noisy_slice;
      quiet_slice.value=quiet->science[slice_number];
      noisy_slice.value=noisy->science[slice_number];

      if(!Accumulate(&quiet_slice,&noisy_slice,beam_number,slice_idx,azimuth))
	return(0);

      slice_number++;
    }
    
  }
  return(1);
}

int Kprs::Smooth(int filter_size){
  if (filter_size % 2 != 1){
    fprintf(stderr,"Kprs::Smooth: filter_size must be an odd number\n");
    return(0);
  }
  // Allocate buffer for median filtering
  float* buffer=new float(_numAzimuths+filter_size-1);
  for(int b=0;b<_numBeams;b++){
    for(int s=0;s<_slicesPerSpot;s++){


      //-------------------------------------------//
      // copy kpr values to buffer with wraparound //
      //-------------------------------------------//
      for(int c=0; c< filter_size/2; c++){ 
        int a=_numAzimuths-filter_size/2+c;
	buffer[c]=_value[b][s][a];
      }
      for(int c=0; c< _numAzimuths; c++){ 
	buffer[c+filter_size/2]=_value[b][s][c];
      }
      for(int c=0; c< filter_size/2; c++){ 
        int a=_numAzimuths+filter_size/2+c;
	buffer[a]=_value[b][s][c];
      }
      
      //-- Perform Median Filtering in order to Smooth --//
      for(int c=0; c<_numAzimuths; c++){
	_value[b][s][c]=median(&(buffer[c]), filter_size);
      }
    }
  }
  delete buffer;
  return(1);
}
//-------------------------------//
//  Kprs::Interpolate        //
//-------------------------------//
float Kprs::Interpolate(int beam_number, int slice_number, float azimuth){
  float azi, kpr;
  int az1, az2;
  int slice_idx;

  if(azimuth< -(2*M_PI)){
    fprintf(stderr,"Error Kprs::Interpolate: Azimuth < -2PI rads\n");
    exit(1);
  }
  azi=azimuth*_numAzimuths/(2*M_PI);
  if(azi<0) azi+=2*_numAzimuths;
  az1=(int)azi;
  az1%=_numAzimuths;
  az2=az1+1;
  
  slice_idx=-1;
  //--------------------------------------------------------------//
  // Numbering For odd number of slices: -n... -2 -1 0 1 2 ...n   //
  // (Negative Center BasebandFreq ==> Negative slice number      //
  //--------------------------------------------------------------//
  if(_slicesPerSpot%2==1) slice_idx=_slicesPerSpot/2 + slice_number;

  //--------------------------------------------------------------//
  // Numbering For even  number of slices: -n... -2 -1 1 2 ...n   //
  //--------------------------------------------------------------//
  else if(slice_number<0) slice_idx=_slicesPerSpot/2 + slice_number;
  else if(slice_number>0) slice_idx=_slicesPerSpot/2 +slice_number -1;

  if(slice_idx < 0 || slice_idx >= _slicesPerSpot){
    fprintf(stderr,"Error Kprs::Interpolate: Bad slice number\n");
    exit(1);
  }

  float w=azi-(float)az1;

  /******* In case of wraparound ****/
  if(az2== _numAzimuths) az2=0;

  kpr=(1-w)*_value[beam_number][slice_idx][az1]
         + w*_value[beam_number][slice_idx][az2];
  return(kpr);
}

//-------------------------------//
//  Kprs::Normalize          //
//-------------------------------//   
int Kprs::Normalize(){
  for(int b=0;b<_numBeams;b++){
    for(int s=0;s<_slicesPerSpot;s++){
      for(int a=0;a<_numAzimuths;a++){
	if(_numSamples[b][s][a] < _minNumSamples){
	  fprintf(stderr,"Too few samples per bin.\n");
	  return(0);
	}
	else{
	  _value[b][s][a]/=_numSamples[b][s][a];
	  _value[b][s][a]=sqrt(_value[b][s][a]);
	}
      }
    }
  }
  return(1);
}

//--------------------------------//
//  Kprs::NormalizeFrom3Sigma //
//--------------------------------//   
int Kprs::NormalizeFrom3Sigma(){
  for(int b=0;b<_numBeams;b++){
    for(int s=0;s<_slicesPerSpot;s++){
      for(int a=0;a<_numAzimuths;a++){
	if(_numSamples[b][s][a] < _minNumSamples){
	  fprintf(stderr,"Too few samples per bin.\n");
	  return(0);
	}
	else{
	  _value[b][s][a]/=_numSamples[b][s][a];
	  _value[b][s][a]=sqrt(_value[b][s][a]);
          _value[b][s][a]/=3.0;
	}
      }
    }
  }
  return(1);
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
int Kprs::WriteXmgr(const char* filename){
      if (filename == NULL)
      return(0);

      FILE* fp = fopen(filename, "w");
      if (fp == NULL)
        return(0);

      for(int a=0;a<_numAzimuths; a++){
	float azi=360.0*(float(a)/(float)_numAzimuths);
	for(int b=0; b<_numBeams; b++){
	  fprintf(fp,"%g ", azi);
	  for(int s=0; s<_slicesPerSpot; s++){
	    float db=10*log10(1+_value[b][s][a]);
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

	_value = (float***)make_array(sizeof(float), 3, _numBeams,
		_slicesPerSpot, _numAzimuths);

	if (_value == NULL)
		return(0);


	_numSamples = (int***)make_array(sizeof(int), 3, _numBeams,
		_slicesPerSpot, _numAzimuths);

	if (_numSamples == NULL)
		return(0);

        //-------------------------------------//
        // Initialize values to zero           //
        //-------------------------------------//
	for(int b=0; b<_numBeams; b++){
	  for(int s=0; s<_slicesPerSpot; s++){
	    for(int a=0;a<_numAzimuths; a++){
	      _numSamples[b][s][a]=0;
	      _value[b][s][a]=0.0;
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
  if (fwrite((void*)&_slicesPerSpot, sizeof(int),1,ofp)!=1) return(0); 
  if (fwrite((void*)&_sliceBandwidth, sizeof(float),1,ofp)!=1) return(0); 
  if (fwrite((void*)&_numAzimuths, sizeof(int),1,ofp)!=1) return(0);  
  return(1);   
}

//----------------------------------//
// Kprs::_ReadHeader           //
//----------------------------------//

int 
Kprs::_ReadHeader(FILE* ifp)
{
  if (fread((void*)&_numBeams, sizeof(int),1,ifp)!=1) return(0); 
  if (fread((void*)&_slicesPerSpot, sizeof(int),1,ifp)!=1) return(0); 
  if (fread((void*)&_sliceBandwidth, sizeof(float),1,ifp)!=1) return(0); 
  if (fread((void*)&_numAzimuths, sizeof(int),1,ifp)!=1) return(0);
  return(1);     
}

//----------------------------------//
// Kprs::_WriteTable           //
//----------------------------------//

int 
Kprs::_WriteTable(FILE* ofp)
{
  for(int b=0;b<_numBeams;b++){
    for(int s=0;s<_slicesPerSpot;s++){
      if(fwrite((void*)&_value[b][s][0], sizeof(float), _numAzimuths,ofp)
	 != (unsigned int)_numAzimuths) return(0);
    }
  }
  return(1);
}

//----------------------------------//
// Kprs::_ReadTable           //
//----------------------------------//

int 
Kprs::_ReadTable(FILE* ifp)
{
  for(int b=0;b<_numBeams;b++){
    for(int s=0;s<_slicesPerSpot;s++){
      if(fread((void*)&_value[b][s][0], sizeof(float), _numAzimuths,ifp)
	 != (unsigned int)_numAzimuths) return(0);
    }
  }
  return(1);
}
