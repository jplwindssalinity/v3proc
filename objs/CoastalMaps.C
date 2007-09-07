#include"CoastalMaps.h"
#include"Meas.h"
#include<stdio.h>
#include"Constants.h"
#include"Array.h"
#include"L2A.h"

CoastalMaps::CoastalMaps()
  : lands0Read(0),_nlooks(0),_neastaz(36),accum_l2a(0)
{
}

 int 
 CoastalMaps::Accumulate(Meas* meas, float s0corr, CoordinateSwitch* gc_to_spot, float** gain, 
			 float xmin, float dx, int nxsteps, float ymin, float dy, int nysteps){
   // if(meas->landFlag==1) return(1); // don't accumulate flagged data
   _ResampleGainMap(meas,gc_to_spot,gain,xmin,dx,nxsteps,ymin,dy,nysteps);
   int foreaft=(meas->scanAngle<pi/2) || (meas->scanAngle>=3*pi/2);
   int looki=2*meas->beamIdx+foreaft;

   for(int i=0;i<_nlats;i++){
     for(int j=0;j<_nlons;j++){

       if(_g[i][j]!=0){
	 if(_n[0][looki][i][j]==0) _eastAzimuth[looki][i][j]+=_g[i][j]*meas->eastAzimuth;
	 else{
	   float azavg= _eastAzimuth[looki][i][j]/_sumg[0][looki][i][j];
	   float az=meas->eastAzimuth;
	   if((az-azavg)>pi) az-=two_pi;
	   if((az-azavg)<-pi) az+=two_pi;
	   _eastAzimuth[looki][i][j]+=_g[i][j]*az;
	 }
	 _sumg[0][looki][i][j]+=_g[i][j];
	 _n[0][looki][i][j]++;
	 _s0[0][looki][i][j]+=_g[i][j]*meas->value;
	 _landfrac[0][looki][i][j]+=_g[i][j]*meas->XK;
         float landcorr=fabs(meas->value-s0corr);
	 if(meas->XK < S0_FLAG_LANDFRAC_THRESH && landcorr < S0_FLAG_LANDCORR_THRESH ){
	   _sumg[1][looki][i][j]+=_g[i][j];
	   _n[1][looki][i][j]++;
	   _s0[1][looki][i][j]+=_g[i][j]*meas->value;
	   _landfrac[1][looki][i][j]+=_g[i][j]*meas->XK;
	 }
	 if(meas->XK < S0_CORR_LANDFRAC_THRESH && landcorr < S0_CORR_LANDCORR_THRESH ){
	   _sumg[2][looki][i][j]+=_g[i][j];
	   _n[2][looki][i][j]++;
	   _s0[2][looki][i][j]+=_g[i][j]*s0corr;
	   _landfrac[2][looki][i][j]+=_g[i][j]*meas->XK;
	 }


	 if(accum_l2a){
	   Meas* m=new Meas;
	   m->value=meas->value;
	   double s0ne=meas->EnSlice;
	   m->EnSlice=meas->value-s0corr;
	   m->landFlag=0;
           if(meas->XK>=S0_CORR_LANDFRAC_THRESH){
	     m->EnSlice=10.0;
	     m->landFlag=1;
	   }
	   m->XK=_g[i][j];
	   m->bandwidth=meas->bandwidth;
	   m->txPulseWidth=meas->txPulseWidth;
	   m->centroid=meas->centroid;
	   m->measType=meas->measType;
	   m->eastAzimuth=meas->eastAzimuth;
	   m->incidenceAngle=meas->incidenceAngle;
	   m->beamIdx=meas->beamIdx;
	   m->startSliceIdx=meas->startSliceIdx;
	   m->numSlices=-1;
	   m->A=1+meas->A;
	   m->B=s0ne*0.1603915;
	   m->C=s0ne*s0ne*0.008075129;
           m->scanAngle=meas->scanAngle;
	   l2aframes[i][j]->measList.Append(m);
	 }
       }
     }

   }
	       
   return(1);
 }		
 
