//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef INSTRUMENT_H
#define INSTRUMENT_H

static const char rcs_id_instrument_h[] =
	"@(#) $Id$";

//======================================================================
// CLASSES
//		Instrument
//======================================================================


//======================================================================
// CLASS
//		Instrument
//
// DESCRIPTION
//		The Instrument object holds all of the instrument parameters.
//		It simulates the operation of the instrument.
//======================================================================

class Instrument
{
public:

	//-------------//
	// contruction //
	//-------------//

	Instrument();
	~Instrument();

	//-------------------//
	// setting variables //
	//-------------------//

	void	SetPriPerBeam(double value);
	void	SetBeamBTimeOffset(double value);

	//-------------------//
	// getting variables //
	//-------------------//

	int		GetPriPerBeam(double *value);
	int		GetBeamBTimeOffset(double *value);

protected:

	//-----------//
	// variables //
	//-----------//

	double		_pri_per_beam;			// seconds
	double		_beam_b_time_offset;	// seconds

	unsigned char	_pri_per_beam_set;
	unsigned char	_beam_b_time_offset_set;
};

#endif
