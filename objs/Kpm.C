//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_kpm_c[] =
    "@(#) $Id$";

#include <stdio.h>
#include <malloc.h>
#include "Kpm.h"
#include "Constants.h"
#include "Array.h"

//=====//
// Kpm //
//=====//

Kpm::Kpm()
:   _metCount(0), _table(NULL)
{
    return;
}

Kpm::~Kpm()
{
    _Deallocate();
    return;
}

//----------------//
// Kpm::ReadTable //
//----------------//

int
Kpm::ReadTable(
    const char*  filename)
{
    FILE* fp = fopen(filename, "r");
    if (fp == NULL)
        return(0);

    if (! _ReadHeader(fp))
        return(0);

    _metCount = 2;    // just force this for now

    if (! _Allocate())
        return(0);

    if (! _ReadTable(fp))
        return(0);

    fclose(fp);
    return(1);
}

//-----------------//
// Kpm::WriteTable //
//-----------------//

int
Kpm::WriteTable(
    const char*  filename)
{
    FILE* fp = fopen(filename, "w");
    if (fp == NULL)
        return(0);

    if (! _WriteHeader(fp))
        return(0);

    if (! _WriteTable(fp))
        return(0);

    fclose(fp);
    return(1);
}

//-------------//
// Kpm::GetKpm //
//-------------//

int
Kpm::GetKpm(
    Meas::MeasTypeE  meas_type,
    float            speed,
    double*          kpm)
{
    //---------------------------------------------//
    // determine coefficients for measurement type //
    //---------------------------------------------//

    int met_idx;
    switch (meas_type)
    {
    case Meas::VV_MEAS_TYPE:
        met_idx = 0;
        break;
    case Meas::HH_MEAS_TYPE:
        met_idx = 1;
        break;
    case Meas::VH_MEAS_TYPE:
    case Meas::HV_MEAS_TYPE:
    case Meas::VV_HV_CORR_MEAS_TYPE:
    case Meas::HH_VH_CORR_MEAS_TYPE:
        //-----------------------//
        // this is a total guess //
        //-----------------------//
        *kpm = 0.1749;    // 0.7 dB
        return(1);
        break;
    default:
        return(0);
        break;
    }

    //----------------------------------//
    // determine coefficients for speed //
    //----------------------------------//

    int idx[2];
    float coef[2];

    if (! _speedIdx.GetLinearCoefsClipped(speed, idx, coef))
        return(0);

    //------------------------//
    // interpolate to get Kpm //
    //------------------------//

    float* spd_table = *(_table + met_idx);
    *kpm = *(spd_table + idx[0]) * coef[0] +
        *(spd_table + idx[1]) * coef[1];

    return(1);
}

//----------------//
// Kpm::_Allocate //
//----------------//

int
Kpm::_Allocate()
{
    _table = (float **)make_array(sizeof(float), 2, _metCount,
        _speedIdx.GetBins());
    if (_table == NULL)
        return(0);
    return(1);
}

//------------------//
// Kpm::_Deallocate //
//------------------//

int
Kpm::_Deallocate()
{
    if (_table == NULL)
        return(1);

    free_array((void *)_table, 2, _metCount, _speedIdx.GetBins());

    _table = NULL;
    return(1);
}

//------------------//
// Kpm::_ReadHeader //
//------------------//

int
Kpm::_ReadHeader(
    FILE*  fp)
{
    if (! _speedIdx.Read(fp))
        return(0);
    return(1);
}

//-------------------//
// Kpm::_WriteHeader //
//-------------------//

int
Kpm::_WriteHeader(
    FILE*  fp)
{
    if (! _speedIdx.Write(fp))
        return(0);
    return(1);
}

//-----------------//
// Kpm::_ReadTable //
//-----------------//

int
Kpm::_ReadTable(
    FILE*  fp)
{
    //----------------//
    // read the array //
    //----------------//

    unsigned int bins = _speedIdx.GetBins();
    for (int i = 0; i < _metCount; i++)
    {
        if (fread((void *)*(_table + i), sizeof(float), bins, fp) != bins)
        {
            return(0);
        }
    }
    return(1);
}

//------------------//
// Kpm::_WriteTable //
//------------------//

int
Kpm::_WriteTable(
    FILE*  fp)
{
    //-----------------//
    // write the array //
    //-----------------//

    unsigned int bins = _speedIdx.GetBins();
    for (int i = 0; i < _metCount; i++)
    {
        if (fwrite((void *)*(_table + i), sizeof(float), bins, fp) != bins)
        {
            return(0);
        }
    }
    return(1);
}

//==========//
// KpmField //
//==========//

KpmField::KpmField()
{
    _gaussianRv.SetMean(0.0);
    _gaussianRv.SetVariance(1.0);
    _corrLength = 0.0;
    return;
}

KpmField::~KpmField()
{
    return;
}

//----------------------//
// KpmField::Build
//----------------------//

// corr_length should be in km.

