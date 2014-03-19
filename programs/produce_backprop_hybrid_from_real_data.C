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
#include "BufferedList.h"
#include "Misc.h"
#include "ConfigList.h"
#include "L2A.h"
#include "ConfigSim.h"
#include "L2B.h"
#include "L2AToL2B.h"
#include "Tracking.h"
#include "MLPData.h"
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
#define NMAXSAMPLES 1000000
#define NDIR 1   // should be 72 if we want MLP by direction
#define CROSSTRACKSPACING 400 // number of cross track bins per MLP 
#define USE_CTD_INPUT 2  // 0 = nothing 1=CTD 2= CTD and RELDIR 
#define NEEDALLLOOKS 1                         
#define OMIT_VAR 0


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

const char* usage_array[] = { "<sim_config_file>", "<max_speed>", "<rainfilelist>","<dataset_output_file>","<max_num_samples_per_cti>","<hidden_num>","[cfgfile1]","[cfgfile2]","[cfgfile3]","...",0};

// This modifies the measlist to correct sigma0 and then returns the obj value
// one would expect for a given direction error
int GetTrueSigma0s(WindVectorPlus* wvp, MeasList* ml,GMF* gmf, float* ts0s){
  int n[8];
  float s0s[8];
  for(int i=0;i<8;i++) {
    ts0s[i]=0.0;
    s0s[i]=0.0;
    n[i]=0;
  }
  for(Meas* m=ml->GetHead();m;m=ml->GetNext()){
    float trues0;
    int look_idx=0;
    float chi = wvp->dir - m->eastAzimuth + pi;
    gmf->GetInterpolatedValue(m->measType,m->incidenceAngle,
				 wvp->spd,chi,&trues0);
    
    
    switch (m->measType)
      {
      case Meas::HH_MEAS_TYPE:
	if (m->scanAngle < pi / 2 || m->scanAngle > 3 * pi / 2)
	  look_idx = 0;
		    
	else
	  look_idx = 1;
	break;
      case Meas::VV_MEAS_TYPE:
	if (m->scanAngle < pi / 2 || m->scanAngle > 3 * pi / 2)
	  look_idx = 2;
	else
	  look_idx = 3;
	break;
      case Meas::C_BAND_HH_MEAS_TYPE:
	if (m->scanAngle < pi / 2 || m->scanAngle > 3 * pi / 2)
	      look_idx = 4;
	else
	  look_idx = 5;
	break;
      case Meas::C_BAND_VV_MEAS_TYPE:
	if (m->scanAngle < pi / 2 || m->scanAngle > 3 * pi / 2)
	  look_idx = 6;
	else
	  look_idx = 7;
	break;
      default:
	look_idx = -1;
	break;
      }
    if (look_idx >= 0)
	  {
	    s0s[look_idx]+=m->value;
	    ts0s[look_idx]+=trues0;
	    n[look_idx]++;
	  }
    else{
      fprintf(stderr,"Warning ... Bad Measurement value =%g  measType =%d eastAzimuth =%g \n",m->value,(int)m->measType,m->eastAzimuth);
    }

  }
  for(int i=0;i<8;i++){
    if(n[i]!=0){
      ts0s[i]/=n[i];
      s0s[i]/=n[i];
      ts0s[i]=fabs(ts0s[i]);
      if(ts0s[i]<0.001) return(0); 
      ts0s[i]=-10*log10(ts0s[i]);
    }
    else{
      ts0s[i]=0;  // dummy empty measurements are set to 0 dB
    }
  }
  
  return(1);
} 

