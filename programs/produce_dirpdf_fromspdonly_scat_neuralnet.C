//==============================================================//
// Copyright (C) 2007, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    produce_full_scat_neuralnet.C
//
// SYNOPSIS
//    produce_full_scat_neuralnet <sim_config_file> <max_speed> <rainfilelist>
//                <geometry_input_file> <dataset_outputfile_base> <num_samples>
//
// DESCRIPTION
//    
//  Takes geometry and noise information in an input file as a function
// of cross track distance. Simulates gridded measurements
// from uniformly distributed random speed and each 5 degree set of directions.
// Outputs data set for use in training expected speed given s0 + direction
// ( one for each cross track distance and each 5 degree band of directions)
// OPTIONS
//    None.
//
// OPERANDS
//    The following operands are supported/required:
//
//      <sim_config_file>  The sim_config_file needed listing
//                         all the wind retrieval parameters
//
//      <max_speed>       maximum speed in m/s
//      <rainfilelist> list of rainfile scenes 1 for each beam (numbeams at top)
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
//       <dataset_outputfile_base> Base name for datasets to train MLPs
//
//       <num_samples> Number of wind cells simulated for each cross track
//                     distance and relative direction. 
//                     A larger number yields a bigger data set
//                     and longer running time.
// 
// EXAMPLES
//    An example of a command line is:
//      % produce_full_scat_neuralnet quikscat.cfg 50 rainexamples.lst quikscat_gn.dat quikscat_full_ann  10000
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
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Misc.h"
#include "ConfigList.h"
#include "L2A.h"
#include "ConfigSim.h"
#include "L2B.h"
#include "L2AToL2B.h"
#include "Tracking.h"
#include "Tracking.C"
#include "MLPData.h"
#include "MLP.h"
#include "RainDistribution.h"
#include "GeomNoiseFile.h"

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
#define NMAXSAMPLES 100000
#define NDIR 72   // should be 72 if we want MLP by direction
#define CROSSTRACKSPACING 2 // number of cross track bins per MLP 



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

