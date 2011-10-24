/*!
  \file WindStress.h
  \author Ernesto Rodriguez
  \brief Calculate the wind stress given the neutral wind and a drag coefficient choice.
*/

#ifndef _ER_WINDSTRESS_H_
#define _ER_WINDSTRESS_H_

#include <blitz/array.h>

#include "Exception.h"

typedef enum {LARGE_POND,YELLAND_TAYLOR,PSEUDO } WindStressModel;

class WindStress {
public:

  WindStress(WindStressModel wind_stress_model):model(wind_stress_model){}

  //! Return |tau| in Newtons/m^2 (Pascals, or kg m^-1 s^-2) given the *neutral* wind speed in m/s.

  float operator() (float speed);
  blitz::Array<float, 2> operator() (blitz::Array<float, 2>& speed);

  //! Return a constant drag coefficient equal to the large and pond coefficient at 8m/s.

  float pseudo_drag(float speed);
  blitz::Array<float, 2> pseudo_drag(blitz::Array<float, 2>& speed);

  float large_pond_drag(float speed);
  blitz::Array<float, 2> large_pond_drag(blitz::Array<float,2>& speed);

  float yelland_taylor_drag(float speed);
  blitz::Array<float, 2> yelland_taylor_drag(blitz::Array<float,2>& speed);

  static const float rho_air; // kg/m^3
  WindStressModel model;
};

#endif
