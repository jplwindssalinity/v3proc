#include<stdio.h>
#include<stdlib.h>
#include"Constants.h"
#include"MLP.h"
#include"MLPData.h"

// Routine for converting Cassini or AmbigSim style beam patterns to
// OVWM style patterns
int main(int argc, char* argv[]){
  if(argc!=5){
    fprintf(stderr,"Usage:%s epochs hidunits trainfile netfile\n",argv[0]);
    exit(1);
  }
  int clidx=1;
  int epochs = atoi(argv[clidx++]);
  int hidnos = atoi(argv[clidx++]);
  char* trainfile = argv[clidx++];
  char* netfile = argv[clidx++];
  MLPData data;
  data.Read(trainfile);
  MLP mlp;
  mlp.RandomInitialize(-1.0,1.0,data.num_inpts,data.num_outpts,hidnos,587789);
  for(int c=0;c<epochs;c++){
    data.Shuffle();
    float mse=mlp.Train(&data,0.5,0.01);
    printf("Epoch %d MSE=%g\n",c,mse);
  }
  mlp.Write(netfile);
  return(0);
}
