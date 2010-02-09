#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "MLP.h"
#include "Array.h"
#include "Distributions.h"


/**** defines for types of input data ****/
// this define should equal the length of the array directly bellow it;
// be sure to update it if you change the array.
#define NUM_INPUT_TYPES         34
MLP_IOType mlp_input_type_defs[] = 
  { {"S0_MEAN_K_HH_INNER_FORE", 1},
    {"S0_MEAN_K_HH_INNER_AFT",  2},
    {"S0_MEAN_K_HH_OUTER_FORE", 3},
    {"S0_MEAN_K_HH_OUTER_AFT",  4},
    {"S0_MEAN_K_VV_INNER_FORE", 5},
    {"S0_MEAN_K_VV_INNER_AFT",  6},
    {"S0_MEAN_K_VV_OUTER_FORE", 7},
    {"S0_MEAN_K_VV_OUTER_AFT",  8},
    {"S0_MEAN_C_HH_INNER_FORE", 9},
    {"S0_MEAN_C_HH_INNER_AFT",  10},
    {"S0_MEAN_C_HH_OUTER_FORE", 11},
    {"S0_MEAN_C_HH_OUTER_AFT",  12},
    {"S0_MEAN_C_VV_INNER_FORE", 13},
    {"S0_MEAN_C_VV_INNER_AFT",  14},
    {"S0_MEAN_C_VV_OUTER_FORE", 15},
    {"S0_MEAN_C_VV_OUTER_AFT",  16},

    {"S0_STD_K_HH_INNER_FORE",  17},
    {"S0_STD_K_HH_INNER_AFT",   18},
    {"S0_STD_K_HH_OUTER_FORE",  19},
    {"S0_STD_K_HH_OUTER_AFT",   20},
    {"S0_STD_K_VV_INNER_FORE",  21},
    {"S0_STD_K_VV_INNER_AFT",   22},
    {"S0_STD_K_VV_OUTER_FORE",  23},
    {"S0_STD_K_VV_OUTER_AFT",   24},
    {"S0_STD_C_HH_INNER_FORE",  25},
    {"S0_STD_C_HH_INNER_AFT",   26},
    {"S0_STD_C_HH_OUTER_FORE",  27},
    {"S0_STD_C_HH_OUTER_AFT",   28},
    {"S0_STD_C_VV_INNER_FORE",  29},
    {"S0_STD_C_VV_INNER_AFT",   30},
    {"S0_STD_C_VV_OUTER_FORE",  31},
    {"S0_STD_C_VV_OUTER_AFT",   32},
    {"CROSS_TRACK_DISTANCE",    33},
    {"DIRTH_SPEED",             34} };

#define NUM_OUTPUT_TYPES         17
MLP_IOType mlp_output_type_defs[] = 
  { {"S0_CORR_K_HH_INNER_FORE", 1},
    {"S0_CORR_K_HH_INNER_AFT",  2},
    {"S0_CORR_K_HH_OUTER_FORE", 3},
    {"S0_CORR_K_HH_OUTER_AFT",  4},
    {"S0_CORR_K_VV_INNER_FORE", 5},
    {"S0_CORR_K_VV_INNER_AFT",  6},
    {"S0_CORR_K_VV_OUTER_FORE", 7},
    {"S0_CORR_K_VV_OUTER_AFT",  8},
    {"S0_CORR_C_HH_INNER_FORE", 9},
    {"S0_CORR_C_HH_INNER_AFT",  10},
    {"S0_CORR_C_HH_OUTER_FORE", 11},
    {"S0_CORR_C_HH_OUTER_AFT",  12},
    {"S0_CORR_C_VV_INNER_FORE", 13},
    {"S0_CORR_C_VV_INNER_AFT",  14},
    {"S0_CORR_C_VV_OUTER_FORE", 15},
    {"S0_CORR_C_VV_OUTER_AFT",  16},
    {"WIND_SPEED",              17} };


MLP::MLP()
  : nin(0), nout(0), hn(0), outputSigmoidFlag(0),htab(NULL)
{
  return;
}

MLP::MLP(const MLP& m)
  : nin(m.nin), nout(m.nout), hn(m.hn), outputSigmoidFlag(m.outputSigmoidFlag),htab(NULL)
{
  Allocate();
  for(int c=0;c<nout;c++){
    for(int d=0;d<hn+1;d++){
      whid[c][d]=m.whid[c][d];
    }
  }

  for(int c=0;c<hn;c++){
    for(int d=0;d<nin+1;d++){
      win[c][d]=m.win[c][d];
    }
  }
}

