#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "Constants.h"
#include "MLP.h"
#include "MLPData.h"

// Routine for converting Cassini or AmbigSim style beam patterns to
// OVWM style patterns
int main(int argc, char* argv[]){
  char train_set_desc_dflt[] = "Unspecified";
  char *trainfile = NULL, *netfile = NULL, *oldmlpfile = NULL, *train_set_desc = train_set_desc_dflt;
  int epochs = -1, hidnos = -1, filter = 0, usevss = 0;
  int c;
  
  char *opt_str = "t:o:e:h:f:v:m:d:";
        
  while((c = getopt(argc, argv, opt_str)) != -1) {
    switch (c) {
        case 't':
            trainfile = optarg; break;
        case 'o':
            netfile = optarg; break;
        case 'e':
            epochs = atoi(optarg); break;
        case 'h':
            hidnos = atoi(optarg); break;
        case 'f':
            filter = atoi(optarg); break;
        case 'v':
            usevss = atoi(optarg); break;
        case 'm':
            oldmlpfile = optarg; break;
        case 'd':
            train_set_desc = optarg; break;
    }
  }
  
  char usage_string[] = "Usage: %s -t training_set_file -o output_net_file -e num_epochs -h num_hidden_units\n"
        "\t[-f filter] [-v usevss] [-m old_mlp_file] [-d \"training set description\"]\n"
        "Filter Values: 0=no filter, 1= no ku, 2= no C, 3= no inner, 4 = no outer, 5 = no Ku inner,\n"
        "\t6 = no Ku outer, 7 = no C inner, 8 = no C outer, 9= no VAR, 10 no Ku VAR\n"
        "The training set description can be up to 1000 characters long and include spaces, but must be surrounded by quotes\n";
  if (trainfile == NULL || netfile == NULL || epochs <= 0 || hidnos <= 0) {
    fprintf(stderr, usage_string, argv[0]);
    exit(1);
  }
  
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
  // WARNING: values bellow just for testing- not actually correct
  char in_types[][IN_TYPE_STR_MAX_LENGTH] = 
   {"S0_MEAN_K_HH_INNER_FORE", "S0_STD_K_HH_INNER_FORE", 
    "S0_MEAN_K_HH_INNER_AFT",  "S0_STD_K_HH_INNER_AFT",
    "S0_MEAN_K_VV_OUTER_FORE", "S0_STD_K_VV_OUTER_FORE",
    "S0_MEAN_K_VV_OUTER_AFT",  "S0_STD_K_VV_OUTER_AFT",
    "S0_MEAN_C_HH_INNER_FORE", "S0_STD_C_HH_INNER_FORE", 
    "S0_MEAN_C_HH_INNER_AFT",  "S0_STD_C_HH_INNER_AFT",
    "S0_MEAN_C_VV_OUTER_FORE", "S0_STD_C_VV_OUTER_FORE",
    "S0_MEAN_C_VV_OUTER_AFT",  "S0_STD_C_VV_OUTER_AFT",
    "CROSS_TRACK_DISTANCE" };
  mlp.setInputTypesByString(in_types);
  mlp.setTrainSetString(train_set_desc);
  
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
