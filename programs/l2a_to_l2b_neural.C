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
#include "BufferedList.h"
#include "Misc.h"
#include "ConfigList.h"
#include "L2A.h"
#include "ConfigSim.h"
#include "L2B.h"
#include "L2AToL2B.h"
#include "Tracking.h"
#include "Array.h"
#include "MLPData.h"

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

const char* usage_array[] = {"<sim_config_file>", "<speednet>","<dirnet (NONE if not used)>","<spdat_outfile>","[need_all_looks, default=0]",0};


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
  FILE* dirdiagfp=NULL;
  // comment out if you don't want debug output
  dirdiagfp=fopen("DIRDIAG.TXT","w");
  
  int clidx=1;
    const char* command = no_path(argv[0]);
    if (argc !=5 && argc!=6)
        usage(command, usage_array, 1);

  
    const char* config_file = argv[clidx++];
    char* spdnetfile=argv[clidx++];
    char* dirnetfile=argv[clidx++];
    char* spdoutfile=argv[clidx++];
    int need_all_looks=0;
    if(argc==6) need_all_looks=1;
    // open speed output file and neural net files
    FILE* ofp=fopen(spdoutfile,"w");
    if(ofp==NULL){
      fprintf(stderr,"Unable to create output file %s\n",spdoutfile);
      exit(1);
    }

    MLPDataArray spdnet(spdnetfile);
    
    
    MLPDataArray* dirnet=NULL;
    if(strcasecmp(dirnetfile,"NONE")!=0) dirnet= new MLPDataArray(dirnetfile);
 
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

    //--------------//
    // configure Kp //
    //--------------//

    Kp kp;
    if (! ConfigKp(&kp, &config_list))
    {
        fprintf(stderr, "%s: error configuring Kp\n", command);
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

   
    //-----------------------------------------//
    // transfer information to level 2B header //
    //-----------------------------------------//

    l2b.header.crossTrackResolution = l2a.header.crossTrackResolution;
    l2b.header.alongTrackResolution = l2a.header.alongTrackResolution;
    l2b.header.zeroIndex = l2a.header.zeroIndex;


    // Set up array for holding direction only retrieval
    float* spddat=(float*)malloc(sizeof(float)*l2a.header.crossTrackBins*along_track_bins);
    float* lat=(float*)malloc(sizeof(float)*l2a.header.crossTrackBins*along_track_bins);
    float* lon=(float*)malloc(sizeof(float)*l2a.header.crossTrackBins*along_track_bins);

    int frame_number=0;
    int ignore_bad_l2a=0;
    //-----------------//
    // conversion loop //
    //-----------------//
    int nsamps=0;
    for (;;)
    {
        frame_number++;
 
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


 
	int ai=l2a.frame.ati;
        int ci=l2a.frame.cti;

	spddat[nsamps]=l2a_to_l2b.NeuralNetRetrieve(&l2a,&l2b,&spdnet,dirnet,&gmf,&kp,need_all_looks);
      
	WVC* wvc=l2b.frame.swath.GetWVC(ci,ai);
        if(wvc){
	  lat[nsamps]=wvc->lonLat.latitude*rtd;
	  lon[nsamps]=wvc->lonLat.longitude*rtd;
	  nsamps++;
	}
        if(dirdiagfp && wvc){
          // FORMAT IS
          // ATI, CTI, NUMAMBIGS, 80perwidth, spd1 dir1 left1 right1,...
          // objs(72) spds(72)
          
	  fprintf(dirdiagfp,"%d %d ",ai,ci);
	  WindVectorPlus* wvp=wvc->ambiguities.GetHead();
          
          int namb=wvc->ambiguities.NodeCount();
          float _dir[4],_spd[4],_obj[4],_right[4],_left[4];
          float width=0;
          for(int c=0;c<4;c++){
	    if(wvp==NULL){
	      _spd[c]=-1;
	      _dir[c]=0;
	      _obj[c]=0;
	      _right[c]=0;
	      _left[c]=0;
	    }
	    else{
	      _spd[c]=wvp->spd;
              _dir[c]=wvp->dir*180/pi;
	      _obj[c]=wvp->obj;

	      AngleInterval* alist=wvc->directionRanges.GetByIndex(c);
              if(!alist){
		_left[c]=0;
		_right[c]=0;
	      }
              else{
		_left[c]=alist->left;
		_right[c]=alist->right;
	      }
              width+=ANGDIF(_left[c],_right[c]);
              _left[c]*=180/pi;
              _right[c]*=180/pi;
              wvp=wvc->ambiguities.GetNext();

	    }
          }
          fprintf(dirdiagfp,"%d %g ",namb,width*180/pi);
	  for(int c=0;c<4;c++){
	    fprintf(dirdiagfp,"%g %g %g %g %g ",_spd[c],_dir[c],_obj[c],_left[c],_right[c]);
	  }
	  for(int p=0;p<72;p++){
	    fprintf(dirdiagfp,"%g ",wvc->directionRanges.bestObj[p]);
	  }
          for(int p=0;p<72;p++){
	    fprintf(dirdiagfp,"%g ",wvc->directionRanges.bestSpd[p]);
	  }
		  fprintf(dirdiagfp,"\n");
	} // end diagnostic output section
    } // end loop over wind vector cells

   // write out spddat file
   if(fwrite(&nsamps,sizeof(int),1,ofp)!=1)
    {
      fprintf(stderr,"Error writing header to outfile %s\n", spdoutfile);
      exit(1);
    }
   if(fwrite(lat,sizeof(int),nsamps,ofp)!=(unsigned int) nsamps ||  
      fwrite(lon,sizeof(int),nsamps,ofp)!=(unsigned int) nsamps ||
      fwrite(spddat,sizeof(int),nsamps,ofp)!=(unsigned int) nsamps){
     fprintf(stderr,"Error writing data to outfile %s\n",spdoutfile);
     exit(1);
   }
   fclose(ofp);
   if(dirdiagfp) fclose(dirdiagfp);
   l2b.frame.swath.nudgeVectorsRead=1;
   l2a_to_l2b.InitFilterAndFlush(&l2b);

   l2a.Close();
   l2b.Close();


   free(spddat);
   free(lat);
   free(lon);
   if(dirnet) delete dirnet;
   return (0);
}