// modifies weights to work on unnormalized data set
int
MLP::postproc(float* bias, float* std){
  for(int c=0;c<hn;c++){
    for(int d=0;d<nin;d++){
      win[c][nin]-=win[c][d]*bias[d]/std[d];
      win[c][d]/=std[d];
    }
  }
  return(1);
}

int
MLP::preproc(float* bias, float* std){
  for(int c=0;c<hn;c++){
    for(int d=0;d<nin;d++){
      win[c][d]*=std[d];
      win[c][nin]+=win[c][d]*bias[d]/std[d];
    }
  }
  return(1);
}
MLP
MLP::operator=(const MLP& m)
{
  Deallocate();
  nin=m.nin;
  nout=m.nout;
  hn=m.hn;
  outputSigmoidFlag=m.outputSigmoidFlag;
  Allocate();
  for(int c=0;c<nout;c++){
    for(int d=0;d<hn+1;d++){
      whid[c][d]=m.whid[c][d];
      dwhid[c][d]=m.dwhid[c][d]; 
    }
  }

  for(int c=0;c<hn;c++){
    for(int d=0;d<nin+1;d++){
      win[c][d]=m.win[c][d];
      dwin[c][d]=m.dwin[c][d];
    }
  }
  return(*this);
}


MLP::~MLP(){
  Deallocate();
}
int
MLP::Deallocate(){  
  if(nin!=0){
    free_array((void*)win,2,hn,nin+1);
    free_array((void*)dwin,2,hn,nin+1);
    free_array((void*)whid,2,nout,hn+1);
    free_array((void*)dwhid,2,nout,hn+1);
    free_array((void*)outp,1,nout);
    free_array((void*)err,1,nout);
    free_array((void*)hnout,1,hn);
    free_array((void*)herr,1,hn);
    free(in_types);
    free(out_types);
    nin=0;
    nout=0;
    hn=0;
  }
  if(htab!=NULL) free_array((void*)htab,2,hn);
  return(1);
}

/*** randomly initialize an MLP ****/
int MLP::RandomInitialize(float range_min, float range_max, int num_inputs, 
	      int num_outputs, int num_hidden_units, long int seed){
  int c,d;

  nin=num_inputs;
  hn=num_hidden_units;
  nout=num_outputs;
  if(!Allocate()) return(0);
  
  if(range_max<=range_min) return(0);

  Uniform rand_gen(0.5*(range_max-range_min),0.5*(range_max+range_min));
  if(seed!=0) rand_gen.SetSeed(seed);
  for(c=0;c<nout;c++){
    for(d=0;d<hn+1;d++){
      whid[c][d]=rand_gen.GetNumber();
    }
  }

  for(c=0;c<hn;c++){
    for(d=0;d<nin+1;d++){
      win[c][d]=rand_gen.GetNumber();
    }
  }
  return(1);
}




/*** allocate an MLP ****/
int MLP::Allocate(){
  win=(float**)make_array(sizeof(float),2,hn,nin+1);
  dwin=(float**)make_array(sizeof(float),2,hn,nin+1);
  whid=(float**)make_array(sizeof(float),2,nout,hn+1);
  dwhid=(float**)make_array(sizeof(float),2,nout,hn+1);
  outp=(float*)make_array(sizeof(float),1,nout);
  err=(float*)make_array(sizeof(float),1,nout);
  herr=(float*)make_array(sizeof(float),1,hn);
  hnout=(float*)make_array(sizeof(float),1,hn);
  in_types=(MLP_IOType*)malloc(nin*sizeof(MLP_IOType));
  out_types=(MLP_IOType*)malloc(nout*sizeof(MLP_IOType));
  train_set_str[0] = '\0';
  moment=0;
  ssize=0;
  if( win && dwin && whid && dwhid && outp && err && herr && hnout && in_types)
    return(1);
  else return(0);
}

/** function to locate the given type_str in the IO type defs, and set either in_types
    or out_types accordingly. must take in a pointer to the type defs and 
    the types buffer (either a pointer to in_types or out_types) **/
