//==============================================================//
// Copyright (C) 1997-2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_XTable_c[] =
    "@(#) $Id$";

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "XTable.h"
#include "Constants.h"
#include "Misc.h"
#include "Array.h"
#include "GenericGeom.h"
#include "CoordinateSwitch.h"

//==============//
// CLASS XTable //
//==============//

XTable::XTable()
:   numBeams(0), numAzimuthBins(0), numOrbitPositionBins(0),
    numScienceSlices(0), numGuardSlicesEachSide(0), numSlices(0),
    scienceSliceBandwidth(0.0), guardSliceBandwidth(0.0), _value(NULL),
    _empty(NULL), _filename(NULL)
{
    return;
}

XTable::XTable(int num_beams, int num_azimuths, int num_orbit_positions,
               int num_science_slices, int num_guard_slices_each_side,
               float science_bandwidth, float guard_bandwidth) {
    numBeams = num_beams;
    numAzimuthBins = num_azimuths;
    numOrbitPositionBins = num_orbit_positions;
    numScienceSlices = num_science_slices;
    numGuardSlicesEachSide = num_guard_slices_each_side;
    scienceSliceBandwidth = science_bandwidth;
    guardSliceBandwidth = guard_bandwidth;
    numSlices = numScienceSlices + 2 * numGuardSlicesEachSide;
    if (! Allocate()) {
        fprintf(stderr, "Error allocating XTable object. \n");
        exit(1);
    }
    return;
}

XTable::~XTable() {
    _Deallocate();
    if (_filename != NULL) {
        free( _filename);
    }
    return;
}

//------------------//
// XTable::Allocate //
//------------------//

int XTable::Allocate() {
    if (numBeams <= 0 || numAzimuthBins <= 0 || (numSlices) <= 0 ||
        numOrbitPositionBins <= 0) {
        return(0);
    }

    if (numScienceSlices < 0 || numGuardSlicesEachSide < 0) {
        return(0);
    }

    _value = (float****)make_array(sizeof(float), 4, numBeams, numAzimuthBins,
                                   numOrbitPositionBins, numScienceSlices
                                   + 2 * numGuardSlicesEachSide);

    _empty = (int****)make_array(sizeof(int), 4, numBeams, numAzimuthBins,
                                 numOrbitPositionBins, numSlices);

    if (_value == NULL) return(0);
    if (_empty == NULL) return(0);
    for (int b = 0; b < numBeams; b++) {
        for (int a = 0; a < numAzimuthBins; a++) {
            for (int o = 0; o < numOrbitPositionBins; o++) {
                for (int s = 0; s < numSlices; s++) {
                    _empty[b][a][o][s] = 1;
                }
            }
        }
    }
    return(1);
}

//-------------------//
// XTable::CopyBlank //
//-------------------//

int XTable::CopyBlank(XTable* copy) {
    copy->_Deallocate();
    copy->numBeams = numBeams;
    copy->numAzimuthBins = numAzimuthBins;
    copy->numOrbitPositionBins = numOrbitPositionBins;
    copy->numScienceSlices = numScienceSlices;
    copy->numGuardSlicesEachSide = numGuardSlicesEachSide;
    copy->numSlices = numSlices;
    copy->scienceSliceBandwidth = scienceSliceBandwidth;
    copy->guardSliceBandwidth = guardSliceBandwidth;;
    if (!copy->Allocate()) return(0);
    return(1);
}

//--------------//
// XTable::Copy //
//--------------//

