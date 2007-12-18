#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "Array.h"
#include "MLPData.h"

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
MLPData::Read(char* filename){
  FILE* ifp;
  int num_values_received=0;
  int num_values_expected, num_inpts_received=0, num_outpts_received=0;
  int num_samps_received=0;
  char* value_string;
  char* line_from_file;
  line_from_file=(char *)malloc(MAXIMUM_LINE_LENGTH*sizeof(char));
  ifp=fopen(filename,"r");
  if(!ifp) return(0);
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
  fclose(ifp);
  free(line_from_file);
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
MLPData::Write(char* filename){
  FILE* ofp;
  int c,d;
  ofp=fopen(filename,"w");
  if(!ofp) return(0);

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

#endif





























	
