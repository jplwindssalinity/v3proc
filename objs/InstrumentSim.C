//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_instrumentsim_c[] =
	"@(#) $Id$";

#include "InstrumentSim.h"
#include "Instrument.h"
#include "GenericGeom.h"
#include "InstrumentGeom.h"
#include "Ephemeris.h"
#include "Sigma0.h"
#include "Constants.h"


//===============//
// InstrumentSim //
//===============//

InstrumentSim::InstrumentSim()
:	startTime(0.0), endTime(0.0), l00FrameReady(0), _spotNumber(0)
{
	return;
}

InstrumentSim::~InstrumentSim()
{
	return;
}

//---------------------------//
// InstrumentSim::Initialize //
//---------------------------//

int
InstrumentSim::Initialize(
	Antenna*	antenna)
{
	for (int i = 0; i < antenna->numberOfBeams; i++)
	{
		_scatBeamTime[i] = startTime + antenna->beam[i].timeOffset;
	}
	return(1);
}

//-----------------------------------//
// InstrumentSim::DetermineNextEvent //
//-----------------------------------//

int
InstrumentSim::DetermineNextEvent(
	Antenna*			antenna,
	InstrumentEvent*	instrument_event)
{
	//----------------------------------------//
	// find minimum time from possible events //
	//----------------------------------------//

	int min_idx = 0;
	double min_time = _scatBeamTime[0];
	for (int i = 1; i < antenna->numberOfBeams; i++)
	{
		if (_scatBeamTime[i] < min_time)
		{
			min_idx = i;
			min_time = _scatBeamTime[i];
		}
	}

	//-----------------------//
	// set event information //
	//-----------------------//

	instrument_event->time = min_time;
	instrument_event->eventId =
		InstrumentEvent::SCATTEROMETER_MEASUREMENT;
	instrument_event->beamIdx = min_idx;

	//----------------------------//
	// update next time for event //
	//----------------------------//

	if (min_idx == 0)
	{
		_scatBeamTime[min_idx] = (double)(int)(min_time /
			antenna->priPerBeam + 1.5) * antenna->priPerBeam;
	}
	else
	{
		_scatBeamTime[min_idx] = (double)(int)(min_time /
			antenna->priPerBeam + 1.0) * antenna->priPerBeam +
			antenna->beam[min_idx].timeOffset;
	}

	return(1);
}

//--------------------------------------//
// InstrumentSim::UpdateAntennaPosition //
//--------------------------------------//

int
InstrumentSim::UpdateAntennaPosition(
	double			time,
	Instrument*		instrument)
{
	antennaSim.UpdatePosition(time, &(instrument->antenna));
	return(1);
}

//-----------------------------//
// InstrumentSim::LocateSlices //
//-----------------------------//

