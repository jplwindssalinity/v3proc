//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef TRACKING_H
#define TRACKING_H

static const char rcs_id_tracking_h[] =
	"@(#) $Id$";


#include "Constants.h"
#include "Instrument.h"


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

//	int		Doppler(int orbit_step, int azimuth_step, float* doppler);
	int		Set(double*** terms);

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

	unsigned short		OrbitTimeToRangeStep(unsigned int orbit_time);
	int					GetDelayAndDuration(int beam_idx, int range_step,
							float xmit_pulse_width, float* delay,
							float* duration);
	int					SetInstrument(Instrument* instrument);
	int					SetRoundTripTime(int beam_idx, int range_step,
							float round_trip_time);
	int					SetDuration(int beam_idx, float duration);
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

	unsigned char**		_delay;				// delay[beam][step] arrays
	unsigned char*		_duration;			// duration[beam] terms
	unsigned int		_ticksPerOrbit;		// orbit period

	//-----------//
	// variables //
	//-----------//

	unsigned int	_numberOfBeams;
	unsigned int	_rangeSteps;
};

#endif
