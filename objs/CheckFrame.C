//==========================================================//
// Copyright (C) 1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_checkframe_c[] =
	"@(#) $Id$";

#include <memory.h>
#include <malloc.h>
#include <math.h>
#include "CheckFrame.h"
#include "Constants.h"


//============//
// CheckFrame //
//============//

CheckFrame::CheckFrame()
:	time(0.0), rsat(Vector3(0.0,0.0,0.0)),
    vsat(0.0,0.0,0.0), attitude(), beamNumber(0),
    ptgr(0.0), orbitFrac(0.0), antennaAziTx(0.0), antennaAziGi(0.0),
    EsCal(0.0), deltaFreq(0.0), spinRate(0.0), txDoppler(0.0), rxGateDelay(0.0),
    XdopplerFreq(0.0), XroundTripTime(0.0),
    idx(NULL), sigma0(NULL),
    wv(NULL), XK(NULL), centroid(NULL), azimuth(NULL), incidence(NULL),
    Es(NULL), En(NULL), var_esn_slice(NULL), R(NULL), GatGar(NULL),
    slicesPerSpot(0)
{
	return;
}

CheckFrame::CheckFrame(int slices_per_spot)
{
	if (!Allocate(slices_per_spot))
	{
		fprintf(stderr,"Error allocating a CheckFrame object\n");
	}
	return;
}

CheckFrame::~CheckFrame()
{
	Deallocate();
	return;
}

//----------------------//
// CheckFrame::Allocate //
//----------------------//

int
CheckFrame::Allocate(
	int		slices_per_spot)
{
	if (slices_per_spot <= 0)
	{
		fprintf(stderr,
			"Error: Can't allocate a CheckFrame with %d slices per spot\n",
			slices_per_spot);
		return(0);
	}

	slicesPerSpot = slices_per_spot;

	//-----------------------------//
	// allocate slice measurements //
	//-----------------------------//

	idx = (int *)malloc(slicesPerSpot * sizeof(int));
	if (idx == NULL)
	{
		return(0);
	}

	sigma0 = (float *)malloc(slicesPerSpot * sizeof(float));
	if (sigma0 == NULL)
	{
		return(0);
	}
	wv = (WindVector *)malloc(slicesPerSpot * sizeof(WindVector));
	if (wv == NULL)
	{
		return(0);
	}
	XK = (float *)malloc(slicesPerSpot * sizeof(float));
	if (XK == NULL)
	{
		return(0);
	}
	centroid = (EarthPosition *)malloc(slicesPerSpot * sizeof(EarthPosition));
	if (centroid == NULL)
	{
		return(0);
	}
	azimuth = (float *)malloc(slicesPerSpot * sizeof(float));
	if (azimuth == NULL)
	{
		return(0);
	}
	incidence = (float *)malloc(slicesPerSpot * sizeof(float));
	if (incidence == NULL)
	{
		return(0);
	}
	Es = (float *)malloc(slicesPerSpot * sizeof(float));
	if (Es == NULL)
	{
		return(0);
	}
	En = (float *)malloc(slicesPerSpot * sizeof(float));
	if (En == NULL)
	{
		return(0);
	}
	var_esn_slice = (float *)malloc(slicesPerSpot * sizeof(float));
	if (var_esn_slice == NULL)
	{
		return(0);
	}
	R = (float *)malloc(slicesPerSpot * sizeof(float));
	if (R == NULL)
	{
		return(0);
	}
	GatGar = (float *)malloc(slicesPerSpot * sizeof(float));
	if (GatGar == NULL)
	{
		return(0);
	}

    Initialize();

	return(1);
}

//------------------------//
// CheckFrame::Deallocate //
//------------------------//

int
CheckFrame::Deallocate()
{
	if (slicesPerSpot > 0)
	{
		if (idx) free(idx);
		if (sigma0) free(sigma0);
		if (wv) free(wv);
		if (XK) free(XK);
		if (centroid) free(centroid);
		if (azimuth) free(azimuth);
		if (incidence) free(incidence);
		if (Es) free(Es);
		if (En) free(En);
		if (var_esn_slice) free(var_esn_slice);
		if (R) free(R);
		if (GatGar) free(GatGar);
	}

	slicesPerSpot = 0;
	idx = NULL;
	sigma0 = NULL;
	wv = NULL;
	XK = NULL;
	centroid = NULL;
	azimuth = NULL;
	incidence = NULL;
	Es = NULL;
	En = NULL;
	var_esn_slice = NULL;
	R = NULL;
	GatGar = NULL;
	
	return(1);
}

