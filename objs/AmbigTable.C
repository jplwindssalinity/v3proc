#include <stdio.h>
#include <istream>
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
  :read_table_(false)
 
{
  Nazi_= (unsigned int)int( 360/Azimuth_step_+0.1);//Nazi_= 306 degree/Azimuth_step

 //set array size based on Nalong, Ncross_
  alongtrack=(float***) make_array(sizeof(float),3,Nazi_,Nbeam_,Nalong_);
  crosstrack=(float***) make_array(sizeof(float),3,Nazi_,Nbeam_,Ncross_);

 


  amb_ratio=           (float****)make_array(sizeof(float),4,Nazi_,Nbeam_,Nalong_,Ncross_);
  amb_first_power=     (float****)make_array(sizeof(float),4,Nazi_,Nbeam_,Nalong_,Ncross_);
  amb_along1=          (float****)make_array(sizeof(float),4,Nazi_,Nbeam_,Nalong_,Ncross_);
  amb_cross1=          (float****)make_array(sizeof(float),4,Nazi_,Nbeam_,Nalong_,Ncross_);
  amb_second_power=    (float****)make_array(sizeof(float),4,Nazi_,Nbeam_,Nalong_,Ncross_);
  amb_along2=          (float****)make_array(sizeof(float),4,Nazi_,Nbeam_,Nalong_,Ncross_);
  amb_cross2=          (float****)make_array(sizeof(float),4,Nazi_,Nbeam_,Nalong_,Ncross_);
  amb_nadir=               (int****)make_array(sizeof(int),4,Nazi_,Nbeam_,Nalong_,Ncross_);
}


AmbigTable::~AmbigTable()
{
  return;
}


int AmbigTable::Read(char* index_filename, char* table_filename)
{

  //reset flag
  read_table_=false;


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

      //read alongtrack
      for(unsigned int i=0;i<Nalong_;++i)
	if(fread((void*)&alongtrack[i_azi][i_beam][i],sizeof(float),1,index_f)!=1) return(0);

      //read crosstrack
      for(unsigned int i=0;i<Ncross_;++i)
	if(fread((void*)&crosstrack[i_azi][i_beam][i],sizeof(float),1,index_f)!=1) return(0);

      //read total amb ratio 
      for(unsigned int i=0;i<Nalong_;++i)
	for(unsigned int j=0;j<Ncross_;++j)
	  if(fread((void*)&amb_ratio[i_azi][i_beam][i][j],sizeof(float),1,table_f)!=1) return(0);
      
      //first in ambiguous power	
      for(unsigned int i=0;i<Nalong_;++i)
	for(unsigned int j=0;j<Ncross_;++j)
	  if(fread((void*)&amb_first_power[i_azi][i_beam][i][j],sizeof(float),1,table_f)!=1) return(0);
	
      
      //first amb location in alongtrack direction w.r.t. boresight
      for(unsigned int i=0;i<Nalong_;++i)
	for(unsigned int j=0;j<Ncross_;++j)
	  if(fread((void*)&amb_along1[i_azi][i_beam][i][j],sizeof(float),1,table_f)!=1) return(0);
	

      //first amb location in crosstrack direction w.r.t. boresight
      for(unsigned int i=0;i<Nalong_;++i)
	for(unsigned int j=0;j<Ncross_;++j)
	  if(fread((void*)&amb_cross1[i_azi][i_beam][i][j],sizeof(float),1,table_f)!=1) return(0);
      
      //second in amb power
      for(unsigned int i=0;i<Nalong_;++i)
	for(unsigned int j=0;j<Ncross_;++j)
	  if(fread((void*)&amb_second_power[i_azi][i_beam][i][j],sizeof(float),1,table_f)!=1) return(0);

      //second amb location in alongtrack direction w.r.t boresight 
      for(unsigned int i=0;i<Nalong_;++i)
	for(unsigned int j=0;j<Ncross_;++j)
	  if(fread((void*)&amb_along2[i_azi][i_beam][i][j],sizeof(float),1,table_f)!=1) return(0);
	
      
      //second ambiguout point in crosstrack direction w.r.t. boresight
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
  
  read_table_=true;
  return(1);
}


