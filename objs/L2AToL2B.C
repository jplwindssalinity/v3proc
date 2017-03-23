//==============================================================//
// Copyright (C) 1997-2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_l2atol2b_c[] =
    "@(#) $Id$";

#include <stdio.h>
#include <string.h>
#include "L2AToL2B.h"
#include "Constants.h"
#include "Misc.h"
#include "Interpolate.h"
#include "Wind.h"
#include "Array.h"
#define NOMINAL_TRACK_LENGTH (1624*25)

extern MLP_IOType* mlp_io_type_defs;

//==========//
// L2AToL2B //
//==========//

L2AToL2B::L2AToL2B()
:   medianFilterWindowSize(0), medianFilterMaxPasses(0), maxRankForNudging(0),
    useManyAmbiguities(0), useAmbiguityWeights(0), useNudging(0),
    smartNudgeFlag(0), wrMethod(GS), useNudgeThreshold(0), useNMF(0),
    useRandomInit(0), useNudgeStream(0), onePeakWidth(0.0), twoPeakSep(181.0),
    probThreshold(0.0), streamThreshold(0.0), useHurricaneNudgeField(0),
    hurricaneRadius(0), useSigma0Weights(0), sigma0WeightCorrLength(25.0), 
    arrayNudgeFlag(0), arrayNudgeSpd(NULL),arrayNudgeDir(NULL),
    use_MLP_mapping(false), MLP_input_map_s0(NULL), MLP_input_map_vars0(NULL),
    do_coastal_processing(0), rain_speed_corr_thresh_for_flagging(-9999)
{
    return;
}

L2AToL2B::~L2AToL2B()
{
  if(arrayNudgeSpd){
    free_array(arrayNudgeSpd,2,arrayNudgeNati,arrayNudgeNcti);
    free_array(arrayNudgeDir,2,arrayNudgeNati,arrayNudgeNcti);
    arrayNudgeSpd=NULL;
    arrayNudgeDir=NULL;
  }
  if( MLP_input_map_s0    != NULL ) delete [] MLP_input_map_s0;
  if( MLP_input_map_vars0 != NULL ) delete [] MLP_input_map_vars0;
    return;
}


// this is fragile. It assumes the L2A file and the ECMWF (E2B) array file 
// are both the proper QS 12.5 km size
int
L2AToL2B::ReadQSCP12ECMWFNudgeArray(char* filename){
  FILE * ifp=fopen(filename,"r");
  if(ifp==NULL){
    fprintf(stderr,"L2AToL2B::ReadQSCP12ECMWFNudgeArray cannot open file %s\n",filename);
    exit(1);
  }
  arrayNudgeNcti=152;
  arrayNudgeNati=3248;
  if(arrayNudgeSpd!=NULL){
    fprintf(stderr,"L2AToL2B::ReadNudgeArray or a variant ran twice!\n");
    exit(1);
  }
  arrayNudgeSpd=(float**) make_array(sizeof(float),2,arrayNudgeNati,arrayNudgeNcti);
  arrayNudgeDir=(float**) make_array(sizeof(float),2,arrayNudgeNati,arrayNudgeNcti);
  
  for(int a=0;a<arrayNudgeNati;a++){
    for(int c=0;c<arrayNudgeNcti;c++){
      arrayNudgeSpd[a][c]=-1;
      arrayNudgeDir[a][c]=0;
    }
  }
  int nati=arrayNudgeNati;
  int ncti=arrayNudgeNcti;
  // skip over lat and lon arrays
  if( ! read_array(ifp,&arrayNudgeSpd[0],sizeof(float),2,nati,ncti) ||
      ! read_array(ifp,&arrayNudgeDir[0],sizeof(float),2,nati,ncti)){

    fprintf(stderr,"L2AToL2B::ReadQSCP12ECMWFNudgeArray error reading arrays from file %s\n",filename);
    exit(1);
 
  }
  // read speed and direction arrays
  if( ! read_array(ifp,&arrayNudgeSpd[0],sizeof(float),2,nati,ncti) ||
      ! read_array(ifp,&arrayNudgeDir[0],sizeof(float),2,nati,ncti)){

    fprintf(stderr,"L2AToL2B::ReadQSCP12ECMWFNudgeArray error reading arrays from file %s\n",filename);
    exit(1);
 
  }

  for(int a=0;a<arrayNudgeNati;a++){
    for(int c=0;c<arrayNudgeNcti;c++){
      if(arrayNudgeDir[a][c]!=0 || arrayNudgeSpd[a][c]!=0){ 
	arrayNudgeDir[a][c]=450-arrayNudgeDir[a][c];
	if(arrayNudgeDir[a][c]>360) arrayNudgeDir[a][c]=arrayNudgeDir[a][c]-360;
	arrayNudgeDir[a][c]*=dtr;
      }
    }
  }
  return(1);
}



// this is fragile. It assumes the L2A file and the ECMWF (E2B) array file 
// are both the proper ISRO 50 km size
int
L2AToL2B::ReadISROECMWFNudgeArray(char* filename){
  FILE * ifp=fopen(filename,"r");
  if(ifp==NULL){
    fprintf(stderr,"L2AToL2B::ReadISROECMWFNudgeArray cannot open file %s\n",filename);
    exit(1);
  }
  arrayNudgeNcti=36;
  arrayNudgeNati=860;
  if(arrayNudgeSpd!=NULL){
    fprintf(stderr,"L2AToL2B::ReadNudgeArray or a variant ran twice!\n");
    exit(1);
  }
  arrayNudgeSpd=(float**) make_array(sizeof(float),2,arrayNudgeNati,arrayNudgeNcti);
  arrayNudgeDir=(float**) make_array(sizeof(float),2,arrayNudgeNati,arrayNudgeNcti);
  
  for(int a=0;a<arrayNudgeNati;a++){
    for(int c=0;c<arrayNudgeNcti;c++){
      arrayNudgeSpd[a][c]=-1;
      arrayNudgeDir[a][c]=0;
    }
  }
  int nati=arrayNudgeNati;
  int ncti=arrayNudgeNcti;
  // skip over lat and lon arrays
  if( ! read_array(ifp,&arrayNudgeSpd[0],sizeof(float),2,nati,ncti) ||
      ! read_array(ifp,&arrayNudgeDir[0],sizeof(float),2,nati,ncti)){

    fprintf(stderr,"L2AToL2B::ReadISROECMWFNudgeArray error reading arrays from file %s\n",filename);
    exit(1);
 
  }
  // read speed and direction arrays
  if( ! read_array(ifp,&arrayNudgeSpd[0],sizeof(float),2,nati,ncti) ||
      ! read_array(ifp,&arrayNudgeDir[0],sizeof(float),2,nati,ncti)){

    fprintf(stderr,"L2AToL2B::ReadISROECMWFNudgeArray error reading arrays from file %s\n",filename);
    exit(1);
 
  }

  for(int a=0;a<arrayNudgeNati;a++){
    for(int c=0;c<arrayNudgeNcti;c++){
      if(arrayNudgeDir[a][c]!=0 || arrayNudgeSpd[a][c]!=0){ 
	arrayNudgeDir[a][c]=450-arrayNudgeDir[a][c];
	if(arrayNudgeDir[a][c]>360) arrayNudgeDir[a][c]=arrayNudgeDir[a][c]-360;
	arrayNudgeDir[a][c]*=dtr;
      }
    }
  }
  return(1);
}




int
L2AToL2B::ReadNudgeArray(char* filename){
  FILE * ifp=fopen(filename,"r");
  if(ifp==NULL){
    fprintf(stderr,"L2AToL2B::ReadNudgeArray cannot open file %s\n",filename);
    exit(1);
  }
  int ati1;
  int nati,ncti;
  if( !fread(&ati1,sizeof(int),1,ifp)==1 ||
      !fread(&nati,sizeof(int),1,ifp)==1 ||
      !fread(&ncti,sizeof(int),1,ifp)==1 ){
    fprintf(stderr,"L2AToL2B::ReadNudgeArray error reading file %s\n",filename);
    exit(1);
  }
  arrayNudgeNcti=ncti;
  arrayNudgeNati=nati+ati1;
  if(arrayNudgeSpd!=NULL){
    fprintf(stderr,"L2AToL2B::ReadNudgeArray ran twice!\n");
    exit(1);
  }
  arrayNudgeSpd=(float**) make_array(sizeof(float),2,arrayNudgeNati,arrayNudgeNcti);
  arrayNudgeDir=(float**) make_array(sizeof(float),2,arrayNudgeNati,arrayNudgeNcti);
  
  for(int a=0;a<arrayNudgeNati;a++){
    for(int c=0;c<arrayNudgeNcti;c++){
      arrayNudgeSpd[a][c]=-1;
      arrayNudgeDir[a][c]=0;
    }
  }
  if( ! read_array(ifp,&arrayNudgeSpd[ati1],sizeof(float),2,nati,ncti) ||
      ! read_array(ifp,&arrayNudgeDir[ati1],sizeof(float),2,nati,ncti)){

    fprintf(stderr,"L2AToL2B::ReadNudgeArray error reading arrays from file %s\n",filename);
    exit(1);
 
  }


  return(1);
}

//----------------------------------//
// L2AToL2B::SetWindRetrievalMethod //
//----------------------------------//
// sets the wind retrieval method based on a passed string
// returns 0 on failure
// returns 1 on success

int
L2AToL2B::SetWindRetrievalMethod(
    const char*  wr_method)
{
    if (strcasecmp(wr_method, "GS") == 0)
    {
        wrMethod = GS;
        return(1);
    }
    else if (strcasecmp(wr_method, "GS_FIXED") == 0)
    {
        wrMethod = GS_FIXED;
        return(1);
    }
    else if (strcasecmp(wr_method, "H1") == 0)
    {
        wrMethod = H1;
        return(1);
    }
    else if (strcasecmp(wr_method, "H2") == 0)
    {
        wrMethod = H2;
        return(1);
    }
    else if (strcasecmp(wr_method, "H3") == 0)
    {
        wrMethod = H3;
        return(1);
    }
    else if (strcasecmp(wr_method, "S1") == 0)
    {
        wrMethod = S1;
        return(1);
    }
    else if (strcasecmp(wr_method, "S2") == 0)
    {
        wrMethod = S2;
        return(1);
    }
    else if (strcasecmp(wr_method, "S3") == 0)
    {
        wrMethod = S3;
        return(1);
    }
    else if (strcasecmp(wr_method, "S3MV") == 0)
    {
        wrMethod = S3MV;
        return(1);
    }
    else if (strcasecmp(wr_method, "S4") == 0)
    {
        wrMethod = S4;
        return(1);
    }
    else if (strcasecmp(wr_method, "POLAR_SPECIAL") == 0)
    {
        wrMethod = POLAR_SPECIAL;
        return(1);
    }
    else if (strcasecmp(wr_method, "CHEAT") == 0)
    {
        wrMethod = CHEAT;
        return(1);
    }
    else if (strcasecmp(wr_method, "S3RAIN")==0)
      {
	wrMethod = S3RAIN;
	return(1);
      }
    else if (strcasecmp(wr_method, "CoastSpecial")==0)
      {
	wrMethod = CoastSpecial;
	return(1);
      }
    else if (strcasecmp(wr_method, "CoastSpecialGS")==0)
      {
	wrMethod = CoastSpecialGS;
	return(1);
      }
    else if (strcasecmp(wr_method, "HurrSp1")==0)
      {
	wrMethod = HurrSp1;
	return(1);
      }
    else
        return(0);
}


//#define OMIT_VAR_IN_ANN 0
//#define USE_CTD_INPUT 2
//---------------------------//
// L2AToL2B::NeuralNetRetrieve//
//---------------------------//
// retrieve using a neural network

