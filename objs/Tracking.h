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
//		RangeTracker, DopplerTracker
//======================================================================

//======================================================================
// CLASS
//		RangeTracker
//
// DESCRIPTION
//		The RangeTracker object is used to store the Range	Tracking
//		Constants and convert them into receiver gate delays.
//======================================================================

#define RANGE_TRACKING_TIME_RESOLUTION		4.9903E-5	// seconds (~0.05 ms)

class RangeTracker
{
public:

	enum { AMPLITUDE_INDEX = 0, PHASE_INDEX, CONSTANTS_INDEX };

	//--------------//
	// construction //
	//--------------//

	RangeTracker();
	~RangeTracker();

	int		Allocate(int range_steps);

	//------------//
	// algorithms //
	//------------//

	unsigned short		OrbitTicksToRangeStep(unsigned int orbit_ticks,
							unsigned int ticks_per_orbit);
	int					GetRxGateDelay(unsigned int range_step,
							float xmit_pulse_width, float rx_gate_width,
							unsigned int antenna_dn, unsigned int antenna_n,
							float* delay);
	float				QuantizeWidth(float width);
	float				QuantizeDelay(float delay, float* residual_delay);
	int					SetInstrument(Instrument* instrument,
							float* residual_delay);
	int					GetRangeSteps() { return(_rangeSteps); };
	int					SetRoundTripTime(double** terms);

	//--------------//
	// input/output //
	//--------------//

	int		WriteBinary(const char* filename);
	int		ReadBinary(const char* filename);
	int		WriteHex(const char* filename);
	int		ReadHex(const char* filename);

private:

	//------------------//
	// header variables //
	//------------------//

	unsigned short		_tableId;
	unsigned short		_dither[2];

	//-----------//
	// variables //
	//-----------//

	float**				_scale;				// [term][coef_order]
	unsigned char**		_delay;				// [step][term]

	//-----------//
	// variables //
	//-----------//

	unsigned int	_rangeSteps;
};


//======================================================================
// CLASS
//		DopplerTracker
//
// DESCRIPTION
//		The DopplerTracker object is used to store the Doppler
//		Tracking Constants and convert them into command Doppler
//		frequencies.
//======================================================================

#define DOPPLER_TRACKING_RESOLUTION		2000		// Hz

class DopplerTracker
{
public:

	enum { AMPLITUDE_INDEX = 0, PHASE_INDEX, CONSTANTS_INDEX };

	//--------------//
	// construction //
	//--------------//

	DopplerTracker();
	~DopplerTracker();

	int				Allocate(int doppler_orbit_steps);

	//------------//
	// algorithms //
	//------------//

	unsigned int	OrbitTicksToDopplerStep(unsigned int orbit_ticks,
						unsigned int ticks_per_orbit);
	int				GetCommandedDoppler(unsigned int doppler_step,
						unsigned int antenna_dn, unsigned int antenna_n,
						float* doppler, float chirp_rate = 0.0,
						float residual_delay = 0.0);
	float			QuantizeFrequency(float frequency);
	int				SetInstrument(Instrument* instrument,
						float residual_delay);
	int				Set(double** terms);

	//--------------//
	// input/output //
	//--------------//

	int		WriteBinary(const char* filename);
	int		ReadBinary(const char* filename);
	int		WriteHex(const char* filename);
	int		ReadHex(const char* filename);

private:

	//------------------//
	// header variables //
	//------------------//

	unsigned short		_tableId;
	unsigned short		_dither[2];

	//-----------//
	// variables //
	//-----------//

	float**				_scale;				// [term][coef_order]
	unsigned short**	_term;				// [step][term]

	//-------------------//
	// used, not written //
	//-------------------//

	unsigned int	_dopplerSteps;
};

int		azimuth_fit(int count, double* terms, double* a, double* p, double* c);
int		write_hex(FILE* fp, unsigned short* buffer, int words);

#endif
