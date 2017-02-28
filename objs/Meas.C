//==============================================================//
// Copyright (C) 1997-2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_measurement_c[] =
    "@(#) $Id$";

#include <assert.h>
#include <stdio.h>
#include <unistd.h>

#include "L1BHdf.h"
#include "L2AHdf.h"
#include "Meas.h"
#include "InstrumentGeom.h"
#include "Beam.h"
#include "Constants.h"
#include "LonLat.h"
#include "GenericGeom.h"
#include "Parameter.h"
#include "ETime.h"
#include "Array.h"
#include "Misc.h"

#ifndef IS_EVEN
#define IS_EVEN(x) (x % 2 == 0 ? 1 : 0)
#endif

#ifndef THE_OTHER_BEAM
#define THE_OTHER_BEAM(x) ((x & 0x00000001) ^ 0x00000001)
#endif

#define START_SLICE_INDEX    -4

//======//
// Meas //
//======//

const char* meas_type_map[] = { "None", "VV", "HH", "VH", "HV", "VVHV",
				"HHVH", "CVV", "CHH", NULL };

Meas::Meas()
:   value(0.0), XK(0.0), EnSlice(0.0), bandwidth(0.0), txPulseWidth(0.0),
    landFlag(0), measType(NONE), eastAzimuth(0.0), incidenceAngle(0.0),
    beamIdx(-1), startSliceIdx(-1), numSlices(0), scanAngle(0.0), A(0.0),
    B(0.0), C(0.0), offset(0), azimuth_width(25.0), range_width(7.0)
{
    return;
}

Meas::~Meas()
{
    return;
}

int Meas::CompositeObsKP(MeasList* meas_list) {
    double sum_Ps = 0.0;
    double sum_Ps2 = 0.0;
    double sum_XK = 0.0;
    double sum_EnSlice = 0.0;
    double sum_bandwidth = 0.0;
    Vector3 sum_centroid(0.0,0.0,0.0);
    double sum_incidenceAngle = 0.0;
    double sum_xk2a = 0.0;
    double sum_xkbwb = 0.0;
    double sum_bw2c = 0.0;
    double sum_cos_azi_X = 0.0;
    double sum_sin_azi_X = 0.0;

    int N = 0;
    //
    // Using X in place of Ps when compositing Kpc assumes that sigma0 is
    // uniform across the composite cell area.  This assumption is used
    // below because we sum X^2 instead of Ps^2.
    // We actually use XK which subsumes the K-factor with X.
    //

    Meas* meas;

    // BUG fix BWS Feb 10 2014 before this meas_start was uninitialized
    Meas* meas_start=meas_list->GetHead();

    int min_slice_idx = meas_start->startSliceIdx;
    landFlag = 0;
    
    int prev_slice_idx = -9999;
    for (meas = meas_start; meas; meas = meas_list->GetNext())
    {
        //-------------------------------------------------//
        // Ensure we do not composite over missing slices. // AGF 11/1/2010
        //-------------------------------------------------// Assumes slice-ordered L1B file....
        if( meas != meas_start && meas->startSliceIdx != prev_slice_idx + 1 ) {
          fprintf(stderr,
                  "Meas::Composite: Non-consecutive slices (start, prev, curr): %d %d %d\n",
                  min_slice_idx, prev_slice_idx, meas->startSliceIdx );
          return(0);
        }
        // Stupid HACK----------------------------------------------------------
        // This assumes that there are an even number of slices, thus none of
        // then have a relative slice index of zero.
        // Either this or abandon the relative slice indexing throughout.
        prev_slice_idx = meas->startSliceIdx;
        if( prev_slice_idx == -1 ) prev_slice_idx = 0;
        // End Stupid HACK------------------------------------------------------
        
        //-------------------------------------------//
        // Don't composite HHVH or VVHV measurements //
        //-------------------------------------------//

        if (meas->measType == VV_HV_CORR_MEAS_TYPE ||
            meas->measType == HH_VH_CORR_MEAS_TYPE)
        {
            return(0);
        }

        // Check for land--tourtured logic required since landFlag is signed!
        if( meas->landFlag == 1 || meas->landFlag == 3 )
          if( landFlag == 0 || landFlag == 2 )
            landFlag += 1;
        // Check for ice        
        if( meas->landFlag == 2 || meas->landFlag == 3 )
          if( landFlag == 0 || landFlag == 1 )
            landFlag += 2;
        
        
        sum_Ps += (double)meas->value * (double)meas->XK;
        sum_Ps2+= pow((double)meas->value,2) * pow((double)meas->XK,2);
        sum_XK += (double)meas->XK;
        sum_EnSlice += (double)meas->EnSlice;
        sum_bandwidth += (double)meas->bandwidth;
        sum_centroid += meas->centroid;
        sum_incidenceAngle += (double)meas->XK * (double)meas->incidenceAngle;
        sum_sin_azi_X += (double)meas->XK * (double)sin( meas->eastAzimuth );
        sum_cos_azi_X += (double)meas->XK * (double)cos( meas->eastAzimuth );
        N++;
    }

    meas = meas_list->GetHead();

    //---------------------------------------------------------------------         
    // Form the composite measurement from appropriate combinations of the
    // elements of each slice measurement in this composite cell.
    //---------------------------------------------------------------------

    value = sum_Ps / sum_XK;

    XK = sum_XK;
    EnSlice = sum_EnSlice;
    bandwidth = sum_bandwidth;
    txPulseWidth = meas->txPulseWidth;
    outline.FreeContents();    // merged outlines not done yet
    // Weighted average of centroids (weighted by XK)
    //centroid = sum_centroid / sum_XK;
    centroid = sum_centroid / double(N); //offical does not weight by X AGF 11/11/2010
    // put centroid on surface
    double alt, lon, lat;
    centroid.GetAltLonGDLat(&alt, &lon, &lat);
    centroid.SetAltLonGDLat(0.0, lon, lat);
    measType = meas->measType;    // same for all slices
    eastAzimuth  = atan2( sum_sin_azi_X / sum_XK , sum_cos_azi_X / sum_XK );
    if( eastAzimuth < 0 ) eastAzimuth += two_pi;
    
    // Weighted average of incidence angles (weighted by XK)
    incidenceAngle = sum_incidenceAngle / sum_XK;
    beamIdx = meas->beamIdx;    // same for all slices
    startSliceIdx = min_slice_idx;
    scanAngle = meas->scanAngle;        // same for all slices (from same pulse)
    numSlices = N;

    //----------------------------//
    // Compute KP from obs var    // (variance on mean)
    //----------------------------//
    double this_var_mean_s0 = (sum_Ps2 / sum_XK - pow(value,2))/(double)N;
    
    A = this_var_mean_s0 / pow( value, 2 );
    B = 0;
    C = 0;
    return(1);
}

//-----------------//
// Meas::Composite //
//-----------------//
// Combine measurements into one composite sigma0 and Kpc coefficients.
// The input measurement list should all come from one spot, but this
// routine does not (and can not) check for this.
// Meas is set with the final composite measurement.
// This function returns 0 (and does not composite) if any of the
// measurements are HHVH or VVHV.

int
Meas::Composite(
    MeasList*  meas_list,
    int        n)
{
    double sum_Ps = 0.0;
    double sum_XK = 0.0;
    double sum_EnSlice = 0.0;
    double sum_bandwidth = 0.0;
    Vector3 sum_centroid(0.0,0.0,0.0);
    double sum_incidenceAngle = 0.0;
//  float sum_X2 = 0.0;
    double sum_xk2a = 0.0;
    double sum_xkbwb = 0.0;
    double sum_bw2c = 0.0;
    double sum_cos_azi_X = 0.0;
    double sum_sin_azi_X = 0.0;
    
    int N = 0;

    //
    // Using X in place of Ps when compositing Kpc assumes that sigma0 is
    // uniform across the composite cell area.  This assumption is used
    // below because we sum X^2 instead of Ps^2.
    // We actually use XK which subsumes the K-factor with X.
    //

    Meas* meas;
    Meas* meas_start;

    if (n == 0)
        meas_start = meas_list->GetHead();
    else if (n > 0)
        meas_start = meas_list->GetCurrent();
    else
        return(0);

    int min_slice_idx = meas_start->startSliceIdx;
    landFlag = 0;
    
    int prev_slice_idx = -9999;
    for (meas = meas_start; meas; meas = meas_list->GetNext())
    {
        //-------------------------------------------------//
        // Ensure we do not composite over missing slices. // AGF 11/1/2010
        //-------------------------------------------------// Assumes slice-ordered L1B file....
        if( meas != meas_start && meas->startSliceIdx != prev_slice_idx + 1 ) {
//           fprintf(stderr,
//                   "Meas::Composite: Non-consecutive slices (start, prev, curr): %d %d %d\n",
//                   min_slice_idx, prev_slice_idx, meas->startSliceIdx );
          return(0);
        }
        // Stupid HACK----------------------------------------------------------
        // This assumes that there are an even number of slices, thus none of
        // then have a relative slice index of zero.
        // Either this or abandon the relative slice indexing throughout.
        prev_slice_idx = meas->startSliceIdx;
        if( prev_slice_idx == -1 ) prev_slice_idx = 0;
        // End Stupid HACK------------------------------------------------------
        
        //-------------------------------------------//
        // Don't composite HHVH or VVHV measurements //
        //-------------------------------------------//

        if (meas->measType == VV_HV_CORR_MEAS_TYPE ||
            meas->measType == HH_VH_CORR_MEAS_TYPE)
        {
            return(0);
        }

        //==================================================//
        // Only composites n consecutive objects if default //
        // parameter n is nonzero                           //
        //==================================================//

        if (n != 0 && N >= n)
            break;
        
        // Check for land--tourtured logic required since landFlag is signed!
        if( meas->landFlag == 1 || meas->landFlag == 3 )
          if( landFlag == 0 || landFlag == 2 )
            landFlag += 1;
        // Check for ice        
        if( meas->landFlag == 2 || meas->landFlag == 3 )
          if( landFlag == 0 || landFlag == 1 )
            landFlag += 2;
        
        
        sum_Ps += (double)meas->value * (double)meas->XK;
        sum_XK += (double)meas->XK;
        sum_EnSlice += (double)meas->EnSlice;
        sum_bandwidth += (double)meas->bandwidth;
        //sum_centroid += meas->centroid * (double)meas->XK;
        sum_centroid += meas->centroid;
        sum_incidenceAngle += (double)meas->XK * (double)meas->incidenceAngle;
        sum_xk2a += (double)meas->XK * (double)meas->XK * (double)meas->A;
//      sum_xkbwb += meas->XK * meas->bandwidth * meas->B;
//      sum_bw2c += meas->bandwidth * meas->bandwidth * meas->C;
//      sum_X2 += meas->XK*meas->XK;
        sum_sin_azi_X += (double)meas->XK * (double)sin( meas->eastAzimuth );
        sum_cos_azi_X += (double)meas->XK * (double)cos( meas->eastAzimuth );
        N++;
    }

    meas = meas_list->GetHead();

    //---------------------------------------------------------------------         
    // Form the composite measurement from appropriate combinations of the
    // elements of each slice measurement in this composite cell.
    //---------------------------------------------------------------------

    value = sum_Ps / sum_XK;

    XK = sum_XK;
    EnSlice = sum_EnSlice;
    bandwidth = sum_bandwidth;
    txPulseWidth = meas->txPulseWidth;
    outline.FreeContents();    // merged outlines not done yet
    // Weighted average of centroids (weighted by XK)
    //centroid = sum_centroid / sum_XK;
    centroid = sum_centroid / double(N); //offical does not weight by X AGF 11/11/2010
    // put centroid on surface
    double alt, lon, lat;
    centroid.GetAltLonGDLat(&alt, &lon, &lat);
    centroid.SetAltLonGDLat(0.0, lon, lat);
    measType = meas->measType;    // same for all slices
    eastAzimuth  = atan2( sum_sin_azi_X / sum_XK , sum_cos_azi_X / sum_XK );
    if( eastAzimuth < 0 ) eastAzimuth += two_pi;
    
    // Weighted average of incidence angles (weighted by XK)
    incidenceAngle = sum_incidenceAngle / sum_XK;
    beamIdx = meas->beamIdx;    // same for all slices
    startSliceIdx = min_slice_idx;
    scanAngle = meas->scanAngle;        // same for all slices (from same pulse)
    numSlices = N;

    //----------------------------//
    // composite Kpc coefficients //
    //----------------------------//
    // Composite Kpc coefficients.
    // Derived from Mike's equations but not assuming that the A, B, C, and
    // bandwidths for each slice are identical.

    A = sum_xk2a / (sum_XK * sum_XK);
    B = meas->B  / N;
    C = meas->C  / N;
//    B = sum_xkbwb / (bandwidth * sum_XK);
//    C = sum_bw2c / (bandwidth * bandwidth);
    return(1);
}