/**** OBSOLETE ROUTINE
 float  

 
 L2AToL2B::NeuralNetRetrieve(L2A* l2a, L2B* l2b, MLPDataArray* spdnet, MLPDataArray* dirnet, GMF* gmf, Kp* kp, int need_all_looks){

    
    gmf->objectiveFunctionMethod=0;
    MeasList* meas_list = &(l2a->frame.measList);

    if(arrayNudgeFlag & l2b->frame.swath.GetCrossTrackBins()!=arrayNudgeNcti){
      fprintf(stderr,"Resolution mismatch between Nudge Array and L2B swath\n");
      exit(1);
    }
 

    //-----------------------------------//
    // check for missing wind field data //
    //-----------------------------------//
    // this should be handled by some kind of a flag!

    int any_zero = 0;
    for (Meas* meas = meas_list->GetHead(); meas; meas = meas_list->GetNext())
    {
        if (! meas->value)
        {
            any_zero = 1;
            break;
        }
    }
    if (any_zero)
    {
        return(-1);
    }
    //-----------------------------------//
    // check for wind retrieval criteria //
    //-----------------------------------//

    if (! gmf->CheckRetrieveCriteria(meas_list))
    {
        return(-1);
    }


    if (useSigma0Weights == 1)
    {
        gmf->CalculateSigma0Weights(meas_list);

        // modify the Kp C coefficient to apply the sigma0 weights to the measurements
        float correlation_length = sigma0WeightCorrLength;       
        for (Meas* meas = meas_list->GetHead(); meas; meas = meas_list->GetNext())
        {

            meas->XK *= correlation_length*correlation_length;
            
        }
	gmf->objectiveFunctionMethod=1;
    }


    //-----------------//
    // Get Nudge Field //
    //-----------------//

    WVC* wvc = new WVC();
    float ctd, speed;
    wvc->lonLat=meas_list->AverageLonLat();
    wvc->nudgeWV=new WindVectorPlus;
    if(smartNudgeFlag){
      if (! nudgeVctrField.InterpolateVectorField(wvc->lonLat,
						  wvc->nudgeWV,0))
	  {
	    delete wvc->nudgeWV;
	    wvc->nudgeWV=NULL;
	    delete wvc;
	    return(-1);
	  }
    }
    else if ( arrayNudgeFlag){
      if(l2a->frame.ati>=arrayNudgeNati){
	    delete wvc->nudgeWV;
	    wvc->nudgeWV=NULL;
	    delete wvc;
	    return(-1);
      }
      float nspd=arrayNudgeSpd[l2a->frame.ati][l2a->frame.cti];
      if(nspd<0){
	    delete wvc->nudgeWV;
	    wvc->nudgeWV=NULL;
	    delete wvc;
	    return(-1);
      }
      else{
        wvc->nudgeWV->spd=nspd;
        wvc->nudgeWV->dir=arrayNudgeDir[l2a->frame.ati][l2a->frame.cti];
      }
    }
    else if (! nudgeField.InterpolatedWindVector(wvc->lonLat,
						 wvc->nudgeWV))
	  {
	    delete wvc->nudgeWV;
	    wvc->nudgeWV=NULL;
	    delete wvc;
	    return(-1);
	  }
    // determine nudge wind direction relative to swath
    float diroff=GetNeuralDirectionOffset(l2a); 
    float reldir=diroff+wvc->nudgeWV->dir; 
    while(reldir<0)reldir+=2*pi;
    while(reldir>2*pi)reldir-=2*pi;
    //--------------------------
    // compute speed assuming nudge direction is correct
    //---------------------------
    //ctd = (l2a->frame.cti - l2a->header.zeroIndex) *
    //l2a->header.crossTrackResolution;
    ctd=(l2a->frame.cti+0.5)*l2a->header.crossTrackResolution-950.0;

    MLP* smlp=spdnet->getMLP(reldir,ctd);
   
    float dummyout;
    
    // compute inputs to speed mlp
    int nc = meas_list->NodeCount();
    int nmeas[8]={0,0,0,0,0,0,0,0};
    int look_idx=-1;

    // for now we will assume if the input vector is size 16 the it has Ku and C inputs
    // if it is size 8 it has Ku-only
    int nlooks=(spdnet->nMLPin)-USE_CTD_INPUT;
    if(!OMIT_VAR_IN_ANN) nlooks/=2;

    float mlpinvec[18]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    float wtsum[8]={0,0,0,0,0,0,0,0};
    Meas* meas = meas_list->GetHead();
    for (int c = 0; c < nc; c++)
      {
	switch (meas->measType)
	  {
	  case Meas::HH_MEAS_TYPE:
	    if (meas->scanAngle < pi / 2 || meas->scanAngle > 3 * pi / 2)
		      look_idx = 0;
		    
	    else
	      look_idx = 1;
	    break;
	  case Meas::VV_MEAS_TYPE:
	    if (meas->scanAngle < pi / 2 || meas->scanAngle > 3 * pi / 2)
	      look_idx = 2;
	    else
	      look_idx = 3;
	    break;
	  case Meas::C_BAND_HH_MEAS_TYPE:
	    if (meas->scanAngle < pi / 2 || meas->scanAngle > 3 * pi / 2)
	      look_idx = 4;
	    else
	      look_idx = 5;
	    break;
	  case Meas::C_BAND_VV_MEAS_TYPE:
	    if (meas->scanAngle < pi / 2 || meas->scanAngle > 3 * pi / 2)
	      look_idx = 6;
	    else
	      look_idx = 7;
	    break;
	  default:
	    look_idx = -1;
	    break;
	  }
        if( look_idx >= 4 && nlooks <= 4 ){
	  fprintf(stderr,"NeuralNetRetrieve error: Speed Net too small to include C-band data\n");
	  exit(1);
	}  
	if (look_idx >= 0)
	  {
	    if(!OMIT_VAR_IN_ANN){
	      mlpinvec[nlooks+look_idx] += meas->value*meas->value;
	    }
  
            float wt=1;
            if(useSigma0Weights){
	      wt=1.0/(1.0+(1.0/meas->XK));
	    }
	    mlpinvec[look_idx] += meas->value*wt;
            wtsum[look_idx]+=wt;
	    nmeas[look_idx]++;
	  }
	meas = meas_list->GetNext();
      }
    
    
    if(nmeas[3]==0) return(-1);
    for(int i=0;i<nlooks;i++){
      if(!useSigma0Weights) wtsum[i]=1;
      else if(nmeas[i]>=1) wtsum[i]/=nmeas[i];
      if(nmeas[i] > 1){
	mlpinvec[i]/=wtsum[i]*nmeas[i];
        if(!OMIT_VAR_IN_ANN){
	  mlpinvec[i+nlooks]/=(wtsum[i]*nmeas[i]);
	}
      }
      else if(nmeas[i]==0){
        if(need_all_looks){
	  return(-1);
	}
	mlpinvec[i]=0;
        if(!OMIT_VAR_IN_ANN)
	  mlpinvec[i+nlooks]=0.1;
      }
    }
    // inpctd_case
    if(USE_CTD_INPUT){
      mlpinvec[spdnet->nMLPin-1]=ctd;
    }
    // inpctd and reldir
    if(USE_CTD_INPUT==2){
      mlpinvec[spdnet->nMLPin-2]=reldir;
    }

    smlp->ForwardMSE(mlpinvec, &dummyout);
    speed=smlp->outp[0];

 
    // compute best speed array
    _phiCount=72; // It uses all 72 even if the neural net array is smaller
    float phistep=2*pi/_phiCount;
    wvc->directionRanges.dirIdx.SpecifyWrappedCenters(0, 2*pi, _phiCount);
    wvc->directionRanges.bestObj = (float*)malloc(sizeof(float)*_phiCount);
    wvc->directionRanges.bestSpd = (float*)malloc(sizeof(float)*_phiCount);
    for(int c=0;c<_phiCount;c++){
      float phi=(c)*phistep;
      float reldir=phi+diroff;
      while(reldir<0)reldir+=2*pi;
      while(reldir>2*pi)reldir-=2*pi;
      smlp=spdnet->getMLP(reldir,ctd);
      // check for reldir dependency in network
      if(USE_CTD_INPUT==2) mlpinvec[spdnet->nMLPin-2]=reldir;
      smlp->ForwardMSE(mlpinvec, &dummyout);
      wvc->directionRanges.bestSpd[c]=smlp->outp[0];
    }
    if (useSigma0Weights == 1){
        float cwt;
  
        // network does not include c-band inputs
	if(nlooks<=4) cwt=0;
   
        // no C-band measurements in file
        else if(nmeas[4]==0 && nmeas[5]==0 && nmeas[6]==0 && nmeas[7]==0){
	  cwt=0;
	}
        else{
	  cwt=0.5;   // weights them equally for now.. Need to talk to Scott about this....
	}
        gmf->SetCBandWeight(cwt); // only used if dirnet is NULL, not used to compute speed
    }

    float* _bestObj=wvc->directionRanges.bestObj;
    float* _bestSpd = wvc->directionRanges.bestSpd;
    if(dirnet!=NULL){
      MLP* dmlp;
      double sumprob=0;
      // compute directional probabilities
      for(int c=0;c<_phiCount;c++){
	float phi=(c)*phistep;
	float reldir=phi+diroff;
	while(reldir<0)reldir+=2*pi;
	while(reldir>2*pi)reldir-=2*pi;
	dmlp=dirnet->getMLP(reldir,ctd);
	// check for reldir dependency in network
	if(USE_CTD_INPUT) mlpinvec[spdnet->nMLPin-2]=reldir;
	dmlp->ForwardMSE(mlpinvec, &dummyout);
        
        double prob=dmlp->outp[0];
	if(prob<0) prob=0;
        prob=pow(prob,2.0);
        sumprob+=prob;
	wvc->directionRanges.bestObj[c]=prob;
# define DEBUGANDSTOP 0
        

      }
      if(l2a->frame.cti==20 && DEBUGANDSTOP){
	     fprintf(stderr,"     Best_obj (from network directly)   = \n");
	     for (int c=0;c<_phiCount/8;c++){
	       int i=8*c;
	       float* x = _bestObj;
	       float k= 1;
	       fprintf(stderr,"      %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f\n",
		       k*x[i],k*x[i+1],k*x[i+2],k*x[i+3],k*x[i+4],k*x[i+5],k*x[i+6],k*x[i+7]);
	     }
	     exit(1);
      }      
      for(int c=0;c<_phiCount;c++){
	wvc->directionRanges.bestObj[c]/=sumprob;
	if(wvc->directionRanges.bestObj[c]<=0) wvc->directionRanges.bestObj[c]=-99;
	else wvc->directionRanges.bestObj[c]=2*log(wvc->directionRanges.bestObj[c]);
      }
    }
    // if dirnet is null compute directional probabilites directly from speeds
    else{
      for(int c=0;c<_phiCount;c++){
	float phi=c*2*pi/_phiCount;
	wvc->directionRanges.bestObj[c]=gmf->_ObjectiveFunction(meas_list,wvc->directionRanges.bestSpd[c],phi,kp,0);
       } 
    }  
 
    float sum = 0.0;
    float scale = _bestObj[0];
    for (int c = 1; c < _phiCount; c++)
      {
	if (scale < _bestObj[c])
	  scale = _bestObj[c];
      }
    for (int c = 0; c < _phiCount; c++)
      {
	_bestObj[c] = exp((_bestObj[c] -  scale) / 2);
	sum += _bestObj[c];
      }
    for (int c = 0; c < _phiCount; c++)
	{
	  _bestObj[c] /= sum;
	}
    int DEBUG = 0;
    int PRINTALLINPUTS=0;
    if(DEBUG && PRINTALLINPUTS){
      for(int k=0;k<spdnet->nMLPin;k++) printf("%g ",mlpinvec[k]);
      printf("%g\n",wvc->nudgeWV->spd);
    }
    if(l2a->frame.cti==67 && l2a->frame.ati==1475) DEBUG=1;
    if(DEBUG){
      fprintf(stderr,"   Before MakeAmbigs-----------\n");
      fprintf(stderr,"     speed=%g nudgevctr(spd,dir)=(%g,%g)\n",
	      speed, wvc->nudgeWV->spd, wvc->nudgeWV->dir*180/pi);
      fprintf(stderr,"     MLPINVEC   = %g %g %g %g %g\n",
	      mlpinvec[0],mlpinvec[1],mlpinvec[2],mlpinvec[3],mlpinvec[4]);

      fprintf(stderr,"     Best_spd   = \n");
      for (int c=0;c<_phiCount/8;c++){
	int i=8*c;
	float* x = _bestSpd;
        float k= 1;
	  fprintf(stderr,"      %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f\n",
		  k*x[i],k*x[i+1],k*x[i+2],k*x[i+3],k*x[i+4],k*x[i+5],k*x[i+6],k*x[i+7]);
      }
      fprintf(stderr,"     Best_obj (percent)   = \n");
      for (int c=0;c<_phiCount/8;c++){
	int i=8*c;
	float* x = _bestObj;
        float k= 100;
	  fprintf(stderr,"      %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f\n",
		  k*x[i],k*x[i+1],k*x[i+2],k*x[i+3],k*x[i+4],k*x[i+5],k*x[i+6],k*x[i+7]);
      }
    }
    MakeAmbigsFromDirectionArrays(wvc,0);
    if(DEBUG){
      fprintf(stderr,"   After MakeAmbigs before BuildDirectionRanges-----------\n");
      fprintf(stderr,"     Best_spd   = \n");
      for (int c=0;c<_phiCount/8;c++){
	int i=8*c;
	float* x = _bestSpd;
        float k= 1;
	  fprintf(stderr,"      %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f\n",
		  k*x[i],k*x[i+1],k*x[i+2],k*x[i+3],k*x[i+4],k*x[i+5],k*x[i+6],k*x[i+7]);
      }
      fprintf(stderr,"     Best_obj (percent)   = \n");
      for (int c=0;c<_phiCount/8;c++){
	int i=8*c;
	float* x = _bestObj;
        float k= 100;
	fprintf(stderr,"      %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f\n",
		k*x[i],k*x[i+1],k*x[i+2],k*x[i+3],k*x[i+4],k*x[i+5],k*x[i+6],k*x[i+7]);
      }
      fprintf(stderr,"     Ambiguities        = \n");
      for(int i=0;i<wvc->ambiguities.NodeCount();i++){
	WindVectorPlus* wvp=wvc->ambiguities.GetByIndex(i);
	fprintf(stderr,"      #%d spd=%g dir=%g obj=%g\n",
		i,wvp->spd,wvp->dir*180/pi,wvp->obj);     
      }
    }
    BuildDirectionRanges(wvc, 0.8);// uses 80% threshold 
    if(DEBUG){
      fprintf(stderr,"   After BuildDirectionRanges-----------\n");
      fprintf(stderr,"     Best_spd   = \n");
      for (int c=0;c<_phiCount/8;c++){
	int i=8*c;
	float* x = _bestSpd;
        float k= 1;
	  fprintf(stderr,"      %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f\n",
		  k*x[i],k*x[i+1],k*x[i+2],k*x[i+3],k*x[i+4],k*x[i+5],k*x[i+6],k*x[i+7]);
      }
      fprintf(stderr,"     Best_obj (percent)   = \n");
      for (int c=0;c<_phiCount/8;c++){
	int i=8*c;
	float* x = _bestObj;
        float k= 100;
	fprintf(stderr,"      %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f\n",
		k*x[i],k*x[i+1],k*x[i+2],k*x[i+3],k*x[i+4],k*x[i+5],k*x[i+6],k*x[i+7]);
      }
      fprintf(stderr,"     Ambiguities        = \n");
      for(int i=0;i<wvc->ambiguities.NodeCount();i++){
	WindVectorPlus* wvp=wvc->ambiguities.GetByIndex(i);
        AngleInterval* ai=wvc->directionRanges.GetByIndex(i);
	fprintf(stderr,"      #%d spd=%g dir=%g obj=%g left=%g right=%g\n",
		i,wvp->spd,wvp->dir*180/pi,wvp->obj,ai->left*180/pi,ai->right*180/pi);     
      }
    }
    if(DEBUG){
      WindVectorPlus* wvp=wvc->ambiguities.GetByIndex(0);
      if(wvp){
	fprintf(stderr,"%d %g %g %g %g %g %g %g\n",l2a->frame.ati, wvc->nudgeWV->dir*180/pi, wvp->dir*180/pi,ANGDIF(wvc->nudgeWV->dir,wvp->dir)*180/pi,diroff,wvc->nudgeWV->spd,wvp->spd,speed);
	       }
    }
    l2b->frame.swath.Add(l2a->frame.cti,l2a->frame.ati,wvc);
    return(speed);

 }
*****/

