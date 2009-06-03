#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "Array.h"
#include "MLPData.h"
#include "MLP.h"
#include "Array.h"

#ifndef DATA_STRUCTS_C
#define DATA_STRUCTS_C
/***####### DS_Static Functions#####*****/
/*** Constructor and Destructor***/
MLPData::~MLPData(){
  Deallocate();
}

int MLPData::Deallocate(){
  if(num_samps!=0){
    free_array(inpt,2,num_samps,num_inpts);
    free_array(outpt,2,num_samps,num_outpts);
    num_samps=0;
    return(1);
  }
  else return(0);
}

MLPData::MLPData()
  : num_inpts(0), num_outpts(0), num_samps(0){
    return;
}


/*** Read from File ***/
int 
MLPData::Read(FILE* ifp){
  int num_values_received=0;
  int num_values_expected, num_inpts_received=0, num_outpts_received=0;
  int num_samps_received=0;
  char* value_string;
  char* line_from_file;
  line_from_file=(char *)malloc(MAXIMUM_LINE_LENGTH*sizeof(char));

  /**** get header information ***/


  /*** get file type ***/
  value_string=skip_comments(ifp,line_from_file);
  value_string=strtok(NULL," :\n\t");
  if(strcmp(value_string,"STATIC")!=0){
    if(strcmp(value_string,"static")!=0){
      if(strcmp(value_string,"Static")!=0){      
	fprintf(stderr,"MLPData::Read: File Type is not STATIC!");
	return(0);
      }
    }
  }


  /*** get number of inputs ***/
  value_string=skip_comments(ifp,line_from_file);
  value_string=strtok(NULL," :\n\t");
  num_inpts=atoi(value_string);

  /*** get number of outputs ***/
  value_string=skip_comments(ifp,line_from_file);
  value_string=strtok(NULL," :\n\t");
  num_outpts=atoi(value_string);

  /*** get number of samples ***/
  value_string=skip_comments(ifp,line_from_file);
  value_string=strtok(NULL," :\n\t");
  num_samps=atoi(value_string);

  /*** allocate data structure **/
  Allocate();

  num_values_expected=(num_outpts+num_inpts)*num_samps;

  /*** read values form file and put them into data structure ***/
  while(num_values_received<num_values_expected){
    /*** get a line of data ***/
    fgets(line_from_file,MAXIMUM_LINE_LENGTH,ifp);
    
    /*** find first value in the line of data***/
    value_string=strtok(line_from_file," \n\t");

    /*** check for comment or blank lines ***/
    if (value_string==NULL){
      /*** ignore blank lines ***/
    }
    else if (value_string[0]=='\%'){
      /*** ignore comment ***/
    }
  
    /*** copy each value in the line into the data structure ***/
    else{
      while(value_string!=NULL){
	/*** Currently receiving input values ***/
        if (num_inpts_received<num_inpts){
	  inpt[num_samps_received][num_inpts_received]=
	    atof(value_string);
	  num_inpts_received++;
          num_values_received++;
	}
        /**** Currently receiving output value ***/
        else{
	  outpt[num_samps_received][num_outpts_received]=
	    atof(value_string);
          num_outpts_received++;
          num_values_received++;
          /*** check to see if a complete sample has been received **/
          if(num_outpts_received==num_outpts){
	    num_outpts_received=0;
            num_inpts_received=0;
            num_samps_received++;
	  }
	}

	/**** find next value in the line of data ***/
	value_string=strtok(NULL," \n\t");

        /*** check for comment at end of line ***/
        if (value_string !=NULL){
	  if (value_string[0]=='\%'){
	    value_string=NULL; /*** end while ***/
	  }
        }
        /*** check for extraneous values at end of line ***/
	if(num_values_received==num_values_expected && value_string!=NULL){
	  fprintf(stderr,"MLPData::Read: Too many values!!!");
	  return(0);
	}
      }
    }
  }
 free(line_from_file);
 return(1);
}
int 
MLPData::Read(char* filename){
  FILE* ifp;
  ifp=fopen(filename,"r");
  if(!ifp) return(0);
  Read(ifp);
  fclose(ifp);
  return(1);
}


int 
MLPData::WriteHeader(FILE* ofp){
    fprintf(ofp,"TYPE:STATIC\n#INPUTS:%d\n#OUTPUTS:%d\n#SAMPLES:%d\n\n",
             num_inpts,num_outpts,num_samps);
    return(1);
}

/*** Write to File ***/

int 
MLPData::Write(FILE* ofp){
  int c,d;
  /**print header***/
  WriteHeader(ofp);

  for(c=0;c<num_samps;c++){
    for(d=0;d<num_inpts;d++){
      fprintf(ofp,"%g ",inpt[c][d]);
    }
    fprintf(ofp,"   ");
    for(d=0;d<num_outpts;d++){
      fprintf(ofp,"%g ",outpt[c][d]);
    }
    fprintf(ofp,"\n");
  }
  return(1);
}