//-------------//
// Meas::Write //
//-------------//

int
Meas::Write(
    FILE*  fp)
{
    // Sanity check on sigma0 and estimated Kp
    if (fabs(value) > 1.0e5)
    {
        printf("Error: Meas::Write encountered invalid sigma0 = %g\n",value);
        exit(-1);
    }

    if (fwrite((void *)&value, sizeof(float), 1, fp) != 1 ||
        fwrite((void *)&XK, sizeof(float), 1, fp) != 1 ||
        fwrite((void *)&EnSlice, sizeof(float), 1, fp) != 1 ||
        fwrite((void *)&bandwidth, sizeof(float), 1, fp) != 1 ||
        fwrite((void *)&txPulseWidth, sizeof(float), 1, fp) != 1 ||
        fwrite((void *)&landFlag, sizeof(int), 1, fp) != 1 ||
        outline.Write(fp) != 1 ||
        centroid.WriteLonLat(fp) != 1 ||
        fwrite((void *)&measType, sizeof(MeasTypeE), 1, fp) != 1 ||
        fwrite((void *)&eastAzimuth, sizeof(float), 1, fp) != 1 ||
        fwrite((void *)&incidenceAngle, sizeof(float), 1, fp) != 1 ||
        fwrite((void *)&beamIdx, sizeof(int), 1, fp) != 1 ||
        fwrite((void *)&startSliceIdx, sizeof(int), 1, fp) != 1 ||
        fwrite((void *)&numSlices, sizeof(int), 1, fp) != 1 ||
        fwrite((void *)&scanAngle, sizeof(float), 1, fp) != 1 ||
        fwrite((void *)&A, sizeof(float), 1, fp) != 1 ||
        fwrite((void *)&B, sizeof(float), 1, fp) != 1 ||
        fwrite((void *)&C, sizeof(float), 1, fp) != 1 ||
        fwrite((void *)&azimuth_width, sizeof(float), 1, fp) != 1 ||
        fwrite((void *)&range_width, sizeof(float), 1, fp) != 1)
    {
        return(0);
    }
    return(1);
}

//------------//
// Meas::Read //
//------------//

int
Meas::Read(
    FILE*    fp)
{
    FreeContents();
    offset = ftello(fp);
    if (fread((void *)&value, sizeof(float), 1, fp) != 1 ||
        fread((void *)&XK, sizeof(float), 1, fp) != 1 ||
        fread((void *)&EnSlice, sizeof(float), 1, fp) != 1 ||
        fread((void *)&bandwidth, sizeof(float), 1, fp) != 1 ||
        fread((void *)&txPulseWidth, sizeof(float), 1, fp) != 1 ||
        fread((void *)&landFlag, sizeof(int), 1, fp) != 1 ||
        outline.Read(fp) != 1 ||
        centroid.ReadLonLat(fp) != 1 ||
        fread((void *)&measType, sizeof(MeasTypeE), 1, fp) != 1 ||
        fread((void *)&eastAzimuth, sizeof(float), 1, fp) != 1 ||
        fread((void *)&incidenceAngle, sizeof(float), 1, fp) != 1 ||
        fread((void *)&beamIdx, sizeof(int), 1, fp) != 1 ||
        fread((void *)&startSliceIdx, sizeof(int), 1, fp) != 1 ||
        fread((void *)&numSlices, sizeof(int), 1, fp) != 1 ||
        fread((void *)&scanAngle, sizeof(float), 1, fp) != 1 ||
        fread((void *)&A, sizeof(float), 1, fp) != 1 ||
        fread((void *)&B, sizeof(float), 1, fp) != 1 ||
        fread((void *)&C, sizeof(float), 1, fp) != 1 ||
        fread((void *)&azimuth_width, sizeof(float), 1, fp) != 1 ||
        fread((void *)&range_width, sizeof(float), 1, fp) != 1)
    {
        return(0);
    }
    return(1);
}

//------------------//
// Meas::WriteAscii //
//------------------//

int
Meas::WriteAscii(
    FILE*    fp)
{
    double lon=0, lat=0, alt=0;
    centroid.GetAltLonGDLat(&alt,&lon,&lat);
    lon*=rtd;
    lat*=rtd;
    fprintf(fp,"\n###########  Slice/Composite  Info  ###############\n");
    fprintf(fp, "BeamIdx: %d ", beamIdx);
    fprintf(fp, "StartSliceIdx: %d ", startSliceIdx);
    fprintf(fp, "NumSlices: %d ", numSlices);
    fprintf(fp, "ScanAngle: %g ", scanAngle*rtd);
    fprintf(fp, "XK: %g ",XK);
    fprintf(fp, "Value: %g ", value);
    fprintf(fp, "EnSlice: %g ", EnSlice);
    fprintf(fp, "MeasType: %s ", meas_type_map[(int)measType]);
    fprintf(fp, "IncAngle: %g ", incidenceAngle*rtd);
    fprintf(fp, "eastAzimuth: %g ", eastAzimuth*rtd);
    fprintf(fp, "Longitude: %g ",lon);
    fprintf(fp, "Latitude: %g ",lat);
    fprintf(fp, "Bandwidth: %g ", bandwidth);
    fprintf(fp, "txPulseWidth: %g ", txPulseWidth);
    fprintf(fp, "LandFlag: %d ", landFlag);
    fprintf(fp, "A: %g ", A);
    fprintf(fp, "B: %g ", B);
    fprintf(fp, "C: %g ", C);
    fprintf(fp, "azimuth_width: %g ", azimuth_width);
    fprintf(fp, "range_width: %g \n", range_width);

    return(1);
}

//--------------------//
// Meas::FreeContents //
//--------------------//

void
Meas::FreeContents()
{
    outline.FreeContents();
    return;
}

//----------------------------//
// MeasSpotList::UnpackL1BHdf //
//----------------------------//