//---------------------------//
// L2AToL2B::HybridNeuralNetRetrieve//
//---------------------------//
// retrieve using a neural network Dirnet estimates "trues0s" used to get obj

/**** Obsolete routine
 float  

 
 L2AToL2B::HybridNeuralNetRetrieve(L2A* l2a, L2B* l2b, MLPDataArray* spdnet, MLPDataArray* dirnet, GMF* gmf, Kp* kp, int need_all_looks){

    
    gmf->objectiveFunctionMethod=0;
    MeasList* meas_list = &(l2a->frame.measList);

    if(arrayNudgeFlag & l2b->frame.swath.GetCrossTrackBins()!=arrayNudgeNcti){
      fprintf(stderr,"Resolution mismatch between Nudge Array and L2B swath\n");
      exit(1);
    }
 

    //-----------------------------------//
    // check for missing wind field data //
    //-----------------------------------//
    // this should be handled by some kind of a flag!

    int any_zero = 0;
    for (Meas* meas = meas_list->GetHead(); meas; meas = meas_list->GetNext())
    {
        if (! meas->value)
        {
            any_zero = 1;
            break;
        }
    }
    if (any_zero)
    {
        return(-1);
    }
    //-----------------------------------//
    // check for wind retrieval criteria //
    //-----------------------------------//

    if (! gmf->CheckRetrieveCriteria(meas_list))
    {
        return(-1);
    }


    if (useSigma0Weights == 1)
    {
        gmf->CalculateSigma0Weights(meas_list);

        // modify the Kp C coefficient to apply the sigma0 weights to the measurements
        float correlation_length = sigma0WeightCorrLength;       
        for (Meas* meas = meas_list->GetHead(); meas; meas = meas_list->GetNext())
        {

            meas->XK *= correlation_length*correlation_length;
            
        }
	gmf->objectiveFunctionMethod=1;
    }


    //-----------------//
    // Get Nudge Field //
    //-----------------//

    WVC* wvc = new WVC();
    float ctd, speed;
    wvc->lonLat=meas_list->AverageLonLat();
    wvc->nudgeWV=new WindVectorPlus;
    if(smartNudgeFlag){
      if (! nudgeVctrField.InterpolateVectorField(wvc->lonLat,
						  wvc->nudgeWV,0))
	  {
	    delete wvc->nudgeWV;
	    wvc->nudgeWV=NULL;
	    delete wvc;
	    return(-1);
	  }
    }
    else if ( arrayNudgeFlag){
      if(l2a->frame.ati>=arrayNudgeNati){
	    delete wvc->nudgeWV;
	    wvc->nudgeWV=NULL;
	    delete wvc;
	    return(-1);
      }
      float nspd=arrayNudgeSpd[l2a->frame.ati][l2a->frame.cti];
      if(nspd<0){
	    delete wvc->nudgeWV;
	    wvc->nudgeWV=NULL;
	    delete wvc;
	    return(-1);
      }
      else{
        wvc->nudgeWV->spd=nspd;
        wvc->nudgeWV->dir=arrayNudgeDir[l2a->frame.ati][l2a->frame.cti];
      }
    }
    else if (! nudgeField.InterpolatedWindVector(wvc->lonLat,
						 wvc->nudgeWV))
	  {
	    delete wvc->nudgeWV;
	    wvc->nudgeWV=NULL;
	    delete wvc;
	    return(-1);
	  }
    // determine nudge wind direction relative to swath
    float diroff=GetNeuralDirectionOffset(l2a); 
    float reldir=diroff+wvc->nudgeWV->dir; 
    while(reldir<0)reldir+=2*pi;
    while(reldir>2*pi)reldir-=2*pi;
    //--------------------------
    // compute speed assuming nudge direction is correct
    //---------------------------
    //ctd = (l2a->frame.cti - l2a->header.zeroIndex) *
    //l2a->header.crossTrackResolution;
    ctd=(l2a->frame.cti+0.5)*l2a->header.crossTrackResolution-950.0;

    MLP* smlp=spdnet->getMLP(reldir,ctd);
   
    float dummyout;
    
    // compute inputs to speed mlp
    int nc = meas_list->NodeCount();
    int nmeas[8]={0,0,0,0,0,0,0,0};
    int look_idx=-1;

    // for now we will assume if the input vector is size 16 the it has Ku and C inputs
    // if it is size 8 it has Ku-only
    int nlooks=(spdnet->nMLPin)-USE_CTD_INPUT;
    if(!OMIT_VAR_IN_ANN) nlooks/=2;

    float mlpinvec[18]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    float eastaz[8]={0,0,0,0,0,0,0,0};
    float inc[8]={0,0,0,0,0,0,0,0};
    float wtsum[8]={0,0,0,0,0,0,0,0};
    Meas* meas = meas_list->GetHead();
    for (int c = 0; c < nc; c++)
      {
	switch (meas->measType)
	  {
	  case Meas::HH_MEAS_TYPE:
	    if (meas->scanAngle < pi / 2 || meas->scanAngle > 3 * pi / 2)
		      look_idx = 0;
		    
	    else
	      look_idx = 1;
	    break;
	  case Meas::VV_MEAS_TYPE:
	    if (meas->scanAngle < pi / 2 || meas->scanAngle > 3 * pi / 2)
	      look_idx = 2;
	    else
	      look_idx = 3;
	    break;
	  case Meas::C_BAND_HH_MEAS_TYPE:
	    if (meas->scanAngle < pi / 2 || meas->scanAngle > 3 * pi / 2)
	      look_idx = 4;
	    else
	      look_idx = 5;
	    break;
	  case Meas::C_BAND_VV_MEAS_TYPE:
	    if (meas->scanAngle < pi / 2 || meas->scanAngle > 3 * pi / 2)
	      look_idx = 6;
	    else
	      look_idx = 7;
	    break;
	  default:
	    look_idx = -1;
	    break;
	  }
        if( look_idx >= 4 && nlooks <= 4 ){
	  fprintf(stderr,"NeuralNetRetrieve error: Speed Net too small to include C-band data\n");
	  exit(1);
	}  
	if (look_idx >= 0)
	  {
	    if(!OMIT_VAR_IN_ANN){
	      mlpinvec[nlooks+look_idx] += meas->value*meas->value;
	    }
  
            float wt=1;
            if(useSigma0Weights){
	      wt=1.0/(1.0+(1.0/meas->XK));
	    }
	    mlpinvec[look_idx] += meas->value*wt;
            wtsum[look_idx]+=wt;
	    float az=meas->eastAzimuth;
	    while(az>eastaz[look_idx]/nmeas[look_idx]+pi) az-=2*pi;
            while(az<eastaz[look_idx]/nmeas[look_idx]-pi) az+=2*pi;
	    nmeas[look_idx]++;
	    eastaz[look_idx]+=az;
            inc[look_idx]+=meas->incidenceAngle;
	  }
	meas = meas_list->GetNext();
      }
    
    
    if(nmeas[3]==0) return(-1);
    for(int i=0;i<nlooks;i++){
      if(!useSigma0Weights) wtsum[i]=1;
      else if(nmeas[i]>=1) wtsum[i]/=nmeas[i];
      if(nmeas[i] > 1){
	mlpinvec[i]/=wtsum[i]*nmeas[i];
        eastaz[i]/=nmeas[i];
	inc[i]/=nmeas[i];
        if(!OMIT_VAR_IN_ANN){
	  mlpinvec[i+nlooks]/=(wtsum[i]*nmeas[i]);
	}
      }
      else if(nmeas[i]==0){
        if(need_all_looks){
	  return(-1);
	}
	mlpinvec[i]=0;
        if(!OMIT_VAR_IN_ANN)
	  mlpinvec[i+nlooks]=0.1;
      }
    }
    // inpctd_case
    if(USE_CTD_INPUT){
      mlpinvec[spdnet->nMLPin-1]=ctd;
    }
    // inpctd and reldir
    if(USE_CTD_INPUT==2){
      mlpinvec[spdnet->nMLPin-2]=reldir;
    }

    smlp->ForwardMSE(mlpinvec, &dummyout);
    speed=smlp->outp[0];

 
    // compute best speed array
    _phiCount=72; // It uses all 72 even if the neural net array is smaller
    float phistep=2*pi/_phiCount;
    wvc->directionRanges.dirIdx.SpecifyWrappedCenters(0, 2*pi, _phiCount);
    wvc->directionRanges.bestObj = (float*)malloc(sizeof(float)*_phiCount);
    wvc->directionRanges.bestSpd = (float*)malloc(sizeof(float)*_phiCount);
    for(int c=0;c<_phiCount;c++){
      float phi=(c)*phistep;
      float reldir=phi+diroff;
      while(reldir<0)reldir+=2*pi;
      while(reldir>2*pi)reldir-=2*pi;
      smlp=spdnet->getMLP(reldir,ctd);
      // check for reldir dependency in network
      if(USE_CTD_INPUT==2) mlpinvec[spdnet->nMLPin-2]=reldir;
      smlp->ForwardMSE(mlpinvec, &dummyout);
      wvc->directionRanges.bestSpd[c]=smlp->outp[0];
    }
    if (useSigma0Weights == 1){
        float cwt;
  
        // network does not include c-band inputs
	if(nlooks<=4) cwt=0;
   
        // no C-band measurements in file
        else if(nmeas[4]==0 && nmeas[5]==0 && nmeas[6]==0 && nmeas[7]==0){
	  cwt=0;
	}
        else{
	  cwt=0.5;   // weights them equally for now.. Need to talk to Scott about this....
	}
        gmf->SetCBandWeight(cwt); // only used if dirnet is NULL, not used to compute speed
    }

    float* _bestObj=wvc->directionRanges.bestObj;
    float* _bestSpd = wvc->directionRanges.bestSpd;
    if(dirnet!=NULL){
      MLP* dmlp;
      // compute directional probabilities
      for(int c=0;c<_phiCount;c++){
	float phi=(c)*phistep;
	float reldir=phi+diroff;
	while(reldir<0)reldir+=2*pi;
	while(reldir>2*pi)reldir-=2*pi;
	dmlp=dirnet->getMLP(reldir,ctd);
	// check for reldir dependency in network
	if(USE_CTD_INPUT) mlpinvec[spdnet->nMLPin-2]=reldir;
        float dirinvec[3];
        dirinvec[0]=wvc->directionRanges.bestSpd[c];
        dirinvec[1]=reldir;
	dirinvec[2]=ctd;
	dmlp->ForwardMSE(dirinvec, &dummyout);
        double obj=0.0;
        float s0mag=0.0;
	for(int i=0;i<nlooks;i++){
	  s0mag+=mlpinvec[i]*mlpinvec[i];
	}
        s0mag=sqrt(s0mag/nlooks);
	for(int i=0;i<nlooks;i++){
	  if(nmeas[i]==0) continue;
	  float ts0=dmlp->outp[i]/100.0;
	  float s0=mlpinvec[i]/s0mag;
          float var=0.06; // HACK this is how well the first QuikSCAT ANN
	                     // estimated sigma0 on 090112
          obj-=(ts0-s0)*(ts0-s0)/var;
	}
	wvc->directionRanges.bestObj[c]=obj;

      }
    }
    // if dirnet is null compute directional probabilites directly from speeds
    else{
      for(int c=0;c<_phiCount;c++){
	float phi=c*2*pi/_phiCount;
	wvc->directionRanges.bestObj[c]=gmf->_ObjectiveFunction(meas_list,wvc->directionRanges.bestSpd[c],phi,kp,0);
       } 
    }  
 
    float sum = 0.0;
    float scale = _bestObj[0];
    for (int c = 1; c < _phiCount; c++)
      {
	if (scale < _bestObj[c])
	  scale = _bestObj[c];
      }
    for (int c = 0; c < _phiCount; c++)
      {
	_bestObj[c] = exp((_bestObj[c] -  scale) / 2);
	sum += _bestObj[c];
      }
    for (int c = 0; c < _phiCount; c++)
	{
	  _bestObj[c] /= sum;
	}
    int DEBUG = 0;
    int PRINTALLINPUTS=0;
    if(DEBUG && PRINTALLINPUTS){
      for(int k=0;k<spdnet->nMLPin;k++) printf("%g ",mlpinvec[k]);
      printf("%g\n",wvc->nudgeWV->spd);
    }
    if(l2a->frame.cti==67 && l2a->frame.ati==1475) DEBUG=1;
    if(DEBUG){
      fprintf(stderr,"   Before MakeAmbigs-----------\n");
      fprintf(stderr,"     speed=%g nudgevctr(spd,dir)=(%g,%g)\n",
	      speed, wvc->nudgeWV->spd, wvc->nudgeWV->dir*180/pi);
      fprintf(stderr,"     MLPINVEC   = %g %g %g %g %g\n",
	      mlpinvec[0],mlpinvec[1],mlpinvec[2],mlpinvec[3],mlpinvec[4]);

      fprintf(stderr,"     Best_spd   = \n");
      for (int c=0;c<_phiCount/8;c++){
	int i=8*c;
	float* x = _bestSpd;
        float k= 1;
	  fprintf(stderr,"      %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f\n",
		  k*x[i],k*x[i+1],k*x[i+2],k*x[i+3],k*x[i+4],k*x[i+5],k*x[i+6],k*x[i+7]);
      }
      fprintf(stderr,"     Best_obj (percent)   = \n");
      for (int c=0;c<_phiCount/8;c++){
	int i=8*c;
	float* x = _bestObj;
        float k= 100;
	  fprintf(stderr,"      %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f\n",
		  k*x[i],k*x[i+1],k*x[i+2],k*x[i+3],k*x[i+4],k*x[i+5],k*x[i+6],k*x[i+7]);
      }
    }
    MakeAmbigsFromDirectionArrays(wvc,0);
    if(DEBUG){
      fprintf(stderr,"   After MakeAmbigs before BuildDirectionRanges-----------\n");
      fprintf(stderr,"     Best_spd   = \n");
      for (int c=0;c<_phiCount/8;c++){
	int i=8*c;
	float* x = _bestSpd;
        float k= 1;
	  fprintf(stderr,"      %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f\n",
		  k*x[i],k*x[i+1],k*x[i+2],k*x[i+3],k*x[i+4],k*x[i+5],k*x[i+6],k*x[i+7]);
      }
      fprintf(stderr,"     Best_obj (percent)   = \n");
      for (int c=0;c<_phiCount/8;c++){
	int i=8*c;
	float* x = _bestObj;
        float k= 100;
	fprintf(stderr,"      %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f\n",
		k*x[i],k*x[i+1],k*x[i+2],k*x[i+3],k*x[i+4],k*x[i+5],k*x[i+6],k*x[i+7]);
      }
      fprintf(stderr,"     Ambiguities        = \n");
      for(int i=0;i<wvc->ambiguities.NodeCount();i++){
	WindVectorPlus* wvp=wvc->ambiguities.GetByIndex(i);
	fprintf(stderr,"      #%d spd=%g dir=%g obj=%g\n",
		i,wvp->spd,wvp->dir*180/pi,wvp->obj);     
      }
    }
    BuildDirectionRanges(wvc, 0.8);// uses 80% threshold 
    if(DEBUG){
      fprintf(stderr,"   After BuildDirectionRanges-----------\n");
      fprintf(stderr,"     Best_spd   = \n");
      for (int c=0;c<_phiCount/8;c++){
	int i=8*c;
	float* x = _bestSpd;
        float k= 1;
	  fprintf(stderr,"      %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f\n",
		  k*x[i],k*x[i+1],k*x[i+2],k*x[i+3],k*x[i+4],k*x[i+5],k*x[i+6],k*x[i+7]);
      }
      fprintf(stderr,"     Best_obj (percent)   = \n");
      for (int c=0;c<_phiCount/8;c++){
	int i=8*c;
	float* x = _bestObj;
        float k= 100;
	fprintf(stderr,"      %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f\n",
		k*x[i],k*x[i+1],k*x[i+2],k*x[i+3],k*x[i+4],k*x[i+5],k*x[i+6],k*x[i+7]);
      }
      fprintf(stderr,"     Ambiguities        = \n");
      for(int i=0;i<wvc->ambiguities.NodeCount();i++){
	WindVectorPlus* wvp=wvc->ambiguities.GetByIndex(i);
        AngleInterval* ai=wvc->directionRanges.GetByIndex(i);
	fprintf(stderr,"      #%d spd=%g dir=%g obj=%g left=%g right=%g\n",
		i,wvp->spd,wvp->dir*180/pi,wvp->obj,ai->left*180/pi,ai->right*180/pi);     
      }
    }
    if(DEBUG){
      WindVectorPlus* wvp=wvc->ambiguities.GetByIndex(0);
      if(wvp){
	fprintf(stderr,"%d %g %g %g %g %g %g %g\n",l2a->frame.ati, wvc->nudgeWV->dir*180/pi, wvp->dir*180/pi,ANGDIF(wvc->nudgeWV->dir,wvp->dir)*180/pi,diroff,wvc->nudgeWV->spd,wvp->spd,speed);
	       }
    }
    l2b->frame.swath.Add(l2a->frame.cti,l2a->frame.ati,wvc);
    return(speed);

 }
*****/

