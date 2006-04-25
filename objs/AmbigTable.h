#ifndef AMBIG_TABLE_H
#define AMBIG_TABLE_H

class AmbigTable{

 public:
  AmbigTable();
  ~AmbigTable();
  int Read(char* index_filename, char* table_filename);


  //supporting functions
  double GetAmbRat1(const unsigned int& beam_number, 
		    const double& azimuth_angle,
		    const double& alongtrack_wrt_boresight,
		    const double& crosstrack_wrt_boresight,
		    double& amb_along_location,
		    double& amb_cross_location);
		    

  double GetAmbRat2(const unsigned int& beam_number, 
		    const double& azimuth_angle,
		    const double& alongtrack_wrt_boresight,
		    const double& crosstrack_wrt_boresight,
		    double& amb_along_location,
		    double& amb_cross_location);
  
  int IsNadirAmbiguous( unsigned int& beam_number, 
		       const double& azimuth_angle,
		       const double& alongtrack_wrt_boresight,
		       const double& crosstrack_wrt_boresight);
  




  float*** alongtrack;
  float*** crosstrack;
 


  unsigned int* beam_number;

  float**** amb_ratio;
  float**** amb_first_power;
  float**** amb_along1;
  float**** amb_cross1;
  float**** amb_second_power;
  float**** amb_along2;
  float**** amb_cross2;
  int****   amb_nadir;
 

 private:
  const static unsigned int Nalong_;
  const static unsigned int Ncross_;
  const static unsigned int Azimuth_step_;
  const static unsigned int Nbeam_;
  float time_in_s_;
  float x_,y_,z_;
  float vx_,vy_,vz_;
  float azi_in_deg_;
  unsigned int Nazi_;
  bool read_table_;
 
  
};



#endif