//------------------//
// CheckFrame::Size //
//------------------//

int
CheckFrame::Size()
{

  // First, the spot quantities
  int size = 7*sizeof(double);
  size += sizeof(int);
  size += 14*sizeof(float);

  // Next, the slice quantities
  size += 3*slicesPerSpot*sizeof(double); // centroid
  size += slicesPerSpot*sizeof(int);      // idx
  size += 11*slicesPerSpot*sizeof(float); // the rest

  return(size);

}

//------------------------//
// CheckFrame::Initialize //
// Prepares for data.     //
//------------------------//

int
CheckFrame::Initialize()

{

  if (idx == NULL)
  {
    return(0);
  }

  // idx needs to start out zeroed
  (void)memset(idx, 0, slicesPerSpot*sizeof(int));

  return(1);
}

//--------------------------//
// CheckFrame::AppendRecord //
//--------------------------//

int
CheckFrame::AppendRecord(
	FILE*	fptr)
{

        float att;

        if (fwrite((void *)&time,sizeof(double),1,fptr) != 1) return(0);
        att = rtd*attitude.GetRoll(); 
        if (fwrite((void *)&att,sizeof(float),1,fptr) != 1) return(0);
        att = rtd*attitude.GetPitch();//
        if (fwrite((void *)&att,sizeof(float),1,fptr) != 1) return(0);
        att = rtd*attitude.GetYaw();//
        if (fwrite((void *)&att,sizeof(float),1,fptr) != 1) return(0);
        if (! rsat.Write(fptr)) return(0);
        if (! vsat.Write(fptr)) return(0);
        if (fwrite((void *)&beamNumber,sizeof(int),1,fptr) != 1) return(0);
        if (fwrite((void *)&ptgr,sizeof(float),1,fptr) != 1) return(0);
        if (fwrite((void *)&orbitFrac,sizeof(float),1,fptr) != 1) return(0);
        if (fwrite((void *)&antennaAziTx,sizeof(float),1,fptr) != 1) return(0);
        if (fwrite((void *)&antennaAziGi,sizeof(float),1,fptr) != 1) return(0);
        if (fwrite((void *)&EsCal,sizeof(float),1,fptr) != 1) return(0);
        if (fwrite((void *)&deltaFreq,sizeof(float),1,fptr) != 1) return(0);
        if (fwrite((void *)&spinRate,sizeof(float),1,fptr) != 1) return(0);
        if (fwrite((void *)&txDoppler,sizeof(float),1,fptr) != 1) return(0);
        if (fwrite((void *)&rxGateDelay,sizeof(float),1,fptr) != 1) return(0);
        if (fwrite((void *)&XdopplerFreq,sizeof(float),1,fptr) != 1) return(0);
        if (fwrite((void *)&XroundTripTime,sizeof(float),1,fptr) != 1)return(0);
	return(1);
}

int
CheckFrame::AppendSliceRecord(
	FILE*   fptr, 
	int     slice_i,     
        double  lon, double lat )
{
        if (fwrite((void *)&idx[slice_i],sizeof(int),1,fptr) != 1) return(0);
        if (fwrite((void *)&sigma0[slice_i],sizeof(float),1,fptr) != 1) return(0);
        if (fwrite((void *)&wv[slice_i].spd,sizeof(float),1,fptr) != 1) return(0);
        if (fwrite((void *)&wv[slice_i].dir,sizeof(float),1,fptr) != 1) return(0);
        if (fwrite((void *)&XK[slice_i],sizeof(float),1,fptr) != 1) return(0);
        if (fwrite((void *)&azimuth[slice_i],sizeof(float),1,fptr) != 1) return(0);
        if (fwrite((void *)&incidence[slice_i],sizeof(float),1,fptr) != 1) return(0);
        if (fwrite((void *)&lon,sizeof(double),1,fptr) != 1) return(0);
        if (fwrite((void *)&lat,sizeof(double),1,fptr) != 1) return(0);
        if (fwrite((void *)&var_esn_slice[slice_i],sizeof(float),1,fptr) != 1)
			return(0);
        if (fwrite((void *)&Es[slice_i],sizeof(float),1,fptr) != 1)
			return(0);
        if (fwrite((void *)&En[slice_i],sizeof(float),1,fptr) != 1)
			return(0);
        if (fwrite((void *)&R[slice_i],sizeof(float),1,fptr) != 1)
			return(0);
        if (fwrite((void *)&GatGar[slice_i],sizeof(float),1,fptr) != 1)
			return(0);
        return(1);
}

