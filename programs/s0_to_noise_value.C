//-----------------------//
// Configuration Control //
//-----------------------//

static const char rcs_id[] =
	"@(#) $Id$";

//----------//
// INCLUDES //
//----------//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Misc.h"
#include "ConfigList.h"
#include "L2A.h"
#include "ConfigSim.h"
#include "L2B.h"
#include "L2AToL2B.h"

//-----------//
// TEMPLATES //
//-----------//

template class List<StringPair>;
template class List<Meas>;
template class List<EarthPosition>;
template class List<WindVectorPlus>;
template class List<MeasSpot>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<long>;
template class List<OffsetList>;

//-----------//
// CONSTANTS //
//-----------//

//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "<sim_config_file>","<s0_file>","<output_file>", 0};

//--------------//
// MAIN PROGRAM //
//--------------//

int
main(
	int		argc,
	char*	argv[])
{

	//------------------------//
	// parse the command line //
	//------------------------//

	const char* command = no_path(argv[0]);
	if (argc != 4)
		usage(command, usage_array, 1);

	int clidx = 1;
	const char* config_file = argv[clidx++];

	//---------------------//
	// read in config file //
	//---------------------//

	ConfigList config_list;
	if (! config_list.Read(config_file))
	{
		fprintf(stderr, "%s: error reading sim config file %s\n",
			command, config_file);
		exit(1);
	}

  	//-------------------------------------//
	// read the geophysical model function //
	//-------------------------------------//
 
	GMF gmf;
	if (! ConfigGMF(&gmf, &config_list))
	{
		fprintf(stderr, "%s: error configuring GMF\n", command);
		exit(1);
	}     

 
	//--------------------//
	// read the windfield //
	//--------------------//

	WindField windfield;
        WindVector wv;
	if (! ConfigWindField(&windfield, &config_list))
	{
		fprintf(stderr, "%s: error configuring wind field\n", command);
		exit(1);
	}

        printf("Tables of values for each wind vector in grid\n");
        printf("true_wind_speed, s0_bias, s0_rms_error\n");
        /********** Open input and output files ***********/
        FILE* ifp= fopen(argv[2],"r");
        FILE* ofp= fopen(argv[3],"w");

	char string[100], *value;
	char* file_still_there; 
        float lon, lat;     
        double bias=0, ave_bias=0, var=0, ave_std=0;  
        int num_grids=0, num_s0=0;
  while(1){
    file_still_there=fgets(string,100,ifp);   
    if(!file_still_there) break;
    if(strcmp(string,"\n")==0) break;
    if(string[2]=='L'){
      fprintf(ofp,"%s",string);
      value=strtok(string," \t\n");  // skip over #
      value=strtok(NULL," \t\n");
      lon=atof(value+4)*dtr;
      value=strtok(NULL," \t\n");
      lat=atof(value+4)*dtr;    

      /**** Get WIND VECTOR FROM TRUTH MAP ***/
      LonLat lon_lat;
      lon_lat.longitude=lon;
      lon_lat.latitude=lat;
      if(num_s0 > 20){
	bias=bias/num_s0;
        var=var/num_s0;
        double std;
        std=sqrt(var);
        printf("%g %g %g\n",wv.spd,bias,std);
	ave_std+=fabs(std);
        ave_bias+=fabs(bias);
	num_grids++;
      }
      else if(num_s0>0){
	// printf("Too few s0s to calculate grid bias\n\n");
      }
      bias=0;
      num_s0=0;
      var=0;
  
      if (! windfield.InterpolatedWindVector(lon_lat, &wv))
	{
	  wv.spd = 0.0;
	  wv.dir = 0.0;
	}
    }
    else if(string[0]=='#'){
      fprintf(ofp,"%s",string);
    }
    else{
      float eastAzimuth, noise_val, sigma0, sigma0_true, incidenceAngle;
      char polc;
      PolE pol;
      value=strtok(string," \t\n");  // read polarization;
      polc=value[0];
      if (polc=='V') pol=V_POL;
      else pol=H_POL;
      value=strtok(NULL," \t\n");
      incidenceAngle=atof(value)*dtr;
      value=strtok(NULL," \t\n");
      eastAzimuth=atof(value)*dtr;    
      value=strtok(NULL," \t\n");
      sigma0=atof(value);    
      
      //--------------------------------//
      // convert wind vector to sigma-0 //
      //--------------------------------//

      // chi is defined so that 0.0 means the wind is blowing towards
      // the s/c (the opposite direction as the look vector)
      float chi = wv.dir - eastAzimuth + pi;

      gmf.GetInterpolatedValue(pol, incidenceAngle, wv.spd,
				chi, &sigma0_true);

      noise_val=(sigma0-sigma0_true)/sigma0_true;
      num_s0++;
      bias+=noise_val;
      var+=noise_val*noise_val;
      //      printf("%g %g %g\n",sigma0,sigma0_true,noise_val);
      fprintf(ofp,"%c %g %g %g\n",polc,incidenceAngle*rtd,eastAzimuth*rtd,noise_val);
    }
  }
  if(num_s0 > 20){
    bias=bias/num_s0;
    var=var/num_s0;
    double std=sqrt(var);
    ave_bias+=fabs(bias);
    ave_std+=std;
    num_grids++;
  }
  ave_bias=ave_bias/num_grids;
  ave_std=ave_std/num_grids;
  printf("Average magnitude of the grid-wise noise bias is %g\n",ave_bias);
  printf("Average grid-wise noise RMS is %g\n",ave_std);
 fclose(ifp);
 fclose(ofp);
 
 exit(0);
}



