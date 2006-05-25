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

        int nBeamIF[XBIN], nBeamIA[XBIN], nBeamOF[XBIN], nBeamOA[XBIN]; 
        float SNR_IF[XBIN], SNR_IA[XBIN], SNR_OF[XBIN], SNR_OA[XBIN];
        float kpc_IF[XBIN], kpc_IA[XBIN], kpc_OF[XBIN], kpc_OA[XBIN];
        float azres_IF[XBIN], azres_IA[XBIN], azres_OF[XBIN], azres_OA[XBIN];

        for (int ii=0; ii<XBIN; ii++) {
          nBeamOF[ii] = 0;
          nBeamOA[ii] = 0;
          nBeamIF[ii] = 0;
          nBeamIA[ii] = 0;
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

	while (l2a.ReadDataRec() && frame_number <= end_frame)
	{
          for (Meas* meas = l2a.frame.measList.GetHead();
               meas; meas = l2a.frame.measList.GetNext())
          {
            // calculate SNR - use XK and Value
            float Es = meas->XK*meas->value;
            float En = bK*SYSTEM_TEMPERATURE;
            //float SNR = Es/En/NUM_RANGE_LOOKS_AVERAGED;
            float SNR = Es/En;

            // calculate kpc - use A, B, C and SNR
            float kpc = sqrt((1. + 2./SNR + 1./(SNR*SNR))/NUM_RANGE_LOOKS_AVERAGED);

            if (meas->beamIdx <= 1 && 
                (meas->scanAngle <= pi/2. || meas->scanAngle >= 3.*pi/2.))
            {

               nBeamOF[l2a.frame.cti]++;
               SNR_OF[l2a.frame.cti] += SNR;
               kpc_OF[l2a.frame.cti] += kpc;
               azres_OF[l2a.frame.cti] += meas->azimuth_width;

            } else if (meas->beamIdx <= 1 &&
                       (meas->scanAngle > pi/2. && meas->scanAngle < 3.*pi/2.))
            {

               nBeamOA[l2a.frame.cti]++;
               SNR_OA[l2a.frame.cti] += SNR;
               kpc_OA[l2a.frame.cti] += kpc;
               azres_OA[l2a.frame.cti] += meas->azimuth_width;

            } else if (meas->beamIdx > 1 &&
                (meas->scanAngle <= pi/2. || meas->scanAngle >= 3.*pi/2.))
            {

               nBeamIF[l2a.frame.cti]++;
               SNR_IF[l2a.frame.cti] += SNR;
               kpc_IF[l2a.frame.cti] += kpc;
               azres_IF[l2a.frame.cti] += meas->azimuth_width;

            } else if (meas->beamIdx > 1 &&
                       (meas->scanAngle > pi/2. && meas->scanAngle < 3.*pi/2.))
            {

               nBeamIA[l2a.frame.cti]++;
               SNR_IA[l2a.frame.cti] += SNR;
               kpc_IA[l2a.frame.cti] += kpc;
               azres_IA[l2a.frame.cti] += meas->azimuth_width;

            }
          }
          //printf("%d\n", l2a.frame.cti);
          //printf("%d\n", l2a.header.crossTrackBins);
          //exit(0);
	  //if(frame_number >= start_frame){
	  //  l2a.WriteReqDataAscii();
	  //}
          if(start_frame>=0) frame_number++;
        }

        for (int ii=0; ii<1000; ii++) {
          if (nBeamOF[ii] != 0) {
            SNR_OF[ii] /= nBeamOF[ii];
            kpc_OF[ii] /= nBeamOF[ii];
            azres_OF[ii] /= nBeamOF[ii];
          }
          if (nBeamOA[ii] != 0) {
            SNR_OA[ii] /= nBeamOA[ii];
            kpc_OA[ii] /= nBeamOA[ii];
            azres_OA[ii] /= nBeamOA[ii];
          }
          if (nBeamIF[ii] != 0) {
            SNR_IF[ii] /= nBeamIF[ii];
            kpc_IF[ii] /= nBeamIF[ii];
            azres_IF[ii] /= nBeamIF[ii];
          }
          if (nBeamIA[ii] != 0) {
            SNR_IA[ii] /= nBeamIA[ii];
            kpc_IA[ii] /= nBeamIA[ii];
            azres_IA[ii] /= nBeamIA[ii];
          }
          fprintf(outfileP, "%d %d %d %d %d %f %f %f %f %f %f %f %f %f %f %f %f\n", ii,
                  nBeamOF[ii]*NUM_RANGE_LOOKS_AVERAGED,
                  nBeamOA[ii]*NUM_RANGE_LOOKS_AVERAGED,
                  nBeamIF[ii]*NUM_RANGE_LOOKS_AVERAGED,
                  nBeamIA[ii]*NUM_RANGE_LOOKS_AVERAGED,
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