float GetNeuralDirectionOffset(L2A* l2a){
    int n=0;
    float az=0;
    float azave=0;
    MeasList* meas_list= &(l2a->frame.measList);
    int look_idx=0;
    Meas* meas = meas_list->GetHead();
    int nc=meas_list->NodeCount();
    for (int c = 0; c < nc; c++)
      {
	switch (meas->measType)
	  {
	  case Meas::HH_MEAS_TYPE:
	    if (meas->scanAngle < pi / 2 || meas->scanAngle > 3 * pi / 2)
		      look_idx = 0;
		    
	    else
	      look_idx = 1;
	    break;
	  case Meas::VV_MEAS_TYPE:
	    if (meas->scanAngle < pi / 2 || meas->scanAngle > 3 * pi / 2)
	      look_idx = 2;
	    else
	      look_idx = 3;
	    break;
	  case Meas::C_BAND_HH_MEAS_TYPE:
	    if (meas->scanAngle < pi / 2 || meas->scanAngle > 3 * pi / 2)
	      look_idx = 4;
	    else
	      look_idx = 5;
	    break;
	  case Meas::C_BAND_VV_MEAS_TYPE:
	    if (meas->scanAngle < pi / 2 || meas->scanAngle > 3 * pi / 2)
	      look_idx = 6;
	    else
	      look_idx = 7;
	    break;
	  default:
	    look_idx = -1;
	    break;
	  }
	if (look_idx == 3)
	  {
	    az=meas->eastAzimuth;
	    while(az>azave/n+pi) az-=2*pi;
            while(az<azave/n-pi) az+=2*pi;
            n++;
	    azave+=az;
	  }
  
	meas = meas_list->GetNext();
      }
    azave/=n;
    return(-azave);
}


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
    if (argc<8)
        usage(command, usage_array, 1);

    int clidx = 1;
    const char* config_file = argv[clidx++];
    float max_speed=atof(argv[clidx++]);
    if(max_speed<=20 || max_speed>100){
      fprintf(stderr,"Max Speed %g failed sanity check\n",max_speed);
      exit(1);
    }
    const char* rainlist_file = argv[clidx++];

    const char* out_file_base = argv[clidx++];
    int n=atoi(argv[clidx++]);
    int hn=atoi(argv[clidx++]);
    int ndatasets=argc-clidx;
    char** cfgnames=&(argv[clidx]);
    char out_spd_file[200], out_rain_file[200], out_obj_file[200];
    sprintf(out_spd_file,"%s_spd.dat",out_file_base); 
    sprintf(out_rain_file,"%s_rain.dat",out_file_base); 
    sprintf(out_obj_file,"%s_obj.dat",out_file_base); 

    char net_spd_file[200], net_rain_file[200], net_obj_file[200];
    sprintf(net_spd_file,"%s_spd.net",out_file_base); 
    sprintf(net_rain_file,"%s_rain.net",out_file_base); 
    sprintf(net_obj_file,"%s_obj.net",out_file_base); 
    printf("Simulating 0-%g m/s using %d samples\n",max_speed,n);
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
   
    ConfigList cfgl2a;
    // check to make sure other config files are valid
    for(int c=0;c<ndatasets;c++){
      if(!cfgl2a.Read(cfgnames[c])){
        fprintf(stderr, "%s: error reading config file %s\n",
		command, cfgnames[c]);
        exit(1);
      }
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

    int need_all_looks=NEEDALLLOOKS;

    int ncti;
    config_list.GetInt("ANN_NCTI",&ncti);      
    float max_ctd,min_ctd,gridres,atgridres;
    config_list.GetFloat("ANN_MIN_CTD",&min_ctd);
    config_list.GetFloat("ANN_MAX_CTD",&max_ctd);
    config_list.GetFloat("CROSS_TRACK_RESOLUTION",&gridres);
    config_list.GetFloat("ALONG_TRACK_RESOLUTION",&atgridres);

    int CTS=CROSSTRACKSPACING;
    if(ncti<CTS)CTS=ncti;
    if(ncti%CTS!=0){
      int addext=CTS - ncti%CTS;
      int addmin=addext/2;
      int addmax=addext-addmin;
      ncti=ncti+addext;
      max_ctd=max_ctd+addmax*gridres;
      min_ctd=min_ctd-addmin*gridres;
    }
    fprintf(stderr,"NCTI=%d CTDRANGE=[%g,%g]\n",ncti,min_ctd,max_ctd);
    int nbeams,nlooks;
    config_list.GetInt("ANN_NUM_BEAMS",&nbeams); 
    nlooks=nbeams*2;

    fprintf(stderr,"Running with %d beams and %d looks per beam\n",nbeams,nlooks/nbeams);



   
    int inMLP=nlooks*2;
    //int inMLP=nlooks;
    if(OMIT_VAR) inMLP=nlooks;
    if(USE_CTD_INPUT) inMLP++;
    if(USE_CTD_INPUT==2) inMLP++;
     int outMLP=1;
    int MLPdim1length=NDIR;
    float MLPdim1min=0;
    float MLPdim1max=2*pi;
    int MLPdim2length=ncti/CTS;
    float MLPdim2min=min_ctd;
    float MLPdim2max=max_ctd;
    
    MLPDataArray spdarr(out_spd_file,net_spd_file,MLPdim1length,MLPdim1min,MLPdim1max,MLPdim2length,MLPdim2min,MLPdim2max,inMLP,outMLP,n*CTS,"bestspeednet",hn); 
    MLPDataArray objarr(out_obj_file,net_obj_file,MLPdim1length,MLPdim1min,MLPdim1max,MLPdim2length,MLPdim2min,MLPdim2max,3,nlooks,n*CTS,"bestobjnet",hn); 
    spdarr.nepochs=2000;
    spdarr.max_bad_epochs=200; 
    spdarr.vss=false;
    objarr.nepochs=100;
    objarr.max_bad_epochs=200; 
    objarr.vss=false;
    // MLPDataArray rainarr(out_rain_file,net_rain_file,MLPdim1length,MLPdim1min,MLPdim1max,MLPdim2length,MLPdim2min,MLPdim2max,inMLP,outMLP,n*CTS,"rainnet"); 




    // measurement resolution for each type is in rainlist_file
    RainDistribution rdist(rainlist_file,gridres);    

    int nati;
    config_list.GetInt("ANN_NATI",&nati);
      //(int)(two_pi * r1_earth / atgridres + 0.5);
    // Allocate truth arrays
    float** tspd, **tdir;

    //-----------------//
    // conversion loop //
    //-----------------//


    //-----------------------------
    // Read each config file
    // get training data from L2A and Truth field
    //-----------------------------


 
   for(int d=0;d<ndatasets;d++){
     if(!cfgl2a.Read(cfgnames[d])){
        fprintf(stderr, "%s: error reading config file %s\n",
		command, cfgnames[d]);
        exit(1);
     }
     L2A l2a;
     if (! ConfigL2A(&l2a, &cfgl2a))
       {
	 fprintf(stderr, "%s: error configuring Level 2A Product #%d form %s\n",command,d,cfgnames[d]);
	 exit(1);
       }
     l2a.OpenForReading();
     if (! l2a.ReadHeader())
       {
	 fprintf(stderr, "%s: error reading Level 2A header of file %s\n", command,cfgnames[d]);
	 exit(1);
       }


 
    if(d==0){
      tspd=(float**) make_array(sizeof(float),2,nati,ncti);
      tdir=(float**) make_array(sizeof(float),2,nati,ncti);
      if (tspd==NULL || tdir==NULL){
	fprintf(stderr,"Error allocating truth arrays\n");
	exit(1);
      }    
    }

    if(l2a.header.crossTrackBins!=ncti){
      fprintf(stderr,"Mismatch between ANN and L2A nctis\n");
      exit(1);
    }

    // read truth windfield
    WindVectorField truthVctrField;
    WindField truthField;
    int tmp_int;
    if (! cfgl2a.GetInt("VCTR_TRUTH_FIELD", &tmp_int)){
      fprintf(stderr,"VCTR_TRUTH_FIELD keyword missing in %s\n",cfgnames[d]);
      exit(1);
    }
            
    int smartTruthFlag = tmp_int;

    cfgl2a.DoNothingForMissingKeywords();
    if (! cfgl2a.GetInt("ARRAY_TRUTH_FIELD", &tmp_int))
      tmp_int=0;
    int arrayTruthFlag = tmp_int;

    if(arrayTruthFlag && smartTruthFlag){
	  fprintf(stderr,"Use either SMART or ARRAY nudging but not both!\n");
	  exit(1);
	}

    cfgl2a.ExitForMissingKeywords();
    //-----------------------//
    // configure truth field //
    //-----------------------//

    char* truth_type = cfgl2a.Get(TRUTH_WIND_TYPE_KEYWORD);
    if (truth_type == NULL)
      return(0);

    if (strcasecmp(truth_type, "SV") == 0)
        {
	  if (!cfgl2a.GetFloat(WIND_FIELD_LAT_MIN_KEYWORD,
				&truthField.lat_min) ||
	      !cfgl2a.GetFloat(WIND_FIELD_LAT_MAX_KEYWORD,
				&truthField.lat_max) ||
	      !cfgl2a.GetFloat(WIND_FIELD_LON_MIN_KEYWORD,
				&truthField.lon_min) ||
	      !cfgl2a.GetFloat(WIND_FIELD_LON_MAX_KEYWORD,
				&truthField.lon_max))
            {
              fprintf(stderr, "ConfigTruthWindField: SV can't determine range of lat and lon\n");
              return(0);
            }
        }

        char* truth_windfield = cfgl2a.Get(TRUTH_WIND_FILE_KEYWORD);
        if (truth_windfield == NULL)
            return(0);

        if (smartTruthFlag)
        {
            if (!cfgl2a.GetFloat(WIND_FIELD_LAT_MIN_KEYWORD,
                                       &truthVctrField.latMin) ||
                !cfgl2a.GetFloat(WIND_FIELD_LAT_MAX_KEYWORD,
                                       &truthVctrField.latMax) ||
                !cfgl2a.GetFloat(WIND_FIELD_LON_MIN_KEYWORD,
                                       &truthVctrField.lonMin) ||
                !cfgl2a.GetFloat(WIND_FIELD_LON_MAX_KEYWORD,
                                       &truthVctrField.lonMax))
            {
              fprintf(stderr, "Config TruthVectorField:  can't determine range of lat and lon\n");
              return(0);
            }
            truthVctrField.lonMax*=dtr;
            truthVctrField.lonMin*=dtr;
            truthVctrField.latMax*=dtr;
            truthVctrField.latMin*=dtr;
            if (truthVctrField.ReadVctr(truth_windfield)){
	      fprintf(stderr,"Error reading truth vctr field %s from %s\n",
		      truth_windfield,cfgnames[d]);
	    }
        }
        else if(arrayTruthFlag){
	  FILE * ifp=fopen(truth_windfield,"r");
	  if(ifp==NULL){
	    fprintf(stderr,"Cannot open file Truth Array file %s from %s\n",truth_windfield,cfgnames[d]);
	    exit(1);
	  }
	  int ati1;
	  int arrnati,arrncti;
	  if( !fread(&ati1,sizeof(int),1,ifp)==1 ||
	      !fread(&arrnati,sizeof(int),1,ifp)==1 ||
	      !fread(&arrncti,sizeof(int),1,ifp)==1 ){
	    fprintf(stderr,"Error reading Truth Array file %s from %s\n",truth_windfield,cfgnames[d]);
	    exit(1);
	  }
	  if(arrncti!=ncti || ati1+arrnati>nati){
	    fprintf(stderr,"Array size mismatch in file %s from %s\n",truth_windfield,cfgnames[d]);
	    exit(1);
	  }
	  for(int a=0;a<nati;a++){
	    for(int c=0;c<ncti;c++){
	      tspd[a][c]=-1;
	      tdir[a][c]=0;
	    }
	  }
	  if( ! read_array(ifp,&tspd[ati1],sizeof(float),2,arrnati,ncti) ||
	      ! read_array(ifp,&tdir[ati1],sizeof(float),2,arrnati,ncti)){

	    fprintf(stderr,"Error reading arrays in %s from %s\n",truth_windfield,cfgnames[d]);
	    exit(1);
 
	  }

        }
        else
        {
            if (! truthField.ReadType(truth_windfield, truth_type))
	      {
		fprintf(stderr,"Error reading wind feild file %s from %s\n",truth_windfield,cfgnames[d]);
		exit(1);
	      }

            //-------------------//
            // Scale Wind Speeds //
            //-------------------//

            cfgl2a.DoNothingForMissingKeywords();
            float scale;
            if (cfgl2a.GetFloat(TRUTH_WIND_SPEED_MULTIPLIER_KEYWORD,
				 &scale))
	      {
                truthField.ScaleSpeed(scale);
	      }
            cfgl2a.ExitForMissingKeywords();
        }

    for(;;){
        if (! l2a.ReadDataRec())
        {
            switch (l2a.GetStatus())
            {
            case L2A::OK:        // end of file
                break;
            case L2A::ERROR_READING_FRAME:
                fprintf(stderr, "%s: error reading Level 2A data\n", command);
                exit(1);
                break;
            case L2A::ERROR_UNKNOWN:
                fprintf(stderr, "%s: unknown error reading Level 2A data\n",
                    command);
                exit(1);
                break;
            default:
                fprintf(stderr, "%s: unknown status\n", command);
                exit(1);
            }
            break;        // done, exit do loop
        }

	MeasList* meas_list = &(l2a.frame.measList);

	//-----------------------------------//
	// check for missing wind field data //
	//-----------------------------------//
	// this should be handled by some kind of a flag!

	int any_zero = 0;
	for (Meas* meas = meas_list->GetHead(); meas; meas = meas_list->GetNext())
	  {
	    if (! meas->value)
	      {
		any_zero = 1;
		break;
	      }
	  }
	if (any_zero)
	  {
	    continue;
	  }
	//-----------------------------------//
	// check for wind retrieval criteria //
	//-----------------------------------//
	
	if (! gmf.CheckRetrieveCriteria(meas_list))
	  {
	    continue;
	  }

        //---------//
        // convert //
        //---------//
	double  ctd=(l2a.frame.cti+0.5)*l2a.header.crossTrackResolution-950.0;
	if(ctd<min_ctd || ctd>max_ctd) continue;


      
    
        LonLat lonLat=meas_list->AverageLonLat();
        WindVectorPlus twvp;
	if(smartTruthFlag){
      if (! truthVctrField.InterpolateVectorField(lonLat,
						  &twvp,0))
	  {
	    continue;  // skip WVC if no truth
	  }
	}
	else if ( arrayTruthFlag){
	  
	  twvp.spd=tspd[l2a.frame.ati][l2a.frame.cti];
	  if(twvp.spd<0){
	    continue;  // skip WVC if no truth
	  }
	  else{
	    twvp.dir=tdir[l2a.frame.ati][l2a.frame.cti];
	  }
	}
	else if (! truthField.InterpolatedWindVector(lonLat,
						 &twvp))
	  {
	    continue;  // skip WVC if no truth
	  }

        if(twvp.spd<=0) continue;

        //-------------------------
	// Create ANN input vector
        //-------------------------
	// determine truth wind direction relative to swath
	float diroff=GetNeuralDirectionOffset(&l2a); 
	float reldir=diroff+twvp.dir; 
	while(reldir<0)reldir+=2*pi;
	while(reldir>2*pi)reldir-=2*pi;
	int nmeas[8]={0,0,0,0,0,0,0,0};
        float trues0s[8]={0,0,0,0,0,0,0,0};
	float mlpinvec[18]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
        float objinvec[3]={0,0,0};
        // uses largest possible array size to avoid array errors
        for(int j=0;j<nlooks;j++){
	  mlpinvec[j]=0; // look by look mean
          mlpinvec[j+nlooks]=0; // look by look variance
	}
        Meas* meas=meas_list->GetHead();

	for(int k=0;k<meas_list->NodeCount();k++){
	  int look_idx=-1;
	  switch (meas->measType)
	    {
	    case Meas::HH_MEAS_TYPE:
	      if (meas->scanAngle < pi / 2 || meas->scanAngle > 3 * pi / 2)
		look_idx = 0;
	      
	      else
		look_idx = 1;
	      break;
	    case Meas::VV_MEAS_TYPE:
	      if (meas->scanAngle < pi / 2 || meas->scanAngle > 3 * pi / 2)
		look_idx = 2;
	      else
		look_idx = 3;
	      break;
	    case Meas::C_BAND_HH_MEAS_TYPE:
	      if (meas->scanAngle < pi / 2 || meas->scanAngle > 3 * pi / 2)
		look_idx = 4;
	      else
		look_idx = 5;
	      break;
	    case Meas::C_BAND_VV_MEAS_TYPE:
	      if (meas->scanAngle < pi / 2 || meas->scanAngle > 3 * pi / 2)
		look_idx = 6;
	      else
		look_idx = 7;
	      break;
	    default:
	      look_idx = -1;
	      break;
	    }
	  if( look_idx >= 4 && nlooks==4){
	    fprintf(stderr,"Number of looks mismatch\n");
	    exit(1);
	  }  
 	  
	  if (look_idx >= 0)
	    {
	      if(!OMIT_VAR){
		mlpinvec[look_idx+nlooks] += meas->value*meas->value;
	      }
  
	    
	      mlpinvec[look_idx] += meas->value;
	      nmeas[look_idx]++;
	    }
	  meas = meas_list->GetNext();
	}
        int look_not_found=0;
	for(int i=0;i<nlooks;i++){
	  
	  if(nmeas[i] > 1){
	    mlpinvec[i]/=nmeas[i];
	    if(!OMIT_VAR){
	      mlpinvec[i+nlooks]/=nmeas[i];
	    }
	  }
	  else if(nmeas[i]==0){
	    if(need_all_looks || i==3 ){
	      look_not_found=1;
	    }
	    mlpinvec[i]=0;
	    if(!OMIT_VAR)
	      mlpinvec[i]=0.0;
	      mlpinvec[i+nlooks]=0.1;
	  }
	}
        if (look_not_found) continue;
	// inpctd_case
 	if(USE_CTD_INPUT){
	  mlpinvec[inMLP-1]=ctd;
	}
	// inpctd and reldir
	if(USE_CTD_INPUT==2){
	  mlpinvec[inMLP-2]=reldir;
	}

          


	// contaminate with rain
        for(int j=0;j<nlooks;j++)
	  rdist.rainContaminateSigma0(j,mlpinvec);

    
         
        // add samples to dataset

	spdarr.addSample(reldir,ctd,mlpinvec,&(twvp.spd));
        float sums0=0.0;
        for(int j=0;j<nlooks;j++){
	  trues0s[j]=mlpinvec[j];
	  sums0+=trues0s[j]*trues0s[j];
	}
        sums0=sqrt(sums0/nlooks);
        for(int j=0;j<nlooks;j++){
	  trues0s[j]/=sums0;
	}       
        objinvec[0]=twvp.spd;
        objinvec[1]=reldir;
	objinvec[2]=ctd;
	objarr.addSample(reldir,ctd,objinvec,trues0s); 
	//rainarr.addSampleInOrderAndWrite(reldir,gnf.ctd,mlpinvec,&true_rain);
    } // end L2A records loop

   } // end data sets loop

   
   objarr.Train();
   spdarr.Train();
   free_array(tspd,2,nati,ncti);
   free_array(tdir,2,nati,ncti);
   return (0);
}