int
Meas::UnpackL1BHdf(
    int32    sd_id,
    int *start,		// array of 3 indexes, indicating where to start in the hdf file
    int *edges)		// array of the size of each dimension to read
{
    FreeContents();
    
    int cur_edges[] = {1,1,1};
    int sliceIndex = start[2];
    bool failed = false;
    
    // macro defined in meas.h
    GET_HDF_VAR(int32, slice_qual_flag, start, cur_edges, 1)
	if (failed) {
	        fprintf(stderr,
    	        "Meas::UnpackL1BHdf: error with SDreaddata (slice_qual_flag)\n");
	        return(0);
    }
    
    // each group of 4 bits represents the quality of this slice, and we want to check
    // the 1st of those 4 bits for this slice. skip this slice if it isn't any good.
    if ((int)slice_qual_flag & (1 << (sliceIndex * 4)))
        return 0;
        
    startSliceIdx = START_SLICE_INDEX + sliceIndex;
    startSliceIdx = (startSliceIdx >= 0 ?
                 startSliceIdx + 1 : startSliceIdx);
    
    GET_HDF_VAR(int16, slice_sigma0, start, cur_edges, 0.01)
    value = (float) pow( 10.0, slice_sigma0 / 10.0 );
    int neg_mask = 1 << sliceIndex*4+1;
    if (neg_mask & (int)slice_qual_flag)
        value *= -1;
    
    GET_HDF_VAR(int16, x_factor, start, cur_edges, 0.01)
    XK = (float) pow( 10.0, x_factor / 10.0 );
    
    GET_HDF_VAR(int16, slice_snr, start, cur_edges, 0.01)
    float sliceSNR = (float) pow( 10.0, slice_snr / 10.0 );
    EnSlice = value * XK / sliceSNR;

    //---------------------------------------------------
    // centroid is from slice_lon and slice_lat.
    // slice_lon,lat are deltas off of cell_lon,lat.
    //---------------------------------------------------
    // scale converts deg-> radians
    GET_HDF_VAR(float, cell_lat, start, cur_edges, dtr)
    GET_HDF_VAR(float, cell_lon, start, cur_edges, dtr)
    GET_HDF_VAR(int16, slice_lat, start, cur_edges, 1e-4 * dtr)
    GET_HDF_VAR(int16, slice_lon, start, cur_edges, 1e-4 * dtr)
    slice_lon /= cos(cell_lat);
    slice_lon += cell_lon;
    slice_lat += cell_lat;
    centroid.SetAltLonGDLat(0.0, slice_lon, slice_lat);

	// other stuff..
    GET_HDF_VAR(uint16, slice_azimuth, start, cur_edges, 0.01 * dtr)
    float northAzimuth = slice_azimuth;
    eastAzimuth = (450.0*dtr - northAzimuth);
    if (eastAzimuth >= two_pi) eastAzimuth -= two_pi;
    
    GET_HDF_VAR(uint16, slice_incidence, start, cur_edges, 0.01 * dtr)
    incidenceAngle = slice_incidence;
    if (incidenceAngle < 50*dtr) {
    	beamIdx = 0;
		measType = HH_MEAS_TYPE;
    } else {
    	beamIdx = 1;
		measType = VV_MEAS_TYPE;
    }
    numSlices = -1;
    
    GET_HDF_VAR(uint16, antenna_azimuth, start, cur_edges, 0.01 * dtr)
    scanAngle = antenna_azimuth;
    
    GET_HDF_VAR(uint16, cell_kpc_a, start, cur_edges, 1e-4)

    // Estimate of QuikSCAT Kpc A coefficient need L2A value not 
    // intemediate value kept in L2B

    float nL=10.0; // assumes 10 looks per slice
    A = 1+ 1/nL; 
    float s0NE=EnSlice/XK;
    B = fabs(2.0*s0NE/nL);
    C = fabs(s0NE*s0NE/nL);

//    printf("start = %d, %d, %d; sig0 = %f; lon = %f; lat = %f\n", 
//    	start[0], start[1], start[2], value, slice_lon*180/M_PI, slice_lat*180/M_PI);
    	
	if (failed) {
        fprintf(stderr, "Meas::UnpackL1BHdf: error with SDreaddata\n");
        return(0);
    } else
		return 1;
}

// OBSOLETE- here only so that legecy code compiles //
int
Meas::UnpackL1BHdf(
L1BHdf*     l1bHdf,
int32       pulseIndex,   // index in pluses
int32       sliceIndex)   // index in slices
{
    FreeContents();

    Parameter* param=0;

    param = l1bHdf->GetParameter(SLICE_QUAL_FLAG, UNIT_DN);
    assert(param != 0);
    unsigned int* uintP = (unsigned int*)param->data;
    unsigned int sliceQualFlags = *(uintP + pulseIndex);

    //------------------------------------------------------------------
    // if gain does not exceed peak gain thrshhold, throw this slice away
    //------------------------------------------------------------------
    if (sliceQualFlags & (1 << (sliceIndex * 4)))
        return 0;

    if ((sliceQualFlags & (1 << (sliceIndex * 4))) == 0)
    {
        startSliceIdx = START_SLICE_INDEX + sliceIndex;
        startSliceIdx = (startSliceIdx >= 0 ?
                                 startSliceIdx + 1 : startSliceIdx);
    }
    assert (startSliceIdx != 0);

    param = l1bHdf->GetParameter(SLICE_SIGMA0, UNIT_DB);
    assert(param != 0);
    float* floatP = (float*)param->data;
    floatP += pulseIndex * MAX_L1BHDF_NUM_SLICES + sliceIndex;
    value = (float) pow( 10.0, (double) *floatP / 10.0 );

    param = l1bHdf->GetParameter(X_FACTOR, UNIT_DB);
    assert(param != 0);
    floatP = (float*)param->data;
    floatP += pulseIndex * MAX_L1BHDF_NUM_SLICES + sliceIndex;
    XK = (float) pow( 10.0, (double) *floatP / 10.0 );

    param = l1bHdf->GetParameter(SLICE_SNR, UNIT_DB);
    assert(param != 0);
    floatP = (float*)param->data;
    floatP += pulseIndex * MAX_L1BHDF_NUM_SLICES + sliceIndex;
    float sliceSNR = (float) pow( 10.0, (double) *floatP / 10.0 );
    EnSlice = value * XK / sliceSNR;

    // bandwidth and tx pulse width come from configuration file
    bandwidth = l1bHdf->configBandwidth;
    txPulseWidth = l1bHdf->configTxPulseWidth;
    landFlag = 0;

    // outline: leave it alone

    //---------------------------------------------------
    // centroid is from slice_lon and slice_lat.
    // slice_lon,lat are deltas off of cell_lon,lat.
    //---------------------------------------------------

    param = l1bHdf->GetParameter(CELL_LON, UNIT_RADIANS);
    assert(param != 0);
    floatP = (float*)param->data;
    double cellLon = (double) *(floatP + pulseIndex);
    param = l1bHdf->GetParameter(CELL_LAT, UNIT_RADIANS);
    assert(param != 0);
    floatP = (float*)param->data;
    double cellLat = (double) *(floatP + pulseIndex);
    param = l1bHdf->GetParameter(SLICE_LON, UNIT_RADIANS);
    assert(param != 0);
    floatP = (float*)param->data;
    double sliceLon = (double) *(floatP +
                        pulseIndex * MAX_L1BHDF_NUM_SLICES + sliceIndex);
    sliceLon /= cos(cellLat);
    sliceLon += cellLon;
    param = l1bHdf->GetParameter(SLICE_LAT, UNIT_RADIANS);
    assert(param != 0);
    floatP = (float*)param->data;
    double sliceLat = (double) *(floatP +
                        pulseIndex * MAX_L1BHDF_NUM_SLICES + sliceIndex);
    sliceLat += cellLat;
    centroid.SetAltLonGDLat(0.0, sliceLon, sliceLat);

    //---------------------------------------------------
    // get beamIdx and pol
    //---------------------------------------------------
    param = l1bHdf->GetParameter(FRAME_INST_STATUS_02, UNIT_MAP);
    assert(param != 0);
    unsigned char beamNo = *((unsigned char*)param->data);
    if (IS_EVEN(pulseIndex))
        beamIdx = (int)beamNo;
    else
        beamIdx = (int) THE_OTHER_BEAM(beamNo);
    if (beamIdx == 0)  // beam A => H, B => V
        measType = HH_MEAS_TYPE;
    else
        measType = VV_MEAS_TYPE;

    param = l1bHdf->GetParameter(SLICE_AZIMUTH, UNIT_RADIANS);
    assert(param != 0);
    floatP = (float*)param->data;
    float northAzimuth = *(floatP + pulseIndex * MAX_L1BHDF_NUM_SLICES +
        sliceIndex);
    eastAzimuth = (450.0*dtr - northAzimuth);
    if (eastAzimuth >= two_pi) eastAzimuth -= two_pi;

    param = l1bHdf->GetParameter(SLICE_INCIDENCE, UNIT_RADIANS);
    assert(param != 0);
    floatP = (float*)param->data;
    incidenceAngle = *(floatP + pulseIndex * MAX_L1BHDF_NUM_SLICES+sliceIndex);

    numSlices = 1;

    param = l1bHdf->GetParameter(ANTENNA_AZIMUTH, UNIT_RADIANS);
    assert(param != 0);
    floatP = (float*)param->data;
    scanAngle = *(floatP + pulseIndex);

    param = l1bHdf->GetParameter(SLICE_KPC_A, UNIT_DN);
    assert(param != 0);
    floatP = (float*)param->data;
    A = *(floatP + pulseIndex * MAX_L1BHDF_NUM_SLICES + sliceIndex);

    return 1;

} // Meas::UnpackL1BHdf

//----------------------//
// Meas::ReadL2AHdfCell //
//----------------------//