int MLP::setIOTypeByString(MLP_IOType *io_type_buf, MLP_IOType *io_type_defs, int num_io_type_defs, char *type_str, int input_idx) {
    for(int type_idx = 0; type_idx < num_io_type_defs; type_idx++) {
        if(!strcasecmp(type_str, io_type_defs[type_idx].str)) {
            io_type_buf[input_idx] = io_type_defs[type_idx];
            return(1);
        }
    }
    
    fprintf(stderr, "MLP::setInputTypeByString: Error: %s is not a recognized input/output type. See MLP.h for allowed types\n", 
        type_str);
    exit(1);
}

/** Set the nth input to be the specified string **/
int MLP::setInputTypeByString(char *type_str, int input_idx){
    if (input_idx >= nin) {
        fprintf(stderr, "MLP::setInputTypeByString: Error: input_idx %d is greater than the number of inputs.\n",
            input_idx);
        exit(1);
    }
    
    return setIOTypeByString(in_types, mlp_input_type_defs, NUM_INPUT_TYPES, type_str, input_idx);
}

/** Set the input types from a list of strings. There must be exactly
    1 string for each input, in the same order as the inputs,
    and the number of inputs (nin) must have already been set.
    (note the function name has 'typeS' rather than 'type') **/
int MLP::setInputTypesByString(char type_strs[][IO_TYPE_STR_MAX_LENGTH]){
    for(int type_str_idx = 0; type_str_idx < nin; type_str_idx++) {
        setInputTypeByString(type_strs[type_str_idx], type_str_idx);
    }
    return(1);
}

/** Set the nth output to be the specified string **/
int MLP::setOutputTypeByString(char *type_str, int input_idx){
    if (input_idx >= nout) {
        fprintf(stderr, "MLP::setOutputTypeByString: Error: input_idx %d is greater than the number of outputs.\n",
            input_idx);
        exit(1);
    }
    
    return setIOTypeByString(out_types, mlp_output_type_defs, NUM_OUTPUT_TYPES, type_str, input_idx);
}

/** Set the output types from a list of strings. There must be exactly
    1 string for each output, in the same order as the outputs,
    and the number of outputs (nout) must have already been set.
    (note the function name has 'typeS' rather than 'type') **/
int MLP::setOutputTypesByString(char type_strs[][IO_TYPE_STR_MAX_LENGTH]){
    for(int type_str_idx = 0; type_str_idx < nout; type_str_idx++) {
        setOutputTypeByString(type_strs[type_str_idx], type_str_idx);
    }
    return(1);
}


int MLP::setTrainSetString(char *train_set_str_) {
    strncpy(train_set_str, train_set_str_, TRAIN_SETS_DESC_MAX_LENGTH);
    // null terminate incase train_set_str_ is longer than TRAIN_SETS_DESC_MAX_LENGTH
    train_set_str[TRAIN_SETS_DESC_MAX_LENGTH] = '\0';
    
    return(1);
}




/** train MLP ***/
float MLP::Train(MLPData* pattern,  
	    float moment_value, float ssize_value){
  int num_patterns,num_inputs,num_outputs,c,fvno;
  float sum;  


  /*** assign learning parameters ***/
  moment=moment_value;
  ssize=ssize_value;

  /*** get constants ***/
  num_patterns=pattern->num_samps;
  num_outputs=pattern->num_outpts;
  num_inputs=pattern->num_inpts;

  if(num_outputs!=nout){
    fprintf(stderr,"MLP::Train: Error nout mismatch\n");
    exit(1);
  }
  if(num_inputs!=nin){
    fprintf(stderr,"MLP::Train: Error nin mismatch\n");
    exit(1);
  }

  /*** loop through feature vectors ***/
  fvno=0;
  sum=0;
  
  for(c=0;c<num_patterns;c++){
    sum=sum+ForwardMSE(pattern->inpt[c], pattern->outpt[c]);
      
    if(!Backward(pattern->inpt[c])){
      fprintf(stderr,"MLP::Train: Backward Pass failed.\n");
      exit(1);
    }
    fvno++;
  }
  sum/=fvno;
  return(sum);
}



