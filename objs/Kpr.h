//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

#ifndef KPR_H
#define KPR_H

static const char rcs_id_kpr_h[] =
	"@(#) $Id$";


#include "Meas.h"
#include "L00Frame.h"

//======================================================================
// CLASSES
//		Kpri, Kprs
//======================================================================

//======================================================================
// CLASS
//		Kpri
//
// DESCRIPTION
//		The Kpri class is used for handling instrument Kpr
//======================================================================

class Kpri
{
public:

	//--------------//
	// construction //
	//--------------//

	Kpri();
	~Kpri();

	GetKpri2(double* kpri2);
};


//======================================================================
// CLASS
//		Kprs
//
// DESCRIPTION
//				Kprs is a class for creating and manipulating a
//				set of spacecraft Kpr values.
//
// NOTES
//		Currently the kprS values are indexed by azimuth and
//				slice number
//
//======================================================================

class Kprs
{
public:

	//--------------//
	// construction //
	//--------------//

	Kprs();
	Kprs(int number_of_beams, float slice_bandwidth, int slices_per_spot,
		int number_of_azimuth_bins, int min_num_samples);
	~Kprs();

	int		Accumulate(MeasSpotList* quiet, MeasSpotList* noisy);
	int		Accumulate(Meas* quiet, Meas* noisy, int beam_idx, int slice_idx,
				double azimuth);
	int		Accumulate(L00Frame* quiet, L00Frame* noisy);
	int		Smooth(int filter_width);
	float	Interpolate(int beam_number, int slice_number, float azimuth);
	int		Normalize();
	int		NormalizeFrom3Sigma();
	int		Write(const char* filename);
	int		WriteXmgr(const char* filename);
	int		Read(const char* filename);
	int		GetNumBeams() { return(_numBeams); };
	int		GetSlicesPerSpot() { return(_slicesPerSpot); };
	float	GetSliceBandwidth() {return(_sliceBandwidth); };
	int		GetNumAzimuths() {return(_numAzimuths); };

protected:

	int		_Allocate();
	int		_ReadHeader(FILE* ifp);
	int		_ReadTable(FILE* ifp);
	int		_WriteHeader(FILE* ofp);
	int		_WriteTable(FILE* ofp);

	/******** variables **********/
	int		_numBeams;
	int		_slicesPerSpot;
	int		_numAzimuths;
	int		_minNumSamples;
	float	_sliceBandwidth;

	int***		_numSamples;
	float***	_value;
};

#endif