int XTable::Copy(XTable* copy, int num_azimuth_bins, int num_orbit_positions) {
    copy->_Deallocate();
    copy->numBeams = numBeams;
    copy->numAzimuthBins = num_azimuth_bins;
    copy->numOrbitPositionBins = num_orbit_positions;
    copy->numScienceSlices = numScienceSlices;
    copy->numGuardSlicesEachSide = numGuardSlicesEachSide;
    copy->numSlices = numSlices;
    copy->scienceSliceBandwidth = scienceSliceBandwidth;
    copy->guardSliceBandwidth = guardSliceBandwidth;;
    if (!copy->Allocate()) return(0);

    for (int b = 0; b < numBeams; b++) {
        for (int a = 0; a < num_azimuth_bins; a++) {
            for (int o = 0; o < num_orbit_positions; o++) {
                for (int s = 0; s < numSlices; s++) {
                    float azi = float(a) * 2 * M_PI / num_azimuth_bins;
                    float orb = float(o) / float(num_orbit_positions);
                    copy->_value[b][a][o][s] = RetrieveBySliceNumber(b,
                            azi, orb, s);
                }
            }
        }
    }
    return(1);
}

//--------------------//
// XTable::CheckEmpty //
//--------------------//

int XTable::CheckEmpty() {
    for (int b = 0; b < numBeams; b++) {
        for (int o = 0; o < numOrbitPositionBins; o++) {
            for (int a = 0; a < numAzimuthBins; a++) {
                for (int s = 0; s < numSlices; s++) {
                    if (_empty[b][a][o][s] == 1) {
                        fprintf(stderr, "Bin b=%d a=%d  o=%d s=%d is empty\n",
                                b, a, o, s);
                        return(0);
                    }
                }
            }
        }
    }
    return(1);
}

//---------------------//
// XTable::CheckHeader //
//---------------------//

int XTable::CheckHeader(int num_beams, int num_science_slices,
                        int num_guard_slices_each_side,
                        float science_bandwidth, float guard_bandwidth) {
  if(numBeams != num_beams) return(0);

  /******** If you have guard slices then the number of slices must be ***/
  /******** the same                                                   ***/
  if(num_guard_slices_each_side !=0) {
    if(numScienceSlices != num_science_slices) return(0);
    if(numGuardSlicesEachSide != num_guard_slices_each_side) return(0);
  }
  else if(numScienceSlices < num_science_slices) return 0;

  // For Bandwidths 1 Hz is close enough.   //

  if((int)(scienceSliceBandwidth + 0.5) != (int)(science_bandwidth + 0.5))
    return(0);
  if((int)(guardSliceBandwidth + 0.5) != (int)(guard_bandwidth + 0.5)
     && numGuardSlicesEachSide != 0 )
    return(0);
  return(1);
}

//---------------//
// XTable::Write //
//---------------//

int XTable::Write() {

      if (_filename == NULL)
    return(0);

      FILE* fp = fopen(_filename, "w");
      if (fp == NULL)
        return(0);

      if (! CheckEmpty()) {
    fprintf(stderr,"XTable::Write: There are empty entries!\n");
        return(0);
      }

      if (! _WriteHeader(fp))
    return(0);

      if (! _WriteTable(fp))
    return(0);

      return(1);

}

//--------------//
// XTable::Read //
//--------------//

int XTable::Read() {

      if (_filename == NULL)
      return(0);

      FILE* fp = fopen(_filename, "r");
      if (fp == NULL)
    return(0);
      if (! _ReadHeader(fp))
    return(0);

      if (! Allocate())
    return(0);

      if (! _ReadTable(fp))
    return(0);

      if (! CheckEmpty()) {
    fprintf(stderr,"XTable::Read: There are empty entries after reading!\n");
        return(0);
      }
      return(1);
}

//---------------------//
// XTable::SetFilename //
//---------------------//

int
XTable::SetFilename(const char* fname) {
    if (_filename != NULL)
        free(_filename);
    _filename = strdup(fname);
    if (_filename == NULL)
        return(0);
    return(1);
}

//-------------------------------//
// XTable::RetrieveBySliceNumber //
//-------------------------------//