int
KpmField::Build(
    float  corr_length)
{
    _corrLength = corr_length;
    if (_corrLength < 0.0)
    {
        printf("Error: KpmField received a negative correlation length\n");
        exit(-1);
    }
    else if (_corrLength == 0.0)
    {    // With no correlation, on the fly gaussian rv's are supplied.
        corr.Deallocate();
        uncorr.Deallocate();
        return(1);
    }

    //--------------------------------------------//
    // Configure field sizes.
    // This will destroy any pre-existing fields.
    //--------------------------------------------//

    // Set step sizes to a fraction of a correlation length at the equator.
    float lonlat_step = corr_length/STEPS_PER_CORRLENGTH / r1_earth;

    corr.Setup(0.0,two_pi,lonlat_step,-pi/2.0,pi/2.0,lonlat_step);
    uncorr.Setup(0.0,two_pi,lonlat_step,-pi/2.0,pi/2.0,lonlat_step);

    //-----------------------------//
    // Allocate fields
    //-----------------------------//

    if (!corr.Allocate() || !uncorr.Allocate())
    {
        printf("Error allocating fields in KpmField::Build\n");
        return(0);
    }

    //-----------------------------//
    // Fill uncorrelated field
    //-----------------------------//

    int Nlon,Nlat;
    int i,j;
    uncorr.GetDimensions(&Nlon,&Nlat);
    printf("Field sizes: %d by %d\n",Nlon,Nlat);

    for (i=0; i < Nlon; i++)
    for (j=0; j < Nlat; j++)
    {
        uncorr.field[i][j] = _gaussianRv.GetNumber();
    }

    //----------------------------------//
    // Compute correlated field.
    // Uses a brute force convolution.
    //----------------------------------//

    double denom = _corrLength*_corrLength/2.0;
    double lat_rad_to_km = (r1_earth + r2_earth)/2.0;
    double latitude,lon_rad_to_km;

    int ip,jp,ipmin,ipmax,jpmin,jpmax;
    for (i=0; i < Nlon; i++)
    {
        printf("corr field: lon = %g\n",i*lonlat_step*rtd);
        for (j=0; j < Nlat; j++)
        {
            latitude = j*lonlat_step;
            lon_rad_to_km = lat_rad_to_km * cos(latitude);
            ipmin = i - STEPS_PER_CORRLENGTH*N_CORRLENGTHS_INTEGRATE;
            ipmax = i + STEPS_PER_CORRLENGTH*N_CORRLENGTHS_INTEGRATE;
            jpmin = j - STEPS_PER_CORRLENGTH*N_CORRLENGTHS_INTEGRATE;
            jpmax = j + STEPS_PER_CORRLENGTH*N_CORRLENGTHS_INTEGRATE;

            if (ipmin < 0) ipmin = 0;
            if (ipmax > Nlon) ipmax = Nlon;
            if (jpmin < 0) jpmin = 0;
            if (jpmax > Nlat) jpmax = Nlat;

            for (ip=ipmin; ip < ipmax; ip++)
            for (jp=jpmin; jp < jpmax; jp++)
            {
                // Crude but fast distance calculation. (short distances only)
                // This method will suffer from larger errors near the poles.
                double lon_step = lonlat_step*(ip - i)*lon_rad_to_km;
                double lat_step = lonlat_step*(jp - j)*lat_rad_to_km;
                double r2 = lon_step*lon_step + lat_step*lat_step;
                // Convolution sum.
                corr.field[i][j] += exp(-r2 / denom) * uncorr.field[ip][jp];
            }
        }
    }

    //-----------------------------------------------//
    // Ditch uncorrelated field to save memory.
    //-----------------------------------------------//

    uncorr.Deallocate();

    //-----------------------------------------------//
    // Scale correlated field to have unit variance.
    //-----------------------------------------------//

    float var = corr.GetVariance();
    corr.Scale(1.0/sqrt(var));

    return(1);
}

//-----------------//
// KpmField::GetRV //
//-----------------//

float
KpmField::GetRV(
    Kpm*             kpm,
    Meas::MeasTypeE  meas_type,
    float            wspd,
    LonLat           lon_lat)
{
    double kpm_value;
    if (! kpm->GetKpm(meas_type, wspd, &kpm_value))
    {
        fprintf(stderr, "KpmField::GetRV: I can't handle bad Kpm values!\n");
        exit(1);
    }

    return(GetRV(kpm_value, lon_lat));
}

float
KpmField::GetRV(
    double  kpm_value,
    LonLat  lon_lat)
{
    float RV;
    float rv1;

    if (! corr.field)
    {    // no spatial correlation, so just draw a gaussian random number
        rv1 = _gaussianRv.GetNumber();
    }
    else
    {
        if (corr.InterpolatedElement(lon_lat,&rv1) == 0)
        {
            printf("Error getting correlated rv in KpmField::GetRV\n");
            exit(-1);
        }
    }

    // Scale for unit mean, variance = Kpm^2.
    // Note that kpm_value is unnormalized standard deviation.
    RV = rv1*kpm_value + 1.0;

    if (RV < 0.0)
    {
        RV = 0.0;    // Do not allow negative sigma0's.
    }

    return(RV);
}
