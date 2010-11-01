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


//-------//
// QSKpr //
//-------// 

QSKpr::QSKpr() 
: _instr_kpr(0.07151931), _gatewidth(3), _att_kpr_table(NULL)
{
  return;
}

QSKpr::~QSKpr() {
  if( _att_kpr_table )
    _Deallocate();
  _att_kpr_table = NULL;
  return;
}

int QSKpr::SetGatewidth( int gatewidth ) {
  if( gatewidth < 0 || gatewidth > 7 )
    return(0);
  _gatewidth = gatewidth;
  return(1);
}

int QSKpr::SetInstKPR( float inst_kpr ) {
  if( inst_kpr < 0 )
    return(0);
  _instr_kpr = inst_kpr;
  return(1);
}

int QSKpr::_Allocate() {
  _att_kpr_table = (float*)calloc(sizeof(float),8*50*2*10*10);
  if( _att_kpr_table == NULL )
    return(0);
  return(1);
}

int QSKpr::_Deallocate() {
  free( _att_kpr_table );
  return(1);
}

int QSKpr::Read( const char* filename ) {
  char  buffer[8*50*2*10*10*4];
  char  buffer_swapped[8*50*2*10*10*4];
  char  num_bytes_be[4], num_bytes_le[4];
  int   num_bytes;  
  
  if( _att_kpr_table == NULL )
    _Allocate();
  
  FILE* fid = fopen( filename, "r" );
  if( fid == NULL ) {
    fprintf(stderr,"In QSKpr::Read: Error opening file %s!\n",filename);
    return(0);
  }
  
  // Read record size from file
  fread( &num_bytes_be, sizeof(char), 4 , fid );
  
  // Byte-swap record size
  for( int ii = 0; ii < 4; ++ii ) 
    num_bytes_le[ii] = num_bytes_be[3-ii];

  memcpy( &num_bytes, &num_bytes_le[0], 4 );

  // Compare to expected size of record; Quit if not what expected.
  if( num_bytes != 8*50*2*10*10*4 ) {
    fprintf(stderr,"In read_kpr_table: Error unexpected size of KPR table: %d\n",num_bytes);
    fclose(fid);
    exit(1);
  }
  
  // Read KPR table
  fread( &buffer[0], sizeof(char), 8*50*2*10*10*4, fid );
  fclose( fid );
  
  // Byte-swap KPR table.
  for ( int ii = 0; ii < 8*50*2*10*10; ++ii )
    for( int jj = 0; jj < 4; ++jj )
      buffer_swapped[4*ii+jj] = buffer[4*ii+3-jj];

  // Copy byte-swapped table into output pointer.    
  memcpy( _att_kpr_table, &buffer_swapped[0], sizeof(char)*8*50*2*10*10*4 );

  //for (int ii = 0; ii < 10; ++ii ) {
  //  printf( "kpr_table[%6d]: %g\n", ii, *(kpr_table+ii) );
  //}
  return(1);
}

