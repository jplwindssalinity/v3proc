#ifndef GEOMNOISEFILE_H
#define GEOMNOISEFILE_H
#define MAX_NUM_LOOKS 16
#include<stdio.h>

class GeomNoiseFile{
 public:
  GeomNoiseFile(const char* in_file, int need_all_looks=0);
  void ReadHeader();
  int ReadLine(); // skips lines with no measurements
  void FirstPass();
  void Rewind();
  ~GeomNoiseFile();
  // variables
  FILE* ifp;
  const char* infile;
  float min_ctd;
  float max_ctd;
  int ncti;
  float forward_direction;
  float bias[MAX_NUM_LOOKS];
  int nmeas[MAX_NUM_LOOKS];
  float s0ne[MAX_NUM_LOOKS];
  int nlpm[MAX_NUM_LOOKS];
  float azim[MAX_NUM_LOOKS];
  float inc[MAX_NUM_LOOKS];
  char pol[MAX_NUM_LOOKS];
  float sambrat[MAX_NUM_LOOKS];
  int nlooks;
  int nbeams;
  float gridres;
  float ctd;
  int lineno;
  int needAllLooks;
  };

#endif