/** train MLP using Variable Step Search ***/
float MLP::TrainVSS(MLPData* pattern, int epochno){
  int num_patterns,num_inputs,num_outputs,fvno;
  float sum=0;  

  /*** assign learning parameters ***/
  float d0=0.25;
  float c1=0.35;
  float h=2.0;
  int nmax = 4; // recommended value
  nmax=20; // my choice
  float c2=1;
  float pruneratio = 0.0001;
  /*** get constants ***/
  num_patterns=pattern->num_samps;
  num_outputs=pattern->num_outpts;
  num_inputs=pattern->num_inpts;

  if(num_outputs!=nout){
    fprintf(stderr,"MLP::Train: Error nout mismatch\n");
    exit(1);
  }
  if(num_inputs!=nin){
    fprintf(stderr,"MLP::Train: Error nin mismatch\n");
    exit(1);
  }

  if(epochno==0) {
    VSSInit(d0,num_patterns);
  }

  // Update hidden output table
  for(int j=0;j<hn;j++){
    UpdateHiddenTable(pattern,j);
  }


  /*** Update each parameter in input layer ***/
  if(nout!=1){
    fprintf(stderr,"TrainVSS: Only work for nout=1 for now!\n");
    exit(1);
  }
  float *ptr, *dptr;
  for(int c=0;c<hn;c++){
     ptr=&(whid[0][c]);
     dptr=&(dwhid[0][c]);
     // BWS Addition 2
     // prune unuseful nodes and replace with random ones
     if(epochno>0){
       VSSPruneIfNecessary(pattern,c,pruneratio,d0*(1-exp(-c2/epochno)));
       float ntry=5;
       float wtry=d0;
       while(whid[0][c]==0 && ntry>0){
	 VSSUpdateParam(pattern,ptr,dptr,-1,wtry,c1,c2,h,nmax,epochno);
         ntry--;
         wtry/=10.0;
       }
     }
     if(whid[0][c]==0) continue;
     for(int d=0;d<nin+1;d++){
       ptr=&(win[c][d]);
       dptr=&(dwin[c][d]);
       VSSUpdateParam(pattern,ptr,dptr,c,d0,c1,c2,h,nmax,epochno);
     }
  }
  ptr=&(whid[0][hn]);
  dptr=&(dwhid[0][hn]);
  VSSUpdateParam(pattern,ptr,dptr,-1,d0,c1,c2,h,nmax,epochno); 

  // compute final MSE
  sum=0;
  fvno=0;
  for(int c=0;c<num_patterns;c++){
    sum=sum+ForwardMSE(pattern->inpt[c], pattern->outpt[c]);
      
    fvno++;
  }
  sum/=fvno;
  return(sum);
}

int MLP::VSSPruneIfNecessary(MLPData* pattern,int hnum, float p, float d0){
  // need to fix this to handle multiple outputs if desired
  if(nout!=1){
    fprintf(stderr,"VSSPruneIfNecessary: Nout must be 1 for now!\n");
    exit(1);
  }
  float wold = whid[0][hnum];
  float OE=GetVSSError(pattern);
  whid[0][hnum]=0;
  UpdateHiddenTable(pattern,hnum);
  float PE=GetVSSError(pattern);

  // don't prune useful hidden nodes
  float dE=(PE-OE)/OE;
  if(dE > p){
    whid[0][hnum]=wold;
    UpdateHiddenTable(pattern,hnum);
  }
  // prune unuseful hidden nodes and replace with random ones
  else{
    fprintf(stderr,"Reinitializing useless hidden node %d, error cahneg will be %g percent\n",hnum,dE*100);
    VSSReinitNode(hnum,d0);
    dwhid[0][hnum]=d0;
    UpdateHiddenTable(pattern,hnum);
  }
  return(1);
}

int MLP::VSSReinitNode(int hnum, float d0){
  Uniform rand_gen(1,0);
  rand_gen.SetSeed(13467578);
  for(int d=0;d<nin+1;d++){
    win[hnum][d]=rand_gen.GetNumber();
    dwin[hnum][d]=d0;
  }
  return(1);
}

/*** reinitialize an MLP for VSS training****/
int MLP::VSSInit(float d0, int num_patterns){
  int c,d;


  for(c=0;c<nout;c++){
    for(d=0;d<hn+1;d++){
      dwhid[c][d]=d0;
    }
  }

  for(c=0;c<hn;c++){
    for(d=0;d<nin+1;d++){
      dwin[c][d]=d0;
    }
  }

  htab=(float**)make_array(sizeof(float),2,hn,num_patterns);

  return(1);
}

