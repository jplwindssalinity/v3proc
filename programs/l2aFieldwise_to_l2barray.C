//==============================================================//
// Copyright (C) 1997-2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    l2aFieldwise_to_l2barray.C
//
// SYNOPSIS
//    l2aFieldwise_to_l2barray <input l2a indicies file (new format for field-wise retrieval) > <output l2b arrays file>
//
// DESCRIPTION
//   Executes field-wise ML (or MAP) wind retrieval on grid defined by l2a indices file the output is a 
//   single wind field in the same format as written by l2b_to_arrays.  The method initializes with the 
//   nudge field and then does a gradient search to find the optimum of the field-wise ML (or MAP) 
//   objective function. 
//    
//
// OPTIONS
//   
//
// OPERANDS
//   
//
// EXAMPLES
//    An example of a command line is:
//      % l2aFieldwise_to_l2barray l2a_indices.dat l2b_arrays
//
// ENVIRONMENT
//    Not environment dependent.
//
// EXIT STATUS
//    The following exit values are returned:
//       0  Program executed successfully
//      >0  Program had an error
//
// NOTES
//    None.
//
// AUTHORS
//    Brent Williams (Brent.A.Williams@jpl.nasa.gov)
//----------------------------------------------------------------------

//-----------------------//
// Configuration Control //
//-----------------------//
static const char rcs_id[] =
    "@(#) $Id$";

//----------//
// INCLUDES //
//----------//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <cstdlib>
//#include <malloc.h>
//#include "Array.h"

#include "List.h"
#include "BufferedList.h"
#include "Misc.h"
#include "ConfigList.h"
#include "L2A.h"
#include "ConfigSim.h"
#include "L2B.h"
#include "L2AToL2B.h"
#include "Tracking.h"
#include "Array.h"
#include "Meas.h"

//#include "fft2d.C"
//#include "fft.C"
//#include "ifft.C"
//#include "cplx.h"
#include <complex>
#include <fftw3.h>

using std::list;
using std::map; 

//-----------//
// TEMPLATES //
//-----------//

// Class declarations needed for templates
// eliminates need to include the entire header file

class AngleInterval;

template class List<StringPair>;
template class List<Meas>;
template class List<EarthPosition>;
template class List<WindVectorPlus>;
template class List<MeasSpot>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<off_t>;
template class List<OffsetList>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;
template class List<AngleInterval>;
template class std::list<string>;
template class std::map<string,string,Options::ltstr>;

//-----------//
// CONSTANTS //
//-----------//
/*
#define MAX_ALONG_TRACK_BINS  1624
#define OPTSTRING "iRt:a:n:w:"
*/
//-------//
// HACKS //
//-------//

//#define LATLON_LIMIT_HACK

//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

//const char* usage_array[] = {"[ -a start:end ]", "[ -n num_frames ]", "[ -i ]", 
//    "[ -w c_band_weight_file ]", "[ -R ]", "[ -t output_train_set_fn ]", "<sim_config_file>", 0};
const char* usage_array[] = {"<l2a indices infile>","<l2b_arrays outfile>","<l2b_arrays nudge input file>","<config_file>", 0};


//--------------//
// MAIN PROGRAM //
//--------------//

