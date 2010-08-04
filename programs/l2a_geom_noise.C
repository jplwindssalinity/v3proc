//==========================================================//
// Copyright (C) 1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		l2a_geom_noise
//
// SYNOPSIS
//	      l2a_geom_noise <input_file> <output_file> <nbeams> <res> [ati]
//
// DESCRIPTION
//          Reads l2a file to get geometry and noise info for a specific ati
//          and all cti. Then, output the average info to an ASCII file
//
//      OPTIONS
//
// AUTHOR
//		Samuel Chan
//		samuel.f.chan@jpl.nasa.gov
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
#include <iostream.h>
#include "Constants.h"
#include "Misc.h"
#include "L2A.h"
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Tracking.h"
#include "Tracking.C"

#define XBIN 3000
#define NUM_RANGE_LOOKS_AVERAGED 8
#define MAX_NBEAMS 4
#define NDIRS 2
#define CROSS_DIST 1900  // works for QuikSCAT.
#define CENTER_OFFSET 20

//-----------//
// TEMPLATES //
//-----------//

template class List<EarthPosition>;
template class List<Meas>;
template class List<MeasSpot>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<off_t>;
template class List<OffsetList>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;

const char* usage_array[] = { "<input_file>", "<output_file>",
			      "<nbeams>", "<res>","[compute_ambig]",
			      "[ati]",0};

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
	if (argc < 5)
		usage(command, usage_array, 1);

	int clidx = 1;
	const char* input_file = argv[clidx++];
	const char* output_file = argv[clidx++];
	int nbeams = atoi(argv[clidx++]);
	float res = atof(argv[clidx++]);

	int ati;
        int searchFlag; // 0: not search, use input ati; 1: search first and last ati
        int computeAmbigFlag; // 0 set Signal to Ambiguity Ratio to 15 dB
	// 1 assumes meas->value contains Ambiguity to Signal ratio in linear
        // scale ie. REPLACE_VALUE_WITH_AMBIG = 1 in config file used
        // to generate l2a

       if (argc > 5) {
          computeAmbigFlag= atoi(argv[clidx++]);
        } else {
          computeAmbigFlag = 0;
        }
        if (argc > 6) {
          ati = atoi(argv[clidx++]);
          searchFlag = 0;
        } else {
          ati = 0;
          searchFlag = 1;
        }

	//------------------------//
	// create L2A object      //
	//------------------------//
	L2A l2a;

	//------------------------//
	// open the input file    //
	//------------------------//

 	if (! l2a.OpenForReading(input_file))
	{
		fprintf(stderr, "%s: error opening input file %s\n", command,
			input_file);
		exit(1);
	}

	//------------------------//
	// open the output file   //
	//------------------------//

        FILE *outfileP;
        if ((outfileP = fopen(output_file,"w"))==NULL) {
		fprintf(stderr, "%s: error creating output file %s\n", command,
			output_file);
		exit(1);
        }

        int firstFlag, lastFlag;
        int firstAti, lastAti;
        int flag[MAX_NBEAMS][NDIRS];

        firstFlag = 0;
        lastFlag = 0;
        firstAti = 0;
        lastAti = 0;
        
        // search first and last atis in l2a file
	while (l2a.ReadDataRec() && searchFlag && !(firstFlag && lastFlag))
        {
          if (!firstFlag) {
            firstAti = l2a.frame.ati;
            firstFlag = 1;
          }
          lastAti = l2a.frame.ati;
        }

        if (lastAti > firstAti) {
          lastFlag = 1;
        }

        //cout << "bound atis: " << firstAti << " " << lastAti << endl;

        // modify ati to the right location
        if (searchFlag==1) {

          if (firstFlag && lastFlag) {
            ati = (firstAti+lastAti)/2;
            cout << "Selected ati: " << ati << endl;
          } else {
            cerr << "No ati at middle of swath with all data!!" << endl;
            exit(1);
          }

          // close and reopen l2a file
          l2a.CloseInputFile();
          l2a.OpenForReading(input_file);
          l2a.ReadHeader();

        }

        char pol[MAX_NBEAMS][NDIRS]; // 2nd index is for fore (0) or after (1)
        int nLook[MAX_NBEAMS][NDIRS];
        float ambRatio[MAX_NBEAMS][NDIRS][XBIN];
        int nMeas[MAX_NBEAMS][NDIRS][XBIN];
        float nes0[MAX_NBEAMS][NDIRS][XBIN], azAng[MAX_NBEAMS][NDIRS][XBIN];
        float incAng[MAX_NBEAMS][NDIRS][XBIN];
        float ctd[XBIN];
        // polarization info
        for (int bb=0; bb<nbeams; bb++) {
          for (int ff=0; ff<NDIRS; ff++) {
            if (bb==0) {
              pol[bb][ff] = 'V';
            } else if (bb==1) {
              pol[bb][ff] = 'H';
            }
          }
        }

        // number of look info
        for (int bb=0; bb<nbeams; bb++) {
          for (int ff=0; ff<NDIRS; ff++) {
            nLook[bb][ff] = 0;
          }
        }

        // initialization
        for (int bb=0; bb<nbeams; bb++) {
          for (int ff=0; ff<NDIRS; ff++) {
            for (int ii=0; ii<XBIN; ii++) {

	      if(!computeAmbigFlag) ambRatio[bb][ff][ii] = 15;
	      else ambRatio[bb][ff][ii]=0;
              nMeas[bb][ff][ii] = 0;
              nes0[bb][ff][ii] = 0.;
              azAng[bb][ff][ii] = 0;
              incAng[bb][ff][ii] = 0.;
            }
          }
        }

	//---------------------//
	// copy desired frames //
	//---------------------//
        int foundfirstmeas[2]={0,0};
        int beamidx=0;
	while (l2a.ReadDataRec() && l2a.frame.ati <= ati)
	  { 
	    if (l2a.frame.ati == ati) {
              ctd[l2a.frame.cti]=l2a.getCrossTrackDistance();
	      for (Meas* meas = l2a.frame.measList.GetHead();
		   meas; meas = l2a.frame.measList.GetNext())
		{
		  /** HACK to make code work for single H pol beam **/
		  
		  if(nbeams==2){
		    beamidx=meas->beamIdx;		
		  }
		  if(!foundfirstmeas[beamidx]){
		    if(meas->measType==Meas::VV_MEAS_TYPE){
		      pol[beamidx][0]='V';
		      pol[beamidx][1]='V';
		    }
		    if(meas->measType==Meas::HH_MEAS_TYPE){
		      pol[beamidx][0]='H';
		      pol[beamidx][1]='H';
		    }
		    if(meas->measType==Meas::C_BAND_VV_MEAS_TYPE){
			pol[beamidx][0]='C';
			pol[beamidx][1]='C';
		    }
		    if(meas->measType==Meas::C_BAND_HH_MEAS_TYPE){
		      pol[beamidx][0]='C';
		      pol[beamidx][1]='C';
		    }
		    foundfirstmeas[beamidx]=1;
		  }

		  
		  if (meas->scanAngle <= pi/2. || meas->scanAngle >= 3.*pi/2.) {
		    
		    nLook[beamidx][0] = (int)(1./(meas->A-1.) + 0.5);
		    nMeas[beamidx][0][l2a.frame.cti]++;
		    nes0[beamidx][0][l2a.frame.cti] += meas->EnSlice/meas->XK;
		    azAng[beamidx][0][l2a.frame.cti] += meas->eastAzimuth;
		    incAng[beamidx][0][l2a.frame.cti] += meas->incidenceAngle;
		    if(computeAmbigFlag)
		      ambRatio[beamidx][0][l2a.frame.cti] += meas->value;
		    
		  } else if (meas->scanAngle > pi/2. && meas->scanAngle < 3.*pi/2.) {
		    
		    nLook[beamidx][1] = int(1./(meas->A-1.) + 0.5);
		    nMeas[beamidx][1][l2a.frame.cti]++;
		    nes0[beamidx][1][l2a.frame.cti] += meas->EnSlice/meas->XK;
		    azAng[beamidx][1][l2a.frame.cti] += meas->eastAzimuth;
		    incAng[beamidx][1][l2a.frame.cti] += meas->incidenceAngle;
		    if(computeAmbigFlag)
		      ambRatio[beamidx][1][l2a.frame.cti] += meas->value;
		    
		  }
		  
		} // meas list loop
	      
	      
	      // perform averaging
	      for (int bb=0; bb<nbeams; bb++) {
		for (int ff=0; ff<NDIRS; ff++) {
		  if (nMeas[bb][ff][l2a.frame.cti] > 0) {
		    nes0[bb][ff][l2a.frame.cti] /= nMeas[bb][ff][l2a.frame.cti];
		    nes0[bb][ff][l2a.frame.cti] = 10.*log10(nes0[bb][ff][l2a.frame.cti]);
		    azAng[bb][ff][l2a.frame.cti] /= nMeas[bb][ff][l2a.frame.cti]/rtd;
		    incAng[bb][ff][l2a.frame.cti] /= nMeas[bb][ff][l2a.frame.cti]/rtd;
		    
		    if(computeAmbigFlag){
		      ambRatio[bb][ff][l2a.frame.cti] /= nMeas[bb][ff][l2a.frame.cti];
		      ambRatio[bb][ff][l2a.frame.cti] = -10.*log10(ambRatio[bb][ff][l2a.frame.cti]);
		    }
		  }
		}
	      }

	    } // ati check

	  } // while, reading loop

        // write out info
        fprintf(outfileP, "%d %f\n", nbeams, res);

        for (int ii=0; ii<l2a.header.crossTrackBins; ii++) {
          int totl=0;
          for (int bb=0; bb<nbeams-1; bb++) {
            for (int ff=0; ff<NDIRS; ff++) {
	      totl+=nLook[bb][ff];
	    }
	  }
          if(totl==0) continue;
          fprintf(outfileP, "%f ", ctd[ii]);

          for (int bb=0; bb<nbeams-1; bb++) {
            for (int ff=0; ff<NDIRS; ff++) {
              fprintf(outfileP, "%d %f %d %f %f %c %f ",
                  nMeas[bb][ff][ii], nes0[bb][ff][ii], nLook[bb][ff], azAng[bb][ff][ii],
                  incAng[bb][ff][ii], pol[bb][ff], ambRatio[bb][ff][ii]); 
 
            }
          }

          for (int bb=nbeams-1; bb<nbeams; bb++) {
            fprintf(outfileP, "%d %f %d %f %f %c %f %d %f %d %f %f %c %f\n", 
                nMeas[bb][0][ii], nes0[bb][0][ii], nLook[bb][0], azAng[bb][0][ii],
                incAng[bb][0][ii], pol[bb][0], ambRatio[bb][0][ii],
                nMeas[bb][1][ii], nes0[bb][1][ii], nLook[bb][1], azAng[bb][1][ii],
                incAng[bb][1][ii], pol[bb][1], ambRatio[bb][1][ii]);
          }

        }

        //----------------------//
        // close files and exit //
        //----------------------//

	l2a.Close();
        fclose(outfileP);
        return(0);
}
