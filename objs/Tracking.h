//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

#ifndef TRACKING_H
#define TRACKING_H

static const char rcs_id_tracking_h[] =
	"@(#) $Id$";

#include "Constants.h"

class Instrument;
class Antenna;
class Beam;

//======================================================================
// CLASSES
//		TrackerBase, RangeTracker, DopplerTracker
//======================================================================

float	Cosine(float angle);

//======================================================================
// CLASS
//		TrackerBase
//
// DESCRIPTION
//		The TrackerBase class is a base class for the RangeTracker and
//		DopplerTracker objects.
//======================================================================

template <class T>
class TrackerBase
{
public:

	enum { AMPLITUDE_INDEX = 0, PHASE_INDEX, BIAS_INDEX };

	//--------------//
	// construction //
	//--------------//

	TrackerBase();
	~TrackerBase();

	int		Allocate(unsigned int steps);

	//--------------//
	// input/output //
	//--------------//

	int		WriteHex(const char* filename);

	//------------//
	// algorithms //
	//------------//

	unsigned short	OrbitTicksToStep(unsigned int orbit_ticks,
						unsigned int ticks_per_orbit);
	unsigned int	AngleOffset(Antenna* antenna, Beam* beam);

	//--------//
	// access //
	//--------//

	int		GetSteps() { return(_steps); };

protected:

	//-----------//
	// variables //
	//-----------//

	unsigned short	_tableId;
	float**			_scaleArray;		// [term][coef_order]
	T**				_termArray;			// [step][term]
	unsigned int	_steps;
	unsigned short	_dither[2];

	float			_previousDelay;		// seconds
};

#define DEFAULT_STEPS		256

//======================================================================
// CLASS
//		RangeTracker
//
// DESCRIPTION
//		The RangeTracker object is used to store the Range Tracking
//		Constants and convert them into receiver gate delays.
//======================================================================

class RangeTracker : public TrackerBase<unsigned char>
{
public:

	enum { AMPLITUDE_INDEX = 0, PHASE_INDEX, BIAS_INDEX };

	//--------------//
	// construction //
	//--------------//

	RangeTracker();
	~RangeTracker();

	//------------//
	// algorithms //
	//------------//

	int		GetRxGateDelay(unsigned int range_step, float xmit_pulse_width,
				float rx_gate_width, unsigned int antenna_dn,
				unsigned int antenna_n, float* delay);
	float	QuantizeWidth(float width);
	float	QuantizeDelay(float delay, float* residual_delay);
	int		SetInstrument(Instrument* instrument, float* residual_delay);
	int		SetRoundTripTime(double** terms);

	//--------------//
	// input/output //
	//--------------//

	int		WriteBinary(const char* filename);
	int		ReadBinary(const char* filename);
	int		WriteHex(const char* filename);
	int		ReadHex(const char* filename);
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

class DopplerTracker : public TrackerBase<unsigned short>
{
public:

	enum { AMPLITUDE_INDEX = 0, PHASE_INDEX, BIAS_INDEX };

	//--------------//
	// construction //
	//--------------//

	DopplerTracker();
	~DopplerTracker();

	//------------//
	// algorithms //
	//------------//

	int			GetCommandedDoppler(unsigned int doppler_step,
					float rx_gate_delay, unsigned int antenna_dn,
					unsigned int antenna_n, float* doppler,
					float chirp_rate = 0.0, float residual_delay = 0.0);
	float		QuantizeFrequency(float frequency);
	int			SetInstrument(Instrument* instrument, float residual_delay);
	int			Set(double** terms);

	//--------------//
	// input/output //
	//--------------//

	int		WriteBinary(const char* filename);
	int		ReadBinary(const char* filename);
	int		WriteHex(const char* filename);
	int		ReadHex(const char* filename);
};

int		azimuth_fit(int count, double* terms, double* a, double* p, double* c);
int		write_hex(FILE* fp, char* buffer, int bytes);
int		read_hex(FILE* fp, char* buffer, int bytes);

#endif
