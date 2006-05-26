//==========================================================//
// Copyright (C) 1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		l2a_paras
//
// SYNOPSIS
//	      l2a_paras <input_file> <output_file>  <start_frame> <end_frame>
//
// DESCRIPTION
//          Reads frames start_frame through end_frame from a L2A file and
//          writes some parameters to an ASCII file
//      OPTIONS
//		Last two arguments are optional
// AUTHOR
//		Bryan Stiles
//		bstiles@acid.jpl.nasa.gov
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
#include "Constants.h"
#include "Misc.h"
#include "L2A.h"
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Tracking.h"
#include "Tracking.C"

#define XBIN 1000
#define YBIN 1500
#define SYSTEM_TEMPERATURE 926.0 // in K 
#define NUM_RANGE_LOOKS_AVERAGED 8

//-----------//
// TEMPLATES //
//-----------//

template class List<EarthPosition>;
template class List<Meas>;
template class List<MeasSpot>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<long>;
template class List<OffsetList>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;

const char* usage_array[] = { "<input_file>", "<output_file>",
			      "<start_frame>(OPT)",
			      "<end_frame>(OPT)",0};

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
	if (argc != 5 && argc!=3)
		usage(command, usage_array, 1);

	int clidx = 1;
	const char* input_file = argv[clidx++];
	const char* output_file = argv[clidx++];
        int start_frame=-1, end_frame=2;
        if(argc==5){
	  start_frame=atoi(argv[clidx++]);
	  end_frame=atoi(argv[clidx++]);
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

 	//if (! l2a.OpenForWriting(output_file))
	//{
		//fprintf(stderr, "%s: error creating output file %s\n", command,
			//output_file);
		//exit(1);
	//}       

        int nWVC_OF[XBIN], nWVC_OA[XBIN], nWVC_IF[XBIN], nWVC_IA[XBIN]; 
        float nLookOF[XBIN], nLookOA[XBIN], nLookIF[XBIN], nLookIA[XBIN]; 
        float SNR_IF[XBIN], SNR_IA[XBIN], SNR_OF[XBIN], SNR_OA[XBIN];
        float kpc_IF[XBIN], kpc_IA[XBIN], kpc_OF[XBIN], kpc_OA[XBIN];
        float azres_IF[XBIN], azres_IA[XBIN], azres_OF[XBIN], azres_OA[XBIN];

        for (int ii=0; ii<XBIN; ii++) {
          nWVC_OF[ii] = 0;
          nWVC_OA[ii] = 0;
          nWVC_IF[ii] = 0;
          nWVC_IA[ii] = 0;
          nLookOF[ii] = 0.;
          nLookOA[ii] = 0.;
          nLookIF[ii] = 0.;
          nLookIA[ii] = 0.;
          SNR_OF[ii] = 0.;
          SNR_OA[ii] = 0.;
          SNR_IF[ii] = 0.;
          SNR_IA[ii] = 0.;
          kpc_OF[ii] = 0.;
          kpc_OA[ii] = 0.;
          kpc_IF[ii] = 0.;
          kpc_IA[ii] = 0.;
          azres_OF[ii] = 0.;
          azres_OA[ii] = 0.;
          azres_IF[ii] = 0.;
          azres_IA[ii] = 0.;
        }

        int frame_number=1;

	//---------------------//
	// copy desired frames //
	//---------------------//

        int findCell_OF, findCell_OA, findCell_IF, findCell_IA;

	while (l2a.ReadDataRec() && frame_number <= end_frame)
	{
          findCell_OF = 0;
          findCell_OA = 0;
          findCell_IF = 0;
          findCell_IA = 0;

          for (Meas* meas = l2a.frame.measList.GetHead();
               meas; meas = l2a.frame.measList.GetNext())
          {
            // calculate SNR - use XK and Value
            float Es = meas->XK*meas->value;
            float En = bK*SYSTEM_TEMPERATURE;
            //float SNR = Es/En/NUM_RANGE_LOOKS_AVERAGED;
            float SNR = Es/En;

            if (meas->beamIdx <= 1 && 
                (meas->scanAngle <= pi/2. || meas->scanAngle >= 3.*pi/2.))
            {

               if (!findCell_OF) {
                 nWVC_OF[l2a.frame.cti]++;
                 findCell_OF = 1;
               }
               nLookOF[l2a.frame.cti]++;
               SNR_OF[l2a.frame.cti] += SNR;
               azres_OF[l2a.frame.cti] += meas->azimuth_width;

            } else if (meas->beamIdx <= 1 &&
                       (meas->scanAngle > pi/2. && meas->scanAngle < 3.*pi/2.))
            {

               if (!findCell_OA) {
                 nWVC_OA[l2a.frame.cti]++;
                 findCell_OA = 1;
               }
               nLookOA[l2a.frame.cti]++;
               SNR_OA[l2a.frame.cti] += SNR;
               azres_OA[l2a.frame.cti] += meas->azimuth_width;

            } else if (meas->beamIdx > 1 &&
                (meas->scanAngle <= pi/2. || meas->scanAngle >= 3.*pi/2.))
            {

               if (!findCell_IF) {
                 nWVC_IF[l2a.frame.cti]++;
                 findCell_IF = 1;
               }
               nLookIF[l2a.frame.cti]++;
               SNR_IF[l2a.frame.cti] += SNR;
               azres_IF[l2a.frame.cti] += meas->azimuth_width;

            } else if (meas->beamIdx > 1 &&
                       (meas->scanAngle > pi/2. && meas->scanAngle < 3.*pi/2.))
            {

               if (!findCell_IA) {
                 nWVC_IA[l2a.frame.cti]++;
                 findCell_IA = 1;
               }
               nLookIA[l2a.frame.cti]++;
               SNR_IA[l2a.frame.cti] += SNR;
               azres_IA[l2a.frame.cti] += meas->azimuth_width;

            }
          }
          //printf("%d\n", l2a.frame.cti);
          //printf("%d\n", l2a.header.crossTrackBins);
	  //if(frame_number >= start_frame){
	  //  l2a.WriteReqDataAscii();
	  //}
          if(start_frame>=0) frame_number++;
        }

        for (int ii=0; ii<XBIN; ii++) {

          if (nWVC_OF[ii]>0) {
            azres_OF[ii] /= nLookOF[ii];
            SNR_OF[ii] /= nLookOF[ii]; // average SNR
            nLookOF[ii] = nLookOF[ii]/nWVC_OF[ii]*NUM_RANGE_LOOKS_AVERAGED; // looks per wind cell
            kpc_OF[ii] = sqrt((1. + 2./SNR_OF[ii] + 1./(SNR_OF[ii]*SNR_OF[ii]))/nLookOF[ii]);
          }

          if (nWVC_OA[ii]>0) {
            azres_OA[ii] /= nLookOA[ii];
            SNR_OA[ii] /= nLookOA[ii]; // average SNR
            nLookOA[ii] = nLookOA[ii]/nWVC_OA[ii]*NUM_RANGE_LOOKS_AVERAGED; // looks per wind cell
            kpc_OA[ii] = sqrt((1. + 2./SNR_OA[ii] + 1./(SNR_OA[ii]*SNR_OA[ii]))/nLookOA[ii]);
          }

          if (nWVC_IF[ii]>0) {
            azres_IF[ii] /= nLookIF[ii];
            SNR_IF[ii] /= nLookIF[ii]; // average SNR
            nLookIF[ii] = nLookIF[ii]/nWVC_IF[ii]*NUM_RANGE_LOOKS_AVERAGED; // looks per wind cell
            kpc_IF[ii] = sqrt((1. + 2./SNR_IF[ii] + 1./(SNR_IF[ii]*SNR_IF[ii]))/nLookIF[ii]);
          }

          if (nWVC_IA[ii]>0) {
            azres_IA[ii] /= nLookIA[ii];
            SNR_IA[ii] /= nLookIA[ii]; // average SNR
            nLookIA[ii] = nLookIA[ii]/nWVC_IA[ii]*NUM_RANGE_LOOKS_AVERAGED; // looks per wind cell
            kpc_IA[ii] = sqrt((1. + 2./SNR_IA[ii] + 1./(SNR_IA[ii]*SNR_IA[ii]))/nLookIA[ii]);
          }

          fprintf(outfileP, "%d %d %d %d %d %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n", ii,
                  nWVC_OF[ii], nWVC_OA[ii], nWVC_IF[ii], nWVC_IA[ii],
                  nLookOF[ii], nLookOA[ii], nLookIF[ii], nLookIA[ii],
                  SNR_OF[ii], SNR_OA[ii], SNR_IF[ii], SNR_IA[ii],
                  kpc_OF[ii], kpc_OA[ii], kpc_IF[ii], kpc_IA[ii],
                  azres_OF[ii], azres_OA[ii], azres_IF[ii], azres_IA[ii]);
        }

        //----------------------//
        // close files and exit //
        //----------------------//

	l2a.Close();
        fclose(outfileP);
        return(0);
}
