//==============================================================//
// Copyright (C) 2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_wind_grad_c[] =
    "@(#) $Id$";

#include "WindGrad.h"

//====================//
//   WGC              //
//====================//

WGC::WGC(){
  return;
}
WGC::~WGC()
{
  return;
}

int
WGC::GetWindVector(
		   WindVector* wv, 
		   LonLat pos){
  float dlat=pos.latitude-lonLat.latitude;
  float dlon=pi-fabs(pi-fabs(pos.longitude-lonLat.longitude));
  wv->dir+=dlat*ddirdlat+dlon*ddirdlon;
  wv->spd+=dlat*dspddlat+dlon*dspddlon;
  if(wv->dir<0) wv->dir+=two_pi;
  if(wv->dir>two_pi) wv->dir-=two_pi;
  return(1);
}
		  

//-----------------//
// WindGrad        //
//-----------------//


WindGrad::WindGrad()
: swath(NULL),_crossTrackBins(0),_alongTrackBins(0),_validCells(0)
{
  return;
};

WindGrad::WindGrad(WindSwath* windswath, int use_dirth=0)
: swath(NULL),_crossTrackBins(0),_alongTrackBins(0),_validCells(0)
{
  _crossTrackBins=windswath->GetCrossTrackBins();
  _alongTrackBins=windswath->GetAlongTrackBins();
  _validCells=0;
  _Allocate();
  _ComputeDerivatives(windswath,use_dirth);
  return;
}

WindGrad::~WindGrad()
{
    DeleteEntireSwath();     
    return;
}

int
WindGrad::DeleteEntireSwath(){
    if (! DeleteWGCs())
        return(0);

    if (! _Deallocate())
        return(0);

    return(1);
}

int
WindGrad::_Deallocate()
{
    if (swath == NULL)
        return(1);

    free_array((void *)swath, 2, _crossTrackBins, _alongTrackBins);
    swath = NULL;
    _crossTrackBins = 0;
    _alongTrackBins = 0;
    return(1);
}

int
WindGrad::DeleteWGCs()
{
    for (int i = 0; i < _crossTrackBins; i++)
    {
        for (int j = 0; j < _alongTrackBins; j++)
        {
            WGC* wgc = *(*(swath + i) + j);
            if (wgc == NULL)
                continue;

            delete wgc;
            *(*(swath + i) + j) = NULL;
        }
    }
    _validCells = 0;
    return(1);
}

//-------------------//
// WindGrad::GetWVC //
//-------------------//

WGC*
WindGrad::GetWGC(
    int  cti,
    int  ati)
{
    if (cti < 0 || cti >= _crossTrackBins ||
        ati < 0 || ati >= _alongTrackBins)
    {
        return(NULL);
    }
    return( *(*(swath + cti) + ati) );
}



//----------------------//
// WindGrad::_Allocate  //
//----------------------//

int
WindGrad::_Allocate()
{
    if (swath != NULL)
        return(0);

    swath = (WGC ***)make_array(sizeof(WGC *), 2, _crossTrackBins,
        _alongTrackBins);

    if (swath == NULL)
        return(0);

    for (int i = 0; i < _crossTrackBins; i++)
    {
        for (int j = 0; j < _alongTrackBins; j++)
        {
            swath[i][j] = NULL;
        }
    }

    return(1);
}