int 
MLPData::Write(char* filename){
  FILE* ofp;
  ofp=fopen(filename,"w");
  if(!ofp) return(0);
  Write(ofp);
  fclose(ofp);
  return(1);
}



/*** Allocate Structure: This function is useful for allocating 
     an data structure which is not read into a file. It takes the
     data set constants, allocates the inpt and outpt arrays, and
     fills them with zeroes****/
int 
MLPData::Allocate(){
  inpt=(float**)make_array(sizeof(float),2,num_samps,num_inpts);
  outpt=(float**)make_array(sizeof(float),2,num_samps,num_outpts);
  if(!inpt || !outpt) return(0);
  start_inpt=inpt[0];
  start_outpt=outpt[0];
  return(1);
}

/*** Randomly shuffle samples in data set ***/
/*** returns pointers in old order of data along with shuffled structure ***/
/*** if either of the parameters old_inpt_ptrs or old_outpt_ptrs is NULL ***/
/*** then the old pointers are not returned                              ***/
int 
MLPData::Shuffle(float** old_inpt_ptrs, float** old_outpt_ptrs){
  float* tmp;
  int rand_no,c;
  
  if(old_inpt_ptrs!=NULL && old_outpt_ptrs!=NULL){
    memcpy((void*)old_inpt_ptrs,(void*)inpt,
	 num_samps*sizeof(float*));
    memcpy((void*)old_outpt_ptrs,(void*)outpt,
	 num_samps*sizeof(float*));
  }
  /** swap each feature vector randomly with another feature vector ***/
  for(c=0;c<num_samps;c++){
    rand_no=(int)(num_samps*drand48());
    /*** swap inputs ***/
    tmp=inpt[c];
    inpt[c]=inpt[rand_no];
    inpt[rand_no]=tmp;
    /*** swap outputs **/
    tmp=outpt[c];
    outpt[c]=outpt[rand_no];
    outpt[rand_no]=tmp;
  }
  return(1);
}

/******** This function incomplete *******/
/*** Copy a data set ***/
int 
MLPData::Copy(MLPData* newptr){
  int c;
  int inpt_offset, outpt_offset;
  newptr->Deallocate();
  newptr->num_inpts=num_inpts;
  newptr->num_outpts=num_outpts;
  newptr->num_samps=num_samps;
  newptr->Allocate();
  memcpy((void*)newptr->inpt[0],(void*)start_inpt,
	 num_samps*num_inpts*sizeof(float));
  memcpy((void*)newptr->outpt[0],(void*)start_outpt,
	 num_samps*num_outpts*sizeof(float));
   
  /*** set up pointers to feature vectors, so that a shuffled file
       is copied in its shuffled form ***/
  inpt_offset=newptr->start_inpt - start_inpt;
  outpt_offset=newptr->start_outpt - start_outpt;
  for(c=0;c<num_samps;c++){
    newptr->inpt[c]=inpt[c]+inpt_offset;
    newptr->outpt[c]=outpt[c]+outpt_offset;
  }
  return(1);
}

/*** This function incomplete ***/
/*** Append one data set (newdata) to the end of another (store) **/
int 
MLPData::Append(MLPData* newdata){
  float **new_inpt, **new_outpt;
  int new_num_samps, inpt_offset, outpt_offset, c;
  float* new_start_inpt, *new_start_outpt;


  /*** Case in which store is empty ***/
  if(num_samps==0){
    newdata->Copy(this);
    return(0);
  }

  /*** Case of a nonempty store ***/
  else{
    new_num_samps=newdata->num_samps + num_samps;
    if(newdata->num_inpts != num_inpts){ 
      fprintf(stderr,"MLPData::Append: Number of inputs don't match.");
      return(0);
    }
    if(newdata->num_outpts !=num_outpts){ 
      fprintf(stderr,"MLPData::Append:Number of outputs don't match.");
      return(0);
    }
    /*** allocate consecutive storage for appended data ***/
    new_inpt=(float**)make_array(sizeof(float),2,new_num_samps,num_inpts);
    new_outpt=(float**)make_array(sizeof(float),2,new_num_samps,num_outpts);
    new_start_inpt=new_inpt[0];
    new_start_outpt=new_outpt[0];
    if(!new_inpt || !new_outpt) return(0);
    /*** copy store to consecutive storage ***/
    memcpy((void*)new_inpt[0],(void*)start_inpt,
	   sizeof(float)*num_samps*num_inpts);
    memcpy((void*)new_outpt[0],(void*)start_outpt,
	   sizeof(float)*num_samps*num_outpts);

    /*** copy pointers so that shuffled datasets are appended correctly ***/

    inpt_offset=new_start_inpt - start_inpt;
    outpt_offset=new_start_outpt - start_outpt;
    for(c=0;c<num_samps;c++){
      new_inpt[c]=inpt[c]+inpt_offset;
      new_outpt[c]=outpt[c]+outpt_offset;
    }

    inpt_offset=num_samps*num_inpts;
    outpt_offset=num_samps*num_outpts;

    /*** copy newdata to consecutive storage after store **/
    memcpy((void*)(new_start_inpt+inpt_offset),(void*)newdata->start_inpt,
	   sizeof(float)*newdata->num_samps*newdata->num_inpts);
    memcpy((void*)(new_start_outpt+outpt_offset),(void*)newdata->start_outpt,
	   sizeof(float)*newdata->num_samps*newdata->num_outpts);

    /*** copy pointers so that shuffled datasets are appended correctly ***/

    inpt_offset+=new_start_inpt - newdata->start_inpt;
    outpt_offset+=new_start_outpt - newdata->start_outpt;
    for(c=num_samps;c<new_num_samps;c++){
      new_inpt[c]=newdata->inpt[c-num_samps]+inpt_offset;
      new_outpt[c]=newdata->outpt[c-num_samps]+outpt_offset;
    }
    /**** Deallocate the input and output pointers in store and
          replace with pointers to the appended data ****/
    Deallocate();
    num_inpts=newdata->num_inpts;
    num_outpts=newdata->num_outpts;
    num_samps=new_num_samps;
    outpt=new_outpt;
    inpt=new_inpt;
    start_outpt=new_start_outpt;
    start_inpt=new_start_inpt;
    return(1);
  }
}


