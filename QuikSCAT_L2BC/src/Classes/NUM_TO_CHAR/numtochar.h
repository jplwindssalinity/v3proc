/*! 
  \file numtochar.h
  \author Ernesto Rodriguez

  Turn numbers of different types into chars. The naming convention
  is the opposite from the one used by stdlib (e.g., itoa, ftoa, etc.).
*/

#ifndef _ER_NUMTOCHAR_H_
#define _ER_NUMTOCHAR_H_

#include <stdio.h>

//! 32 bit integer to char

const char* itoa(int i, const char* format = "%d");

//! 16 bit integer (short) to char

const char* stoa(short i, const char* format = "%d");

//! 8 bit unsigned integer (byte) to char

const char* btoa(unsigned char i, const char* format = "%d");

//! 32 bit float to char

const char* ftoa(float x, const char* format = "%f");

//! 64 bit float (double) to char

const char* dtoa(double x, const char* format = "%f");

#endif