//--------------------------//
// CheckFrame::WriteDataRec //
//--------------------------//

int
CheckFrame::WriteDataRec(
	FILE*	fptr)
{

  for (int slice_i=0; slice_i < slicesPerSpot; slice_i++)
  {
    if (fwrite((void *)&idx[slice_i],sizeof(int),1,fptr) != 1) return(0);
    if (fwrite((void *)&sigma0[slice_i],sizeof(float),1,fptr) != 1) return(0);
    if (fwrite((void *)&wv[slice_i].spd,sizeof(float),1,fptr) != 1) return(0);
    if (fwrite((void *)&wv[slice_i].dir,sizeof(float),1,fptr) != 1) return(0);
    if (fwrite((void *)&XK[slice_i],sizeof(float),1,fptr) != 1) return(0);
    if (fwrite((void *)&azimuth[slice_i],sizeof(float),1,fptr) != 1) return(0);
    if (fwrite((void *)&incidence[slice_i],sizeof(float),1,fptr)!=1) return(0);
    if (! centroid[slice_i].Write(fptr)) return(0);
    if (fwrite((void *)&var_esn_slice[slice_i],sizeof(float),1,fptr) != 1)
	  return(0);
    if (fwrite((void *)&Es[slice_i],sizeof(float),1,fptr) != 1)
	  return(0);
    if (fwrite((void *)&En[slice_i],sizeof(float),1,fptr) != 1)
	  return(0);
    if (fwrite((void *)&R[slice_i],sizeof(float),1,fptr) != 1)
	  return(0);
    if (fwrite((void *)&GatGar[slice_i],sizeof(float),1,fptr) != 1)
	  return(0);
  }

  float att;

  if (fwrite((void *)&time,sizeof(double),1,fptr) != 1) return(0);
  att = rtd*attitude.GetRoll(); 
  if (fwrite((void *)&att,sizeof(float),1,fptr) != 1) return(0);
  att = rtd*attitude.GetPitch();//
  if (fwrite((void *)&att,sizeof(float),1,fptr) != 1) return(0);
  att = rtd*attitude.GetYaw();//
  if (fwrite((void *)&att,sizeof(float),1,fptr) != 1) return(0);
  if (! rsat.Write(fptr)) return(0);
  if (! vsat.Write(fptr)) return(0);
  if (fwrite((void *)&beamNumber,sizeof(int),1,fptr) != 1) return(0);
  if (fwrite((void *)&ptgr,sizeof(float),1,fptr) != 1) return(0);
  if (fwrite((void *)&orbitFrac,sizeof(float),1,fptr) != 1) return(0);
  if (fwrite((void *)&antennaAziTx,sizeof(float),1,fptr) != 1) return(0);
  if (fwrite((void *)&antennaAziGi,sizeof(float),1,fptr) != 1) return(0);
  if (fwrite((void *)&EsCal,sizeof(float),1,fptr) != 1) return(0);
  if (fwrite((void *)&deltaFreq,sizeof(float),1,fptr) != 1) return(0);
  if (fwrite((void *)&spinRate,sizeof(float),1,fptr) != 1) return(0);
  if (fwrite((void *)&txDoppler,sizeof(float),1,fptr) != 1) return(0);
  if (fwrite((void *)&rxGateDelay,sizeof(float),1,fptr) != 1) return(0);
  if (fwrite((void *)&XdopplerFreq,sizeof(float),1,fptr) != 1) return(0);
  if (fwrite((void *)&XroundTripTime,sizeof(float),1,fptr) != 1)return(0);
  return(1);
}