int
MLPData::Unnormalize(float* bias, float* std){
  for(int c=0;c<num_inpts;c++){
    for(int d=0;d<num_samps;d++){
      inpt[d][c]=(inpt[d][c]*std[c])+bias[c];
    }
  }
  return(1);
}

int
MLPData::Normalize(float* bias, float* std){
  return(Normalize(bias,std,num_samps));
}
int
MLPData::Normalize(float* bias, float* std, int nvalidsamps){
  for(int c=0;c<num_inpts;c++){
    bias[c]=0;
    std[c]=0;
    for(int d=0;d<nvalidsamps;d++){
      bias[c]+=inpt[d][c];
      std[c]+=inpt[d][c]*inpt[d][c];
    }
    bias[c]/=nvalidsamps;
    std[c]=(std[c]-nvalidsamps*bias[c]*bias[c])/(nvalidsamps-1);
    if(std[c]>0.001*bias[c]) std[c]=sqrt(std[c]);
    else std[c]=1;
  }
  for(int c=0;c<num_inpts;c++){
    for(int d=0;d<nvalidsamps;d++){
      inpt[d][c]=(inpt[d][c]-bias[c])/std[c];
    }
  }
  return(1);
}
/***###### End DS_Static Functions #######***/



/***###### Miscellaneous Parsing Functions #####*****/
/**** read in an array of floats skipping over comment lines and comments
      at end of lines ****/
int
get_floats_array(float* ptr, int size, FILE* ifp){
  int num_values_received=0;
  char* value_string;
  char* line_from_file; 
  line_from_file=(char *)malloc(MAXIMUM_LINE_LENGTH*sizeof(char));
 
  /*** read values form file and put them into data structure ***/
  while(num_values_received<size){
    /*** get a line of data ***/
    fgets(line_from_file,MAXIMUM_LINE_LENGTH,ifp);
    
    /*** find first value in the line of data***/
    value_string=strtok(line_from_file," \n\t");

    /*** check for comment or blank lines ***/
    if (value_string==NULL){
      /*** ignore blank lines ***/
    }
    else if (value_string[0]=='\%'){
      /*** ignore comment ***/
    }
  
    /*** copy each value in the line into the buffer ***/
    else{
      while(value_string!=NULL){
	/*** Currently receiving input values ***/
        
	ptr[num_values_received]=atof(value_string);
	num_values_received++;
	
	/**** find next value in the line of data ***/
	value_string=strtok(NULL," \n\t");

        /*** check for comment at end of line ***/
        if (value_string !=NULL){
	  if (value_string[0]=='\%'){
	    value_string=NULL; /*** end while ***/
	  }
        }
        /*** check for extraneous values at end of line ***/
	if(num_values_received==size && value_string!=NULL){
	  fprintf(stderr,"get_floats_array: Too many values!!!");
	  return(0);
	}
      }
    }
  }
  free(line_from_file);
  return(1);
}

/*** grab a lines from the file  and skip over commments and
     blank lines until token is reached ***/
/*** takes parameters:
     ifp: pointer to input file
     line_from_file: buffer for storing each line
     (returns)  
     a pointer to the first token encountered 
     After skip_comments the following syntax gets the next token
     on the current line
     value_string=strtok(NULL," :\n\t");
     " :\n\t" is a list of separator characters between tokens.
***/
char*
skip_comments(FILE* ifp,char* line_from_file){
  /*** check for comments *****/
  char* value_string;
  fgets(line_from_file,MAXIMUM_LINE_LENGTH,ifp);
  value_string=strtok(line_from_file," :\n\t"); 
  if(value_string!=NULL){
    if(value_string[0]=='\%'){
      value_string=NULL; 
    }
  } 
  while(value_string==NULL){
    fgets(line_from_file,MAXIMUM_LINE_LENGTH,ifp);
    value_string=strtok(line_from_file," :\n\t"); 
    if(value_string!=NULL){
      if(value_string[0]=='\%'){
	value_string=NULL;
      }
    }     
  }
  return(value_string);
}