#define INTERP_RATIO 8
#define MERGE_BORDERS 0
int
L2AToL2B::BuildDirectionRanges(
     WVC*   wvc,
     float threshold){
     //---------------->>>>>>>>>>>>>>>>>>>>>>>> debugging tools
     static int wvcno=0;
     // static FILE* dbg=fopen("debugfile","w");
     wvcno++;
     if(wvcno==48839){
       int c=0;
       c=c+1; // breakpoint placeholder for gdb
     }
     //---------------->>>>>>>>>>>>>>>>>>>>>>>> debugging tools
     int pdfarraysize=_phiCount;
     float pdfstepsize=2*pi/_phiCount;
     float _phiStepSize=pdfstepsize;
     float* pdf =  wvc->directionRanges.bestObj;

     // Method reduce size of probability bins by a factor of INTERP_RATIO
     // in order to eliminate quantization effects.
     // Cubic Spline Interpolation is used.

     if(INTERP_RATIO>1){

       pdfarraysize*=INTERP_RATIO;
       pdfstepsize/=INTERP_RATIO;
       pdf=new float[pdfarraysize];
       float sum=0.0;
       float* _bestObj = wvc->directionRanges.bestObj;
       // Set up arrays for wraparound cubic spline
       // By including four extra wrapped points on each end
       double* xarray = new double[_phiCount+8];
       double* yarray = new double[_phiCount+8];
       for(int i = 0; i<4;i++){
	 xarray[i]=_phiStepSize*(i-4);
	 yarray[i]=_bestObj[i+_phiCount-4];
       }
       for(int i=4;i<_phiCount+4;i++){
	 xarray[i]=_phiStepSize*(i-4);
	 yarray[i]=_bestObj[i-4];
       }
       for(int i=_phiCount+4;i<_phiCount+8;i++){
	 xarray[i]=_phiStepSize*(i-4);
	 yarray[i]=_bestObj[i-_phiCount-4];
       }

       // Compute second derivates for cubic spline
       double* y2array = new double[_phiCount+8];
       cubic_spline(xarray,yarray,_phiCount+8,1e+40,1e+40,y2array);

       double xvalue, yvalue;
       for(int c=0;c<pdfarraysize;c++){

         // compute interpolation direction
	 xvalue=c*pdfstepsize;

         //Interpolate
         interpolate_cubic_spline(xarray,yarray,y2array,_phiCount+8,xvalue,
                  &yvalue);

         // To eliminate gross errors from spline
         // bound below by 0.5*min two neighboring original samples
         // and above by 2.0* max two neighboring original samples

         int idxleft=int(xvalue/_phiStepSize)%_phiCount;
         int idxright=(idxleft+1)%_phiCount;
         float lower_bound=_bestObj[idxleft];
         float upper_bound=_bestObj[idxright];
         if(lower_bound > upper_bound){
	   lower_bound=_bestObj[idxright];
	   upper_bound=_bestObj[idxleft];
	 }
         lower_bound*=0.5;
         upper_bound*=2;
         if(yvalue<lower_bound) yvalue=lower_bound;
         if(yvalue>upper_bound) yvalue=upper_bound;
         pdf[c]=yvalue;
         sum+=pdf[c];
       }

       for(int c=0;c<pdfarraysize;c++) pdf[c]/=sum;

       delete[] xarray;
       delete[] yarray;
       delete[] y2array;
     } // END If( INTERP_RATIO > 1)

     int num=wvc->ambiguities.NodeCount();
     if(num==0) return(1);
     AngleInterval* range=new AngleInterval[num];

     // Initialize Ranges to Width 0
     int offset=0;
     for(WindVectorPlus* wvp=wvc->ambiguities.GetHead();wvp;
     wvp=wvc->ambiguities.GetNext(), offset++){
       range[offset].SetLeftRight(wvp->dir,wvp->dir);
     }

     // Initialize intermediate variables
     float prob_sum=0;
     int* dir_include=new int[pdfarraysize];
     for(int c=0;c<pdfarraysize;c++) dir_include[c]=0;
     int* left_idx= new int[num];
     int* right_idx=new int[num];
     for(int c=0;c<num;c++){
        left_idx[c]=int(floor(range[c].left/pdfstepsize));
        right_idx[c]=(left_idx[c]+1)%pdfarraysize;
     }
     //Add the maximal probability among directions not included in range
     //to prob_sum, and expand ranges accordingly.
     float max=0.0;
     while(1){

       // Determine maximum available directions by sliding down the
       // peaks on both sides
       max=0.0;
       int max_idx=-1;
       int right=0;
       for(int c=0;c<num;c++){
     if(pdf[left_idx[c]]>max && !dir_include[left_idx[c]]){
       max=pdf[left_idx[c]];
       right=0;
       max_idx=c;
     }
     if(pdf[right_idx[c]]>max && !dir_include[right_idx[c]]){
       max=pdf[right_idx[c]];
       right=1;
       max_idx=c;
     }
       }
       if(max==0.0){
	 fprintf(stderr,"GMF::BuildDirectionRanges() Max=0.0????? wvcno=%d\n",wvcno);
         fprintf(stderr,"max=%g prob_sum=%g thresh=%g\n",max,prob_sum,threshold);
         for(int c=0;c<num;c++)
       fprintf(stderr,"%d left %d right %d\n",c,left_idx[c],right_idx[c]);

         break;
       }
       // Add maximum to total included probability
       prob_sum+=max;
       
       // Break if threshold reached
       if(prob_sum>threshold) break;

       // Expand range to include best available direction
       //  and update intermediate variables
       if(right){
	 float tmp_left=range[max_idx].left;
         float tmp_right=right_idx[max_idx]*pdfstepsize+0.5*pdfstepsize;
         dir_include[right_idx[max_idx]]=1;
	 range[max_idx].SetLeftRight(tmp_left,tmp_right);
	 right_idx[max_idx]++;
	 right_idx[max_idx]%=pdfarraysize;
       }
       else{
	 float tmp_right=range[max_idx].right;
         float tmp_left=left_idx[max_idx]*pdfstepsize-0.5*pdfstepsize;
         dir_include[left_idx[max_idx]]=1;
	 range[max_idx].SetLeftRight(tmp_left,tmp_right);
	 left_idx[max_idx]--;
	 if(left_idx[max_idx]<0) left_idx[max_idx]+=pdfarraysize;
       }
     }


     //  2) Merge bordering intervals (for now but if this causes problems then
     //   eliminate this step and replace with trough finding.


     if(MERGE_BORDERS){

       // Merge bordering intervals.
       for(int c=0;c<num-1;c++){
	 for(int d=c+1; d< num; d++){
	   if(range[c].left == range[d].right){
	     // incorporate range d into range c
	     range[c].left = range[d].left;
	     // eliminate range[d]
	     for(int e=d;e<num-1;e++){
	       range[e]=range[e+1];
	     }
	     num--;
	     // eliminate ambiguity d
	     wvc->ambiguities.GetByIndex(d);
	     WindVectorPlus* wvp=wvc->ambiguities.RemoveCurrent();
	     delete wvp;
	   }
	   else if(range[c].right == range[d].left){
	     // incorporate range d into range c
	     range[c].right = range[d].right;
	     // eliminate range[d]
	     for(int e=d;e<num-1;e++){
	       range[e]=range[e+1];
	     }
	     num--;
	     // eliminate ambiguity d
	     wvc->ambiguities.GetByIndex(d);
	     WindVectorPlus* wvp=wvc->ambiguities.RemoveCurrent();
	     delete wvp;
	   }
	 }
       }
       
     }

     for(int c=0;c<num;c++){
       AngleInterval* ai=new AngleInterval;
       *ai=range[c];
       wvc->directionRanges.Append(ai);
     }

     //------------------------------------------>>>> debugging tools
     //for(int c=0;c<pdfarraysize;c++)
     //  fprintf(dbg,"%g %g PDF:WVC#%d\n",c*pdfstepsize*rtd,pdf[c],wvcno);
     //fprintf(dbg,"& PDF:WVC#%d\n",wvcno);
     //for(int c=0;c<_phiCount;c++)
     //  fprintf(dbg,"%g %g BestObj:WVC#%d\n",c*_phiStepSize*rtd,_bestObj[c],wvcno);
     //fprintf(dbg,"& BestObj:WVC#%d\n",wvcno);
     //for(int c=0;c<num;c++)
     //      fprintf(dbg,"%d %g Left:WVC#%d\n",c,rtd*range[c].left,wvcno);
     //fprintf(dbg,"& Left:WVC#%d\n",wvcno);
     //for(int c=0;c<num;c++)
     //      fprintf(dbg,"%d %g Right:WVC#%d\n",c,rtd*range[c].right,wvcno);
     //fprintf(dbg,"& Right:WVC#%d\n",wvcno);
     //------------------------------------------>>>> debugging tools

     if (INTERP_RATIO > 1) delete[] pdf;
     delete[] range;
     delete[] dir_include;
     delete[] left_idx;
     delete[] right_idx;
     return(1);
}