int
Meas::ReadL2AHdfCell(
    L2AHdf*  l2aHdf,
    int      dataIndex,    // 0 - { _dataLength - 1 }
    int      arrayIndex)   // 0 - 3239
{
    FreeContents();

    Parameter* param = 0;

    param = l2aHdf->ExtractParameter(SIGMA0_QUAL_FLAG, UNIT_DN, dataIndex);
    assert(param != 0);
    unsigned short* ushortP = (unsigned short*)param->data;
    unsigned short sigma0QualFlags = *(ushortP + arrayIndex);

    //------------------------------------------------------------------
    // if bit 0 in sigma0_qual_flag is not 0, this measurement is not usable
    //------------------------------------------------------------------

    if (sigma0QualFlags & 0x0001)
        return(0);

    param = l2aHdf->ExtractParameter(SIGMA0, UNIT_DB, dataIndex);
    assert(param != 0);
    float* floatP = (float*)param->data;
    floatP += arrayIndex;
    value = (float) pow( 10.0, (double) *floatP / 10.0 );

    //------------------------------------------------------------------
    // if bit 2 in sigma0_qual_flag is 1, sigma0 is negative
    //------------------------------------------------------------------

    if ((sigma0QualFlags & 0x0004) && value > 0)
        value = -(value);

    // ignore bandwidth and tx pulse width

    //---------------------------------------------------
    // langFlag is bits 0-1, includes ice-flag
    //---------------------------------------------------

    param = l2aHdf->ExtractParameter(SURFACE_FLAG, UNIT_DN, dataIndex);
    assert(param != 0);
    ushortP = (unsigned short*)param->data;
    landFlag = (int) (*(ushortP + arrayIndex) & 0x00000003);

    // outline: leave it alone

    //---------------------------------------------------
    // centroid is from cell_lon and cell_lat.
    //---------------------------------------------------

    param = l2aHdf->ExtractParameter(CELL_LON, UNIT_RADIANS, dataIndex);
    assert(param != 0);
    floatP = (float*)param->data;
    double cellLon = (double) *(floatP + arrayIndex);

    param = l2aHdf->ExtractParameter(CELL_LAT, UNIT_RADIANS, dataIndex);
    assert(param != 0);
    floatP = (float*)param->data;
    double cellLat = (double) *(floatP + arrayIndex);

    centroid.SetAltLonGDLat(0.0, cellLon, cellLat);

    //---------------------------------------------------
    // get beamIdx (bit 2 in sigma0_mode_flag)
    //---------------------------------------------------

    param = l2aHdf->ExtractParameter(SIGMA0_MODE_FLAG, UNIT_DN, dataIndex);
    assert(param != 0);
    ushortP = (unsigned short*)param->data;
    unsigned short sigma0ModeFlags = *(ushortP + arrayIndex);
    beamIdx = (int) (sigma0ModeFlags & 0x0004);
    beamIdx >>= 2;

    //---------------------------------------------------
    // measurement type depends on beam number
    //---------------------------------------------------

    if (beamIdx == 0)  // beam A => H, B => V
        measType = HH_MEAS_TYPE;
    else
        measType = VV_MEAS_TYPE;

    param = l2aHdf->ExtractParameter(CELL_AZIMUTH, UNIT_DEGREES, dataIndex);
    assert(param != 0);
    floatP = (float*)param->data;
    float northAzimuth = *(floatP + arrayIndex);
    eastAzimuth = (450.0 - northAzimuth) * dtr;
    if (eastAzimuth >= two_pi) eastAzimuth -= two_pi;

    param = l2aHdf->ExtractParameter(CELL_INCIDENCE, UNIT_RADIANS, dataIndex);
    assert(param != 0);
    floatP = (float*)param->data;
    incidenceAngle = *(floatP + arrayIndex);

    // -1 indicate this is converted from HDF
    numSlices = -1;

    param = l2aHdf->ExtractParameter(KP_ALPHA, UNIT_DN, dataIndex);
    assert(param != 0);
    floatP = (float*)param->data;
    A = *(floatP + arrayIndex);

    param = l2aHdf->ExtractParameter(KP_BETA, UNIT_DN, dataIndex);
    assert(param != 0);
    floatP = (float*)param->data;
    B = *(floatP + arrayIndex);

    param = l2aHdf->ExtractParameter(KP_GAMMA, UNIT_DN, dataIndex);
    assert(param != 0);
    floatP = (float*)param->data;
    C = *(floatP + arrayIndex);

//printf("value = %f\n", value);
    return(1);
} // Meas::UnpackL2AHdf

//==========//
// MeasList //
//==========//

MeasList::MeasList()
{
    return;
}

MeasList::~MeasList()
{
    FreeContents();
    return;
}

//-----------------//
// MeasList::Write //
//-----------------//

int
MeasList::Write(
    FILE*  fp)
{
    int count = NodeCount();
    if (fwrite((void *)&count, sizeof(int), 1, fp) != 1)
        return(0);

    for (Meas* meas = GetHead(); meas; meas = GetNext())
    {
        if (! meas->Write(fp))
            return(0);
    }
    return(1);
}

//----------------//
// MeasList::Read //
//----------------//

int
MeasList::Read(
    FILE*    fp)
{
    FreeContents();

    int count;
    if (fread((void *)&count, sizeof(int), 1, fp) != 1)
        return(0);

    for (int i = 0; i < count; i++)
    {
        Meas* new_meas = new Meas();
        if (! new_meas->Read(fp) ||
            ! Append(new_meas))
        {
            return(0);
        }
    }
    return(1);
}

//----------------------//
// MeasList::WriteAscii //
//----------------------//

int
MeasList::WriteAscii(
    FILE*    fp)
{
    fprintf(fp, "\n### Slice/Composite Count: %d ####\n", NodeCount());
    for (Meas* meas = GetHead(); meas; meas = GetNext())
    {
        if (! meas->WriteAscii(fp))
            return(0);
    }
    return(1);
}

//--------------------------//
// MeasList::ReadL2AHdfCell //
//--------------------------//

int
MeasList::ReadL2AHdfCell(
    L2AHdf*  l2aHdf,
    int      rowNo,    // row number: 1-1624
    int      cellNo)   // column number: 1-76
{
    assert(l2aHdf != 0);
    FreeContents();

    Parameter* param = 0;
    for (int i=0; i < l2aHdf->GetDataLength(); i++)
    {
        // extract only if row_number is the target one
        param = l2aHdf->ExtractParameter(ROW_NUMBER, UNIT_DN, i);
        assert(param != 0);
        short* shortP = (short*) (param->data);
        if (*shortP != (short)rowNo)
            continue;

        param = l2aHdf->ExtractParameter(CELL_INDEX, UNIT_DN, i);
        assert(param != 0);
        char*  cellIndexP = (char*)param->data;

        param = l2aHdf->ExtractParameter(SIGMA0_MODE_FLAG, UNIT_DN, i);
        assert(param != 0);
        unsigned short* sliceModeFlagsP = (unsigned short*) (param->data);

        for (int j=0; j < l2aHdf->numCells; j++)
        {
            // extract only if cell_index is the target one
            if (*(cellIndexP + j) != (char)cellNo)
                continue;

            //-------------------------------------------------------
            // the bit 0-1 in sigma0_mode_flag must be 0 - meas pulse
            // otherwise, skip this cell
            //-------------------------------------------------------

            unsigned short sliceModeFlags = *(sliceModeFlagsP + j);
            if ((sliceModeFlags & 0x0003) != 0)
                continue;

            //-------------------------------------------------------
            // if this is a valid cell, add to the list
            //-------------------------------------------------------

            Meas* new_meas = new Meas();
            if (new_meas->ReadL2AHdfCell(l2aHdf, i, j))
            {
                if (Append(new_meas) == 0)
                    return(0);
            }
            else delete new_meas;
        }
    }
    return(1);
}

//-------------------------//
// MeasList::AverageLonLat //
//-------------------------//

LonLat
MeasList::AverageLonLat(int wtbyXK)
{
    EarthPosition sum;
    sum.SetPosition(0.0, 0.0, 0.0);
    for (Meas* meas = GetHead(); meas; meas = GetNext())
    {
   
      if(wtbyXK){
        sum += meas->centroid*meas->XK;
      }
      else{
        sum += meas->centroid;
      }
    }

    // The center of the earth is at 0,0,0 (geocentric coords)
    EarthPosition earth_center;
    earth_center.SetPosition(0.0, 0.0, 0.0);
    // Find the surface point lying along the averaged direction.
    EarthPosition ravg;
    if(earth_intercept(earth_center,sum,&ravg)!=1){
      fprintf(stderr,"MeasList::AveLonLat: earth_intercept error\n");
      exit(1);
    }

    LonLat lon_lat;
    lon_lat.Set(ravg);
    return(lon_lat);
}

//------------------------//
// MeasList::FreeContents //
//------------------------//

void
MeasList::FreeContents()
{
    Meas* meas;
    GotoHead();
    while ((meas = RemoveCurrent()) != NULL)
        delete meas;
    return;
}


//============//
// OffsetList //
//============//

OffsetList::OffsetList()
{
    return;
}

OffsetList::~OffsetList()
{
    FreeContents();
    return;
}

//--------------------------//
// OffsetList::MakeMeasList //
//--------------------------//

int
OffsetList::MakeMeasList(
    FILE*        fp,
    MeasList*    meas_list)
{
    for (off_t* offset = GetHead(); offset; offset = GetNext())
    {
        Meas* meas = new Meas();
        if (fseeko(fp, *offset, SEEK_SET) == -1)
            return(0);

        if (! meas->Read(fp))
            return(0);

        if (! meas_list->Append(meas))
        {
            delete meas;
            return(0);
        }
    }

    return(1);
}

//--------------------------//
// OffsetList::FreeContents //
//--------------------------//

void
OffsetList::FreeContents()
{
    off_t* offset;
    GotoHead();
    while ((offset = RemoveCurrent()) != NULL)
        delete offset;
    return;
}

//================//
// OffsetListList //
//================//

OffsetListList::OffsetListList()
{
    return;
}

OffsetListList::~OffsetListList()
{
    FreeContents();
    return;
}

//------------------------------//
// OffsetListList::FreeContents //
//------------------------------//

void
OffsetListList::FreeContents()
{
    OffsetList* offsetlist;
    GotoHead();
    while ((offsetlist = RemoveCurrent()) != NULL)
        delete offsetlist;
    return;
}

//==========//
// MeasSpot //
//==========//

MeasSpot::MeasSpot()
:    time(0.0), scOrbitState(), scAttitude()
{
    return;
}

MeasSpot::~MeasSpot()
{
    return;
}

//-----------------//
// MeasSpot::Write //
//-----------------//

int
MeasSpot::Write(
    FILE*    fp)
{
    if (fwrite((void *)&time, sizeof(double), 1, fp) != 1 ||
        scOrbitState.Write(fp) != 1 ||
        scAttitude.Write(fp) != 1 ||
        MeasList::Write(fp) != 1)
    {
        return(0);
    }
    
    return(1);
}

//-----------------//
// MeasSpot::Write //
//-----------------//

int
MeasSpot::WriteAscii(
    FILE*    fp)
{
        fprintf(fp,"\n##############################################\n");
        fprintf(fp,"#######         Spot Info               ######\n");
        fprintf(fp,"##############################################\n");
        fprintf(fp,"\n");
	ETime et;
        et.SetTime(time);
        char str[100];
        et.ToCodeA(str);
        fprintf(fp,"Time: %.20g %s\n", time,str );
    if (scOrbitState.WriteAscii(fp) != 1 ||
        scAttitude.WriteAscii(fp) != 1 ||
        MeasList::WriteAscii(fp) != 1)
    {
        return(0);
    }
    return(1);
}

