//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_instrumentgeom_c[] =
    "@(#) $Id$";

#include <stdio.h>
#include <stdlib.h>
#include "Ephemeris.h"
#include "Attitude.h"
#include "Antenna.h"
#include "GenericGeom.h"
#include "Spacecraft.h"
#include "Meas.h"
#include "InstrumentGeom.h"
#include "Misc.h"
#include "Qscat.h"
#include "Interpolate.h"
#include "Array.h"
#include "BYUXTable.h"

// prototype for AccurateGeom routines
int GetPeakSpectralResponse(CoordinateSwitch* antenna_frame_to_gc,
			    Spacecraft* spacecraft, Qscat* qscat,
			     double* look, double* azimuth);

//------------------//
// AntennaFrameToGC //
//------------------//

CoordinateSwitch
AntennaFrameToGC(
    OrbitState*  sc_orbit_state,
    Attitude*    sc_attitude,
    Antenna*     antenna,
    double       azimuth_angle)
{
    CoordinateSwitch total;

    // geocentric to s/c velocity
    Vector3 sc_xv, sc_yv, sc_zv;
    velocity_frame(sc_orbit_state->rsat, sc_orbit_state->vsat, &sc_xv,
        &sc_yv, &sc_zv);
    CoordinateSwitch gc_to_scv(sc_xv, sc_yv, sc_zv);
    total = gc_to_scv;

    // s/c velocity to s/c body
    CoordinateSwitch scv_to_sc_body(*sc_attitude);
    total.Append(&scv_to_sc_body);

    // s/c body to antenna pedestal
    CoordinateSwitch sc_body_to_ant_ped = antenna->GetScBodyToAntPed();
    total.Append(&sc_body_to_ant_ped);

    // antenna pedestal to antenna frame
    Attitude att;
    att.Set(0.0, 0.0, azimuth_angle, 1, 2, 3);
    CoordinateSwitch ant_ped_to_ant_frame(att);
    total.Append(&ant_ped_to_ant_frame);

    total = total.ReverseDirection();
    return(total);
}

//------------//
// LocateSpot //
//------------//

int
LocateSpot(
    Spacecraft*  spacecraft,
    Qscat*       qscat,
    MeasSpot*    meas_spot,
    float        Esn,
    float        contour_level)
{
	//-----------//
	// predigest //
	//-----------//

	Antenna* antenna = &(qscat->sas.antenna);
	Beam* beam = qscat->GetCurrentBeam();
	OrbitState* orbit_state = &(spacecraft->orbitState);
	Attitude* attitude = &(spacecraft->attitude);

	//------------------//
	// set up meas spot //
	//------------------//

	meas_spot->FreeContents();
	meas_spot->time = qscat->cds.time;
	meas_spot->scOrbitState = *orbit_state;
	meas_spot->scAttitude = *attitude;

	//--------------------------------//
	// generate the coordinate switch //
	//--------------------------------//

	CoordinateSwitch antenna_frame_to_gc = AntennaFrameToGC(orbit_state,
		attitude, antenna, antenna->txCenterAzimuthAngle);

	//---------------------------------------------------//
	// calculate the look vector in the geocentric frame //
	//---------------------------------------------------//

	Vector3 rlook_antenna;

	double look, azim;
	if (! beam->GetElectricalBoresight(&look, &azim))
	{
		fprintf(stderr,"Error determining electrical boresight\n");
		return(0);
	}

	rlook_antenna.SphericalSet(1.0, look, azim);
    ScatTargetInfo sti;
	if (! sti.GetScatTargetInfo(&antenna_frame_to_gc,
        spacecraft->orbitState.rsat, rlook_antenna))
    {
        return(0);
    }

	//
	// Using the round trip time from the one way boresight, compute the
	// direction of the 2 way boresight.  Ideally, this should be iterated,
	// but the round trip time difference should be very small.
	//

	if (! GetPeakSpatialResponse(beam, sti.roundTripTime,
        qscat->sas.antenna.spinRate, &look, &azim))
	{
		fprintf(stderr,"Error determining 2 way electrical boresight\n");
		return(0);
	}

	// Update with new boresight
	rlook_antenna.SphericalSet(1.0, look, azim);
    if (! sti.GetScatTargetInfo(&antenna_frame_to_gc,
        spacecraft->orbitState.rsat, rlook_antenna))
    {
        return(0);
    }

	Vector3 rlook_gc = antenna_frame_to_gc.Forward(rlook_antenna);

	//----------------------------//
	// calculate the 3 dB outline //
	//----------------------------//

	Meas* meas = new Meas();	// need the outline to append to

	// get the max gain value.
	float gp_max;
    if (beam->GetSpatialResponse(look, azim, sti.roundTripTime,
        qscat->sas.antenna.spinRate, &gp_max)!=1)
    {
        fprintf(stderr,"Locate Spot: Cannot compute max gain.\n");
        return(0);
	}

	// Align beam frame z-axis with the electrical boresight.
	Attitude beam_frame;
	beam_frame.Set(0.0, look, azim, 3, 2, 1);
	CoordinateSwitch ant_to_beam(beam_frame);
	CoordinateSwitch beam_to_ant = ant_to_beam.ReverseDirection();

	//
	// In the beam frame, for a set of azimuth angles, search for the
	// theta angle that has the required gain product.
	// Convert the results to the geocentric frame and find
	// the earth intercepts.
	//

	for (int i=0; i < POINTS_PER_SPOT_OUTLINE + 1; i++)
	{
		double phi = (two_pi * i) / POINTS_PER_SPOT_OUTLINE;

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
			theta = (theta_max + theta_min) / 2.0;
			look_mid.SphericalSet(1.0, theta, phi);
			look_mid_ant = beam_to_ant.Forward(look_mid);
			double r, look, azim;
			look_mid_ant.SphericalGet(&r,&look,&azim);
			float gp;
			if (! beam->GetSpatialResponse(look, azim, sti.roundTripTime,
				qscat->sas.antenna.spinRate, &gp))
            {
                return(0);
            }
			if (gp > contour_level * gp_max)
			{
				theta_max = theta;
			}
			else
			{
				theta_min = theta;
			}
		}

		look_mid_gc = antenna_frame_to_gc.Forward(look_mid_ant);
		EarthPosition *rspot = new EarthPosition;
		if(earth_intercept(orbit_state->rsat,look_mid_gc,rspot)!=1)
		  return(0);
		if (! meas->outline.Append(rspot))
		{
			fprintf(stderr,"Error appending to spot outline\n");
			return(0);
		}
	}

	//---------------------------//
	// generate measurement data //
	//---------------------------//

	meas->pol = beam->polarization;

	// get local measurement azimuth
	CoordinateSwitch gc_to_surface = sti.rTarget.SurfaceCoordinateSystem();
	Vector3 rlook_surface = gc_to_surface.Forward(rlook_gc);
	double r, theta, phi;
	rlook_surface.SphericalGet(&r, &theta, &phi);
	meas->eastAzimuth = phi;

	// get incidence angle
	meas->incidenceAngle = sti.rTarget.IncidenceAngle(rlook_gc);
	meas->centroid = sti.rTarget;

	// set energy measurement to be consistent with slice handling
	meas->value = Esn;

	//-----------------------------//
	// add measurment to meas spot //
	//-----------------------------//

	meas_spot->Append(meas);

	return(1);
}