//------------------------------------------//
// Routine to compute direction offset for  //
// along track and cross track index        //
//------------------------------------------//
/*** Obsolete version of obsolete routine
float L2AToL2B::GetNeuralDirectionOffset(L2A* l2a){

  // diroff is the value to add to the reldir form the neural network
  // to get the truedir.
  //
  // The direction used in training the network is
  // traindir=reldir+ann_train_diroff;
  //
  // Since the direction offset was only approximated in neural net training
  // we need to subtract out the actual spacecraft velocity vector at the
  // training atd and ctd to get the truereldir, i.e., 
  // the angle counterclockwise from 
  // the velocity vector to the downwind direction
  // truereldir=ann_train_diroff - training_spacecraft_velocity_CCW_fromeast
  //
  // Finally the true dir is computed by adding the true spacecraft velocity 
  // vector at the current time
  // true_dir=truereldir+current_spacecraft_velocity_CCW_fromeast;
  // so diroff=truedir-reldir=ann_train_diroff-train_scvel+current_scvel

  // If we ever eliminate the diroff computation in the training we
  // will need to eliminate this check
  if(ann_train_diroff==0){
    fprintf(stderr,"L2AToL2B:GetNeuralDirectionOffset: ann_train_diroff is 0. Fix config file\n");
    exit(0);
  }

  if(ann_train_ati==0){
    fprintf(stderr,"L2AToL2B:GetNeuralDirectionOffset: ann_train_ati is 0. Fix config file\n");
    exit(0);
  }

  if(orbitInclination==0){
    fprintf(stderr,"L2AToL2B:GetNeuralDirectionOffset: orbitInclination is 0. Fix config file\n");
    exit(0);
  }


  float ctd = (l2a->frame.cti - l2a->header.zeroIndex) *
  l2a->header.crossTrackResolution;
  float atd = l2a->frame.ati*l2a->header.alongTrackResolution;
  float train_atd = ann_train_ati*l2a->header.alongTrackResolution;

  float train_scvelang = GetSpacecraftVelocityAngle(train_atd,ctd);
  float scvelang = GetSpacecraftVelocityAngle(atd,ctd);
  
  float diroff=ann_train_diroff-train_scvelang+scvelang;
  int DEBUG =0;
  if(DEBUG && l2a->frame.cti==17){
    float atdratio=atd/(NOMINAL_TRACK_LENGTH);
    int c=(int)(360*atdratio);
    if(c==360) c=0;
    fprintf(stderr,"\nDUMP of WVC ati=%d cti=%d\n",l2a->frame.ati,l2a->frame.cti);
      fprintf(stderr,"   After diroff computation-----------\n");
      fprintf(stderr,"   ctd=%g atd=%g atdratio=%g diroff=%g gclat=%g train_atd=%g\n",ctd,atd,atdratio,diroff*180/pi,atdToNadirLat[c]*180/pi,train_atd);
      fprintf(stderr,"   ann_train_diroff=%g train_scvelang=%g scvelang=%g\n",ann_train_diroff*180/pi,train_scvelang*180/pi,scvelang*180/pi);
  }
  while(diroff>two_pi) diroff-=two_pi;
  while(diroff<0) diroff+=two_pi;
  return(diroff);
}
*****/

/**** Newer version of Obsolete routine
float L2AToL2B::GetNeuralDirectionOffset(L2A* l2a){
    int n=0;
    float az=0;
    float azave=0;
    MeasList* meas_list= &(l2a->frame.measList);
    int look_idx=0;
    Meas* meas = meas_list->GetHead();
    int nc=meas_list->NodeCount();
    for (int c = 0; c < nc; c++)
      {
	switch (meas->measType)
	  {
	  case Meas::HH_MEAS_TYPE:
	    if (meas->scanAngle < pi / 2 || meas->scanAngle > 3 * pi / 2)
		      look_idx = 0;
		    
	    else
	      look_idx = 1;
	    break;
	  case Meas::VV_MEAS_TYPE:
	    if (meas->scanAngle < pi / 2 || meas->scanAngle > 3 * pi / 2)
	      look_idx = 2;
	    else
	      look_idx = 3;
	    break;
	  case Meas::C_BAND_HH_MEAS_TYPE:
	    if (meas->scanAngle < pi / 2 || meas->scanAngle > 3 * pi / 2)
	      look_idx = 4;
	    else
	      look_idx = 5;
	    break;
	  case Meas::C_BAND_VV_MEAS_TYPE:
	    if (meas->scanAngle < pi / 2 || meas->scanAngle > 3 * pi / 2)
	      look_idx = 6;
	    else
	      look_idx = 7;
	    break;
	  default:
	    look_idx = -1;
	    break;
	  }
	if (look_idx == 3)
	  {
	    az=meas->eastAzimuth;
	    while(az>azave/n+pi) az-=2*pi;
            while(az<azave/n-pi) az+=2*pi;
            n++;
	    azave+=az;
	  }
  
	meas = meas_list->GetNext();
      }
    azave/=n;
    return(-azave);
}
***/

float
L2AToL2B::computeGroundTrackParameters(){
  float rp=r2_earth; //polar radius
  float re=r1_earth; // equatorial radius
  float z=-sqrt((re*re*rp*rp)/(rp*rp/tan(orbitInclination)/tan(orbitInclination)+re*re));
  float tano=tan(orbitInclination);
  float y=z/tano;
  float zmin=z;
  float zmax=-z;
  float x=0;
  EarthPosition p(x,y,z);
  EarthPosition p0=p;
  double alt,gclat,lon;
  double lat[2001];
  double atd[2001];
  p.GetAltLonGCLat(&alt,&lon,&gclat);
  lat[0]=gclat;
  atd[0]=0;
  for(int c=1;c<2001;c++){
    if(c<=1000) z=zmin+c*(zmax-zmin)/1000.0;
    else z=zmax-(c-1000)*(zmax-zmin)/1000.0;
    y=z/tano;
    x=re*sqrt(1-y*y/re/re-z*z/rp/rp);
    if(c>1000)x=-x;
    // numerical precision bug fix
    if(c==2000 || c==1000) x=0;
    EarthPosition pnext(x,y,z);
    double dp=pnext.SurfaceDistance(p);
    groundTrackLength+=dp;
    atd[c]=groundTrackLength;
    p=pnext;
    p.GetAltLonGCLat(&alt,&lon,&gclat);
    lat[c]=gclat;
  }
  for(int c=0;c<2001;c++){
    atd[c]/=groundTrackLength;
  }
  p=p0;
  p.GetAltLonGCLat(&alt,&lon,&gclat);
  atdToNadirLat[0]=gclat;
  int i=0;
  for(int c=1;c<360;c++){
    float a=c/360.0;
    while(atd[i]<a && i< 2000 ) i++;
    int i0=i-1;
    int i1=i;
    double w1=(a-atd[i0])/(atd[i1]-atd[i0]);
    double w0=1-w1;
    atdToNadirLat[c]=w0*lat[i0]+w1*lat[i1];
  }
  return(groundTrackLength);
}
// Routine to estimate spacecraft velocity vector
// Or more precisely the vector from (atd,ctd) to (atd+epsilon,ctd)
// CCW from east
float
L2AToL2B::GetSpacecraftVelocityAngle(float atd, float ctd){
  static float gtl=computeGroundTrackParameters(); // static so this is only called once
  gtl=groundTrackLength; // avoids compiler warning

  // compute constants

  // estimate ground track as an ellipse
  float re=r1_earth;  // equatorial radius of earth
  float rp=r2_earth;  // polar radius
 

  // approximate mean anomaly from along track distance
  float atd_ratio=atd/(NOMINAL_TRACK_LENGTH);

  
  // compute latitude at  along track distance atd and ctd=0
  int i0=(int)(360*atd_ratio);
  if(i0==360) i0 = 0;
  int i1=i0+1;
  if(i1==360) i1=0;
  float w1=atd_ratio-i0/360.0;
  if(w1>360.0) w1=0;
  float w0=1.0-w1;
  float nadirgclat=atdToNadirLat[i0]*w0+atdToNadirLat[i1]*w1;

  // compute nadir x,y,z coordinates
  double slat=sin(nadirgclat);
  double nadrad=re*(1.0-flat*slat*slat);
  float z=slat*nadrad;
  float y=z/tan(orbitInclination);
  float x=sqrt(1-z*z/rp/rp -y*y/re/re)*sqrt(re*re);
  if(atd_ratio>0.5) x=-x;

  // compute coordinates for atd,ctd and cross track direction
  Vector3 nadir(x,y,z);
  double slattop=sin(atdToNadirLat[180]);
  double toprad=re*(1.0-flat*slattop*slattop);
  double ztop=toprad*slattop;
  Vector3 top(0,ztop/tan(orbitInclination),ztop);
  Vector3 ctddir=top & nadir;
  ctddir=ctddir/ctddir.Magnitude();
  float ctangle=ctd/nadir.Magnitude(); // approximation assumes local radius doesn't change
                                        // much within swath
  float normaldist;//distance between ground track great circle and ctd parallel
  normaldist=ctd*sin(ctangle)/ctangle;
  
  Vector3  loc=nadir+ctddir*normaldist;
  loc=loc/(loc.Magnitude());
  float locgclat=asin(loc.Get(2));
  // first compute surface vector without earth rotation
  Vector3 sc_dir=loc & ctddir;
  if(atd_ratio>0.5) sc_dir=-sc_dir;
  Vector3 north(0,0,1);
  Vector3 east=north & loc;
  float northcomp= sc_dir%north;
  float eastcomp=sc_dir%east;
  

  // add in Earth rotation
  float scspeed=groundTrackLength/6000;       // ground speed at nadir (approximate)
                                              // assumes 100 min orbit

  float rotspeed=cos(locgclat)*re*pi/(12*3600);
  northcomp=northcomp*scspeed;  eastcomp=eastcomp*scspeed-rotspeed;
  

  // compute angle CCW from east
  float outangle=atan2(northcomp,eastcomp);
  return(outangle);
}

//------------------------------------------//
// Routine to complete construction of WVC  //
// from neural net outputs                  //
//------------------------------------------//
int  L2AToL2B::MakeAmbigsFromDirectionArrays(WVC* wvc, float diroff){
  int peaks[4];
  static int wvcno=0;
  wvcno++;
     if(wvcno==48839){
       int c=0;
       c=c+1; // breakpoint placeholder for gdb
     }
  float* probs=wvc->directionRanges.bestObj;
  float* spds =wvc->directionRanges.bestSpd;
  static float finespd[360];
  static float fineprob[360];
  static int mask[1000];
  if(_phiCount>1000){
    fprintf(stderr,"L2AToL2B::MakeAmbigsFromDirectionArrays: Phi count too large!!!\n");
    exit(1);
  }  

  // spline interpolate
  //---------  Only linear interpolation for now !!!!
  // shift is performed during interpolation

  float phistep=2*pi/_phiCount;
  float cstep=pi/180;
  for(int c=0;c<360;c++){
    
    float idx=diroff/phistep+c*cstep/phistep;
    while(idx<0) idx+=_phiCount;
    while(idx>=_phiCount) idx-=_phiCount;
    int i0=(int)(idx);
    int i1=i0+1;
    if(i1==_phiCount) i1=0;
    float w1= idx-i0;
    float w0=1-w1;

    finespd[c]=spds[i0]*w0+spds[i1]*w1;
    fineprob[c]=probs[i0]*w0+probs[i1]*w1;
  }
  // find peaks
  for(int c=0;c<360;c++) mask[c]=0;
  for(int p=0;p<4;p++){
    peaks[p]=-1;
    float maxp=0;
    // find maximum remaining peak
    for(int c=0;c<360;c++){
      if(fineprob[c]>maxp && mask[c]==0){
	peaks[p]=c;
        maxp=fineprob[c];
      }
    }
    // exclude shoulders around peak
    if(peaks[p]!=-1){
      mask[peaks[p]]=1;
      // positive shoulder
      for(int c=(peaks[p]+1)%360;c!=peaks[p];c=((c+1)%360)){
	int c0=(c-1)%360;
	if(c0==-1) c0=359;
        if(fineprob[c]<=fineprob[c0]){
	  mask[c]=1;
	}
        else{
	  break;
	}
      }


      // negative shoulder
      for(int c=(peaks[p]-1+360)%360;c!=peaks[p];c=((c-1+360)%360)){
	int c0=(c+1)%360;
        if(fineprob[c]<=fineprob[c0]){
	  mask[c]=1;
	}
        else{
	  break;
	}
      }

    }
  }
  //----------- shift arrays
  for(int c=0;c<_phiCount;c++){
    int fineidx=(int)(c*360/_phiCount +0.5);
    probs[c]=fineprob[fineidx];
    spds[c]=finespd[fineidx];
  }

  //------------ create ambiguities
  for(int p=0;p<4;p++){
    if(peaks[p]==-1) break;
    WindVectorPlus* wvp= new WindVectorPlus();
    wvp->spd=finespd[peaks[p]];
    wvp->dir=peaks[p]*pi/180;
    wvp->obj=2*log(fineprob[peaks[p]]);
    wvc->ambiguities.Append(wvp);
    wvp=NULL;
  }

  
  return(1);
}