float
XTable::RetrieveBySliceNumber(int beam_number, float azimuth_angle,
                  float orbit_position, int slice_number) {
  float azi, orb, acoeff1, acoeff2,retval,ocoeff1, ocoeff2;
  int azi1,azi2,orb1,orb2;

  if(beam_number < 0 || beam_number >= numBeams) {
    fprintf(stderr,"Error XTable::RetrieveBySliceNumber: Bad Beam No.\n");
    exit(1);
  }

  azi = azimuth_angle * numAzimuthBins/(2 * M_PI);

  while(azi < 0) azi += 2 * numAzimuthBins;
  azi1 = (int)(floor(azi)+0.1);
  azi2 = azi1+1;

  /**** Calculate coefficient for liner interpolation ***/
  acoeff1 = azi2-azi;
  acoeff2 = azi-azi1;

  /**** Make sure azimuth indices are in range ***/
  azi1 %= numAzimuthBins;
  azi2 %= numAzimuthBins;

  orb = orbit_position * numOrbitPositionBins;
  orb1 = (int)(floor(orb)+0.1);
  orb2 = orb1+1;

  /**** Calculate coefficient for linear interpolation ***/
  ocoeff1 = orb2-orb;
  ocoeff2 = orb-orb1;

  /**** Make sure orbit indices are in range ***/
  orb1 %= numOrbitPositionBins;
  orb2 %= numOrbitPositionBins;

  if(slice_number < 0 || slice_number >= numSlices) {
    fprintf(stderr,"Error XTable::RetrieveBySliceNumber: Bad Slice No.\n");
    exit(1);
  }

  retval = acoeff1 * ocoeff1 * _value[beam_number][azi1][orb1][slice_number];
  retval += acoeff1 * ocoeff2 * _value[beam_number][azi1][orb2][slice_number];
  retval += acoeff2 * ocoeff2 * _value[beam_number][azi2][orb2][slice_number];
  retval += acoeff2 * ocoeff1 * _value[beam_number][azi2][orb1][slice_number];
  return(retval);
}

//---------------------------------------//
// XTable::RetrieveByRelativeSliceNumber //
//---------------------------------------//

float
XTable::RetrieveByRelativeSliceNumber(int beam_number, float azimuth_angle,
                      float orbit_position,int slice_number) {
  int slice_idx;
  if(numSlices%2==1) slice_idx = numSlices/2+slice_number;
  else if(slice_number < 0) slice_idx = numSlices/2+slice_number;
  else if(slice_number > 0) slice_idx = numSlices/2-1+slice_number;
  else{
    fprintf(stderr,"Error in XTable::RetrieveByRelativeSliceNumber\n");
    fprintf(stderr,"For an even number of slices, slice #0 is undefined.");
    exit(1);
  }
  return(RetrieveBySliceNumber(beam_number, azimuth_angle,
                   orbit_position, slice_idx));
}

//-----------------------------//
// XTable::RetrieveBySliceFreq //
//-----------------------------//

