//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_qscatsimaccurate_c[] =
	"@(#) $Id$";

#include "GenericGeom.h"
#include "InstrumentGeom.h"
#include "Ephemeris.h"
#include "Sigma0.h"
#include "Constants.h"
#include "QscatSimAccurate.h"
#include "AccurateGeom.h"

//==================//
// QscatSimAccurate //
//==================//

QscatSimAccurate::QscatSimAccurate()
{
	return;
}

QscatSimAccurate::~QscatSimAccurate()
{
	return;
}

//-----------------------------------//
// QscatSimAccurate::SetMeasurements //
//----------------------------------//

int
QscatSimAccurate::SetMeasurements(
    Qscat*       qscat,
    MeasSpot*    meas_spot,
    WindField*   windfield,
    GMF*         gmf)
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

		float sigma0;
		if (uniformSigmaField) sigma0=1;
                else{
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


		  gmf->GetInterpolatedValue(meas->pol, meas->incidenceAngle, wv.spd,
					    chi, &sigma0);
		}

		//--------------------------//
		// convert sigma-0 to power //
		//--------------------------//
		// meas->value hold the GA/R^4 integral

		double lambda = speed_light_kps / qscat->ses.txFrequency;
		double constants = qscat->ses.transmitPower * qscat->ses.rxGainEcho;
		constants *= lambda*lambda / (64*pi*pi*pi*qscat->systemLoss);
		meas->XK = meas->value*constants;
		meas->value *= sigma0;
	}
	return(1);
}

//---------------------------//
// QscatSimAccurate::ScatSim //
//---------------------------//

int
QscatSimAccurate::ScatSim(
    Spacecraft*  spacecraft,
    Qscat*       qscat,
    WindField*   windfield,
    GMF*         gmf,
    L1AFrame*    l1a_frame)
{
	MeasSpot meas_spot;

	//-----------------------------//
	// If at the start of a frame //
	// Compute Frame header info //
	//-----------------------------//

	if (_spotNumber == 0)
	{
		if (! SetL1ASpacecraft(spacecraft,l1a_frame))
			return(0);
		l1a_frame->time = qscat->cds.time;
	}
	//------------------------//
	// calculate measurements //
	//------------------------//

	if (qscat->ses.scienceSlicesPerSpot <= 1)
	{
		fprintf(stderr,"Sorry no eggs!!!\n");
		return(0);
	}
	else
	{

		if (! IntegrateSlices(spacecraft, qscat, &meas_spot,
            numLookStepsPerSlice, azimuthIntegrationRange, azimuthStepSize))
		{
			return(0);
		}
		if (outputXToStdout)
            printf("%g ", qscat->sas.antenna.txCenterAzimuthAngle / dtr);
	}

	//------------------------//
	// set measurement values //
	//------------------------//

	if (! SetMeasurements(qscat, &meas_spot, windfield, gmf))
		return(0);

        //-----------------------------------------------//
        //-------- Output X to Stdout if enabled--------//
        //-----------------------------------------------//

	if(outputXToStdout)
	{
		for(Meas* slice=meas_spot.GetHead(); slice; slice=meas_spot.GetNext())
		{
			printf("%g ",slice->XK);
		}
		if(qscat->cds.currentBeamIdx==1)
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
			float orbit_position = qscat->cds.OrbitFraction();

			if(! xTable.AddEntry(slice->XK, qscat->cds.currentBeamIdx,
               qscat->sas.antenna.azimuthAngle, orbit_position, sliceno))
            {
                return(0);
            }

			sliceno++;
		}

	}
	//--------------------------------//
	// Add Spot Specific Info to Frame //
	//--------------------------------//

	// No check data allowed.
	if (! SetL1AScience(&meas_spot, NULL, qscat, l1a_frame))
		return(0);

	//-----------------------------//
	// determine if frame is ready //
	//-----------------------------//

	if (_spotNumber >= l1a_frame->spotsPerFrame)
	{
		l1aFrameReady = 1;	// indicate frame is ready
		_spotNumber = 0;	// prepare to start a new frame
	}
	else
	{
		l1aFrameReady = 0;	// indicate frame is not ready
	}

	return(1);
}