int
main(
    int    argc,
    char*  argv[])
{
  char*  inputFilename;
  char*  outputFilename;
  char*  outputFilenameAVE;
  char  outputFilenameAVE_reconst[200];
  char  outputFilenameAVE_grad[200];
  char*  nudgeFilename;
  char tmpstr[200];
  const char* config_file;
  FILE*  inputFp;
  //FILE*  outputFp;

  int maxiter=50;

  float dtr=3.14159/180.0;
  float rtd=180.0/3.14159;
  

  float dmeas_time;
  int dbeamIdx;
  int dstartSliceIdx;
  float dvalue;
  float deastAzimuth;
  float dincidenceAngle;
  float dA;
  float dB;
  float dC;
  float datd;
  float dctd;
  int dvati0;
  int dcti0;
  int dnumWVCs;

  int len;
  int dati;
  int dcti;
  float dap;
 

  int windPrior=5;//5=gaussian prior on U V components with k^-2 spectrum.  -1=sigma^0 prior

  float atres,ctres;//alongtrack and cross track resolution
  const char* line;
  //read in command line arguments
  if (argc<5)
    {
      fprintf(stderr,"Usage: l2aFieldwise_to_l2barray %s %s %s %s\n",usage_array[0],usage_array[1],usage_array[2],usage_array[3]);
      return(1);
    }else
    {
      inputFilename=argv[1];
      outputFilename=argv[2];
      nudgeFilename=argv[3];
      config_file=argv[4];
      //const char* config_file = argv[optind++];

      printf("inputFilename: %s, outputFilename: %s, nudgeFilename: %s config_file: %s \n",inputFilename, outputFilename,nudgeFilename,config_file);
      if (argc>5)
	{
	  sscanf(argv[5],"%d",&maxiter);
	  printf("Max Iterations: %d\n",maxiter);
	}
      if (argc>6)
	{
	  outputFilenameAVE=argv[6];
	  printf("AVE Output File Name: %s\n",outputFilenameAVE);
	}

      if (argc>7)
	{
	  sscanf(argv[7],"%d",&windPrior);
	  printf("Prior Type: %d\n",windPrior);
	}
    }


   
   

  //---------------------//
  // read in config file //
  //---------------------//

  ConfigList config_list;
  if (! config_list.Read(config_file))
    {
      fprintf(stderr, " error reading sim config file %s\n",config_file);
      exit(1);
    }
  
  
  //set along and cross track res from config
  line="CROSSTRACK_RESOLUTION";
  config_list.GetFloat(line,&ctres);
  line="ALONGTRACK_RESOLUTION";
  config_list.GetFloat(line,&atres);
  
  printf("ctres: %f, atres %f\n",ctres,atres);

  //-----------------------//
  // read nudge array file //
  //-----------------------//
  
  FILE* wfp=fopen(nudgeFilename,"r");
  if(!wfp){
    fprintf(stderr,"Cannot open file %s for reading\n",nudgeFilename);
    exit(1);
  }
  int first_valid_ati;
  int nvalid_ati;
  int ncti;
  
  if (fread(&first_valid_ati,sizeof(int),1,wfp)!=1||
      fread(&nvalid_ati,sizeof(int),1,wfp)!=1  ||
      fread(&ncti,sizeof(int),1,wfp)!=1)
    {
      fprintf(stderr,"Error reading nudge file %s\n",nudgeFilename);
      exit(1);
    }
 
  int f=first_valid_ati;
  int n=nvalid_ati;
  int nati=n+f+10;
  printf("f: %d n: %d ncti: %d nati: %d\n",f,n,ncti,nati);

  // make arrays
  
  float ** spd=(float**)make_array(sizeof(float),2,nati,ncti);
  float ** dir=(float**)make_array(sizeof(float),2,nati,ncti);
  float ** lat=(float**)make_array(sizeof(float),2,nati,ncti);
  float ** lon=(float**)make_array(sizeof(float),2,nati,ncti);
  
  char ** flg=(char**)make_array(sizeof(char),2,nati,ncti);
  int ** numMeas=(int**)make_array(sizeof(int),2,nati,ncti);

  //read arrays
  
  if(!read_array(wfp,&spd[f],sizeof(float),2,n,ncti) ||
     !read_array(wfp,&dir[f],sizeof(float),2,n,ncti) ||
     !read_array(wfp,&lat[f],sizeof(float),2,n,ncti) ||
     !read_array(wfp,&lon[f],sizeof(float),2,n,ncti) ||
     !read_array(wfp,&flg[f],sizeof(char),2,n,ncti)){
    fprintf(stderr,"Error reading to file %s\n",nudgeFilename);
    exit(1);
  }
 
  fclose(wfp);
  


  
  for (int ia=0;ia<nati;ia++)
    {
      for (int ic=0;ic<ncti;ic++)
	{
	  //initialize speed and dir
	  //hard coded for testing should comment out next two lones to initialize with input feld
	  //spd[ia][ic]=0.0;
	  //dir[ia][ic]=160.0*dtr;
	  
	  dir[ia][ic]=dir[ia][ic]*rtd;//convert to degrees
	  while(dir[ia][ic]>=360.0) dir[ia][ic]-=360.0;
	  while(dir[ia][ic]<0.0) dir[ia][ic]+=360.0;
	  
	  numMeas[ia][ic]=0;
	  if (lat[ia][ic]==0&&lon[ia][ic]==0){
	    flg[ia][ic]='6';
	  }
	  if (spd[ia][ic]==-1){
	    flg[ia][ic]='5';
	    spd[ia][ic]=0;
	  }
	  
	}
    } 
  






// open l2a indices input file
  if (inputFilename == NULL){
    fprintf(stderr,"No Input File");
    return(1);
  }
  inputFp = fopen(inputFilename, "r");
  if (inputFp == NULL){
    fprintf(stderr,"Error opening input file %s for reading \n",inputFilename);
    return(1);
  }

  //----------------//
  //read input file //
  //----------------//

  //first scan through file and get max dimentions for arrays then read through and fill arrays
  // the file format of the input indices file should probably should be changed so that we dont have to 
  // scan throught the file twice 

  int nmeas=0;
  int atmax=0;
  int atmin=100000;
  int ctmax=0;
  int ctmin=100000;
  int maxnumWVCs=0;
  for(;;)
    {
      fscanf(inputFp,"%*d %*g %*d %*d %*f %*f %*f %*f %*f %*f %*g %*g %d %d %d\n",&dvati0,&dcti0,&dnumWVCs);
      if (feof(inputFp)) break;
      if (dnumWVCs>maxnumWVCs) maxnumWVCs++;
      if (dcti0>ctmax) ctmax++;
      if (dvati0>atmax) atmax++;
      if (dcti0<ctmin) ctmin--;
      if (dvati0<atmin) atmin--;
      // scan through each wvc hit by each measurement
      for (int iwvcs=0;iwvcs<dnumWVCs;iwvcs++)
	{
	  fscanf(inputFp,"%d %d %*f\n",&dati,&dcti);

	  if (dcti>ctmax) ctmax++;
	  if (dati>atmax) atmax++;
	  if (dcti<ctmin) ctmin--;
	  if (dati<atmin) atmin--;
	  
	}
      nmeas++;
    }

 maxnumWVCs++;//add one to account for the centriod bin at index 0

  printf("nmeas: %d ctmin: %d ctmax: %d atmin: %d atmax: %d maxnumWVCs: %d\n",nmeas,ctmin,ctmax,atmin,atmax,maxnumWVCs);
     
 
 
  //make measurement arrays
  float * value=(float*)make_array(sizeof(float),1,nmeas);
  float * inc=(float*)make_array(sizeof(float),1,nmeas);
  float * az=(float*)make_array(sizeof(float),1,nmeas);
  float * A=(float*)make_array(sizeof(float),1,nmeas);
  float * B=(float*)make_array(sizeof(float),1,nmeas);
  float * C=(float*)make_array(sizeof(float),1,nmeas);
  int * numWVCs=(int*)make_array(sizeof(int),1,nmeas);
  int ** ctis=(int**)make_array(sizeof(int),2,nmeas,maxnumWVCs);
  int ** atis=(int**)make_array(sizeof(int),2,nmeas,maxnumWVCs);
  float ** ap=(float**)make_array(sizeof(float),2,nmeas,maxnumWVCs);

  int dnumWVCs2;

  //rewind input file and fill the arrays
  fseek(inputFp,0,SEEK_SET);
  int imeas=0;
  for(;;)
    {
     
      fscanf(inputFp,"%d %g %d %d %f %f %f %f %f %f %g %g %d %d %d\n",&len,&dmeas_time,&dbeamIdx,&dstartSliceIdx,&dvalue,&deastAzimuth,&dincidenceAngle,&dA,&dB,&dC,&datd,&dctd,&dvati0,&dcti0,&dnumWVCs);
      
      if (feof(inputFp)) break;
     
      value[imeas]=dvalue;
      az[imeas]=deastAzimuth;
      inc[imeas]=dincidenceAngle;
      A[imeas]=dA;
      B[imeas]=dB;
      C[imeas]=dC;
      atis[imeas][dnumWVCs+1]=dvati0;//put centroid bin at end of array (the WVC is already in the middle of array array)
      ctis[imeas][dnumWVCs+1]=dcti0;
      numWVCs[imeas]=dnumWVCs;

      //fscanf(inputFp,"%*d %*g %*d %*d %*f %*f %*f %*f %*f %*f %*g %*g %d %d %d\n",&dvati0,&dcti0,&dnumWVCs);
     
      

      dnumWVCs2=0;
      // scan through each wvc hit by each measurement
      for (int iwvcs=0;iwvcs<dnumWVCs;iwvcs++)
	{
	  //fscanf(inputFp,"%d %d %f\n",&atis[imeas][iwvcs+1],&ctis[imeas][iwvcs+1],&ap[imeas][iwvcs+1]);
	  fscanf(inputFp,"%d %d %f\n",&dati,&dcti,&dap);
	  atis[imeas][iwvcs]=dati;
	  ctis[imeas][iwvcs]=dcti;
	  //ap[imeas][iwvcs]=dap;
	  
	  ap[imeas][iwvcs]=1.0;///float(dnumWVCs);//for now, force constant aperture function that sums to 1
	  //printf("%f %f\n",ap[imeas][iwvcs],1.0/float(dnumWVCs));
	  //fprintf(stderr,"%d %d %f\n",dcti,dati,dap);
	  if (1==1)// (spd[atis[imeas][iwvcs]][ctis[imeas][iwvcs]]!=-1) 
	    {
	      dnumWVCs2++;//dont sample non-data
	      
	    }else 
	    {
	      ap[imeas][iwvcs]=0.0;
	    }
	  numMeas[dati][dcti]++;
	}
      if (dnumWVCs2>0){
	for (int iwvcs=0;iwvcs<dnumWVCs;iwvcs++) {
	  ap[imeas][iwvcs]/=float(dnumWVCs2);
	}
      }
      
      imeas++;
    }
  
  fclose(inputFp);
  
  printf("DONE READING ARRAYS\n");
  
  
  //-------------------------------------//
  // read the geophysical model function //
  //-------------------------------------//
  
  GMF gmf;
  if (! ConfigGMF(&gmf, &config_list))
    {
      fprintf(stderr, " error configuring GMF\n");
      exit(1);
    }
  
  printf("DONE READING GMF\n");



  //-------------------//
  // do the processing //
  //-------------------//
  float sigwvc,sig;
  float sigL,sigW,apL,apW,val,PL,PW;
  float R,alpha,beta,gamma;
  double Kobj,gradprior1,gradprior2;
  double gradpriorV,gradpriorH;
  /* 
  float ** gradobj1=(float**)make_array(sizeof(float),2,nati,ncti);
  float ** gradobj2=(float**)make_array(sizeof(float),2,nati,ncti);
  float ** gradobjV=(float**)make_array(sizeof(float),2,nati,ncti);
  float ** gradobjH=(float**)make_array(sizeof(float),2,nati,ncti);
  */
  double ** gradobj1=(double**)make_array(sizeof(double),2,nati,ncti);
  double ** gradobj2=(double**)make_array(sizeof(double),2,nati,ncti);
  double ** gradobjV=(double**)make_array(sizeof(double),2,nati,ncti);
  double ** gradobjH=(double**)make_array(sizeof(double),2,nati,ncti);

  int mxmeas=nmeas;
  double gradgmf1,gradgmf2,mxgradobj1,mxgradobj2,mxgradobj,mxgradobjV,mxgradobjH;
  double muS,muD;
  double muD0=3.0;//30.0;
  double muS0=1.0;//5.0;//increase if doing fieldwise search
  double muH0=0.01;//0.01;for land h-pol
  double muV0=0.01;//for land v-pol
  double muH,muV;
  int SDflag=1;//1 for searching objectiove function in Speed and Dir, 0 for U and V
  if (windPrior==5){SDflag=0;}
  if (SDflag==0) {
    muD0=0.5;//1.0//decrease if doing point-wise
    muS0=0.5;//1.0
  }
  int Filtflg=1;
  
  if (windPrior==5){Filtflg=0;}
  //int windPrior=5;//5=gaussian prior on U V components with k^-2 spectrum.  -1=sigma^0 prior
  int flg2;
  int muFlg=1;//switches how to do variable mu search 0=field-wise scaling 1=point-wise scaling
  // if doing a field-wise prior in wind space variable mu must also be field-wise (=0)
  if (windPrior==5){muFlg=0;}
  int noifft=0;//flag for doing fake inverse of covariance (useful for nonstationary covariance)
 
  float sgn=0.0;

  Meas::MeasTypeE meast;
  float speed,chi,dirt;
  
  float sigp=0;
  float sigm=0;
  float ds=0.05;
  double mxcngspeed,mncngspeed,mxcngdir,mncngdir;
  double mxcngH,mncngH,mxcngV,mncngV;

  //float ** spd2=(float**)make_array(sizeof(float),2,nati,ncti);
  //float ** dir2=(float**)make_array(sizeof(float),2,nati,ncti);
  //printf("here nati: %d, ncti %d\n",nati,ncti);

  //These next few variables are for the prior that shapes the spectrum
  float * cova=(float*)make_array(sizeof(float),1,nati);
  float * covc=(float*)make_array(sizeof(float),1,ncti);
  int ix;
  float pi=3.14159;
  float kbl=2.0*pi;//
  float k0,k0a,k0c;//correlation lengths (at and ct) in prior

  k0a=25.0/(float(nati)*atres);
  k0c=25.0/(float(ncti)*ctres);
  //printf("nati: %d, ncti %d\n",nati,ncti);
  
  //For Fisher Information Matrix 
  
  int nati2=1;//nati;//set these to nati and ncti if WindPrior==4
  int ncti2=1;//ncti;
  float ** J_uu=(float**)make_array(sizeof(float),2,nati2,ncti2);
  float ** J_vv=(float**)make_array(sizeof(float),2,nati2,ncti2);
  float ** J_uv=(float**)make_array(sizeof(float),2,nati2,ncti2);
  float ** J_vu=(float**)make_array(sizeof(float),2,nati2,ncti2);
  
  
  //for fft implemented prior
  /*
  int nmx=max(nati-f,ncti);
  printf("nati: %d ncti: %d nmx: %d\n",nati,ncti,nmx);
  float grad1U, grad1V,grad2U,grad2V;
  //nmx=500;  //complex U [nmx][nmx];
  //complex V [nmx][nmx];
  complex ** U=(complex**)make_array(sizeof(complex),2,nmx,nmx);
  complex ** V=(complex**)make_array(sizeof(complex),2,nmx,nmx);
  */
  double magU,magV,grad1U,grad1V,grad2U,grad2V,magC,magC0;
  //use fftw libraries
  fftw_complex *outU, *outV, *outcorr;
  //double inU[nati][ncti], inV[nati][ncti],  incorr[nati][ncti];
  //double *inU, *inV,  *incorr;
  fftw_complex *inU, *inV,  *incorr;
  fftw_plan plnU,plnUI,plnV,plnVI,plnC;
   // assumed signal correlation fucntion. 
  //float ** corr=(float**)make_array(sizeof(float),2,nati,ncti);
  int natif=2*nati;
  int nctif=2*ncti;
  //inU = (double*) malloc(sizeof(double) * ncti*nati);
  inU = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * nctif*natif);
  outU = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * nctif*natif);
  //incorr = (double*) malloc(sizeof(double) * ncti*nati);
  incorr = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * nctif*natif);
  outcorr = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * nctif*natif);
 
  //inV = (double*) malloc(sizeof(double) * ncti*nati); 
  inV = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * nctif*natif);
  outV = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * nctif*natif);
  //p = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
  
  //pln = fftw_plan fftw_plan_dft_2d(int n0, int n1,fftw_complex *in, fftw_complex *out,int sign, unsigned flags);
  /*plnU = fftw_plan_dft_r2c_2d(nati, ncti,inU, outU,FFTW_FORWARD, FFTW_ESTIMATE);
  plnUI = fftw_plan_dft_c2r_2d(nati, ncti,outU, inU,-1, FFTW_ESTIMATE);
  plnC = fftw_plan_dft_r2c_2d(nati, ncti,incorr, outcorr,FFTW_FORWARD, FFTW_ESTIMATE);
  plnV = fftw_plan_dft_r2c_2d(nati, ncti,inV, outV,FFTW_FORWARD, FFTW_ESTIMATE);
  plnVI = fftw_plan_dft_c2r_2d(nati, ncti,outV, inV,-1, FFTW_ESTIMATE);
  
  plnU = fftw_plan_dft_r2c_2d(nati, ncti,inU, outU, FFTW_ESTIMATE);
  plnUI = fftw_plan_dft_c2r_2d(nati, ncti,outU, inU, FFTW_ESTIMATE);
  plnC = fftw_plan_dft_r2c_2d(nati, ncti,incorr, outcorr, FFTW_ESTIMATE);
  plnV = fftw_plan_dft_r2c_2d(nati, ncti,inV, outV, FFTW_ESTIMATE);
  plnVI = fftw_plan_dft_c2r_2d(nati, ncti,outV, inV, FFTW_ESTIMATE);
  */
  plnU = fftw_plan_dft_2d(natif, nctif,inU, outU,FFTW_FORWARD, FFTW_ESTIMATE);
  plnUI = fftw_plan_dft_2d(natif, nctif,outU, inU,FFTW_BACKWARD, FFTW_ESTIMATE);
  plnC = fftw_plan_dft_2d(natif, nctif,incorr, outcorr,FFTW_FORWARD, FFTW_ESTIMATE);
  plnV = fftw_plan_dft_2d(natif, nctif,inV, outV, FFTW_FORWARD,FFTW_ESTIMATE);
  plnVI = fftw_plan_dft_2d(natif, nctif,outV, inV,FFTW_BACKWARD, FFTW_ESTIMATE);
 


  //fftw_execute(pln); /* repeat as needed */
  


  printf("got here\n");

  if (windPrior==1){
  for (int i=0;i<nati-1;i++)
    { ix=i;
      if (ix>nati/2) ix=ix-nati;
      k0=k0a;
      if (ix==0)
	{
	  cova[i]=kbl*k0/(2.0*pi)+kbl*kbl*kbl/(6.0*pi*k0);
	}else
	{ //note this is the deconvolution function for an exponentially correlated signal
	  //this is used to produce the inverse of the covariance matrix of the prior 
	  //whose spectrum falls off as k^-2 after k0
	  cova[i]=(k0*ix*ix*sin(kbl*ix)+2*kbl*ix*cos(kbl*ix)+(kbl*kbl*ix*ix-2)*sin(kbl*ix))/(2*pi*k0*ix*ix*ix);
	}
    }
  for (int i=0;i<ncti-1;i++)
    {ix=i;
      if (ix>ncti/2) ix=ix-ncti;
      k0=k0c;
      if (ix==0)
	{
	  covc[i]=kbl*k0/(2.0*pi)+kbl*kbl*kbl/(6.0*pi*k0);
	}else
	{
	  covc[i]=(k0*ix*ix*sin(kbl*ix)+2*kbl*ix*cos(kbl*ix)+(kbl*kbl*ix*ix-2)*sin(kbl*ix))/(2*pi*k0*ix*ix*ix);
	}
    }
  }//else if (windPrior==5){

    k0a=atres/(50.0);//nati*atres/25.0;//25.0/(float(nati)*atres);
    k0c=ctres/(50.0);//ncti*ctres/25.0;//25.0/(float(ncti)*ctres);
    //initialize correlation function
    magC0=0;
    for (int ia=0;ia<natif;ia++)
    {
      for (int ic=0;ic<nctif;ic++)
	{
	  //incorr[ia*ncti+ic]=exp(-1.0*k0a*fabs(double(ia-floor(nati/2))))*exp(-1.0*k0c*fabs(double(ic-floor(ncti/2))));
	  //dir[ia][ic]=(float)incorr[ia*ncti+ic]*rtd;
	  /*
	  if (ia<nati&&ic<ncti){
	  incorr[ia+nati*ic][0]=exp(-1.0*k0a*fabs(double(ia-floor(nati/2))))*exp(-1.0*k0c*fabs(double(ic-floor(ncti/2))));
	  incorr[ia+nati*ic][1]=0.0;
	  }else{
	     incorr[ia+nati*ic][0]=0.0;
	     incorr[ia+nati*ic][1]=0.0;
	  }
	  */
	  if (ia<nati&&ic<ncti){
	    incorr[ia*nctif+ic][0]=exp(-1.0*k0a*fabs(double(ia-floor(nati/2))))*exp(-1.0*k0c*fabs(double(ic-floor(ncti/2))));
	    incorr[ia*nctif+ic][1]=0.0;
	  }else{
	    incorr[ia*nctif+ic][0]=0.0;
	    incorr[ia*nctif+ic][1]=0.0;
	  }
	  magC0+=incorr[ia*nctif+ic][0];
	  //dir[ia][ic]=(float)incorr[ia+nati*ic][0]*rtd;
	}
    }
    printf("magC0=%g\n",magC0);
    
    fftw_execute(plnC);
    /*
    for (int ia=0;ia<natif;ia++)
      {
	for (int ic=0;ic<nctif;ic++)
	  {
	    outcorr[ia*nctif+ic][0]/=(1.0*natif*nctif*(magC0));
	    outcorr[ia*nctif+ic][1]/=(1.0*natif*nctif*(magC0));
	  }
      }
    */
    // }
    
  
  // printf("here\n");

  /*
  float noise;
  //option to modify the sigma0 value
  for (int imeas=0;imeas<mxmeas;imeas++)
	{
	  
	  //------------------------------------------------//
	  // generate synthetic sigma0 for this measurement //
	  //------------------------------------------------//
	  sig=0;
	  //hard code QSCAT style gmf type 
	  if (inc[imeas]>50.0*dtr)// *3.14159/180.0)
	    {
	      meast = Meas::MeasTypeE(1);// VV_MEAS_TYPE);
	    }else
	    {
	      meast = Meas::MeasTypeE(2);//HH_MEAS_TYPE;
	    }
	  //printf("inc: %f type: %s\n",inc[imeas]*180.0/3.14159,meas_type_map[(int)meast]);
	  for (int iwvcs=0;iwvcs<numWVCs[imeas];iwvcs++)
	    { 
	      //chi=(dir[atis[imeas][iwvcs]][ctis[imeas][iwvcs]]-az[imeas]*rtd+360.0);
	      chi=(180.0-az[imeas]*rtd+360.0);
	      chi=chi*dtr;
	      gmf.GetInterpolatedValue(meast,inc[imeas],spd[atis[imeas][iwvcs]][ctis[imeas][iwvcs]],chi,&sigwvc);
	     
	      sig+=ap[imeas][iwvcs]*sigwvc;
	      //}
	    }//iwvcs loop
	  //noise=float(rand( )) / (float(RAND_MAX) + 1.0);//uniforn rand num between 0 and 1
	  noise=0.0;
	  for (int i=0;i<12;i++){
	    noise+=float(rand()) / (float(RAND_MAX) + 1.0);//uniforn rand num between 0 and 1
	  }
	  noise=(noise-6.0)/12.0;//~normal distributed rand num. mean=0 var=1 
	  noise=noise*((A[imeas]-1.0)*sig*sig+B[imeas]*sig+C[imeas]);
	  value[imeas]=sig+noise;
	}//imeas loop
  */


  //--------
  // do ave reconstruction on sigma0 to use over land/ice/rain
  //--------
  float ** aveH=(float**)make_array(sizeof(float),2,nati,ncti);
  float ** aveV=(float**)make_array(sizeof(float),2,nati,ncti);
  float ** aveHcnt=(float**)make_array(sizeof(float),2,nati,ncti);
  float ** aveVcnt=(float**)make_array(sizeof(float),2,nati,ncti);
  for (int ia=0;ia<nati;ia++)
    {
      for (int ic=0;ic<ncti;ic++)
	{
	  aveH[ia][ic]=0;
	  aveV[ia][ic]=0;
	  aveHcnt[ia][ic]=0;
	  aveVcnt[ia][ic]=0;
	}
    }
  
  for (int imeas=0;imeas<mxmeas;imeas++)
    {
      //printf("inc: %f type: %s\n",inc[imeas]*180.0/3.14159,meas_type_map[(int)meast]);
      for (int iwvcs=0;iwvcs<numWVCs[imeas];iwvcs++)
	{
	  if (inc[imeas]>50.0*dtr)//*3.14159/180.0)
	    {
	      aveV[atis[imeas][iwvcs]][ctis[imeas][iwvcs]]+=value[imeas];
	      aveVcnt[atis[imeas][iwvcs]][ctis[imeas][iwvcs]]+=1;
	    }else
	    {
	      aveH[atis[imeas][iwvcs]][ctis[imeas][iwvcs]]+=value[imeas];
	      aveHcnt[atis[imeas][iwvcs]][ctis[imeas][iwvcs]]+=1;
	    }
	}
    }
  for (int ia=0;ia<nati;ia++)
    {
      for (int ic=0;ic<ncti;ic++)
	{
	  if (aveHcnt[ia][ic]>0){ 
	    aveH[ia][ic]/= aveHcnt[ia][ic];
	      }
	  if (aveVcnt[ia][ic]>0){ 
	    aveV[ia][ic]/= aveVcnt[ia][ic];
	  }
	  //set non-wind cells to AVE
	  /*	
	      flg2=int(flg[ia][ic]);
	  if (flg2!=0)
	    {
	      spd[ia][ic]=aveV[ia][ic];
	      dir[ia][ic]=aveH[ia][ic];
	    }
	  */
	}
    }

  //write out ave file here if desired
  if (outputFilenameAVE!=NULL){
    //-------------------------//
    // Write the output arrays //
    //-------------------------//
    //this outputs in same format as l2b_to_arrays
    //sscanf(outputFilenameAVE,"%s",tmpstr);
    //sprintf(tmpstr,"%s_%d",outputFilenameAVE,iter+1);
    printf("writing file %s\n",outputFilenameAVE);
    //FILE* 
    wfp=fopen(outputFilenameAVE,"w");
    if(!wfp){
      fprintf(stderr,"Cannot open file %s for writing\n",outputFilenameAVE);
      exit(1);
    }
    f=first_valid_ati;
    n=nvalid_ati;
    if(fwrite(&first_valid_ati,sizeof(int),1,wfp)!=1  ||
       fwrite(&nvalid_ati,sizeof(int),1,wfp)!=1  ||
       fwrite(&ncti,sizeof(int),1,wfp)!=1  ||
       !write_array(wfp,&aveV[f],sizeof(float),2,n,ncti) ||
       !write_array(wfp,&aveH[f],sizeof(float),2,n,ncti) ||
       !write_array(wfp,&lat[f],sizeof(float),2,n,ncti) ||
       !write_array(wfp,&lon[f],sizeof(float),2,n,ncti) ||
       !write_array(wfp,&flg[f],sizeof(char),2,n,ncti)){
      fprintf(stderr,"Error writing to file %s\n",outputFilename);
      exit(1);
    }

   
    sscanf(outputFilenameAVE,"%s",outputFilenameAVE_reconst);
    printf("%s\n",outputFilenameAVE_reconst);
    strcat(outputFilenameAVE_reconst,"_reconst");
    printf("%s\n",outputFilenameAVE_reconst);

    sscanf(outputFilenameAVE,"%s",outputFilenameAVE_grad);
    strcat(outputFilenameAVE_grad,"_grad");
    printf("%s %s %s\n",outputFilenameAVE,outputFilenameAVE_reconst,outputFilenameAVE_grad);

    
  }
  
  

  //-------------------------------//
  //start reconsruction iterations //
  //-------------------------------//
  for (int iter=0;iter<maxiter;iter++)
    {
      printf("iteration %d\n",iter);
      //initialize gradient to zero on each iteration
      mxgradobj1=0;
      mxgradobj2=0;
      mxgradobjH=0;
      mxgradobjV=0;

      for (int ia=0;ia<nati;ia++)
	{
	  for (int ic=0;ic<ncti;ic++)
	    {
	      gradobj1[ia][ic]=0.0;
	      gradobj2[ia][ic]=0.0;
	      gradobjV[ia][ic]=0.0;
	      gradobjH[ia][ic]=0.0;
	     
	    }
	} 
     

      if ((iter%10==0||iter==0)&&Filtflg==1){//low pass filter the wind field
		//convert to U and V
	for (int ia=0;ia<natif;ia++){
	    for (int ic=0;ic<nctif;ic++){
	     
	      if (ia<nati&&ic<ncti){
		inU[ia*nctif+ic][0]=(double)spd[ia][ic]*cos(dir[ia][ic]*dtr);
		inU[ia*nctif+ic][1]=0.0;
		inV[ia*nctif+ic][0]=(double)spd[ia][ic]*sin(dir[ia][ic]*dtr);
		inV[ia*nctif+ic][1]=0.0;
	      }else{
		inV[ia*nctif+ic][0]=0.0;
		inV[ia*nctif+ic][1]=0.0;
		inU[ia*nctif+ic][0]=0.0;
		inU[ia*nctif+ic][1]=0.0;
	      }

	    }//ic
	 }//ia
	printf("before fft smooth\n");

	fftw_execute(plnU);
	fftw_execute(plnV);
	
	//fftw_execute(plnC);
	
	for (int ia=0;ia<natif;ia++){
	  for (int ic=0;ic<nctif;ic++){
	    
	    magC=sqrt(outcorr[ia*nctif+ic][0]*outcorr[ia*nctif+ic][0]+outcorr[ia*nctif+ic][1]*outcorr[ia*nctif+ic][1])/magC0;
	    
	    outU[ia*nctif+ic][1]=outU[ia*nctif+ic][1]*magC;
	    outU[ia*nctif+ic][0]=outU[ia*nctif+ic][0]*magC;
	    
	    outV[ia*nctif+ic][1]=outV[ia*nctif+ic][1]*magC;
	    outV[ia*nctif+ic][0]=outV[ia*nctif+ic][0]*magC;
	    
	  }
	}
	
	fftw_execute(plnUI);
	fftw_execute(plnVI);
	
	printf("after fft smooth\n");
	for (int ia=0;ia<nati;ia++){
	    for (int ic=0;ic<ncti;ic++){
	      magU=inU[ia*nctif+ic][0]/(natif*nctif*1.0);
	      magV=inV[ia*nctif+ic][0]/(natif*nctif*1.0);
	      spd[ia][ic]=sqrt(magU*magU+magV*magV);
	      dir[ia][ic]=atan2(magV,magU)*rtd;
	      if (dir[ia][ic]<0) dir[ia][ic]+=360.0;
	    }//ic
	 }//ia
      }//if iter%5==0

      for (int imeas=0;imeas<mxmeas;imeas++)
	{
	  
	  //------------------------------------------------//
	  // generate synthetic sigma0 for this measurement //
	  //------------------------------------------------//
	  sig=0;
	  sigL=0;
	  sigW=0;
	  apL=0;
	  apW=0;
	  //hard code QSCAT style gmf type 
	  if (inc[imeas]>50.0*dtr)//*3.14159/180.0)
	    {
	      meast = Meas::MeasTypeE(1);// VV_MEAS_TYPE);
	    }else
	    {
	      meast = Meas::MeasTypeE(2);//HH_MEAS_TYPE;
	    }
	  //printf("inc: %f type: %s\n",inc[imeas]*180.0/3.14159,meas_type_map[(int)meast]);
	  for (int iwvcs=0;iwvcs<numWVCs[imeas];iwvcs++)
	    {
	      flg2=int(flg[atis[imeas][iwvcs]][ctis[imeas][iwvcs]]);
	      
	      if (flg2==0){
		//project wind trough gmf
		
		chi=(dir[atis[imeas][iwvcs]][ctis[imeas][iwvcs]]-az[imeas]*rtd+180.0);
		
		chi=chi*dtr;
		
		gmf.GetInterpolatedValue(meast,inc[imeas],spd[atis[imeas][iwvcs]][ctis[imeas][iwvcs]],chi,&sigwvc);
	      	
		sigW+=ap[imeas][iwvcs]*sigwvc;
		apW+=ap[imeas][iwvcs];
	      }else{
		//printf("this cell is not wind, flg: %d\n",flg2);
		if (inc[imeas]>50.0*dtr){
		  sigwvc=aveV[atis[imeas][iwvcs]][ctis[imeas][iwvcs]];//v-pol
		}else{
		  sigwvc=aveH[atis[imeas][iwvcs]][ctis[imeas][iwvcs]];//h-pol
		}
		
		sigL+=ap[imeas][iwvcs]*sigwvc;
		apL+=ap[imeas][iwvcs];
	      }
	      //sample synthetic sigma0 with aperture function
	      sig+=ap[imeas][iwvcs]*sigwvc;
	      

	    }//iwvcs loop
	  if (apL>0){ 
	    sigL=sigL/apL;
	    //if (imeas>1066110) 
	    //printf("OUTSIDE apL: %g \n",apL);
	    //printf("OUTSIDE apW: %g \n",apW);
	  }
	  if (apW>0){ 
	    sigW=sigW/apW;
	    //printf("apW: %g \n",apW);
	    //printf("OUTSIDE apW: %g \n",apW);
	  }
	  
	  
	  //if (apL>0&&apW>0) {
	  //  printf("OUTSIDE apL: %g apW: %g \n",apL, apW);
	  // }
	  //printf("value: %g sig: %g sigL: %g sigW: %g, apL: %g apW :%g at: %d ct: %d imeas %d\n",value[imeas],sig,sigL,sigW,apL,apW,atis[imeas][0],ctis[imeas][0],imeas,spd[atis[imeas][0]][ctis[imeas][0]]);
	  //printf("inc: %f type: %s sig: %f value: %f\n",inc[imeas]*180.0/3.14159,meas_type_map[(int)meast],sig,value[imeas]);
	 

	  //--------------------------------------//
	  //calulate gradent of objective function//
	  //--------------------------------------//
	  alpha=A[imeas]-1.0;
	  beta=B[imeas];
	  gamma=C[imeas];
	  R=alpha*(sig*sig)+beta*sig+gamma;
	  if (R<=0) R=1e-5;//avoid divide by zero
	  Kobj=((value[imeas]-sig)-(alpha*sig+beta/2.0))/R+((value[imeas]-sig)*(value[imeas]-sig)*(alpha*sig+beta/2.0))/(R*R);
	 
	  //printf("imeas: %d \n",imeas);
	  PW=apW*apW*1.0e9;
	  PL=apL*apL*1.0e9;
	  //update gradent due to this measurement
	  for (int iwvcs=0;iwvcs<numWVCs[imeas];iwvcs++)
	    {//printf("iwvcs: %d \n",iwvcs);
	      flg2=int(flg[atis[imeas][iwvcs]][ctis[imeas][iwvcs]]);
	      if (flg2==0){
		
	      //get gradient of gmf
		gradgmf1=0;
		gradgmf2=0;
		if (SDflag==1){	
		  chi=(dir[atis[imeas][iwvcs]][ctis[imeas][iwvcs]]-az[imeas]*rtd+180.0);
		  chi=chi*dtr;

		  speed=spd[atis[imeas][iwvcs]][ctis[imeas][iwvcs]];
		  
		  if (speed>50.0-ds) speed=50.0-ds;
		  if (speed<ds) speed=ds;
		  
		  gmf.GetInterpolatedValue(meast,inc[imeas],speed+ds,chi,&sigp);
		  gmf.GetInterpolatedValue(meast,inc[imeas],speed-ds,chi,&sigm);
		  
		  gradgmf1=(sigp-sigm)/(2.0*ds);
		  
		  
		  gmf.GetInterpolatedValue(meast,inc[imeas],speed,chi+ds*dtr,&sigp);
		  gmf.GetInterpolatedValue(meast,inc[imeas],speed,chi-ds*dtr,&sigm);
		  
		  gradgmf2=(sigp-sigm)/(2.0*ds);
		}else{//get gradient in U V
		  magU=spd[atis[imeas][iwvcs]][ctis[imeas][iwvcs]]*cos(dir[atis[imeas][iwvcs]][ctis[imeas][iwvcs]]*dtr);
		  magV=spd[atis[imeas][iwvcs]][ctis[imeas][iwvcs]]*sin(dir[atis[imeas][iwvcs]][ctis[imeas][iwvcs]]*dtr);

		  speed=sqrt((magU+1)*(magU+1)+magV*magV);
		  if (speed>50.0) speed=50.0;
		  //if (speed<0) speed=ds;
		  
		  dirt=atan2(magV,magU+1)*rtd;
		  if (dirt<0) dirt+=360.0;
 
		  chi=(dirt-az[imeas]*rtd+180.0);
		  chi=chi*dtr;


		  gmf.GetInterpolatedValue(meast,inc[imeas],speed,chi,&sigp);

		  speed=sqrt((magU-1)*(magU-1)+(magV)*(magV));
		  if (speed>50.0) speed=50.0;
		  //if (speed<0) speed=ds;
		  
		  dirt=atan2(magV,magU-1)*rtd;
		  if (dirt<0) dirt+=360.0;
		  
		  chi=(dirt-az[imeas]*rtd+180.0);
		  chi=chi*dtr;


		  gmf.GetInterpolatedValue(meast,inc[imeas],speed,chi,&sigm);
		  
		  gradgmf1=(sigp-sigm)/(2.0);
		  
		  speed=sqrt((magU)*(magU)+(magV+1)*(magV+1));
		  if (speed>50.0) speed=50.0;
		  //if (speed<0) speed=ds;
		  
		  dirt=atan2(magV+1,magU)*rtd;
		  if (dirt<0) dirt+=360.0;
		  
		  chi=(dirt-az[imeas]*rtd+180.0);
		  chi=chi*dtr;

		  gmf.GetInterpolatedValue(meast,inc[imeas],speed,chi*dtr,&sigp);

		  speed=sqrt((magU)*(magU)+(magV-1)*(magV-1));
		  if (speed>50.0) speed=50.0;
		  //if (speed<0) speed=ds;
		  
		  dirt=atan2(magV-1,magU)*rtd;
		  if (dirt<0) dirt+=360.0;
		  
		  chi=(dirt-az[imeas]*rtd+180.0);
		  chi=chi*dtr;


		  gmf.GetInterpolatedValue(meast,inc[imeas],speed,chi*dtr,&sigm);
		  
		  gradgmf2=(sigp-sigm)/(2.0);
		  
		}//SDflag
		
		//update gradient
		
		gradobj1[atis[imeas][iwvcs]][ctis[imeas][iwvcs]]+=Kobj*ap[imeas][iwvcs]*gradgmf1;//speed or U
		gradobj2[atis[imeas][iwvcs]][ctis[imeas][iwvcs]]+=Kobj*ap[imeas][iwvcs]*gradgmf2;//direction or V
	      }else{
		if (inc[imeas]>50.0*dtr){
		  gradobjV[atis[imeas][iwvcs]][ctis[imeas][iwvcs]]+=Kobj*ap[imeas][iwvcs];//sigma_v
		}else{
		  gradobjH[atis[imeas][iwvcs]][ctis[imeas][iwvcs]]+=Kobj*ap[imeas][iwvcs];//sigma_h
		}
		
	      }
	      
	      //-----------//
	      //add a prior in sigma0//
	      //-----------//
	      gradprior1=0;
	      gradprior2=0;
	      gradpriorH=0;
	      gradpriorV=0;

	      if (windPrior==-1){
		gmf.GetInterpolatedValue(meast,inc[imeas],spd[atis[imeas][iwvcs]][ctis[imeas][iwvcs]],chi,&sigwvc);
		
		//------------
		//Gaussian prior in sigma0
		flg2=int(flg[atis[imeas][iwvcs]][ctis[imeas][iwvcs]]);
		if (flg2==0){//only update wind over valid wind cells
		if (apW>0){
		  val=(value[imeas]-sigL*apL)/apW;
		  //}else{
		  //val=value[imeas];
		  //}
		  
		    gradprior1=(val-sigwvc)*PW*gradgmf1;///1.0e-7*gradgmf1;
		    gradprior2=(val-sigwvc)*PW*gradgmf2;///1.0e-7*gradgmf2;
		  
		    gradpriorV=0.0;///1.0e-7*gradgmf1;
		    gradpriorH=0.0;///1.0e-7*gradgmf2;
		}
		}else{//only update sigma^0 over invalid wind cells
		if (apL>0){
		  
		  		  //if (apL>0){
		  val=(value[imeas]-sigW*apW)/apL;
		  //}else{
		  //val=value[imeas];
		  //}
		  if (inc[imeas]>50.0*dtr){
		    gradpriorV=(val-aveV[atis[imeas][iwvcs]][ctis[imeas][iwvcs]])*PL;///1.0e-7;
		    gradpriorH=0;//(value[imeas]-sigwvc)/1.0e-7;
		  }else{
		    gradpriorV=0;//(value[imeas]-sigwvc)/1.0e-7;
		    gradpriorH=(val-aveH[atis[imeas][iwvcs]][ctis[imeas][iwvcs]])*PL;///1.0e-7;
		  }
		  gradprior1=0.0;
		  gradprior2=0.0;
		  //gradprior1=(value[imeas]-sigwvc)/1.0e-7;
		  //gradprior2=(value[imeas]-sigwvc)/1.0e-7;
		  //gradprior1=(aveV[atis[imeas][iwvcs]][ctis[imeas][iwvcs]]-spd[atis[imeas][iwvcs]][ctis[imeas][iwvcs]]);
		  //gradprior2=(aveH[atis[imeas][iwvcs]][ctis[imeas][iwvcs]]-dir[atis[imeas][iwvcs]][ctis[imeas][iwvcs]]);
		}
		}
		
		/////////////
		//add prior//
		/////////////
		
		gradobj1[atis[imeas][iwvcs]][ctis[imeas][iwvcs]]+=gradprior1;
		gradobj2[atis[imeas][iwvcs]][ctis[imeas][iwvcs]]+=gradprior2;
		
		gradobjV[atis[imeas][iwvcs]][ctis[imeas][iwvcs]]+=gradpriorV;
		gradobjH[atis[imeas][iwvcs]][ctis[imeas][iwvcs]]+=gradpriorH;
		
		
	      }//if windPrior
	      
	      //find the maximum gradient for step size normalization later
	      
	      if(mxgradobj1<fabs(gradobj1[atis[imeas][iwvcs]][ctis[imeas][iwvcs]])) mxgradobj1=fabs(gradobj1[atis[imeas][iwvcs]][ctis[imeas][iwvcs]]);
	      if(mxgradobj2<fabs(gradobj2[atis[imeas][iwvcs]][ctis[imeas][iwvcs]])) mxgradobj2=fabs(gradobj2[atis[imeas][iwvcs]][ctis[imeas][iwvcs]]);
	      if(mxgradobjV<fabs(gradobjV[atis[imeas][iwvcs]][ctis[imeas][iwvcs]])) mxgradobjV=fabs(gradobjV[atis[imeas][iwvcs]][ctis[imeas][iwvcs]]);
	      if(mxgradobjH<fabs(gradobjH[atis[imeas][iwvcs]][ctis[imeas][iwvcs]])) mxgradobjH=fabs(gradobjH[atis[imeas][iwvcs]][ctis[imeas][iwvcs]]);
	      
	    }//iwvcs loop
	  
	}//imeas loop



      //---------------------------------------//
      // add a prior in wind domain if desired //
      //---------------------------------------//

      if (windPrior==1){
      //-------------------------------
      // add independent prior of wind
      //Gaussian prior independent in U,V with k^-2 spectrum and zero means 
      //--------------------------------
	//this is very slow for faster implementation use windPrior==5
	float rinv_uu,rinv_vv,rinv_uv,rinv_vu;
      int da,dc;
      int alim,clim;
      int ia2,ic2;
      for (int ia=0;ia<nati;ia++)
	{
	  for (int ic=0;ic<ncti;ic++)
	    {
	      gradprior1=0;

	      gradprior2=0;
	      //for each pixel find gradient of prior wth respect to every other pixel
	      //for (int ia2=0;ia2<nati;ia2++)//whole thing is way to slow
	      alim=2.0/k0a;//go out at least twice the correlation length
	      clim=2.0/k0c;
	      alim=10;
	      clim=10;
	      for (int ia2t=-alim;ia2t<=alim;ia2t++)
		{
		  if (ia2t>=0) ia2=ia2t;
		  if (ia2t<0) ia2=nati+ia2t;
		  //for (int ic2=0;ic2<ncti;ic2++)//whole thing is way to slow
		  for (int ic2t=-clim;ic2t<=clim;ic2t++)
		    {
		      if (ic2t>=0) ic2=ic2t;
		      if (ic2t<0) ic2=ncti+ic2t;
		      da=(ia-ia2);
		      if (da<0) da+=nati;
		      dc=(ic-ic2);
		      if (dc<0) dc+=ncti;
		      //more genearal form
		      rinv_uu=0.5*cova[da]+0.5*covc[dc];
		      //rinv_uu=0.0;
		      //if (fabs(1.0*da)<5&fabs(1.0*dc)<5) rinv_uu=1.0;
		      rinv_vv=rinv_uu;
		      rinv_uv=0;
		      rinv_vu=0;
		      if (da==0&dc==0)
			{
			  gradprior1-=(rinv_uu*cos(dir[ia][ic]*dtr)*cos(dir[ia][ic]*dtr)+2*spd[ia][ic]*cos(dir[ia][ic]*dtr)*sin(dir[ia][ic]*dtr)+spd[ia][ic]*sin(dir[ia][ic]*dtr)*sin(dir[ia][ic]*dtr));
			  gradprior2-=(rinv_uu*spd[ia][ic]*-1.0*sin(dir[ia][ic]*dtr)+rinv_uv*spd[ia][ic]*spd[ia][ic]*(2*cos(dir[ia][ia]*dtr)-1)+rinv_vv*spd[ia][ic]*spd[ia][ic]*sin(dir[ia][ic]*dtr)*cos(dir[ia][ic]*dtr));
			}else
			{
			  gradprior1-=0.5*(rinv_uu*spd[ia2][ic2]*cos(dir[ia2][ic2]*dtr)*cos(dir[ia][ic]*dtr)+rinv_uv*spd[ia2][ic2]*cos(dir[ia2][ic2]*dtr)*sin(dir[ia][ic]*dtr)+rinv_vu*spd[ia2][ic2]*sin(dir[ia2][ic2]*dtr)*cos(dir[ia][ic]*dtr)+rinv_vv*spd[ia2][ic2]*sin(dir[ia2][ic2]*dtr)*sin(dir[ia][ic]*dtr));
			  gradprior2-=0.5*(rinv_uu*spd[ia2][ic2]*cos(dir[ia2][ic2]*dtr)*spd[ia][ic]*-1.0*sin(dir[ia][ic]*dtr)+rinv_uv*spd[ia2][ic2]*spd[ia][ic]*cos(dir[ia2][ic2]*dtr)*cos(dir[ia][ic]*dtr)+rinv_vu*spd[ia2][ic2]*sin(dir[ia2][ic2]*dtr)*spd[ia][ic]*-1.0*sin(dir[ia][ic]*dtr)+rinv_vv*spd[ia2][ic2]*sin(dir[ia2][ic2]*dtr)*spd[ia][ic]*cos(dir[ia][ic]*dtr));
			}

		      //old derivation, may be wrong
		      //rinv=0.5*cova[da]+0.5*covc[dc];
		      //gradprior1-=rinv*spd[ia2][ic2]*(cos(dir[ia2][ic2]*dtr)*cos(dir[ia][ic]*dtr)+sin(dir[ia2][ic2]*dtr)*sin(dir[ia][ic]*dtr));
		      
		      //gradprior2+=rinv*spd[ia2][ic2]*spd[ia][ic]*(sin(dir[ia][ic]*dtr)*cos(dir[ia2][ic2]*dtr)-cos(dir[ia][ic]*dtr)*sin(dir[ia2][ic2]*dtr));
		      


		    }
		}
	      //da=0;
	      //dc=0;
	      //rinv=0.5*cova[da]+0.5*covc[dc];
	      //gradprior1-=spd[ia][ic]*rinv;
	      /////////////
	      //add prior//
	      /////////////
	      gradobj1[ia][ic]+=gradprior1/float(4*alim*clim)/10.0;
	      gradobj2[ia][ic]+=gradprior2/float(4*alim*clim)/10.0;
	      
	      if(mxgradobj1<fabs(gradobj1[ia][ic])) mxgradobj1=fabs(gradobj1[ia][ic]);
	      if(mxgradobj2<fabs(gradobj2[ia][ic])) mxgradobj2=fabs(gradobj2[ia][ic]);
	      
	    }
	}
      }else if(windPrior==2){
	//min norm prior to drive nullspace of current linearized problem to zero
	//zero mean Gaussian prior in U,V with diagonal covariance 1/p*I
	for (int ia=0;ia<nati;ia++)
	  {
	    for (int ic=0;ic<ncti;ic++)
	      {
		gradprior1=-spd[ia][ic]/1e5;
		gradprior2=0;
		
		gradobj1[ia][ic]+=gradprior1;
		gradobj2[ia][ic]+=gradprior2;
	      }
	  }

      }else if(windPrior==3){

      //-------------------------------
      // add independent prior of wind
      //Gaussian prior in U,V, zero means 
      //and R^{-1} = J (Fisher Information) for the covariance  
      // This attenuates components that are expected to be noisy on each iteration
      //--------------------------------
	//this option is HORRIBLY SLOW
      float rinv_uu,rinv_vv,rinv_uv,rinv_vu;
      int da,dc;
      int alim,clim;
      int ia2,ic2;
      int isinflag;
      float chip,chim,speedp,speedm,direc,direcp,direcm,gradgmfu,gradgmfv,gradgmfu1,gradgmfv1,u,v;

      for (int ia=0;ia<nati;ia++)
	{printf("Calculating Gradient of Prior ia=%d, nati=%d\n",ia,nati);
	  for (int ic=0;ic<ncti;ic++)
	    {
	      //printf("Calculating Gradient of Prior ic=%d, ncti=%d, speed=%f\n",ic,ncti,spd[ia][ic]);
	      if (spd[ia][ic]>0){
		//printf("Calculating Gradient of Prior ic=%d, ncti=%d, speed=%f\n",ic,ncti,spd[ia][ic]);
	     
	      gradprior1=0;
	      gradprior2=0;
	      //	      for (int ia2=0;ia2<nati;ia2++)
	      //{
	      //  for (int ic2=0;ic2<ncti;ic2++)
	      //		    {
	      alim=2.0*25.0/atres;//go out at least twice the azimuth beamwidth (hard coded to 50 km for now)
	      clim=2.0*25.0/ctres;
	      alim=10;
	      clim=10;
	      //printf("initializing J's \n");
	      for (int ia2t=-alim;ia2t<=alim;ia2t++)
		{
		  if (ia2t>=0) ia2=ia2t;
		  if (ia2t<0) ia2=nati+ia2t;
		  //for (int ic2=0;ic2<ncti;ic2++)//whole thing is way to slow
		  for (int ic2t=-clim;ic2t<=clim;ic2t++)
		    {
		      if (ic2t>=0) ic2=ic2t;
		      if (ic2t<0) ic2=ncti+ic2t;
		      J_uu[ia2][ic2]=0;
		      J_vv[ia2][ic2]=0;
		      J_uv[ia2][ic2]=0;
		      J_vu[ia2][ic2]=0;
		    }
		}
	      //printf("Calculating Fisher Info \n");
	      //calculate Fisher Information matrix for this cell
	      
	      for (int imeas=0;imeas<mxmeas;imeas++)
		{
		  //check to see if this measurement is in the current cell 
		  isinflag=0;
		  for (int iwvcs=0;iwvcs<numWVCs[imeas];iwvcs++)
		    {
		      if (atis[imeas][iwvcs]==ia&ctis[imeas][iwvcs]==ic) isinflag=1;
		    }
		  if (isinflag){
		    //------------------------------------------------//
		    // generate synthetic sigma0 for this measurement //
		    //------------------------------------------------//
		    sig=0;
		    //hard code QSCAT style gmf type 
		    if (inc[imeas]>50.0*dtr)//*3.14159/180.0)
		      {
			meast = Meas::MeasTypeE(1);// VV_MEAS_TYPE);
		      }else
		      {
			meast = Meas::MeasTypeE(2);//HH_MEAS_TYPE;
		      }
		    //printf("inc: %f type: %s\n",inc[imeas]*180.0/3.14159,meas_type_map[(int)meast]);
		    for (int iwvcs=0;iwvcs<numWVCs[imeas];iwvcs++)
		      {
			
			//project wind trough gmf
			
			chi=(dir[atis[imeas][iwvcs]][ctis[imeas][iwvcs]]-az[imeas]*rtd+180.0);
			
			chi=chi*dtr;
			gmf.GetInterpolatedValue(meast,inc[imeas],spd[atis[imeas][iwvcs]][ctis[imeas][iwvcs]],chi,&sigwvc);
			
			//sample synthetic sigma0 with aperture function
			sig+=ap[imeas][iwvcs]*sigwvc;
			
		      }//iwvcs loop
		    
		    alpha=A[imeas]-1.0;
		    beta=B[imeas];
		    gamma=C[imeas];
		    R=alpha*(sig*sig)+beta*sig+gamma;
		    if (R<=0) R=1e-5;//avoid divide by zero
		    Kobj=1.0/R+(2*alpha*sig+beta)/(2*R*R);
		    
		    
		    direc=dir[ia][ic];
		    speed=spd[ia][ic];
		    if (speed>50.0-ds) speed=50.0-ds;
		    if (speed<ds) speed=ds;
		    u=speed*cos(direc*dtr);
		    v=speed*sin(direc*dtr);
		    speedp=sqrt((u+ds)*(u+ds)+v*v);
		    speedm=sqrt((u-ds)*(u-ds)+v*v);
		    direcp=atan2(v,u+ds)*rtd;
		    direcm=atan2(v,u-ds)*rtd;
		    
		    chip=(direcp-az[imeas]*rtd+180.0);
		    chip=chi*dtr;
		    
		    chim=(direcm-az[imeas]*rtd+180.0);
		    chim=chi*dtr;
		    
		    gmf.GetInterpolatedValue(meast,inc[imeas],speedp,chip,&sigp);
		    gmf.GetInterpolatedValue(meast,inc[imeas],speedm,chim,&sigm);
		    
		    gradgmfu=(sigp-sigm)/(2.0*ds);
		    //gradgmf1=0.0;
		    
		    speedp=sqrt((v+ds)*(v+ds)+u*u);
		    speedm=sqrt((v-ds)*(v-ds)+u*u);
		    direcp=atan2(v+ds,u)*rtd;
		    direcm=atan2(v-ds,u)*rtd;
		    
		    chip=(direcp-az[imeas]*rtd+180.0);
		    chip=chi*dtr;
		    
		    chim=(direcm-az[imeas]*rtd+180.0);
		    chim=chi*dtr;
		    
		    gmf.GetInterpolatedValue(meast,inc[imeas],speedp,chip*dtr,&sigp);
		    gmf.GetInterpolatedValue(meast,inc[imeas],speedm,chim*dtr,&sigm);
		    
		    gradgmfv=(sigp-sigm)/(2.0*ds);
		    
		    
		    
		    //go through all wvcs that this meas hits 
		    for (int iwvcs=0;iwvcs<=numWVCs[imeas];iwvcs++)
		      {
			//calculate the gradient of gmf
			
			direc=dir[atis[imeas][iwvcs]][ctis[imeas][iwvcs]];
			speed=spd[atis[imeas][iwvcs]][ctis[imeas][iwvcs]];
			if (speed>50.0-ds) speed=50.0-ds;
			if (speed<ds) speed=ds;
			u=speed*cos(direc*dtr);
			v=speed*sin(direc*dtr);
			speedp=sqrt((u+ds)*(u+ds)+v*v);
			speedm=sqrt((u-ds)*(u-ds)+v*v);
			direcp=atan2(v,u+ds)*rtd;
			direcm=atan2(v,u-ds)*rtd;
			
			chip=(direcp-az[imeas]*rtd+180.0);
			chip=chi*dtr;
			
			chim=(direcm-az[imeas]*rtd+180.0);
			chim=chi*dtr;
			
			gmf.GetInterpolatedValue(meast,inc[imeas],speedp,chip,&sigp);
			gmf.GetInterpolatedValue(meast,inc[imeas],speedm,chim,&sigm);
			
			gradgmfu1=(sigp-sigm)/(2.0*ds);
			//gradgmf1=0.0;
			
			speedp=sqrt((v+ds)*(v+ds)+u*u);
			speedm=sqrt((v-ds)*(v-ds)+u*u);
			direcp=atan2(v+ds,u)*rtd;
			direcm=atan2(v-ds,u)*rtd;
			
			chip=(direcp-az[imeas]*rtd+180.0);
			chip=chi*dtr;
			
			chim=(direcm-az[imeas]*rtd+180.0);
			chim=chi*dtr;
			
			gmf.GetInterpolatedValue(meast,inc[imeas],speedp,chip*dtr,&sigp);
			gmf.GetInterpolatedValue(meast,inc[imeas],speedm,chim*dtr,&sigm);
			
			gradgmfv1=(sigp-sigm)/(2.0*ds);
			
			//accumulate Fisher info over measurements
			J_uu[atis[imeas][iwvcs]][ctis[imeas][iwvcs]]+=ap[imeas][iwvcs]*ap[imeas][numWVCs[imeas]]*Kobj*gradgmfu*gradgmfu1;
			J_uv[atis[imeas][iwvcs]][ctis[imeas][iwvcs]]+=ap[imeas][iwvcs]*ap[imeas][numWVCs[imeas]]*Kobj*gradgmfu*gradgmfv1;
			J_vu[atis[imeas][iwvcs]][ctis[imeas][iwvcs]]+=ap[imeas][iwvcs]*ap[imeas][numWVCs[imeas]]*Kobj*gradgmfv*gradgmfu1;
			J_vv[atis[imeas][iwvcs]][ctis[imeas][iwvcs]]+=ap[imeas][iwvcs]*ap[imeas][numWVCs[imeas]]*Kobj*gradgmfv*gradgmfv1;
		      }//iwvcs loop
		    
		    
		    

		  }//else{printf("Not This Cell\n");}//isinflag
		  
		}//for imeas
	      
	      //printf("calculating gradient of prior\n");  
	      //for each pixel find gradient of prior wth respect to every other pixel
	      //for (int ia2=0;ia2<nati;ia2++)//whole thing is way to slow
	      //alim=2.0*25.0/atres;//go out at least twice the azimuth beamwidth (hard coded to 50 km for now)
	      //clim=2.0*25.0/ctres;
	      //alim=10;
	      //clim=10;
	      for (int ia2t=-alim;ia2t<=alim;ia2t++)
		{
		  if (ia2t>=0) ia2=ia2t;
		  if (ia2t<0) ia2=nati+ia2t;
		  //for (int ic2=0;ic2<ncti;ic2++)//whole thing is way to slow
		  for (int ic2t=-clim;ic2t<=clim;ic2t++)
		    {
		      if (ic2t>=0) ic2=ic2t;
		      if (ic2t<0) ic2=ncti+ic2t;
		      da=(ia-ia2);
		      if (da<0) da+=nati;
		      dc=(ic-ic2);
		      if (dc<0) dc+=ncti;
		      //put Fisher information into rinv
		      //rinv=0.5*cova[da]+0.5*covc[dc];
		      rinv_uu=J_uu[ia2][ic2];
		      rinv_vv=J_vv[ia2][ic2];
		      rinv_uv=J_uv[ia2][ic2];
		      rinv_vu=J_vu[ia2][ic2];
		      

		      if (da==0&dc==0)
			{
			  gradprior1=(rinv_uu*cos(dir[ia][ic]*dtr)*cos(dir[ia][ic]*dtr)+2*spd[ia][ic]*cos(dir[ia][ic]*dtr)*sin(dir[ia][ic]*dtr)+spd[ia][ic]*sin(dir[ia][ic]*dtr)*sin(dir[ia][ic]*dtr));
			  gradprior2-=(rinv_uu*spd[ia][ic]*-1.0*sin(dir[ia][ic]*dtr)+rinv_uv*spd[ia][ic]*spd[ia][ic]*(2*cos(dir[ia][ia]*dtr)-1)+rinv_vv*spd[ia][ic]*spd[ia][ic]*sin(dir[ia][ic]*dtr)*cos(dir[ia][ic]*dtr));
			}else
			{
			  gradprior1-=0.5*(rinv_uu*spd[ia2][ic2]*cos(dir[ia2][ic2]*dtr)*cos(dir[ia][ic]*dtr)+rinv_uv*spd[ia2][ic2]*cos(dir[ia2][ic2]*dtr)*sin(dir[ia][ic]*dtr)+rinv_vu*spd[ia2][ic2]*sin(dir[ia2][ic2]*dtr)*cos(dir[ia][ic]*dtr)+rinv_vv*spd[ia2][ic2]*sin(dir[ia2][ic2]*dtr)*sin(dir[ia][ic]*dtr));
			  gradprior2-=0.5*(rinv_uu*spd[ia2][ic2]*cos(dir[ia2][ic2]*dtr)*spd[ia][ic]*-1.0*sin(dir[ia][ic]*dtr)+rinv_uv*spd[ia2][ic2]*spd[ia][ic]*cos(dir[ia2][ic2]*dtr)*cos(dir[ia][ic]*dtr)+rinv_vu*spd[ia2][ic2]*sin(dir[ia2][ic2]*dtr)*spd[ia][ic]*-1.0*sin(dir[ia][ic]*dtr)+rinv_vv*spd[ia2][ic2]*sin(dir[ia2][ic2]*dtr)*spd[ia][ic]*cos(dir[ia][ic]*dtr));
			}

		      
		      
		    }
		}
	      //printf("updating Gradient \n");  
	      /////////////
	      //add prior//
	      /////////////
	      gradobj1[ia][ic]+=gradprior1/float(4*alim*clim)/10.0;
	      gradobj2[ia][ic]+=gradprior2/float(4*alim*clim)/10.0;
	      
	      if(mxgradobj1<fabs(gradobj1[ia][ic])) mxgradobj1=fabs(gradobj1[ia][ic]);
	      if(mxgradobj2<fabs(gradobj2[ia][ic])) mxgradobj2=fabs(gradobj2[ia][ic]);
	      }//if spd     
	    }// for ic
	}//for ia




      }else if(windPrior==4){//Max entropy metric in sigma^0 for several types
	float azAng,incAng,gf;
	meast = Meas::MeasTypeE(1);// VV_MEAS_TYPE);
	incAng=53.0*dtr;
	for (int k=1;k<=4;k++){//loop over several azimuth angles
	  azAng=360.0/4.0*k;
	  for (int ia=0;ia<nati;ia++){
	    for (int ic=0;ic<ncti;ic++){
	      
	      //calculate gradient of gmf
	      chi=(dir[ia][ic]-azAng+180.0);
	      chi=chi*dtr;

	      speed=spd[ia][ic];
	      if (speed>50.0-ds) speed=50.0-ds;
	      if (speed<ds) speed=ds;
	      
	      gmf.GetInterpolatedValue(meast,incAng,speed+ds,chi,&sigp);
	      gmf.GetInterpolatedValue(meast,incAng,speed-ds,chi,&sigm);
	      
	      gradgmf1=(sigp-sigm)/(2.0*ds);
	     

	      gmf.GetInterpolatedValue(meast,incAng,speed,chi+ds*dtr,&sigp);
	      gmf.GetInterpolatedValue(meast,incAng,speed,chi-ds*dtr,&sigm);
	      
	      gradgmf2=(sigp-sigm)/(2.0*ds);
	      
	      //calculate log gmf(param)
	      gmf.GetInterpolatedValue(meast,incAng,speed+ds,chi,&gf);
	      if (gf>0) {gf=log(gf);}else{gf=-1.0;}
	      
	      flg2=int(flg[ia][ic]);
	      if (flg2==0){
		
	      }else{
		gradgmf1=1.0;
		gradgmf2=1.0;
	      }
	      //update gradient
	      gradobj1[ia][ic]+=(gf+1)*gradgmf1;
	      gradobj2[ia][ic]+=(gf+1)*gradgmf2;
	      
	      if(mxgradobj1<fabs(gradobj1[ia][ic])) mxgradobj1=fabs(gradobj1[ia][ic]);
	      if(mxgradobj2<fabs(gradobj2[ia][ic])) mxgradobj2=fabs(gradobj2[ia][ic]);
	      
	    }//ic
	  }//ia
	}//k

      }else if (windPrior==5){
	//-------------------------------
	// add independent prior of wind
	//Gaussian prior independent in U,V with k^-2 spectrum and zero means 
	//--------------------------------
	
	//for testing. For regular processing, comment the next 4 lines.
	/*mxgradobj1=0;
	mxgradobj2=0;
	mxgradobjV=0;
	mxgradobjH=0;
	*/
	//convert to U and V
	for (int ia=0;ia<natif;ia++){
	    for (int ic=0;ic<nctif;ic++){
	     
	      if (ia<nati&&ic<ncti){
		inU[ia*nctif+ic][0]=(double)spd[ia][ic]*cos(dir[ia][ic]*dtr);
		inU[ia*nctif+ic][1]=0.0;
		inV[ia*nctif+ic][0]=(double)spd[ia][ic]*sin(dir[ia][ic]*dtr);
		inV[ia*nctif+ic][1]=0.0;
	      }else{
		inV[ia*nctif+ic][0]=0.0;
		inV[ia*nctif+ic][1]=0.0;
		inU[ia*nctif+ic][0]=0.0;
		inU[ia*nctif+ic][1]=0.0;
	      }

	    }//ic
	 }//ia

	//convolve the current estimate with deconvolution kernel
	//implement in Fourrier Domain using an FFT to minimize multiplies
	printf("before fft\n");

	fftw_execute(plnU);
	fftw_execute(plnV);
	
	//fftw_execute(plnC);
	
	for (int ia=0;ia<natif;ia++){
	  for (int ic=0;ic<nctif;ic++){
	   
	    magC=sqrt(outcorr[ia*nctif+ic][0]*outcorr[ia*nctif+ic][0]+outcorr[ia*nctif+ic][1]*outcorr[ia*nctif+ic][1])/magC0;
	    //magC=1;
	    if (magC<=0){//(outcorr[ia*ncti+ic][0]==0&&outcorr[ia*ncti+ic][1]==0){
	      outU[ia*nctif+ic][1]=0.0;
	      outU[ia*nctif+ic][0]=0.0;
	      outV[ia*nctif+ic][1]=0.0;
	      outV[ia*nctif+ic][0]=0.0;
	    }else{
	      //magC=sqrt(outcorr[ia*ncti+ic][0]*outcorr[ia*ncti+ic][0]+outcorr[ia*ncti+ic][1]*outcorr[ia*ncti+ic][1]);
	      
	      if (noifft==0){
		outU[ia*nctif+ic][1]=outU[ia*nctif+ic][1]/magC;
		outU[ia*nctif+ic][0]=outU[ia*nctif+ic][0]/magC;
		
		outV[ia*nctif+ic][1]=outV[ia*nctif+ic][1]/magC;
		outV[ia*nctif+ic][0]=outV[ia*nctif+ic][0]/magC;
	      }else{
		outU[ia*nctif+ic][1]=outU[ia*nctif+ic][1]*magC;
		outU[ia*nctif+ic][0]=outU[ia*nctif+ic][0]*magC;
		
		outV[ia*nctif+ic][1]=outV[ia*nctif+ic][1]*magC;
		outV[ia*nctif+ic][0]=outV[ia*nctif+ic][0]*magC;
	      }
	      /*
	      outU[ia*ncti+ic][1]=outU[ia*ncti+ic][1]*magC;
	      outU[ia*ncti+ic][0]=outU[ia*ncti+ic][0]*magC;
	      
	      outV[ia*ncti+ic][1]=outV[ia*ncti+ic][1]*magC;
	      outV[ia*ncti+ic][0]=outV[ia*ncti+ic][0]*magC;
	      */
	      //outU[ia*ncti+ic][0]=magC;
	      //outU[ia*ncti+ic][1]=0.0;
	      //if (ia<nati&&ic<ncti) aveH[ia][ic]=magC;

	    }
	    
	    
	  }
	}
	
	fftw_execute(plnUI);
	fftw_execute(plnVI);
	//printf("ifft in: %g out: %g spd: %g\n",in[nati-1000][100],out[nati-1000][100],spd[nati-1000][100]);
	printf("after fft\n");

	for (int ia=0;ia<nati;ia++){
	  for (int ic=0;ic<ncti;ic++){

	    //add gradient of U V components
	   

	    //(inU and inV) should be real and we should use just the real component
	    //they represent a high pass filtered version of the u and v compoents
	    //Also must normalize by nati*ncti since fftw computes unnormalized DFT 
	    //magU=(float)sqrt(inU[ia+nati*ic][1]*inU[ia+nati*ic][1]+inU[ia+nati*ic][0]*inU[ia+nati*ic][0])/(1.0*natif*nctif);
	    //magV=(float)sqrt(inV[ia+nati*ic][1]*inV[ia+nati*ic][1]+inV[ia+nati*ic][0]*inV[ia+nati*ic][0])/(1.0*natif*nctif);
	    
	    //magU=(float)inU[ia*ncti+ic][0]/(1.0*natif*nctif);
	    //magV=(float)inV[ia*ncti+ic][0]/(1.0*natif*nctif);
	    magU=inU[ia*nctif+ic][0]/(1.0*natif*nctif);
	    magV=inV[ia*nctif+ic][0]/(1.0*natif*nctif);

	    //magU=inU[ia*ncti+ic][0]/(1.0*nati*ncti);
	    //magV=inV[ia*ncti+ic][0]/(1.0*nati*ncti);
	    
	    grad1U=cos(dir[ia][ic]*dtr);
	    grad1V=sin(dir[ia][ic]*dtr);
	    grad2U=-1.0*spd[ia][ic]*sin(dir[ia][ic]*dtr);
	    grad2V=spd[ia][ic]*cos(dir[ia][ic]*dtr);
	    //----------------
	    // update gradient only for valid wind cells
	    //---------------
	    flg2=int(flg[ia][ic]);
	    if (flg2==0){
	      
	      //aveH[ia][ic]=gradobj1[ia][ic];
	      //aveV[ia][ic]=gradobj2[ia][ic];

	      if (noifft==0){//do correct way
		if (SDflag==1){
		  //gradient in S D
		  gradobj1[ia][ic]-=1.0/(121.0)*(magU*grad1U+magV*grad1V);
		  gradobj2[ia][ic]-=1.0/(121.0)*(magU*grad2U+magV*grad2V);
		}else{
		  //gradient in U V
		  gradobj1[ia][ic]-=1.0/(121.0)*(magU);
		  gradobj2[ia][ic]-=1.0/(121.0)*(magV);
		}
	      }else{
	      //fake way using convoluion instead of devonvolution (i.e., approx R^-1 by (I-R))
	      //for this magU and magV are ifft of inU*magC and inV*magC instead of inU/magC and inV/magC
	      //if (noifft==1){
		if (SDflag==1){
		  //gradient in S D
		  gradobj1[ia][ic]-=1.0/(121.0)*(spd[ia][ic]*sin(dir[ia][ic]*dtr)-magU)*grad1U+(spd[ia][ic]*sin(dir[ia][ic]*dtr)-magV)*grad1V;
		  gradobj2[ia][ic]-=1.0/(121.0)*(spd[ia][ic]*sin(dir[ia][ic]*dtr)-magU)*grad2U+(spd[ia][ic]*sin(dir[ia][ic]*dtr)-magV)*grad2V;
		  //this hasnt been checked
		}else{
		  //gradient in U V
		  PW=sqrt(magU*magU+magV*magV);
		  if (PW<=1) {
		    PW=1;
		  }else{
		    PW=121.0/PW;
		  }
		  gradobj1[ia][ic]-=PW*(spd[ia][ic]*cos(dir[ia][ic]*dtr)-magU);
		  gradobj2[ia][ic]-=PW*(spd[ia][ic]*sin(dir[ia][ic]*dtr)-magV);
		}
	      }
	      
	      if(mxgradobj1<fabs(gradobj1[ia][ic])) mxgradobj1=fabs(gradobj1[ia][ic]);
	      if(mxgradobj2<fabs(gradobj2[ia][ic])) mxgradobj2=fabs(gradobj2[ia][ic]);
	      if(mxgradobjV<fabs(gradobjV[ia][ic])) mxgradobjV=fabs(gradobj1[ia][ic]);
	      if(mxgradobjH<fabs(gradobjH[ia][ic])) mxgradobjH=fabs(gradobj2[ia][ic]);
	    
	      //aveH[ia][ic]=sqrt(magU*magU+magV*magV);//1.0/(121.0)*(magU*grad1U+magV*grad1V);
	      //aveV[ia][ic]=incorr[ia*ncti+ic][0];//magV;//1.0/(121.0)*(magU*grad2U+magV*grad2V);
	    }

	    

	    //aveH[ia][ic]=(float)inU[ia*nctif+ic][0];//
	    //aveV[ia][ic]=(float)inU[ia*nctif+ic][1];//
	    //aveH[ia][ic]=(float) gradobj1[ia][ic];//1.0/(121.0)*(magU*grad1U+magV*grad1V);
	    //aveV[ia][ic]=(float) gradobj2[ia][ic];//1.0/(121.0)*(magU*grad2U+magV*grad2V);
	    


	  }//ic
	}//ia
      }//if windPrior
      

      //--------------------------------------//
      // done adding  a prior in wind domain  //
      //--------------------------------------//











      //-------------------//
      // update wind field //
      //-------------------//
      
      //first normalize gradient so that no WVC moves more than muS0, muD0
      if (1==0){
	
	//independent speed and dir scaling
      if (mxgradobj1>0)
	{
	  muS=muS0/mxgradobj1;
	}else
	{
	  muS=muS0;
	}
      if (mxgradobj2>0)
	{
	  muD=muD0/mxgradobj2;
	}else
	{
	  muD=muD0;
	}
      mxgradobj=0.0;
      /*
      //connected speed and dir scaling
     // mxgradobj=0.0;
      if (mxgradobj1>muS0||mxgradobj2>muD0) mxgradobj=fmax(mxgradobj1/muS0,mxgradobj2/muD0);
      
      if (mxgradobj>0)
	{
	  muS=muS0/mxgradobj;
	  muD=muD0/mxgradobj;
	}else
	{
	  muS=muS0;
	  muD=muD0;
	}
      */

      //printf("max speed gradient: %f max dir gradient %f norm factor %f mu %f\n",mxgradobj1,mxgradobj2,mxgradobj,mu);
      }
      //printf("max speed gradient: %f max dir gradient %f norm factor %f mu %f\n",mxgradobj1,mxgradobj2,mxgradobj,mu);

      printf("max speed gradient: %f max dir gradient %f max H-pol gradient: %f max V-pol gradient %f\n",mxgradobj1,mxgradobj2,mxgradobjH,mxgradobjV);
      //printf("max speed gradient: %f max dir gradient %f \n",mxgradobj1,mxgradobj2);
      if (muFlg==0){//Field-wise variable mu
	//variable mu (i.e., scale step size)
	muS=muS0;
	muD=muD0;
	muV=muV0;
	muH=muH0;

	if (SDflag==1){
	  while(fabs(muS * mxgradobj1) < muS0&&muS<=1.0){
	    muS=muS * 2.0;
	  }
	  while(fabs(muD * mxgradobj2) < muD0&&muD<=1.0){
	    muD=muD * 2.0;
	  }
	  while(fabs(muS * mxgradobj1) > muS0){
	    muS=muS / 2.0;
	  }
	  while(fabs(muD * mxgradobj2) > muD0){
	    muD=muD / 2.0;
	  }
	}else{//need to scale U and V together
	  //variable mu
	 
	  while(fabs(muS * mxgradobj1) < muS0&&muS<=1.0 && fabs(muD * mxgradobj2) < muD0&&muD<=1.0){
	    muS=muS * 2.0;
	    muD=muD * 2.0;
	  }
	  while(fabs(muS * mxgradobj1) > muS0||fabs(muD * mxgradobj2) > muD0){
	    muS=muS / 2.0;
	    muD=muD / 2.0;
	  }
	  
	 

	}


	while(fabs(muH * mxgradobjH) < muH0&&muH<=1.0){
	  muH=muH * 2.0;
	}
	while(fabs(muV * mxgradobjV) < muV0&&muV<=1.0){
	  muV=muV * 2.0;
	}
	while(fabs(muH * mxgradobjH) > muH0){
	  muH=muH / 2.0;
	}
	while(fabs(muV * mxgradobjV) > muV0){
	  muV=muV / 2.0;
	}
	
	
	
	printf("muS: %g muD: %g cng1: %g, cng2: %g\n",muS,muD,muS * mxgradobj1,muD * mxgradobj2);
	printf("muV: %g muH: %g cngV: %g, cngH: %g\n",muV,muH,muV * mxgradobjV,muH * mxgradobjH);
      }
      
      mxcngspeed=0;
      mxcngdir=0;
      mncngspeed=1.0e20;
      mncngdir=1.0e20;
      
      
      mxcngV=0;
      mxcngH=0;
      mncngV=1.0e20;
      mncngH=1.0e20;
      //update wind
      
      for(int ia=0;ia<nati;ia++)
	{
	  for (int ic=0;ic<ncti;ic++)
	    { 
	      //only do stuff to data WVCs that change (are hit by measurements and gradient !=0)
	      //if (gradobj1[ia][ic]!=0.0||gradobj2[ia][ic]!=0.0)
	      //if (aveV[ia][ic]==0&&aveH[ia][ic]==0){flg[ia][ic]='5';}else
	      if (1==1)//(spd[ia][ic]!=-1)
		{
		  if(muFlg==1){
		    //these next few lines are for moving each WVC in direction of 
		    //current gradient but clipping each (i.e., point-wise variable mu), 
		    //ignores above normalization
		    
		    muS=1.0;
		    muD=1.0;
		    muV=1.0;
		    muH=1.0;
		    // if (fabs(gradobj1[ia][ic])>muS0) muS=muS0/fabs(gradobj1[ia][ic]);
		    //if (fabs(gradobj2[ia][ic])>muD0) muD=muD0/fabs(gradobj2[ia][ic]);
		    //if (muS*gradobj1[ia][ic]<0&&-1.0*muS*gradobj1[ia][ic]>spd[ia][ic]) muS=spd[ia][ic]/(-2.0*gradobj1[ia][ic]);
		    
		    //variable mu pointwise (indep speed and dir scaling)
		    //flg2=int(flg[ia][ic]);
		    //if (flg2==0)
		    //printf("flg: %d, spd %f, ia: %d ic %d\n",flg2,spd[ia][ic],ia,ic);
		    //if (flg2==0){
		    if (SDflag==1){
		      while(fabs(muS * gradobj1[ia][ic]) < muS0&&muS<=1){
			muS=muS * 2.0;
		      }
		      while(fabs(muD * gradobj2[ia][ic]) < muD0&&muD<=1){
			muD=muD * 2.0;
		      }
		      while(fabs(muS * gradobj1[ia][ic]) > muS0){
			muS=muS / 2.0;
		      }
		      while(fabs(muD * gradobj2[ia][ic]) > muD0){
			muD=muD / 2.0;
		      }
		    }else{//scale U and V components together
		      while(fabs(muS * gradobj1[ia][ic]) < muS0&&muS<=1&&fabs(muD * gradobj2[ia][ic]) < muD0&&muD<=1){
			muS=muS * 2.0;
			muD=muD * 2.0;
		      }
		     
		      while(fabs(muS * gradobj1[ia][ic]) > muS0||fabs(muD * gradobj2[ia][ic]) > muD0){
			muS=muS / 2.0;
			muD=muD / 2.0;
		      }
		    }
		    if (gradobjV[ia][ic]!=0){
		      while(fabs(muV * gradobjV[ia][ic]) < muV0&&muV<=1){
			muV=muV * 2.0;
		      }
		      while(fabs(muV * gradobjV[ia][ic]) > muV0){
			muV=muV / 2.0;
		      }
		    }
		    if (gradobjH[ia][ic]!=0){
		      while(fabs(muH * gradobjH[ia][ic]) < muH0&&muH<=1){
			muH=muH * 2.0;
		      }
		      while(fabs(muH * gradobjH[ia][ic]) > muH0){
			muH=muH / 2.0;
		      }
		    }
		   
		    //}
		    /*
		    //variable mu pointwise connected speed and dir scaling
		    while(fabs(muS * gradobj1[ia][ic]) < muS0&&muS<=1){
		    muS=muS * 2.0;
		    muD=muD * 2.0;
		    }
		    while(fabs(muD * gradobj2[ia][ic]) < muD0&&muD<=1){
		    muD=muD * 2.0;
		    muS=muS * 2.0;
		    }
		    while(fabs(muS * gradobj1[ia][ic]) > muS0){
		    muS=muS / 2.0;
		    muD=muD / 2.0;
		    }
		    while(fabs(muD * gradobj2[ia][ic]) > muD0){
		    muD=muD / 2.0;
		    muS=muS / 2.0;
		    }
		    */
		  }//muFlg
		  // if (muS*gradobj1[ia][ic]<0&&-1.0*muS*gradobj1[ia][ic]>spd[ia][ic]) muS=spd[ia][ic]/(-2.0*gradobj1[ia][ic]);
		  flg2=int(flg[ia][ic]);
		  if (flg2==0){//only update valid speeds
		  if (SDflag==1){
		    //update speed
		   
		    while (spd[ia][ic]+(float) muS*gradobj1[ia][ic]<0.0) muS/=2.0;

		    spd[ia][ic]+=(float) muS*gradobj1[ia][ic];
		    if (spd[ia][ic]>50.0) {
		      spd[ia][ic]=49.9999;
		    }
		    
		    //update dir
		    dir[ia][ic]+=(float) muD*gradobj2[ia][ic];
		    while(dir[ia][ic]<0.0) dir[ia][ic]+=360.0;
		    while(dir[ia][ic]>=360.0) dir[ia][ic]-=360.0;
		    
		    
		    //update land/ice/rain sigma0
		    /*
		      aveH[ia][ic]+=muH*gradobjH[ia][ic];
		      aveV[ia][ic]+=muV*gradobjV[ia][ic];
		      if (aveH[ia][ic]<0.0) aveH[ia][ic]=1e-5;
		      if (aveV[ia][ic]<0.0) aveV[ia][ic]=1e-5;
		    */
		    
		  }else{
		    //update in U and V
		    //convert to u v
		    if (1==1){//update in U V
		      //may not handle low winds well
		      magU=spd[ia][ic]*cos(dir[ia][ic]*dtr);
		      magV=spd[ia][ic]*sin(dir[ia][ic]*dtr);
		      magU+=muS*gradobj1[ia][ic];
		      magV+=muD*gradobj2[ia][ic];
		      speed=(float) sqrt(magU*magU+magV*magV);
		      if (speed > spd[ia][ic]) sgn=1.0;
		      if (speed < spd[ia][ic]) sgn=-1.0;
		      if (abs(spd[ia][ic]-speed)<5.0){
			spd[ia][ic]=speed;
		      }else{
			spd[ia][ic]+=5.0*sgn;
		      }
		      magV=atan2(magV,magU)*rtd;
		      if (magV<0) magV+=360.0;
		      dir[ia][ic]=magV;
		      
		    }else{// convert U V gradient to S D and update 
		      //this dosnt work
		      magU=(float) muS*gradobj1[ia][ic];
		      magV=(float) muD*gradobj2[ia][ic];
		      sgn=1.0;
		      if (spd[ia][ic]*cos(dir[ia][ic]*dtr)>=0.0&&magU<0.0&&spd[ia][ic]*cos(dir[ia][ic]*dtr)<abs(magU)) sgn=-1.0;
		      if (spd[ia][ic]*cos(dir[ia][ic]*dtr)<=0.0&&magU>0.0&&-1.0*spd[ia][ic]*cos(dir[ia][ic]*dtr)<magU) sgn=-1.0;
		      if (spd[ia][ic]*sin(dir[ia][ic]*dtr)>=0.0&&magV<0.0&&spd[ia][ic]*sin(dir[ia][ic]*dtr)<abs(magV)) sgn=-1.0;
		      if (spd[ia][ic]*sin(dir[ia][ic]*dtr)<=0.0&&magV>0.0&&-1.0*spd[ia][ic]*sin(dir[ia][ic]*dtr)<magV) sgn=-1.0;
		      

		      gradobj1[ia][ic]=sgn*sqrt(magU*magU+magV*magV);
		      gradobj2[ia][ic]=atan2(magV,magU)*rtd;
		      muS=1.0;
		      muD=1.0;
		      while (spd[ia][ic]+(float) muS*gradobj1[ia][ic]<0.0) muS/=2.0;
		      
		      spd[ia][ic]+=(float) muS*gradobj1[ia][ic];
		      if (spd[ia][ic]>50.0) {
			spd[ia][ic]=49.9999;
		      }
		      
		      //update dir
		      dir[ia][ic]+=(float) muD*gradobj2[ia][ic];
		      while(dir[ia][ic]<0.0) dir[ia][ic]+=360.0;
		      while(dir[ia][ic]>=360.0) dir[ia][ic]-=360.0;
		    }
		    
		  }
		  }else{//only update sigma^0 for invalid wind cells 
		  //update land/ice/rain sigma0
		  
		  aveH[ia][ic]+=muH*gradobjH[ia][ic];
		  aveV[ia][ic]+=muV*gradobjV[ia][ic];
		  if (aveH[ia][ic]<0.0) aveH[ia][ic]=1e-5;
		  if (aveV[ia][ic]<0.0) aveV[ia][ic]=1e-5;
		  }
		  /*
		    spd[ia][ic]+=muS*gradobj1[ia][ic];
		    flg2=int(flg[ia][ic]);
		    if (flg2==0){
		    if (spd[ia][ic]>50.0) spd[ia][ic]=49.0;
		    if (spd[ia][ic]<0.0) spd[ia][ic]=1.0;
		    }else{if (spd[ia][ic]<0.0) spd[ia][ic]=1e-5;}
		    
		    //update dir
		    dir[ia][ic]+=muD*gradobj2[ia][ic];
		    if (flg2==0){
		    while(dir[ia][ic]<0.0) dir[ia][ic]+=360.0;
		    while(dir[ia][ic]>=360.0) dir[ia][ic]-=360.0;
		    }else{if (dir[ia][ic]<0.0) dir[ia][ic]=1e-5;}
		  */
		  
		  //spd[ia][ic]=muS*gradobj1[ia][ic];
		  //dir[ia][ic]=muD*gradobj2[ia][ic];
		  //spd[ia][ic]=gradobj1[ia][ic];
		  //dir[ia][ic]=(float)numMeas[ia][ic];
		  //calculate max speed/dir change
		  if(mxcngspeed<fabs(muS*gradobj1[ia][ic])) mxcngspeed=fabs(muS*gradobj1[ia][ic]);
		  if(mxcngdir<fabs(muD*gradobj2[ia][ic])) mxcngdir=fabs(muD*gradobj2[ia][ic]);
 
		  //calculate min speed/dir change
		  if(mncngspeed>fabs(muS*gradobj1[ia][ic])) mncngspeed=fabs(muS*gradobj1[ia][ic]);
		  if(mncngdir>fabs(muD*gradobj2[ia][ic])) mncngdir=fabs(muD*gradobj2[ia][ic]);
		  


		  //calculate max H/V change
		  if(mxcngH<fabs(muH*gradobjH[ia][ic])) mxcngH=fabs(muH*gradobjH[ia][ic]);
		  if(mxcngV<fabs(muV*gradobjV[ia][ic])) mxcngV=fabs(muV*gradobjV[ia][ic]);
		  
		  //calculate min H/V change
		  if(mncngH>fabs(muH*gradobjH[ia][ic])) mncngH=fabs(muH*gradobjH[ia][ic]);
		  if(mncngV>fabs(muV*gradobjV[ia][ic])) mncngV=fabs(muV*gradobjV[ia][ic]);
		  

		  /*  //on first iteration set non-wind cells to AVE 
		  if (iter==0){
		    //printf("resetting speed\n");
		    flg2=int(flg[ia][ic]);
		    if (flg2!=0){
		      spd[ia][ic]=aveV[ia][ic];
		      dir[ia][ic]=aveH[ia][ic];
		      }
		      }*/

		}//if (gradobj1[ia][ic]!=0.0||gradobj2[ia][ic]!=0.0)
	    }//ic loop
	} //ia loop
      
      printf("max change speed: %g max change dir %g\n",mxcngspeed,mxcngdir);
      printf("min change speed: %g min change dir %g\n\n",mncngspeed,mncngdir);

      printf("max change H-pol: %g max change V-pol %g\n",mxcngH,mxcngV);
      printf("min change H-pol: %g min change V-pol %g\n\n",mncngH,mncngV);
      // }//iter loop
  
    
      


  //  output every n iterations
      if ((iter+1)%5==0||iter==maxiter-1||iter==0){
  
  
  //convert back to radians
  for (int ia=0;ia<nati;ia++)
    {
      for (int ic=0;ic<ncti;ic++)
	{
	  
	  dir[ia][ic]*=dtr;
	  //printf("speed; %f dir: %f\n",spd[ia][ic],dir[ia][ic]);
	  //gradobjV[ia][ic]=incorr[ia*nctif+ic][0];
	}
    } 
  
  
  
  
  
  
  //-------------------------//
  // Write the output arrays //
  //-------------------------//
  //this outputs in same format as l2b_to_arrays
  sprintf(tmpstr,"%s_%d",outputFilename,iter+1);
  printf("writing file %s\n",tmpstr);
  //FILE* 
  wfp=fopen(tmpstr,"w");
  if(!wfp){
    fprintf(stderr,"Cannot open file %s for writing\n",tmpstr);
    exit(1);
  }
  f=first_valid_ati;
  n=nvalid_ati;
  if(fwrite(&first_valid_ati,sizeof(int),1,wfp)!=1  ||
     fwrite(&nvalid_ati,sizeof(int),1,wfp)!=1  ||
     fwrite(&ncti,sizeof(int),1,wfp)!=1  ||
     !write_array(wfp,&spd[f],sizeof(float),2,n,ncti) ||
     !write_array(wfp,&dir[f],sizeof(float),2,n,ncti) ||
     !write_array(wfp,&lat[f],sizeof(float),2,n,ncti) ||
     !write_array(wfp,&lon[f],sizeof(float),2,n,ncti) ||
     !write_array(wfp,&flg[f],sizeof(char),2,n,ncti)){
    fprintf(stderr,"Error writing to file %s\n",tmpstr);
    exit(1);
  }
  fclose(wfp);

  if (outputFilenameAVE_reconst!=NULL){
    //-------------------------//
    // Write the output arrays //
    //-------------------------//
    //this outputs in same format as l2b_to_arrays
    printf("writing file %s\n",outputFilenameAVE_reconst);
    //FILE* 
    wfp=fopen(outputFilenameAVE_reconst,"w");
    if(!wfp){
      fprintf(stderr,"Cannot open file %s for writing\n",outputFilenameAVE_reconst);
      exit(1);
    }
    f=first_valid_ati;
    n=nvalid_ati;
    if(fwrite(&first_valid_ati,sizeof(int),1,wfp)!=1  ||
       fwrite(&nvalid_ati,sizeof(int),1,wfp)!=1  ||
       fwrite(&ncti,sizeof(int),1,wfp)!=1  ||
       !write_array(wfp,&aveV[f],sizeof(float),2,n,ncti) ||
       !write_array(wfp,&aveH[f],sizeof(float),2,n,ncti) ||
       !write_array(wfp,&lat[f],sizeof(float),2,n,ncti) ||
       !write_array(wfp,&lon[f],sizeof(float),2,n,ncti) ||
       !write_array(wfp,&flg[f],sizeof(char),2,n,ncti)){
      fprintf(stderr,"Error writing to file %s\n",outputFilenameAVE_reconst);
      exit(1);
    }
    fclose(wfp);
    

    //
    //output the gradient to a file
    //
    //outputFilenameAVE_grad=strcat(outputFilenameAVE,"_grad");
    printf("writing file %s\n",outputFilenameAVE_grad);
    //FILE* 
    wfp=fopen(outputFilenameAVE_grad,"w");
    if(!wfp){
      fprintf(stderr,"Cannot open file %s for writing\n",outputFilenameAVE_grad);
      exit(1);
    }
    f=first_valid_ati;
    n=nvalid_ati;
    if(fwrite(&first_valid_ati,sizeof(int),1,wfp)!=1  ||
       fwrite(&nvalid_ati,sizeof(int),1,wfp)!=1  ||
       fwrite(&ncti,sizeof(int),1,wfp)!=1  ||
       !write_array(wfp,&gradobj1[f],sizeof(double),2,n,ncti) ||
       !write_array(wfp,&gradobj2[f],sizeof(double),2,n,ncti) ||
       !write_array(wfp,&gradobjH[f],sizeof(double),2,n,ncti) ||
       !write_array(wfp,&gradobjV[f],sizeof(double),2,n,ncti) ||
       !write_array(wfp,&flg[f],sizeof(char),2,n,ncti)){
      fprintf(stderr,"Error writing to file %s\n",outputFilenameAVE_grad);
      exit(1);
    }
    fclose(wfp);
  }


  //convert back to degrees
  for (int ia=0;ia<nati;ia++)
    {
      for (int ic=0;ic<ncti;ic++)
	{
	  
	  dir[ia][ic]/=dtr;
	  //printf("speed; %f dir: %f\n",spd[ia][ic],dir[ia][ic]);
	}
    } 

      }//if mod(iter)
    }//iter
  
  free_array(spd,2,nati,ncti);
  free_array(dir,2,nati,ncti);
  free_array(lat,2,nati,ncti);
  free_array(lon,2,nati,ncti);
  free_array(flg,2,nati,ncti);
  
  
  
  
  printf("Freeing l1b data arrays \n");
  
  //free up other arrays
  free_array(value,1,nmeas);
  free_array(inc,1,nmeas);
  free_array(az,1,nmeas);
  free_array(A,1,nmeas);
  free_array(B,1,nmeas);
  free_array(C,1,nmeas);

  free_array(numWVCs,1,nmeas);
  //printf("Here\n");
  free_array(atis,2,nmeas,maxnumWVCs);
  free_array(ctis,2,nmeas,maxnumWVCs);
 
  free_array(ap,2,nmeas,maxnumWVCs);
 //float ** ap=(float**)make_array(sizeof(float),2,nmeas,maxnumWVCs);

  printf("Freeing obj func arrays \n");
  
  free_array(gradobj1,2,nati,ncti);
  free_array(gradobj2,2,nati,ncti);
  free_array(numMeas,2,nati,ncti);

  printf("Freeing prior arrays \n");
  
  free_array(cova,1,nati);
  free_array(covc,1,ncti);

  free_array(J_uu,2,nati2,ncti2);
  free_array(J_vv,2,nati2,ncti2);
  free_array(J_uv,2,nati2,ncti2);
  free_array(J_vu,2,nati2,ncti2);
  
  printf("Freeing ave arrays \n");
 
  free_array(aveV,2,nati,ncti);
  free_array(aveH,2,nati,ncti);
  free_array(aveVcnt,2,nati,ncti);
  free_array(aveHcnt,2,nati,ncti);

  printf("Freeing fft arrays \n");
  
  fftw_destroy_plan(plnU);
  fftw_destroy_plan(plnUI);
  fftw_destroy_plan(plnV);
  fftw_destroy_plan(plnVI);
  fftw_destroy_plan(plnC);
  
  fftw_free(inU); 
  fftw_free(outU);
  fftw_free(inV); 
  fftw_free(outV);
  fftw_free(incorr); 
  fftw_free(outcorr);
  return (0);
}