int CoastalMaps::Normalize(){
  for(int t=0;t<3;t++){
    for(int looki=0;looki<_nlooks;looki++){
      for(int i=0;i<_nlats;i++){
	for(int j=0;j<_nlons;j++){
	  if(_n[t][looki][i][j]!=0){
	    _s0[t][looki][i][j]/=_sumg[t][looki][i][j];
	    _landfrac[t][looki][i][j]/=_sumg[t][looki][i][j];
	    if(t==0){
	      _eastAzimuth[looki][i][j]/=_sumg[t][looki][i][j];
	    }
	    _sumg[t][looki][i][j]/=_n[t][looki][i][j];
	  }
	}
      }
    }
  }
   
  return(1);
}

int 
CoastalMaps::Write(char* prefix){
  char filename[200];
  char pref2[200];
  char pref1[200];
  for(int t=0;t<3;t++){
    switch(t){
    case 0:
      sprintf (pref1,"%s_total",prefix);
      break;
    case 1:
      sprintf (pref1,"%s_flag",prefix);
      break;
    case 2:
      sprintf (pref1,"%s_corr",prefix);
      break;
    default:
      fprintf(stderr,"CoastalMaps:Write Bad t %d\n",t);
      exit(1);
    }
    for(int l=0;l<_nlooks;l++){   
      switch(l){
      case 0:
	sprintf (pref2,"%s_innerfore",pref1);
	break;
      case 1:
	sprintf (pref2,"%s_inneraft",pref1);
	break;
      case 2:
	sprintf (pref2,"%s_outerfore",pref1);
	break;
      case 3:
	sprintf (pref2,"%s_outeraft",pref1);
	break;
      default:
	fprintf(stderr,"CoastalMaps:Write Bad look set %d\n",l);
	exit(1);
      }

      // write s0file
      sprintf(filename,"%s.s0",pref2);
      FILE* ofp=fopen(filename,"w");
      if(ofp==NULL){
	fprintf(stderr,"CoastalMaps::Write Error creating file %s\n",filename);
	exit(1);
      }
      for(int i=0;i<_nlats;i++){
	if(fwrite((void*)&_s0[t][l][i][0],sizeof(float),_nlons,ofp)!=(unsigned int)_nlons){
	  fprintf(stderr,"CoastalMaps::Write Error writing to file %s\n",filename);
	  exit(1);
	}
      }
      fclose(ofp);


      // write gain file
      sprintf(filename,"%s.gain",pref2); 
      ofp=fopen(filename,"w");
      if(ofp==NULL){
	fprintf(stderr,"CoastalMaps::Write Error creating file %s\n",filename);
	exit(1);
      }
      for(int i=0;i<_nlats;i++){
	if(fwrite((void*)&_sumg[t][l][i][0],sizeof(float),_nlons,ofp)!=(unsigned int)_nlons){
	  fprintf(stderr,"CoastalMaps::Write Error writing to file %s\n",filename);
	  exit(1);
	}
      }
      fclose(ofp);   
      // write landfrac file
      sprintf(filename,"%s.landfrac",pref2); 
      ofp=fopen(filename,"w");
      if(ofp==NULL){
	fprintf(stderr,"CoastalMaps::Write Error creating file %s\n",filename);
	exit(1);
      }
      for(int i=0;i<_nlats;i++){
	if(fwrite((void*)&_landfrac[t][l][i][0],sizeof(float),_nlons,ofp)!=(unsigned int)_nlons){
	  fprintf(stderr,"CoastalMaps::Write Error writing to file %s\n",filename);
	  exit(1);
	}
      }
      fclose(ofp);      
      // write  num_slices file
      sprintf(filename,"%s.numslices",pref2);    
      ofp=fopen(filename,"w");
      if(ofp==NULL){
	fprintf(stderr,"CoastalMaps::Write Error creating file %s\n",filename);
	exit(1);
      }
      for(int i=0;i<_nlats;i++){
	if(fwrite((void*)&_n[t][l][i][0],sizeof(int),_nlons,ofp)!=(unsigned int)_nlons){
	  fprintf(stderr,"CoastalMaps::Write Error writing to file %s\n",filename);
	  exit(1);
	}
      }
      fclose(ofp);   

      // write  land file
      if(t==0){
	sprintf(filename,"%s.landmask",pref2);    
	ofp=fopen(filename,"w");
	if(ofp==NULL){
	  fprintf(stderr,"CoastalMaps::Write Error creating file %s\n",filename);
	  exit(1);
	}
	for(int i=0;i<_nlats;i++){
	  for(int j=0;j<_nlons;j++){
	    int land=IsLand(_lonstart+_lonres/2+j*_lonres,_latstart+_latres/2+i*_latres);
	    if(fwrite((void*)&land,sizeof(int),1,ofp)!=1){
	      fprintf(stderr,"CoastalMaps::Write Error writing to file %s\n",filename);
	      exit(1);
	    }
	  }
	}
	fclose(ofp);  
      
      // write eastAzimuth file
	sprintf(filename,"%s.eastaz",pref2);
	FILE* ofp=fopen(filename,"w");
	if(ofp==NULL){
	  fprintf(stderr,"CoastalMaps::Write Error creating file %s\n",filename);
	  exit(1);
	}
	for(int i=0;i<_nlats;i++){
	  if(fwrite((void*)&_eastAzimuth[l][i][0],sizeof(float),_nlons,ofp)!=(unsigned int)_nlons){
	  fprintf(stderr,"CoastalMaps::Write Error writing to file %s\n",filename);
	  exit(1);
	  }
	}
	fclose(ofp);

	// write  lands0 file
	sprintf(filename,"%s.lands0",pref2);    
	ofp=fopen(filename,"w");
	if(ofp==NULL){
	  fprintf(stderr,"CoastalMaps::Write Error creating file %s\n",filename);
	  exit(1);
	}
	// compute average ocean value
	float ocean_s0 = 0.0;
	int n_ocean=0;
      
	for(int i=0;i<_nlats;i++){
	  for(int j=0;j<_nlons;j++){
	    if(_n[t][l][i][j]!=0 && _landfrac[t][l][i][j]<0.01) {
	    ocean_s0+=_s0[t][l][i][j];
	    n_ocean++;
	    }
	  }
	}
	if(n_ocean!=0) ocean_s0/=n_ocean;
	else ocean_s0 = 0;
	for(int i=0;i<_nlats;i++){
	  for(int j=0;j<_nlons;j++){
	    float value =0.0;
	    int land=IsLand(_lonstart+_lonres/2+j*_lonres,_latstart+_latres/2+i*_latres);
	    if(!land) value=0.0;
	    else if(_n[t][l][i][j]==0) value=0.0;
	    else{
	      value=(_s0[t][l][i][j]-(1-_landfrac[t][l][i][j])*ocean_s0)/_landfrac[t][l][i][j];
	    }
	    if(fwrite((void*)&value,sizeof(int),1,ofp)!=1){
	      fprintf(stderr,"CoastalMaps::Write Error writing to file %s\n",filename);
	      exit(1);
	    }
	  }
	}
	fclose(ofp);    
      }
    }
  }
  return(1);
}