int QSKpr::GetKpr( int   start_slice, // in range 0 - 9
                   int   num_slices,  // in range 1 - 8
                   int   i_beam,      // 0 (HH), 1 (VV)
                   float azimuth,     // in radians
                   float *value ) {   // return KPR (attitude + instrument)

  azimuth *= rtd; // convert to degrees
  
  // wrap once to interval [0,360]
  if( azimuth < 0    ) azimuth += 360;
  if( azimuth >= 360 ) azimuth -= 360;
  
  // compute interpolation indcies
  int i_azi_low  = floor( azimuth / 7.2 );
  int i_azi_high = i_azi_low + 1;
  if( i_azi_high == 50 ) i_azi_high = 0;
  
  int i_n_slices = num_slices - 1; // index for num_slices dimension.
  
  // Check bounds of indicies into KPR table
  if( _gatewidth  < 0  || _gatewidth  > 7  ||
      i_azi_low   < 0  || i_azi_high  > 49 ||
      i_beam      < 0  || i_beam      > 1  ||
      i_n_slices  < 0  || i_n_slices  > 7  || // only best 8 in L1B, so can't have more
      start_slice < 0  || start_slice > 9  ) {
    fprintf(stderr, "QSKpr::GetKpr: Error: inputs out of range in get_kpr: %d %d %d %f %d\n",
                    start_slice, i_n_slices, i_beam, azimuth, _gatewidth );
    *value = 0.0;
    return(0);
  }
  
  // Linear interpolation in azimuth dimension of table.
  int ii_0 = _gatewidth  * 10 * 10 * 2 * 50 +
              i_azi_low  * 10 * 10 * 2      +
              i_beam     * 10 * 10          +
              i_n_slices * 10               +
              start_slice;

  int ii_1 = _gatewidth  * 10 * 10 * 2 * 50 +
              i_azi_high * 10 * 10 * 2      +
              i_beam     * 10 * 10          +
              i_n_slices * 10               +
              start_slice;
  
  double a_1 = (azimuth-7.2*i_azi_low)/7.2;
  
  if( a_1 < 0 || a_1 > 1 ) {
    fprintf(stderr,"In QSKpr::GetKpr, Error, interpolation coef out of range.\n");
    exit(1);
  }
  
  float att_kpr =  ( 1 - a_1 ) * _att_kpr_table[ii_0] 
                +        a_1   * _att_kpr_table[ii_1];
  
  *value = sqrt( pow(att_kpr,2) + pow(_instr_kpr,2) );
  return(1);
}


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
   _scienceBandwidth(0.0), _numSamples(NULL), _value(NULL)
{
  return;
}

