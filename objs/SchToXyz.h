//----------------
//SchToXyz.h
// supports coordinate transformation between (xyz) and (sch)
//-------------------
#ifndef SchToXyz_H
#define SchToXyz_H
#include "Matrix3.h"
#include "Utils.h"

class SchToXyz
{
  
 public:
  SchToXyz();
  SchToXyz(const double& r_a, const double& r_e2);


  //-----------------
  //set peg
  //-----------------
  void SetPegPoint(const double& r_lat, const double& r_lon, const double& r_heading);

  //--------------
  //xyz to sch
  //---------------
  void xyz_to_sch(const Vector3& r_xyz, Vector3& r_sch);
  void sch_to_xyz(const Vector3& r_sch, Vector3& r_xyz);


 private:
  bool target_set_;
  double r_radcur_;
  double r_a_,r_e2_;
  double r_mat_[3][3];
  double r_mat_inv_[3][3];
  double r_ov_[3];

};


#endif
