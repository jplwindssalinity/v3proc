//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_instrumentsim_c[] =
	"@(#) $Id$";

#include "Instrument.h"

//============//
// Instrument //
//============//

Instrument::Instrument()
:	_pri_per_beam(0.0), _pri_per_beam_set(0),
	_beam_b_time_offset(0.0), _beam_b_time_offset_set(0)
{
	return;
}

Instrument::~Instrument()
{
	return;
}

//--------------------//
// Instrument::SetXxx //
//--------------------//

void Instrument::SetPriPerBeam(double value)
{
	_pri_per_beam = value;
	_pri_per_beam_set = 1;
	return;
}

void Instrument::SetBeamBTimeOffset(double value)
{
	_beam_b_time_offset = value;
	_beam_b_time_offset_set = 1;
	return;
}

//--------------------//
// Instrument::GetXxx //
//--------------------//

int Instrument::GetPriPerBeam(double* value)
{
	if (! _pri_per_beam_set) return(0);
	*value = _pri_per_beam;
	return(1);
}

int Instrument::GetBeamBTimeOffset(double* value)
{
	if (! _beam_b_time_offset_set) return(0);
	*value = _beam_b_time_offset;
	return(1);
}