int
CheckFrame::ReadDataRec(
	FILE*	fptr)
{

  for (int slice_i=0; slice_i < slicesPerSpot; slice_i++)
  {
    if (fread((void *)&idx[slice_i],sizeof(int),1,fptr) != 1) return(0);
    if (fread((void *)&sigma0[slice_i],sizeof(float),1,fptr) != 1) return(0);
    if (fread((void *)&(wv[slice_i].spd),sizeof(float),1,fptr) != 1) return(0);
    if (fread((void *)&(wv[slice_i].dir),sizeof(float),1,fptr) != 1) return(0);
    if (fread((void *)&XK[slice_i],sizeof(float),1,fptr) != 1) return(0);
    if (fread((void *)&azimuth[slice_i],sizeof(float),1,fptr) != 1) return(0);
    if (fread((void *)&incidence[slice_i],sizeof(float),1,fptr) != 1) return(0);
    if (! centroid[slice_i].Read(fptr)) return(0);
    if (fread((void *)&var_esn_slice[slice_i],sizeof(float),1,fptr) != 1)
      return(0);
    if (fread((void *)&Es[slice_i],sizeof(float),1,fptr) != 1)
      return(0);
    if (fread((void *)&En[slice_i],sizeof(float),1,fptr) != 1)
      return(0);
    if (fread((void *)&R[slice_i],sizeof(float),1,fptr) != 1)
      return(0);
    if (fread((void *)&GatGar[slice_i],sizeof(float),1,fptr) != 1)
      return(0);
  }

  float roll,pitch,yaw;
  if (fread((void *)&time,sizeof(double),1,fptr) != 1) return(0);
  if (fread((void *)&roll,sizeof(float),1,fptr) != 1) return(0);
  if (fread((void *)&pitch,sizeof(float),1,fptr) != 1) return(0);
  if (fread((void *)&yaw,sizeof(float),1,fptr) != 1) return(0);
  if (! rsat.Read(fptr)) return(0);
  if (! vsat.Read(fptr)) return(0);
  if (fread((void *)&beamNumber,sizeof(int),1,fptr) != 1) return(0);
  if (fread((void *)&ptgr,sizeof(float),1,fptr) != 1) return(0);
  if (fread((void *)&orbitFrac,sizeof(float),1,fptr) != 1) return(0);
  if (fread((void *)&antennaAziTx,sizeof(float),1,fptr) != 1) return(0);
  if (fread((void *)&antennaAziGi,sizeof(float),1,fptr) != 1) return(0);
  if (fread((void *)&EsCal,sizeof(float),1,fptr) != 1) return(0);
  if (fread((void *)&deltaFreq,sizeof(float),1,fptr) != 1) return(0);
  if (fread((void *)&spinRate,sizeof(float),1,fptr) != 1) return(0);
  if (fread((void *)&txDoppler,sizeof(float),1,fptr) != 1) return(0);
  if (fread((void *)&rxGateDelay,sizeof(float),1,fptr) != 1) return(0);
  if (fread((void *)&XdopplerFreq,sizeof(float),1,fptr) != 1) return(0);
  if (fread((void *)&XroundTripTime,sizeof(float),1,fptr) != 1)return(0);
  attitude.Set(dtr*roll,dtr*pitch,dtr*yaw,1,2,3);

return(1);
}