int read_mlpint(FILE*ifp){
  char line_from_file[1024];
  fgets(line_from_file,1024,ifp);
  char* value_string=strtok(line_from_file," :\n\t");
  value_string=strtok(NULL," :\n\t");
  int retval=atoi(value_string);
  return(retval);
}
float read_mlpfloat(FILE*ifp){
  char line_from_file[1024];
  fgets(line_from_file,1024,ifp);
  char* value_string=strtok(line_from_file," :\n\t");
  value_string=strtok(NULL," :\n\t");
  float retval=atof(value_string);
  return(retval);
}


//------------------------------------------------------------------------
//------------------MLPDataArray Routines
//------------------------------------------------------------------------
MLPDataArray::MLPDataArray(char* datfile, char* netfile, int dim1, float min1, float max1, int dim2, float min2, float max2, int MLPindim, int MLPoutdim, int num_samps, char* namestr, int hn)
  : MLParray(NULL),size1(dim1),size2(dim2),numsamps(num_samps),idx1(0),idx2(0),sampno(0),
    min1_(min1),max1_(max1),
    min2_(min2),max2_(max2),nMLPin(MLPindim),nMLPout(MLPoutdim),nMLPhn(hn),dirpdf(false),nepochs(100),
    max_bad_epochs(10),name(namestr),moment(0.5),ssize(0.01),vss(false)
{
  datfp=fopen(datfile,"w");
  if(datfp==NULL){
    fprintf(stderr,"Cannot create file %s\n",datfile);
    exit(1);
  }
  writeHeader();
  netfp=fopen(netfile,"w");
  if(netfp==NULL){
    fprintf(stderr,"Cannot create file %s\n",netfile);
    exit(1);
  }
  writeMLPHeader();

  // allocate one MLPData object and ann array of MLPs
  latestDataSet.num_inpts=nMLPin;
  latestDataSet.num_outpts=nMLPout;
  latestDataSet.num_samps=numsamps;
  latestDataSet.Allocate();
  allocateMLPArray();
}
int
MLPDataArray::allocateMLPArray(){
  degdist = (float*) malloc(sizeof(float)*nMLPout);
  MLParray = (MLP***) make_array(sizeof(MLP*),2,size1,size2);
  if(MLParray==NULL) {
    fprintf(stderr,"Cannot Allocate MLParray");
    exit(1);
  }
  for(int i=0;i<size1;i++){
    for(int j=0;j<size2;j++){
      MLParray[i][j]= new MLP();
      MLP* m=MLParray[i][j];
      m->nin=nMLPin;
      m->nout=nMLPout;
      m->hn=nMLPhn;
      m->nhlayers=1;
      m->RandomInitialize(-1.0,1.0,nMLPin,nMLPout,nMLPhn,1323545);
     }
  }
  return(1);
}

int
MLPDataArray::SetDirPdf(){
  dirpdf=true;
  for(int c=0;c<size1;c++){
     for(int d=0;d<size2;d++){
        MLParray[c][d]->outputSigmoidFlag=1;
     }
  }
  return(1);
}

int
MLPDataArray::readMLPArray(){
  degdist = (float*) malloc(sizeof(float)*nMLPout);
  MLParray = (MLP***) make_array(sizeof(MLP*),2,size1,size2);
  if(MLParray==NULL) {
    fprintf(stderr,"Cannot Allocate MLParray");
    exit(1);
  }
  for(int i=0;i<size1;i++){
    for(int j=0;j<size2;j++){
      MLParray[i][j]= new MLP();
      MLP* m=MLParray[i][j];
      if(!m->Read(netfp)){
	fprintf(stderr,"Cannot read MLPs\n");
        exit(1);
      }
      nMLPin=m->nin;
      nMLPout=m->nout;
      nMLPhn=m->hn;

    }
  }
  return(1);
}

int MLPDataArray::writeHeader(){
  fprintf(datfp,"#MLPDataArray_Data_File\n#Size1:%d\n#Size2:%d\n#Min1:%g\n#Max1:%g\n#Min2:%g\n#Max2:%g\n",size1,size2,min1_,max1_,min2_,max2_);
  return(1);
}


int MLPDataArray::writeMLPHeader(){
  fprintf(netfp,"#MLPDataArray_Net_File\n#Size1:%d\n#Size2:%d\n#Min1:%g\n#Max1:%g\n#Min2:%g\n#Max2:%g\n",size1,size2,min1_,max1_,min2_,max2_);
  return(1);
}

MLPDataArray::~MLPDataArray()
{
  if(MLParray!=NULL){
    for(int i=0;i<size1;i++){
      for(int j=0;j<size2;j++){
	MLP* m=MLParray[i][j];
	delete m;
      }
    }
    free_array(MLParray,2,size1,size2);
    free(degdist);
    MLParray=NULL;
  }
}

