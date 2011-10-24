#include "WindStress.h"

using namespace blitz;

const float WindStress::rho_air = 1.223; // kg/m^3

float
WindStress::pseudo_drag(float speed) {
  return this->large_pond_drag(8.);
}

Array<float, 2>
WindStress::pseudo_drag(Array<float, 2>& speed) {
  Array<float, 2> Cd(speed.shape());
  Cd = this->large_pond_drag(8.);
  return Cd;
}

float
WindStress::large_pond_drag(float speed) {
  return speed <= 10. ? 1.14e-3 : (0.49e-3 + 0.065e-3*speed);
}

Array<float, 2>
WindStress::large_pond_drag(Array<float,2>& speed) {
  Array<float, 2> Cd(speed.shape());
  Cd = where(speed <= 10.,1.14e-3,(0.49e-3 + 0.065e-3*speed));
  return Cd;
}

float
WindStress::yelland_taylor_drag(float speed) {

  if( speed < 3. ) {
    return 0.29e-3 + 3.1e-3/3. + 7.7e-3/9.; //  wind stress at 3 m/s
  }
  if( speed > 26. ) {
    return 0.50e-3 + 0.071e-3*26.; // wind stress at 26 m/s
  }

  float Cd = speed <= 6.?
    0.29e-3 + (3.1e-3 + 7.7e-3/speed)/speed:
    0.50e-3 + 0.071e-3*speed;

  return Cd;
}

Array<float, 2>
WindStress::yelland_taylor_drag(Array<float,2>& speed) {

  Array<float, 2> Cd(speed.shape());

  Cd = where(speed <= 6.,
	     0.29e-3 + (3.1e-3 + 7.7e-3/speed)/speed,
	     0.50e-3 + 0.071e-3*speed);

  // Fix the values at the endpoints

  float Cd_3 = 0.29e-3 + 3.1e-3/3. + 7.7e-3/9.; // wind stress at 3 m/s
  float Cd_26 = 0.50e-3 + 0.071e-3*26.; // wind stress at 26 m/s

  Cd = where(speed < 3., Cd_3, Cd);
  Cd = where(speed > 26., Cd_26, Cd);

  return Cd;
}

float
WindStress::operator() (float speed) {

  float Cd;

  if( model == LARGE_POND ) {
    Cd = large_pond_drag(speed);
  }
  else if( model == YELLAND_TAYLOR ) {
    Cd = yelland_taylor_drag(speed);
  }
  else if ( model == PSEUDO ) {
    Cd = pseudo_drag(speed);
  }
  else {
    throw Exception("Don't know how to calculate drag coeficientfor this model",
		    "WindStress::operator() (float speed)");
  }

  float tau = rho_air*Cd*speed;

  return tau;
}


Array<float, 2> 
WindStress::operator() (Array<float, 2>& speed) {

  Array<float,2> Cd(speed.shape());

  if( model == LARGE_POND ) {
    Cd = large_pond_drag(speed);
  }
  else if( model == YELLAND_TAYLOR ) {
    Cd = yelland_taylor_drag(speed);
  }
  else if ( model == PSEUDO ) {
    Cd = pseudo_drag(speed);
  }
  else {
    throw Exception("Don't know how to calculate drag coeficientfor this model",
		    "WindStress::operator() (Array<float, 2>& speed)");
  }

  Array<float,2> tau(speed.shape());
  tau = rho_air*Cd*speed;

  return tau;
}
