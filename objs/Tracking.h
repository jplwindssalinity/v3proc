//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef TRACKING_H
#define TRACKING_H

static const char rcs_id_tracking_h[] =
	"@(#) $Id$";


//======================================================================
// CLASSES
//		DopplerTracking, RangeTracking
//======================================================================

//======================================================================
// CLASS
//		DopplerTracking
//
// DESCRIPTION
//		The DopplerTracking object is used to store the Doppler
//		Tracking Constants and convert them into command Doppler
//		frequencies.
//======================================================================

/*
class DopplerTracking
{
public:

	//--------------//
	// construction //
	//--------------//

	DopplerTracking();
	~DopplerTracking();

	int		Allocate(int orbit_steps);

	//------------//
	// algorithms //
	//------------//

	int		Doppler(int orbit_step, int azimuth_step, float* doppler);

	//--------------//
	// input/output //
	//--------------//

	int		WriteBinary(const char* filename);
	int		ReadBinary(const char* filename);

	//-----------//
	// variables //
	//-----------//

	float				am, ab;		// scale factors
	float				pm, pb;
	float				cm, cb;

	short*				a;			// expansion terms
	short*				p;
	short*				c;

	float				z1, z2;		// dithering terms

	unsigned short		period;		// orbit period

	unsigned short		checksum;	// duh!

private:

	//-----------//
	// variables //
	//-----------//

	int			_orbitSteps;
	int			_azimuthSteps;
};
*/

//======================================================================
// CLASS
//		RangeTracking
//
// DESCRIPTION
//		The RangeTracking object is used to store the Range	Tracking
//		Constants and convert them into receiver gate delays.
//======================================================================

#define RANGE_TIME_RESOLUTION		5E-5		// seconds (0.05 ms)

class RangeTracking
{
public:

	//--------------//
	// construction //
	//--------------//

	RangeTracking();
	~RangeTracking();

	int		Allocate(int number_of_beams, int orbit_steps);

	//---------//
	// setting //
	//---------//

	int		SetReceiverGateWidth(float receiver_gate_width);
	int		SetXmitPulseWidth(float xmit_pulse_width);


	//------------//
	// algorithms //
	//------------//

	unsigned short		OrbitTimeToRangeStep(unsigned int orbit_time);
	int					DelayAndDuration(int beam_idx, int orbit_step,
							float* delay, float* duration);

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
	unsigned short		_ticksPerOrbit;		// orbit period

	//-----------//
	// variables //
	//-----------//

	int				_numberOfBeams;
	int				_rangeSteps;
	unsigned char	_receiverGateWidth;
	unsigned char	_xmitPulseWidth;
};

#endif
