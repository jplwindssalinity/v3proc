//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

static const char rcs_id_measurement_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include <unistd.h>
#include "Meas.h"
#include "Beam.h"
#include "Constants.h"
#include "LonLat.h"
#include "GenericGeom.h"


//======//
// Meas //
//======//

Meas::Meas()
:	value(0.0), XK(0.0), EnSlice(0.0), bandwidth(0.0),
	txPulseWidth(0.0), pol(NONE), eastAzimuth(0.0), incidenceAngle(0.0),
	beamIdx(-1), startSliceIdx(-1), numSlices(0), scanAngle(0.0),
	A(0.0), B(0.0), C(0.0), offset(0)
{
	return;
}

Meas::~Meas()
{
	return;
}

//-----------------//
// Meas::Composite //
//-----------------//
// Combine measurements into one composite sigma0 and Kpc coefficients.
// The input measurement list should all come from one spot, but this
// routine does not (and can not) check for this.
// Meas is set with the final composite measurement.

int
Meas::Composite(
	MeasList*	meas_list,
	int			n)
{
	float sum_Ps = 0.0;
	float sum_XK = 0.0;
	float sum_EnSlice = 0.0;
	float sum_bandwidth = 0.0;
	Vector3 sum_centroid(0.0,0.0,0.0);
	float sum_incidenceAngle = 0.0;
//	float sum_X2 = 0.0;
	float sum_xk2a = 0.0;
	float sum_xkbwb = 0.0;
	float sum_bw2c = 0.0;
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

	for (meas = meas_start; meas; meas = meas_list->GetNext())
	{
		//==================================================//
		// Only composites n consecutive objects if default //
		// parameter n is nonzero							//
		//==================================================//
		if(n != 0 && N >= n)
			break;

		sum_Ps += meas->value * meas->XK;
		sum_XK += meas->XK;
		sum_EnSlice += meas->EnSlice;
		sum_bandwidth += meas->bandwidth;
		sum_centroid += meas->centroid * meas->XK;
		sum_incidenceAngle += meas->XK * meas->incidenceAngle;
		sum_xk2a += meas->XK * meas->XK * meas->A;
		sum_xkbwb += meas->XK * meas->bandwidth * meas->B;
		sum_bw2c += meas->bandwidth * meas->bandwidth * meas->C;
//		sum_X2 += meas->XK*meas->XK;
		N++;
	}

	meas = meas_list->GetHead();


	//---------------------------------------------------------------------        //
	// Form the composite measurement from appropriate combinations of the
	// elements of each slice measurement in this composite cell.
	//---------------------------------------------------------------------//

	value = sum_Ps / sum_XK;

	XK = sum_XK;
	EnSlice = sum_EnSlice;
	bandwidth = sum_bandwidth;
	txPulseWidth = meas->txPulseWidth;
	outline.FreeContents();				// merged outlines not done yet
	// Weighted average of centroids (weighted by XK)
	centroid = sum_centroid / sum_XK;
	// put centroid on surface
	double alt, lon, lat;
	centroid.GetAltLonGDLat(&alt, &lon, &lat);
	centroid.SetAltLonGDLat(0.0, lon, lat);
	pol = meas->pol;					// same for all slices
	eastAzimuth = meas->eastAzimuth;	// same for all slices
	// Weighted average of incidence angles (weighted by XK)
	incidenceAngle = sum_incidenceAngle / sum_XK;
	beamIdx = meas->beamIdx;			// same for all slices
	startSliceIdx = min_slice_idx;
	scanAngle = meas->scanAngle;		// same for all slices
	numSlices = N;

	//----------------------------//
	// composite Kpc coefficients //
	//----------------------------//
	// Composite Kpc coefficients.
	// Derived from Mike's equations but not assuming that the A, B, C, and
	// bandwidths for each slice are identical.

	A = sum_xk2a / (sum_XK * sum_XK);
	B = sum_xkbwb / (bandwidth * sum_XK);
	C = sum_bw2c / (bandwidth * bandwidth);

/*
	A = meas->A * sum_X2 / (sum_XK * sum_XK);
	B = meas->B / N;
	C = meas->C / N;
*/

	return(1);
}

//-------------//
// Meas::Write //
//-------------//

