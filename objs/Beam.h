//==============================================================//
// Copyright (C) 1997-2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#ifndef BEAM_H
#define BEAM_H

static const char rcs_id_beam_h[] =
    "@(#) $Id$";

#include "CoordinateSwitch.h"

//======================================================================
// CLASSES
//    Beam
//======================================================================

//======================================================================
// CLASS
//    Beam
//
// DESCRIPTION
//    The Beam object contains beam state information and fixed
//    coordinate transforms related to the beam "mounting" on the
//    antenna.  The beam antenna pattern is also held by this object.
//    The beam pattern has to be loaded from an external file.
//
// NAME CONVENTIONS
//    To aid in identifying the reference frame an angle is defined in,
//    the following conventions are followed in here and in Beam.C:
//    look_angle and azimuth_angle pairs refer to standard spherical
//    angles in the antenna frame.
//    elevation or Em and azimuth or Am pairs refer to the modifed
//    spherical angles used to access the beam pattern in the beam
//    reference frame.
//======================================================================

enum PolE { V_POL=0, H_POL=1, NONE };

extern const char* beam_map[];

class Beam
{
public:

    //--------------//
    // construction //
    //--------------//

    Beam();
    Beam(const Beam& from);
    ~Beam();

    // Get and Set Beams separately.
    int  GetElectricalBoresight(double* look_angle, double* azimuth_angle);
    int  SetElectricalBoresight(double look_angle, double azimuth_angle);

    // Set mechanical reference (beam directions determined by pattern data)
    // Use the same inputs for each beam to have a consistent reference.
    int  SetMechanicalBoresight(double look_angle, double azimuth_angle);

    int  SetBeamPattern(int Nx, int Ny, int ix_zero, int iy_zero,
             double x_spacing, double y_spacing,
             double electrical_boresight_Em, double electrical_boresight_Am,
             float** power_gain);

    int  ReadBeamPattern(const char* filename);
    int  WriteBeamPattern(const char* filename);

    int  GetPowerGain(double look_angle, double azimuth_angle, float* gain);
    int  GetPowerGain(double look_angle, double azimuth_angle, double* gain);
    int  GetPowerGainProduct(double look_angle, double azimuth_angle,
             double round_trip_time, double azimuth_rate, float* gain_product);
    int  GetPowerGainProduct(double look_angle, double azimuth_angle,
             double round_trip_time, double azimuth_rate,
             double* gain_product);

    int  GetSpatialResponse(double look_angle, double azimuth_angle,
              double round_trip_time, double azimuth_rate, float* response);
    int  GetSpatialResponse(double look_angle, double azimuth_angle,
              double round_trip_time, double azimuth_rate, double* response);

    //-----------//
    // Operators //
    //-----------//

    Beam& operator=(const Beam& from);

    //-----------//
    // variables //
    //-----------//

    int     silentFlag;    // don't report power gain problems
    PolE    polarization;
    double  _elecBoresightLook;    // in the antenna frame
    double  _elecBoresightAzim;

    // coordinate swith from antenna frame to beam measurement frame
    CoordinateSwitch  _antennaFrameToBeamFrame;

    // Beam pattern info
    double   _electrical_boresight_Em;
    double   _electrical_boresight_Am;
    int      _Nx;
    int      _Ny;
    int      _ix_zero;
    int      _iy_zero;
    double   _x_spacing;
    double   _y_spacing;
    float**  _power_gain;
    float    peakGain;
};

#endif