int
CheckFrame::ReadDataRecFortran(
	FILE*	fptr)
{

  //----------------------------------------------------------------------//
  // Same as ReadDataRec, except this one reads unformatted fortran binary
  // records which place record sizes before and after every element.
  //----------------------------------------------------------------------//

  for (int slice_i=0; slice_i < slicesPerSpot; slice_i++)
  {
    float x,y,z;
    if (fread_f77((void *)&idx[slice_i],sizeof(int),1,fptr) != 1) return(0);
    if (fread_f77((void *)&sigma0[slice_i],sizeof(float),1,fptr) != 1) return(0);
    if (fread_f77((void *)&(wv[slice_i].spd),sizeof(float),1,fptr) != 1) return(0);
    if (fread_f77((void *)&(wv[slice_i].dir),sizeof(float),1,fptr) != 1) return(0);
    if (fread_f77((void *)&XK[slice_i],sizeof(float),1,fptr) != 1) return(0);
    if (fread_f77((void *)&azimuth[slice_i],sizeof(float),1,fptr) != 1) return(0);
    if (fread_f77((void *)&incidence[slice_i],sizeof(float),1,fptr) != 1) return(0);
    if (fread_f77((void *)&x,sizeof(float),1,fptr) != 1) return(0);
    if (fread_f77((void *)&y,sizeof(float),1,fptr) != 1) return(0);
    if (fread_f77((void *)&z,sizeof(float),1,fptr) != 1) return(0);
    centroid[slice_i].Set(x,y,z);
    if (fread_f77((void *)&var_esn_slice[slice_i],sizeof(float),1,fptr) != 1)
      return(0);
    if (fread_f77((void *)&Es[slice_i],sizeof(float),1,fptr) != 1)
      return(0);
    if (fread_f77((void *)&En[slice_i],sizeof(float),1,fptr) != 1)
      return(0);
    if (fread_f77((void *)&R[slice_i],sizeof(float),1,fptr) != 1)
      return(0);
    if (fread_f77((void *)&GatGar[slice_i],sizeof(float),1,fptr) != 1)
      return(0);
  }

  float roll,pitch,yaw;
  double x,y,z;
  if (fread_f77((void *)&time,sizeof(double),1,fptr) != 1) return(0);
  if (fread_f77((void *)&roll,sizeof(float),1,fptr) != 1) return(0);
  if (fread_f77((void *)&pitch,sizeof(float),1,fptr) != 1) return(0);
  if (fread_f77((void *)&yaw,sizeof(float),1,fptr) != 1) return(0);
  if (fread_f77((void *)&x,sizeof(double),1,fptr) != 1) return(0);
  if (fread_f77((void *)&y,sizeof(double),1,fptr) != 1) return(0);
  if (fread_f77((void *)&z,sizeof(double),1,fptr) != 1) return(0);
  rsat.Set(x,y,z);
  if (fread_f77((void *)&x,sizeof(double),1,fptr) != 1) return(0);
  if (fread_f77((void *)&y,sizeof(double),1,fptr) != 1) return(0);
  if (fread_f77((void *)&z,sizeof(double),1,fptr) != 1) return(0);
  vsat.Set(x,y,z);
  if (fread_f77((void *)&beamNumber,sizeof(int),1,fptr) != 1) return(0);
  if (fread_f77((void *)&ptgr,sizeof(float),1,fptr) != 1) return(0);
  if (fread_f77((void *)&orbitFrac,sizeof(float),1,fptr) != 1) return(0);
  if (fread_f77((void *)&antennaAziTx,sizeof(float),1,fptr) != 1) return(0);
  if (fread_f77((void *)&antennaAziGi,sizeof(float),1,fptr) != 1) return(0);
  if (fread_f77((void *)&EsCal,sizeof(float),1,fptr) != 1) return(0);
  if (fread_f77((void *)&deltaFreq,sizeof(float),1,fptr) != 1) return(0);
  if (fread_f77((void *)&spinRate,sizeof(float),1,fptr) != 1) return(0);
  if (fread_f77((void *)&txDoppler,sizeof(float),1,fptr) != 1) return(0);
  if (fread_f77((void *)&rxGateDelay,sizeof(float),1,fptr) != 1) return(0);
  if (fread_f77((void *)&XdopplerFreq,sizeof(float),1,fptr) != 1) return(0);
  if (fread_f77((void *)&XroundTripTime,sizeof(float),1,fptr) != 1)return(0);
  attitude.Set(dtr*roll,dtr*pitch,dtr*yaw,1,2,3);

return(1);
}