int MLP::UpdateHiddenTable(MLPData* pattern, int hnum){
  for(int c=0;c<pattern->num_samps;c++){
    float sum=0;
    for(int d=0;d<nin;d++){
      sum+=pattern->inpt[c][d]*win[hnum][d];
    }
    /*** add threshold **/
    sum+=win[hnum][nin];
    /*** perform sigmoid ***/
    htab[hnum][c]=1/(1+exp(-sum));
  }
  return(1);
}

float MLP::GetVSSError(MLPData* pattern){
  float sum=0;
  for(int c=0;c<pattern->num_samps;c++){
    float y=0;
    if(nout!=1){
      fprintf(stderr,"Error:VSS Only works for single outputs at present\n");
      exit(1);
    }
    for(int d=0;d<hn;d++){
     y+=htab[d][c]*whid[0][d];
    }
    y+=whid[0][hn];
    if (outputSigmoidFlag) y=1/(1+exp(-y));
    float err = y-pattern->outpt[c][0];
    sum+=(err*err);
  }
  sum/=pattern->num_samps;
  return(sum);
}
int MLP::VSSUpdateParam(MLPData* pattern,float* w, float* dw,int hnum,
			float d0,float c,float c2,float h,int nmax,int epochno){
  float wold=*w;
  float dwold=*dw;
  int VUPDEBUG=1;

  // FlowChart1

  if(dwold==0){
     //FlowChart3
    *dw=d0*(1-exp(-c2/epochno));
    // BWS Addition 1:  Don't update weights that are obviously frozen
    if(*dw<0.01){
      *dw=dwold;
      return(1);
    }
  }
  else{
    //FlowChart2
    *dw=c*dwold;
  }


  // FlowChart4
  float OE=GetVSSError(pattern);
  *w=wold+*dw;
  if(hnum!=-1) UpdateHiddenTable(pattern,hnum);
  float NE=GetVSSError(pattern);
  if(NE>=OE){

    // FlowChart5
    *dw=-(*dw);
    *w=wold+*dw;
    if(hnum!=-1) UpdateHiddenTable(pattern,hnum);
    NE=GetVSSError(pattern);
    // FlowChart6
    if(NE>=OE){
      *dw=0;
      *w=wold;
      if(hnum!=-1) UpdateHiddenTable(pattern,hnum);
      return(1);
    }
  }

  //FlowChart7
  int n=1;
  while(n<=nmax){
    *dw=h*(*dw);
    OE=NE;
    *w+=*dw;
    if(hnum!=-1) UpdateHiddenTable(pattern,hnum);
    NE=GetVSSError(pattern);
    if(NE>=OE){
      *w=*w-*dw;
      *dw=*dw/h;
      if(hnum!=-1) UpdateHiddenTable(pattern,hnum); 
      NE=OE;
      break;
    }
    n++;
  }
  if(VUPDEBUG) fprintf(stderr,"VUPDEBUG: MSE=%g\n",NE);
  return(1);
}
/** test MLP **/
float MLP::Test(MLPData* pattern, 
	   MLPData* results){
  int num_patterns,num_inputs,num_outputs,c,e,fvno;
  float sum;



  /*** get constants ***/
  num_patterns=pattern->num_samps;
  num_outputs=pattern->num_outpts;
  num_inputs=pattern->num_inpts;

  if(num_outputs!=nout){
    fprintf(stderr,"MLP::Test: Error nout mismatch\n");
    exit(1);
  }
  if(num_inputs!=nin){
    fprintf(stderr,"MLP::Test: Error nin mismatch\n");
    exit(1);
  }

  /**** Results must be the right size to hold the results ***/
  /**** num_inpts=num_outpts *****/

  if(results->num_samps != num_patterns ||
     results->num_outpts != num_outputs ||
     results->num_inpts != num_outputs){
    fprintf(stderr,"MLP::Test: Results object incorrectly sized\n");
    exit(1);
  }

  /*** loop through feature vectors ***/
  fvno=0;
  sum=0;
  for(c=0;c<num_patterns;c++){
   
    sum=sum+ForwardMSE(pattern->inpt[c], pattern->outpt[c]);
    for(e=0;e<nout;e++){
      results->inpt[c][e]=outp[e];
      results->outpt[c][e]=pattern->outpt[c][e];
    }
    fvno++;  
  }
  sum/=fvno;
  return(sum);
}

