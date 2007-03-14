/* Routine to generate OVWM antenna beam pattern */

#include<stdio.h>
#include<stdlib.h>
#include"Constants.h"
#include"YahyaAnt.h"
#include"ConfigSim.h"

int main(int argc, char* argv[]){
  if(argc!=3){
    fprintf(stderr,"Usage:%s config_file\n",argv[0]);
    exit(1);
  }
  int clidx=1;
  char* cassini_file = argv[clidx++];

  FILE* ofp=fopen(ovwm_file,"w");
  if(ofp==NULL){
    fprintf(stderr,"Error Unable to create file %s\n",ovwm_file);
    exit(1);
  }


/* Read configure file and get antenna parameters */
  ConfigList config_list;
  if (! config_list.Read(config_file))
    {
      fprintf(stderr, "%s: error reading sim config file %s\n",
	      command, config_file);
      exit(1);
    }

  antennaStruct Ant;
  ConfigYahyaAnt(&Ant,&config_list);



/* Genrate Yahya beam pattern and convert it to Cassini style */

  float gain[MaxAngIncSize2],gain_in[MaxAngIncSize2];

  Yahya2CassiniBeamPat(gain,gain_in,Ant);

  
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


  // write gain array

  float fx;

  for(int i=0;i<Nx;i++){
    for(int j=0;j<Ny;j++){

      int offin=Nx*j+i;
      fx=(float)pow(10.0,gain[offin]/10.0);
      
      if(fwrite(&fx,sizeof(float),1,ofp)!=1){
	fprintf(stderr,"Error: Unable to write to file %s\n",ovwm_file);
	exit(1);
      }
    }
  }

  fclose(ofp);
  return(0);

}
