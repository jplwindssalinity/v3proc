//==============================================================//
// Copyright (C) 2001, California Institute of Technology.      //
// U.S. Government sponsorship acknowledged.                    //
// Author:  Bryan Stiles  Bryan.W.Stiles@jpl.nasa.gov           //
//==============================================================//

#ifndef KP_STATISTICS_H
#define KP_STATISTICS_H

#include "Index.h"
#include "GMF.h"
#include "Wind.h"
#include "WindGrad.h"
#include "Meas.h"


static const char rcs_id_kp_statistics_h[] =
    "@(#) $Id$";
//======================================================================
// CLASSES
//    KpStatistics
//======================================================================

//======================================================================
// CLASS
//    KpStatistics
//
// DESCRIPTION
//    An object that estimates Kp from SeaWinds L2A and L2B data and keeps 
//    track of intermediate statistics.
//======================================================================

class KpStatistics
{
public:
  //--------------//
  // construction //
  // and          //
  // destruction  //
  //--------------//

  KpStatistics();
  int Allocate();
  ~KpStatistics();
  int Deallocate();


  //--------------//
  // I/O          //
  //--------------//

  int Read(const char* filename);
  int Write(const char* filename);
  int WriteAscii(const char* filename);
  int WriteXmgr(const char* filename);

  //--------------//
  // updating     //
  // statistics   //
  //--------------//

  int Update(int ati, int cti, MeasList* meas_list, WVC* wvc, WGC* wgc,
	     GMF* gmf);
  int ComputeKp(int beam, int iat, int ict, int ilook, int ispd, int ichi);

  //-----------//
  // variables //
  //-----------//

  // indices
  Index beamIdx;
  Index aTIdx;
  Index cTIdx;
  Index scanAngleIdx;
  Index spdIdx;
  Index chiIdx;


  // tables
  int****** numMeas;
  int****** numLook;
  float****** sumVpcTheor;
  float****** sumVpiEst;
  float****** sumVpiWGEst;
  float****** sumVpEst;

 protected:

  float _kp;
  float _kpi;
  float _kpiWG;
  float _kpr;
  float _kpm;
  float _kpc; 
  float _kpWG;
  int _n;
  int _nl;
};

#endif