double AmbigTable::GetAmbRat1(const unsigned int& beam_number, 
			      const double& azimuth_angle,
			      const double& alongtrack_input,
			      const double& crosstrack_input ,
			      double& amb_along,
			      double& amb_cross)
{
			     

  double value=pow(10,100/10);//100 dB
  amb_along=0;
  amb_cross=0;
  if(!read_table_) {
    fprintf(stderr, "AmbigTable::GetAmbRat1: beam pattern has not been read in\n");
    exit(1);
  }

  if(beam_number>Nbeam_){
    fprintf(stderr, "AmbigTable::GetAmbRat1: beam number is out of range\n");
    exit(1);
  }

  int i_azi= int( azimuth_angle/Azimuth_step_ + 0.1);
  if(i_azi <0 || i_azi >=int(Nazi_)){
    fprintf(stderr, "AmbigTable::GetAmbRat1: azimuth angle index is out of range\n");
    exit(1);
  }
  

  //compute indices for along/cross
  unsigned int azi_index=(unsigned int) i_azi;
  if(alongtrack_input <alongtrack[azi_index][beam_number][0] 
     || alongtrack_input >=alongtrack[azi_index][beam_number][Nalong_-1]){
    return(0.0);//checking outside process window?,  return 0.0
  }
  if(crosstrack_input<crosstrack[azi_index][beam_number][0] 
     || crosstrack_input>=crosstrack[azi_index][beam_number][Ncross_-1]){
    return(0.0);//checking outside processing window?,  return 0.0
  }
  
  double along_index_r= alongtrack_input - alongtrack[azi_index][beam_number][0];
  along_index_r /=  alongtrack[azi_index][beam_number][1]- alongtrack[azi_index][beam_number][0];
  if(along_index_r <0.0 || along_index_r >=float(Nalong_)){
    fprintf(stderr, "AmbigTable::GetAmbRat1: alongtrack  index is out of range\n");
    exit(1);
  }

  double cross_index_r= crosstrack_input - crosstrack[azi_index][beam_number][0];
  cross_index_r /= crosstrack[azi_index][beam_number][1]-crosstrack[azi_index][beam_number][0];
  if(cross_index_r <0.0 || cross_index_r >float(Ncross_)){
    fprintf(stderr, "AmbigTable::GetAmbRat1: crosstrack  index is out of range\n");
    exit(1);
  }

  unsigned int along_index=(unsigned int) int(along_index_r);
  unsigned int cross_index=(unsigned int) int(cross_index_r);

  value=amb_first_power[azi_index][beam_number][along_index][cross_index];

  //addition location information
  amb_along=amb_along1[azi_index][beam_number][along_index][cross_index];
  amb_cross=amb_cross1[azi_index][beam_number][along_index][cross_index];

  return(value);

}

double AmbigTable::GetAmbRat2(const unsigned int& beam_number, 
			      const double& azimuth_angle,
			      const double& alongtrack_input,
			      const double& crosstrack_input,
			      double& amb_along,
			      double& amb_cross)
			     
