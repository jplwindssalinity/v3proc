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

#define MAX_SPOTS  100000
#define SEARCH_WINDOW 1000

//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

int
match_cf_frame(
  CheckFrame* cf_target,
  CheckFrame* cf_source,
  FILE* source_fp);
int
match_frame(
  CheckFrame* cf_target,
  CheckFrame* cf_source,
  double* rr);
float identity(float* elem);
float identitydB(float* elem);
float dBerr(float* elem);
float frac(float* elem);
float derr(float* elem);
float dif_err(float* elem);
int
plot4(char* filename,
      int count,
      int *use,
      float (*func)(float*),
      float** x,
      float** y1,
      float** y2,
      float** y3,
      float** y4,
      char* xlabel,
      char* y1label,
      char* y2label,
      char* y3label,
      char* y4label);

//------------------//
// OPTION VARIABLES //
//------------------//

#define OPTSTRING               "sfpo:r:"

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -s ]", "[ -f ]", "[ -p (print to stdout) ]",
  " [-o output_base] ", "[ -r low:hi (pulse count) ]",
  "<cfg file>", "<simcheckfile>",
  "<onebcheckfile>", 0};
extern int optind;

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
    char* output_base = NULL;

    int range_low = -1;
    int range_hi = -1;
    int f_flag = 0;
    int s_flag = 0;
    int p_flag = 0;
    while (1)
    {
      int c = getopt(argc, argv, OPTSTRING);
      if (c == 'f') f_flag = 1;
      else if (c == 's') s_flag = 1;
      else if (c == 'p') p_flag = 1;
      else if (c == 'o')
      {
        output_base = optarg;
      }
      else if (c == 'r')
      {
        if (sscanf(optarg, "%d:%d", &range_low, &range_hi) != 2)
        {
          fprintf(stderr, "%s: error determining spot range %s\n",
              command, optarg);
          exit(1);
        }
      }
      else if (c == -1) break;
    }

	int clidx = optind;
	if (argc-optind == 3)
    {
      config_file = argv[clidx++];
	  simcheckfile = argv[clidx++];
	  onebcheckfile = argv[clidx++];
    }
    else if (argc-optind == 2)
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

	//--------------------//
	// Setup check frames //
	//--------------------//

	CheckFrame cf;
	CheckFrame cf1b;
	if (! cf.Allocate(check_fp))
	{
	  fprintf(stderr, "%s: error allocating check frame\n", command);
	  exit(1);
	}
    if (onebcheckfile == NULL) 
    {
      if (! cf1b.Allocate(cf.slicesPerSpot))
      {
        fprintf(stderr,"Error allocating onebcheckframe\n");
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
  long total_slices = cf.slicesPerSpot * total_spots;
  if (fseek(check_fp,0,SEEK_SET) != 0)
  {
    fprintf(stderr, "%s: error seeking to the beginning in %s\n", command,
      simcheckfile);
	  exit(1);
  }

  if (range_low == -1 && range_hi == -1)
  {
    range_low = 0;
    range_hi = total_spots;
  }

  // Subsample large numbers of spots to keep the number of points on
  // a graph reasonable.
  int subsample = (int)((range_hi - range_low)/MAX_SPOTS) + 1;
  int Nspots = (range_hi - range_low)/subsample;
  int Nslices = Nspots*cf.slicesPerSpot;
  fprintf(stderr,"subsample=%d Nspots=%d Nslices=%d\n",
    subsample,Nspots,Nslices);

  Index snr_idx;
  Index pulse_count_idx;
  snr_idx.SpecifyCenters(-44.0,24.0,35);
  pulse_count_idx.SpecifyEdges(0.0,total_spots,Nspots);
  int Nstat = snr_idx.GetBins();

  float* e_sigma0_sum = NULL;
  float* e_sigma0_sum2 = NULL;
  float* e_sigma0_max = NULL;
  float* e_sigma0_min = NULL;
  float* e_X_sum = NULL;
  float* e_X_sum2 = NULL;
  float* e_X_max = NULL;
  float* e_X_min = NULL;
  float* e_Es_sum = NULL;
  float* e_Es_sum2 = NULL;
  float* e_Es_max = NULL;
  float* e_Es_min = NULL;
  float* e_En_sum = NULL;
  float* e_En_sum2 = NULL;
  float* e_En_max = NULL;
  float* e_En_min = NULL;

  int* c_stat = NULL;

  if (s_flag == 1)
  {
    e_sigma0_sum = snr_idx.MakeFloatArray();
    e_sigma0_sum2 = snr_idx.MakeFloatArray();
    e_sigma0_min = snr_idx.MakeFloatArray();
    e_sigma0_max = snr_idx.MakeFloatArray();
    e_X_sum = snr_idx.MakeFloatArray();
    e_X_sum2 = snr_idx.MakeFloatArray();
    e_X_min = snr_idx.MakeFloatArray();
    e_X_max = snr_idx.MakeFloatArray();
    e_Es_sum = snr_idx.MakeFloatArray();
    e_Es_sum2 = snr_idx.MakeFloatArray();
    e_Es_min = snr_idx.MakeFloatArray();
    e_Es_max = snr_idx.MakeFloatArray();
    e_En_sum = snr_idx.MakeFloatArray();
    e_En_sum2 = snr_idx.MakeFloatArray();
    e_En_min = snr_idx.MakeFloatArray();
    e_En_max = snr_idx.MakeFloatArray();
    c_stat = snr_idx.MakeIntArray();
    if (e_sigma0_sum == NULL ||
        e_sigma0_sum2 == NULL ||
        e_sigma0_max == NULL ||
        e_sigma0_min == NULL ||
        e_X_sum == NULL ||
        e_X_sum2 == NULL ||
        e_X_max == NULL ||
        e_X_min == NULL ||
        e_Es_sum == NULL ||
        e_Es_sum2 == NULL ||
        e_Es_max == NULL ||
        e_Es_min == NULL ||
        e_En_sum == NULL ||
        e_En_sum2 == NULL ||
        e_En_max == NULL ||
        e_En_min == NULL ||
        c_stat == NULL)
    {
	    fprintf(stderr, "%s: error allocating statistics arrays\n", command);
	    exit(1);
	}
  }

  //----------------------//
  // Allocate data arrays //
  //----------------------//

  int** slice_idx = NULL;
  float** pulse_count = NULL;
  float** sigma0 = NULL;
  float** X = NULL;
  float** Es = NULL;
  float** En = NULL;
  float** azim = NULL;
  float** incang = NULL;
  float** range = NULL;
  float** lon = NULL;
  float** lat = NULL;

  float** ant_aziGi = NULL;
  float** tx_doppler = NULL;
  float** x_doppler = NULL;
  float** rx_gate_delay = NULL;
  float** delta_freq = NULL;

  if (output_base != NULL)
  {
    slice_idx = (int**)make_array(sizeof(int),2,Nslices,2);
    pulse_count = (float**)make_array(sizeof(float),2,Nslices,1);
    sigma0 = (float**)make_array(sizeof(float),2,Nslices,2);
    X = (float**)make_array(sizeof(float),2,Nslices,2);
    Es = (float**)make_array(sizeof(float),2,Nslices,2);
    En = (float**)make_array(sizeof(float),2,Nslices,2);
    azim = (float**)make_array(sizeof(float),2,Nslices,2);
    incang = (float**)make_array(sizeof(float),2,Nslices,2);
    range = (float**)make_array(sizeof(float),2,Nslices,2);
    lon = (float**)make_array(sizeof(float),2,Nslices,2);
    lat = (float**)make_array(sizeof(float),2,Nslices,2);

    ant_aziGi = (float**)make_array(sizeof(float),2,Nspots,2);
    tx_doppler = (float**)make_array(sizeof(float),2,Nspots,2);
    x_doppler = (float**)make_array(sizeof(float),2,Nspots,2);
    rx_gate_delay = (float**)make_array(sizeof(float),2,Nspots,2);
    delta_freq = (float**)make_array(sizeof(float),2,Nspots,2);

    if (sigma0 == NULL || X == NULL || Es == NULL || En == NULL ||
        azim == NULL || incang == NULL || range == NULL || ant_aziGi == NULL ||
        lon == NULL || lat == NULL || tx_doppler == NULL || x_doppler == NULL ||
        rx_gate_delay == NULL || delta_freq == NULL || pulse_count == NULL)
	{
	    fprintf(stderr, "%s: error allocating data arrays\n", command);
	    exit(1);
	}
  }

  //------------//
  // check loop //
  //------------//

  int spot_count = 0;  // counts spots actually included
  int cf_frame_count = 0; // counts all spots (cf frames).
  int count = 0;
  int subcount = 0;

  while(1)
  {
    subcount++;  // step through the subsampled spots
    if (cf_frame_count > range_hi) break;
    cf_frame_count++;  // count all spots

    if (onebcheckfile == NULL) 
    {
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

            //-------------------------------------//
            // Load spot data into cf1b CheckFrame //
            //-------------------------------------//

            cf1b.pulseCount = cf_frame_count-1;
            cf1b.time = meas_spot->time;
            cf1b.beamNumber = first_meas->beamIdx;
            cf1b.antennaAziTx = 0.0;
            cf1b.antennaAziGi = first_meas->scanAngle;
            cf1b.orbitFrac = 0.0;
            cf1b.spinRate = 0.0;
            cf1b.txDoppler = 0.0;
            cf1b.XdopplerFreq = 0.0;
            cf1b.XroundTripTime = 0.0;
            cf1b.EsCal = 0.0;
            cf1b.deltaFreq = 0.0;
            cf1b.EsnEcho = 0.0;
            cf1b.EsnNoise = 0.0;
            cf1b.rxGateDelay = 0.0;
            cf1b.alpha = 0.0;
            cf1b.rsat = meas_spot->scOrbitState.rsat;
            cf1b.vsat = meas_spot->scOrbitState.vsat;

            //------------------//
            // for each Meas... //
            //------------------//
 
            int j = 0;
            for (Meas* meas = meas_spot->GetHead(); meas;
                meas = meas_spot->GetNext())
            {
              //-------------------------------------------//
              // ...load Meas slice data into CheckFrame   //
              //-------------------------------------------//

              if (j >= cf.slicesPerSpot)
              {
                fprintf(stderr,"Error, Too many measurements in a spot\n");
                exit(1);
              }

              double alt1b,lon1b,lat1b;
              meas->centroid.GetAltLonGDLat(&alt1b,&lon1b,&lat1b);
              double measR =
                (meas_spot->scOrbitState.rsat - meas->centroid).Magnitude();

              cf1b.idx[j] = meas->startSliceIdx;
              cf1b.measType[j] = meas->measType;
              cf1b.sigma0[j] = meas->value;
              cf1b.XK[j] = meas->XK;
              cf1b.Es[j] = meas->value*meas->XK;
              cf1b.En[j] = meas->EnSlice;
              cf1b.centroid[j] = meas->centroid;
              cf1b.azimuth[j] = meas->eastAzimuth;
              cf1b.incidence[j] = meas->incidenceAngle;
              cf1b.R[j] = measR;
              cf1b.GatGar[j] = 0.0;
              j++;
            }
        } 
    }
    else
    {  // A 2nd checkfile is provided.

      //-----------------------//
      // Read a 1B check frame //
      //-----------------------//

      int ret;
      if (f_flag == 0)
      {
        ret = cf1b.ReadDataRec(oneb_fp);
      }
      else
      {
        ret = cf1b.ReadFortranStructure(oneb_fp);
      }
      if (! ret) break; // end of oneb_fp file

    }

    if (cf_frame_count < range_low) continue;  // wait till in range.

    //-------------------------------------//
    // Accumulate statistics on all pulses //
    //-------------------------------------//

    if (s_flag == 1)
    {
      if (match_cf_frame(&cf1b,&cf,check_fp) == 0) continue;
      for (int i=0; i < cf.slicesPerSpot; i++)
      for (int j=0; j < cf.slicesPerSpot; j++)
      {
        if (cf1b.idx[i] != cf.idx[j] || cf1b.idx[i] == 0 || cf.idx[j] == 0)
          continue;

        float snr = 10.0/log(10.0)*log(cf.Es[j]/cf.En[j]);
        int isnr;
        if (!snr_idx.GetNearestIndex(snr,&isnr))
        {
          fprintf(stderr,"Warning: Snr index out of range (snr (dB) = %g)\n",
                  snr);
          continue;
        }

        c_stat[isnr]++;

        float diff = (cf1b.sigma0[i] - cf.sigma0[j])/cf.sigma0[j];
        e_sigma0_sum[isnr] += diff;
        e_sigma0_sum2[isnr] += diff*diff;
        if (e_sigma0_max[isnr] < diff) e_sigma0_max[isnr] = diff;
        if (e_sigma0_min[isnr] > diff) e_sigma0_min[isnr] = diff;

        diff = (cf1b.XK[i] - cf.XK[j])/cf.XK[j];
        e_X_sum[isnr] += diff;
        e_X_sum2[isnr] += diff*diff;
        if (e_X_max[isnr] < diff) e_X_max[isnr] = diff;
        if (e_X_min[isnr] > diff) e_X_min[isnr] = diff;

        diff = (cf1b.Es[i] - cf.Es[j])/cf.Es[j];
        e_Es_sum[isnr] += diff;
        e_Es_sum2[isnr] += diff*diff;
        if (e_Es_max[isnr] < diff) e_Es_max[isnr] = diff;
        if (e_Es_min[isnr] > diff) e_Es_min[isnr] = diff;

        diff = (cf1b.En[i] - cf.En[j])/cf.En[j];
        e_En_sum[isnr] += diff;
        e_En_sum2[isnr] += diff*diff;
        if (e_En_max[isnr] < diff) e_En_max[isnr] = diff;
        if (e_En_min[isnr] > diff) e_En_min[isnr] = diff;

        break; // done with this slice
      }
    }

    //--------------------------------------------//
    // Accumulate data for subsampled pulses only //
    //--------------------------------------------//

    if (subcount < subsample)
    {  // skip over pulses until the next subsample
      continue;
    }
    else
    {  // process this pulse and start subsample count again
      subcount = 0;
    }

    if (s_flag == 0)
    {  // still need to read a matching frame
      if (match_cf_frame(&cf1b,&cf,check_fp) == 0) continue;
    }

    if (output_base != NULL)
    {
      ant_aziGi[spot_count][0] = cf.antennaAziGi;
      ant_aziGi[spot_count][1] = cf1b.antennaAziGi;
      tx_doppler[spot_count][0] = cf.txDoppler;
      tx_doppler[spot_count][1] = cf1b.txDoppler;
      x_doppler[spot_count][0] = cf.XdopplerFreq;
      x_doppler[spot_count][1] = cf1b.XdopplerFreq;
      rx_gate_delay[spot_count][0] = cf.rxGateDelay;
      rx_gate_delay[spot_count][1] = cf1b.rxGateDelay;
      delta_freq[spot_count][0] = cf.deltaFreq;
      delta_freq[spot_count][1] = cf1b.deltaFreq;
    }
    spot_count++;

    for (int i=0; i < cf.slicesPerSpot; i++)
    for (int j=0; j < cf.slicesPerSpot; j++)
    {
      if (cf1b.idx[i] != cf.idx[j] || cf1b.idx[i] == 0 || cf.idx[j] == 0)
        continue;
      double alt,lon1,lat1,alt1b,lon1b,lat1b;
      cf.centroid[j].GetAltLonGDLat(&alt,&lon1,&lat1);
      cf1b.centroid[i].GetAltLonGDLat(&alt1b,&lon1b,&lat1b);

      if (output_base != NULL)
      {
        int abs_idx;
        if (! rel_to_abs_idx(cf.idx[j],cf.slicesPerSpot,&abs_idx))
        {
          fprintf(stderr,"Error converting to abs idx\n");
          exit(1);
        }
        pulse_count[count][0] = abs_idx/(float)cf.slicesPerSpot +
          cf_frame_count;
        slice_idx[count][0] = cf.idx[j];
        sigma0[count][0] = cf.sigma0[j];
        X[count][0] = cf.XK[j];
        Es[count][0] = cf.Es[j];
        En[count][0] = cf.En[j];
        // convert east azimuth to north azimuth
        azim[count][0] = pi/2.0 - cf.azimuth[j];
//        if (azim[count][0] < -pi) azim[count][0] += 2.0*pi;
//        if (azim[count][0] > -pi) azim[count][0] -= 2.0*pi;
        incang[count][0] = cf.incidence[j];
        range[count][0] = cf.R[j];
        lon[count][0] = lon1;
        lat[count][0] = lat1;

        slice_idx[count][1] = cf1b.idx[i];
        sigma0[count][1] = cf1b.sigma0[i];
        X[count][1] = cf1b.XK[i];
        Es[count][1] = cf1b.Es[i];
        En[count][1] = cf1b.En[i];
        azim[count][1] = cf1b.azimuth[i];
        incang[count][1] = cf1b.incidence[i];
        range[count][1] = cf1b.R[i];
        lon[count][1] = lon1b;
        lat[count][1] = lat1b;
      }

      float beta = 2.6915348;
      float esn_slice1 = cf.Es[j] + cf.En[j];
      float esn_slice2 = cf1b.Es[i] + cf1b.En[i];
      float en_spot1 = 1.0/(1.0 - cf.alpha)*
        (cf.EsnEcho - cf.EsnNoise)/beta;
      float en_spot2 = 1.0/(1.0 - cf1b.alpha)*
        (cf1b.EsnEcho - cf1b.EsnNoise)/beta;
      float q_slice1 = cf.En[j]/en_spot1;
      float q_slice2 = cf1b.En[i]/en_spot2;

      if (p_flag == 1)
      {
        printf("%d %d %.8g %.8g %d %d %d %d %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g\n",
               cf.pulseCount, cf.idx[j],
               cf.time, cf1b.time,
               cf.beamNumber, cf1b.beamNumber,
               cf.measType[j], cf1b.measType[i],
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
               cf.EsnEcho, cf1b.EsnEcho,
               cf.EsnNoise, cf1b.EsnNoise,
               cf.rxGateDelay,cf1b.rxGateDelay,
               cf.alpha, cf1b.alpha,
               lon1,lon1b,lat1,lat1b,
               cf.azimuth[j], cf1b.azimuth[i],
               cf.incidence[j], cf1b.incidence[i],
               cf.R[j], cf1b.R[i],
               cf.GatGar[j], cf1b.GatGar[i],
               cf.rsat.Get(0), cf1b.rsat.Get(0),
               cf.rsat.Get(1), cf1b.rsat.Get(1),
               cf.rsat.Get(2), cf1b.rsat.Get(2),
               cf.attitude.GetRoll(), cf1b.attitude.GetRoll(),
               cf.attitude.GetPitch(), cf1b.attitude.GetPitch(),
               cf.attitude.GetYaw(), cf1b.attitude.GetYaw(),
               esn_slice1, esn_slice2,
               en_spot1, en_spot2,
               q_slice1, q_slice2);
      }
      count++;
      break; // done with this slice
    }
  }

  if (output_base != NULL) 
  {
    fprintf(stderr,"total_slices = %ld, total_spots = %ld, file_size = %ld\n",
           total_slices,
           total_spots,file_size);
    fprintf(stderr,"count = %d, spot_count = %d, cf.Size = %d\n",
      count,spot_count, cf.Size());

    float** snr = (float**)make_array(sizeof(float),2,count,1);
    int* use = (int*)malloc(sizeof(int)*count);
    if (use == NULL || snr == NULL)
    {
      fprintf(stderr,"Error allocating memory in check_l1b\n");
      exit(1);
    }

    // Fill snr array (dB)
    for (int i = 0; i < count; i++)
      snr[i][0] = 10.0/log(10.0)*log(Es[i][0]/En[i][0]);

    char filename[1024];
    for (int rel_i = -4; rel_i <= 4; rel_i++)
    {
      if (rel_i == 0) rel_i = 1;
      for (int i = 0; i < count; i++)
      {
        if (slice_idx[i][0] == rel_i) use[i] = 1; else use[i] = 0;
      }

      if (rel_i < 0) sprintf(filename,"%s.sigma0.m%d",output_base,-rel_i);
      else sprintf(filename,"%s.sigma0.%d",output_base,rel_i);
      plot4(filename,count,use,dBerr,pulse_count,Es,En,X,sigma0,"pulse count",
        "Es errors (dB)","En errors (dB)","X errors (dB)","sigma0 errors (dB)");

      if (rel_i < 0) sprintf(filename,"%s.loc.m%d",output_base,-rel_i);
      else sprintf(filename,"%s.loc.%d",output_base,rel_i);
      plot4(filename,count,use,derr,pulse_count,lon,lat,incang,azim,
        "pulse count",
        "lon errors (deg)","lat errors (deg)",
        "inc. ang. errors (deg)","slice azimuth errors (deg)");
    }

    for (int i = 0; i < count; i++) use[i] = 1;
    sprintf(filename,"%s.sigma0.snr",output_base);
    plot4(filename,count,use,dBerr,snr,Es,En,X,sigma0,"SNR (dB)",
      "Es errors (dB)","En errors (dB)","X errors (dB)","sigma0 errors (dB)");

    sprintf(filename,"%s.X",output_base);
    plot4(filename,spot_count,use,dif_err,pulse_count,
          tx_doppler,x_doppler,rx_gate_delay,delta_freq,"pulse count",
      "commanded doppler errors (Hz)","computed doppler errors (Hz)",
      "rcv gate delay errors (sec)","delta frequency errors (Hz)");

  }

  if (output_base != NULL && s_flag == 1) 
  {
    float** snr = (float**)make_array(sizeof(float),2,Nstat,1);
    int* use = (int*)malloc(sizeof(int)*Nstat);
    if (use == NULL || snr == NULL)
    {
      fprintf(stderr,"Error allocating memory in check_l1b\n");
      exit(1);
    }

    for (int i=0; i < Nstat; i++)
    {
      if (! snr_idx.IndexToValue(i,&(snr[i][0])))
      {
        fprintf(stderr,"Error converting snr index to value\n");
        exit(1);
      }
      if (c_stat[i] == 0)
      {
        use[i] = 0;
      }
      else
      {
        Es[i][0] = e_Es_sum[i]/c_stat[i];
        En[i][0] = e_En_sum[i]/c_stat[i];
        sigma0[i][0] = e_sigma0_sum[i]/c_stat[i];
        X[i][0] = e_X_sum[i]/c_stat[i];
        Es[i][1] = e_Es_sum2[i]/c_stat[i];
        En[i][1] = e_En_sum2[i]/c_stat[i];
        sigma0[i][1] = e_sigma0_sum2[i]/c_stat[i];
        X[i][1] = e_X_sum2[i]/c_stat[i];
        use[i] = 1;
      }
    }

    char filename[1024];
    sprintf(filename,"%s.sigma0.stat",output_base);
    plot4(filename,Nstat,use,identitydB,snr,Es,En,X,sigma0,"SNR (dB)",
      "Average Es errors (dB)","Average En errors (dB)",
      "Average X errors (dB)","Average sigma0 errors (dB)");
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

int
match_cf_frame(
  CheckFrame* cf_target,
  CheckFrame* cf_source,
  FILE* source_fp)

{
  static long call_count = 0;
  long search_win = SEARCH_WINDOW;

  if (call_count < 3) search_win = 10000000; // look hard at the beginning
  float adiff;
  double rr;
  float minadiff = 100.0;
  double minrr = 10000.0;

  if (cf_source->ReadDataRec(source_fp))
  {  // check next frame first
    int rv = match_frame(cf_target,cf_source,&rr);
    if (rv == 1)
    {
//      fprintf(stderr,"%g %g %d\n",rr,
//          fabs(cf_source->antennaAziGi - cf_target->antennaAziGi),
//          cf_source->beamNumber);
      return(1);
    }
  }

  int position = ftell(source_fp);

  // backup to scan the search window
  if (fseek(source_fp,-search_win*cf_source->Size(),SEEK_CUR) != 0)
  {  // can't back up enough, just go to the beginning.
    if (fseek(source_fp,0,SEEK_SET) != 0)
    {
      fprintf(stderr,
        "check_l1b: Error seeking in primary checkfile\n");
       exit(1);
    }
  }

  int count = 0;
  int found = 0;

  while (cf_source->ReadDataRec(source_fp))
  {  // look for matching sim check frame
    int rv = match_frame(cf_target,cf_source,&rr);
    if (rv == 1)
    {
//      fprintf(stderr,"%g %g %d\n",rr,
//        fabs(cf_source->antennaAziGi - cf_target->antennaAziGi),
//        cf_source->beamNumber);
      found = 1;
      break;
    }

    adiff = fabs(cf_source->antennaAziGi - cf_target->antennaAziGi);
    if (adiff < minadiff) minadiff = adiff;
    if (rr < minrr) minrr = rr;

    count++;
    if (count > 2*search_win) break;
  }

  if (found == 0)
  {
    fprintf(stderr,"Can't match a target checkframe at index = %d\n",
    cf_target->pulseCount);
    fprintf(stderr,"minadiff = %g, minrr = %g\n",minadiff,minrr);
    if (fseek(source_fp,position,SEEK_SET) != 0)
    {
      fprintf(stderr,
              "checkframe_extract: Error seeking in primary checkfile\n");
       exit(1);
    }
  }

  call_count++;
  return(found);

}

int
match_frame(
  CheckFrame* cf_target,
  CheckFrame* cf_source,
  double *rr)

{
  int i;

  if (fabs(cf_source->antennaAziGi - cf_target->antennaAziGi) < 0.005 &&
      cf_source->beamNumber == cf_target->beamNumber )
  {
    for (i=0; i < cf_source->slicesPerSpot; i++)
    {
      if (cf_source->idx[i] == cf_target->idx[0]) break;
    } 
    if (i >= cf_source->slicesPerSpot)
    {
      fprintf(stderr,"can't match slice %d\n",cf_target->idx[0]);
      return(2);
    } 
    *rr = cf_source->centroid[i].SurfaceDistance(cf_target->centroid[0]);
    if (cf_source->centroid[i].SurfaceDistance(cf_target->centroid[0]) < 30.0)
    {
      return(1);
    }
  }

  return(0);
}

float identity(float* elem)
{
  return(elem[0]);
}

float identitydB(float* elem)
{
  return(10.0/log(10.0)*log(1.0 + elem[0]));
}

float dBerr(float* elem)
{
  if (elem[0] == 0.0)
  {
    return(0.0);
  }
  else if (elem[1]/elem[0] < 0)
  {  // opposite sign so can't do dB error
    return(0.0);
  }
  else
  {
    return(10.0/log(10.0)*log(elem[1]/elem[0]));
  }
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
  float err = rtd*(elem[0]-elem[1]);
  while (err > 360.0) err -= 360.0;
  if (err > 180.0) err -= 360.0;
  if (err < -180.0) err += 360.0;
  return(err);
}

float dif_err(float* elem)
{
  return(elem[0]-elem[1]);
}

int
plot4(char* filename,
      int count,
      int *use,
      float (*func)(float*),
      float** x,
      float** y1,
      float** y2,
      float** y3,
      float** y4,
      char* xlabel,
      char* y1label,
      char* y2label,
      char* y3label,
      char* y4label)

{
  FILE* fptr = fopen(filename,"w");
  if (fptr == NULL)
  {
    fprintf(stderr, "check_l1b: Error opening %s\n", filename);
    exit(1);
  }

  fprintf(fptr,"@with g0\n@g0 on\n");
  for (int i=0; i < count; i++)
  {
    if (use[i] == 1)
      fprintf(fptr,"%g %g\n",x[i][0],func(y4[i]));
  }
  fprintf(fptr,"&\n");
  fprintf(fptr,"@with g1\n@g1 on\n");
  for (int i=0; i < count; i++)
  {
    if (use[i] == 1)
      fprintf(fptr,"%g %g\n",x[i][0],func(y3[i]));
  }
  fprintf(fptr,"&\n");
  fprintf(fptr,"@with g2\n@g2 on\n");
  for (int i=0; i < count; i++)
  {
    if (use[i] == 1)
      fprintf(fptr,"%g %g\n",x[i][0],func(y2[i]));
  }
  fprintf(fptr,"&\n");
  fprintf(fptr,"@with g3\n@g3 on\n");
  for (int i=0; i < count; i++)
  {
    if (use[i] == 1)
      fprintf(fptr,"%g %g\n",x[i][0],func(y1[i]));
  }
  fprintf(fptr,"&\n");
  fprintf(fptr,"@with g0\n@g0 on\n");
  fprintf(fptr,"@view xmin 0.2\n@view xmax 0.8\n");
  fprintf(fptr,"@view ymin 0.1\n@view ymax 0.22\n");
  fprintf(fptr,"@autoscale xaxes\n");
  fprintf(fptr,"@autoscale yaxes\n");
  fprintf(fptr,"@s0 linestyle 0\n");
  fprintf(fptr,"@s0 symbol 2\n");
  fprintf(fptr,"@s0 symbol size 0.5\n");
  fprintf(fptr,"@subtitle \"%s\"\n",y4label);
  fprintf(fptr,"@xaxis label \"%s\"\n",xlabel);
  fprintf(fptr,"@with g1\n@g1 on\n");
  fprintf(fptr,"@view xmin 0.2\n@view xmax 0.8\n");
  fprintf(fptr,"@view ymin 0.28\n@view ymax 0.40\n");
  fprintf(fptr,"@autoscale xaxes\n");
  fprintf(fptr,"@autoscale yaxes\n");
  fprintf(fptr,"@s0 linestyle 0\n");
  fprintf(fptr,"@s0 symbol 2\n");
  fprintf(fptr,"@s0 symbol size 0.5\n");
  fprintf(fptr,"@subtitle \"%s\"\n",y3label);
  fprintf(fptr,"@with g2\n@g2 on\n");
  fprintf(fptr,"@view xmin 0.2\n@view xmax 0.8\n");
  fprintf(fptr,"@view ymin 0.46\n@view ymax 0.58\n");
  fprintf(fptr,"@autoscale xaxes\n");
  fprintf(fptr,"@autoscale yaxes\n");
  fprintf(fptr,"@s0 linestyle 0\n");
  fprintf(fptr,"@s0 symbol 2\n");
  fprintf(fptr,"@s0 symbol size 0.5\n");
  fprintf(fptr,"@subtitle \"%s\"\n",y2label);
  fprintf(fptr,"@with g3\n@g3 on\n");
  fprintf(fptr,"@view xmin 0.2\n@view xmax 0.8\n");
  fprintf(fptr,"@view ymin 0.64\n@view ymax 0.76\n");
  fprintf(fptr,"@autoscale xaxes\n");
  fprintf(fptr,"@autoscale yaxes\n");
  fprintf(fptr,"@s0 linestyle 0\n");
  fprintf(fptr,"@s0 symbol 2\n");
  fprintf(fptr,"@s0 symbol size 0.5\n");
  fprintf(fptr,"@subtitle \"%s\"\n",y1label);
  fclose(fptr);

  return(1);
}