//----------------//
// MeasSpot::Read //
//----------------//

int
MeasSpot::Read(
    FILE*    fp)
{
    FreeContents();

    if (fread((void *)&time, sizeof(double), 1, fp) != 1 ||
        scOrbitState.Read(fp) != 1 ||
        scAttitude.Read(fp) != 1 ||
        MeasList::Read(fp) != 1)
    {
        return(0);
    }
    return(1);
}

// Be careful using this function it is poorly named
// It uses hardcoded constants for chirp rate and carrier frequency
// It ignores any quantities which are constant for a given footprint, i.e., RxGATEDELAY, chirpstartfreq,etc

#define NOMINAL_QUIKSCAT_TRANSMIT_FREQUENCY 13.403e9
#define NOMINAL_QUIKSCAT_CHIRPRATE 2.50729323e8
float
MeasSpot::NominalQuikScatBaseBandFreq(Vector3 pos){
  Vector3 gclook= pos - scOrbitState.rsat;
  double range=gclook.Magnitude(); 
  gclook/=range;
  Vector3 vspot(-w_earth * pos.Get(1),
        w_earth * pos.Get(0), 0);
  Vector3 vrel = scOrbitState.vsat - vspot;
  double lambda = speed_light_kps / NOMINAL_QUIKSCAT_TRANSMIT_FREQUENCY;
  double rtt= 2.0 * range/speed_light_kps;
  float rangefreq=NOMINAL_QUIKSCAT_CHIRPRATE*rtt;
  float dopplerfreq=2.0 * ( vrel % gclook)/lambda;
  return(rangefreq+dopplerfreq);
  return(0.0);
}

int
MeasSpot::UnpackL1BHdf(
    int32    sd_id,
    int32	h_id,
    int *start,		// array of 3 indexes, indicating where to start in the hdf file
    int *edges)		// array of the size of each dimension to read
{
    FreeContents();
    
    bool failed = false;
    int orbit_state_edges[] = {1};
    
    //----------------------------
    // get orbit time
    //----------------------------
	#define BUF_LEN		64
	char frame_time[BUF_LEN];
    // stupid VSread doesn't null terminate the string,
    // so initiate initiate the buffer to all nulls
    for(int i = 0; i < BUF_LEN; i++)
    	frame_time[i] = NULL;
    int32 frame_time_ref_id = VSfind(h_id, "frame_time");
    int32 frame_time_id = VSattach(h_id, frame_time_ref_id, "r");
    VSseek(frame_time_id, start[0]);
    
	if (VSread(frame_time_id, (uint8 *)frame_time, 1, NO_INTERLACE) == FAIL) {
    	fprintf(stderr, "MeasSpot::UnpackL1BHdf: Error: could not get frame time\n");
    	return(0);
    }
    
    ETime etime;
    // 4/7/10- Ken Oslund
    // when this code was originally written (end of 2009) I determined that the time
    // should be seconds since Jan 1, 1993. Now, from running l1b_to_l2a and having it fail
    // due to incorrect time stamps, it would appear that this should be seconds since
    // Jan 1 1970 (unix time). So, we're going to change it and cross our fingers that it
    // doesn't break anything else.
    etime.FromCodeB("1970-001T00:00:00.000");
    double time_base = (double)etime.GetSec() + (double)etime.GetMs()/1000;

    if(!etime.FromCodeB(frame_time)) {
    	fprintf(stderr, "MeasSpot::UnpackL1BHdf: Error: could not parse time string: %s\n",
    		frame_time);
    	return 0;
    }
    time = (double)etime.GetSec() + (double)etime.GetMs()/1000 - time_base;
    
    // obsolete block of code
//	printf("time_base = %f, time = %f\n", time_base, time);

    // get time VD, use TAI time as base
//    Itime itime;
//    if (l1bHdf->GetTime(hdfIndex, &itime) != HdfFile::OK)
//    {
//        fprintf(stderr, "Fail to get time on HDF index %d\n", hdfIndex);
//        return 0;
//    }
//    time = (double) itime.sec - ITIME_DEFAULT_SEC + (double)(itime.ms)/1000.0;
//
//    if (l1bHdf->GetTime(hdfIndex+1, &itime) == HdfFile::OK)
//    {
//      double time2 = (double) itime.sec - ITIME_DEFAULT_SEC +
//        (double)(itime.ms)/1000.0;
//      time += (time2 - time)*((float)pulseIndex)/((float)Npulse);
//    }
//    else if (l1bHdf->GetTime(hdfIndex-1, &itime) == HdfFile::OK)
//    {
//      double time1 = (double) itime.sec - ITIME_DEFAULT_SEC +
//        (double)(itime.ms)/1000.0;
//      time += (time - time1)*((float)pulseIndex)/((float)Npulse);
//    }
//    else
//    {
//        fprintf(stderr, "Fail to get time on HDF index %d\n", hdfIndex);
//        return 0;
//    }
    scOrbitState.time = time;

    //----------------------------
    // get orbit state
    //----------------------------

    // position //
    // this macro is defined in meas.h
   	// scale factor to convert m -> km
    GET_HDF_VAR(float, x_pos, start, orbit_state_edges, 1e-3)
    GET_HDF_VAR(float, y_pos, start, orbit_state_edges, 1e-3)
    GET_HDF_VAR(float, z_pos, start, orbit_state_edges, 1e-3)

    scOrbitState.rsat = Vector3(x_pos, y_pos, z_pos);

	// velocity //
    GET_HDF_VAR(float, x_vel, start, orbit_state_edges, 1e-3)
    GET_HDF_VAR(float, y_vel, start, orbit_state_edges, 1e-3)
    GET_HDF_VAR(float, z_vel, start, orbit_state_edges, 1e-3)
	// convert m/s -> km/s
    scOrbitState.vsat = Vector3(x_vel, y_vel, z_vel);

	// orientation
    GET_HDF_VAR(int16, roll, start, orbit_state_edges, 1e-3*dtr)
    GET_HDF_VAR(int16, pitch, start, orbit_state_edges, 1e-3*dtr)
    GET_HDF_VAR(int16, yaw, start, orbit_state_edges, 1e-3*dtr)

	// convert deg -> radians
    scAttitude.SetRPY(roll, pitch, yaw);
    
    if (failed) {
    	fprintf(stderr, "MeasSpot::UnpackL1BHdf: Error: could not get orbit state\n");
    	return(0);
    }
    	

    int num_slices = edges[2];
    for (int i = 0; i < num_slices; i++)
    {
        // save only the good slices
        Meas* new_meas = new Meas();
        int cur_start[] = {start[0], start[1], i};

        // Meas::UnpackL1BHdf return 0 when there is no valid data
        // so it is ok to ignore the return code 0
        if (new_meas->UnpackL1BHdf(sd_id, cur_start, edges)) {
            if ( ! Append(new_meas)) return(0);
        } else
        	delete new_meas;
    }

    return(1);

}


// OBSOLETE //
int
MeasSpot::UnpackL1BHdf(
L1BHdf*      l1bHdf,    // SVT L1BHdf object
int32        hdfIndex,  // index in the L1BHDF
int32        pulseIndex)   // index of the pulses (max of 100)
{
    assert(l1bHdf != 0);

    Parameter* param=0;

    FreeContents();

    param = l1bHdf->GetParameter(NUM_PULSES, UNIT_DN);
    assert(param != 0);
    int Npulse = (int)(*((char*)(param->data)));

    // get time VD, use TAI time as base
    Itime itime;
    if (l1bHdf->GetTime(hdfIndex, &itime) != HdfFile::OK)
    {
        fprintf(stderr, "Fail to get time on HDF index %d\n", hdfIndex);
        return 0;
    }
    time = (double) itime.sec - ITIME_DEFAULT_SEC + (double)(itime.ms)/1000.0;

    if (l1bHdf->GetTime(hdfIndex+1, &itime) == HdfFile::OK)
    {
      double time2 = (double) itime.sec - ITIME_DEFAULT_SEC +
        (double)(itime.ms)/1000.0;
      time += (time2 - time)*((float)pulseIndex)/((float)Npulse);
    }
    else if (l1bHdf->GetTime(hdfIndex-1, &itime) == HdfFile::OK)
    {
      double time1 = (double) itime.sec - ITIME_DEFAULT_SEC +
        (double)(itime.ms)/1000.0;
      time += (time - time1)*((float)pulseIndex)/((float)Npulse);
    }
    else
    {
        fprintf(stderr, "Fail to get time on HDF index %d\n", hdfIndex);
        return 0;
    }

    //----------------------------
    // get orbit state
    //----------------------------
    scOrbitState.time = time;

    param = l1bHdf->GetParameter(X_POS, UNIT_KILOMETERS);
    assert(param != 0);
    double x_pos = (double)(*((float*)(param->data)));

    param = l1bHdf->GetParameter(Y_POS, UNIT_KILOMETERS);
    assert(param != 0);
    double y_pos = (double)(*((float*)(param->data)));

    param = l1bHdf->GetParameter(Z_POS, UNIT_KILOMETERS);
    assert(param != 0);
    double z_pos = (double)(*((float*)(param->data)));

    scOrbitState.rsat = Vector3(x_pos, y_pos, z_pos);

    param = l1bHdf->GetParameter(X_VEL, UNIT_KMPS);
    assert(param != 0);
    double x_vel = (double)(*((float*)(param->data)));
    param = l1bHdf->GetParameter(Y_VEL, UNIT_KMPS);
    assert(param != 0);
    double y_vel = (double)(*((float*)(param->data)));
    param = l1bHdf->GetParameter(Z_VEL, UNIT_KMPS);
    assert(param != 0);
    double z_vel = (double)(*((float*)(param->data)));
    scOrbitState.vsat = Vector3(x_vel, y_vel, z_vel);

    //----------------------------
    // get attitude
    //----------------------------
    param = l1bHdf->GetParameter(ROLL, UNIT_RADIANS);
    assert(param != 0);
    float roll = *((float*)(param->data));
    param = l1bHdf->GetParameter(PITCH, UNIT_RADIANS);
    assert(param != 0);
    float pitch = *((float*)(param->data));
    param = l1bHdf->GetParameter(YAW, UNIT_RADIANS);
    assert(param != 0);
    float yaw = *((float*)(param->data));
    scAttitude.SetRPY(roll, pitch, yaw);

    for (int i = 0; i < MAX_L1BHDF_NUM_SLICES; i++)
    {
        // save only the good slices
        Meas* new_meas = new Meas();
        // Meas::UnpackL1BHdf return 0 when there is no valid data
        // so it is ok to ignore the return code 0
        if (new_meas->UnpackL1BHdf(l1bHdf, pulseIndex, i))
        {
            if ( ! Append(new_meas)) return(0);
        }
        else delete new_meas;
    }

    return(1);
}

