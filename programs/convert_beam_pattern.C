#include<stdio.h>
#include<stdlib.h>
#include"Constants.h"

// Routine for converting Cassini or AmbigSim style beam patterns to
// OVWM style patterns
int main(int argc, char* argv[]){
  if(argc!=3){
    fprintf(stderr,"Usage:%s cassini_file ovwm_file\n",argv[0]);
    exit(1);
  }
  int clidx=1;
  char* cassini_file = argv[clidx++];
  char* ovwm_file = argv[clidx++];
  FILE* ifp=fopen(cassini_file,"r");
  if(ifp==NULL){
    fprintf(stderr,"Error Unable to open file %s\n",cassini_file);
    exit(1);
  }
  FILE* ofp=fopen(ovwm_file,"w");
  if(ofp==NULL){
    fprintf(stderr,"Error Unable to create file %s\n",ovwm_file);
    exit(1);
  }
  
  // hard coded Cassini beam pattern stuff
  int Nelev=200;
  int Nazim=200;
  
  double elev_step=0.05*dtr;
  double azim_step=0.05*dtr;

  // set/write Ovwm header
  int Nx=Nelev;
  int Ny=Nazim;
  int ix_zero=100;     // Cassini Start elevation is -5 degrees
  int iy_zero=100;     // Cassini Start azimuth is -5 degrees
  double x_spacing=elev_step;
  double y_spacing=azim_step;
  double electrical_boresight_Em=0; // center of beam pattern is electrical boresight
  double electrical_boresight_Am=0;
  if (fwrite(&Nx, sizeof(int), 1, ofp) != 1 ||
      fwrite(&Ny, sizeof(int), 1, ofp) != 1 ||
      fwrite(&ix_zero, sizeof(int), 1, ofp) != 1 ||
      fwrite(&iy_zero, sizeof(int), 1, ofp) != 1 ||
      fwrite(&x_spacing, sizeof(double), 1, ofp) != 1 ||
      fwrite(&y_spacing, sizeof(double), 1, ofp) != 1 ||
      fwrite(&electrical_boresight_Em, sizeof(double), 1, ofp) != 1 ||
      fwrite(&electrical_boresight_Am, sizeof(double), 1, ofp) != 1)
    {
        fprintf(stderr, "Error writing header data to %s\n", ovwm_file);
	exit(1);
    }



  // read/write gain array

  float* x=(float*)malloc(Nx*Ny*sizeof(float));
  float fx;
  if(fread(&(x[0]),sizeof(float),Nx*Ny,ifp)!=(unsigned int)Nx*Ny){
	fprintf(stderr,"Error: Unable to read from file %s\n",cassini_file);
	exit(1);
  }

  for(int i=0;i<Nx;i++){
    for(int j=0;j<Ny;j++){
      int offin=Nx*j+i;
      fx=(float)pow(10.0,x[offin]/10.0);
      
      if(fwrite(&fx,sizeof(float),1,ofp)!=1){
	fprintf(stderr,"Error: Unable to write to file %s\n",ovwm_file);
	exit(1);
      }
    }
  }
  free(x);
  fclose(ifp);
  fclose(ofp);
  return(0);
}
