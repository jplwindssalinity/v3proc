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

int	GetKpri2(double* kpri2);
int	SetKpPtGr(double kp_ptgr);

protected:

	double _kpPtGr;
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
	Kprs(int num_beams, float science_bandwidth, 
	     int num_science_slices,
             int num_guard_slices_each_side,
	     int number_of_azimuth_bins, int min_num_samples);
	~Kprs();

	int		Accumulate(MeasSpotList* quiet, MeasSpotList* noisy);
        int             GetSliceShift(MeasSpot* spot);
	int		Accumulate(Meas* quiet, Meas* noisy, 
				   int start_slice_idx); 
	int		Smooth(int filter_width);
	float	        Interpolate(int beam_number, int num_slices_in_comp,
				    int start_slice_rel_idx, float azimuth);
	int		Normalize();
        int             Empty(); 

	// Checks to see if header matches parameters. // 
	int             CheckHeader(int num_beams, int num_science_slices,
				    int num_guard_slices_each_side, 
				    float science_bandwidth,
				    float guard_bandwidth); 



	int		Write(const char* filename);
	int		WriteXmgr(const char* filename, 
				  int num_slices_per_comp);
	int		Read(const char* filename);
	int		GetNumBeams() { return(_numBeams); };
	int		GetNumSlices() { return(_numSlices); };
	int		GetNumScienceSlices() { return(_numScienceSlices); };
	int		GetNumGuardSlicesEachSide() 
	                { return(_numGuardSlicesEachSide); };
	float	        GetScienceBandwidth() {return(_scienceBandwidth); };
	int		GetNumAzimuths() {return(_numAzimuths); };

protected:

	int		_Allocate();
	int		_ReadHeader(FILE* ifp);
	int		_ReadTable(FILE* ifp);
	int		_WriteHeader(FILE* ofp);
	int		_WriteTable(FILE* ofp);

	/******** variables **********/
	int		_numBeams;
	int		_numScienceSlices;
        int             _numGuardSlicesEachSide;
        int             _numSlices;
	int		_numAzimuths;
	int		_minNumSamples;
	float	        _scienceBandwidth;

	int****		_numSamples;
	float****	_value;
};

#endif