CoastalMaps::~CoastalMaps(){
  if(_nlooks)_DeallocateExtraMaps();
  _nlooks=0;
  if(accum_l2a){
    for(int i=0;i<_nlats;i++){
      for(int j=0;j<_nlons;j++){
	delete l2aframes[i][j];
      }
    }
  }
  free_array((void*)l2aframes,2,_nlats,_nlons);
  accum_l2a=0;
}

int
CoastalMaps::InitExtraMaps(double latstart, double lonstart, double res, double latsize, double lonsize, int makel2a)
{
   accum_l2a=makel2a;
  _nlooks=4;
  _nlats=(int)(latsize/res+0.5);
  _nlons=(int)(lonsize/res+0.5);
  _latstart=latstart;
  _lonstart=lonstart;
  _latres=latsize/_nlats;
  _lonres=lonsize/_nlons;
  
  if(!_AllocateExtraMaps()){
    fprintf(stderr,"Coastals:: AllocateExtraMaps failed\n");
    _nlooks=0;
    exit(1);
  }

  if(accum_l2a){
    l2aframes=(L2AFrame***)make_array(sizeof(L2AFrame*),2,_nlats,_nlons);
    for(int i=0;i<_nlats;i++){
      for(int j=0;j<_nlons;j++){
	l2aframes[i][j]=new L2AFrame;
	l2aframes[i][j]->rev=1;
	l2aframes[i][j]->ati=i;
	l2aframes[i][j]->cti=j;
      }
    }
  }
  return(1);
}

