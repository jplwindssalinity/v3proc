//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_instrumentsimaccurate_c[] =
	"@(#) $Id$";

#include "InstrumentSim.h"
#include "Instrument.h"
#include "GenericGeom.h"
#include "InstrumentGeom.h"
#include "Ephemeris.h"
#include "Sigma0.h"
#include "Constants.h"
#include "InstrumentSimAccurate.h"
#include "AccurateGeom.h"


//=======================//
// InstrumentSimAccurate //
//=======================//

InstrumentSimAccurate::InstrumentSimAccurate()
  : numLookStepsPerSlice(0), azimuthIntegrationRange(0.0), azimuthStepSize(0.0)
{
	return;
}

InstrumentSimAccurate::~InstrumentSimAccurate()
{
	return;
}

//----------------------------------------//
// InstrumentSimAccurate::SetMeasurements //
//---------------------------------------//

int
InstrumentSimAccurate::SetMeasurements(
	Instrument*		instrument,
	MeasSpot*		meas_spot,
	WindField*		windfield,
	GMF*			gmf)
{
	//-------------------------//
	// for each measurement... //
	//-------------------------//

	for (Meas* meas = meas_spot->GetHead(); meas;
		meas = meas_spot->GetNext())
	{
		//----------------------------------------//
		// get lon and lat for the earth location //
		//----------------------------------------//

		double alt, lat, lon;
		if (! meas->centroid.GetAltLonGDLat(&alt, &lon, &lat))
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
		if (uniformSigmaField) sigma0=1;
                else{
		  gmf->GetInterpolatedValue(meas->pol, meas->incidenceAngle, wv.spd,
			chi, &sigma0);
		}



		//--------------------------//
		// convert sigma-0 to power //
		//--------------------------//
		/**** meas->value hold the GA/R^4 integral ****/

		double lambda = speed_light_kps / instrument->transmitFreq;
		double constants =instrument->transmitPower*
			instrument->echo_receiverGain;
		constants*=lambda*lambda/(64*pi*pi*pi*instrument->systemLoss);
		meas->value*=sigma0*constants;
	}
	return(1);
}

//--------------------------------//
// InstrumentSimAccurate::ScatSim //
//--------------------------------//

int
InstrumentSimAccurate::ScatSim(
	Spacecraft*		spacecraft,
	Instrument*		instrument,
	WindField*		windfield,
	GMF*			gmf,
	L00Frame*		l00_frame)
{
	MeasSpot meas_spot;

	//-----------------------------//
	// If at the start of a frame //
	// Compute Frame header info //
	//-----------------------------//

	if (_spotNumber == 0)
	{
		if (! SetL00Spacecraft(spacecraft,l00_frame))
			return(0);
		l00_frame->time = instrument->time;
	}
	//------------------------//
	// calculate measurements //
	//------------------------//

	if (instrument->scienceSlicesPerSpot <= 1)
	{
		fprintf(stderr,"Sorry no eggs!!!\n");
		return(0);
	}
	else
	{

		if (! IntegrateSlices(spacecraft, instrument, &meas_spot,
				      numLookStepsPerSlice,azimuthIntegrationRange,
				      azimuthStepSize))
		{
			return(0);
		}
		if(outputPrToStdout) printf("%g ",instrument->antenna.azimuthAngle/dtr);
	}

	//------------------------//
	// set measurement values //
	//------------------------//



	if (! SetMeasurements(instrument, &meas_spot, windfield, gmf))
		return(0);


        //-----------------------------------------------//
        //-------- Output Pr to Stdout if enabled--------//
        //-----------------------------------------------//

	if(outputPrToStdout)
	{
		for(Meas* slice=meas_spot.GetHead(); slice; slice=meas_spot.GetNext())
		{
			printf("%g ",slice->value);
		}
		if(instrument->antenna.currentBeamIdx==1)
			printf("\n");
	}

        //-----------------------------------------------//
        //  Output X values to X table if enabled        //
        //-----------------------------------------------//

	if(createXtable)
	{
	        int sliceno=0;
		for(Meas* slice=meas_spot.GetHead(); slice; slice=meas_spot.GetNext())
		{
			if(!xTable.AddEntry(slice->value,
					instrument->antenna.currentBeamIdx,
					instrument->antenna.azimuthAngle,
					sliceno)) return(0);

			sliceno++;
		}

	}
	//--------------------------------//
	// Add Spot Specific Info to Frame //
	//--------------------------------//

	if (! SetL00Science(&meas_spot, instrument, l00_frame))
		return(0);

	//-----------------------------//
	// determine if frame is ready //
	//-----------------------------//

	if (_spotNumber >= l00_frame->spotsPerFrame)
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






