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
:	slicesPerSpot(0)
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
	true_Es = (float *)malloc(slicesPerSpot * sizeof(float));
	if (true_Es == NULL)
	{
		return(0);
	}
	true_En = (float *)malloc(slicesPerSpot * sizeof(float));
	if (true_En == NULL)
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
		if (sigma0) free(sigma0);
		if (wv) free(wv);
		if (XK) free(XK);
		if (centroid) free(centroid);
		if (azimuth) free(azimuth);
		if (incidence) free(incidence);
		if (true_Es) free(true_Es);
		if (true_En) free(true_En);
		if (var_esn_slice) free(var_esn_slice);
		if (R) free(R);
		if (GatGar) free(GatGar);
	}

	slicesPerSpot = 0;
	sigma0 = NULL;
	wv = NULL;
	XK = NULL;
	centroid = NULL;
	azimuth = NULL;
	incidence = NULL;
	true_Es = NULL;
	true_En = NULL;
	var_esn_slice = NULL;
	R = NULL;
	GatGar = NULL;
	
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
        if (fwrite((void *)&ptgr,sizeof(float),1,fptr) != 1) return(0);
        if (fwrite((void *)&orbit_frac,sizeof(float),1,fptr) != 1) return(0);
        if (fwrite((void *)&antenna_azi,sizeof(float),1,fptr) != 1) return(0);
	return(1);
}

int
CheckFrame::AppendSliceRecord(
	FILE*   fptr, 
	int     slice_i,     
        double  lon, double lat )
{
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
        if (fwrite((void *)&true_Es[slice_i],sizeof(float),1,fptr) != 1)
			return(0);
        if (fwrite((void *)&true_En[slice_i],sizeof(float),1,fptr) != 1)
			return(0);
        if (fwrite((void *)&R[slice_i],sizeof(float),1,fptr) != 1)
			return(0);
        if (fwrite((void *)&GatGar[slice_i],sizeof(float),1,fptr) != 1)
			return(0);
        return(1);
}

int
CheckFrame::ReadDataRec(
	FILE*	fptr)
{

  double lon,lat;
  for (int slice_i=0; slice_i < 10; slice_i++)
  {
    if (fread((void *)&sigma0[slice_i],sizeof(float),1,fptr) != 1) return(0);
    if (fread((void *)&(wv[slice_i].spd),sizeof(float),1,fptr) != 1) return(0);
    if (fread((void *)&(wv[slice_i].dir),sizeof(float),1,fptr) != 1) return(0);
    if (fread((void *)&XK[slice_i],sizeof(float),1,fptr) != 1) return(0);
    if (fread((void *)&azimuth[slice_i],sizeof(float),1,fptr) != 1) return(0);
    if (fread((void *)&incidence[slice_i],sizeof(float),1,fptr) != 1) return(0);
    if (fread((void *)&lon,sizeof(double),1,fptr) != 1) return(0);
    if (fread((void *)&lat,sizeof(double),1,fptr) != 1) return(0);
    centroid[slice_i].SetAltLonGDLat(0.0,lon,lat);
    if (fread((void *)&var_esn_slice[slice_i],sizeof(float),1,fptr) != 1)
      return(0);
    if (fread((void *)&true_Es[slice_i],sizeof(float),1,fptr) != 1)
      return(0);
    if (fread((void *)&true_En[slice_i],sizeof(float),1,fptr) != 1)
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
  if (fread((void *)&ptgr,sizeof(float),1,fptr) != 1) return(0);
  if (fread((void *)&orbit_frac,sizeof(float),1,fptr) != 1) return(0);
  if (fread((void *)&antenna_azi,sizeof(float),1,fptr) != 1) return(0);
  attitude.Set(dtr*roll,dtr*pitch,dtr*yaw,1,2,3);

return(1);
}

int
CheckFrame::WriteDataRecAscii(
	FILE*	fptr)
{

  fprintf(fptr,"**** Spot Data ****\n");
  fprintf(fptr,"time (sec): %g\n",time);
  fprintf(fptr,"PtGr: %6g\n",ptgr);
  float roll,pitch,yaw;
  attitude.GetRPY(&roll,&pitch,&yaw);
  fprintf(fptr,"S/C roll,pitch,yaw (deg): %g %g %g\n",
    rtd*roll,rtd*pitch,rtd*yaw);
  fprintf(fptr,"orbit fraction (from ascending node): %g\n",orbit_frac);
  fprintf(fptr,"antenna azimuth (rel to s/c y-axis. (deg)): %g\n",
    rtd*antenna_azi);
  fprintf(fptr,"**** Slices Data ****\n");
  double alt,lon,lat;
  int sliceno = -5;
  for (int i=0; i < 10; i++)
  {
    fprintf(fptr,"  Slice %d\n",sliceno);
    fprintf(fptr,"    wind spd (m/s), dir (deg): %g %g\n",wv[i].spd,
      rtd*wv[i].dir);
    fprintf(fptr,"    sigma0: %g\n",sigma0[i]); 
    fprintf(fptr,"    XK: %8g\n",XK[i]); 
    fprintf(fptr,"    true_Es (J): %g\n",true_Es[i]); 
    fprintf(fptr,"    true_En (J): %g\n",true_En[i]); 
    float true_snr = true_Es[i] / true_En[i];
    float true_snrdB = 10.0*log(true_snr)/log(10.0);
    fprintf(fptr,"    true SNR (dB): %g\n",true_snrdB); 
    fprintf(fptr,"    variance of Es+n: %g\n",var_esn_slice[i]); 
    float ekpc2 = var_esn_slice[i] / (true_Es[i]*true_Es[i]);
    float ekpcdB = 10.0*log(sqrt(ekpc2)+1.0)/log(10.0);
	float A = 1.0/(8314.0 * 0.0015);
	float B = 2.0/(8314.0 * 0.0020);
	float C = B/2.0 * (1.0 + 8314.0/1e6);
	float kpc2 = A + B/true_snr + C/true_snr/true_snr;
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
