//==========================================================//
// Copyright (C) 1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		check_l1b
//
// SYNOPSIS
//		check_l1b <cfg file> <simcheckfile> <onebcheckfile>
//
// DESCRIPTION
//    Read in a checkframe file and the corresponding l1b file, and
//    computes various comparisons.
//
// OPTIONS
//		None.
//
// EXAMPLES
//		An example of a command line is:
//			% check_l1b qscat.cfg simcheck.dat onebcheck.dat
//
// ENVIRONMENT
//		Not environment dependent.
//
// EXIT STATUS
//		The following exit values are returned:
//		1	Program executed successfully
//		>0	Program had an error
//
// NOTES
//		None.
//
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
#include "Misc.h"
#include "CheckFrame.h"
#include "Meas.h"
#include "List.h"
#include "List.C"
#include "Ephemeris.h"
#include "ConfigList.h"
#include "ConfigSim.h"
#include "L1B.h"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Tracking.h"
#include "Tracking.C"
#include "AngleInterval.h"

//-----------//
// TEMPLATES //
//-----------//

template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<StringPair>;
template class List<Meas>;
template class List<EarthPosition>;
template class List<MeasSpot>;
template class List<WindVectorPlus>;
template class List<long>;
template class List<OffsetList>;
template class List<AngleInterval>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;

//-----------//
// CONSTANTS //
//-----------//

#define MAX_FRAMES 2000

//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