//--------------------------------//
// L2AToL2B::convertMeasToMPL_IOType //
//--------------------------------//
// given an instance of a meas object and whether we're looking of the mean or variance of
// that measurement, and an output buffer return the equivalent MPL_IOType string
// type is either 'MEAN' or 'VAR' or CORR
// output is in out_buf; return val is 1 on sucess 0 on failure
int L2AToL2B::convertMeasToMLP_IOType(Meas* meas, char *type, char *out_buf) {
    char band[IO_TYPE_STR_MAX_LENGTH] = "", pol[IO_TYPE_STR_MAX_LENGTH] = "",
        beam[IO_TYPE_STR_MAX_LENGTH] = "", look_dir[IO_TYPE_STR_MAX_LENGTH] = "";
    switch (meas->measType)
    {
        case Meas::HH_MEAS_TYPE:
        case Meas::VV_MEAS_TYPE:
            strcat(band, "K");
            break;
        case Meas::C_BAND_HH_MEAS_TYPE:
        case Meas::C_BAND_VV_MEAS_TYPE:
            strcat(band, "C");
            break;
        default:
            fprintf(stderr, "ERROR: L2AToL2B::convertMeasToMPL_IOType: Unknown MeasType: %d\n", meas->measType);
            return 0;
    }
    
    switch (meas->measType)
    {
    case Meas::HH_MEAS_TYPE:
    case Meas::C_BAND_HH_MEAS_TYPE:
      strcat(pol, "HH");
      strcat(beam, "INNER");
      break;
    case Meas::VV_MEAS_TYPE:
    case Meas::C_BAND_VV_MEAS_TYPE:
      strcat(pol, "VV");
      strcat(beam, "OUTER");
      break;
    default:
      break;
    }
    
    if (meas->scanAngle < pi / 2 || meas->scanAngle > 3 * pi / 2)
        strcat(look_dir, "FORE");
    else
        strcat(look_dir, "AFT");
    
    // clear the output buffer
    out_buf[0] = NULL;
    
    // copy the strings into the buffer
    strcat(out_buf, "S0_");
    strcat(out_buf, type); strcat(out_buf, "_");
    strcat(out_buf, band); strcat(out_buf, "_");
    strcat(out_buf, pol); strcat(out_buf, "_");
    strcat(out_buf, beam); strcat(out_buf, "_");
    strcat(out_buf, look_dir);
    return 1;
}

int 
L2AToL2B::GenSigma0Flags( MeasList* meas_list, GMF *gmf, WVC* wvc ) {
  
  float flagThreshDiversity = gmf->minimumAzimuthDiversity; // radians, from config file
  int   flagThreshGoodS0    = 4;
  
  int   foreInnerFound = 0;
  int   foreOuterFound = 0;
  int   aftInnerFound  = 0;
  int   aftOuterFound  = 0;
  int   landFound      = 0;
  int   iceFound       = 0;
  float max_div_found  = 0;
  int   count_good_s0  = 0;
  
  int   meas_count = meas_list->NodeCount();
  float s0_azi_vector[meas_count];
  
  // Check for land + ice, only flag if found in this function
  for (Meas* meas = meas_list->GetHead(); meas; meas = meas_list->GetNext()) {
    
    // Check for land
    if( meas->landFlag == 1 || meas->landFlag == 3 )
      landFound = 1;
    
    // Check if any slices in this composite had a non-zero land fraction
    if( do_coastal_processing && meas->bandwidth > 0 )
      landFound = 1;
    
    // Check for ice      
    if( meas->landFlag == 2 || meas->landFlag == 3 )
      iceFound = 1;
    
    // Check for usable sigma0
    double snr = meas->value * meas->XK / meas->EnSlice;
    if( meas->landFlag == 0 && finite(meas->value) && 10*log10(fabs(snr)) > -20 ) {
      // Check flavor of sigma0
      float antazi = meas->scanAngle * rtd;
      if( antazi >= 180 ) antazi -= 360;
      if( meas->measType == Meas::VV_MEAS_TYPE ) {
        if( fabs(antazi) < 90 )
          foreOuterFound++;
        else
          aftOuterFound++;
      }
      else {
        if( fabs(antazi) < 90 )
          foreInnerFound++;
        else
          aftInnerFound++;
      }
      s0_azi_vector[count_good_s0] = meas->eastAzimuth;
      count_good_s0++;
    }
  }
  
  if( count_good_s0 > 0 ) {
    for( int ii = 0; ii < count_good_s0-1; ++ii ) {
      for( int jj = ii+1; jj < count_good_s0; ++jj ) {
        float azidiv = ANGDIF( s0_azi_vector[ii], s0_azi_vector[jj] );
        max_div_found = (azidiv > max_div_found) ? azidiv : max_div_found;
      }
    }
  }
  
  // Set flavor count
  wvc->numInFore = foreInnerFound;
  wvc->numInAft = aftInnerFound;
  wvc->numOutFore = foreOuterFound;
  wvc->numOutAft = aftOuterFound;

  int allfour = foreOuterFound*aftOuterFound*foreInnerFound*aftInnerFound;
  
  // Set flags
  wvc->qualFlag = (wvc->qualFlag & ~L2B_QUAL_FLAG_ADQ_S0)
   | (count_good_s0<flagThreshGoodS0) * L2B_QUAL_FLAG_ADQ_S0;

  wvc->qualFlag = (wvc->qualFlag & ~L2B_QUAL_FLAG_ADQ_AZI_DIV)
   | (max_div_found<flagThreshDiversity) * L2B_QUAL_FLAG_ADQ_AZI_DIV;
  
  wvc->qualFlag = (wvc->qualFlag & ~L2B_QUAL_FLAG_FOUR_FLAVOR)
   | (allfour == 0) * L2B_QUAL_FLAG_FOUR_FLAVOR;

  wvc->qualFlag = (wvc->qualFlag & ~L2B_QUAL_FLAG_LAND)
   | (landFound != 0) * L2B_QUAL_FLAG_LAND;

  wvc->qualFlag = (wvc->qualFlag & ~L2B_QUAL_FLAG_ICE)
   | (iceFound  != 0) * L2B_QUAL_FLAG_ICE;
  
  // For compatability
  wvc->landiceFlagBits = 0;
  wvc->landiceFlagBits = (wvc->landiceFlagBits & ~LAND_ICE_FLAG_COAST)
   | (landFound != 0) * LAND_ICE_FLAG_COAST;

  wvc->landiceFlagBits = (wvc->landiceFlagBits & ~LAND_ICE_FLAG_ICE)
   | (iceFound != 0) * LAND_ICE_FLAG_ICE;
  
  return(1);
}