int
CoastalMaps::WriteL2A(char* filename){

  FILE* ofp = fopen(filename,"w");
  if(ofp == NULL){
    fprintf(stderr,"Error creating L2A file %s\n",filename);
    exit(1);
  }
  L2AHeader l2ah;
  EarthPosition p,pcplus,paplus;
  double lonmid=(_lonstart+(_nlons*_lonres)/2);
  double latmid=(_latstart+(_nlats*_latres)/2);
  p.SetAltLonGDLat(0,lonmid,latmid);
  pcplus.SetAltLonGDLat(0,lonmid+_lonres,latmid);
  paplus.SetAltLonGDLat(0,lonmid,latmid+_latres);
  double ctires=fabs((pcplus-p).Magnitude());
  double atires=fabs((paplus-p).Magnitude());
  l2ah.crossTrackResolution=ctires;
  l2ah.alongTrackResolution=atires;
  l2ah.crossTrackBins=_nlons;
  l2ah.alongTrackBins=_nlats;
  l2ah.zeroIndex=0;
  l2ah.startTime=0.0;
  if(!l2ah.Write(ofp)){
    fprintf(stderr,"Error writing header to L2A file %s\n",filename);
    exit(1);
  }
  for(int i=0;i<_nlats;i++){
    for(int j=0;j<_nlons;j++){
      if(!l2aframes[i][j]->Write(ofp)){
	fprintf(stderr,"Error writing frame %d %d to L2A file %s\n",i,j,filename);
	exit(1);
      }
    }
  }
  fclose(ofp);
  return(1);
}
int
CoastalMaps::_AllocateExtraMaps(){
  _g=(float**)make_array(sizeof(float),2,_nlats,_nlons);
  _sumg=(float****)make_array(sizeof(float),4,3,_nlooks,_nlats,_nlons);
  _s0=(float****)make_array(sizeof(float),4,3,_nlooks,_nlats,_nlons);
  _landfrac=(float****)make_array(sizeof(float),4,3,_nlooks,_nlats,_nlons);
  _prelands0=(float***)make_array(sizeof(float),3,_neastaz,_nlats,_nlons);
  _eastAzimuth=(float***)make_array(sizeof(float),3,_nlooks,_nlats,_nlons);
  _n=(int****)make_array(sizeof(int),4,3,_nlooks,_nlats,_nlons);
  if(!_n || !_landfrac || !_s0 || !_sumg || !_g || !_prelands0) return(0);
  return(1);
}

int 
CoastalMaps::_DeallocateExtraMaps(){
  free_array((void*)_g,2,_nlats,_nlons);
  free_array((void*)_sumg,4,3,_nlooks,_nlats,_nlons);
  free_array((void*)_s0,4,3,_nlooks,_nlats,_nlons);
  free_array((void*)_landfrac,4,3,_nlooks,_nlats,_nlons);
  free_array((void*)_prelands0,3,_neastaz,_nlats,_nlons);
  free_array((void*)_eastAzimuth,3,_nlooks,_nlats,_nlons);
  free_array((void*)_n,4,3,_nlooks,_nlats,_nlons);
  return(1);
}