float XTable::RetrieveBySliceFreq(int beam_number, float azimuth_angle,
                                  float orbit_position, float slice_min_freq,
                                  float slice_bandwidth) {
    float X = 0;

    if (beam_number < 0 || beam_number >= numBeams) {
        fprintf(stderr, "Error XTable::RetrieveBySliceFreq: Bad Beam No.\n");
        exit(1);
    }

    if (azimuth_angle < -(2 * M_PI)) {
        fprintf(stderr,
                "Error XTable::RetrieveBySliceFreq: Azimuth < -2PI rads\n");
        exit(1);
    }

    if (orbit_position < 0 || orbit_position > 1) {
        fprintf(stderr,
          "Error XTable::RetrieveBySliceFreq: Orbit Position not on [0,1]\n");
        exit(1);
    }

  //====================================================================//
  // To compute X we sum all the X's in the table which fall within the //
  // specified slice boundaries. Special care must be taken for the two //
  // X bins (slices in table0 which border the edges of the specified   //
  // slice. If the specifed slice is smaller than the table slices      //
  // an error results.                                                  //
  //====================================================================//

  float slice_max_freq = slice_min_freq+slice_bandwidth;

  int bin_start = FindSliceNum(slice_min_freq);
  int bin_end = FindSliceNum(slice_max_freq);

  if(bin_start==bin_end) {
    float bandwidth = GetBandwidth(bin_start);
    float bin_min_freq = GetMinFreq(bin_start);
    if((int)(slice_min_freq + 0.5) == (int)(bin_min_freq + 0.5)
       && (int)(bandwidth +0.5) == (int)(slice_bandwidth+0.5)) {
      return(RetrieveBySliceNumber(beam_number,azimuth_angle, orbit_position,
                   bin_start));
    }
    else{
      fprintf(stderr,"Error XTable::RetrieveBySliceFreq: Cannot resolve slice\n");
      exit(1);
    }
  }

  if(bin_start < 0 || bin_end >= numSlices) {
    fprintf(stderr,"Error XTable::RetrieveBySliceFreq: Slice out of range\n");
    exit(1);
  }

  float bandwidth = GetBandwidth(bin_start);
  float Xstart = RetrieveBySliceNumber(beam_number, azimuth_angle,
                     orbit_position, bin_start);
  float bin_min_freq = GetMinFreq(bin_start);
  float bin_max_freq = bin_min_freq+bandwidth;
  X = Xstart * (bin_max_freq-slice_min_freq)/bandwidth;
  for (int c = bin_start+1; c< bin_end; c++) {
    X = X+RetrieveBySliceNumber(beam_number,azimuth_angle,orbit_position,c);
  }
  bandwidth = GetBandwidth(bin_end);
  float Xend = RetrieveBySliceNumber(beam_number, azimuth_angle,
                   orbit_position, bin_end);
  bin_min_freq = GetMinFreq(bin_end);
  X = X+Xend * (slice_max_freq-bin_min_freq)/bandwidth;
  return(X);
}

//------------------//
// XTable::AddEntry //
//------------------//

int
XTable::AddEntry(float X, int beam_number, float azimuth_angle,
         float orbit_position, int slice_number) {
  float azi,orb;
  int azi_idx,orb_idx;

  if(beam_number < 0 || beam_number >= numBeams) {
    fprintf(stderr,"Error XTable::AddEntry: Bad Beam No.\n");
    return(0);
  }

  if(azimuth_angle < -(2 * M_PI)) {
    fprintf(stderr,"Error XTable::AddEntry: Azimuth < -2PI rads\n");
    return(0);
  }
  azi = azimuth_angle * numAzimuthBins/(2 * M_PI);
  if(azi < 0) azi += 2 * numAzimuthBins;
  azi_idx = (int)(azi+0.5);
  azi_idx %= numAzimuthBins;

  if(orbit_position < 0 || orbit_position > 1) {
    fprintf(stderr,"Error XTable::AddEntry: Orbit Position not on [0,1]\n");
    return(0);
  }

  orb = orbit_position * numOrbitPositionBins;
  orb_idx = (int)(orb+0.5);
  orb_idx %= numOrbitPositionBins;

  if(slice_number < 0 || slice_number >= numSlices) {
    fprintf(stderr,"Error XTable::AddEntry: Bad Slice No.\n");
    return(0);
  }

  //=======================//
  // Nonempty case         //
  // entry already written //
  //=======================//
  if(_empty[beam_number][azi_idx][orb_idx][slice_number]==0) return(1);

  _empty[beam_number][azi_idx][orb_idx][slice_number] = 0;
  _value[beam_number][azi_idx][orb_idx][slice_number] = X;

  return(1);
}

//--------------------//
// XTable::GetMinFreq //
//--------------------//