// read in from files
MLPDataArray::MLPDataArray(char* datfile, char* netfile, char* netfilemode)
 : MLParray(NULL),size1(0),size2(0),numsamps(0),idx1(0),idx2(0),sampno(0),min1_(0),max1_(0),
   min2_(0),max2_(0),nMLPin(0),nMLPout(0),nMLPhn(0),nepochs(0),max_bad_epochs(0),name("")
{

  datfp=fopen(datfile,"r");
  if(datfp==NULL){
    fprintf(stderr,"Cannot open file %s\n",datfile);
    exit(1);
  }
  if(strcmp(netfilemode,"r+")!=0 && strcmp(netfilemode,"r")!=0){
    fprintf(stderr,"MLPDataArray constructor bad netfilemode\n");
    exit(1);
  }
  netfp=fopen(netfile,"r");
  if(netfp==NULL){
    fprintf(stderr,"Cannot open file %s\n",netfile);
    exit(1);
  }
  readHeader();
  readMLPHeader();
  readMLPArray();
  if(strcmp(netfilemode,"r+")==0){
    fclose(netfp);
    netfp=fopen(netfile,"r+");
    if(netfp==NULL){
      fprintf(stderr,"Cannot open file %s as r+\n",netfile);
      exit(1);
    }
    writeMLPHeader();
  }
  latestDataSet.Read(datfp);
  numsamps=latestDataSet.num_samps;
  nMLPin=latestDataSet.num_inpts;
  nMLPout=latestDataSet.num_outpts;
  fclose(datfp);  
}


// read in from neural network files  only
MLPDataArray::MLPDataArray(char* netfile)
 : MLParray(NULL),size1(0),size2(0),numsamps(0),idx1(0),idx2(0),sampno(0),min1_(0),max1_(0),
   min2_(0),max2_(0),nMLPin(0),nMLPout(0),nMLPhn(0),nepochs(0),max_bad_epochs(0),name("")
{

  datfp=NULL;
  netfp=fopen(netfile,"r");
  if(netfp==NULL){
    fprintf(stderr,"Cannot open file %s\n",netfile);
    exit(1);
  }
  readMLPHeader();
  readMLPArray();
  fclose(netfp);
}

int MLPDataArray::readHeader(){
  readGenericHeader(datfp);
  return(1);
}

int MLPDataArray::readMLPHeader(){
  readGenericHeader(netfp);
  return(1);
}

int MLPDataArray::readGenericHeader(FILE* ifp){
  char line[1024];
  fgets(line,1024,ifp);
  size1=read_mlpint(ifp);
  size2=read_mlpint(ifp);
  min1_=read_mlpfloat(ifp);
  max1_=read_mlpfloat(ifp);
  min2_=read_mlpfloat(ifp);
  max2_=read_mlpfloat(ifp);
  return(1);
}

int
MLPDataArray::addSampleInOrderAndWrite(float val1, float val2, float* inputs, float* outputs)
{
  float d1=(max1_-min1_)/size1;
  float d2=(max2_-min2_)/size2;
  int i=(int)((val1-min1_)/d1);
  int j=(int)((val2-min2_)/d2);

  // clip if out of range
  if(i<0) i=0;
  if(i>=size1) i=size1-1;
  if(j<0) j=0;
  if(j>=size2) j=size2-1;

  // check to see if we need to move to the next DataSet
  if(i!=idx1 || j!=idx2){
    // check to see if there were enough samples
    if(sampno!=numsamps){
      fprintf(stderr,"AddSample Error only %d samples were added to DataSet[%d,%d]\n",sampno,idx1,idx2);
      exit(1);
    }
    // check to see if this is the next continguous DataSet
    if( !(i==idx1 && j==idx2+1) && !(i==idx1+1 && j==0)){
      fprintf(stderr,"AddSample Error Skipped from DataSet[%d,%d] to DataSet[%d,%d]\n",idx1,idx2,i,j);
      exit(1);      
    }
    else{
      trainMLPAndWrite();
      sampno=0;
      initializeDataSet();
      idx1=i;
      idx2=j;
    }    
  } // end of next DataSet case
  for(int c=0;c<nMLPin;c++){
    latestDataSet.inpt[sampno][c]=inputs[c];
  }
  for(int c=0;c<nMLPout;c++){
    if(!dirpdf)
    latestDataSet.outpt[sampno][c]=outputs[c];
    // special case for Directional Pdf
    else{
      float step=360.0/nMLPout;
      int d=(int)outputs[0]/step;
      while(d<0) d+=nMLPout;
      d=d%nMLPout; 
      if(c==d){
	latestDataSet.outpt[sampno][c]=1;
      }
      else{
	latestDataSet.outpt[sampno][c]=0;
      }
    }
  }
  sampno++;
  return(1);
}


