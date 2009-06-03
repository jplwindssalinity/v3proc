#include<stdio.h>
#include<stdlib.h>
#include"Constants.h"
#include"MLP.h"
#include"MLPData.h"

// Routine for converting Cassini or AmbigSim style beam patterns to
// OVWM style patterns
int main(int argc, char* argv[]){
  if(argc!=5 && argc!=6 && argc!=7 && argc!=8){
    fprintf(stderr,"Usage:%s epochs hidunits trainfile netfile\n [usevss] [filter 0=no filter, 1= no ku, 2= no C, 3= no inner, 4 = no outer,\n 5 = no Ku inner, 6 = no Ku outer, 7 = no C inner, 8 = no C outer, 9= no VAR, 10 no Ku VAR]\n",argv[0]);
    exit(1);
  }
  int clidx=1;
  int epochs = atoi(argv[clidx++]);
  int hidnos = atoi(argv[clidx++]);
  char* trainfile = argv[clidx++];
  char* netfile = argv[clidx++];
  int filter=0;
  int usevss=0;
  char * oldmlpfile=NULL;
  if(argc>=6) usevss=atoi(argv[clidx++]);
  if(argc>=7) filter=atoi(argv[clidx++]);
  if(argc>=8) oldmlpfile=argv[clidx++];
  MLPData data;
  data.Read(trainfile);
  for(int c=0;c<data.num_samps;c++){
    switch(filter){
    case 1:
      data.inpt[c][0]=0;
      data.inpt[c][1]=0.1;
      data.inpt[c][2]=0;
      data.inpt[c][3]=0.1;
      data.inpt[c][4]=0;
      data.inpt[c][5]=0.1;
      data.inpt[c][6]=0;
      data.inpt[c][7]=0.1;
      break;
    case 2:
      data.inpt[c][8]=0;
      data.inpt[c][9]=0.1;
      data.inpt[c][10]=0;
      data.inpt[c][11]=0.1;
      data.inpt[c][12]=0;
      data.inpt[c][13]=0.1;
      data.inpt[c][14]=0;
      data.inpt[c][15]=0.1;
      break;
    case 3:
      data.inpt[c][0]=0;
      data.inpt[c][1]=0.1;
      data.inpt[c][2]=0;
      data.inpt[c][3]=0.1;
      data.inpt[c][8]=0;
      data.inpt[c][9]=0.1;
      data.inpt[c][10]=0;
      data.inpt[c][11]=0.1;
      break;
    case 4:
      data.inpt[c][4]=0;
      data.inpt[c][5]=0.1;
      data.inpt[c][6]=0;
      data.inpt[c][7]=0.1;
      data.inpt[c][12]=0;
      data.inpt[c][13]=0.1;
      data.inpt[c][14]=0;
      data.inpt[c][15]=0.1;
      break;
    case 5:
      data.inpt[c][0]=0;
      data.inpt[c][1]=0.1;
      data.inpt[c][2]=0;
      data.inpt[c][3]=0.1;
      break;
    case 6:
      data.inpt[c][4]=0;
      data.inpt[c][5]=0.1;
      data.inpt[c][6]=0;
      data.inpt[c][7]=0.1;
      break;
    case 7:
      data.inpt[c][8]=0;
      data.inpt[c][9]=0.1;
      data.inpt[c][10]=0;
      data.inpt[c][11]=0.1;
      break;
    case 8:
      data.inpt[c][12]=0;
      data.inpt[c][13]=0.1;
      data.inpt[c][14]=0;
      data.inpt[c][15]=0.1;
      break;
    case 9:
      data.inpt[c][1]=0.1;
      data.inpt[c][3]=0.1;
      data.inpt[c][5]=0.1;
      data.inpt[c][7]=0.1;
      data.inpt[c][9]=0.1;
      data.inpt[c][11]=0.1;
      data.inpt[c][13]=0.1;
      data.inpt[c][15]=0.1;
    case 10:
      data.inpt[c][1]=0.1;
      data.inpt[c][3]=0.1;
      data.inpt[c][5]=0.1;
      data.inpt[c][7]=0.1;
    default:
      break;
    }
  }
  MLP mlp;
  if(!oldmlpfile){
    mlp.RandomInitialize(-1.0,1.0,data.num_inpts,data.num_outpts,hidnos,587789);
  }
  else{
    mlp.Read(oldmlpfile);
  }
  data.Shuffle();
  for(int c=0;c<epochs;c++){
    float mse;
    if(usevss){
      if(!oldmlpfile && c==0){
	for(int d=0;d<epochs;d++){
	  data.Shuffle();
	  mse = mlp.Train(&data,0.0,0.005);
	  printf("VSS Epoch %d BP Epoch %d MSE=%g\n",c,d,mse);
	  fflush(stdout);
	}
      }
      mse=mlp.TrainVSS(&data,c);
    }
    else{ 
      data.Shuffle();
      mse=mlp.Train(&data,0.5,0.01);
    }
    printf("Epoch %d MSE=%g\n",c,mse);
    fflush(stdout);
  }
  mlp.Write(netfile);
  return(0);
}
