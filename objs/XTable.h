//==========================================================//
// Copyright (C) 1997, California Institute of Technology.  //
// U.S. Government sponsorship acknowledged.		    //
//==========================================================//

#ifndef XTable_H
#define XTable_H

#include<stdio.h>
static const char rcs_id_XTable_h[] =
	"@(#) $Id$";


//=====================================================
// CLASSES                     
//             XTable
//=====================================================

//======================================================================
// CLASS
//		XTable
//
// DESCRIPTION
//              A class for manipulating Azimuth angle and slice_number indexed
//              tables of X and Kfactor values.
//
//		
//======================================================================

class XTable
{
  friend int MakeKfactorTable(XTable* trueX, XTable* estX, XTable* Kfactor);

public:
  
  XTable();
  XTable(int num_beams, int num_azimuths, int num_orbit_positions,
	 int num_science_slices,
	 int num_guard_slices_each_side, float science_bandwidth,
         float guard_bandwidth);
  ~XTable();

  int Allocate();
  int CopyBlank(XTable* copy); // copies header info and Allocates
  int Copy(XTable* copy, int num_azimuth_bins, int num_orbit_positions);
  int CheckEmpty();   // Check to see if there are any empty entries in the
                      // table. Returns 0 if there is an empty entry 
                      // 1 otherwise
  int CheckHeader(int num_beams, int num_science_slices,
	 int num_guard_slices_each_side, float science_bandwidth,
         float guard_bandwidth); // Checks to see if header matches
                                 // parameters.
  int Write();
  int Read();
  
  int SetFilename(const char* fname);

  float RetrieveBySliceNumber(int beam_number, float azimuth_angle, 
			      float orbit_position, int slice_number);
  float RetrieveByRelativeSliceNumber(int beam_number, float azimuth_angle, 
			      float orbit_position, int slice_number);
  float RetrieveBySliceFreq(int beam_number, float azimuth_angle, 
			     float orbit_position, float slice_min_freq,
			     float slice_bandwidth);

  // write an entry to the table
  int AddEntry(float X, int beam_number, float azimuth_angle, 
	       float orbit_position, int slice_number); 

  float GetMinFreq(int slice_number);
  float GetBandwidth(int slice_number);
  int FindSliceNum(float freq);
  int WriteXmgr(const char* filename, float orbit_position);

  
  //--------------------------//
  //  constants               //
  //--------------------------//

  int numBeams;
  int numAzimuthBins;
  int numOrbitPositionBins;
  int numScienceSlices;
  int numGuardSlicesEachSide;
  int numSlices;
  float scienceSliceBandwidth;  // in Hz
  float guardSliceBandwidth;    // in Hz


  //---------------------------//
  // arrays                    //
  //---------------------------//

protected:


  int _Deallocate();
  int _WriteHeader(FILE* ofp);
  int _WriteTable(FILE* ofp);
  int _ReadHeader(FILE* ifp);
  int _ReadTable(FILE* ifp);

  float**** _value;
  int**** _empty;
  char* _filename;
};
#endif