/*
//-------------------//
// RttToIdealRxDelay //
//-------------------//
// Calculates the ideal receiver delay based on the round trip time.

int
RttToIdealRxDelay(
    Qscat*          qscat,
	double			rtt)
{
	double pulse_width = qscat->ses.txPulseWidth;

    SesBeamInfo* ses_beam_info = qscat->GetCurrentSesBeamInfo();
	qscat->ses.rxGateDelay = rtt +
        (pulse_width - ses_beam_info->rxGateWidth) / 2.0;

	return(1);
}
*/



/**********************************************************************
//-------------------------------//
// IdealCommandedDopplerForRange //
//-------------------------------//
// Estimate the ideal commanded doppler frequency.
// for the point on the same azimuth as the peak 2-way gain
// point and a look angle corresponding to the range at the center
// of the range gate.

#define DOPPLER_ACCURACY	1.0		// 1 Hz


int
IdealCommandedDopplerForRange(
    Spacecraft*  spacecraft,
    Qscat*       qscat,
    float        offset)
{
	//------------------------------//
	// zero the spacecraft attitude //
	//------------------------------//

	OrbitState* sc_orbit_state = &(spacecraft->orbitState);
	Attitude zero_rpy;
	zero_rpy.Set(0.0, 0.0, 0.0, 1, 2, 3);
	CoordinateSwitch zero_rpy_antenna_frame_to_gc =
		AntennaFrameToGC(sc_orbit_state, &zero_rpy, &(qscat->sas.antenna));

	//-------------------------------------------//
	// find the current beam's two-way peak gain //
	//-------------------------------------------//

	Beam* beam = qscat->GetCurrentBeam();
	double azimuth_rate = qscat->sas.antenna.spinRate;
	double look, azim;
	if (! GetPeakSpatialResponse2(&zero_rpy_antenna_frame_to_gc, spacecraft,
        beam, azimuth_rate, &look, &azim))
	{
		return(0);
	}

    //-----------------------------------------------------//
    // compute look angle corresponding to the given range //
    //-----------------------------------------------------//

    double pulse_width = qscat->ses.txPulseWidth;
    SesBeamInfo* ses_beam_info = qscat->GetCurrentSesBeamInfo();
    double cmd_rtt = qscat->ses.rxGateDelay -
        (pulse_width - ses_beam_info->rxGateWidth) / 2.0;
    double cmd_range = cmd_rtt*speed_light_kps/2.0;

    double bracket=5;

    double start_look=0;
    double end_look=0;
    double start_range=0;
    double end_range=0;
	Vector3 vector;
	TargetInfoPackage tip;

    //--------------------------------------------//
    // make sure desired look_angle is bracketted //
    //--------------------------------------------//

    while(cmd_range<=start_range || cmd_range>=end_range)
    {
        start_look=look-bracket*dtr;
        end_look=look+bracket*dtr;
        vector.SphericalSet(1.0, start_look, azim);
        TargetInfo(&zero_rpy_antenna_frame_to_gc, spacecraft, qscat, vector,
            &tip);
        start_range=tip.slantRange;
        vector.SphericalSet(1.0, end_look, azim);
        TargetInfo(&zero_rpy_antenna_frame_to_gc, spacecraft, qscat, vector,
            &tip);
        end_range=tip.slantRange;
        bracket+=5;
    }

    //--------------------------------------------//
    // Perform Binary Search for Desired Look     //
    // to 100m tolerance in range                 //
    //--------------------------------------------//

    double new_look=0, new_range=0;
    while(fabs(cmd_range-new_range)>0.1)
    {
        new_look=(start_look+end_look)/2;
        vector.SphericalSet(1.0, new_look, azim);
        TargetInfo(&zero_rpy_antenna_frame_to_gc, spacecraft, qscat, vector,
            &tip);
        new_range=tip.slantRange;
        if (new_range< cmd_range)
            start_look=new_look;
        else
            end_look=new_look;
	}

    look=new_look;

	//----------------------------//
	// calculate commanded Doppler //
	//----------------------------//


	vector.SphericalSet(1.0, look, azim);
	instrument->SetCommandedDoppler(0.0);
	do
	{
		TargetInfo(&zero_rpy_antenna_frame_to_gc, spacecraft, qscat,
            instrument, vector, &tip);
		float freq = instrument->commandedDoppler + tip.basebandFreq;
		instrument->SetCommandedDoppler(freq);
	} while (fabs(tip.basebandFreq) > DOPPLER_ACCURACY);


	//---------------------------------------//
        // Add Offset to Commanded Doppler       //
        //---------------------------------------//
        float freq = instrument->commandedDoppler + offset;
        instrument->SetCommandedDoppler(freq);

	return(1);
}
************************************************************/