float
XTable::GetMinFreq(int slice_number) {
  float abs_min_freq, min_freq;
  abs_min_freq=-(float)numScienceSlices/2.0 * scienceSliceBandwidth;
  abs_min_freq-=numGuardSlicesEachSide * guardSliceBandwidth;
  int sn = slice_number;

  if(slice_number < 0 || slice_number >= numSlices) {
    fprintf(stderr,"Error XTable::GetMinFreq: Bad Slice No.\n");
    exit(1);
  }

  min_freq = abs_min_freq;
  if(sn < numGuardSlicesEachSide) {
    return(min_freq+sn * guardSliceBandwidth);
  }

  min_freq += numGuardSlicesEachSide * guardSliceBandwidth;
  sn-=numGuardSlicesEachSide;

  if(sn< numScienceSlices) {
    return(min_freq+sn * scienceSliceBandwidth);
  }

  min_freq += numScienceSlices * scienceSliceBandwidth;
  sn-=numScienceSlices;
  return(min_freq+sn * guardSliceBandwidth);
}

float
XTable::GetBandwidth(int slice_number) {

  if(slice_number < 0 || slice_number >= numSlices) {
    fprintf(stderr,"Error XTable::GetBandwidth: Bad Slice No.\n");
    exit(1);
  }
  if((slice_number >= numGuardSlicesEachSide) &&
     (slice_number < numGuardSlicesEachSide+numScienceSlices))
    return(scienceSliceBandwidth);
  else
    return(guardSliceBandwidth);
}

int
XTable::FindSliceNum(float freq) {
  float abs_min_freq;
  abs_min_freq=-(float)numScienceSlices/2.0 * scienceSliceBandwidth;
  abs_min_freq-=numGuardSlicesEachSide * guardSliceBandwidth;
  int sn;
  float fr = freq;

  if(fr < abs_min_freq) return(-1);

  fr-=abs_min_freq;
  sn = 0;

  if(fr < numGuardSlicesEachSide * guardSliceBandwidth) {
    return((int)(fr/guardSliceBandwidth));
  }

  fr-=numGuardSlicesEachSide * guardSliceBandwidth;
  sn += numGuardSlicesEachSide;

  if(fr < numScienceSlices * scienceSliceBandwidth) {
    return(sn+(int)(fr/scienceSliceBandwidth));
  }

  fr-=numScienceSlices * scienceSliceBandwidth;
  sn += numScienceSlices;
  sn += (int)(fr/guardSliceBandwidth);

  /*** If less than 1 Hz past top ignore discrepancy ***/
  if (sn==numSlices &&
      (int)(fr-numGuardSlicesEachSide * guardSliceBandwidth)==0 )
    sn = numSlices-1;
  return(sn);
}

int XTable::WriteXmgr(const char* filename, float orbit_position) {
      if (filename == NULL)
      return(0);

      FILE* fp = fopen(filename, "w");
      if (fp == NULL)
        return(0);
      for (int a = 0; a< numAzimuthBins; a++) {
    float azi = 360.0 * (float(a)/(float)numAzimuthBins);
    for (int b = 0; b< numBeams; b++) {
      fprintf(fp,"%g ", azi);
      for (int s = 0; s< numSlices; s++) {
        float X = RetrieveBySliceNumber(b,azi * dtr,orbit_position,s);
        fprintf(fp,"%g ",X);
      }
    }
    fprintf(fp,"\n");
      }
      return(1);

}

//===================//
// Protected Methods //
//===================//

int XTable::_Deallocate() {
  if(numBeams==0) return(0);

  free_array((void *)_value, 4, numBeams, numAzimuthBins, numOrbitPositionBins,
         numSlices);
  free_array((void *)_empty, 4, numBeams, numAzimuthBins, numOrbitPositionBins,
         numSlices);

  _value = NULL;
  _empty = NULL;
  numBeams = 0;
  return(1);
}

//----------------------------------//
// XTable::_WriteHeader           //
//----------------------------------//