/*** perform one forward pass ***/
int MLP::Forward(float* inpts){
  int c,d;
  float sum;

  /*** calculate hidden node outputs ***/
  for(c=0;c<hn;c++){
    sum=0;
    for(d=0;d<nin;d++){
      sum+=inpts[d]*win[c][d];
    }
    /*** add threshold **/
    sum+=win[c][nin];
    /*** perform sigmoid ***/
    hnout[c]=1/(1+exp(-sum));
  }

  /*** calculate outputs ***/
  for(c=0;c<nout;c++){
    sum=0;
    for(d=0;d<hn;d++){
      sum+=hnout[d]*whid[c][d];
    }
    /*** add threshold **/
    sum+=whid[c][hn];
    /*** perform sigmoid ***/
    if (outputSigmoidFlag) outp[c]=1/(1+exp(-sum));
    else  outp[c]=sum;
  }
  return(1);
}
  
/*** perform one forward pass and calculate MSE ***/
float MLP::ForwardMSE(float* inpts, float* dout){
  int c;
  float sum;

  Forward(inpts);
  /*** calculate MSE and errs ***/
  sum=0;

  int nvalid=0;
  for(c=0;c<nout;c++){ 
    err[c]=(dout[c]-outp[c]);
    nvalid++;
    sum+=err[c]*err[c];
  }
  sum=sum/nvalid;
  return(sum);
}

int MLP::Backward(float* inpts){
   int c,d;
  
   
   /*** compute output sigmoid derivatives ***/
   if(outputSigmoidFlag){
     for(c=0;c<nout;c++) err[c]*=outp[c]*(1-outp[c]);
   }
   /**** clear herr ***/
   for(c=0;c<hn;c++) herr[c]=0;
   
   /*** calculate herr ***/
   for(c=0;c<nout;c++){
     for(d=0;d<hn;d++){
       herr[d]+=err[c]*whid[c][d];
     }
   }

   /*** calculate herr at first sigmoid **/
   for(c=0;c<hn;c++) 
     herr[c]*=hnout[c]*(1-hnout[c]);
   

   /*** compute standard deviations of errors ***/
   /***
   float stdoutbias=0,stdhidbias=0,stdhid=0,stdinp=0;
   for(c=0;c<nout;c++){
     stdoutbias+=err[c]*err[c];
   }
   stdoutbias=sqrt(stdoutbias/nout);

   for(c=0;c<nout;c++){
     for(d=0;d<hn;d++){
       stdhid+=err[c]*hnout[d]*err[c]*hnout[c];
     }
   }
   stdhid=sqrt(stdhid/(nout*hn));

   for(c=0;c<hn;c++){
     stdhidbias+=herr[c]*herr[c];
   }
   stdhidbias=sqrt(stdhidbias/hn);

   for(c=0;c<hn;c++){
     for(d=0;d<nin;d++){
       stdinp+=herr[c]*inpts[d]*herr[c]*inpts[d]; 
      }
   }
   stdinp=sqrt(stdinp/(nin*hn));
   ***/
   /*** update output thresholds ***/  
   for(c=0;c<nout;c++){
     dwhid[c][hn]*=moment;
     dwhid[c][hn]+=ssize*err[c];
     whid[c][hn]+=dwhid[c][hn];
   }

   /*** update remaining whid ****/
   for(c=0;c<nout;c++){
     for(d=0;d<hn;d++){
       dwhid[c][d]*=moment;
       dwhid[c][d]+=ssize*err[c]*hnout[d];
       whid[c][d]+=dwhid[c][d];
     }
   }

   /*** update hidden thresholds ***/
   for(c=0;c<hn;c++){
     dwin[c][nin]*=moment;
     dwin[c][nin]+=ssize*herr[c];
     win[c][nin]+=dwin[c][nin];
   }

   /*** update remaining whid ****/
   for(c=0;c<hn;c++){
     for(d=0;d<nin;d++){
       dwin[c][d]*=moment;
       dwin[c][d]+=ssize*herr[c]*inpts[d]; 
       win[c][d]+=dwin[c][d];
     }
   }
   return(1);
}



