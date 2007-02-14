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

#define XBIN 1000
#define NUM_RANGE_LOOKS_AVERAGED 8
#define MAX_NBEAMS 4
#define NDIRS 2
#define CROSS_DIST 2000
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
			      "<nbeams>", "<res>", "[ati]",0};

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
	int res = atof(argv[clidx++]);

	int ati;
        int searchFlag; // 0: not search, use input ati; 1: search first and last ati

        if (argc > 5) {
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

        // search atis such that it gets all beams and all directions data
	while (l2a.ReadDataRec() && searchFlag && !(firstFlag && lastFlag))
	{
          if (l2a.frame.cti == (CENTER_OFFSET+CROSS_DIST/2)/res) { // index of middle of swath

            // initialize flags
            for (int bb=0; bb<nbeams; bb++) {
              for (int ff=0; ff<NDIRS; ff++) {
                flag[bb][ff] = 0;
              }
            }

            for (Meas* meas = l2a.frame.measList.GetHead();
                 meas; meas = l2a.frame.measList.GetNext())
            {
              if (meas->beamIdx ==0 &&
                  (meas->scanAngle <= pi/2. || meas->scanAngle >= 3.*pi/2.))
              {
                flag[0][0] = 1;
              } else if (meas->beamIdx == 0 &&
                         (meas->scanAngle > pi/2. && meas->scanAngle < 3.*pi/2.))
              {
                flag[0][1] = 1;
              } else if (meas->beamIdx == 1 &&
                         (meas->scanAngle <= pi/2. || meas->scanAngle >= 3.*pi/2.))
              {
                flag[1][0] = 1;
              } else if (meas->beamIdx == 1 &&
                         (meas->scanAngle > pi/2. && meas->scanAngle < 3.*pi/2.))
              {
                flag[1][1] = 1;
              }
            }

            int tmp = 1;

            for (int bb=0; bb<nbeams; bb++) {
              for (int ff=0; ff<NDIRS; ff++) {
                tmp *= flag[bb][ff];
              }
            }

            if (!firstFlag && tmp==1) {
              firstFlag = 1;
              firstAti = l2a.frame.ati;
              cout << "First ati with all data at swath center: " << firstAti << endl;
            }

            if (firstFlag && !lastFlag && tmp==0) {
              lastFlag = 1;
              lastAti = l2a.frame.ati;
              cout << "Last ati with all data at swath center: " << lastAti << endl;
            }

          } // cti == swath center

        } // while loop of l2a

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
        float ambRatio[MAX_NBEAMS][NDIRS];
        int nMeas[MAX_NBEAMS][NDIRS][XBIN];
        float nes0[MAX_NBEAMS][NDIRS][XBIN], azAng[MAX_NBEAMS][NDIRS][XBIN];
        float incAng[MAX_NBEAMS][NDIRS][XBIN];

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
            nLook[bb][ff] = 8;
          }
        }

        // ambiguity info
        for (int bb=0; bb<nbeams; bb++) {
          for (int ff=0; ff<NDIRS; ff++) {
            ambRatio[bb][ff] = 15;
          }
        }

        // initialization
        for (int bb=0; bb<nbeams; bb++) {
          for (int ff=0; ff<NDIRS; ff++) {
            for (int ii=0; ii<XBIN; ii++) {
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

	while (l2a.ReadDataRec() && l2a.frame.ati <= ati)
	{
          if (l2a.frame.ati == ati) {
            for (Meas* meas = l2a.frame.measList.GetHead();
                 meas; meas = l2a.frame.measList.GetNext())
            {

              if (meas->beamIdx ==0 &&
                  (meas->scanAngle <= pi/2. || meas->scanAngle >= 3.*pi/2.))
              {

                 nMeas[0][0][l2a.frame.cti]++;
                 nes0[0][0][l2a.frame.cti] += meas->EnSlice/meas->XK;
                 azAng[0][0][l2a.frame.cti] += meas->eastAzimuth;
                 incAng[0][0][l2a.frame.cti] += meas->incidenceAngle;

              } else if (meas->beamIdx ==0 &&
                       (meas->scanAngle > pi/2. && meas->scanAngle < 3.*pi/2.))
              {

                 nMeas[0][1][l2a.frame.cti]++;
                 nes0[0][1][l2a.frame.cti] += meas->EnSlice/meas->XK;
                 azAng[0][1][l2a.frame.cti] += meas->eastAzimuth;
                 incAng[0][1][l2a.frame.cti] += meas->incidenceAngle;

              } else if (meas->beamIdx == 1 &&
                (meas->scanAngle <= pi/2. || meas->scanAngle >= 3.*pi/2.))
              {

                 nMeas[1][0][l2a.frame.cti]++;
                 nes0[1][0][l2a.frame.cti] += meas->EnSlice/meas->XK;
                 azAng[1][0][l2a.frame.cti] += meas->eastAzimuth;
                 incAng[1][0][l2a.frame.cti] += meas->incidenceAngle;

              } else if (meas->beamIdx == 1 &&
                       (meas->scanAngle > pi/2. && meas->scanAngle < 3.*pi/2.))
              {

                 nMeas[1][1][l2a.frame.cti]++;
                 nes0[1][1][l2a.frame.cti] += meas->EnSlice/meas->XK;
                 azAng[1][1][l2a.frame.cti] += meas->eastAzimuth;
                 incAng[1][1][l2a.frame.cti] += meas->incidenceAngle;

              }

            } // meas list loop

            // perform averaging
            if (nMeas[0][0][l2a.frame.cti] > 0) {
              nes0[0][0][l2a.frame.cti] /= nMeas[0][0][l2a.frame.cti];
              nes0[0][0][l2a.frame.cti] = 10.*log10(nes0[0][0][l2a.frame.cti]);
              azAng[0][0][l2a.frame.cti] /= nMeas[0][0][l2a.frame.cti]/rtd;
              incAng[0][0][l2a.frame.cti] /= nMeas[0][0][l2a.frame.cti]/rtd;
            }
            if (nMeas[0][1][l2a.frame.cti] > 0) {
              nes0[0][1][l2a.frame.cti] /= nMeas[0][1][l2a.frame.cti];
              nes0[0][1][l2a.frame.cti] = 10.*log10(nes0[0][1][l2a.frame.cti]);
              azAng[0][1][l2a.frame.cti] /= nMeas[0][1][l2a.frame.cti]/rtd;
              incAng[0][1][l2a.frame.cti] /= nMeas[0][1][l2a.frame.cti]/rtd;
            }
            if (nMeas[1][0][l2a.frame.cti] > 0) {
              nes0[1][0][l2a.frame.cti] /= nMeas[1][0][l2a.frame.cti];
              nes0[1][0][l2a.frame.cti] = 10.*log10(nes0[1][0][l2a.frame.cti]);
              azAng[1][0][l2a.frame.cti] /= nMeas[1][0][l2a.frame.cti]/rtd;
              incAng[1][0][l2a.frame.cti] /= nMeas[1][0][l2a.frame.cti]/rtd;
            }
            if (nMeas[1][1][l2a.frame.cti] > 0) {
              nes0[1][1][l2a.frame.cti] /= nMeas[1][1][l2a.frame.cti];
              nes0[1][1][l2a.frame.cti] = 10.*log10(nes0[1][1][l2a.frame.cti]);
              azAng[1][1][l2a.frame.cti] /= nMeas[1][1][l2a.frame.cti]/rtd;
              incAng[1][1][l2a.frame.cti] /= nMeas[1][1][l2a.frame.cti]/rtd;
            }

          } // ati check

        } // while, reading loop

        // write out info
        fprintf(outfileP, "%d %d\n", nbeams, res);

        for (int ii=0; ii<CROSS_DIST/res; ii++) {

          fprintf(outfileP, "%d %d %f %d %f %f %c %f %d %f %d %f %f %c %f %d %f %d %f %f %c %f %d %f %d %f %f %c %f\n", int((ii+0.5)*res-CROSS_DIST/2),
                  nMeas[0][0][ii], nes0[0][0][ii], nLook[0][0], azAng[0][0][ii],
                  incAng[0][0][ii], pol[0][0], ambRatio[0][0], 
                  nMeas[0][1][ii], nes0[0][1][ii], nLook[0][1], azAng[0][1][ii],
                  incAng[0][1][ii], pol[0][1], ambRatio[0][1], 
                  nMeas[1][0][ii], nes0[1][0][ii], nLook[1][0], azAng[1][0][ii],
                  incAng[1][0][ii], pol[1][0], ambRatio[1][0], 
                  nMeas[1][1][ii], nes0[1][1][ii], nLook[1][1], azAng[1][1][ii],
                  incAng[1][1][ii], pol[1][1], ambRatio[1][1]);
        }

        //----------------------//
        // close files and exit //
        //----------------------//

	l2a.Close();
        fclose(outfileP);
        return(0);
}