Kprs::Kprs(int num_beams, float science_bandwidth,
	   int num_science_slices, 
           int num_guard_slices_each_side,
	   int number_of_azimuth_bins,
	   int min_num_samples
){
  _numBeams=num_beams;
  _minNumSamples=min_num_samples;
  _scienceBandwidth=science_bandwidth;
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

	free_array((void *)_value, 4, _numBeams, _numScienceSlices,
		   _numScienceSlices, _numAzimuths);
	free_array((void *)_numSamples, 4, _numBeams, _numScienceSlices,
		   _numScienceSlices, _numAzimuths);

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


    //----------------------//
    // For each Slice       //
    //----------------------//

    int shift=GetSliceShift(quiet_spot); 
    int first_slice_no=_numGuardSlicesEachSide+shift;
    int last_slice_no=MIN((first_slice_no+_numScienceSlices-1),(_numSlices-_numGuardSlicesEachSide));
    first_slice_no=MAX(_numGuardSlicesEachSide,first_slice_no);
    
    for(int slice_number=first_slice_no;slice_number<=last_slice_no;slice_number++){


      for(int n=1; n<= _numScienceSlices - slice_number; n++){
            Meas comp_quiet, comp_noise;

            
            quiet_spot->GetByIndex(slice_number);
            noisy_spot->GetByIndex(slice_number);

            if(!comp_quiet.Composite(quiet_spot,n)) return(0);
            if(!comp_noise.Composite(noisy_spot,n)) return(0);

	    int startidx=slice_number-_numGuardSlicesEachSide-shift;
            if(!Accumulate(&comp_quiet,&comp_noise,startidx))  return(0);
      }
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
Kprs::Accumulate(Meas* quiet, Meas* noisy, int start_slice_idx){
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

  int num_slices_per_comp = (int)(quiet->numSlices);

  //-------------------------------------//
  // Increments number of samples in bin //
  //-------------------------------------//

  _numSamples[beam_idx][num_slices_per_comp-1][start_slice_idx][azi_idx]++;

  //---------------------------------//
  // Add square of normalized Sigma0 //
  // difference to _value            //
  //---------------------------------//
  sqrdif=(quiet->value - noisy->value)/(quiet->value);
  sqrdif*=sqrdif;
  _value[beam_idx][num_slices_per_comp-1][start_slice_idx][azi_idx]+=
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
   for(int n=0;n<_numScienceSlices;n++){
    for(int s=0;s<_numScienceSlices;s++){
 

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

  if(start_slice_abs_idx < 0 || start_slice_abs_idx >= _numScienceSlices){
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
    for(int n=0;n<_numScienceSlices;n++){
      for(int s=0;s<_numScienceSlices-n;s++){
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
	  for(int s=0; s<_numScienceSlices; s++){
	    float db=10*log10(1+_value[b][num_slices_per_comp-1][s][a]);
            // int num=_numSamples[b][num_slices_per_comp-1][s][a];
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
		_numScienceSlices, _numScienceSlices, _numAzimuths);

	if (_value == NULL)
		return(0);


	_numSamples = (int****)make_array(sizeof(int), 4, _numBeams,
		_numScienceSlices, _numScienceSlices, _numAzimuths);

	if (_numSamples == NULL)
		return(0);

        //-------------------------------------//
        // Initialize values to zero           //
        //-------------------------------------//
	for(int b=0; b<_numBeams; b++){
	  for(int n=0; n<_numScienceSlices; n++){
	    for(int s=0; s<_numScienceSlices; s++){
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
  if (fread((void*)&_numAzimuths, sizeof(int),1,ifp)!=1) return(0);

  _numScienceSlices=2*_numGuardSlicesEachSide + _numScienceSlices;
  return(1);     
}

//----------------------------------//
// Kprs::_ReadTable                 //
//----------------------------------//

int 
Kprs::_ReadTable(FILE* ifp)
{
  for(int b=0;b<_numBeams;b++){
    for(int n=0;n<_numScienceSlices;n++){
      for(int s=0;s<_numScienceSlices;s++){
	if(fread((void*)&_value[b][n][s][0], sizeof(float), _numAzimuths,ifp)
	   != (unsigned int)_numAzimuths) return(0);
      }
    }
  }

  /****** Code to read number of samples for use in debugging.
  for(int b=0;b<_numBeams;b++){
    for(int n=0;n<_numScienceSlices;n++){
      for(int s=0;s<_numScienceSlices;s++){
	if(fread((void*)&_numSamples[b][n][s][0], sizeof(int), 
		 _numAzimuths,ifp)
	   != (unsigned int)_numAzimuths) return(0);
      }
    }
  }
  ******/
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
    for(int n=0;n<_numScienceSlices;n++){
      for(int s=0;s<_numScienceSlices;s++){
	if(fwrite((void*)&_value[b][n][s][0], sizeof(float), _numAzimuths,ofp)
	   != (unsigned int)_numAzimuths) return(0);
      }
    }
  }

  /****** Code to write number of samples for use in debugging.
  for(int b=0;b<_numBeams;b++){
    for(int n=0;n<_numScienceSlices;n++){
      for(int s=0;s<_numScienceSlices;s++){
	if(fwrite((void*)&_numSamples[b][n][s][0], sizeof(int), 
		  _numAzimuths,ofp)
	   != (unsigned int)_numAzimuths) return(0);
      }
    }
  }
  ****/
  return(1);
}

int Kprs::GetSliceShift(MeasSpot* spot){
  int start_slice=_numGuardSlicesEachSide;
  int end_slice=start_slice+_numScienceSlices-1;

  Meas* slice=spot->GetByIndex(start_slice);
  float X1=slice->XK;

  slice=spot->GetByIndex(start_slice+1);
  float X2=slice->XK;

  slice=spot->GetByIndex(end_slice-1);
  float X3=slice->XK;

  slice=spot->GetByIndex(end_slice);
  float X4=slice->XK;

  int offset=0;
  if(X1>X3) offset=-1;
  if(X4>X2) offset=1;
  return(offset);
}