//---------------------------//
// L2AToL2B::ConvertAndWrite //
//---------------------------//
// returns 0 on failure for bad reason (memory, etc.)
// returns 1 on success
// returns higher numbers for other reasons
int
L2AToL2B::ConvertAndWrite(
    L2A*  l2a,
    GMF*  gmf,
    Kp*   kp,
    L2B*  l2b)
{
    static int last_rev_number = 0;

    // initialize MLP inputs arrays
    for(int c=0;c<NUM_MLP_IO_TYPES;c++){
      MLP_inpt_array[c]=0;
      MLP_valid_array[c]=false;
    }

    MeasList* meas_list = &(l2a->frame.measList);

    //-----------------------------------//
    // check for missing wind field data //
    //-----------------------------------//
    // this should be handled by some kind of a flag!

    int any_zero = 0;
    for (Meas* meas = meas_list->GetHead(); meas; meas = meas_list->GetNext())
    {
        if (! meas->value)
        {
            any_zero = 1;
            break;
        }
    }
    if (any_zero)
    {
        return(3);
    }
    //----------------------
    // Add Kprc Error
    //-----------------------

    float kprc_err=kprc.GetNumber();
    for (Meas* meas = meas_list->GetHead(); meas; meas = meas_list->GetNext())
    {
      meas->value*=(1+kprc_err);
    }
    
    // allocate WVC
    WVC* wvc = new WVC();
    
    // Generate sigma0 related quality flags 3/9/2011 AGF
    GenSigma0Flags( meas_list, gmf, wvc );
    
    { // Remove land/ice flagged meas in meas_list
      Meas* meas = meas_list->GetHead();
      while(meas) {
        if( meas->landFlag == 0 )
          meas = meas_list->GetNext();
        else {
          meas = meas_list->RemoveCurrent();
          delete meas;
          meas = meas_list->GetCurrent();
        }
      }
    }


    //-----------------------------------//
    // check for wind retrieval criteria //
    //-----------------------------------//

    if (! gmf->CheckRetrieveCriteria(meas_list))
    {
        delete wvc;
        return(4);
    }

    if (useSigma0Weights == 1)
    {
        gmf->CalculateSigma0Weights(meas_list);

        // modify the Kp C coefficient to apply the sigma0 weights to the measurements
        float correlation_length = sigma0WeightCorrLength;       
        for (Meas* meas = meas_list->GetHead(); meas; meas = meas_list->GetNext())
        {

            meas->XK *= correlation_length*correlation_length;
            
            //          meas->C+=1.0/(correlation_length*correlation_length*meas->XK);  // meas->XK holds the sigma0 weight
//            meas->A*=(1.0+1.0/(correlation_length*correlation_length*meas->XK));  // meas->XK holds the sigma0 weight
//            printf("Beam %d Inc %g scan %g aziwidth %g ranwidth %g XK %g\n",meas->beamIdx,meas->incidenceAngle*rtd,
//                   meas->scanAngle*rtd,meas->azimuth_width,meas->range_width,meas->XK);
            
        }

    }

    //---------------------------------------//
    // Apply neural net rain Sig0 Correction //
    //---------------------------------------//

    // NOTE: MLPs read and Input Buffers allocated in ConfigL2AToL2B
    if(rainCorrectMethod==ANN_NRCS_CORRECTION)
    {
      ComputeMLPInputs(l2a,meas_list,NULL);
          
      // if MLP inputs are valid
      if(s0corr_mlp.AssignInputs(MLP_inpt_array,MLP_valid_array)){
	  s0corr_mlp.Forward();

	  // apply those correction factors to all the measurements
	  for (Meas* meas = meas_list->GetHead(); meas; meas = meas_list->GetNext())
	    {
              char* meas_type_buff;
	      convertMeasToMLP_IOType(meas, "CORR", meas_type_buff);
	      int out_i = s0corr_mlp.findIOTypeInd(meas_type_buff, MLP_IO_OUT_TYPE);
	      if (out_i > -1) {
                float corr = pow(10.0, 0.1*s0corr_mlp.outp[out_i]);  // convert dB into straight amplitude
                // apply correction
                meas->value /= corr;
	      } else {
                fprintf(stderr, "L2AToL2B::ConvertAndWrite: Error: neural network does not output a correction for measurement type: %s\n",
			meas_type_buff);
                exit(1);
	      }
	    } // end loop over measurements to apply corrections
      } // end valid inputs to MLP case
      // for now exit if inputs are invalid (this will toss out single beam swath )
      else{
    delete wvc;
	return(18);
      }
    } // end of RainCorrectMethod==ANN_NRCS_CORRECTION
    //---------------//
    // retrieve wind //
    //---------------//

    
    float ctd, speed, dir;
    WindVectorPlus* wvp;
    static int num = 1;
    
    
    //HACK for breakpoint in gdb
    if(l2a->frame.cti==722 && l2a->frame.ati==2181){
      int bp=0;
      bp++;
    }
    switch (wrMethod)
    {
    case GS:
        if (! gmf->RetrieveWinds_GS(meas_list, kp, wvc))
        {
            delete wvc;
            return(5);
        }
        break;
/*
    case GS_FIXED:
        if (! gmf->RetrieveWinds_GSFixed(meas_list, kp, wvc))
        {
            delete wvc;
            return(6);
        }
        break;
*/
    case H1:
        if (! gmf->RetrieveWinds_H1(meas_list, kp, wvc))
        {
            delete wvc;
            return(7);
        }
        break;
    case H2:
        if (! gmf->RetrieveWinds_H2(meas_list, kp, wvc))
        {
            delete wvc;
            return(7);
        }
        break;
    case H3:
        if (! gmf->RetrieveWinds_H2(meas_list, kp, wvc, 1))
        {
            delete wvc;
            return(8);
        }
        break;
    case S1:
        if (! gmf->RetrieveWinds_H2(meas_list, kp, wvc, 2))
        {
            delete wvc;
            return(9);
        }
        break;
    case S2:
        if (! gmf->RetrieveWinds_S2(meas_list, kp, wvc))
        {
            delete wvc;
            return(10);
        }

#ifdef S2_DEBUG_INTERVAL
        if (num % S2_DEBUG_INTERVAL == 0)
        {
            ctd = (l2a->frame.cti - l2a->header.zeroIndex) *
                l2a->header.crossTrackResolution;
            wvp = wvc->ambiguities.GetHead();
            speed = wvp->spd;
            printf("CTD %g Speed First Rank %g\n", ctd, speed);
            fflush(stdout);
        }
#endif
        num++;
        break;

    case S3:
        if (! gmf->RetrieveWinds_S3(meas_list, kp, wvc))
        {
            delete wvc;
            return(11);
        }
        break;

    case S3MV:
        if (! gmf->RetrieveWinds_S3MV(meas_list, kp, wvc))
        {
            delete wvc;
            return(11);
        }
        break;

    case S3RAIN:
        if (! gmf->RetrieveWinds_S3Rain(meas_list, kp, wvc))
        {
            delete wvc;
            return(11);
        }
        break;


    case CoastSpecial:
        if (! gmf->RetrieveWinds_CoastSpecial(meas_list, kp, wvc))
        {
            delete wvc;
            return(11);
        }
        break;

    case CoastSpecialGS:
      if (! gmf->RetrieveWinds_CoastSpecial(meas_list, kp, wvc,0,0))
        {
            delete wvc;
            return(11);
        }
        break;

    case S4:
        if (! gmf->RetrieveWinds_S3(meas_list, kp, wvc,1))
        {
            delete wvc;
            return(12);
        }
        break;

    case POLAR_SPECIAL:
        if (! gmf->RetrieveWinds_GS(meas_list, kp, wvc,1))
        {
            delete wvc;
            return(13);
        }
        ctd = (l2a->frame.cti - l2a->header.zeroIndex) *
            l2a->header.crossTrackResolution;
        wvp = NULL;
        if (wvc)
            wvp = wvc->ambiguities.GetHead();
        if (wvp)
        {
            speed = wvp->spd;
            dir = wvp->dir*rtd;
            printf("%g %g %g %d %d\n", ctd, speed, dir, l2a->frame.cti,
                l2a->frame.ati);
            fflush(stdout);
        }
        break;

    case HurrSp1:
      if (! HurrSp1Top(gmf,kp,meas_list, wvc))
        {
	  delete wvc;
	  return(15);
        }
        break;

    case CHEAT:
        if (! Cheat(meas_list, wvc))
        {
            return(14);
        }
        break;

    default:
        return(15);
    }

    if (wvc->ambiguities.NodeCount() == 0)
    {
        delete wvc;
        return(16);
    }
    wvc->lonLat = meas_list->AverageLonLat();

    // use weighted average to get position
    if(wrMethod == CoastSpecial){
      wvc->lonLat = meas_list->AverageLonLat(1);       
    }

    // set Artificial Neural Network outputs to zero
    float liquid_est=0;
    float ann_speed1=0;
    float ann_speed2=0;

    //----------------------------------------------------------
    // Code for ANN speed correction and rain flagging, for now the speed correction
    // only works when the rain flagging is on, but this need not be the case.
    // This was done so that a rain impact threshold could be used to determine when to
    // do the speed correction. We could ALWAYS do the speed correction.
    if(rainFlagMethod == ANNRainFlag1 || rainCorrectMethod == ANNSpeed1){
      
      if( rainFlagMethod == ANNRainFlag1 )   // init with rain flag not usable
        wvc->rainFlagBits = RAIN_FLAG_UNUSABLE;
      
      // compute liquid and ann_speed1 quantities
      ComputeMLPInputs(l2a,meas_list,wvc);
      
      // assign inputs to MLPs and check that all inputs are valid
      if(spdnet1_mlp.AssignInputs(MLP_inpt_array,MLP_valid_array)){

        // Estimate speed
        spdnet1_mlp.Forward();
        ann_speed1=spdnet1_mlp.outp[0];
        
        // Add speed estimate to MLP inputs array and mark it valid
        int idx=spdnet1_mlp.out_types[0].id;
        MLP_inpt_array[idx]=ann_speed1;
        MLP_valid_array[idx]=true;

        // This should never happen because liqnet1 inputs are the same and spdnet1 inputs
        // except for the spdnet1 output that was jsut computed and added to input array
        if(!liqnet1_mlp.AssignInputs(MLP_inpt_array,MLP_valid_array)){
          fprintf(stderr,"Liqnet1 inputs wer invalid although spdnet1 inputs were OK.\n");
          fprintf(stderr,"THIS SHOULD NEVER HAPPEN! Dying now.\n");
          exit(1);
        }
        
        // estimate liquid 
        liqnet1_mlp.Forward();
        liquid_est=liqnet1_mlp.outp[0];

        // Add liquid estimate to MLP inputs array and mark it valid
        idx=liqnet1_mlp.out_types[0].id;
        MLP_inpt_array[idx]=liquid_est;
        MLP_valid_array[idx]=true;


        //------ compute rain flag quantity if desired -//
        if(rainFlagMethod== ANNRainFlag1){

          if(rainflag_mlp.AssignInputs(MLP_inpt_array,MLP_valid_array) ){

            rainflag_mlp.Forward();
            wvc->rainImpact=rainflag_mlp.outp[0];

            // Set WVC flag value and bits
            if(wvc->rainImpact>rain_impact_thresh_for_flagging){
              // flag as rain & rain flag usable.
              wvc->rainFlagBits=RAIN_FLAG_RAIN; 
            }  
            else{
              // flag as no-rain & rain flag usable.
              wvc->rainFlagBits=0; 
            }

            //------ perform ann speed correction if desired -//
            if(rainCorrectMethod == ANNSpeed1 &&
               spdnet2_mlp.AssignInputs(MLP_inpt_array,MLP_valid_array) ){

              spdnet2_mlp.Forward();
              ann_speed2=spdnet2_mlp.outp[0];

              //------ remove residual speed bias -//
              float bias=-0.4967*ann_speed2-0.8227*log(cosh(0.5*(ann_speed2-15)))+5.7520;
              float spd=ann_speed2-bias;

              wvc->rainCorrectedSpeed=spd;

            }// end rainCorrectMethod==ANNSpeed1
	    else{
	      wvc->rainCorrectedSpeed=-1;
	    }

          } // end valid inputs to rainflag MLP case
            // Handle invalid inputs to rainflag case; flag as rain flag unusable.
          else{
	    wvc->rainFlagBits=RAIN_FLAG_UNUSABLE;
            wvc->rainCorrectedSpeed=-1;
            wvc->rainImpact=0;
          } 
        } // end if rainFlagMethod==ANNRainFlag1 case
      } // end of invalid inputs to liquid or speed1 networks

      // Handle invalid inputs to liquid or speed net1 case.
      else{
	wvc->rainFlagBits=RAIN_FLAG_UNUSABLE;
	wvc->rainCorrectedSpeed=-1;
	wvc->rainImpact=0;
      } 
    } // end of rainFlagMethod==ANNRainFlag1 || rainCorrectMethodANNSpeed1 case

    //-------------------------//
    // determine grid indicies //
    //-------------------------//

    int rev = (int)l2a->frame.rev;
    int cti = (int)l2a->frame.cti;
    int ati = (int)l2a->frame.ati;

    //------------------------------//
    // determine if rev is complete //
    //------------------------------//
    // this is some code that only thinks about doing rev splitting
    // since last_rev_number doesn't get incremented (yet), this
    // should do nothing.  the data will get filtered and flushed
    // once the l2a file is empty.

    if (rev != last_rev_number && last_rev_number)
        InitFilterAndFlush(l2b);    // process and write

    //-------------------//
    // add to wind swath //
    //-------------------//

    if (! l2b->frame.swath.Add(cti, ati, wvc))
        return(0);

    return(1);
}


void
L2AToL2B::RainCorrectSpeed(L2B* l2b){
  // Die if Rain Correction is not set up appropriately
  if(rainCorrectMethod!=ANNSpeed1){
    fprintf(stderr,"Error:L2AToL2B::RainCorrectSpeed called when rainCorrectMethod != ANNSpeed1\n");
    exit(1);
  }
  WindSwath* swath = &(l2b->frame.swath);
  int ncti=swath->GetCrossTrackBins();
  int nati=swath->GetAlongTrackBins();

  for (int cti = 0; cti < ncti; cti++)
    {
      for (int ati = 0; ati < nati; ati++)
        {
          WVC* wvc = swath->GetWVC(cti,ati);
          
          if ( ! wvc ) continue;
	  // modify all ambiguities when threshold exceeded
# define RC_AMBIGFIX_METHOD 2
          WindVectorPlus* wvp;
          int nbins=wvc->directionRanges.dirIdx.GetBins();
          float expected_speed;
          float* obj;
          float scale, sum=0; 

	  switch(RC_AMBIGFIX_METHOD){

          case 1:  // Replace all speeds with RainCorrectSpeed set objs values to zero
            if ( wvc->rainImpact <= rain_impact_thresh_for_correction || wvc->rainCorrectedSpeed<0) continue;
	    wvp=wvc->ambiguities.GetHead();
	    while(wvp){
	      wvp->spd=wvc->rainCorrectedSpeed;
	      wvp->obj=0;
	      wvp=wvc->ambiguities.GetNext();
	    }
              
	    //---------modify directionRanges speed array --------//
	    // set best speed and obj function curves to be flat vs azimuth.
	    
	    for(int c=0;c<nbins;c++){
	      wvc->directionRanges.bestSpd[c]=wvc->rainCorrectedSpeed;
	      wvc->directionRanges.bestObj[c]=0;
	    }
	    break;

	  case 2:  // Add bias E(spd)-rainCorrectedSpeed to all speeds; set obj values to zero

            float speedBias;
            // convert bestObjs to probabilities float sum = 0.0;
            obj=wvc->directionRanges.bestObj;
            scale = obj[0];
            for (int c = 1; c < nbins; c++)
              {
                if (scale < obj[c])
                  scale = obj[c];
              }
            sum=0;
            for (int c = 0; c < nbins; c++)
              {
                obj[c] = exp((obj[c] - scale) / 2);
                sum += obj[c];
              }
            for (int c = 0; c < nbins; c++)
              {
                obj[c] /= sum;
              }
            // compute expected speed and bias
            expected_speed=0;
            for (int c = 0; c < nbins; c++){
              expected_speed+= wvc->directionRanges.bestSpd[c]*obj[c];
            }
            speedBias=expected_speed-wvc->rainCorrectedSpeed;

           // put in extra rain impact dependent correction to bias
           speedBias+=3.9*exp(-(pow(fabs(wvc->rainImpact-5.5)/3,3)));

            wvc->qualFlag &= ~L2B_QUAL_FLAG_RAIN_CORR_APPL;
            if ( wvc->rainImpact <= rain_impact_thresh_for_correction || wvc->rainCorrectedSpeed<0) continue;
            wvc->qualFlag |=  L2B_QUAL_FLAG_RAIN_CORR_APPL;

            // unbias all speeds and set ambig obj values to zero
            for (int c = 0; c < nbins; c++){
    	      wvc->directionRanges.bestSpd[c]-=speedBias;
    	    }       

    	    wvp=wvc->ambiguities.GetHead();
    	    while(wvp){
    	      wvp->spd-=speedBias;
              // only set objective function to zero for high rain impact cases
    	      if(wvc->rainImpact>2.5) wvp->obj=0;
    	      wvp=wvc->ambiguities.GetNext();
    	    }     
            /* Only set the speed bias in the WVC if rain correction is applied */
            wvc->speedBias = speedBias;

            if(rain_speed_corr_thresh_for_flagging > 0) {
                if(wvc->rainImpact > rain_impact_thresh_for_flagging &&
                   fabs(wvc->speedBias) > rain_speed_corr_thresh_for_flagging) {
                    wvc->rainFlagBits = RAIN_FLAG_RAIN;
                } else {
                    wvc->rainFlagBits = 0;
                }
            }

	    break;
	  default:
	    fprintf(stderr,"L2AtoL2B::RainCorrectSpeed  Bag RC_AMBIGFIX_METHOD %d\n", RC_AMBIGFIX_METHOD);
	    exit(1);
	  } // end switch
	} // ati loop end
    } // cti loop end
}

//-----------------//
// L2AToL2B::Cheat //
//-----------------//

int
L2AToL2B::Cheat(
    MeasList*  meas_list,
    WVC*       wvc)
{
    wvc->lonLat = meas_list->AverageLonLat();
    WindVectorPlus* wvp = new WindVectorPlus;
    if (! nudgeField.InterpolatedWindVector(wvc->lonLat, wvp))
    {
        delete wvp;
        return(0);
    }
    wvp->obj = 0;
    wvc->ambiguities.Append(wvp);
    return(1);
}

//-----------------------//
// L2AToL2B:: HurrSp1Top //
//-----------------------//

int
L2AToL2B::HurrSp1Top(
    GMF* gmf,
    Kp* kp,
    MeasList*  meas_list,
    WVC*       wvc)
{
  // Set nudge vector to start out with
    wvc->lonLat = meas_list->AverageLonLat();
    WindVectorPlus* wvp = new WindVectorPlus;
    if (! nudgeField.InterpolatedWindVector(wvc->lonLat, wvp))
    {
      delete wvp;
        return(0);
    }
    wvp->obj = 0;
    wvc->nudgeWV=wvp;
 
    if(!gmf->RetrieveWinds_HurrSp1(meas_list,kp,wvc)) return(0);
    return(1);
}

//-------------------------//
// L2AToL2B::InitAndFilter //
//-------------------------//

#define ONE_STAGE_WITHOUT_RANGES  1
#define S3_WINDOW_SIZE            medianFilterWindowSize
#define S3_NUDGE                  0