int
CoastalMaps::_ResampleGainMap( Meas* meas, CoordinateSwitch* gc_to_spot, float** gain, float xmin, float dx,
			       int nxsteps, float ymin, float dy, int nysteps){
  // compute max gain
  float max_gain=0;
  for(int i=0;i<nxsteps;i++){
    for(int j=0;j<nysteps;j++){
      if(gain[i][j]>max_gain) max_gain=gain[i][j];
    }
  }  
  for(int i=0;i<_nlats;i++){
    for(int j=0;j<_nlons;j++){
      double lat=_latstart+_latres/2+i*_latres;
      double lon=_lonstart+_lonres/2+j*_lonres;
      EarthPosition p;
      p.SetAltLonGDLat(0,lon,lat);
      p-=meas->centroid;
      Vector3 pspot=gc_to_spot->Forward(p);
      float x=pspot.Get(0);
      float y=pspot.Get(1);
      int ix=int((x-xmin)/dx +0.5);
      int iy=int((y-ymin)/dy +0.5);
    
      if(ix<0 || iy<0 || iy>=nysteps || ix>=nxsteps){
	_g[i][j]=0.0;
      }
      else{
	_g[i][j]=gain[ix][iy];
      }
      if(_g[i][j]<max_gain*SLICE_GAIN_THRESH) _g[i][j]=0.0;
    }
  }
  return(1);
}

int
CoastalMaps::ReadPrecomputeLands0(char* prefix){
	// read  lands0 files
  char filename[200];
  sprintf (filename,"%s_eastaz.lands0",prefix);    
  FILE* ifp=fopen(filename,"r");
  if(ifp==NULL){
    fprintf(stderr,"CoastalMaps::Error opening file %s\n",filename);
    exit(1);
  }
  for(int l=0;l<_neastaz;l++){   
    for(int i=0;i<_nlats;i++){
      if(fread((void*)&_prelands0[l][i][0],sizeof(float),_nlons,ifp)!=(unsigned int)_nlons){
	fprintf(stderr,"CoastalMaps::Error reading from file %s\n",filename);
	exit(1);
      }
    }
  }
  fclose(ifp);    

  lands0Read=1;
  return(1);
}
float 
CoastalMaps::GetPrecomputedLands0(Meas* meas,double lon, double lat){

  while(lon<_lonstart)lon+=2*pi;
  while(lon>=_lonstart+_nlons*_lonres)lon-=2*pi;

  if(lon<_lonstart || lat<_latstart || lat>=_latstart+_nlats*_latres) return(-100);

  float azstep=two_pi/_neastaz;
  int l=(int)floor(meas->eastAzimuth/azstep);
  l%=_neastaz;

  int i=(int)((lat-_latstart)/_latres);
  int j=(int)((lon-_lonstart)/_lonres);

  // make sure it returns a non-zero value when land is found
  float value=_prelands0[l][i][j];
  if(value==0 && IsLand(lon,lat)){ 
    int nnotzero=0;
    for(int l2=l-1;l2<l+1;l2++){
      int l3=l2%_neastaz;
      for(int i2=i-1;i2<=i+1;i2++){
	if(i2<0 || i2>=_nlats) continue;
	for(int j2=j-1;j2<=j+1;j2++){
	  if(j2<0 || j2>=_nlons) continue;
	  if(_prelands0[l3][i2][j2]!=0){
	    value+=_prelands0[l3][i2][j2];
	    nnotzero++;
	  }
	}
      }
    }
    if(nnotzero==0){
      fprintf(stderr,"Warning no nonzero landsigma0s in 3 x 3 x 3 neighborhood\n");
      fprintf(stdout,"Warning no nonzero landsigma0s in 3 x 3 x 3neighborhood\n");
    }
    else value/=nnotzero;
  }
  return(value);
}