int
InstrumentSim::LocateSlices(
	double			time,
	Spacecraft*		spacecraft,
	Instrument*		instrument,
	MeasSpot*		meas_spot)
{
	//-----------//
	// predigest //
	//-----------//

	Antenna* antenna = &(instrument->antenna);
	Beam* beam = antenna->GetCurrentBeam();
	OrbitState* orbit_state = &(spacecraft->orbitState);
	Attitude* attitude = &(spacecraft->attitude);

	//------------------//
	// set up meas spot //
	//------------------//

	meas_spot->FreeContents();
	meas_spot->time = time;
	meas_spot->scOrbitState = *orbit_state;
	meas_spot->scAttitude = *attitude;

	//--------------------------------//
	// generate the coordinate switch //
	//--------------------------------//

	CoordinateSwitch antenna_frame_to_gc = AntennaFrameToGC(orbit_state,
		attitude, antenna);

	//------------------------//
	// determine slicing info //
	//------------------------//

	int slice_count = l00.frame.slicesPerSpot;
	float total_freq = slice_count * instrument->sliceBandwidth;
	float min_freq = -total_freq / 2.0;

	//-------------------------------------------------//
	// calculate Doppler shift and receiver gate delay //
	//-------------------------------------------------//

	Vector3 vector;

	double look, azimuth;
	if (! beam->GetElectricalBoresight(&look, &azimuth))
		return(0);

	vector.SphericalSet(1.0, look, azimuth);		// boresight
	DopplerAndDelay(&antenna_frame_to_gc, spacecraft, instrument, vector);

	//-------------------//
	// for each slice... //
	//-------------------//

	for (int slice_idx = 0; slice_idx < l00.frame.slicesPerSpot;
		slice_idx++)
	{
		//-------------------------//
		// create a new measurment //
		//-------------------------//

		Meas* meas = new Meas();
		meas->pol = beam->polarization;
	
		//----------------------------------------//
		// determine the baseband frequency range //
		//----------------------------------------//

		float f1 = min_freq + slice_idx * instrument->sliceBandwidth;
		float f2 = f1 + instrument->sliceBandwidth;

		//----------------//
		// find the slice //
		//----------------//

		EarthPosition centroid;
		Vector3 look_vector;
		// guess at a reasonable slice frequency tolerance of 1%
		float ftol = fabs(f1 - f2) / 100.0;
		if (! FindSlice(&antenna_frame_to_gc, spacecraft, instrument,
			look, azimuth, f1, f2, ftol, &(meas->outline), &look_vector,
			&centroid))
		{
			return(0);
		}

		//---------------------------//
		// generate measurement data //
		//---------------------------//

		// get local measurement azimuth
		CoordinateSwitch gc_to_surface =
			centroid.SurfaceCoordinateSystem();
		Vector3 rlook_surface = gc_to_surface.Forward(look_vector);
		double r, theta, phi;
		rlook_surface.SphericalGet(&r, &theta, &phi);
		meas->eastAzimuth = phi;

		// get incidence angle
		meas->incidenceAngle = centroid.IncidenceAngle(look_vector);
		meas->center = centroid;

		//-----------------------------//
		// add measurment to meas spot //
		//-----------------------------//

		meas_spot->slices.Append(meas);
	}
	return(1);
}

//---------------------------//
// InstrumentSim::LocateSpot //
//---------------------------//

