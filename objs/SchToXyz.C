//--------
//SchToXyz.C
//---------
#include <iostream>
#include <math.h>
#include "SchToXyz.h"
#include "Utils.h"
using std::cout;
using std::endl;
using std::cerr;



//------------------
//Default constructor
//------------------
 SchToXyz::SchToXyz()
   :target_set_(false)
{


 
}

//-----------------
//Construct with radius and eccentricity
//----------------
SchToXyz::SchToXyz(const double& r_a, const double& r_e2)
  :target_set_(true)
{
  r_a_= r_a;
  r_e2_=r_e2;
}


 //-----------------
  //set peg point
  //-----------------
  void SchToXyz::SetPegPoint(const double& r_lat, const double& r_lon, const double& r_hdg)
{
  if(!target_set_){
    fprintf(stderr, "SchToXyz::target is not set\n");
    exit(1);
  } 
 
  double r_clt = cos(r_lat);
  double r_slt = sin(r_lat);
  double r_clo = cos(r_lon);
  double r_slo = sin(r_lon);
  double r_chg = cos(r_hdg);
  double r_shg = sin(r_hdg);
  
  r_mat_[0][0] = r_clt*r_clo;
  r_mat_[0][1] = -r_shg*r_slo - r_slt*r_clo*r_chg;
  r_mat_[0][2] = r_slo*r_chg - r_slt*r_clo*r_shg;
  
  r_mat_[1][0] = r_clt*r_slo ;
  r_mat_[1][1] = r_clo*r_shg - r_slt*r_slo*r_chg ;
  r_mat_[1][2] = -r_clo*r_chg - r_slt*r_slo*r_shg;

  r_mat_[2][0] = r_slt;
  r_mat_[2][1] = r_clt*r_chg;
  r_mat_[2][2] = r_clt*r_shg;


  for(unsigned int i=0;i<3;++i)
    for(unsigned int j=0;j<3;++j){
      r_mat_inv_[i][j] = r_mat_[j][i];
    }



  /*
  for(unsigned int i=0;i<3;++i){
    cout<<endl;
    for(unsigned int j=0;j<3;++j){
      cout<<r_mat_[i][j]<<" ";
    }
  }

 for(unsigned int i=0;i<3;++i){
    cout<<endl;
    for(unsigned int j=0;j<3;++j){
      cout<<r_mat_inv_[i][j]<<" ";
    }
  }

 
 
 cout<<endl;
  
  */




  //   find the translation vector
  r_radcur_ = radius_along(r_a_, r_e2_,r_hdg,r_lat);

  //cout<<"rad cur "<< r_radcur_<<endl;
  Vector3 r_llh,r_p, r_up;
  r_llh.Set( r_lat, r_lon, 0.0);
  llh_to_xyz(r_a_, r_e2_,r_llh,r_p);
 

  r_clt = cos(r_lat);  
  r_slt = sin(r_lat);
  r_clo = cos(r_lon);
  r_slo = sin(r_lon);
  r_up.Set( r_clt*r_clo, r_clt*r_slo, r_slt);        

  for(unsigned int i=0;i<3;++i){
    r_ov_[i] = r_p(i) - r_radcur_*r_up(i);
  }

  //cout<<"rov "<< r_ov_[0]<<" "<<r_ov_[1]<<" "<<r_ov_[2]<<endl;
  //cout<<"r_up "<< r_up(0)<<" "<<r_up(1)<<" "<<r_up(2)<<endl;
}


 //--------------
  //xyz to sch
  //---------------
void SchToXyz::xyz_to_sch(const Vector3& r_xyz, Vector3& r_sch){
  Vector3 r_scht;
  //cout<<"rxyz "<<r_xyz<<endl;
  r_scht.SetX(r_xyz(0) - r_ov_[0]);
  r_scht.SetY(r_xyz(1) - r_ov_[1]);
  r_scht.SetZ(r_xyz(2) - r_ov_[2]);

  //cout<<"scht "<<r_scht<<endl;
  double  tmp_schv[3];
  for(unsigned int i=0;i<3;++i){
    tmp_schv[i]=0.0;
    for(unsigned int j=0;j<3;++j){
      tmp_schv[i]= tmp_schv[i]+r_mat_inv_[i][j]*r_scht(j);
    }
  }
  Vector3 r_schv(tmp_schv[0],tmp_schv[1],tmp_schv[2]);
  //cout<<"r_schv "<<r_schv<<endl;

  Vector3 r_llh;
  xyz_to_llh(r_radcur_,0.0, r_schv,r_llh);
  //cout<<"r_llh "<< r_llh<<endl;
  r_sch.SetX( r_radcur_*r_llh(1));
  r_sch.SetY( r_radcur_*r_llh(0));
  r_sch.SetZ( r_llh(2));

  //cout<<"r_sch "<< r_sch<<endl;

}



void SchToXyz::sch_to_xyz(const Vector3& r_sch, Vector3& r_xyz){
  Vector3 r_llh;
 
  r_llh.SetX( r_sch(1)/r_radcur_);
  r_llh.SetY( r_sch(0)/r_radcur_);
  r_llh.SetZ( r_sch(2));

  Vector3 r_scht;
  llh_to_xyz(r_radcur_,0.0, r_llh, r_scht);//convert llh to vector
  
  double r_xyzt[3];
  for(unsigned int i=0;i<3;++i){
    r_xyzt[i]=0.0;
    for(unsigned int j=0;j<3;++j){
      r_xyzt[i]= r_xyzt[i]+r_mat_[i][j]*r_scht(j);
    }
  }
  
  r_xyz.SetX( r_xyzt[0] + r_ov_[0]);
  r_xyz.SetY( r_xyzt[1] + r_ov_[1]);
  r_xyz.SetZ( r_xyzt[2] + r_ov_[2]);
          
}
