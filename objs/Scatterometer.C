//=========================================================//
// Copyright (C) 1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

static const char rcs_id_scatterometer_c[] =
    "@(#) $Id$";

#include "Scatterometer.h"
#include "InstrumentGeom.h"
#include "GenericGeom.h"

//================//
// ScatTargetInfo //
//================//

//-----------------------------------//
// ScatTargetInfo::GetScatTargetInfo //
//-----------------------------------//
// Calculates the target position vector, slant range, and round trip time

int
ScatTargetInfo::GetScatTargetInfo(
    CoordinateSwitch*  antenna_frame_to_gc,
    EarthPosition      rsat,
    Vector3            vector)
{
    // Compute earth intercept point and range.
    Vector3 ulook_gc = antenna_frame_to_gc->Forward(vector);
    if (earth_intercept(rsat, ulook_gc, &rTarget) != 1)
        return(0);
    slantRange = (rsat - rTarget).Magnitude();
    roundTripTime = 2.0 * slantRange / speed_light_kps;

    return(1);
}

//========//
// ScatRF //
//========//

ScatRF::ScatRF()
{
    return;
}

ScatRF::~ScatRF()
{
    return;
}

//=========//
// ScatDig //
//=========//

ScatDig::ScatDig()
:   currentBeamIdx(0), time(0.0)
{
    return;
}

ScatDig::~ScatDig()
{
    return;
}

//=========//
// ScatAnt //
//=========//

ScatAnt::ScatAnt()
{
    return;
}

ScatAnt::~ScatAnt()
{
    return;
}

//===============//
// Scatterometer //
//===============//

Scatterometer::Scatterometer()
{
    return;
}

Scatterometer::~Scatterometer()
{
    return;
}

//-------------------------------//
// Scatterometer::GetCurrentBeam //
//-------------------------------//

Beam*
Scatterometer::GetCurrentBeam()
{
    return(&(scatAnt->antenna.beam[scatDig->currentBeamIdx]));
}

//---------------------------//
// Scatterometer::LocateSpot //
//---------------------------//

int
Scatterometer::LocateSpot(
    Spacecraft*  spacecraft,
    MeasSpot*    meas_spot,
    float        Esn,
    float        contour_level)
{
    //-----------//
    // predigest //
    //-----------//

    Antenna* antenna = &(scatAnt->antenna);
    Beam* beam = GetCurrentBeam();
    OrbitState* orbit_state = &(spacecraft->orbitState);
    Attitude* attitude = &(spacecraft->attitude);

    //------------------//
    // set up meas spot //
    //------------------//

    meas_spot->FreeContents();
    meas_spot->time = scatDig->time;
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
        scatAnt->antenna.spinRate, &look, &azim))
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

    Meas* meas = new Meas();    // need the outline to append to

    // get the max gain value.
    float gp_max;
    if (beam->GetSpatialResponse(look, azim, sti.roundTripTime,
        scatAnt->antenna.spinRate, &gp_max)!=1)
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
        {   // Bisection search
            theta = (theta_max + theta_min) / 2.0;
            look_mid.SphericalSet(1.0, theta, phi);
            look_mid_ant = beam_to_ant.Forward(look_mid);
            double r, look, azim;
            look_mid_ant.SphericalGet(&r,&look,&azim);
            float gp;
            if (! beam->GetSpatialResponse(look, azim, sti.roundTripTime,
                scatAnt->antenna.spinRate, &gp))
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

//------------------------//
// GetPeakSpatialResponse2 //
//------------------------//

int
GetPeakSpatialResponse2(
    CoordinateSwitch*   antenna_frame_to_gc,
    Spacecraft*         spacecraft,
    Beam*               beam,
    double              azimuth_rate,
    double*             look,
    double*             azim,
    int                 ignore_range)
{
    //---------------------------------------------//
    // start with the one-way electrical boresight //
    //---------------------------------------------//

    if (! beam->GetElectricalBoresight(look, azim))
        return(0);

    Vector3 rlook_antenna;
    rlook_antenna.SphericalSet(1.0, *look, *azim);
    ScatTargetInfo sti;
    if (! sti.GetScatTargetInfo(antenna_frame_to_gc,
        spacecraft->orbitState.rsat, rlook_antenna))
    {
        return(0);
    }

    //---------------------------//
    // get the two-way peak gain //
    //---------------------------//

    if (! GetPeakSpatialResponse(beam, sti.roundTripTime, azimuth_rate,
        look, azim, ignore_range))
    {
        return(0);
    }

    return(1);
}
