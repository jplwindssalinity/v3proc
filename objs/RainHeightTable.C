//==============================================================//
// Copyright (C) 1998-1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_rainheighttable_c[] =
    "@(#) $Id$";

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "Array.h"
#include "RainHeightTable.h"
#include "Constants.h"

#define DEBUG_RHT 0
//=====================================
// RainHeightTable
//=====================================

RainHeightTable::RainHeightTable()
:table(0){
  return;
}

//=====================================
// ~RainHeightTable
//=====================================

RainHeightTable::~RainHeightTable()
{
  if(table!=NULL){
    int num_day_bins=dayIdx.GetBins();
    int num_lat_bins=latIdx.GetBins();
    int num_lon_bins=lonIdx.GetBins();
    free_array((void*)table,3,num_day_bins,num_lat_bins,num_lon_bins);
  }
  return;
}

//=====================================
// ReadWentz(char* filename)
//=====================================

int RainHeightTable::ReadWentz(const char* filename)
{
  FILE* ifp = fopen(filename,"r");
  if(ifp==NULL) return(0);
  float days_per_month=365.25/12.0;
  dayIdx.SpecifyWrappedCenters(0.5*days_per_month,365.25+0.5*days_per_month,12);
  lonIdx.SpecifyWrappedCenters(0,two_pi,180);
  latIdx.SpecifyCenters(-80*dtr,80*dtr,81);
  unsigned char *** buffer = (unsigned char***)make_array(sizeof(char),3,12,81,180);
  table=(float***)make_array(sizeof(float),3,12,81,180);
  for(int m=0;m<12;m++){
    for (int ilat=0;ilat<81;ilat++){
      if(fread(&buffer[m][ilat][0],sizeof(char),180,ifp)!=180)
	return(0);
    
      for (int ilon=0;ilon<180;ilon++){
	float sst=(float)(buffer[m][ilat][ilon]);
        sst=0.2*(sst-25);
	table[m][ilat][ilon]=1+0.14*sst -0.0025*sst*sst;
	if(sst<0) table[m][ilat][ilon]=1;
	if(sst>28) table[m][ilat][ilon]=2.96;
	if(DEBUG_RHT) 
	  printf("%d %d %d %g %g\n",m,ilat,ilon,sst,table[m][ilat][ilon]);
      }
    }
  }
  free_array((void*)buffer,3,12,81,180);
  return(1);
}

//=========================================
// RainHeightTable::Read
//=========================================
int RainHeightTable::Read(const char* filename){
  FILE* ifp=fopen(filename,"r");
  if(ifp==NULL) return(0);
  return(Read(ifp));
}
int RainHeightTable::Read(FILE* ifp){
  if(! dayIdx.Read(ifp)) return(0);
  if(! latIdx.Read(ifp)) return(0);
  if(! lonIdx.Read(ifp)) return(0);
  int num_day_bins=dayIdx.GetBins();
  int num_lat_bins=latIdx.GetBins();
  int num_lon_bins=lonIdx.GetBins();
  table=(float***)make_array(sizeof(float),3,num_day_bins,num_lat_bins,
			     num_lon_bins);
  for(int m=0;m<(int)num_day_bins;m++){
    for (int ilat=0;ilat<(int)num_lat_bins;ilat++){
      if(fread(&table[m][ilat][0],sizeof(float),num_lon_bins,ifp)
	 !=(unsigned int)num_lon_bins)
	return(0);
    }
  }
  return(1);
}


//=========================================
// RainHeightTable::Write
//=========================================
int RainHeightTable::Write(const char* filename){
  FILE* ofp=fopen(filename,"w");
  if(ofp==NULL) return(0);
  return(Write(ofp));
}

int RainHeightTable::Write(FILE* ofp){
  if(! dayIdx.Write(ofp)) return(0);
  if(! latIdx.Write(ofp)) return(0);
  if(! lonIdx.Write(ofp)) return(0);
  int num_day_bins=dayIdx.GetBins();
  int num_lat_bins=latIdx.GetBins();
  int num_lon_bins=lonIdx.GetBins();
  for(int m=0;m<num_day_bins;m++){
    for (int ilat=0;ilat<num_lat_bins;ilat++){
      if(fwrite(&table[m][ilat][0],sizeof(float),num_lon_bins,ofp)
	 !=(unsigned int)num_lon_bins)
	return(0);
    }
  }
  return(1);
}

//================================
// RainHeightTable::Interpolate
//================================

float
RainHeightTable::Interpolate(
 float day,
 float lat,
 float lon){
 int i[2],j[2],k[2];
 float a[2],b[2],c[2];
 dayIdx.GetLinearCoefsWrapped(day,i,a);
 latIdx.GetLinearCoefsStrict(lat,j,b);
 lonIdx.GetLinearCoefsWrapped(lon,k,c);
 float retval;
 retval=a[0]*b[0]*(c[0]*table[i[0]][j[0]][k[0]]+c[1]*table[i[0]][j[0]][k[1]]) 
   +a[0]*b[1]*(c[0]*table[i[0]][j[1]][k[0]]+c[1]*table[i[0]][j[1]][k[1]]) 
   +a[1]*b[0]*(c[0]*table[i[1]][j[0]][k[0]]+c[1]*table[i[1]][j[0]][k[1]]) 
   +a[1]*b[1]*(c[0]*table[i[1]][j[1]][k[0]]+c[1]*table[i[1]][j[1]][k[1]]);
 return(retval);
}