int
MLPDataArray::addSample(float val1, float val2, float* inputs, float* outputs)
{
  float d1=(max1_-min1_)/size1;
  float d2=(max2_-min2_)/size2;
  int i=(int)((val1-min1_)/d1);
  int j=(int)((val2-min2_)/d2);

  // clip if out of range
  if(i<0) i=0;
  if(i>=size1) i=size1-1;
  if(j<0) j=0;
  if(j>=size2) j=size2-1;

  // check to see if we need to move to the next DataSet
  if(i!=idx1 || j!=idx2){
    fprintf(stderr,"For Now MLPData::addSample only works for 1 x 1 MLP array\n");
    exit(1);
  }
    // check to see if there were enough samples
    if(sampno>=numsamps){
      fprintf(stderr,"AddSample Max numsamps exceeded\n");
      exit(1);
    }
    for(int c=0;c<nMLPin;c++){
      latestDataSet.inpt[sampno][c]=inputs[c];
    }
    for(int c=0;c<nMLPout;c++){
      if(!dirpdf)
	latestDataSet.outpt[sampno][c]=outputs[c];
      // special case for Directional Pdf
      else{
	float step=360.0/nMLPout;
	int d=(int)outputs[0]/step;
	while(d<0) d+=nMLPout;
	d=d%nMLPout; 
	if(c==d){
	  latestDataSet.outpt[sampno][c]=1;
	}
	else{
	  latestDataSet.outpt[sampno][c]=0;
	}
      }
    }
    sampno++;
    return(1);
}

int
MLPDataArray::Train(){
  if(size1!=1 || size2!=1){
    fprintf(stderr,"For now MLPDataArray::Train only works for 1x1 MLP array\n");
    exit(1);
  }
  latestDataSet.Write(datfp);
  //Debug HACK
  //latestDataSet.Deallocate();
  //latestDataSet.Read("jaxaAC_hurr4_trainset_5km.dat");
  // end HACK
  _trainMLP(sampno);
  fflush(datfp);
  return(1);
}


int
MLPDataArray::reTrain(){
  if(size1!=1 || size2!=1){
    fprintf(stderr,"For now MLPDataArray::reTrain only works for 1x1 MLP array\n");
    exit(1);
  }

  // find last valid training sample
  int sampno=0;
  for(int c=0;c<numsamps;c++){
    double sum=0;
    for(int d=0;d<nMLPin;d++){
      sum+=latestDataSet.inpt[c][d];
    }
    if(sum==0)break;
    sampno++;
  }

  // adjust MLP weights for the normalized data using in training
  MLP* m=MLParray[idx1][idx2];
  float* inbias=(float*)malloc(nMLPin*sizeof(float));
  float* instd=(float*)malloc(nMLPin*sizeof(float));
  latestDataSet.Normalize(inbias,instd,sampno);  
  m->preproc(inbias,instd);

  // unnormalize to allow for data normalization step in _trainMLP
  latestDataSet.Unnormalize(inbias,instd); 
  free(inbias);
  free(instd);
  _trainMLP(sampno);
  return(1);
}

