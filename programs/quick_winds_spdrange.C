//==============================================================//
// Copyright (C) 2007, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    quick_winds.C
//
// SYNOPSIS
//    quick_winds <sim_config_file> <wind_speed> 
//                <geometry_input_file> <metrics_output_file> <num_samples>
//                [opt_c_band]
//
// DESCRIPTION
//    
//  Takes geometry and noise information in an input file as a function
// of cross track distance. Simulates gridded measurements
// from constant wind speed and uniformly distributed random directions.
// Performs point wise wind retrieval and computes nearest RMS direction
// error, nearest RMS speed error, and first rank skill.
//
// OPTIONS
//    None.
//
// OPERANDS
//    The following operands are supported/required:
//
//      <sim_config_file>  The sim_config_file needed listing
//                         all the wind retrieval parameters
//
//      <wind_speed>       True speed in m/s
//
//      <geometry input file> Files contains geometry, look distribution, and
//                            SNR as a function of cross track distance
//       Format is:
//       XX columns of ASCII text with a single ASCII header line. 
//       Header line is: Number_of_Beams  grid_cell_resolution 
//       Data lines are:
//       Column 1: Cross track distance   starts at -1000 km end at +1000 
//                 in 1 km steps (for example).
//       The quantities in columns 2-8 are for Beam 1 Fore Look Measurements.
//       Column 2: Number of Beam 1 Fore Look Measurements in wind vector cell
//       Column 3: Average Noise Equivalent Sigma0 in dB 
//       Column 4: Number of Looks per measurement 
//                 (typically number of range looks averaged)
//       Column 5: Azimuth angle
//       Column 6: Incidence angle
//       Column 7: Polarization  (V or H)
//       Column 8: Signal to Ambiguity Ratio  in dB
//       Columns 9-15 are the same as 2-8 but for Beam 1 Aft Look
//       Columns 16-22 are the same as 2-8 but for Beam 2 Fore Look
//       Columns 23-29 are the same as 2-8 but for Beam 2  Aft Look
//       Further columns are necessary if there are more than 2 beams.
//
//       <metrics output file> (grid resolution should be part of file name)
//       ASCII Format for each line is:
//       Column 1: Cross Track Distance   
//       Column 2: Nearest RMS Dir Err 
//       Column 3: Nearest RMS Speed Error
//       Column 4: Nearest Speed Bias (Retrieved - Truth )
//       Column 5: First Rank Skill
//
//       <num_samples> Number of wind cells simulated for each cross track
//                     distance. A larger number yields more accurate 
//                     stats and longer running time.
// 
//       [opt_c_band]  If optional operand is nonzero then C BAND
//                     GMF is used/required
//
//       [opt_coast]   If second optional operand is nonzero then ambiguities
//                     over land are simulated
// EXAMPLES
//    An example of a command line is:
//      % quick_winds ovwm.rdf  7 geomfile5kmres.txt metrics7mps5kmres.txt
//
// ENVIRONMENT
//    Not environment dependent.
//
// EXIT STATUS
//    The following exit values are returned:
//       0  Program executed successfully
//      >0  Program had an error
//
// NOTES
//    None.
//
// AUTHORS
//    Bryan.W.Stiles
//----------------------------------------------------------------------

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
#include <signal.h>
#include "List.h"
#include "BufferedList.h"
#include "Misc.h"
#include "ConfigList.h"
#include "L2A.h"
#include "ConfigSim.h"
#include "L2B.h"
#include "L2AToL2B.h"
#include "Tracking.h"

using std::list;
using std::map; 

//-----------//
// TEMPLATES //
//-----------//

// Class declarations needed for templates
// eliminates need to include the entire header file
class AngleInterval;

template class List<StringPair>;
template class List<Meas>;
template class List<EarthPosition>;
template class List<WindVectorPlus>;
template class List<MeasSpot>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<off_t>;
template class List<OffsetList>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;
template class List<AngleInterval>;
template class std::list<string>;
template class std::map<string,string,Options::ltstr>;

//-----------//
// CONSTANTS //
//-----------//

//#define DEBUG 
#define NMAXSAMPLES 1000

//-------//
// HACKS //
//-------//



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

const char* usage_array[] = { "<sim_config_file>", "<wind_speed>", "<geom_input_file>","<metrics_output_file>","<num_samples>","[opt_c_band]","[opt_coast]",0};