int MeasSpot::ComputeRangeWidth(Meas* meas, float* range_width) {
    // Computes the rate of change of base-band freq with range on ground,
    // returns the range width corresponding to bandwidth of meas
    Vector3 look = meas->centroid - scOrbitState.rsat;

    // z is unit normal, y = z cross look, x = y cross z.
    // x is look direction projected onto local tangent plane.
    Vector3 zvec0 = meas->centroid.Normal();
    Vector3 yvec0 = zvec0 & look;  yvec0 = yvec0 / yvec0.Magnitude();
    Vector3 xvec0 = yvec0 & zvec0; xvec0 = xvec0 / xvec0.Magnitude();

    float bbf1 = NominalQuikScatBaseBandFreq(meas->centroid+xvec0);
    float bbf0 = NominalQuikScatBaseBandFreq(meas->centroid-xvec0);

    // rate of change of base-band freq with range
    float dbbfdx0 = (bbf1-bbf0)/2.0;

    *range_width = meas->bandwidth / dbbfdx0;
    return(1);
}

#define COAST_NXSTEPS 5
#define COAST_NYSTEPS 25

int MeasSpot::ComputeLandFraction( LandMap*   lmap,
                                   QSLandMap* qs_lmap,
                                   Antenna*   ant,
                                   float      freq_shift,
                                   double     spot_lon,
                                   double     spot_lat) {
  //printf("Length MeasSpot: %d\n",NodeCount());
  
  if( NodeCount() == 0 ) return(1);
  Meas* first_meas = GetHead();

  EarthPosition spot_centroid;
  spot_centroid.SetAltLonGDLat(0.0, spot_lon, spot_lat);
  
  Vector3 look = spot_centroid - scOrbitState.rsat;
  float   rtt  = 2*look.Magnitude()/speed_light_kps;
  
  //printf("rtt: %f\n",rtt);
  
  look = look/look.Magnitude();
  ant->SetTxCenterAzimuthAngle( first_meas->scanAngle );

  // Compute Axes of SpotFrame 
  CoordinateSwitch gc_to_antenna;
  gc_to_antenna = AntennaFrameToGC( &scOrbitState,
                                    &scAttitude, 
                                    ant,
                                    ant->txCenterAzimuthAngle);
  gc_to_antenna = gc_to_antenna.ReverseDirection();
  
  // Compute Coordinate Switch from Spot Frame to GC
  Vector3 zvec0 = spot_centroid.Normal();
  Vector3 yvec0 = zvec0 & look;  yvec0 = yvec0 / yvec0.Magnitude();
  Vector3 xvec0 = yvec0 & zvec0; xvec0 = xvec0 / xvec0.Magnitude();
  
  Vector3 b[2];
  b[0] = spot_centroid + xvec0 + yvec0; 
  b[1] = spot_centroid - xvec0 + yvec0;
  
  // compute baseband frequency of each point
  float bbf[2];
  for(int k=0;k<2;k++)
    bbf[k] = NominalQuikScatBaseBandFreq(b[k]);
  
  float bbfcent = NominalQuikScatBaseBandFreq(spot_centroid);
  
  // find another point on equi-freq line with centroid
  float   dbbfdx0 = ( bbf[0] - bbf[1] ) / 2.0;
  Vector3 p2      = b[1] + xvec0*((bbfcent-bbf[1])/dbbfdx0);
  
  // now set up spot frame Coordinate Switch
  // (yvec lies along iso-frequency lines)
  Vector3 zvec = zvec0;
  Vector3 yvec = p2-spot_centroid; yvec = yvec / yvec.Magnitude();
  Vector3 xvec = yvec & zvec;      xvec = xvec / xvec.Magnitude();
  
  CoordinateSwitch gc_to_spotframe(xvec,yvec,zvec);
  
  // estimate dtheta in antenna poitning due to observed frequency_shift
  Vector3 antenna_look0 = gc_to_antenna.Forward(spot_centroid - scOrbitState.rsat);

  double theta0,phi0,range0;
  antenna_look0.SphericalGet(&range0,&theta0,&phi0);
  
  Vector3 spot_centroid_adj = xvec0*(freq_shift/dbbfdx0) + spot_centroid;
  Vector3 antenna_look1     = gc_to_antenna.Forward(spot_centroid_adj - scOrbitState.rsat);

  double theta1,phi1,range1;
  antenna_look1.SphericalGet(&range1,&theta1,&phi1);

  float dtheta = theta1 - theta0;
  if( freq_shift==0 ) dtheta = 0;
  
  // Loop over Meas in this MeasSpot
  for( Meas* meas = GetHead(); meas; meas = GetNext() ) {
    // Only compute land fraction for coastal slices
    double lon,lat,alt;
    meas->centroid.GetAltLonGDLat(&alt,&lon,&lat);
    // If not near the coast, set land fraction to zero or one
    // depending on other land flag method.
    int qs_lmap_bits;
    qs_lmap_bits = qs_lmap->IsLand( lon, lat, -1 );
    if (qs_lmap_bits==0||qs_lmap_bits==255)  {
      if( meas->landFlag==1 || meas->landFlag==3 ) {
        meas->bandwidth = 1;
      } else {
        meas->bandwidth = 0;
      }
      continue;
    }
    
    float  sum     = 0;
    float  landsum = 0;
    float  bbf0    = NominalQuikScatBaseBandFreq(meas->centroid);
    float  bbf1    = NominalQuikScatBaseBandFreq(meas->centroid+xvec*0.1);
    float  bw      = meas->bandwidth;
    float  dbbfdx  = (bbf1-bbf0)/0.1;
    float  xmin    = -fabs((bw/2)/dbbfdx);
    float  xmax    =  fabs((bw/2)/dbbfdx);
    float  ymin    = -50;
    float  ymax    =  50;
    int    nxsteps = COAST_NXSTEPS;
    int    nysteps = COAST_NYSTEPS;
    double dx      = (xmax-xmin)/nxsteps;
    double dy      = (ymax-ymin)/nysteps;
    
    CoordinateSwitch spotframe_to_gc = gc_to_spotframe.ReverseDirection();
    
    for(int ix=0;ix<nxsteps;ix++){
      for(int iy=0;iy<nysteps;iy++){
        float x = xmin + dx/2 + dx*ix;
        float y = ymin + dy/2 + dy*iy;

        Vector3 pspot(x,y,0);
        EarthPosition p = spotframe_to_gc.Forward(pspot) + meas->centroid;
        
        //p.GetAltLonGDLat(&alt,&lon,&lat);
        
        lon = atan2( p.GetY(), p.GetX() );
        lat = atan( tan( asin( p.GetZ() / p.Magnitude() ) ) / (1-e2) );
        
        if( lon < 0 ) lon += two_pi;

        Vector3 antenna_look = gc_to_antenna.Forward(p - scOrbitState.rsat);
        
        double theta,phi,range,g2;
        antenna_look.SphericalGet(&range,&theta,&phi);
        rtt  = 2.0 * range /speed_light_kps;

        if( !ant->beam[meas->beamIdx].GetPowerGainProduct(
            theta, phi, rtt, ant->spinRate, &g2)) g2=0.0;

         float dW = g2*dx*dy / (range*range*range*range);
         sum     += dW;
         landsum += lmap->IsLand(lon,lat) ? dW : 0;
      } // iy loop
    }   // ix loop
    meas->bandwidth = landsum / sum; // Use EnSlice to hold land fraction values
  } // loop over Meas in MeasSpot
  return(1);
}

//==============//
// MeasSpotList //
//==============//

MeasSpotList::MeasSpotList()
{
    return;
}

MeasSpotList::~MeasSpotList()
{
    MeasSpot* meas_spot;
    GotoHead();
    while ((meas_spot=RemoveCurrent()) != NULL)
        delete meas_spot;

    return;
}

//---------------------//
// MeasSpotList::Write //
//---------------------//

int
MeasSpotList::Write(
    FILE*    fp)
{
    int count = NodeCount();
    if (fwrite((void *)&count, sizeof(int), 1, fp) != 1)
        return(0);
        
    for (MeasSpot* meas_spot = GetHead(); meas_spot; meas_spot = GetNext())
    {
        if (! meas_spot->Write(fp))
            return(0);
    }
    return(1);
}
//---------------------//
// MeasSpotList::WriteEphemeris //
//---------------------//

int
MeasSpotList::WriteEphemeris(
    FILE*	ephemeris_fp)
{
    if (ephemeris_fp != NULL)	// write the ephemeris for the first point in this frame
    {
    	MeasSpot* meas_spot = GetHead();
        if (meas_spot->scOrbitState.Write(ephemeris_fp) != 1)
        	return(0);
    }
    return(1);
}

//--------------------------//
// MeasSpotList::WriteAscii //
//--------------------------//