int
MLPDataArray::_trainMLP(int num_valid_samps){
    MLP* m=MLParray[idx1][idx2];
  float* inbias=(float*)malloc(nMLPin*sizeof(float));
  float* instd=(float*)malloc(nMLPin*sizeof(float));
  latestDataSet.Normalize(inbias,instd,num_valid_samps);

  // copy weights from most relevant previous MLP
  if(idx2!=0){
   MLP* mold=MLParray[idx1][idx2-1];
   *m=*mold;
    m->preproc(inbias,instd);
  }
  else if(idx1!=0){
   MLP* mold=MLParray[idx1-1][idx2];
   *m=*mold;
   m->preproc(inbias,instd);
  }
 

  // set up train,validation and test sets
  MLPData trainset, testset, validset, resultset;
  trainset.num_inpts=latestDataSet.num_inpts;
  trainset.num_outpts=latestDataSet.num_outpts;
  trainset.num_samps=num_valid_samps/3;
  trainset.Allocate();

  validset.num_inpts=latestDataSet.num_inpts;
  validset.num_outpts=latestDataSet.num_outpts;
  validset.num_samps=num_valid_samps/3;
  validset.Allocate();

  testset.num_inpts=latestDataSet.num_inpts;
  testset.num_outpts=latestDataSet.num_outpts;
  testset.num_samps=num_valid_samps/3;
  testset.Allocate();

  resultset.num_inpts=latestDataSet.num_outpts;
  resultset.num_outpts=latestDataSet.num_outpts;
  resultset.num_samps=num_valid_samps/3;
  resultset.Allocate();

  int endsamp=3*(num_valid_samps/3);
  for(int c=0;c<endsamp;c++){
    if(c%3==0){
      for(int d=0;d<nMLPin;d++) trainset.inpt[c/3][d]=latestDataSet.inpt[c][d];
      for(int d=0;d<nMLPout;d++) trainset.outpt[c/3][d]=latestDataSet.outpt[c][d];
    }
   if(c%3==1){
      for(int d=0;d<nMLPin;d++) validset.inpt[c/3][d]=latestDataSet.inpt[c][d];
      for(int d=0;d<nMLPout;d++) validset.outpt[c/3][d]=latestDataSet.outpt[c][d];
    }
   if(c%3==2){
      for(int d=0;d<nMLPin;d++) testset.inpt[c/3][d]=latestDataSet.inpt[c][d];
      for(int d=0;d<nMLPout;d++) testset.outpt[c/3][d]=latestDataSet.outpt[c][d];
    }
  }

  MLP bestmlp = *m;
  float trainmse, testmse, validmse, trainnrms,validnrms,testnrms,checkmse;
  float bestmse=1000000000000.0;
  int num_bad_epochs=0;
  for(int c=0;c<nepochs;c++){
    if(!vss) trainset.Shuffle();
    if(!dirpdf){
      if(!vss) trainmse=m->Train(&trainset,0.5,0.01);
      else{ 
	trainmse=m->TrainVSS(&trainset,c);
      }
      validmse=m->Test(&validset, &resultset);
      testmse=m->Test(&testset, &resultset);
      checkmse=validmse;
    }
    // special dirpdf case
    else{
      trainmse=trainDirPdf(m,&trainset,&trainnrms);
      validmse=testDirPdf(m,&validset,&validnrms);
      testmse=testDirPdf(m,&testset,&testnrms);
      checkmse=validnrms;  // HACK to choose best MLP using nearest RMS
    }

    if(checkmse<bestmse){
      bestmlp=*m;
      bestmse=checkmse;
      num_bad_epochs=0;
    }
    else{
      num_bad_epochs++;
    }
    if(c%1==0 && !dirpdf)
      fprintf(stderr,"%s Network[%d,%d] Epoch %d Train MSE=%g Valid MSE=%g Test MSE=%g\n",name,idx1,idx2,c
	     ,trainmse,validmse,testmse);
    if(c%1==0 && dirpdf){
      fprintf(stderr,"%s Network[%d,%d] Epoch %d Train RMS=%g NRMS=%g Valid RMS=%g NRMS=%g Test RMS=%g NRMS=%g\n",name,idx1,idx2,c
	      ,trainmse,trainnrms,validmse,validnrms,testmse,testnrms);
    }
 
    if(num_bad_epochs>=max_bad_epochs) break;

  }

  // pick the epoch which did best on the validation set
  *m = bestmlp;
  // report Final mses
  m->postproc(inbias,instd);
  trainset.Unnormalize(inbias,instd);
  testset.Unnormalize(inbias,instd);
  validset.Unnormalize(inbias,instd);
  free(inbias);
  free(instd);
  if(!dirpdf){
    trainmse=m->Test(&trainset,&resultset);
    validmse=m->Test(&validset,&resultset);
    testmse=m->Test(&testset,&resultset);
  printf("%s Network[%d,%d] FINAL Train MSE=%g Valid MSE=%g Test MSE=%g\n",name,idx1,idx2
	     ,trainmse,validmse,testmse);
  }
  // special Directional Pdf case
  else{
    trainmse=testDirPdf(m,&trainset,&trainnrms);
    validmse=testDirPdf(m,&validset,&validnrms);
    testmse=testDirPdf(m,&testset,&testnrms);
      fprintf(stderr,"%s Network[%d,%d] FINAL Train MSE=%g NRMS=%g Valid MSE=%g NRMS=%g Test MSE=%g NRMS=%g\n",name,idx1,idx2
	      ,trainmse,trainnrms,validmse,validnrms,testmse,testnrms);
   }

  fflush(stdout);
#define DEBUG 0
  if(DEBUG){
    printf("Debugging report .................\n");
    for(int c=1;c<num_valid_samps;c+=10){
      printf("Example %d: INPUTS:",c);
      m->Forward(latestDataSet.outpt[c],latestDataSet.inpt[c]);
      for(int i=0;i<nMLPin;i++) printf("%g ",latestDataSet.inpt[c][i]);
      printf("\n          TRAINING OUTPUTS:");
      for(int i=0;i<nMLPout;i++) printf("%g ",latestDataSet.outpt[c][i]);
      printf("\n          MLP OUTPUTS     :");
      for(int i=0;i<nMLPout;i++) printf("%g ",m->outp[i]);
      printf("\n");
    }
    fflush(stdout);
  }

  m->Write(netfp);
  fflush(netfp);
  return(1);
}

int
MLPDataArray::trainMLPAndWrite(){

  latestDataSet.Write(datfp);
  _trainMLP(numsamps);
  fflush(datfp);
  return(1);
}

float
MLPDataArray::trainDirPdf(MLP* m, MLPData* d, float* nearrms){
  float rms=0;
  float nrms=0;
  *nearrms=0;
  m->moment=moment;
  m->ssize=ssize;  
  for(int c=0;c<d->num_samps;c++){
    m->Forward(d->outpt[c],d->inpt[c]);
    // compute -derivative of err with respct to each output and mse
    rms+=computeDirPdfErr(m,d->outpt[c],&nrms);
    *nearrms+=nrms*nrms;
    m->Backward(d->inpt[c]);
  }
  rms=sqrt(rms/d->num_samps);
  *nearrms=sqrt(*nearrms/d->num_samps);
  return(rms);
}