int
InstrumentSim::LocateSpot(
	double			time,
	Spacecraft*		spacecraft,
	Instrument*		instrument,
	MeasSpot*		meas_spot)
{
	//-----------//
	// predigest //
	//-----------//

	Antenna* antenna = &(instrument->antenna);
	Beam* beam = antenna->GetCurrentBeam();
	OrbitState* orbit_state = &(spacecraft->orbitState);
	Attitude* attitude = &(spacecraft->attitude);

	//------------------//
	// set up meas spot //
	//------------------//

	meas_spot->FreeContents();
	meas_spot->time = time;
	meas_spot->scOrbitState = *orbit_state;
	meas_spot->scAttitude = *attitude;

	//--------------------------------//
	// generate the coordinate switch //
	//--------------------------------//

	CoordinateSwitch antenna_frame_to_gc = AntennaFrameToGC(orbit_state,
		attitude, antenna);

	//---------------------------------------------------//
	// calculate the look vector in the geocentric frame //
	//---------------------------------------------------//

	Vector3 rlook_antenna;

	double look, azimuth;
	if (! beam->GetElectricalBoresight(&look, &azimuth))
		return(0);

	rlook_antenna.SphericalSet(1.0, look, azimuth);
	TargetInfoPackage  tip;
	RangeAndRoundTrip(&antenna_frame_to_gc, spacecraft, rlook_antenna, &tip);

	Vector3 rlook_gc = antenna_frame_to_gc.Forward(rlook_antenna);

	//----------------------------//
	// calculate the 3 dB outline //
	//----------------------------//

	Meas* meas = new Meas();	// need the outline to append to

	// get the max gain value.
	float gp_max;
	beam->GetPowerGainProduct(look,azimuth,tip.roundTripTime,
			instrument->antenna.spinRate,&gp_max);

	// Align beam frame z-axis with the electrical boresight.
    Attitude beam_frame;
    beam_frame.Set(0.0,look,azimuth,3,2,1);
    CoordinateSwitch ant_to_beam(beam_frame);
    CoordinateSwitch beam_to_ant = ant_to_beam.ReverseDirection();

	//
	// In the beam frame, for a set of azimuth angles, search for the
	// theta angle that has the required gain product.
	// Convert the results to the geocentric frame and find
	// the earth intercepts.
	//
	
	for (int i=0; i < 16; i++)
	{
		double phi = pi/8*i;

		// Setup for bisection search for the half power product point.

		double theta_max = 0.0;
		double theta_min = 5.0*dtr;
		double theta;
		int NN = (int)(log((theta_min-theta_max)/(0.01*dtr))/log(2)) + 1;
		Vector3 look_mid;
		Vector3 look_mid_ant;
		Vector3 look_mid_gc;

		for (int j = 1; j <= NN; j++)
		{	// Bisection search
			theta = (theta_max + theta_min)/2.0;
			look_mid.SphericalSet(1.0,theta,phi);
			look_mid_ant = beam_to_ant.Forward(look_mid);
			double r,look,azimuth;
			look_mid_ant.SphericalGet(&r,&look,&azimuth);
			float gp;
			beam->GetPowerGainProduct(look,azimuth,tip.roundTripTime,
				instrument->antenna.spinRate,&gp);
			if (gp > 0.5*gp_max)
			{
				theta_max = theta;
			}
			else
			{
				theta_min = theta;
			}
		}

		look_mid_gc = antenna_frame_to_gc.Forward(look_mid_ant);
		EarthPosition *r = new EarthPosition;
		*r = earth_intercept(orbit_state->rsat,look_mid_gc);
		if (! meas->outline.Append(r))
		{
			printf("Error appending to spot outline\n");
			return(0);
		}
	}

	//---------------------------//
	// generate measurement data //
	//---------------------------//

	meas->pol = beam->polarization;

	// get local measurement azimuth
	CoordinateSwitch gc_to_surface = tip.rTarget.SurfaceCoordinateSystem();
	Vector3 rlook_surface = gc_to_surface.Forward(rlook_gc);
	double r, theta, phi;
	rlook_surface.SphericalGet(&r, &theta, &phi);
	meas->eastAzimuth = phi;

	// get incidence angle
	meas->incidenceAngle = tip.rTarget.IncidenceAngle(rlook_gc);
	meas->center = tip.rTarget;

	//-----------------------------//
	// add measurment to meas spot //
	//-----------------------------//

	meas_spot->slices.Append(meas);

	return(1);
}

//--------------------------------//
// InstrumentSim::SetMeasurements //
//--------------------------------//

int
InstrumentSim::SetMeasurements(	
	Spacecraft*             spacecraft,
	Instrument*             instrument,
	MeasSpot*		meas_spot,
	WindField*		windfield,
	GMF*			gmf)
{
	//-------------------------//
	// for each measurement... //
	//-------------------------//

	for (Meas* meas = meas_spot->slices.GetHead(); meas;
		meas = meas_spot->slices.GetNext())
	{
		//----------------------------------------//
		// get lon and lat for the earth location //
		//----------------------------------------//

		double alt, lat, lon;
		if (! meas->center.GetAltLonGDLat(&alt, &lon, &lat))
			return(0);

		LonLat lon_lat;
		lon_lat.longitude = lon;
		lon_lat.latitude = lat;

		//-----------------//
		// get wind vector //
		//-----------------//

		WindVector wv;
		if (! windfield->InterpolatedWindVector(lon_lat, &wv))
		{
			wv.spd = 0.0;
			wv.dir = 0.0;
		}


		//--------------------------------//
		// convert wind vector to sigma-0 //
		//--------------------------------//

		// chi is defined so that 0.0 means the wind is blowing towards
		// the s/c (the opposite direction as the look vector)
		float chi = wv.dir - meas->eastAzimuth + pi;
		float sigma0;
		gmf->GetInterpolatedValue(meas->pol, meas->incidenceAngle, wv.spd,
			chi, &sigma0);


		//--------------------------------//
		// generate the coordinate switch //
		//--------------------------------//
		
		CoordinateSwitch gc_to_antenna = AntennaFrameToGC(
						    &(spacecraft->orbitState),
						    &(spacecraft->attitude), 
						    &(instrument->antenna));
		gc_to_antenna=gc_to_antenna.ReverseDirection();


		//-------------------------------//
		// convert Sigma0 to Power       //
		//-------------------------------//

                /************* FOR NOW Kfactor=1.0  *********/
		float Kfactor=1.0;
		if(! sigma0_to_Pr(spacecraft, instrument, meas,
				  Kfactor, &gc_to_antenna, sigma0,
					&(meas->value))) return(0);

	}
	return(1);
}