//--------------//
// MAIN PROGRAM //
//--------------//

int
main(
    int    argc,
    char*  argv[])
{
    //------------------------//
    // parse the command line //
    //------------------------//

    const char* command = no_path(argv[0]);
    if (argc < 6 || argc>9)
        usage(command, usage_array, 1);

    int clidx = 1;
    const char* config_file = argv[clidx++];
    float true_speed_mean=atof(argv[clidx++]);
    if(true_speed_mean<0 || true_speed_mean>100){
      fprintf(stderr,"True Speed Mean %g failed sanity check\n",true_speed_mean);
      exit(1);
    }
    const char* in_file = argv[clidx++];
    const char* out_file = argv[clidx++];
    int n=atoi(argv[clidx++]);
    bool use_cband=false;
    bool coast=false;
    bool use_spdrange = false;
    float spd_rad=0;
    if(argc>=7) use_cband=(bool)atoi(argv[clidx++]);
    if(argc>=8) coast=(bool)atoi(argv[clidx++]);
    if(argc==9) {
      use_spdrange =true;
      spd_rad=atof(argv[clidx++]);
    }
    printf("Simulating %g to %g m/s using %d samples\n",true_speed_mean-spd_rad,
	   true_speed_mean+spd_rad,n);
    if(use_spdrange) printf("    using speed priors.\n");
    fflush(stdout);


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


    // Determine whether or not Kpm is simulated
    int sim_kpm;

    config_list.GetInt(SIM_UNCORR_KPM_FLAG_KEYWORD,&sim_kpm);

    // Determine wind Retrieval method

    char* method;
    int opt_method=0;
    method=config_list.Get(WIND_RETRIEVAL_METHOD_KEYWORD);
    if(strcasecmp(method,"BRUTE_FORCE")==0) opt_method=1;
    else{
      fprintf(stderr,"Warning:Wind retrieval method %s not valid for use in quick_winds\n",method);
      fprintf(stderr,"Using BRUTE_FORCE instead\n");
      opt_method=1;
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

    //--------------//
    // configure Kp //
    //--------------//

    Kp kp;
    if (! ConfigKp(&kp, &config_list))
    {
        fprintf(stderr, "%s: error configuring Kp\n", command);
        exit(1);
    }


    //------------//
    // open files //
    //------------//

    FILE* ifp=fopen(in_file,"r");
    FILE* ofp=fopen(out_file,"w");
    if(ifp==NULL){
      fprintf(stderr,"Cannot open geometry_input_file %s\n",in_file);
      exit(1);
    }
    if(ofp==NULL){
      fprintf(stderr,"Cannot create metrics_output_file %s\n",out_file);
      exit(1);
    }
    
    //--------------------------
    // Read geometry file header
    //--------------------------
    char line[4096];
    char* str;
    int nbeams, nlooks;
    float gridres;
    int lineno=0;
#define MAX_NUM_LOOKS 16
    float bias[MAX_NUM_LOOKS]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    while(1){
      lineno++;
      if (fgets(line, 4096, ifp) != line){
     
	fprintf(stderr,"Error reading first line of geom file %s\n",in_file);
	exit(1);
      }

      if (line[0]=='#') continue; // skip comments
      else{
	str=strtok(line," \t");
        nbeams=atoi(str);


	nlooks=2*nbeams;
	if(nlooks>MAX_NUM_LOOKS || nlooks<1){
	  fprintf(stderr,"Bad number of beams %d and thus looks %d\n",nbeams,nlooks);
	  fprintf(stderr,"If you need more beams edit MAX_NUM_LOOKS in quick_winds.C and recompile\n");
	  exit(1);
	}
	str=strtok(NULL," \t");
        gridres=atof(str);

	str=strtok(NULL," \t");
	if(str!=NULL){
	  bias[0]=pow(10,0.1*atof(str))-1;
	  for(int c=1;c<nlooks;c++){
	    str=strtok(NULL," \t");
	    bias[c]=pow(10,0.1*atof(str))-1;
	  }
	}
        break; // done with while loop
      }
    }
    
    //----------------------- 
    // setup geometry arrays
    //-----------------------



    float ctd;
    int nmeas[MAX_NUM_LOOKS];
    float s0ne[MAX_NUM_LOOKS];
    int nlpm[MAX_NUM_LOOKS];
    float azim[MAX_NUM_LOOKS];
    float inc[MAX_NUM_LOOKS];
    char pol[MAX_NUM_LOOKS];
    float sambrat[MAX_NUM_LOOKS];


    //-----------------//
    // conversion loop //
    //-----------------//


    for (;;)
    {
      //-----------------------------
      // Read line from geometry file
      //-----------------------------

      bool line_found=true;
      while(1){
	lineno++;
	if (fgets(line, 4096, ifp) != line){
	  line_found=false;
	  break;
	}
	
	if (line[0]=='#') continue; // skip comments
	else{
	  break; // finished reading line
	}
      }

      // check for end of input file
      if(!line_found) break;

      if(line[0]==' '){
	fprintf(stderr,"Error line %d in %s starts with a space character",lineno,in_file);
	exit(1);
      }
      if(line[0]=='\n') continue;
      //-----------------------------
      // Parse line
      //-----------------------------
      str=strtok(line," \t");
      if(str==NULL) continue;  // skips blank line
      ctd=atof(str);
      for(int i=0;i<nlooks;i++){
	str=strtok(NULL," \t");
	nmeas[i]=atoi(str);

	str=strtok(NULL," \t");
	s0ne[i]=atof(str);
	s0ne[i]=pow(10,0.1*s0ne[i]);

	str=strtok(NULL," \t");
	nlpm[i]=atoi(str);

	str=strtok(NULL," \t");
	azim[i]=atof(str)*dtr;	

	str=strtok(NULL," \t");
	inc[i]=atof(str)*dtr;

	str=strtok(NULL," \t");
	pol[i]=str[0];

	str=strtok(NULL," \t");
	sambrat[i]=atof(str);
	sambrat[i]=pow(10,0.1*sambrat[i]);
	
      }
      double speed_rms=0, speed_bias=0, dir_rms=0;
      double skill=0;
      double retWindSpdMean=0, retWindSpdSd=0;

      int zero_count = 0;

      Uniform randdirgen(pi,pi);
      Uniform randspdgen(spd_rad,true_speed_mean);
      randdirgen.SetSeed(10023455);
      Gaussian noisegen(1,0);
      noisegen.SetSeed(23838478);
      randspdgen.SetSeed(1555534);
      float retWindSpd[NMAXSAMPLES];

      //-------------
      // sample loop
      //--------------
      for(int i=0;i<n;i++){

	// get random direction

	float dir=randdirgen.GetNumber();
	float true_speed;
        if(use_spdrange){
	  true_speed=randspdgen.GetNumber();
	}
        else{
	  true_speed=true_speed_mean;
	}
        //-------------------------
	// Create Measurement List
        //-------------------------
        MeasList meas_list;
        for(int j=0;j<nlooks;j++){
	  for(int k=0;k<nmeas[j];k++){
	    
            // set geometric quantities
	    Meas* meas = new Meas;
	    meas->beamIdx=j/2;
	    meas->scanAngle=(j%2)*pi;
	    meas->eastAzimuth=azim[j];
            meas->incidenceAngle=inc[j];
	    meas->startSliceIdx=k;
            meas->numSlices=-1;

	    if(use_cband){
	      if(pol[j]=='v' || pol[j]=='V'){
		meas->measType=Meas::C_BAND_VV_MEAS_TYPE;
	      }
	      else{
		meas->measType=Meas::C_BAND_HH_MEAS_TYPE;
	      }
	    }
	    else{
	      if(pol[j]=='v' || pol[j]=='V'){
		meas->measType=Meas::VV_MEAS_TYPE;
	      }
	      else{
		meas->measType=Meas::HH_MEAS_TYPE;
	      }
	    }
            // set unused quantities
            meas->XK=0;
	    meas->EnSlice=0;
            meas->bandwidth=0;
	    meas->txPulseWidth=0;
	    meas->landFlag=0;
	    meas->azimuth_width=0;
            meas->range_width=0;
          


            // get true GMF
            float s0true;
            float chi = dir - meas->eastAzimuth + pi;
	    gmf.GetInterpolatedValue(meas->measType,meas->incidenceAngle,
				 true_speed,chi,&s0true);


            // add bias due to ambiguities over land
            // for now s0land=0.1 (-10 dB)
            // need to read a config file parameter and make it
            // incidence angle and frequency dependent
         
            float s0land=0.1;
            if(!coast) s0land=0;
            //printf("%g %g %g %g\n",s0true,s0land,sambrat[j],s0land/sambrat[j]);
	    s0true+=s0land/sambrat[j];
            
	    // set value, A, B, C
            
            // fudge factors
	    float SNRff=1;
            double kpm2=0;
            if(sim_kpm){
	      kp.GetKpm2(meas->measType,true_speed,&kpm2);
	    }

            float SNR=s0true/s0ne[j];
	    SNR*=SNRff;

   
	    float kpc2 = (1.0/nlpm[j])*(1+2/SNR+1/(SNR*SNR));
	    float kptotal2=kpc2+kpm2;
            float kptotal=sqrt(kptotal2);



	    float noise=noisegen.GetNumber()*s0true*kptotal;
            meas->value=s0true+noise+s0true*bias[j];
            meas->A=1+1.0/(float)nlpm[j];
            meas->B=2.0*s0ne[j]/(float)nlpm[j];
            meas->C=s0ne[j]*s0ne[j]/(float)nlpm[j];
            // add measurement to list
	    meas_list.Append(meas);

#define DEBUG
#ifdef DEBUG
	    float var=gmf.GetVariance(meas,true_speed,chi,s0true,&kp);
            printf("%g %g %g %g %g %g %g\n", s0true, meas->value, kptotal, sqrt(kpc2), sqrt(kpm2),s0true*s0true*kptotal*kptotal,var);
#endif
	  }
	}

	//-----------------
	// Retrieve Winds
        //-----------------
        WVC wvc;
        if(meas_list.NodeCount()>1){
	  if(use_spdrange) gmf.RetrieveWinds_BruteForce(&meas_list,&kp,&wvc,0,
							true_speed_mean-spd_rad,
							true_speed_mean+spd_rad);
	  else gmf.RetrieveWinds_BruteForce(&meas_list,&kp,&wvc);
	  wvc.SortByObj();
          if (wvc.ambiguities.NodeCount() > 0) {
  	    WindVectorPlus* first=wvc.ambiguities.GetHead();
	    WindVectorPlus* near;
            if(opt_method==0) near=wvc.GetNearestToDirection(dir);
            else near=wvc.GetNearestVector(dir,true_speed);
	    if(near==first)skill++;
	    float direrr=ANGDIF(near->dir,dir)*rtd;
	    float spderr=near->spd - true_speed;
	    speed_bias+=spderr;
	    speed_rms+=spderr*spderr;
	    dir_rms+=direrr*direrr;
            retWindSpd[i] = near->spd;
          } else {
            zero_count ++;
            retWindSpd[i] = -1.0;
          }

	  //-------------------------
	  // Accumulate error metrics
	  //-------------------------

	  // free MeasList object
	  meas_list.FreeContents();
	} // end num meas > 0

      } // end number of samples loop
      
      // normalize metrics
      if(n!=0 && zero_count<n){
	speed_rms=sqrt(speed_rms/(n-zero_count));
	speed_bias=speed_bias/(n-zero_count);
	dir_rms=sqrt(dir_rms/(n-zero_count));
	skill=skill/(n-zero_count);
      } else {
	speed_rms=0.;
	speed_bias=0.;
	dir_rms=0.;
	skill=0.;
      }

      // find standard deviation of retrieved wind speed
      if (speed_rms != 0.0) {  // perform retrieval
        float sum2 = 0.;
        float sum=0.;
        for (int ii=0; ii<n; ii++) {
          if (retWindSpd[ii] != -1.0) {
            sum2 += retWindSpd[ii]*retWindSpd[ii];
	    sum+=retWindSpd[ii];
          }
        }

        retWindSpdMean = sum/(n-zero_count);
        retWindSpdSd = sqrt(sum2/(n-zero_count)-retWindSpdMean*retWindSpdMean);
      }

      // output metrics to file
      fprintf(ofp,"%g %g %g %g %g %g %g %d\n",ctd,dir_rms,speed_rms,speed_bias,skill,
                  retWindSpdMean,retWindSpdSd,zero_count);
       
      
    }


    fclose(ifp);
    fclose(ofp);
    return (0);
}
