#include<stdio.h>
#include<stdlib.h>
#include"Constants.h"
#include"MLP.h"
#include"MLPData.h"

// Routine for converting Cassini or AmbigSim style beam patterns to
// OVWM style patterns
int main(int argc, char* argv[]){
  if(argc!=2){
    fprintf(stderr,"Usage:get_ave_num_flavors mlpdatafile\n");
    exit(1);
  }
int clidx=1;
char* infile = argv[clidx++];
MLPData data;
data.Read(infile);
double nfave=0;
 double spdave=0;
 double minctd=100000;
 double maxctd=-100000;
 for(int c=0;c<data.num_samps;c++){
   int nf=0;
   for(int d=0;d<16;d+=2){
     if(data.inpt[c][d]!=0) nf++;
   }
   spdave+=data.outpt[c][0];
   nfave+=nf;
   if(data.inpt[c][16]<minctd) minctd=data.inpt[c][16];
   if(data.inpt[c][16]>maxctd) maxctd=data.inpt[c][16];
 }
nfave/=data.num_samps;
spdave/=data.num_samps;
printf("Averages %g number of flavors per WVC\n",nfave);
printf("Averages %g m/s\n",spdave);
 printf("CTD input range is [%g,%g]\n",minctd,maxctd);
}