int
MeasSpotList::WriteAscii(
    FILE*    fp)
{
    fprintf(fp, "\n");
    fprintf(fp, "########################################\n");
    fprintf(fp, "###          Spot Count: %4d       ###\n", NodeCount());
    fprintf(fp, "########################################\n");
    fprintf(fp, "\n");
    for (MeasSpot* spot = GetHead(); spot; spot= GetNext())
    {
        if (! spot->WriteAscii(fp))
            return(0);
    }
    return(1);
}

//--------------------//
// MeasSpotList::Read //
//--------------------//

int
MeasSpotList::Read(
    FILE*    fp)
{
    FreeContents();

    int count;
    if (fread((void *)&count, sizeof(int), 1, fp) != 1)
        return(0);

    for (int i = 0; i < count; i++)
    {
        MeasSpot* new_meas_spot = new MeasSpot();
        if (! new_meas_spot->Read(fp) ||
            ! Append(new_meas_spot))
        {
            return(0);
        }
    }
    return(1);
}



//----------------------------//
// MeasSpotList::UnpackL1BHdfCoastal //
//----------------------------//

int
MeasSpotList::UnpackL1BHdfCoastal(
    L1BHdf*  l1bHdf,
    int32    hdfIndex,
    CoastalMaps* lmap,
    Antenna* ant)    // index in the HDF
{
    assert(l1bHdf != 0);
    FreeContents();

    Parameter* param = l1bHdf->GetParameter(NUM_PULSES, UNIT_DN);
    assert(param != 0);
    int num_pulses = (int)(*((char*)(param->data)));

    for (int i = 0; i < num_pulses; i++)
    {
        //-------------------------------------------------------
        // the bit 0-1 in sigma0_mode_flag must be 0 - meas pulse
        // otherwise, skip this pulse
        //-------------------------------------------------------
        param = l1bHdf->GetParameter(SIGMA0_MODE_FLAG, UNIT_DN);
        assert(param != 0);
        unsigned short* ushortP = (unsigned short*) (param->data);
        unsigned short sliceModeFlags = *(ushortP + i);
        if ((sliceModeFlags & 0x0003) != 0){
	  continue;
	}

	param = l1bHdf->GetParameter(FREQUENCY_SHIFT, UNIT_HZ);
	assert(param != 0);
	float *dfptr = (float*)(param->data);
	float df = *(dfptr+i);
 

        MeasSpot* new_meas_spot = new MeasSpot();
        if (! new_meas_spot->UnpackL1BHdf(l1bHdf, hdfIndex, i))
            return(0);

        // empty spot case
	if (new_meas_spot->NodeCount()==0){
	  delete new_meas_spot;
	  continue;
	}
#define SCAN_ANGLE_OFFSET (0.0 *dtr)
	// Compute Axes of SpotFrame 
	// x= vector from centroid of slice 1 to slice NumSlices
        // y = z cross x
	// z = surface normal at spot centroid

        // Compute Coordinate Switch from GC Frame to AntennaFrame
        CoordinateSwitch gc_to_antenna;
        Meas* firstmeas=new_meas_spot->GetHead();
        // use average of 4th and 5th slice as default spot_centroid 
        Meas* fourthmeas, *fifthmeas;
	for(int k=0;k<3;k++){
	  fourthmeas=new_meas_spot->GetNext();
	}
	fifthmeas=new_meas_spot->GetNext();
        EarthPosition spot_centroid((fourthmeas->centroid+fifthmeas->centroid)/2);


        // LATLON limits HACK
        double spcalt,spclat,spclon;
        spot_centroid.GetAltLonGDLat(&spcalt,&spclon,&spclat);

       
	//#define MIN_SPCLAT (23*dtr)
	//#define MAX_SPCLAT (32*dtr)

	//#define MIN_SPCLON (267*dtr)
	//#define MAX_SPCLON (276*dtr)

#define MIN_SPCLAT (33.5*dtr)
#define MAX_SPCLAT (40.5*dtr)

#define MIN_SPCLON (234.5*dtr)
#define MAX_SPCLON (241.5*dtr)

        if(spclat<MIN_SPCLAT  || spclon < MIN_SPCLON 
	   || spclat>MAX_SPCLAT || spclon > MAX_SPCLON){
	  delete new_meas_spot;
	  continue;
	}
        Vector3 look=spot_centroid - new_meas_spot->scOrbitState.rsat;
	float rtt=2*look.Magnitude()/speed_light_kps;
        look=look/look.Magnitude();
	ant->SetTxCenterAzimuthAngle((firstmeas->scanAngle)+SCAN_ANGLE_OFFSET);
	gc_to_antenna = AntennaFrameToGC(&(new_meas_spot->scOrbitState),
				   &(new_meas_spot->scAttitude), 
				   ant,
				   ant->txCenterAzimuthAngle);
	gc_to_antenna=gc_to_antenna.ReverseDirection();
        
        // Find SpotCentroid
	EarthPosition nominal_spot_centroid;
	
        // Compute Coordinate Switch from Spot Frame to GC
        // Hardest part is to compute the y-axis of spot frame
        // along the slice edges
    
        // First compute y as range vector, z normal, x y cross z
 
        Vector3 zvec0=spot_centroid.Normal();
        Vector3 yvec0=zvec0 & look;
	yvec0=yvec0/yvec0.Magnitude();
        Vector3 xvec0=yvec0 & zvec0;
	xvec0=xvec0/xvec0.Magnitude();

        // DEBUG TOOLS
	printf("look:%g %g %g\n",look.Get(0),look.Get(1),look.Get(2));
	printf("xvec0:%g %g %g\n",xvec0.Get(0),xvec0.Get(1),xvec0.Get(2));
	printf("yvec0:%g %g %g\n",yvec0.Get(0),yvec0.Get(1),yvec0.Get(2));
	printf("zvec0:%g %g %g\n",zvec0.Get(0),zvec0.Get(1),zvec0.Get(2));
	
	// compute four points about the centroid
        Vector3 b[2];
	b[0]=spot_centroid+xvec0+yvec0; 
	b[1]=spot_centroid-xvec0+yvec0;


        // compute basebandfrequency of each point
        float bbf[2];
	for(int k=0;k<2;k++){
	  bbf[k]=new_meas_spot->NominalQuikScatBaseBandFreq(b[k]);
	}
        // compute baseband frequency of centroid
        float bbfcent=new_meas_spot->NominalQuikScatBaseBandFreq(spot_centroid);
        // find another point on equi-freq line with centroid
        float dbbfdx0=(bbf[0]-bbf[1])/2.0;
        Vector3 p2=b[1]+xvec0*((bbfcent-bbf[1])/dbbfdx0);


        // DEBUG TOOLS
	printf("spot_centroid:%g %g %g bbf %g\n",spot_centroid.Get(0),spot_centroid.Get(1),spot_centroid.Get(2),bbfcent);
 	printf("b[0]:%g %g %g bbf %g\n",b[0].Get(0),b[0].Get(1),b[0].Get(2),bbf[0]);
	printf("b[1]:%g %g %g bbf %g\n",b[1].Get(0),b[1].Get(1),b[1].Get(2),bbf[1]);
	printf("p2:%g %g %g bbf %g\n",p2.Get(0),p2.Get(1),p2.Get(2),bbfcent);

        // now set up spot frame Coordinate Switch
        Vector3 yvec=p2-spot_centroid;
        yvec=yvec/yvec.Magnitude();
        Vector3 zvec=zvec0;
        Vector3 xvec = yvec & zvec;
        xvec=xvec/xvec.Magnitude();
       // DEBUG TOOLS
	printf("xvec:%g %g %g\n",xvec.Get(0),xvec.Get(1),xvec.Get(2));
	printf("yvec:%g %g %g\n",yvec.Get(0),yvec.Get(1),yvec.Get(2));
	printf("zvec:%g %g %g\n",zvec.Get(0),zvec.Get(1),zvec.Get(2));

        CoordinateSwitch gc_to_spotframe(xvec,yvec,zvec);

        // estimate dtheta in antenna poitning due to observed frequency_shift
	Vector3 look0=spot_centroid-new_meas_spot->scOrbitState.rsat;
        
        // Start  Debug tools
        /****
	double vx=new_meas_spot->scOrbitState.vsat.Get(0);
        double vy=new_meas_spot->scOrbitState.vsat.Get(1);
	double lx=look0.Get(0);
	double ly=look0.Get(1);
        printf("vx %g vy %g lx %g ly %g scanang %g appang %g\n", vx,vy,lx,ly,firstmeas->scanAngle*rtd,
	       atan2(ly-vy,lx-vx)*rtd);
        float r,p,y;
        new_meas_spot->scAttitude.GetRPY(&r,&p,&y);
	printf("roll %g pitch %g yaw %g\n",r*rtd,p*rtd,y*rtd);
	***/
        // end DEBUG TOOLS
        
	Vector3 antenna_look0=gc_to_antenna.Forward(look0);
	double theta0,phi0,range0;
	antenna_look0.SphericalGet(&range0,&theta0,&phi0);

        // Debug Tools
        // START DEBUG TOOLS
	printf("antenna_look0: range0 %g theta0 %g phi0 %g\n",range0,theta0*rtd,phi0*rtd);
        /***
	CoordinateSwitch gc_to_antenna_a = AntennaFrameToGC(&(new_meas_spot->scOrbitState),
				   &(new_meas_spot->scAttitude), 
				   ant,
				   -firstmeas->scanAngle);
        gc_to_antenna_a=gc_to_antenna_a.ReverseDirection();

	Vector3 antenna_looka=gc_to_antenna_a.Forward(look0);
	double theta_a,phi_a,range_a;
	antenna_looka.SphericalGet(&range_a,&theta_a,&phi_a);
	printf("antenna_looka: rangea %g thetaa %g phia %g\n",range_a,theta_a*rtd,phi_a*rtd);


	CoordinateSwitch gc_to_antenna_b = AntennaFrameToGC(&(new_meas_spot->scOrbitState),
				   &(new_meas_spot->scAttitude), 
				   ant,
				   -firstmeas->scanAngle - pi/2);

        gc_to_antenna_b=gc_to_antenna_b.ReverseDirection();

	Vector3 antenna_lookb=gc_to_antenna_b.Forward(look0);
	double theta_b,phi_b,range_b;
	antenna_lookb.SphericalGet(&range_b,&theta_b,&phi_b);
	printf("antenna_lookb: rangeb %g thetab %g phib %g\n",range_b,theta_b*rtd,phi_b*rtd);
        ****/
        // END DEBUG TOOLS

        //HACK
        //df=0;
        Vector3 spot_centroid_adj=xvec0*(df/dbbfdx0)+spot_centroid;

	Vector3 look1=spot_centroid_adj-new_meas_spot->scOrbitState.rsat;
	Vector3 antenna_look1=gc_to_antenna.Forward(look1);
	double theta1,phi1,range1;
	antenna_look1.SphericalGet(&range1,&theta1,&phi1);
	float dtheta=theta1-theta0;
        if(df==0) dtheta=0; 
        //debug tools
	printf("df %g range0 %g theta0 %g phi0 %g range1 %g theta1 %g phi1 %g\n",df,range0,theta0,phi0,range1,theta1,phi1);

	// loop over slices
        int im=0;

	float ***gain=(float***)make_array(sizeof(float),3,8,COAST_NXSTEPS,COAST_NYSTEPS);

	float lands0[8];
        for(Meas* meas=new_meas_spot->GetHead();meas;meas=new_meas_spot->GetNext()){
          // Compute Integration bounds of each Slice in Spot Frame
	  //meas->scanAngle=-(meas->scanAngle)+SCAN_ANGLE_OFFSET;
	  float sum=0.0;
	  float landsum=0.0;
          lands0[im]=0;
          float ymin=-60;
          float ymax=60;

	  float bbf0=new_meas_spot->NominalQuikScatBaseBandFreq(meas->centroid);
	  float bbf1=new_meas_spot->NominalQuikScatBaseBandFreq(meas->centroid+xvec*0.1);
	  float bw=meas->bandwidth;
	  float dbbfdx=(bbf1-bbf0)/0.1;
          float xmin=-(bw/2)/dbbfdx;
          float xmax=(bw/2)/dbbfdx;
	  if(xmin>xmax){
	    float tmp=xmin;
	    xmin=xmax;
	    xmax=tmp;
	  }
	  int nxsteps=COAST_NXSTEPS;
          double dx=(xmax-xmin)/nxsteps;
	  int nysteps=COAST_NYSTEPS;
          double dy=(ymax-ymin)/nysteps;
	  // debug tools
          // printf("dbbfdx %g xmin %g xmax %g \n ",dbbfdx,xmin,xmax);

          double sumg=0;
	  for(int ix=0;ix<nxsteps;ix++){
	    for(int iy=0;iy<nysteps;iy++){
	      float x=xmin+dx/2 + dx*ix;
	      float y=ymin+dy/2 +dy*iy;
              Vector3 pspot(x,y,0);
              // DEBUGTOOLS
              //printf("pspot %g %g %g \n",x,y,0.0);
	      EarthPosition p=gc_to_spotframe.Backward(pspot)+meas->centroid;
              //DEBUGTOOLS
              //printf("p %g %g %g \n",p.Get(0),p.Get(1),p.Get(2));
	      double lon,lat,alt;
	      p.GetAltLonGDLat(&alt,&lon,&lat);
	      int land=lmap->IsLand(lon,lat);
	      
	      Vector3 look=p-new_meas_spot->scOrbitState.rsat;
              Vector3 antenna_look=gc_to_antenna.Forward(look);
              double theta,phi,range,g2;
              antenna_look.SphericalGet(&range,&theta,&phi);

              // START DEBUG TOOLS 
              /***
              printf("range %g theta %g dtheta %g phi %g  beam_idx %d\n",range,theta*rtd,dtheta*rtd,phi*rtd,meas->beamIdx);
	      double elook,eazim;
	      ant->beam[meas->beamIdx].GetElectricalBoresight(&elook,&eazim);
              printf("elook %g eazim %g antazim %g\n",elook*rtd,eazim*rtd, ant->txCenterAzimuthAngle*rtd);
	      EarthPosition ahead1min=new_meas_spot->scOrbitState.rsat+new_meas_spot->scOrbitState.vsat*60.0;
	      double alon,alat,sclon,sclat,clon,clat,blon,blat;
	      ahead1min.GetAltLonGDLat(&alt,&alon,&alat);
	      new_meas_spot->scOrbitState.rsat.GetAltLonGDLat(&alt,&sclon,&sclat);
	      meas->centroid.GetAltLonGDLat(&alt,&clon,&clat);
              Vector3 bs_antenna;
              bs_antenna.SphericalSet(range,elook,eazim);
              EarthPosition bs=new_meas_spot->scOrbitState.rsat+gc_to_antenna.Backward(bs_antenna);
 	      bs.GetAltLonGDLat(&alt,&blon,&blat);            
	      printf("lon %g lat %g clon %g clat %g sclon %g sclat %g alon %g alat %g blon %g blat %g\n", lon*rtd,lat*rtd,
	      clon*rtd,clat*rtd,sclon*rtd,sclat*rtd,alon*rtd,alat*rtd,blon*rtd,blat*rtd); 
              ****/
              // END DEBUGTOOLS

	      if(!ant->beam[meas->beamIdx].GetPowerGainProduct(theta+dtheta,phi,rtt,ant->spinRate,&g2)) g2=0.0;

              //exit(0);
	      float dW=g2*dx*dy/(range*range*range*range);
              //printf("%g %g %g %d\n",lon*rtd,lat*rtd,dW,meas->startSliceIdx);
	      sum+=dW;
	      if(land){
		landsum+=dW;
		float landpix;
		if(lmap->lands0Read) landpix=lmap->GetPrecomputedLands0(meas,lon,lat);
		else{
		  landpix=meas->value; // no correction if lands0 not read
		}
		if(landpix<-10) landpix=meas->value; // no correction at boundaries;
                lands0[im]+=dW*landpix;
	      }

	      gain[im][ix][iy]=dW;
	      sumg+=dW;
	    }
	  }
	  for(int ix=0;ix<nxsteps;ix++){
	    for(int iy=0;iy<nysteps;iy++){
	      gain[im][ix][iy]/=sumg;
	    }
	  }
	  
          //printf("%g %d %d %g %g %g %g\n",meas->scanAngle*rtd,meas->beamIdx,meas->startSliceIdx,sum,(sum/meas->XK),meas->XK,df);
          //fflush(stdout);
          meas->EnSlice/=meas->XK;
	  meas->XK=landsum/sum;
          if(landsum>0){
	    lands0[im]/=landsum;
	  }
	  else lands0[im]=0;
	  im++;
	}



 
 
       im=0;
       for(Meas* meas=new_meas_spot->GetHead();meas;meas=new_meas_spot->GetNext()){
	          float ymin=-60;
          float ymax=60;

	  float bbf0=new_meas_spot->NominalQuikScatBaseBandFreq(meas->centroid);
	  float bbf1=new_meas_spot->NominalQuikScatBaseBandFreq(meas->centroid+xvec*0.1);
	  float bw=meas->bandwidth;
	  float dbbfdx=(bbf1-bbf0)/0.1;
          float xmin=-(bw/2)/dbbfdx;
          float xmax=(bw/2)/dbbfdx;
	  if(xmin>xmax){
	    float tmp=xmin;
	    xmin=xmax;
	    xmax=tmp;
	  }
	  int nxsteps=COAST_NXSTEPS;
          double dx=(xmax-xmin)/nxsteps;
	  int nysteps=COAST_NYSTEPS;
          double dy=(ymax-ymin)/nysteps;
	  float s0_corr=(meas->value - meas->XK*lands0[im])/(1-meas->XK);
	  if(meas->XK >= 0.5 ){
	    s0_corr=meas->value;
	    meas->landFlag=1;
	  }
	  lmap->Accumulate(meas,s0_corr,&gc_to_spotframe,gain[im],xmin,dx,nxsteps,ymin,dy,nysteps);
          // debug tool
          printf("CheckLandCorr: lands0 %g  landfrac %g  s0uncorr %g  s0corr %g\n",lands0[im],meas->XK,meas->value,s0_corr);
	  meas->value=s0_corr;
 	  im++;
       }
       free_array(gain,3,8,COAST_NXSTEPS,COAST_NYSTEPS);
       if(! Append(new_meas_spot)) return(0);

    }
    return(1);
}