int MLP::WriteHeader(FILE* ofp){
  fprintf(ofp,"TYPE: MLP\n");
  fprintf(ofp,"#LAYERS: 2\n#INPUTS: %d\n#HIDDENS: %d\n#OUTPUTS: %d\n",
	  nin,hn,nout);	  
  // input types header
  fprintf(ofp,"#INPUT_TYPES:");
  for(int in_i=0; in_i < nin; in_i++)
    fprintf(ofp, " %s", in_types[in_i].str);
  fprintf(ofp,"\n");
  // output types header
  fprintf(ofp,"#OUTPUT_TYPES:");
  for(int out_i=0; out_i < nout; out_i++)
    fprintf(ofp, " %s", out_types[out_i].str);
  fprintf(ofp,"\n");
  // description of the training set
  fprintf(ofp,"#TRAINING_SET_STR: %s\n", train_set_str);
  return(1);
}

int MLP::Write(FILE* ofp){
  int c,d;
  if(!WriteHeader(ofp)) return(0);
  fprintf(ofp,"%c input to hidden weights:\n",'%');
  fprintf(ofp,"%c each row is a hidden unit, each (but the last) column \n",
	  '%');
  fprintf(ofp,"%c  is an input unit and a bias term is the last column. \n",
	  '%');
  for(c=0;c<hn;c++){
    for(d=0;d<nin+1;d++){
      fprintf(ofp,"%g ",win[c][d]);
    }
    fprintf(ofp,"\n");
  }
  fprintf(ofp,"%c hidden to output weights:\n",'%');
  fprintf(ofp,"%c each row is a output unit, each (but the last) column \n",
	  '%');
  fprintf(ofp,"%c  is an hidden unit and a bias term is the last column. \n",
	  '%');
  for(c=0;c<nout;c++){
    for(d=0;d<hn+1;d++){
      fprintf(ofp,"%g ",whid[c][d]);
    }
    fprintf(ofp,"\n");
  }
  return(1);
}
int MLP::Write(char* filename){
  FILE* ofp;
  ofp=fopen(filename,"w");
  if(!ofp) return(0);
  if(!Write(ofp)) return(0);
  fclose(ofp);
  return(1);
}

int MLP::WriteBin(char* filename){
  FILE* ofp;
  ofp=fopen(filename,"w");
  if(!ofp) return(0);
  if(!WriteBin(ofp)) return(0);
  fclose(ofp);
  return(1);
}

int MLP::WriteBin(FILE* ofp){
   if( fwrite(&nin,sizeof(int),1,ofp)!=1 ||
       fwrite(&nout,sizeof(int),1,ofp)!=1 ||
       fwrite(&hn,sizeof(int),1,ofp)!=1 ){
     return(0);
   }
   for(int c=0;c<hn;c++){
     if( fwrite(&win[c][0],sizeof(float),nin+1,ofp)!=(unsigned int)(nin+1))
       return(0);
   }
   for(int c=0;c<nout;c++){
     if( fwrite(&whid[c][0],sizeof(float),hn+1,ofp)!=(unsigned int)(hn+1))
       return(0);
   }
   return(1);
}

/*** write MLP weights including previous weight updates ****/
int MLP::WriteDelta(char* filename){
  FILE* ofp;
  int c,d;
  ofp=fopen(filename,"w");
  if(!ofp) return(0);
  if(!Write(ofp)) return(0);

  fprintf(ofp,"%c input to hidden weights updates:\n",'%');
  fprintf(ofp,"%c each row is a hidden unit, each (but the last) column \n",
	  '%');
  fprintf(ofp,"%c  is an input unit and a bias term is the last column. \n",
	  '%');
  for(c=0;c<hn;c++){
    for(d=0;d<nin+1;d++){
      fprintf(ofp,"%g ",dwin[c][d]);
    }
    fprintf(ofp,"\n");
  }
  fprintf(ofp,"%c hidden to output weights updates:\n",'%');
  fprintf(ofp,"%c each row is a output unit, each (but the last) column \n",
	  '%');
  fprintf(ofp,"%c  is an hidden unit and a bias term is the last column. \n",
	  '%');
  for(c=0;c<nout;c++){
    for(d=0;d<hn+1;d++){
      fprintf(ofp,"%g ",dwhid[c][d]);
    }
    fprintf(ofp,"\n");
  }
  fclose(ofp);
  return(1);
}