int
CheckFrame::WriteDataRecAscii(
	FILE*	fptr)
{

  double alt,lon,lat;
  fprintf(fptr,"**** Spot Data ****\n");
  fprintf(fptr,"time (sec): %g\n",time);
  fprintf(fptr,"beam number: %d\n",beamNumber);
  fprintf(fptr,"PtGr: %6g\n",ptgr);
  float roll,pitch,yaw;
  attitude.GetRPY(&roll,&pitch,&yaw);
  fprintf(fptr,"S/C roll,pitch,yaw (deg): %g %g %g\n",
    rtd*roll,rtd*pitch,rtd*yaw);
  fprintf(fptr,"S/C Tx position (x,y,z km): %g %g %g\n",
    rsat.Get(0),rsat.Get(1),rsat.Get(2));
  fprintf(fptr,"S/C Tx velocity (x,y,z km/s): %g %g %g\n",
    vsat.Get(0),vsat.Get(1),vsat.Get(2));
  fprintf(fptr,"orbit fraction (from ascending node): %g\n",orbitFrac);
  fprintf(fptr,"antenna azimuth Tx (rel to s/c y-axis. (deg)): %g\n",
    rtd*antennaAziTx);
  fprintf(fptr,"antenna azimuth ground impact (rel to s/c y-axis. (deg)): %g\n",
    rtd*antennaAziGi);
  fprintf(fptr,"EsCal (DN): %g\n",EsCal);
  fprintf(fptr,"deltaFreq (Hz): %g\n",deltaFreq);
  fprintf(fptr,"spinRate (rad/s): %g\n",spinRate);
  fprintf(fptr,"txDoppler (Hz): %g\n",txDoppler);
  fprintf(fptr,"rxGateDelay (sec): %g\n",rxGateDelay);
  fprintf(fptr,"XdopplerFreq (Hz): %g\n",XdopplerFreq);
  fprintf(fptr,"XroundTripTime (sec): %g\n",XroundTripTime);
  fprintf(fptr,"**** Slices Data ****\n");
  int sliceno = -5;
  for (int i=0; i < slicesPerSpot; i++)
  {
    if (idx[i] < -6 || idx[i] > 6 || idx[i] == 0) continue;  // invalid data
    fprintf(fptr,"  Slice %d\n",idx[i]);
    fprintf(fptr,"    wind spd (m/s), dir (deg): %g %g\n",wv[i].spd,
      rtd*wv[i].dir);
    fprintf(fptr,"    sigma0: %g\n",sigma0[i]); 
    fprintf(fptr,"    XK: %8g\n",XK[i]); 
    fprintf(fptr,"    Es (J): %g\n",Es[i]); 
    fprintf(fptr,"    En (J): %g\n",En[i]); 
    float snr = Es[i] / En[i];
    float snrdB = 10.0*log(snr)/log(10.0);
    fprintf(fptr,"    SNR (dB): %g\n",snrdB); 
    fprintf(fptr,"    variance of Es+n: %g\n",var_esn_slice[i]); 
    float ekpc2 = var_esn_slice[i] / (Es[i]*Es[i]);
    float ekpcdB = 10.0*log(sqrt(ekpc2)+1.0)/log(10.0);
	float A = 1.0/(8314.0 * 0.0015);
	float B = 2.0/(8314.0 * 0.0020);
	float C = B/2.0 * (1.0 + 8314.0/1e6);
	float kpc2 = A + B/snr + C/snr/snr;
    float kpcdB = 10.0*log(sqrt(kpc2)+1.0)/log(10.0);
    fprintf(fptr,"    Kpc(A,B,C), est Kpc (dB): %g %g\n",kpcdB,ekpcdB); 
    fprintf(fptr,"    azimuth (deg): %g\n",rtd*azimuth[i]); 
    fprintf(fptr,"    incidence (deg): %g\n",rtd*incidence[i]); 
    centroid[i].GetAltLonGDLat(&alt,&lon,&lat);
    fprintf(fptr,"    lon,lat of centroid (deg): %g %g\n",rtd*lon,rtd*lat); 
	fprintf(fptr,"    slant range (km): %g\n",R[i]);
    fprintf(fptr,"    GatGar (dB): %g\n",10.0*log(GatGar[i])/log(10.0));
    sliceno++;
    if (sliceno == 0) sliceno = 1;
  }

  return(1);

}
