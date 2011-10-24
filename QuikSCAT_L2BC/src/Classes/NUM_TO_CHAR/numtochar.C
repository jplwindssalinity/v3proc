#include "numtochar.h"

const char* itoa(int i, const char* format){
  char* s = new char[256];
  sprintf(s,format,i);
  return s;
}

const char* stoa(short i, const char* format){
  char* s = new char[256];
  sprintf(s,format,i);
  return s;
}

const char* btoa(unsigned char i, const char* format){
  char* s = new char[256];
  sprintf(s,format,i);
  return s;
}

const char* ftoa(float x, const char* format){
  char* s = new char[256];
  sprintf(s,format,x);
  return s;
}

const char* dtoa(double x, const char* format){
  char* s = new char[256];
  sprintf(s,format,x);
  return s;
}
