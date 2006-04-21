#include <stdio.h>
#include <math.h>
#include"AmbigTable.h"
#include "Array.h"

//----------------------
//Static constant member
//-----------------------
const  unsigned int AmbigTable::Nalong_=31;
const  unsigned int AmbigTable::Ncross_=31;
const  unsigned int AmbigTable::Azimuth_step_=1;
const  unsigned int AmbigTable::Nbeam_=4;

AmbigTable::AmbigTable()
 
{
  Nazi_= (unsigned int)int( 360/Azimuth_step_+0.1);//Nazi_= 306 degree/Azimuth_step

 //set array size based on Nalong, Ncross_
  alongtrack=(float***) make_array(sizeof(float),2,Nazi_,Nbeam_,Nalong_);
  crosstrack=(float***) make_array(sizeof(float),2,Nazi_,Nbeam_,Ncross_);

 


  amb_ratio=           (float****)make_array(sizeof(float),4,Nazi_,Nbeam_,Nalong_,Ncross_);
  amb_first_power=     (float****)make_array(sizeof(float),4,Nazi_,Nbeam_,Nalong_,Ncross_);
  amb_along1=          (float****)make_array(sizeof(float),4,Nazi_,Nbeam_,Nalong_,Ncross_);
  amb_cross1=          (float****)make_array(sizeof(float),4,Nazi_,Nbeam_,Nalong_,Ncross_);
  amb_second_power=    (float****)make_array(sizeof(float),4,Nazi_,Nbeam_,Nalong_,Ncross_);
  amb_along2=          (float****)make_array(sizeof(float),4,Nazi_,Nbeam_,Nalong_,Ncross_);
  amb_cross2=          (float****)make_array(sizeof(float),4,Nazi_,Nbeam_,Nalong_,Ncross_);
  amb_nadir=             (int****)make_array(sizeof(int),4,Nazi_,Nbeam_,Nalong_,Ncross_);
}


AmbigTable::~AmbigTable()
{
  return;
}


int AmbigTable::Read(char* index_filename, char* table_filename)
{
  FILE* index_f= fopen(index_filename,"rb");
  if(index_f==NULL){
    fprintf(stderr,"AmbigTable: Read : no ambiguity index file\n");
    return(0);
  }

  FILE* table_f= fopen(table_filename,"rb");
  if(table_f==NULL){
    fprintf(stderr,"AmbigTable: Read : no ambiguity table file\n");
    return(0);
  }

 
  //temporay variable
  float x;
  unsigned int beam_number;

  //read index file first: 0 - 359 azimuth angle
  for(unsigned int i_azi=0;i_azi<Nazi_;++i_azi)
    for(unsigned int i_beam=0;i_beam<Nbeam_;i_beam++){

      //time
      if(fread((void*)&time_in_s_,sizeof(float),1,index_f)!=1) return(0);

      //position
      if(fread((void*)&x_,sizeof(float),1,index_f)!=1) return(0);
      if(fread((void*)&y_,sizeof(float),1,index_f)!=1) return(0);
      if(fread((void*)&z_,sizeof(float),1,index_f)!=1) return(0);
      
      //velocity
      if(fread((void*)&vx_,sizeof(float),1,index_f)!=1) return(0);
      if(fread((void*)&vy_,sizeof(float),1,index_f)!=1) return(0);
      if(fread((void*)&vz_,sizeof(float),1,index_f)!=1) return(0);
      
      //azimuth angle in deg
      if(fread((void*)&azi_in_deg_,sizeof(float),1,index_f)!=1) return(0);
      if( fabs(azi_in_deg_ - float(i_azi)*Azimuth_step_) > Azimuth_step_) return(0);

      //beam number
      if(fread((void*)&x,sizeof(float),1,index_f)!=1) return(0);
      beam_number=(unsigned int) int( x+ 0.1);//beam number
      if(beam_number !=i_beam) return(0);

      //dim size check
      if(fread((void*)&x,sizeof(float),1,index_f)!=1) return(0);
      if( (unsigned int) int(x+0.1) != Nalong_) return(0);

      if(fread((void*)&x,sizeof(float),1,index_f)!=1) return(0);
      if( (unsigned int) int(x+0.1) != Ncross_) return(0);

 

      //read amb ratio 
      for(unsigned int i=0;i<Nalong_;++i)
	for(unsigned int j=0;j<Ncross_;++j)
	  if(fread((void*)&amb_ratio[i_azi][i_beam][i][j],sizeof(float),1,table_f)!=1) return(0);
	
      for(unsigned int i=0;i<Nalong_;++i)
	for(unsigned int j=0;j<Ncross_;++j)
	  if(fread((void*)&amb_first_power[i_azi][i_beam][i][j],sizeof(float),1,table_f)!=1) return(0);
	

      //first amb point in along
      for(unsigned int i=0;i<Nalong_;++i)
	for(unsigned int j=0;j<Ncross_;++j)
	  if(fread((void*)&amb_along1[i_azi][i_beam][i][j],sizeof(float),1,table_f)!=1) return(0);
	

      //first amb point in cross
      for(unsigned int i=0;i<Nalong_;++i)
	for(unsigned int j=0;j<Ncross_;++j)
	  if(fread((void*)&amb_cross1[i_azi][i_beam][i][j],sizeof(float),1,table_f)!=1) return(0);
      //second amb power
      for(unsigned int i=0;i<Nalong_;++i)
	for(unsigned int j=0;j<Ncross_;++j)
	  if(fread((void*)&amb_second_power[i_azi][i_beam][i][j],sizeof(float),1,table_f)!=1) return(0);

      //second amb point in along
      for(unsigned int i=0;i<Nalong_;++i)
	for(unsigned int j=0;j<Ncross_;++j)
	  if(fread((void*)&amb_along2[i_azi][i_beam][i][j],sizeof(float),1,table_f)!=1) return(0);
	
      
      //second amb point in cross
      for(unsigned int i=0;i<Nalong_;++i)
	for(unsigned int j=0;j<Ncross_;++j)
	  if(fread((void*)&amb_cross2[i_azi][i_beam][i][j],sizeof(float),1,table_f)!=1) return(0);
	


      //nadir location inside the processing window
      for(unsigned int i=0;i<Nalong_;++i)
	for(unsigned int j=0;j<Ncross_;++j){
	  if(fread((void*)&x,sizeof(float),1,table_f)!=1) return(0);
	  amb_nadir[i_azi][i_beam][i][j]=int(x+0.1);
	}
    }

  return(1);
}