int MLP::Read(FILE* ifp){
  if(!ReadHeader(ifp))return(0);

  /*** allocate data structure **/
  Allocate();

  /**** get weights between input and hidden units ****/
  for(int c=0;c<hn;c++) get_floats_array(win[c],(nin+1),ifp);

  /**** get weights between hidden and outputs units ****/
  for(int c=0;c<nout;c++) get_floats_array(whid[c],(hn+1),ifp);
  return(1);
}
int MLP::Read(char* filename){
  FILE* ifp;

  ifp=fopen(filename,"r");
  if(!ifp) return(0);
  if(!Read(ifp)) return(0);
  fclose(ifp);
  return(1);
}

int MLP::ReadBin(char* filename){
  FILE* ifp;

  ifp=fopen(filename,"r");
  if(!ifp) return(0);
  if(!ReadBin(ifp)) return(0);
  fclose(ifp);
  return(1);
}

int MLP::ReadBin(FILE* ifp){
   if( fread(&nin,sizeof(int),1,ifp)!=1 ||
       fread(&nout,sizeof(int),1,ifp)!=1 || 
       fread(&hn,sizeof(int),1,ifp)!=1 ){
     return(0);
   }
   if(!Allocate()) return(0);
   for(int c=0;c<hn;c++){
     if( fread(&win[c][0],sizeof(float),nin+1,ifp)!=(unsigned int)(nin+1))
       return(0);
   }
   for(int c=0;c<nout;c++){
     if( fread(&whid[c][0],sizeof(float),hn+1,ifp)!=(unsigned int)(hn+1))
       return(0);
   }
   return(1);
}

int MLP::ReadHeader(FILE* ifp){
  char* value_string;
  char* line_from_file;
  line_from_file=(char *)malloc(MAXIMUM_LINE_LENGTH*sizeof(char));
  /**** get header information ***/


  /*** get file type ***/
  value_string=skip_comments(ifp,line_from_file);
  value_string=strtok(NULL," :\n\t");
  if(strcasecmp(value_string,"MLP")!=0){
	fprintf(stderr,"MLP::Read: File Type is not MLP!");
	return(0);
  }

  /*** get number of layers if available and then number of inputs***/
  value_string=skip_comments(ifp,line_from_file);
  int layers_listing=1;
  if(strcasecmp(value_string,"#LAYERS")!=0){
	layers_listing=0;
	/*** get number of inpts ***/
	value_string=strtok(NULL," :\n\t");
	nin=atoi(value_string);
  }
  if(layers_listing==1){
    value_string=strtok(NULL," :\n\t");
    int num_layers=atoi(value_string);
    if(num_layers==2){
      /*** get number of inpts ***/
      value_string=skip_comments(ifp,line_from_file);
      value_string=strtok(NULL," :\n\t");
      nin=atoi(value_string);
    }
    else{
      fprintf(stderr,"MLP::Read: sorry only 2 layer MLPs implemented \n");
      return(0);
    }
  }

  /**** get number of hidden units ****/
  value_string=skip_comments(ifp,line_from_file);
  value_string=strtok(NULL," :\n\t");
  hn=atoi(value_string);

  /**** get number of output units ***/  
  value_string=skip_comments(ifp,line_from_file);
  value_string=strtok(NULL," :\n\t");
  nout=atoi(value_string);
  
  /**** get the type of inputs ****/
  value_string=skip_comments(ifp,line_from_file);
  for(int in_i=0; in_i < nin; in_i++) {
    value_string=strtok(NULL," :\n\t");
    setInputTypeByString(value_string, in_i);
  }

  /**** get the type of outputs ****/
  value_string=skip_comments(ifp,line_from_file);
  for(int out_i=0; out_i < nout; out_i++) {
    value_string=strtok(NULL," :\n\t");
    setOutputTypeByString(value_string, out_i);
  }
    
  /**** get training set string ***/  
  value_string=skip_comments(ifp,line_from_file);
  value_string=strtok(NULL,"\n");
  setTrainSetString(value_string);
  
  return(1);
}
/**** reads and MLP file with included weight updates *****/
int MLP::ReadDelta(char* filename){
  FILE* ifp;

  ifp=fopen(filename,"r");
  if(!ifp) return(0);

  if(!Read(ifp)) return(0);
  /**** get weights between input and hidden units ****/
  for(int c=0;c<hn;c++) get_floats_array(dwin[c],(nin+1),ifp);

  /**** get weights between hidden and outputs units ****/
  for(int c=0;c<nout;c++) get_floats_array(dwhid[c],(hn+1),ifp);
  return(1);
}





