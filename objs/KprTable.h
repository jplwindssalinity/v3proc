//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef KPRTABLE_H
#define KPRTABLE_H

static const char rcs_id_kprtable_h[] =
	"@(#) $Id$";


#include "Meas.h"
#include "L00Frame.h"

//======================================================================
// CLASSES
//		KprTable
//======================================================================

//======================================================================
// CLASS
//		KprTable
//
// DESCRIPTION
//              The KprTable is a class for creating and manipulating a
//              set of Kpr values.
//
// NOTES
//		Currently the kpr values are indexed by azimuth and
//              slice number
//		
//======================================================================

class KprTable
{
public:
   KprTable();
   KprTable(int number_of_beams, float slice_bandwidth, int slices_per_spot, 
	    int number_of_azimuth_bins, int min_num_samples);
   ~KprTable();
   int Accumulate(MeasSpotList* quiet, MeasSpotList* noisy);
   int Accumulate(Meas* quiet, Meas* noisy, int beam_idx, int slice_idx,
		  double azimuth);
   int Accumulate(L00Frame* quiet, L00Frame* noisy);
   int Smooth(int filter_width);
   float Interpolate(int beam_number, int slice_number, float azimuth);
   int Normalize();
   int NormalizeFrom3Sigma();
   int Write(const char* filename);
   int WriteXmgr(const char* filename);
   int Read(const char* filename);
   int GetNumBeams(){return(_numBeams);}
   int GetSlicesPerSpot(){return(_slicesPerSpot);}
   float GetSliceBandwidth(){return(_sliceBandwidth);}
   int GetNumAzimuths(){return(_numAzimuths);}

protected:

   int _Allocate();
   int _ReadHeader(FILE* ifp);
   int _ReadTable(FILE* ifp);
   int _WriteHeader(FILE* ofp);
   int _WriteTable(FILE* ofp);

   /********  variables **********/
   int _numBeams;
   int _slicesPerSpot;
   int _numAzimuths;
   int _minNumSamples;
   float _sliceBandwidth;

   int*** _numSamples;
   float*** _value;
   

};

#endif