float frac(float* elem);
float derr(float* elem);

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "<cfg file>", "<simcheckfile>",
  "<onebcheckfile>", 0};

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

    char* config_file = NULL;
	char* simcheckfile = NULL;
	char* onebcheckfile = NULL;

	int clidx = 1;
	if (argc == 4)
    {
      config_file = argv[clidx++];
	  simcheckfile = argv[clidx++];
	  onebcheckfile = argv[clidx++];
    }
    else if (argc == 3)
    {
      config_file = argv[clidx++];
	  simcheckfile = argv[clidx++];
    }
    else
    {
	  usage(command, usage_array, 1);
    }

	//--------------------------------//
	// read in simulation config file //
	//--------------------------------//

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

	L1B l1b;
	if (! ConfigL1B(&l1b, &config_list))
	{
		fprintf(stderr, "%s: error configuring Level 1B Product\n", command);
		exit(1);
	}

	//------------------//
	// Get slice counts //
	//------------------//

    int s_count;
    if (! config_list.GetInt(SCIENCE_SLICES_PER_SPOT_KEYWORD, &s_count))
        return(0);

    int g_count;
    if (! config_list.GetInt(GUARD_SLICES_PER_SIDE_KEYWORD, &g_count))
        return(0);

    int slices_per_spot = s_count + 2 * g_count;

	//--------------------//
	// Setup check frames //
	//--------------------//

	CheckFrame cf;
	if (! cf.Allocate(slices_per_spot))
	{
		fprintf(stderr, "%s: error allocating check frame\n", command);
		exit(1);
	}

	CheckFrame cf1b;
	if (! cf1b.Allocate(slices_per_spot))
	{
		fprintf(stderr, "%s: error allocating check frame\n", command);
		exit(1);
	}

	//--------------------------------//
	// create and configure ephemeris //
	//--------------------------------//

	Ephemeris ephemeris;
	if (! ConfigEphemeris(&ephemeris, &config_list))
	{
		fprintf(stderr, "%s: error configuring ephemeris\n", command);
		exit(1);
	}

	//------------//
	// open files //
	//------------//

	FILE* oneb_fp = NULL;
	FILE* check_fp = fopen(simcheckfile,"r");
	if (check_fp == NULL)
	{
		fprintf(stderr, "%s: error opening sim check file %s\n", command,
			simcheckfile);
		exit(1);
	}

    if (onebcheckfile == NULL)
    {
	  l1b.OpenForReading();
    }
    else
    {
	  oneb_fp = fopen(onebcheckfile,"r");
 	  if (oneb_fp == NULL)
	  {
	    fprintf(stderr, "%s: error opening oneb check file %s\n", command,
	            onebcheckfile);
		exit(1);
	  }
    }

  //------------------//
  // Get record count //
  //------------------//

  if (fseek(check_fp,0,SEEK_END) != 0)
	{
	  fprintf(stderr, "%s: error seeking to the end in %s\n", command,
	          simcheckfile);
	  exit(1);
	}
  long file_size = ftell(check_fp);
  long total_spots = file_size/cf.Size();
  long total_slices = slices_per_spot * total_spots;
  if (fseek(check_fp,0,SEEK_SET) != 0)
	{
	  fprintf(stderr, "%s: error seeking to the beginning in %s\n", command,
	          simcheckfile);
	  exit(1);
	}

  //----------------------//
  // Allocate data arrays //
  //----------------------//

  int** slice_idx = (int**)make_array(sizeof(int),2,total_slices,2);
  float** sigma0 = (float**)make_array(sizeof(float),2,total_slices,2);
  float** X = (float**)make_array(sizeof(float),2,total_slices,2);
  float** Es = (float**)make_array(sizeof(float),2,total_slices,2);
  float** En = (float**)make_array(sizeof(float),2,total_slices,2);
  float** azim = (float**)make_array(sizeof(float),2,total_slices,2);
  float** incang = (float**)make_array(sizeof(float),2,total_slices,2);
  float** range = (float**)make_array(sizeof(float),2,total_slices,2);
  float** ant_aziGi = (float**)make_array(sizeof(float),2,total_spots,2);

  if (sigma0 == NULL || X == NULL || Es == NULL || En == NULL ||
      azim == NULL || incang == NULL || range == NULL || ant_aziGi == NULL)
	{
	  fprintf(stderr, "%s: error allocating data arrays\n", command);
	  exit(1);
	}


  //------------//
  // check loop //
  //------------//

  int spot_count = 0;
  int count = 0;

  if (onebcheckfile == NULL) 
  {
    while(1)
    {
        if (spot_count >= MAX_FRAMES) break;

		//-----------------------------//
		// read a level 1B data record //
		//-----------------------------//

		if (! l1b.ReadDataRec())
		{
			switch (l1b.GetStatus())
			{
			case L1B::OK:	// end of file
				break;
			case L1B::ERROR_READING_FRAME:
				fprintf(stderr, "%s: error reading Level 1B data\n", command);
				exit(1);
				break;
			case L1B::ERROR_UNKNOWN:
				fprintf(stderr, "%s: unknown error reading Level 1B data\n",
					command);
				exit(1);
				break;
			default:
				fprintf(stderr, "%s: unknown status (???)\n", command);
				exit(1);
			}
			break;	// done, exit loop
		}

        MeasSpotList* meas_spot_list = &(l1b.frame.spotList);

        //----------------------//
        // for each MeasSpot... //
        //----------------------//

        for (MeasSpot* meas_spot = meas_spot_list->GetHead(); meas_spot;
            meas_spot = meas_spot_list->GetNext())
        {
            double meas_time = meas_spot->scOrbitState.time;
            Meas* first_meas = meas_spot->GetHead();
            double meas_azi = first_meas->scanAngle;

		    //---------------------------------------------//
		    // Read in the corresponding checkframe record //
		    //---------------------------------------------//

            spot_count++;
            if (spot_count >= MAX_FRAMES) break;
            int found = 0;
	        while (cf.ReadDataRec(check_fp))
	        {
                if (fabs(cf.antennaAziGi - meas_azi) < 0.011/4.0)
                {
                    found = 1;
                    break;
                }
//                found = 1;
//                break;
	        }
            if (found == 0)
            {
                fprintf(stderr,"Can't find a checkframe at time = %g\n",
                        meas_time);
                rewind(check_fp);
                continue;
//                exit(-1);
            }

            //------------------//
            // for each Meas... //
            //------------------//
 
            for (Meas* meas = meas_spot->GetHead(); meas;
                meas = meas_spot->GetNext())
            {
              //-------------------------------------------//
              // ...compare Meas data with CheckFrame data //
              //-------------------------------------------//

              int j;
              for (j=0; j < slices_per_spot; j++)
              {  // search for matching checkframe slice record
                if (cf.idx[j] == meas->startSliceIdx && cf.idx[j] != 0) break;
              }
              if (j >= slices_per_spot) continue;  // no match found

              double alt,lon,lat,alt1b,lon1b,lat1b;
              cf.centroid[j].GetAltLonGDLat(&alt,&lon,&lat);
              meas->centroid.GetAltLonGDLat(&alt1b,&lon1b,&lat1b);
              double measR =
                (meas_spot->scOrbitState.rsat - meas->centroid).Magnitude();

              printf("%d %g %g %d %d %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g\n",
                cf.idx[j],
                cf.time, meas_spot->time,
                cf.beamNumber, meas->beamIdx,
                cf.antennaAziTx, meas->scanAngle,
                cf.antennaAziGi, 0.0,
                cf.orbitFrac, 0.0,
                cf.spinRate, 0.0,
                cf.txDoppler, 0.0,
                cf.XdopplerFreq, 0.0,
                cf.XroundTripTime, 0.0,
                cf.sigma0[j], meas->value,
                cf.XK[j], meas->XK,
                cf.EsCal, 0.0,
                cf.deltaFreq, 0.0,
                cf.Es[j], meas->value*meas->XK,
                cf.En[j], meas->EnSlice,
                lon,lon1b,lat,lat1b,
                cf.azimuth[j], meas->eastAzimuth,
                cf.incidence[j], meas->incidenceAngle,
                cf.R[j], measR,
                cf.GatGar[j], 0.0,
                cf.rsat.Get(0), meas_spot->scOrbitState.rsat.Get(0),
                cf.rsat.Get(1), meas_spot->scOrbitState.rsat.Get(1),
                cf.rsat.Get(2), meas_spot->scOrbitState.rsat.Get(2),
                cf.vsat.Get(0), meas_spot->scOrbitState.vsat.Get(0),
                cf.vsat.Get(1), meas_spot->scOrbitState.vsat.Get(1),
                cf.vsat.Get(2), meas_spot->scOrbitState.vsat.Get(2));
            }
        } 
    }
  }
  else
  {
    while(cf1b.ReadDataRec(oneb_fp))
    {  // for each 1B check frame...
      if (spot_count >= MAX_FRAMES) break;
      int found = 0;
	  while (cf.ReadDataRec(check_fp))
	  {  // look for matching sim check frame
          if (fabs(cf.time - cf1b.time) < 0.0108/2.0)
          {
            found = 1;
            break;
          }
	  }
      if (found == 0)
      {
        fprintf(stderr,"Can't match a 1B checkframe at time = %g\n",
                cf1b.time);
        rewind(check_fp);
        continue;
      }

      ant_aziGi[spot_count][0] = cf.antennaAziGi;
      ant_aziGi[spot_count][1] = cf1b.antennaAziGi;
      spot_count++;

      for (int i=0; i < slices_per_spot; i++)
      for (int j=0; j < slices_per_spot; j++)
      {
        if (cf1b.idx[i] != cf.idx[j] || cf1b.idx[i] == 0 || cf.idx[j] == 0)
          continue;
        double alt,lon,lat,alt1b,lon1b,lat1b;
        cf.centroid[j].GetAltLonGDLat(&alt,&lon,&lat);
        cf1b.centroid[i].GetAltLonGDLat(&alt1b,&lon1b,&lat1b);

        slice_idx[count][0] = cf.idx[j];
        sigma0[count][0] = cf.sigma0[j];
        X[count][0] = cf.XK[j];
        Es[count][0] = cf.Es[j];
        En[count][0] = cf.En[j];
        azim[count][0] = cf.azimuth[j];
        incang[count][0] = cf.incidence[j];
        range[count][0] = cf.R[j];

        slice_idx[count][1] = cf1b.idx[i];
        sigma0[count][1] = cf1b.sigma0[i];
        X[count][1] = cf1b.XK[i];
        Es[count][1] = cf1b.Es[i];
        En[count][1] = cf1b.En[i];
        azim[count][1] = cf1b.azimuth[i];
        incang[count][1] = cf1b.incidence[i];
        range[count][1] = cf1b.R[i];
        count++;

/*
        printf("%d %g %g %d %d %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g\n",
               cf.idx[j],
               cf.time, cf1b.time,
               cf.beamNumber, cf1b.beamNumber,
               cf.antennaAziTx, cf1b.antennaAziTx,
               cf.antennaAziGi, cf1b.antennaAziGi,
               cf.orbitFrac, cf1b.orbitFrac,
               cf.spinRate, cf1b.spinRate,
               cf.txDoppler, cf1b.txDoppler,
               cf.XdopplerFreq, cf1b.XdopplerFreq,
               cf.XroundTripTime, cf1b.XroundTripTime,
               cf.sigma0[j], cf1b.sigma0[i],
               cf.XK[j], cf1b.XK[i],
               cf.EsCal, cf1b.EsCal,
               cf.deltaFreq, cf1b.deltaFreq,
               cf.Es[j], cf1b.Es[i],
               cf.En[j], cf1b.En[i],
               lon,lon1b,lat,lat1b,
               cf.azimuth[j], cf1b.azimuth[i],
               cf.incidence[j], cf1b.incidence[i],
               cf.R[j], cf1b.R[i],
               cf.GatGar[j], cf1b.GatGar[i],
               cf.rsat.Get(0), cf1b.rsat.Get(0),
               cf.rsat.Get(1), cf1b.rsat.Get(1),
               cf.rsat.Get(2), cf1b.rsat.Get(2),
               cf.vsat.Get(0), cf1b.vsat.Get(0),
               cf.vsat.Get(1), cf1b.vsat.Get(1),
               cf.vsat.Get(2), cf1b.vsat.Get(2));
*/
      }
    }
  }

  if (onebcheckfile != NULL) 
  {
//    printf("total_slices = %ld, total_spots = %ld, file_size = %ld\n",
//           total_slices,
//           total_spots,file_size);
//    printf("count = %d, spot_count = %d, cf.Size = %d\n",count,spot_count,
//      cf.Size());
    FILE* c1_plot = fopen("c1_plot.xmg","w");
    if (c1_plot == NULL)
    {
	  fprintf(stderr, "%s: error opening c1_plot.xmg file\n", command);
	  exit(1);
	}
    fprintf(c1_plot,"@with g0\n@g0 on\n");
    for (int i=0; i < count; i++)
    {
      fprintf(c1_plot,"%d %g\n",i,frac(sigma0[i]));
    }
    fprintf(c1_plot,"&\n");
    fprintf(c1_plot,"@with g1\n@g1 on\n");
    for (int i=0; i < count; i++)
    {
      fprintf(c1_plot,"%d %g\n",i,frac(X[i]));
    }
    fprintf(c1_plot,"&\n");
    fprintf(c1_plot,"@with g0\n@g0 on\n");
    fprintf(c1_plot,"@view xmin 0.2\n@view xmax 0.8\n");
    fprintf(c1_plot,"@view ymin 0.1\n@view ymax 0.375\n");
    fprintf(c1_plot,"@autoscale xaxes\n@autoscale yaxes\n");
    fprintf(c1_plot,"@with g1\n@g1 on\n");
    fprintf(c1_plot,"@view xmin 0.2\n@view xmax 0.8\n");
    fprintf(c1_plot,"@view ymin 0.4\n@view ymax 0.775\n");
    fprintf(c1_plot,"@autoscale xaxes\n@autoscale yaxes\n");
	fclose(c1_plot);
  }

  //-----------------//
  // close the files //
  //-----------------//

  fclose(check_fp);
  if (onebcheckfile == NULL)
  {
    l1b.Close();
  }
  else
  {
    fclose(oneb_fp);
  }

  return (0);
}

float frac(float* elem)
{
  if (elem[0] == 0.0)
    return(0.0);
  else
    return((elem[0] - elem[1]) / elem[0]);
}

float derr(float* elem)
{
  return(rtd*(elem[0]-elem[1]));
}

