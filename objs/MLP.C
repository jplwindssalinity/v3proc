#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "MLP.h"
#include "Array.h"
#include "Distributions.h"

MLP::MLP()
  : nin(0), nout(0), hn(0), outputSigmoidFlag(0)
{
  return;
}

MLP::~MLP(){  
  if(nin!=0){
    free_array((void*)win,2,hn,nin+1);
    free_array((void*)dwin,2,hn,nin+1);
    free_array((void*)whid,2,nout,hn+1);
    free_array((void*)dwhid,2,nout,hn+1);
    free_array((void*)outp,1,nout);
    free_array((void*)err,1,nout);
    free_array((void*)hnout,1,hn);
    free_array((void*)herr,1,hn);
    nin=0;
    nout=0;
    hn=0;
  }
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
  moment=0;
  ssize=0;
  if( win && dwin && whid && dwhid && outp && err && herr && hnout)
    return(1);
  else return(0);
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
    sum=sum+Forward(pattern->outpt[c],
		    pattern->inpt[c]);
      
    if(!Backward(pattern->inpt[c])){
      fprintf(stderr,"MLP::Train: Backward Pass failed.\n");
      exit(1);
    }
    fvno++;
  }
  sum/=fvno;
  return(sum);
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
   
    sum=sum+Forward(pattern->outpt[c],
		    pattern->inpt[c]);
    for(e=0;e<nout;e++){
      results->inpt[c][e]=outp[e];
      results->outpt[c][e]=pattern->outpt[c][e];
    }
    fvno++;  
  }
  sum/=fvno;
  return(sum);
}

/*** perform one forward pass and calculate MSE ***/
float MLP::Forward(float* dout, float* inpts){
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

  /*** calculate MSE and errs ***/
  sum=0;

  int nvalid=0;
  for(c=0;c<nout;c++){ 
    /***#############HACK_ALERT#########HACK_ALERT############****/
    /**** dout[c]=-1 means don't care, do not train on this value ***/
    /*****####################################################****/
    if(dout[c]>=-0.5){
      err[c]=(dout[c]-outp[c]);
      nvalid++;
    }
    else err[c]=0;
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
  if(strcmp(value_string,"MLP")!=0){
    if(strcmp(value_string,"mlp")!=0){
	fprintf(stderr,"MLP::Read: File Type is not MLP!");
	return(0);
     }
  }

  /*** get number of layers if available and then number of inputs***/
  value_string=skip_comments(ifp,line_from_file);
  int layers_listing=1;
  if(strcmp(value_string,"#LAYERS")!=0){
    if(strcmp(value_string,"#layers")!=0){
      if(strcmp(value_string,"#Layers")!=0){
	layers_listing=0;
	/*** get number of inpts ***/
	value_string=strtok(NULL," :\n\t");
	nin=atoi(value_string);
      }
    }
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