{
  double value=pow(10,100/10);//100 dB
  amb_along=0;
  amb_cross=0;
  if(!read_table_) {
    fprintf(stderr, "AmbigTable::GetAmbRat2: beam pattern has not been read in\n");
    exit(1);
  }

  if(beam_number>Nbeam_){
    fprintf(stderr, "AmbigTable::GetAmbRat2: beam number is out of range\n");
    exit(1);
  }

  int i_azi= int( azimuth_angle/Azimuth_step_ + 0.1);
  if(i_azi <0 || i_azi >= int(Nazi_)){
    fprintf(stderr, "AmbigTable::GetAmbRat2: azimuth angle index is out of range\n");
    exit(1);
  }

  //compute indices for along/cross
  unsigned int azi_index=(unsigned int) i_azi;
  if(alongtrack_input <alongtrack[azi_index][beam_number][0] 
     || alongtrack_input >=alongtrack[azi_index][beam_number][Nalong_-1]){
    return(0.0);//checking outside process window?,  return 0.0
  }
  if(crosstrack_input<crosstrack[azi_index][beam_number][0] 
     || crosstrack_input>=crosstrack[azi_index][beam_number][Ncross_-1]){
    return(0.0);//checking outside processing window?,  return 0.0
  }
  
  double along_index_r= alongtrack_input - alongtrack[azi_index][beam_number][0];
  along_index_r /=  alongtrack[azi_index][beam_number][1]- alongtrack[azi_index][beam_number][0];
  if(along_index_r <0.0 || along_index_r >=float(Nalong_)){
    fprintf(stderr, "AmbigTable::GetAmbRat2: alongtrack  index is out of range\n");
    exit(1);
  }
  
  double cross_index_r= crosstrack_input - crosstrack[azi_index][beam_number][0];
  cross_index_r /= crosstrack[azi_index][beam_number][1]-crosstrack[azi_index][beam_number][0];
  if(cross_index_r <0.0 || cross_index_r >float(Ncross_)){
    fprintf(stderr, "AmbigTable::GetAmbRat2: crosstrack  index is out of range\n");
    exit(1);
  }

  unsigned int along_index=(unsigned int) int(along_index_r);
  unsigned int cross_index=(unsigned int) int(cross_index_r);

  value=amb_second_power[azi_index][beam_number][along_index][cross_index];

  //addition location information
  amb_along=amb_along2[azi_index][beam_number][along_index][cross_index];
  amb_cross=amb_cross2[azi_index][beam_number][along_index][cross_index];

  return(value);
}

 int AmbigTable::IsNadirAmbiguous( unsigned int& beam_number, 
				  const double& azimuth_angle,
				  const double& alongtrack_input,
				  const double& crosstrack_input)
{
  int value=1;
 if(!read_table_) {
    fprintf(stderr, "AmbigTable::IsNadirAmbiguous: beam pattern has not been read in\n");
    exit(1);
  }

  if(beam_number>Nbeam_){
    fprintf(stderr, "AmbigTable::IsNadirAmbiguous: beam number is out of range\n");
    exit(1);
  }

  int i_azi= int( azimuth_angle/Azimuth_step_ + 0.1);
  if(i_azi <0 || i_azi >=int(Nazi_)){
    fprintf(stderr, "AmbigTable::IsNadirAmbiguous: azimuth angle index is out of range\n");
    exit(1);
  }
  


  //compute indices for along/cross
  unsigned int azi_index=(unsigned int) i_azi;
  if(alongtrack_input <alongtrack[azi_index][beam_number][0] 
     || alongtrack_input >=alongtrack[azi_index][beam_number][Nalong_-1]){
    return(0);//checking outside process window?,  return 0.0
  }
  if(crosstrack_input<crosstrack[azi_index][beam_number][0] 
     || crosstrack_input>=crosstrack[azi_index][beam_number][Ncross_-1]){
    return(0);//checking outside processing window?,  return 0.0
  }
  
  double along_index_r= alongtrack_input - alongtrack[azi_index][beam_number][0];
  along_index_r /=  alongtrack[azi_index][beam_number][1]- alongtrack[azi_index][beam_number][0];
  if(along_index_r <0.0 || along_index_r >=float(Nalong_)){
    fprintf(stderr, "AmbigTable::IsNadirAmbiguous: alongtrack  index is out of range\n");
    exit(1);
  }
  
  double cross_index_r= crosstrack_input - crosstrack[azi_index][beam_number][0];
  cross_index_r /= crosstrack[azi_index][beam_number][1]-crosstrack[azi_index][beam_number][0];
  if(cross_index_r <0.0 || cross_index_r >float(Ncross_)){
    fprintf(stderr, "AmbigTable::IsNadirAmbiguous: crosstrack  index is out of range\n");
    exit(1);
  }

  unsigned int along_index=(unsigned int) int(along_index_r);
  unsigned int cross_index=(unsigned int) int(cross_index_r);

  value=amb_nadir[azi_index][beam_number][along_index][cross_index];
  return(value);
}
  