int
XTable::_WriteHeader(FILE* ofp)
{
  if (fwrite((void*)&numBeams, sizeof(int),1,ofp) != 1) return(0);
  if (fwrite((void*)&numAzimuthBins, sizeof(int),1,ofp) != 1) return(0);
  if (fwrite((void*)&numOrbitPositionBins, sizeof(int),1,ofp) != 1) return(0);
  if (fwrite((void*)&numScienceSlices, sizeof(int),1,ofp) != 1) return(0);
  if (fwrite((void*)&numGuardSlicesEachSide, sizeof(int),1,ofp) != 1)
    return(0);
  if (fwrite((void*)&scienceSliceBandwidth, sizeof(float),1,ofp) != 1)
    return(0);
  if (fwrite((void*)&guardSliceBandwidth, sizeof(float),1,ofp) != 1)
    return(0);
  return(1);
}

//----------------------------------//
// XTable::_ReadHeader           //
//----------------------------------//

int
XTable::_ReadHeader(FILE* ifp)
{
  if (fread((void*)&numBeams, sizeof(int),1,ifp) != 1) return(0);
  if (fread((void*)&numAzimuthBins, sizeof(int),1,ifp) != 1) return(0);
  if (fread((void*)&numOrbitPositionBins, sizeof(int),1,ifp) != 1) return(0);
  if (fread((void*)&numScienceSlices, sizeof(int),1,ifp) != 1) return(0);
  if (fread((void*)&numGuardSlicesEachSide, sizeof(int),1,ifp) != 1)
    return(0);
  if (fread((void*)&scienceSliceBandwidth, sizeof(float),1,ifp) != 1)
    return(0);
  if (fread((void*)&guardSliceBandwidth, sizeof(float),1,ifp) != 1)
    return(0);

  numSlices = numScienceSlices+numGuardSlicesEachSide;
  return(1);
}

//----------------------------------//
// XTable::_WriteTable           //
//----------------------------------//

int
XTable::_WriteTable(FILE* ofp)
{
  for (int b = 0; b < numBeams; b++) {
    for (int a = 0; a < numAzimuthBins; a++) {
      for (int o = 0; o < numOrbitPositionBins; o++) {
    if(fwrite((void*)&_value[b][a][o][0], sizeof(float), numSlices,ofp)
       != (unsigned int) numSlices) return(0);
      }
    }
  }
  return(1);
}

//----------------------------------//
// XTable::_ReadTable               //
//----------------------------------//

int
XTable::_ReadTable(FILE* ifp)
{
  for (int b = 0; b < numBeams; b++) {
    for (int a = 0; a < numAzimuthBins; a++) {
      for (int o = 0; o < numOrbitPositionBins; o++) {
    if(fread((void*)&_value[b][a][o][0], sizeof(float), numSlices,ifp)
       != (unsigned int) numSlices) return(0);
    for (int s = 0; s < numSlices; s++) _empty[b][a][o][s] = 0;
      }
    }
  }
  return(1);
}

//==================================================//
//  FUNCTION  MakeKfactorTable                      //
//==================================================//

int
MakeKfactorTable(XTable* trueX, XTable* estX, XTable* Kfactor) {
  estX->CopyBlank(Kfactor);
    if(estX->numBeams != trueX->numBeams)
    {
        fprintf(stderr,"MakeKfactorTable: XTable mismatch. /n");
        return(0);
    }
  for (int b = 0; b < Kfactor->numBeams; b++) {
    for (int a = 0; a < Kfactor->numAzimuthBins; a++) {
      for (int o = 0; o < Kfactor->numOrbitPositionBins; o++) {
    for (int s = 0; s < Kfactor->numSlices; s++) {
      float azi = (float)a * (2 * M_PI)/(float)(Kfactor->numAzimuthBins);
          float orb = (float)o/(float)(Kfactor->numOrbitPositionBins);
      float bw = Kfactor->GetBandwidth(s);
      float freq = Kfactor->GetMinFreq(s);
      Kfactor->_value[b][a][o][s] =
        trueX->RetrieveBySliceFreq(b,azi,orb,freq,bw);
      Kfactor->_value[b][a][o][s]/=estX->_value[b][a][o][s];
      Kfactor->_empty[b][a][o][s] = 0;
    }
      }
    }
  }
  return(1);
}