const char* usage_array[] = { "<sim_config_file>", "<max_speed>", "<rainfilelist>""<geom_input_file>","<spdnet_input_file>","<dataset_output_file>","<num_samples>","[num_hidno]", "[max_epochs]", "[momentum]","[stepsize]","[max_bad_epochs]","[min_speed]",0};


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
    if (argc<8 || argc>14)
        usage(command, usage_array, 1);
    int hn=50;
    float momentum=0.5;
    float stepsize=0.01;
    int max_epochs=2000;
    int max_bad_epochs=10;
    float min_speed = 0;
    int clidx = 1;
    const char* config_file = argv[clidx++];
    float max_speed=atof(argv[clidx++]);
    //if(max_speed<=20 || max_speed>100){
    //  fprintf(stderr,"Max Speed %g failed sanity check\n",max_speed);
    //  exit(1);
    //}
    const char* rainlist_file = argv[clidx++];

    const char* in_file = argv[clidx++];
    char* spd_file = argv[clidx++];
    const char* out_file_base = argv[clidx++];
    int n=atoi(argv[clidx++]);
    if(argc>=clidx) hn=atoi(argv[clidx++]);
    if(argc>=clidx) max_epochs=atoi(argv[clidx++]);
    if(argc>=clidx) momentum=atof(argv[clidx++]);
    if(argc>=clidx) stepsize=atof(argv[clidx++]);
    if(argc>=clidx) max_bad_epochs=atoi(argv[clidx++]);
    if(argc>=clidx) min_speed=atof(argv[clidx++]);

    char out_dirpdf_file[200];
    sprintf(out_dirpdf_file,"%s_dirpdf.dat",out_file_base); 

    char net_dirpdf_file[200];
    sprintf(net_dirpdf_file,"%s_dirpdf.net",out_file_base); 

    printf("Simulating %g-%g m/s using %d samples\n",min_speed,max_speed,n);
    printf("With hidno=%d mom=%g ss=%g #epochs=%d #badepochs=%d\n",hn,momentum,stepsize,max_epochs,max_bad_epochs);

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

    GeomNoiseFile gnf(in_file); // open file, readsHeader, one pass through file
                                // compute look direction valid ctd range

    // Nominal Version
    int ncti=gnf.ncti;       
    float max_ctd=gnf.max_ctd;
    float min_ctd=gnf.min_ctd;   
    
    /**** Hack version 
    int ncti=1; 
    float min_ctd=(gnf.min_ctd+gnf.max_ctd)/2;;         
    float max_ctd=min_ctd+gnf.gridres;
    *****/
  
    float dir_offset = gnf.forward_direction;  // forward direction in radians



    if(ncti%CROSSTRACKSPACING!=0){
      int addext=CROSSTRACKSPACING - ncti%CROSSTRACKSPACING;
      int addmin=addext/2;
      int addmax=addext-addmin;
      ncti=ncti+addext;
      max_ctd=max_ctd+addmax*gnf.gridres;
      min_ctd=min_ctd-addmin*gnf.gridres;
    }
    fprintf(stderr,"NCTI=%d CTDRANGE=[%g,%g]\n",ncti,min_ctd,max_ctd);
    int nbeams=gnf.nbeams; 
    int nlooks=gnf.nlooks;
    fprintf(stderr,"Running with %d beams and %d looks per beam\n",nbeams,nlooks/nbeams);
    float gridres=gnf.gridres;


   
    int inMLP=nlooks*2+1;
    int outMLP=NDIR;
    int MLPdim1length=1;
    float MLPdim1min=0;
    float MLPdim1max=2*pi;
    int MLPdim2length=ncti/CROSSTRACKSPACING;
    float MLPdim2min=min_ctd;
    float MLPdim2max=max_ctd;
    
    MLPDataArray dirarr(out_dirpdf_file,net_dirpdf_file,MLPdim1length,MLPdim1min,MLPdim1max,MLPdim2length,MLPdim2min,MLPdim2max,inMLP,outMLP,n*CROSSTRACKSPACING,"dirpdfnet",hn); 
    dirarr.moment=momentum;
    dirarr.ssize=stepsize;
    dirarr.nepochs=max_epochs;
    dirarr.max_bad_epochs=max_bad_epochs;

    dirarr.SetDirPdf();

    MLPDataArray spdarr(spd_file);
    // measurement resolution for each type is in rainlist_file
    RainDistribution rdist(rainlist_file,gridres);    
    

    //-----------------//
    // conversion loop //
    //-----------------//


    //-----------------------------
    // Read line from geometry file
    //-----------------------------


 
   while(gnf.ReadLine()){
      if(gnf.ctd<min_ctd || gnf.ctd>max_ctd) continue;
      Uniform randdirgen(pi,pi);
      randdirgen.SetSeed(10023455);
      Uniform randspdgen((max_speed-min_speed)/2,(max_speed+min_speed)/2);
      randspdgen.SetSeed(5678943);
      Gaussian noisegen(1,0);
      noisegen.SetSeed(23838478);


      //-------------
      // sample loop
      //--------------
 
      for(int i=0;i<n;i++){

	// get random direction

	float reldir=randdirgen.GetNumber();
        float true_spd=randspdgen.GetNumber();
        float dir=reldir+dir_offset;

        // gets rain rate and sets location in rain maps to use to contaminate this wind cell
        rdist.getRandomRainRate();
        //-------------------------
	// Create Measurement List
        //-------------------------
	float*  mlpinvec_dir = (float*) malloc(sizeof(float)*inMLP);
	float*  mlpinvec = (float*) malloc(sizeof(float)*2*nlooks);
        for(int j=0;j<nlooks;j++){
	  mlpinvec[j]=0; // look by look mean
          mlpinvec[j+nlooks]=0; // look by look variance
	  for(int k=0;k<gnf.nmeas[j];k++){
	    
            // set geometric quantities
	    Meas* meas = new Meas;
	    meas->beamIdx=j/2;
	    meas->scanAngle=(j%2)*pi;
	    meas->eastAzimuth=gnf.azim[j];
            meas->incidenceAngle=gnf.inc[j];
	    meas->startSliceIdx=k;
            meas->numSlices=-1;
            
	    if(gnf.pol[j]=='v' || gnf.pol[j]=='V'){
	      meas->measType=Meas::VV_MEAS_TYPE;
	    }
	    else if(gnf.pol[j]=='h' || gnf.pol[j]=='H'){
	      meas->measType=Meas::HH_MEAS_TYPE;
	    }
 
            /*** HACK ALERT this confusion is necessary in order
		 to use QuikSCAT format model functions 
		 should work for XOVWM and JAXASCAT configurations
                 need to check geomnoise file to make sure ...
                 This works because the CHONLY model functions use
                 the polarization as a proxy for incidence angle
            **/
   	    else if(gnf.pol[j]=='c' || gnf.pol[j]=='C'){
	      if(gnf.inc[j]>52*dtr)
		meas->measType=Meas::C_BAND_VV_MEAS_TYPE;
	      else
		meas->measType=Meas::C_BAND_HH_MEAS_TYPE;
	    }
            else{
	      fprintf(stderr,"Bad Measurement type: %c\n",gnf.pol[j]);
	      exit(0);
	    }

            // set unused quantities
            meas->XK=0;
	    meas->EnSlice=0;
            meas->bandwidth=0;
	    meas->txPulseWidth=0;
	    meas->landFlag=0;
	    meas->azimuth_width=0;
            meas->range_width=0;
          
            rdist.checkMeasTypeMatch(j,meas); // this only checks first meas of each look j

            // get true GMF
            float s0true;
            float chi = dir - meas->eastAzimuth + pi;
	    gmf.GetInterpolatedValue(meas->measType,meas->incidenceAngle,
				 true_spd,chi,&s0true);


            
	    // set value, A, B, C
            
            // fudge factors
	    float SNRff=1;
            double kpm2=0;
            if(sim_kpm){
	      kp.GetKpm2(meas->measType,true_spd,&kpm2);
	    }

            float SNR=s0true/gnf.s0ne[j];
	    SNR*=SNRff;

            float kpc2 = (1.0/gnf.nlpm[j])*(1+2/SNR+1/(SNR*SNR));
	    float kptotal2=kpc2+kpm2;
            float kptotal=sqrt(kptotal2);



	    float noise=noisegen.GetNumber()*s0true*kptotal;
            meas->value=s0true+noise+s0true*gnf.bias[j];

//#define NONOISE
//#ifdef NONOISE
//	    meas->value=s0true;
//#endif
	    mlpinvec[j]+=meas->value;
            mlpinvec[j+nlooks]+=meas->value*meas->value;
            
#ifdef DEBUG
            printf("%g %g %g %g %g\n", s0true, meas->value, kptotal, sqrt(kpc2), sqrt(kpm2));
#endif
            delete meas;
	  } // number of measurements per look loop
          // normalize appropriately for greater than 2 nmeas
          // do nothing for nmeas =1 set to defaults if nmeas==0
          if(gnf.nmeas[j]>2){
	    mlpinvec[j]/=gnf.nmeas[j];
	    mlpinvec[j+nlooks]-=gnf.nmeas[j]*mlpinvec[j]*mlpinvec[j];
	    mlpinvec[j+nlooks]/=(gnf.nmeas[j]-1);
          }
          else if(gnf.nmeas[j]==0){
	    mlpinvec[j]=0;
            mlpinvec[j+nlooks]=0.1;
	  }
          // contaminate with rain
	  rdist.rainContaminateSigma0(j,mlpinvec);
	}// end number of looks/beams
         
        // add samples to dataset
        reldir=reldir*180/pi;
        for(int c=0;c<2*nlooks;c++) mlpinvec_dir[c]=mlpinvec[c];
        mlpinvec_dir[nlooks*2]=0.0;
        for(int c=0;c<NDIR;c++){
          float dirstep=2*pi/NDIR;
          float dir=(c+0.5*dirstep);
          MLP* m=spdarr.getMLP(dir,gnf.ctd);
          float dummyout=0;
          m->Forward(&dummyout,mlpinvec);
	  mlpinvec_dir[nlooks*2]+=m->outp[0];
	}
        mlpinvec_dir[nlooks*2]/=NDIR;
	dirarr.addSampleInOrderAndWrite(0,gnf.ctd,mlpinvec_dir,&reldir);

        free(mlpinvec);
        free(mlpinvec_dir);

	
      } // end number of samples loop
      
      } // end cti loop (line of geomnoise file)


   dirarr.Flush();

   return (0);
}
