//==============================================================//
// Copyright (C) 2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#ifndef WIND_GRAD_H
#define WIND_GRAD_H

static const char rcs_id_wind_grad_h[] =
    "@(#) $Id$";

#include"Wind.h"
#include"Array.h"

//======================================================================
// CLASSES
//    WGC, WindGrad
//======================================================================

//======================================================================
// CLASS
//    WGC
//
// DESCRIPTION
//    The WGC object represents a wind gradient cell.  It contains a
//    center point and 4 derivatives, 
//======================================================================

class WGC
{
 public:

  //--------------//
  // construction //
  //--------------//

  WGC();
  ~WGC();

 //-----------------------------------//
 // calculate wind vector at position //
 //-----------------------------------//

  int GetWindVector(WindVector* wv, LonLat pos);

  // variables

  LonLat lonLat;
  float ddirdlat;
  float ddirdlon;
  float dspddlat;
  float dspddlon;

};

//======================================================================
// CLASS
//    WindGrad
//
// DESCRIPTION
//    The WindGrad object holds a wind gradient field gridded in
//    along track and cross track.
//======================================================================

class WindGrad{
 public:

  //--------------//
  // construction //
  //--------------//

  WindGrad();
  WindGrad(WindSwath* windswath, int use_dirth=0);
  ~WindGrad();

  //---------//
  // freeing //
  //---------//
   int  DeleteEntireSwath();
   int  DeleteWGCs();

   //---------------------//
   // setting and getting //
   //---------------------//

    WGC*  GetWGC(int cti, int ati);

   //-----------//
   // variables //
   //-----------//

    WGC***  swath;

 protected:
   //---------------------//
   // Compute derivatives //
   //---------------------//

   int _ComputeDerivatives(WindSwath* windswath, int use_dirth);


    //--------------//
    // construction //
    //--------------//

    int  _Allocate();
    int  _Deallocate();

    //-----------//
    // variables //
    //-----------//

    int  _crossTrackBins;
    int  _alongTrackBins;
    int  _validCells;
};

#endif





