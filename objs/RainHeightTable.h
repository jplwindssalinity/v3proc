//==============================================================//
// Copyright (C) 1998-1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//
#ifndef RAINHEIGHTTABLE_H
#define RAINHEIGHTTABLE_H

static const char rcs_id_rainheighttable_h[] =
    "@(#) $Id$";

#include"Index.h"

//======================================================================
// CLASS
//    RainHeightTable
//======================================================================

class RainHeightTable
{
 public:
  Index dayIdx;
  Index lonIdx;
  Index latIdx;
  float*** table;
  RainHeightTable();
  ~RainHeightTable();
  int ReadWentz(const char* filename);
  int Read(const char* filename);
  int Write(const char* filename);
  int Write(FILE* ifp);
  int Read(FILE* ifp);
  float Interpolate(float day, float lat, float lon);
};
#endif