//----------------------------//
// MeasSpotList::UnpackL1BHdf //
//----------------------------//

int
MeasSpotList::UnpackL1BHdf(
    int32    sd_id,
    int32	h_id,
    int *start,		// array of 3 indexes, indicating where to start in the hdf file
    int *edges)		// array of the size of each dimension to read
{
    FreeContents();

    int num_pulses = edges[1];
    int flag_edges[] = {1, num_pulses};
    
    for (int i = 0; i < num_pulses; i++)
    {
        //-------------------------------------------------------
        // the bit 0-1 in sigma0_mode_flag must be 0 - meas pulse
        // otherwise, skip this pulse
        //-------------------------------------------------------
        int cur_start[] = {start[0], i, 0};
        int cur_edges[] = {1,1};
        bool failed = false;
        GET_HDF_VAR(uint16, sigma0_mode_flag, cur_start, cur_edges, 1)

        unsigned short sliceModeFlags = (unsigned char) sigma0_mode_flag;
        if (!failed && (sliceModeFlags & 0x0003) == 0)
        {
	        MeasSpot* new_meas_spot = new MeasSpot();
	        if (! new_meas_spot->UnpackL1BHdf(sd_id, h_id, cur_start, edges) ||
	                                        ! Append(new_meas_spot))
	            return(0);
        }
    }
    return(1);
} // MeasSpotList::UnpackL1BHdf

//----------------------------//
// MeasSpotList::FreeContents //
//----------------------------//

void
MeasSpotList::FreeContents()
{
    MeasSpot* meas_spot;
    GotoHead();
    while ((meas_spot = RemoveCurrent()) != NULL)
        delete meas_spot;
    return;
}
