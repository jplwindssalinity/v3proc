//==============================================================//
// Copyright (C) 1997-2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    l2a_to_l2b
//
// SYNOPSIS
//    l2a_to_l2b [ -a start:end ] [ -n num_frames ] [ -i ] <sim_config_file>
//
// DESCRIPTION
//    Simulates the SeaWinds 1b ground processing of Level 2A to
//    Level 2B data.  This program retrieves wind from measurements.
//
// OPTIONS
//    [ -a start:end ]  The range of along track index.
//    [ -n num_frames ] The maximum frame number.
//    [ -i ]            Ignore bad l2a.
//    [ -R ]     Remove measurements more than 10 stds from average
//
// OPERANDS
//    The following operand is supported:
//      <sim_config_file>  The sim_config_file needed listing
//                         all input parameters, input files, and
//                         output files.
//
// EXAMPLES
//    An example of a command line is:
//      % l2a_to_l2b sws1b.cfg
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
//    James N. Huddleston (James.N.Huddleston@jpl.nasa.gov)
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
#include "Array.h"

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

#define MAX_ALONG_TRACK_BINS  1624
#define OPTSTRING "iWRa:n:w:"

//-------//
// HACKS //
//-------//

//#define LATLON_LIMIT_HACK

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

const char* usage_array[] = {"[ -a start:end ]", "[-R]","[ -n num_frames ]", "[ -i ]","[ -W ]", "[ -w c_band_weight_file ]" , "<sim_config_file>", 0};


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
    int frame_number =0;
    int ignore_bad_l2a=0;
    int use_freq_weights=0;
    int calc_cband_weights=0; // new
    long int max_record_no = 0;
    char weight_file[200];
    float** weights=NULL;
    FILE* wfp;
    int ncti_wt=0, nati_wt=0, first_valid_ati=0, nvalid_ati=0;
    bool opt_remove_outlying_s0=false;
    
    const char* command = no_path(argv[0]);
    if (argc < 2)
        usage(command, usage_array, 1);

    long int start_ati = -100000000;
    long int end_ati = 100000000;
    
    fprintf(stderr, 
        "******************************** WARNING **********************************\n"
        "*** hurricane_l2b_preproc is depreciated. DO NOT USE. Run l2a_to_l2b    ***\n"
        "*** with the '-t train_set_fn' option for the equivalent functionality. ***\n");

    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
      switch(c)
      {
        case 'a':
          if (sscanf(optarg, "%ld:%ld", &start_ati, &end_ati) != 2)
          {
            fprintf(stderr, "%s: error determining ati range %s\n",
                command, optarg);
            exit(1);
          }
          break;

        case 'R':
            opt_remove_outlying_s0=true;
            break;
            
        case 'n':
          if (sscanf(optarg, "%ld", &max_record_no) != 1)
          {
            fprintf(stderr, "%s: error determining max frame number %s\n",
                command, optarg);
            exit(1);
          }
          break;

        case 'w':
          if (sscanf(optarg, "%s", weight_file) != 1)
          {
            fprintf(stderr, "%s: error parsing weight file name %s\n",
                command, optarg);
            exit(1);
          }

	  wfp=fopen(weight_file,"r");
	  if(!wfp){
	    fprintf(stderr,"Cannot open file %s\n",weight_file);
	    exit(1);
	  }
	  if(fread(&first_valid_ati,sizeof(int),1,wfp)!=1  ||
	     fread(&nvalid_ati,sizeof(int),1,wfp)!=1  ||
	     fread(&ncti_wt,sizeof(int),1,wfp)!=1){
	    fprintf(stderr,"Error reading from file %s\n",weight_file);
	    exit(1);
	  }
	  nati_wt=first_valid_ati+nvalid_ati;
	  weights=(float**)make_array(sizeof(float),2,nati_wt,ncti_wt);
	  for(int a=0;a<nati_wt;a++){
	    for(int c=0;c<ncti_wt;c++){
	      weights[a][c]=0.5;
	    }
	  }
	  if(!read_array(wfp,&weights[first_valid_ati],sizeof(float),2,nvalid_ati,ncti_wt)){
	    fprintf(stderr,"Error reading from file %s\n",weight_file);
	    exit(1);
	  }
          use_freq_weights = 1;
	  fclose(wfp);
	  break;

        case 'W':
          calc_cband_weights = 1;
          break;
        case 'i':
          ignore_bad_l2a=1;
          break;
        case '?':
          usage(command, usage_array, 1);
          break;
      }
    }

    if (argc != optind + 1)
      usage(command, usage_array, 1);

    const char* config_file = argv[optind++];

    //------------------------//
    // tell how far you have  //
    // gotten if you recieve  //
    // the siguser1 signal    //
    //------------------------//

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
    // create and configure level products //
    //-------------------------------------//

    L2A l2a;
    if (! ConfigL2A(&l2a, &config_list))
    {
        fprintf(stderr, "%s: error configuring Level 2A Product\n", command);
        exit(1);
    }

    L2B l2b;
    if (! ConfigL2B(&l2b, &config_list))
    {
        fprintf(stderr, "%s: error configuring Level 2B Product\n", command);
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

    //--------------//
    // configure Kp //
    //--------------//

    Kp kp;
    if (! ConfigKp(&kp, &config_list))
    {
        fprintf(stderr, "%s: error configuring Kp\n", command);
        exit(1);
    }

    //------------------------------------//
    // create and configure the converter //
    //------------------------------------//

    L2AToL2B l2a_to_l2b;
    if (! ConfigL2AToL2B(&l2a_to_l2b, &config_list))
    {
        fprintf(stderr, "%s: error configuring L2AToL2B\n", command);
        exit(1);
    }

    //------------//
    // open files //
    //------------//

    l2a.OpenForReading();
    l2b.OpenForWriting();

    //---------------------------------//
    // read the header to set up swath //
    //---------------------------------//

    if (! l2a.ReadHeader())
    {
        fprintf(stderr, "%s: error reading Level 2A header\n", command);
        exit(1);
    }

    int along_track_bins =
        (int)(two_pi * r1_earth / l2a.header.alongTrackResolution + 0.5);
 
    if (! l2b.frame.swath.Allocate(l2a.header.crossTrackBins,
        along_track_bins))
    {
        fprintf(stderr, "%s: error allocating wind swath\n", command);
        exit(1);
    }

    if(use_freq_weights && ( l2a.header.crossTrackBins!=ncti_wt )){
      fprintf(stderr,"Error Size mismatch between L2A and weights file\n");
      fprintf(stderr,"L2A NATI=%d NCTI=%d, CBandWeights NATI = %d NCTI=%d\n",
	      l2a.header.alongTrackBins,l2a.header.crossTrackBins,nati_wt,ncti_wt);
      exit(1);
    } 

    //-----------------------------------------//
    // transfer information to level 2B header //
    //-----------------------------------------//

    l2b.header.crossTrackResolution = l2a.header.crossTrackResolution;
    l2b.header.alongTrackResolution = l2a.header.alongTrackResolution;
    l2b.header.zeroIndex = l2a.header.zeroIndex;

    //-----------------//
    // conversion loop //
    //-----------------//

    for (;;)
    {
        frame_number++;
        if (max_record_no > 0 && frame_number > max_record_no) break;

        //-----------------------------//
        // read a level 2A data record //
        //-----------------------------//

        if (! l2a.ReadDataRec())
        {
            switch (l2a.GetStatus())
            {
            case L2A::OK:        // end of file
                break;
            case L2A::ERROR_READING_FRAME:
                fprintf(stderr, "%s: error reading Level 2A data\n", command);
                if(!ignore_bad_l2a) exit(1);
                break;
            case L2A::ERROR_UNKNOWN:
                fprintf(stderr, "%s: unknown error reading Level 2A data\n",
                    command);
                if(!ignore_bad_l2a) exit(1);
                break;
            default:
                fprintf(stderr, "%s: unknown status\n", command);
                if(!ignore_bad_l2a) exit(1);
            }
            break;        // done, exit do loop
        }

        //---------//
        // convert //
        //---------//

#ifdef LATLON_LIMIT_HACK
        // start hack
        Meas* tstmeas = l2a.frame.measList.GetHead();
        double alt, lat, lon;
        if (! tstmeas)
        {
            printf("NULL MeasList \n");
            continue;
        }
        else
        {
            tstmeas->centroid.GetAltLonGDLat(&alt, &lon, &lat);
            lon*=rtd;
            lat*=rtd;
        }
        if (lat < 23.80 && lat > 23.78 && lon < 296.06 && lon > 296.04)
        {
        // end hack
#endif
	if(use_freq_weights)
	  gmf.SetCBandWeight(weights[l2a.frame.ati][l2a.frame.cti]);

        if (calc_cband_weights) {
//  first find all C-band measurements and average the sigma0

            float ave_cband_V = 0.0;  // "V-pol" is 54-deg H-pol in sim
            float ave_cband_H = 0.0;  // "H-pol" is 46-deg H-pol in sim
            int n_cband_V = 0;
            int n_cband_H = 0;
    
            for (Meas* meas = l2a.frame.measList.GetHead(); meas; meas = l2a.frame.measList.GetNext()){
                if (meas->measType==Meas::C_BAND_VV_MEAS_TYPE){
                    n_cband_V++;
                    ave_cband_V += meas->value;
                }
            
                if (meas->measType==Meas::C_BAND_HH_MEAS_TYPE){
                    n_cband_H++;
                    ave_cband_H += meas->value;
                }
            
            }
            if (n_cband_V){
                ave_cband_V /= n_cband_V;
            }
            if (n_cband_H){
                ave_cband_H /= n_cband_H;
            }
    

// then determine weight based on value of mean sigma0 relative to
// threshold value (weight at threshold = 0.5??).  threshold determined
// by [tbd] the a0 at the speed at which the Ku-band model becomes double-
// valued.
            float thr_V = 0.013;
            float thr_H = 0.04;
            float t = 0.0;
            float wt = 0.0;
    
// weight ranges from 0.1 to 0.9 with the 0.5 point at the threshold

            t = 0.5*((ave_cband_V/thr_V) + (ave_cband_H/thr_H));
            if (t < 0.1) t = 0.1;
    
            wt = t/(1.0 + t);

// use GMF::SetCBandWeight() to set the relative C and Ku weights used in wind retrieval
            gmf.SetCBandWeight(wt);
    
        }

        int retval = 1;
        if(opt_remove_outlying_s0) gmf.RemoveBadCopol(&(l2a.frame.measList),&kp);
        
        if (l2a.frame.ati >= start_ati && l2a.frame.ati <= end_ati) {
          retval = l2a_to_l2b.ConvertAndWrite(&l2a, &gmf, &kp, &l2b);
          if(frame_number%100==0) fprintf(stderr,"%d l2a frames processed\n",
					frame_number);
        } else if (l2a.frame.ati > end_ati) {
          break;
        }
        int ai=l2a.frame.ati;
        int ci=l2a.frame.cti;
        WVC* wvc0=l2b.frame.swath.GetWVC(ci,ai); 

        if(wvc0){
          // nudge by hand
          MeasList* meas_list = &(l2a.frame.measList);

	  wvc0->lonLat=meas_list->AverageLonLat();
	  wvc0->nudgeWV=new WindVectorPlus;
	  if (! l2a_to_l2b.nudgeField.InterpolatedWindVector(wvc0->lonLat,
						  wvc0->nudgeWV))
	  {
	    delete wvc0->nudgeWV;
	    wvc0->nudgeWV=NULL;
	  }
	  if(wvc0->ambiguities.NodeCount() && wvc0->nudgeWV){
            // compute diagnostic data
 	    int nc = meas_list->NodeCount();
	    int* look_idx = new int[nc];
            int nmeas[8]={0,0,0,0,0,0,0,0};
	    float varest[8]={0,0,0,0,0,0,0,0};
            float meanest[8]={0,0,0,0,0,0,0,0};
	    Meas* meas = meas_list->GetHead();
	    for (int c = 0; c < nc; c++)
	      {
		switch (meas->measType)
		  {
		  case Meas::HH_MEAS_TYPE:
		    if (meas->scanAngle < pi / 2 || meas->scanAngle > 3 * pi / 2)
		      look_idx[c] = 0;
		    
		    else
		      look_idx[c] = 1;
		    break;
		  case Meas::VV_MEAS_TYPE:
		    if (meas->scanAngle < pi / 2 || meas->scanAngle > 3 * pi / 2)
		      look_idx[c] = 2;
		    else
		      look_idx[c] = 3;
		    break;
		  case Meas::C_BAND_HH_MEAS_TYPE:
		    if (meas->scanAngle < pi / 2 || meas->scanAngle > 3 * pi / 2)
		      look_idx[c] = 4;
		    else
		      look_idx[c] = 5;
		    break;
		  case Meas::C_BAND_VV_MEAS_TYPE:
		    if (meas->scanAngle < pi / 2 || meas->scanAngle > 3 * pi / 2)
		      look_idx[c] = 6;
		    else
		      look_idx[c] = 7;
		    break;
		  default:
		    look_idx[c] = -1;
		    break;
		  }
		if (look_idx[c] >= 0)
		  {
		    varest[look_idx[c]] += meas->value*meas->value;
		    meanest[look_idx[c]] += meas->value;
		    nmeas[look_idx[c]]++;
		  }
		meas = meas_list->GetNext();
	      }
	    printf("%d %d %g %g",ai, ci,l2a.getCrossTrackDistance(), wvc0->rainProb);
	  
	    for(int i=0;i<8;i++){
	      meanest[i]/=nmeas[i];
	      varest[i]=(varest[i]-nmeas[i]*meanest[i]*meanest[i])/(nmeas[i]-1);
	      if(nmeas[i]<1) meanest[i]=0;
	      if(nmeas[i]<2) varest[i]=0.1;
	      printf(" %d %g %g",nmeas[i],meanest[i],varest[i]);
	    }
            printf(" %g %g\n",wvc0->nudgeWV->spd,wvc0->nudgeWV->dir*pi/180);
	  }
	}
        fflush(stdout);
	
#ifdef LATLON_LIMIT_HACK
        // start hack
        if (retval != 1)
        {
            printf("%g lat %g lon   retval=%d\n", lat, lon, retval);
        }
        // end hack
#endif

        switch (retval)
        {
        case 1:
            break;
        case 2:
            break;
        case 4:
        case 5:
            break;
        case 0:
            fprintf(stderr, "%s: error converting Level 2A to Level 2B\n",
                command);
            exit(1);
            break;
        }

#ifdef LATLON_LIMIT_HACK
        // start hack
        }
        // end hack
#endif

    }

    l2a_to_l2b.InitFilterAndFlush(&l2b);

    l2a.Close();
    l2b.Close();

    free_array(weights,2,nati_wt,ncti_wt);
    return (0);
}