//------------------------//
// GetPeakSpatialResponse //
//------------------------//

//
// This function locates the maximum spatial response of a beam in the
// antenna frame,
// and returns the apparent direction. (look,azimuth (rads)).
// NOTE: This is NOT the direction of the transmitted pulse which determines
// the scattering geometry.
//

#define SPATIAL_RESPONSE_ANGLE_TOLERANCE    1e-6

int
GetPeakSpatialResponse(
    Beam*    beam,
    double   round_trip_time,
    double   azimuth_rate,
    double*  look,
    double*  azim,
    int      ignore_range)
{
    int ndim = 2;
    double** p = (double**)make_array(sizeof(double), 2, 3, 4);
    if (p == NULL)
    {
        printf("Error allocating memory in GetPeakSpatialResponse\n");
        return(0);
    }

    p[0][0] = *look;
    p[0][1] = *azim;
    p[1][0] = *look + 1.0*dtr;
    p[1][1] = *azim;
    p[2][0] = *look;
    p[2][1] = *azim + 1.0*dtr;

    for (int i = 0; i < ndim+1; i++)
    {
        p[i][2] = round_trip_time;
        p[i][3] = azimuth_rate;
    }

    char* ptr[2];
    ptr[0] = (char*)beam;
    int* flag_ptr = &ignore_range;
    ptr[1] = (char*)flag_ptr;
    double ftol = SPATIAL_RESPONSE_ANGLE_TOLERANCE;
    downhill_simplex((double**)p, ndim, ndim+2, ftol,
        NegativeSpatialResponse, ptr);

    *look = p[0][0];
    *azim = p[0][1];

    free_array(p, 2, 3, 4);

    return(1);
}

//--------------------------//
// NegativeSpatialResponse //
//--------------------------//

//
// NegativeSpatialResponse is the function to be minimized by
// GetPeakSpatialResponse.
// It computes the negative of the SpatialResponse for the inputs given
// in the input vector.  The elements of the input vector are:
//
// x[0] = look angle (rad)
// x[1] = azimuth angle (rad)
// x[2] = round trip time (sec)
// x[3] = spin rate (rad/sec)
// beam = pointer to a beam object containing the pattern to use.
// ignore_range = flag which when set to one commands routine to compute
//             the PowerGainProduct instead of the SpatialResponse

double
NegativeSpatialResponse(
	double*		x,
	void*		ptr)
{
        double response;
	char** ptr2=(char**)ptr;
        Beam* beam=(Beam*)ptr2[0];
        int ignore_range=*(int*)(ptr2[1]);

	//-------------------------------------------------//
	// Calculate Two-way Gain Only if ignore_range = 1 //
	//-------------------------------------------------//

    if (ignore_range)
    {
        if(beam->GetPowerGainProduct(x[0],x[1],x[2],x[3],&response)!=1)
        {
            fprintf(stderr,"Error:NegativeSpatialResponse function failed\n");
            exit(1);
        }
	}
    else
    {
        if(beam->GetSpatialResponse(x[0],x[1],x[2],x[3],&response)!=1)
        {
            fprintf(stderr,"Error:NegativeSpatialResponse function failed\n");
            exit(1);
        }
	}

	return(-response);
}