//---------------------------------//
// InstrumentSim::SetL00Spacecraft //
//---------------------------------//

int
InstrumentSim::SetL00Spacecraft(
	Spacecraft*		spacecraft)
{
	OrbitState* orbit_state = &(spacecraft->orbitState);
	L00Frame* l00_frame = &(l00.frame);

	double alt, lon, lat;
	if (! orbit_state->rsat.GetAltLonGDLat(&alt, &lon, &lat))
		return(0);

	l00_frame->gcAltitude = alt;
	l00_frame->gcLongitude = lon;
	l00_frame->gcLatitude = lat;
	l00_frame->gcX = orbit_state->rsat.get(0);
	l00_frame->gcY = orbit_state->rsat.get(1);
	l00_frame->gcZ = orbit_state->rsat.get(2);
	l00_frame->velX = orbit_state->vsat.get(0);
	l00_frame->velY = orbit_state->vsat.get(1);
	l00_frame->velZ = orbit_state->vsat.get(2);

	return(1);
}

//------------------------------//
// InstrumentSim::SetL00Science //
//------------------------------//

int
InstrumentSim::SetL00Science(
	MeasSpot*		meas_spot,
	Instrument*		instrument)
{
        int total_sliceNumber=_spotNumber*l00.frame.slicesPerSpot;
	L00Frame* l00_frame = &(l00.frame);
	Antenna* antenna = &(instrument->antenna);

	//----------------------//
	// set antenna position //
	//----------------------//

	l00_frame->antennaPosition[_spotNumber] =
		(unsigned short)antenna->GetEncoderValue();

	//-------------------------//
	// for each measurement... //
	//-------------------------//

	for (Meas* meas = meas_spot->slices.GetHead(); meas;
		meas = meas_spot->slices.GetNext())
	{
		//----------------------------//
		// update the level 0.0 frame //
		//----------------------------//

		l00_frame->science[total_sliceNumber] = meas->value;
		total_sliceNumber++;
	}
	_spotNumber++;

	return(1);
}

//------------------------//
// InstrumentSim::ScatSim //
//------------------------//

int
InstrumentSim::ScatSim(
	double			time,
	Spacecraft*		spacecraft,
	Instrument*		instrument,
	WindField*		windfield,
	GMF*			gmf)
{
	MeasSpot meas_spot;

	//-----------------------------//
        // If at the start of a frame  //
	// Compute Frame header info   //
	//-----------------------------//
	if(_spotNumber==0){	
	  if (! SetL00Spacecraft(spacecraft))
	      return(0);
	  l00.frame.time=time;
	  
	}
	//---------------------//
	// locate measurements //
	//---------------------//

	if (l00.frame.slicesPerSpot <= 1)
	{
		if (! LocateSpot(time, spacecraft, instrument, &meas_spot))
			return(0);
	}
	else
	{
		if (! LocateSlices(time, spacecraft, instrument, &meas_spot))
			return(0);
	}

	//------------------------//
	// set measurement values //
	//------------------------//

	if (! SetMeasurements(spacecraft, instrument, &meas_spot, windfield, 
			      gmf))
		return(0);


	//--------------------------------//
	// Add Spot Specific Info to Frame //
	//--------------------------------//

	if (! SetL00Science(&meas_spot, instrument))
		return(0);

	//-----------------------------//
	// determine if frame is ready //
	//-----------------------------//

	Antenna* antenna = &(instrument->antenna);

	if (_spotNumber >= l00.frame.beamCyclesPerFrame * antenna->numberOfBeams)
	{
		l00FrameReady = 1;	// indicate frame is ready
		_spotNumber = 0;	// prepare to start a new frame
	}
	else
	{
		l00FrameReady = 0;	// indicate frame is not ready
	}

	return(1);
}