float
MLPDataArray::testDirPdf(MLP* m, MLPData* d, float* nearrms){
  float rms=0;
  *nearrms=0;
  float nrms=0;
  for(int c=0;c<d->num_samps;c++){
    m->Forward(d->outpt[c],d->inpt[c]);
    // compute -derivative of err with respct to each output and mse
    rms+=computeDirPdfErr(m,d->outpt[c],&nrms);
    *nearrms+=nrms*nrms;
  }
  rms=sqrt(rms/d->num_samps);
  *nearrms=sqrt(*nearrms/d->num_samps);
  return(rms);
}

float 
MLPDataArray::computeDirPdfErr(MLP* m, float* dout, float* nearrms){
  float totalerr=0;
  float err=0;
  float step = 360.0/nMLPout;
  float half=nMLPout/2.0;
  int ctrue=0;
  float sumout=0;  
  // find true direction
  for(int c=0;c<nMLPout;c++){
    if(dout[c]>0.999) {
      ctrue = c;
    }
  }

  // compute sum of outputs;
  for(int c=0;c<nMLPout;c++){
    sumout= sumout+ m->outp[c];
  }
  // compute total directional error
  for(int c=0;c<nMLPout;c++){
    degdist[c]=step*fabs(half-fabs(half-abs(c-ctrue)));
    totalerr+=degdist[c]*degdist[c]*m->outp[c]/sumout;
    err+=(m->outp[c]-dout[c])*(m->outp[c]-dout[c]);
  }  


  
  // compute derivative of direction error w.r.t each MLP output
  if(0){
    for(int c=0;c<nMLPout;c++){
      m->err[c]=0;
      for(int d=0;d<nMLPout;d++){
	m->err[c]+=degdist[d]*degdist[d]*m->outp[d]/(sumout*sumout);
      }
      m->err[c]-=degdist[c]*degdist[c]/sumout;
      m->err[c]/=100.0;
    }
  }
  int peaks[4];
  
  // find peaks
  *nearrms=0;
  for(int c=0;c<nMLPout;c++) degdist[c]=0;
  for(int p=0;p<4;p++){
    peaks[p]=-1;
    float maxp=0;
    // find maximum remaining peak
    for(int c=0;c<nMLPout;c++){
      if(m->outp[c]>maxp && degdist[c]==0){
	peaks[p]=c;
        maxp=m->outp[c];
      }
    }
    // exclude shoulders around peak
    if(peaks[p]!=-1){
      degdist[peaks[p]]=1;
      // positive shoulder
      for(int c=(peaks[p]+1)%nMLPout;c!=peaks[p];c=((c+1)%nMLPout)){
	int c0=(c-1)%nMLPout;
        if(m->outp[c]<=m->outp[c0]){
	  degdist[c]=1;
	}
        else{
	  break;
	}
      }


      // negative shoulder
      for(int c=(peaks[p]-1+nMLPout)%nMLPout;c!=peaks[p];c=((c-1+nMLPout)%nMLPout)){
	int c0=(c+1)%nMLPout;
        if(m->outp[c]<=m->outp[c0]){
	  degdist[c]=1;
	}
        else{
	  break;
	}
      }

    }
  }

  // Find nearest peak
  *nearrms=step*fabs(half-fabs(half-abs(peaks[0]-ctrue)));
  for(int p=1;p<4;p++){
    if(peaks[p]!=-1){
      float val=step*fabs(half-fabs(half-abs(peaks[p]-ctrue)));
      if(val<*nearrms) *nearrms=val;
    }
  }

  err=err/nMLPout;
  return(err);
  
}

int
MLPDataArray::initializeDataSet(){
  for(int c=0;c<numsamps;c++){
    for(int d=0;d<nMLPin;d++){
      latestDataSet.inpt[c][d]=0.0;
    }
    for(int d=0;d<nMLPout;d++){
      latestDataSet.outpt[c][d]=0.0;
    }
  }
  return(1);
}

MLPData&

MLPDataArray::readNextDataSet(){
  if(latestDataSet.num_inpts!=0) latestDataSet.Deallocate();
  latestDataSet.Read(datfp);
  return(latestDataSet);
}

MLP*
MLPDataArray::getMLP(float val1,float val2){

  float d1=(max1_-min1_)/size1;
  float d2=(max2_-min2_)/size2;
  int i=(int)((val1-min1_)/d1);
  int j=(int)((val2-min2_)/d2);

  // clip if out of range
  if(i<0) i=0;
  if(i>=size1) i=size1-1;
  if(j<0) j=0;
  if(j>=size2) j=size2-1;
  return(MLParray[i][j]);

}

int
MLPDataArray::Flush(){

  if(sampno!=numsamps){
      fprintf(stderr,"Flush Error only %d samples were added to DataSet[%d,%d]\n",sampno,idx1,idx2);
      exit(1);
  }
  trainMLPAndWrite();
  fclose(datfp);
  fclose(netfp);
  if(idx1!=size1-1 || idx2!=size2-1){
    fprintf(stderr,"Warning: Network %s flushed after [%d,%d]\n",name,idx1,idx2);
  }
  return(1);
}

#endif





























	
