//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef TRACKING_H
#define TRACKING_H

static const char rcs_id_tracking_h[] =
	"@(#) $Id$";


#include "Constants.h"

class Instrument;

//======================================================================
// CLASSES
//		DopplerTracker, RangeTracker
//======================================================================

#define DOPPLER_TRACKING_RESOLUTION		2000		// Hz

//======================================================================
// CLASS
//		DopplerTracker
//
// DESCRIPTION
//		The DopplerTracker object is used to store the Doppler
//		Tracking Constants and convert them into command Doppler
//		frequencies.
//======================================================================

class DopplerTracker
{
public:

	enum { AMPLITUDE_INDEX = 0, PHASE_INDEX, CONSTANTS_INDEX };

	//--------------//
	// construction //
	//--------------//

	DopplerTracker();
	~DopplerTracker();

	int		Allocate(int number_of_beams, int doppler_orbit_steps);

	//------------//
	// algorithms //
	//------------//

	unsigned int	OrbitTicksToDopplerStep(unsigned int orbit_ticks);
	int				GetCommandedDoppler(int beam_idx,
						unsigned int doppler_step, unsigned int antenna_dn,
						unsigned int antenna_n, float* doppler);
	int				SetInstrument(Instrument* instrument);
	int				Set(double*** terms);
	int				SetTicksPerOrbit(unsigned int period);

	//--------------//
	// input/output //
	//--------------//

	int		WriteBinary(const char* filename);
	int		ReadBinary(const char* filename);

private:

	//-----------//
	// variables //
	//-----------//

	float***			_scale;		// [beam][term][coef_order]
	unsigned short***	_term;		// [beam][step][term]
	unsigned int		_ticksPerOrbit;		// orbit period

	//-----------//
	// variables //
	//-----------//

	unsigned int	_numberOfBeams;
	unsigned int	_dopplerSteps;
};

//======================================================================
// CLASS
//		RangeTracker
//
// DESCRIPTION
//		The RangeTracker object is used to store the Range	Tracking
//		Constants and convert them into receiver gate delays.
//======================================================================

#define RANGE_TRACKING_TIME_RESOLUTION		5E-5		// seconds (0.05 ms)

class RangeTracker
{
public:

	enum { AMPLITUDE_INDEX = 0, PHASE_INDEX, CONSTANTS_INDEX };

	//--------------//
	// construction //
	//--------------//

	RangeTracker();
	~RangeTracker();

	int		Allocate(int number_of_beams, int range_steps);

	//---------//
	// setting //
	//---------//

	//------------//
	// algorithms //
	//------------//

	unsigned short		OrbitTicksToRangeStep(unsigned int orbit_ticks);
	int					GetDelayAndDuration(int beam_idx,
							unsigned int range_step, float xmit_pulse_width,
							unsigned int antenna_dn, unsigned int antenna_n,
							float* delay, float* width);
	int					GetNumberOfBeams() { return(_numberOfBeams); };
	int					GetRangeSteps() { return(_rangeSteps); };
	int					SetInstrument(Instrument* instrument);
	int					SetRoundTripTime(double*** terms);
	int					SetDuration(int beam_idx, float width);
	int					SetTicksPerOrbit(unsigned int period);

	//--------------//
	// input/output //
	//--------------//

	int		WriteBinary(const char* filename);
	int		ReadBinary(const char* filename);

private:

	//-----------//
	// variables //
	//-----------//

	float***			_scale;				// [beam][term][coef_order]
	unsigned char***	_delay;				// [beam][step][term]
	unsigned char*		_width;				// [beam]
	unsigned int		_ticksPerOrbit;		// orbit period

	//-----------//
	// variables //
	//-----------//

	unsigned int	_numberOfBeams;
	unsigned int	_rangeSteps;
};

int		azimuth_fit(int count, double* terms, double* a, double* p, double* c);

#endif