// computes gradients from swath
// ignores invalid WVCs
int
WindGrad::_ComputeDerivatives(
			      WindSwath* windswath,
			      int use_dirth){

#define DEBUG 0


  for (int i = 0; i < _crossTrackBins; i++)
    {
      for (int j = 0; j < _alongTrackBins; j++)
        {
	 

	  // Extract central wind vector
	  WVC* wvc=windswath->GetWVC(i,j);
	  if(!wvc) continue;
	  WindVector* wv;
	  if(use_dirth) wv=wvc->specialVector;
	  else wv=wvc->selected;
	  if(!wv) continue;
	  
	  // compute u, v, lat ,lon
	  float lat=wvc->lonLat.latitude;
	  float lon=wvc->lonLat.longitude;
	  float spd=wv->spd;
	  float dir=wv->dir;	

  
	  int num_valid_neighbors=0;
          float sumdlondlat=0, sumddirdlat=0, sumddirdlon=0;
          float sumdlat2=0, sumdlon2=0, sumdspddlat=0,sumdspddlon=0;
	  // loop through 3x3 neighborhhod to calculate derivatives

	  //-------Start debugging stuff ---------//
	  if(DEBUG && (i<67 || i>68 || j<94 || j>95)) continue;
	  //--- Arrays used for debugging ----//
	  float dddir[3][3];
	  float ddspd[3][3];
	  float ddlat[3][3];
	  float ddlon[3][3];
	  int dmask[3][3]={{0,0,0},{0,0,0},{0,0,0}};
	  if(DEBUG){
	    printf("-----------------------\nCTI=%d ATI=%d\n\n",i,j);
	  }
	  //-------End debugging stuff ---------//

	  for(int i2=MAX(0,i-1);i2<=MIN(_crossTrackBins-1,i+1);i2++){
	    for(int j2=MAX(0,j-1);j2<=MIN(_alongTrackBins-1,j+1);j2++){
	      // Extract neighboring wind vector
	      WVC* wvc2=windswath->GetWVC(i2,j2);
	      if(!wvc2) continue;
	      WindVector* wv2;
	      if(use_dirth) wv2=wvc2->specialVector;
	      else wv2=wvc2->selected;
	      if(!wv2) continue;
	      
	      // compute ddir, dspd, dlat ,dlon
	      float dlat=wvc2->lonLat.latitude-lat;
	      float dlon=pi-fabs(pi-fabs(wvc2->lonLat.longitude-lon));
	      float ddir=pi-fabs(pi-fabs(wv2->dir-dir));
	      float dspd=wv2->spd-spd;
	      
	      //-------Start debugging stuff ---------//
              if(DEBUG){
		int i3=i2-i+1;
		int j3=j2-j+1;

		dddir[i3][j3]=ddir*rtd;
		ddspd[i3][j3]=dspd;
		dmask[i3][j3]=1;
		ddlat[i3][j3]=dlat*rtd;
		ddlon[i3][j3]=dlon*rtd;
	      }
	      //-------End debugging stuff---------//

              // Accumulate sums
	      sumdlon2+=dlon*dlon;
              sumdlat2+=dlat*dlat;
              sumdlondlat+=dlon*dlat;
              sumddirdlat+=ddir*dlat;
              sumddirdlon+=ddir*dlon;
              sumdspddlat+=dspd*dlat;
              sumdspddlon+=dspd*dlon;

	      num_valid_neighbors++;
	    }
	  }
          float denom = sumdlondlat*sumdlondlat - sumdlat2*sumdlon2;
          if(num_valid_neighbors>2 && denom != 0){
	    WGC* wgc = new WGC;
	    wgc->ddirdlat=sumddirdlon*sumdlondlat -sumddirdlat*sumdlon2;
	    wgc->ddirdlat/=denom;

	    wgc->ddirdlon=sumddirdlat*sumdlondlat -sumddirdlon*sumdlat2;
	    wgc->ddirdlon/=denom;

	    wgc->dspddlat=sumdspddlon*sumdlondlat -sumdspddlat*sumdlon2;
	    wgc->dspddlat/=denom;

	    wgc->dspddlon=sumdspddlat*sumdlondlat -sumdspddlon*sumdlat2;
	    wgc->dspddlon/=denom;
            wgc->lonLat.latitude=lat;
            wgc->lonLat.longitude=lon;
	    swath[i][j]=wgc;
	    _validCells++;

	    //-------Start debugging stuff ---------//
            if(DEBUG){
	      printf("DLON MATRIX\n");
	      for(int j3=0;j3<3;j3++){
		for(int i3=0;i3<3;i3++){
		  if(dmask[i3][j3]){
		    printf("%7.3f ",ddlon[i3][j3]);
		  }
		  else{
		    printf("xxxxxxx ");
		  }
		}
		printf("\n");
	      }
	      printf("\n");
	      
	      printf("DLAT MATRIX\n");
	      for(int j3=0;j3<3;j3++){
		for(int i3=0;i3<3;i3++){
		  if(dmask[i3][j3]){
		    printf("%7.3f ",ddlat[i3][j3]);
		  }
		  else{
		    printf("xxxxxxx ");
		  }
		}
		printf("\n");
	      }
	      printf("\n");

	      printf("DDIR MATRIX\n");
	      for(int j3=0;j3<3;j3++){
		for(int i3=0;i3<3;i3++){
		  if(dmask[i3][j3]){
		    printf("%7.3f ",dddir[i3][j3]);
		  }
		  else{
		    printf("xxxxxxx ");
		  }
		}
		printf("\n");
	      }
	      printf("\n");
	      
	      printf("DSPD MATRIX\n");
	      for(int j3=0;j3<3;j3++){
		for(int i3=0;i3<3;i3++){
		  if(dmask[i3][j3]){
		    printf("%7.3f ",ddspd[i3][j3]);
		  }
		  else{
		    printf("xxxxxxx ");
		  }
		}
		printf("\n");
	      }
	      printf("\n");

	      printf("DDIRCALC MATRIX\n");
	      for(int j3=0;j3<3;j3++){
		for(int i3=0;i3<3;i3++){
		  if(dmask[i3][j3]){
		    dddir[i3][j3]=ddlat[i3][j3]*wgc->ddirdlat;
		    dddir[i3][j3]+=ddlon[i3][j3]*wgc->ddirdlon;
		    printf("%7.3f ",dddir[i3][j3]);
		  }
		  else{
		    printf("xxxxxxx ");
		  }
		}
		printf("\n");
	      }
	      printf("\n");
	      
	      printf("DSPDCALC MATRIX\n");
	      for(int j3=0;j3<3;j3++){
		for(int i3=0;i3<3;i3++){
		  if(dmask[i3][j3]){
		    ddspd[i3][j3]=ddlat[i3][j3]*wgc->dspddlat/rtd;
		    ddspd[i3][j3]+=ddlon[i3][j3]*wgc->dspddlon/rtd;
		    printf("%7.3f ",ddspd[i3][j3]);
		  }
		  else{
		    printf("xxxxxxx ");
		  }
		}
		printf("\n");
	      }
	      printf("\nDDIRDLON=%7.3f DDIRDLAT=%7.3f DSPDDLON=%7.3f DSPDDLAT=%7.3f\n\n",
		     wgc->ddirdlon,wgc->ddirdlat,wgc->dspddlon/rtd,
		     wgc->dspddlat/rtd);
	    }
	    //-------End debugging stuff ---------//
	  }
        }
    }

  return(1);
}