int
Meas::Write(
	FILE*	fp)
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
		outline.Write(fp) != 1 ||
		centroid.WriteLonLat(fp) != 1 ||
		fwrite((void *)&pol, sizeof(PolE), 1, fp) != 1 ||
		fwrite((void *)&eastAzimuth, sizeof(float), 1, fp) != 1 ||
		fwrite((void *)&incidenceAngle, sizeof(float), 1, fp) != 1 ||
		fwrite((void *)&beamIdx, sizeof(int), 1, fp) != 1 ||
		fwrite((void *)&startSliceIdx, sizeof(int), 1, fp) != 1 ||
		fwrite((void *)&numSlices, sizeof(int), 1, fp) != 1 ||
		fwrite((void *)&scanAngle, sizeof(float), 1, fp) != 1 ||
		fwrite((void *)&A, sizeof(float), 1, fp) != 1 ||
		fwrite((void *)&B, sizeof(float), 1, fp) != 1 ||
		fwrite((void *)&C, sizeof(float), 1, fp) != 1)
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
	FILE*	fp)
{
	FreeContents();
	offset = ftell(fp);
	if (fread((void *)&value, sizeof(float), 1, fp) != 1 ||
		fread((void *)&XK, sizeof(float), 1, fp) != 1 ||
		fread((void *)&EnSlice, sizeof(float), 1, fp) != 1 ||
		fread((void *)&bandwidth, sizeof(float), 1, fp) != 1 ||
		fread((void *)&txPulseWidth, sizeof(float), 1, fp) != 1 ||
		outline.Read(fp) != 1 ||
		centroid.ReadLonLat(fp) != 1 ||
		fread((void *)&pol, sizeof(PolE), 1, fp) != 1 ||
		fread((void *)&eastAzimuth, sizeof(float), 1, fp) != 1 ||
		fread((void *)&incidenceAngle, sizeof(float), 1, fp) != 1 ||
		fread((void *)&beamIdx, sizeof(int), 1, fp) != 1 ||
		fread((void *)&startSliceIdx, sizeof(int), 1, fp) != 1 ||
		fread((void *)&numSlices, sizeof(int), 1, fp) != 1 ||
		fread((void *)&scanAngle, sizeof(float), 1, fp) != 1 ||
		fread((void *)&A, sizeof(float), 1, fp) != 1 ||
		fread((void *)&B, sizeof(float), 1, fp) != 1 ||
		fread((void *)&C, sizeof(float), 1, fp) != 1)
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
	FILE*	fp)
{
        double lon=0, lat=0, alt=0; 
        centroid.GetAltLonGCLat(&alt,&lon,&lat);
        lon*=rtd;
        lat*=rtd;
        fprintf(fp,"###########  Slice/Composite  Info  ###############\n");
        fprintf(fp, "BeamIdx: %d ", beamIdx);
        fprintf(fp, "StartSliceIdx: %d ", startSliceIdx);
        fprintf(fp, "NumSlices: %d ", numSlices);
	fprintf(fp, "ScanAngle: %g ", scanAngle*rtd);
        fprintf(fp, "XK: %g\n",XK);
	fprintf(fp, "Pol: %s ", beam_map[(int)pol]);
	fprintf(fp, "IncAngle: %g ", incidenceAngle*rtd);
        fprintf(fp, "Longitude: %g ",lon);
        fprintf(fp, "Latitude: %g ",lat);
	fprintf(fp, "Value: %g\n", value);
        fprintf(fp, "Bandwidth: %g ", bandwidth);
        fprintf(fp, "txPulseWidth: %g ", txPulseWidth);
        fprintf(fp, "A: %g ", A);
        fprintf(fp, "B: %g ", B);
        fprintf(fp, "C: %g ", C);
        fprintf(fp, "EnSlice: %g\n", EnSlice);
    

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
	FILE*	fp)
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
	FILE*	fp)
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
	FILE*	fp)
{
	fprintf(fp, "\n### Slice/Composite Count: %d ####\n", NodeCount());
	for (Meas* meas = GetHead(); meas; meas = GetNext())
	{
		if (! meas->WriteAscii(fp))
			return(0);
	}
	return(1);
}

//-------------------------//
// MeasList::AverageLonLat //
//-------------------------//

LonLat
MeasList::AverageLonLat()
{
	EarthPosition sum;
	sum.SetPosition(0.0, 0.0, 0.0);
	for (Meas* meas = GetHead(); meas; meas = GetNext())
	{
		sum += meas->centroid;
	}

	// The center of the earth is at 0,0,0 (geocentric coords)
	EarthPosition earth_center;
	earth_center.SetPosition(0.0, 0.0, 0.0);
	// Find the surface point lying along the averaged direction.
	EarthPosition ravg = earth_intercept(earth_center,sum);

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
	FILE*		fp,
	MeasList*	meas_list)
{
	for (long* offset = GetHead(); offset; offset = GetNext())
	{
		Meas* meas = new Meas();
		if (fseek(fp, *offset, SEEK_SET) == -1)
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
	long* offset;
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
:	time(0.0), scOrbitState(), scAttitude()
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
	FILE*	fp)
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
	FILE*	fp)
{       
        fprintf(fp,"\n##############################################\n");
        fprintf(fp,"#######         Spot Info               ######\n");
        fprintf(fp,"##############################################\n");
        fprintf(fp,"\n");
        fprintf(fp,"Time: %g\n", time);
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
	FILE*	fp)
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
	FILE*	fp)
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

//--------------------------//
// MeasSpotList::WriteAscii //
//--------------------------//

int
MeasSpotList::WriteAscii(
	FILE*	fp)
{
        fprintf(fp, "\n########################################\n");
	fprintf(fp, "###          Spot Count: %4d       ####\n", NodeCount());
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
	FILE*	fp)
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
