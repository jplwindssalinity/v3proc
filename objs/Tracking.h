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

//======================================================================
// CLASS
//		RangeTracking
//
// DESCRIPTION
//		The RangeTracking object is used to store the Range	Tracking
//		Constants and convert them into receiver gate delays.
//======================================================================

class RangeTracking
{
public:

	//--------------//
	// construction //
	//--------------//

	RangeTracking(int orbit_steps);
	~RangeTracking();

	//------------//
	// algorithms //
	//------------//

	int		DelayAndDuration(int orbit_step, float* delay, float* duration);

	//--------------//
	// input/output //
	//--------------//

	int		WriteBinary(const char* filename);
	int		ReadBinary(const char* filename);

	//-----------//
	// variables //
	//-----------//

	unsigned char*		da;			// delay arrays
	unsigned char*		db;

	unsigned char		wa;			// duration arrays
	unsigned char		wb;

	unsigned char		z;			// dithering constant

	unsigned short		period;		// orbit period

	unsigned short		checksum;	// duh!

private:

	//-----------//
	// variables //
	//-----------//

	int			_orbitSteps;
	int			_azimuthSteps;
};

#endif
