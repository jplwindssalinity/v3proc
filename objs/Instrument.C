//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_instrumentsim_c[] =
	"@(#) $Id$";

#include "Instrument.h"


//=================//
// InstrumentEvent //
//=================//

InstrumentEvent::InstrumentEvent()
:	time(0.0), eventId(NONE), beamIdx(0)
{
	return;
}

InstrumentEvent::~InstrumentEvent()
{
	return;
}


//============//
// Instrument //
//============//

Instrument::Instrument()
:	time(0.0), orbitTicks(0), commandedDoppler(0.0), receiverGateDelay(0.0),
	receiverGateDuration(0.0), systemLoss(0.0), transmitPower(0.0),
	receiverGain(0.0), chirpRate(0.0), chirpStartM(0.0), chirpStartB(0.0),
	systemDelay(0.0), systemTemperature(0.0), baseTransmitFreq(0.0),
	transmitFreq(0.0), sliceBandwidth(0.0), noiseBandwidth(0.0),
	signalBandwidth(0.0), useKpc(1), _eqxTime(0)
{
	return;
}

Instrument::~Instrument()
{
	return;
}

//---------------------//
// Instrument::SetTime //
//---------------------//

int
Instrument::SetTime(
	double		new_time)
{
	time = new_time;
	double time_since_eqx = time - _eqxTime;
	orbitTicks = TimeToOrbitTicks(time_since_eqx);
	return(1);
}

//-----------------//
// Instrument::Eqx //
//-----------------//

int
Instrument::Eqx(
	double	eqx_time)
{
	_eqxTime = eqx_time;
	return(1);
}

//------------------------------//
// Instrument::TimeToOrbitTicks //
//------------------------------//

unsigned int
Instrument::TimeToOrbitTicks(
	double		time)
{
	unsigned int ticks = (unsigned int)(time * ORBIT_TICKS_PER_SECOND);
	return(ticks);
}