int 
L2AToL2B::PopulateOneNudgeVector(L2B* l2b,
			       int cti,
				 int ati,
			       MeasList* ml){
  WVC* wvc=l2b->frame.swath.swath[cti][ati];
  if(!wvc) return(0);

  wvc->nudgeWV=new WindVectorPlus;
  wvc->lonLat=ml->AverageLonLat();
  //-----------------------------------------------------//
  // Copy Interpolated NudgeVectors to Wind Vector Cells //
  //-----------------------------------------------------//
    
  if (useNudging && ! l2b->frame.swath.nudgeVectorsRead && !smartNudgeFlag &&!arrayNudgeFlag)
    {
      if(!nudgeField.InterpolatedWindVector(wvc->lonLat,wvc->nudgeWV)){
	delete wvc->nudgeWV;
        wvc->nudgeWV=NULL;
        return(0);
      }
    }
  else if(useNudging && ! l2b->frame.swath.nudgeVectorsRead && smartNudgeFlag){
    if(!nudgeVctrField.InterpolateVectorField(wvc->lonLat,wvc->nudgeWV,0)){
	delete wvc->nudgeWV;
        wvc->nudgeWV=NULL;
        return(0);
      }
  }
  else if(useNudging && ! l2b->frame.swath.nudgeVectorsRead && arrayNudgeFlag){
    wvc->nudgeWV->spd=arrayNudgeSpd[ati][cti];
    wvc->nudgeWV->dir=arrayNudgeDir[ati][cti];
    if(wvc->nudgeWV->spd ==0 && wvc->nudgeWV->dir==0){
	delete wvc->nudgeWV;
        wvc->nudgeWV=NULL;
        return(0);
    }
  }
  if (useHurricaneNudgeField)
    {
	delete wvc->nudgeWV;
        wvc->nudgeWV=NULL;
        return(0);
    }
  return(1);
}
int 
L2AToL2B::PopulateNudgeVectors(
    L2B*  l2b)
{
   //-----------------------------------------------------//
    // Copy Interpolated NudgeVectors to Wind Vector Cells //
    //-----------------------------------------------------//
    
    if (useNudging && ! l2b->frame.swath.nudgeVectorsRead && !smartNudgeFlag &&!arrayNudgeFlag)
    {
        l2b->frame.swath.GetNudgeVectors(&nudgeField);
    }
    else if(useNudging && ! l2b->frame.swath.nudgeVectorsRead && smartNudgeFlag){
      l2b->frame.swath.GetNudgeVectors(&nudgeVctrField);
    }
    else if(useNudging && ! l2b->frame.swath.nudgeVectorsRead && arrayNudgeFlag){
      l2b->frame.swath.GetNudgeVectors(arrayNudgeSpd,arrayNudgeDir,arrayNudgeNati,arrayNudgeNcti);
    }
    if (useHurricaneNudgeField)
    {
        l2b->frame.swath.GetHurricaneNudgeVectors(&hurricaneField,
            &hurricaneCenter, hurricaneRadius);
    }
  return(1);
}
int
L2AToL2B::InitAndFilter(
    L2B*  l2b)
{
 
    PopulateNudgeVectors(l2b);


    //------ perform ann speed correction if desired -//
    // This routine populates ambiguities using already computed
    // ANN speeds for rainy conditions
    if(rainCorrectMethod == ANNSpeed1 && medianFilterMaxPasses>0) RainCorrectSpeed(l2b);

    //------------//
    // initialize //
    //------------//

    #define ALREADY_INITD    0
    #define USEBESTK         0
    #define BESTKPARAMETER   1000
    #define BESTKWINDOWSIZE  15
    #define USE4PASS         1

    if (ALREADY_INITD)
    {
        // Do Nothing
    }
    else if (USEBESTK)
    {
        l2b->frame.swath.BestKFilter(BESTKWINDOWSIZE, BESTKPARAMETER);
    }
    else if (useNudging)
    {
        
        if (S3_NUDGE)
            l2b->frame.swath.S3Nudge();
        else if (useNudgeThreshold)
        {
            printf("useNudgeThreshold == 1\n");
            l2b->frame.swath.ThresNudge(maxRankForNudging, nudgeThresholds);
	    }
	    else if (useNudgeStream){
	        printf("L2AToL2B::StreamT %g\n",streamThreshold);
	        l2b->frame.swath.StreamNudge(streamThreshold);
	    }
        else
        {   
            l2b->frame.swath.Nudge(maxRankForNudging);
        }
    }
    else if(useRandomInit)
    {
        l2b->frame.swath.InitRandom();
    }
    else
    {
        l2b->frame.swath.InitWithRank(1);
    }

    #define HURRICANE_MAX_RANK 4

    if (useHurricaneNudgeField)
    {
        l2b->frame.swath.HurricaneNudge(HURRICANE_MAX_RANK, &hurricaneCenter,
            hurricaneRadius);
    }

    //---------------//
    // median filter //
    //---------------//

    int bound;
    int special = 0;

    switch(wrMethod)
    {
    case S3:
    case S3RAIN:
    case CoastSpecial:
        special=1;
        break;
    case S4:
        special=2;
        break;
    case HurrSp1:
      //medianFilterMaxPasses=0;
	special=1;
	break;
    default:
        special=0;
        break;
    }

    int special_first_pass = special;
    if (special == 1 && ONE_STAGE_WITHOUT_RANGES)
        special_first_pass = 0;
    int freeze=0;
    if (useNMF)
    {
        bound = 9;
        l2b->frame.swath.MedianFilter(medianFilterWindowSize,
            medianFilterMaxPasses, bound, useAmbiguityWeights,
            special_first_pass);
    }
    bound = 0;
    if (useNMF) freeze=9;
    else
    {
        if( USE4PASS )
        {
          special_first_pass = 0;  // No DIRTH for the 4-Pass stage of Amb selection.
          l2b->frame.swath.MedianFilter_4Pass(medianFilterWindowSize,
              medianFilterMaxPasses, bound, useAmbiguityWeights, special_first_pass, freeze);
        }
        else 
        {
          l2b->frame.swath.MedianFilter(medianFilterWindowSize,
              medianFilterMaxPasses, bound, useAmbiguityWeights, special_first_pass,freeze);
        }
        
        if (special == 1 && ONE_STAGE_WITHOUT_RANGES)
        {
            if (medianFilterMaxPasses > 0)
            {
                l2b->frame.swath.DiscardUnselectedRanges();
            }
            l2b->frame.swath.MedianFilter(S3_WINDOW_SIZE, medianFilterMaxPasses,
                bound, useAmbiguityWeights,special);
        }
    }
    return(1);
}

//------------------------------//
// L2AToL2B::InitFilterAndFlush //
//------------------------------//

int
L2AToL2B::InitFilterAndFlush(
    L2B*  l2b)
{
    if (! InitAndFilter(l2b))
        return(0);

    //------------//
    // output l2b //
    //------------//

    if (! l2b->WriteHeader())
        return(0);
    if (! l2b->WriteDataRec())
        return(0);
    // l2b->frame.swath.DeleteWVCs();

    return(1);
}

//-------------------------------//
// L2AToL2B::WriteSolutionCurves //
//-------------------------------//

int
L2AToL2B::WriteSolutionCurves(
    L2A*         l2a,
    GMF*         gmf,
    Kp*          kp,
    const char*  output_file)
{
    //------------------//
    // open output file //
    //------------------//

    FILE* ofp = fopen(output_file, "w");
    if (ofp == NULL)
        return(0);

    //-----------------------//
    // write solution curves //
    //-----------------------//

    gmf->WriteSolutionCurves(ofp, &(l2a->frame.measList), kp);

    //-------------------//
    // close output file //
    //-------------------//

    fclose(ofp);

    return(1);
}

void
	L2AToL2B::ComputeMLPInputs(L2A* l2a, MeasList* meas_list, WVC* wvc){
  // to simplify my life and speed up the code
  // I am just hardcoding the numbers from the MLP_IO_TYPE table
  // in MLP.C rather than doing a bunch of string operations
  // -- BWS June 23 2010

  // hardcoded indicies
  int firstrank_spd_idx=52;
  int mean_idx[]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
  int var_idx[]={17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32};
  int ctd_idx=33;
  int ctd_frac_idx=34;

  // set First Rank wind speed
  if(wvc){
    WindVectorPlus* wvp=wvc->ambiguities.GetHead();
    if(wvp){
      MLP_inpt_array[firstrank_spd_idx]=wvp->spd;
      MLP_valid_array[firstrank_spd_idx]=true;
    }
  }

  // set Cross Track distance values

  MLP_inpt_array[ctd_frac_idx] = ((float)l2a->frame.cti - ((float)l2a->header.crossTrackBins /2.0))
                    / ((float)l2a->header.crossTrackBins /2.0);
  MLP_valid_array[ctd_frac_idx] = true;
  MLP_inpt_array[ctd_idx]=l2a->getCrossTrackDistance();
  MLP_valid_array[ctd_idx] = true;

  // set sigma0 mean and variance values
  // array list includes all possible polarization, incidence angle, frequency, 
  // and look angle combinations, but only half of these are currently implemented in the
  // measurement class. Inner beam Ku VV and Outer beam Ku HH are unlikely to ever be used for
  // consistency of climate record reasons, and currently we implement C band outer HH and
  // C band inner VV by changing the GMF. As far as the code knows the inner beam is always HH
  // and the outer beam is always VV.
  float s0_mean[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  float s0_var[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  int s0_n[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  
  // accumulate the sum of each type of measurement, for calculating the mean
  for (Meas* meas = meas_list->GetHead(); meas; meas = meas_list->GetNext())
    {
      int i=-1;
      int aftfore= !(meas->scanAngle < pi / 2 || meas->scanAngle > 3 * pi / 2);
      switch (meas->measType){
      case Meas::HH_MEAS_TYPE:
        i=0+aftfore;
	break;
      case Meas::VV_MEAS_TYPE:
        i=6+aftfore;
	break;
      case Meas::C_BAND_HH_MEAS_TYPE:
        i=8+aftfore;
	break;
      case Meas::C_BAND_VV_MEAS_TYPE:
        i=14+aftfore;
	break;
      default:
	break;
      }
      if(i!=-1){
	s0_n[i]++;
        s0_mean[i]+=meas->value;
        s0_var[i]+=meas->value*meas->value;
      }
    }

  for(int c=0;c<16;c++){
    // populate valid mean arrays
    if(s0_n[c]>0){
      MLP_inpt_array[mean_idx[c]]=s0_mean[c]/s0_n[c];
      MLP_valid_array[mean_idx[c]]=true;
      
      if( use_MLP_mapping && MLP_input_map_ns0>0 && MLP_valid_array[mean_idx[c]] ) {
        float value = 10*log10(MLP_inpt_array[mean_idx[c]]);
        int idx     = floor((value-MLP_input_map_s0min)/MLP_input_map_ds0 + 0.5);
        if( idx >= 0 && idx < MLP_input_map_ns0 ) {
          MLP_inpt_array[mean_idx[c]] = pow(10.0,MLP_input_map_s0[c*MLP_input_map_ns0+idx]/10.0);          
        }  
      }
    }
    // populate valid variance arrays
    if(s0_n[c]>1){
      MLP_inpt_array[var_idx[c]]=(s0_var[c]-(s0_mean[c]*s0_mean[c])/s0_n[c])/(s0_n[c]-1);
      MLP_valid_array[var_idx[c]]=true;
      
      if( use_MLP_mapping && MLP_input_map_nvars0>0 && MLP_valid_array[var_idx[c]] ) {
        float value = MLP_inpt_array[var_idx[c]];
        int idx     = floor((value-MLP_input_map_vars0min)/MLP_input_map_dvars0 + 0.5);
        MLP_inpt_array[var_idx[c]] = MLP_input_map_vars0[c*MLP_input_map_nvars0+idx];
      }
      
    }
  }
}

void L2AToL2B::ReadMLPMapping( char* filename ) {
  use_MLP_mapping      = false;
    
  FILE* ifp = fopen(filename,"r");
  if( !ifp ) 
    return;
  
  fread( &MLP_input_map_s0min,    sizeof(float), 1, ifp );
  fread( &MLP_input_map_ds0,      sizeof(float), 1, ifp );
  fread( &MLP_input_map_ns0,      sizeof(int),   1, ifp );
  fread( &MLP_input_map_vars0min, sizeof(float), 1, ifp );
  fread( &MLP_input_map_dvars0,   sizeof(float), 1, ifp );
  fread( &MLP_input_map_nvars0,   sizeof(int),   1, ifp );

  if( MLP_input_map_ns0 > 0 ) {
    MLP_input_map_s0 = new float[16*MLP_input_map_ns0];    
    fread( &MLP_input_map_s0[0], sizeof(float), MLP_input_map_ns0*16, ifp );
  }
  if( MLP_input_map_nvars0 > 0 ) {
    MLP_input_map_vars0 = new float[16*MLP_input_map_nvars0];    
    fread( &MLP_input_map_vars0[0], sizeof(float), MLP_input_map_nvars0*16, ifp );
  }
  fclose(ifp);
  use_MLP_mapping = true;
  
  fprintf(stdout,"s0:    min, step, ns0: %f %f %d\n",
    MLP_input_map_s0min,MLP_input_map_ds0,MLP_input_map_ns0);
  fprintf(stdout,"s0var: min, step, ns0: %f %f %d\n",
    MLP_input_map_vars0min,MLP_input_map_dvars0,MLP_input_map_nvars0);
  
  return;
}


