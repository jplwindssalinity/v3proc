//==============================================================//
// Copyright (C) 1998-2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//		checkframe_extract
//
// SYNOPSIS
//		checkframe_extract <cfg file> <checkfile1> <checkfile2 <optional>>
//
// DESCRIPTION
//    Extract one field from a checkframe file and if a second is listed,
//    correlate it with the same data from the second.
//
// OPTIONS
//		None.
//
// EXAMPLES
//		An example of a command line is:
//			% checkframe_extract qscat.cfg simcheck.dat onebcheck.dat
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

#define SEARCH_WINDOW 100

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
/*
void print_slice_field(FILE* fptr, char* fieldname, CheckFrame* cf, int i,
                       int range_flag, double range_hi, double range_lo);
void print_spot_field(FILE* fptr, char* fieldname, CheckFrame* cf,
                      int range_flag, double range_hi, double range_lo);
int inrange(double value, double hi, double lo);
*/
double get_parameter(
  char* fieldname,
  CheckFrame* cf,
  int i);

//------------------//
// OPTION VARIABLES //
//------------------//

#define OPTSTRING               "fhc:i:j:o:r:"

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = {"[ -f ]", "[ -h (shows parameter names) ]",
   "[-c config file]", "[-i primary checkfile]", "[-j secondary checkfile]",
   "[-o output file]", "[-r lo:hi (parameter range for 1st parameter)]",
   "<parameter list>", 0};
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
  char* checkfile1 = NULL;
  char* checkfile2 = NULL;
  char* output_file = NULL;

  int f_flag = 0;
  double range_low,range_hi;
  int range_flag = 0;
  while (1)
  {
    int c = getopt(argc, argv, OPTSTRING);
    if (c == 'f') f_flag = 1;
    else if (c == 'h')
    {
      printf("-------------------------------------------------------\n");
      printf("Available parameter names:\n");
      printf("  Spots:  slicesPerSpot     deltaFreq   vx     rx\n");
      printf("          commandedDoppler  orbitFrac   vy     ry\n");
      printf("          commandedDelay    beamNumber  vz     rz\n");
      printf("          Xdoppler          EsnEcho     roll   alpha\n");
      printf("          XroundTripTime    EsnNoise    pitch  EsCal\n");
      printf("          antennaAziTx      spinRate    yaw    time\n");
      printf("          antennaAziGi      pulseCount\n");
      printf("  Slices: idx  sigma0  measType   GatGar\n");
      printf("          Es   wv_spd  incidence  alt\n");
      printf("          En   wv_dir  azimuth    lon\n");
      printf("          X    var     range      lat\n");
      printf("-------------------------------------------------------\n");
      usage(command, usage_array, 1);
      exit(1);
    }
    else if (c == 'c')
    {
      config_file = optarg;
    }
    else if (c == 'i')
    {
      checkfile1 = optarg;
    }
    else if (c == 'j')
    {
      checkfile2 = optarg;
    }
    else if (c == 'o')
    {
      output_file = optarg;
    }
    else if (c == 'r')
    {
      if (sscanf(optarg, "%lf:%lf", &range_low, &range_hi) != 2)
      {
          fprintf(stderr, "%s: error determining parameter range %s\n",
              command, optarg);
          exit(1);
      }
      range_flag = 1;
    }

    else if (c == -1) break;
  }

  if (config_file == NULL || checkfile1 == NULL || output_file == NULL)
  {
    usage(command, usage_array, 1);
    exit(-1);
  }

  int N = argc - optind;  // the number of parameters to extract
  if (N < 1 || N > 26+16)
  {
    usage(command, usage_array, 1);
  }
  int clidx = optind;

  //----------------------------//
  // Check for slice parameters //
  //----------------------------//

  int slice_p = 0;
  char* pname[16] = {"idx","measType","sigma0","wv_spd","wv_dir","x",
                     "alt","lon","lat",
                     "azimuth","incidence","Es","En","var","range","GatGar"};
  for (int i=0; i < N; i++)
  for (int j=0; j < 16; j++)
  {
    if (strcmp(argv[clidx+i],pname[j]) == 0) slice_p = 1;
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

	FILE* check1 = fopen(checkfile1,"r");
	if (check1 == NULL)
	{
	  fprintf(stderr,"%s: error opening check file %s\n",command,checkfile1);
	  exit(1);
	}

	FILE* output = fopen(output_file,"w");
	if (output == NULL)
	{
	  fprintf(stderr,"%s: error opening output file %s\n",command,output_file);
	  exit(1);
	}

    FILE* check2 = NULL;
    if (checkfile2 != NULL)
    {
	  check2 = fopen(checkfile2,"r");
	  if (output == NULL)
	  {
	    fprintf(stderr,"%s: error opening check file %s\n",command,checkfile2);
	    exit(1);
	  }
    }

	//--------------------//
	// Setup check frames //
	//--------------------//

	CheckFrame cf;
	if (! cf.Allocate(check1))
	{
	  fprintf(stderr, "%s: error allocating check frame 1\n", command);
	  exit(1);
    }
	CheckFrame cf2;
	if (check2)
    {
      if (! cf2.Allocate(check2))
	  {
	    fprintf(stderr, "%s: error allocating check frame 2\n", command);
	    exit(1);
      }
    }

  //------------------//
  // Get record count //
  //------------------//

/*
  if (fseek(check1,0,SEEK_END) != 0)
  {
    fprintf(stderr, "%s: error seeking to the end in %s\n", command,
            checkfile1);
    exit(1);
  }
*/
//  long file_size = ftell(check1);
//  long total_spots = file_size/cf.Size();
//  long total_slices = cf.slicesPerSpot * total_spots;
/*
  if (fseek(check1,0,SEEK_SET) != 0)
  {
    fprintf(stderr, "%s: error seeking to the beginning in %s\n", command,
      checkfile1);
	  exit(1);
  }
*/

  //------------//
  // check loop //
  //------------//

//  int cf_frame_count = 0; // counts all spots (cf frames).

  while (1)
  {  // for each check frame...

    //--------------------------------------------------------------//
    // If present, read secondary check frame and match to primary. //
    // Otherwise, just read a primary check frame.                  //
    //--------------------------------------------------------------//

    if (check2)
    {
      int ret;
      if (f_flag == 0)
      {
        ret = cf2.ReadDataRec(check2);
      }
      else
      {
        ret = cf2.ReadFortranStructure(check2);
      }
      if (! ret) break; // end of check2 file

      //------------------------------------------------------//
      // Match the secondary check frame with a primary frame //
      //------------------------------------------------------//

      if (match_cf_frame(&cf2,&cf,check1) == 0) continue;
    }
    else
    {
      int ret = cf.ReadDataRec(check1);
      if (! ret) break; // end of check1 file
    }

    int k;
    if (slice_p == 0)
    {  // No slice data, so write one line per spot.
      if (check2)
      {  // Write data for two checkframes.
        for (k=0; k < N; k++)
        {
          double param = get_parameter(argv[clidx+k],&cf,0);
          double param2 = get_parameter(argv[clidx+k],&cf2,0);
          if (k == 0 && range_flag && (param < range_low || param > range_hi))
          {
            break;  // 1st parameter out of range, so skip this line.
          }
          else
          {  // print the field
            fprintf(output,"%g %g ",param,param2);
          }
        }
      }
      else
      {  // Write data for one checkframe.
        for (k=0; k < N; k++)
        {
          double param = get_parameter(argv[clidx+k],&cf,0);
          if (k == 0 && range_flag && (param < range_low || param > range_hi))
          {
            break;  // 1st parameter out of range, so skip this line.
          }
          else
          {  // print the field
            fprintf(output,"%g ",param);
          }
        }
      }
      if (k > 0) fprintf(output,"\n"); // only close lines that were printed
    }
    else if (check2)
    {  // at least one slice field, so write one line per slice.
      for (int i=0; i < cf.slicesPerSpot; i++)
      for (int j=0; j < cf.slicesPerSpot; j++)
      {
        if (cf2.idx[i] != cf.idx[j] || cf2.idx[i] == 0 || cf.idx[j] == 0)
          continue;
        for (k=0; k < N; k++)
        {
          double param = get_parameter(argv[clidx+k],&cf,j);
          double param2 = get_parameter(argv[clidx+k],&cf2,i);
          if (k == 0 && range_flag && (param < range_low || param > range_hi))
          {
            break;  // 1st parameter out of range, so skip this line.
          }
          else
          {  // print the field
            fprintf(output,"%g %g ",param,param2);
          }
        }
        if (k > 0) fprintf(output,"\n");
      }
    }
    else
    {  // at least one slice field, but only one checkfile.
      for (int j=0; j < cf.slicesPerSpot; j++)
      {
        if (cf.idx[j] == 0) continue;
        for (k=0; k < N; k++)
        {
          double param = get_parameter(argv[clidx+k],&cf,j);
          if (k == 0 && range_flag && (param < range_low || param > range_hi))
          {
            break;  // 1st parameter out of range, so skip this line.
          }
          else
          {  // print the field
            fprintf(output,"%g ",param);
          }
        }
        fprintf(output,"\n");
      }
    }
  }

  //-----------------//
  // close the files //
  //-----------------//

  fclose(check1);
  if (check2 == NULL) fclose(check2);
  fclose(output);

  return (0);
}

int
match_cf_frame(
  CheckFrame* cf_target,
  CheckFrame* cf_source,
  FILE* source_fp)

{

  if (cf_source->ReadDataRec(source_fp))
  {  // check next frame first
    if (cf_source->pulseCount == cf_target->pulseCount)
    {
      return(1);
    }
  }

  int position = ftell(source_fp);

  // backup to scan the search window
  if (fseek(source_fp,-SEARCH_WINDOW*cf_source->Size(),SEEK_CUR) != 0)
  {  // can't back up enough, just go to the beginning.
    if (fseek(source_fp,0,SEEK_SET) != 0)
    {
      fprintf(stderr,
        "checkframe_extract: Error seeking in primary checkfile\n");
       exit(1);
    }
  }

  int count = 0;
  int found = 0;

  while (cf_source->ReadDataRec(source_fp))
  {  // look for matching sim check frame
    if (cf_source->pulseCount == cf_target->pulseCount)
    {
      found = 1;
      break;
    }
    count++;
    if (count > 2*SEARCH_WINDOW) break;
  }

  if (found == 0)
  {
    fprintf(stderr,"Can't match a target checkframe at index = %d\n",
    cf_target->pulseCount);
    if (fseek(source_fp,position,SEEK_SET) != 0)
    {
      fprintf(stderr,
              "checkframe_extract: Error seeking in primary checkfile\n");
       exit(1);
    }
  }

  return(found);

}

/*
void print_spot_field(
  FILE* fptr,
  char* fieldname,
  CheckFrame* cf,
  int range_flag,
  double range_low,
  double range_hi)
{
  if (strcmp(fieldname,"slicesPerSpot") == 0)
  {
    if (range_flag == 0)
    {
      fprintf(fptr,"%d ",cf->slicesPerSpot);
    }
    else if (inrange((double)(cf->slicesPerSpot),range_low,range_hi))
    {
      fprintf(fptr,"%d ",cf->slicesPerSpot);
    }

  }
  if (strcmp(fieldname,"pulseCount") == 0)
  {
    if (range_flag == 0)
    {
      fprintf(fptr,"%d ",cf->pulseCount);
    }
    else if (inrange((double)(cf->pulseCount),range_low,range_hi))
    {
      fprintf(fptr,"%d ",cf->pulseCount);
    }
  }
  if (strcmp(fieldname,"time") == 0)
  {
    if (range_flag == 0)
    {
      fprintf(fptr,"%g ",cf->time);
    }
    else if (inrange((double)(cf->time),range_low,range_hi))
    {
      fprintf(fptr,"%g ",cf->time);
    }
  }
  if (strcmp(fieldname,"rx") == 0)
  {
    if (range_flag == 0)
    {
      fprintf(fptr,"%g ",cf->rsat.Get(0));
    }
    else if (inrange((double)(cf->rsat.Get(0)),range_low,range_hi))
    {
      fprintf(fptr,"%g ",cf->rsat.Get(0));
    }
  }
  if (strcmp(fieldname,"ry") == 0)
  {
    if (range_flag == 0)
    {
      fprintf(fptr,"%g ",cf->rsat.Get(1));
    }
    else if (inrange((double)(cf->rsat.Get(1)),range_low,range_hi))
    {
      fprintf(fptr,"%g ",cf->rsat.Get(1));
    }
  }
  if (strcmp(fieldname,"rz") == 0)
  {
    if (range_flag == 0)
    {
      fprintf(fptr,"%g ",cf->rsat.Get(2));
    }
    else if (inrange((double)(cf->rsat.Get(2)),range_low,range_hi))
    {
      fprintf(fptr,"%g ",cf->rsat.Get(2));
    }
  }
  if (strcmp(fieldname,"vx") == 0)
  {
    if (range_flag == 0)
    {
      fprintf(fptr,"%g ",cf->vsat.Get(0));
    }
    else if (inrange((double)(cf->vsat.Get(0)),range_low,range_hi))
    {
      fprintf(fptr,"%g ",cf->vsat.Get(0));
    }
  }
  if (strcmp(fieldname,"vy") == 0)
  {
    if (range_flag == 0)
    {
      fprintf(fptr,"%g ",cf->vsat.Get(1));
    }
    else if (inrange((double)(cf->vsat.Get(1)),range_low,range_hi))
    {
      fprintf(fptr,"%g ",cf->vsat.Get(1));
    }
  }
  if (strcmp(fieldname,"vz") == 0)
  {
    if (range_flag == 0)
    {
      fprintf(fptr,"%g ",cf->vsat.Get(2));
    }
    else if (inrange((double)(cf->vsat.Get(2)),range_low,range_hi))
    {
      fprintf(fptr,"%g ",cf->vsat.Get(2));
    }
  }
  float roll,pitch,yaw;
  cf->attitude.GetRPY(&roll,&pitch,&yaw);
  if (strcmp(fieldname,"roll") == 0)
  {
    if (range_flag == 0)
    {
      fprintf(fptr,"%g ",roll);
    }
    else if (inrange((double)(roll),range_low,range_hi))
    {
      fprintf(fptr,"%g ",roll);
    }
  }
  if (strcmp(fieldname,"pitch") == 0)
  {
    if (range_flag == 0)
    {
      fprintf(fptr,"%g ",pitch);
    }
    else if (inrange((double)(pitch),range_low,range_hi))
    {
      fprintf(fptr,"%g ",pitch);
    }
  }
  if (strcmp(fieldname,"yaw") == 0)
  {
    if (range_flag == 0)
    {
      fprintf(fptr,"%g ",yaw);
    }
    else if (inrange((double)(yaw),range_low,range_hi))
    {
      fprintf(fptr,"%g ",yaw);
    }
  }
  if (strcmp(fieldname,"beamNumber") == 0)
  {
    if (range_flag == 0)
    {
      fprintf(fptr,"%d ",cf->beamNumber);
    }
    else if (inrange((double)(cf->beamNumber),range_low,range_hi))
    {
      fprintf(fptr,"%d ",cf->beamNumber);
    }
  }
  if (strcmp(fieldname,"orbitFrac") == 0)
  {
    if (range_flag == 0)
    {
      fprintf(fptr,"%g ",cf->orbitFrac);
    }
    else if (inrange((double)(cf->orbitFrac),range_low,range_hi))
    {
      fprintf(fptr,"%g ",cf->orbitFrac);
    }
  }
  if (strcmp(fieldname,"antennaAziTx") == 0)
  {
    if (range_flag == 0)
    {
      fprintf(fptr,"%g ",cf->antennaAziTx);
    }
    else if (inrange((double)(cf->antennaAziTx),range_low,range_hi))
    {
      fprintf(fptr,"%g ",cf->antennaAziTx);
    }
  }
  if (strcmp(fieldname,"antennaAziGi") == 0)
  {
    if (range_flag == 0)
    {
      fprintf(fptr,"%g ",cf->antennaAziGi);
    }
    else if (inrange((double)(cf->antennaAziGi),range_low,range_hi))
    {
      fprintf(fptr,"%g ",cf->antennaAziGi);
    }
  }
  if (strcmp(fieldname,"EsCal") == 0)
  {
    if (range_flag == 0)
    {
      fprintf(fptr,"%g ",cf->EsCal);
    }
    else if (inrange((double)(cf->EsCal),range_low,range_hi))
    {
      fprintf(fptr,"%g ",cf->EsCal);
    }
  }
  if (strcmp(fieldname,"deltaFreq") == 0)
  {
    if (range_flag == 0)
    {
      fprintf(fptr,"%g ",cf->deltaFreq);
    }
    else if (inrange((double)(cf->deltaFreq),range_low,range_hi))
    {
      fprintf(fptr,"%g ",cf->deltaFreq);
    }
  }
  if (strcmp(fieldname,"spinRate") == 0)
  {
    if (range_flag == 0)
    {
      fprintf(fptr,"%g ",cf->spinRate);
    }
    else if (inrange((double)(cf->spinRate),range_low,range_hi))
    {
      fprintf(fptr,"%g ",cf->spinRate);
    }
  }
  if (strcmp(fieldname,"commandedDoppler") == 0)
  {
    if (range_flag == 0)
    {
      fprintf(fptr,"%g ",cf->txDoppler);
    }
    else if (inrange((double)(cf->txDoppler),range_low,range_hi))
    {
      fprintf(fptr,"%g ",cf->txDoppler);
    }
  }
  if (strcmp(fieldname,"commandedDelay") == 0)
  {
    if (range_flag == 0)
    {
      fprintf(fptr,"%g ",cf->rxGateDelay);
    }
    else if (inrange((double)(cf->rxGateDelay),range_low,range_hi))
    {
      fprintf(fptr,"%g ",cf->rxGateDelay);
    }
  }
  if (strcmp(fieldname,"Xdoppler") == 0)
  {
    if (range_flag == 0)
    {
      fprintf(fptr,"%g ",cf->XdopplerFreq);
    }
    else if (inrange((double)(cf->XdopplerFreq),range_low,range_hi))
    {
      fprintf(fptr,"%g ",cf->XdopplerFreq);
    }
  }
  if (strcmp(fieldname,"XroundTripTime") == 0)
  {
    if (range_flag == 0)
    {
      fprintf(fptr,"%g ",cf->XroundTripTime);
    }
    else if (inrange((double)(cf->XroundTripTime),range_low,range_hi))
    {
      fprintf(fptr,"%g ",cf->XroundTripTime);
    }
  }
  if (strcmp(fieldname,"alpha") == 0)
  {
    if (range_flag == 0)
    {
      fprintf(fptr,"%g ",cf->alpha);
    }
    else if (inrange((double)(cf->alpha),range_low,range_hi))
    {
      fprintf(fptr,"%g ",cf->alpha);
    }
  }
  if (strcmp(fieldname,"EsnEcho") == 0)
  {
    if (range_flag == 0)
    {
      fprintf(fptr,"%g ",cf->EsnEcho);
    }
    else if (inrange((double)(cf->EsnEcho),range_low,range_hi))
    {
      fprintf(fptr,"%g ",cf->EsnEcho);
    }
  }
  if (strcmp(fieldname,"EsnNoise") == 0)
  {
    if (range_flag == 0)
    {
      fprintf(fptr,"%g ",cf->EsnNoise);
    }
    else if (inrange((double)(cf->EsnNoise),range_low,range_hi))
    {
      fprintf(fptr,"%g ",cf->EsnNoise);
    }
  }

}

void print_slice_field(FILE* fptr,
  char* fieldname,
  CheckFrame* cf,
  int i,
  int range_flag,
  double range_low,
  double range_hi)
{
  if (strcmp(fieldname,"idx") == 0)
  {
    if (range_flag == 0)
    {
      fprintf(fptr,"%d ",cf->idx[i]);
    }
    else if (inrange((double)(cf->idx[i]),range_low,range_hi))
    {
      fprintf(fptr,"%d ",cf->idx[i]);
    }
  }
  if (strcmp(fieldname,"measType") == 0)
  {
    if (range_flag == 0)
    {
      fprintf(fptr,"%d ",cf->measType[i]);
    }
    else if (inrange((double)(cf->measType[i]),range_low,range_hi))
    {
      fprintf(fptr,"%d ",cf->measType[i]);
    }
  }
  if (strcmp(fieldname,"wv_spd") == 0)
  {
    if (range_flag == 0)
    {
      fprintf(fptr,"%g ",cf->wv[i].spd);
    }
    else if (inrange((double)(cf->wv[i].spd),range_low,range_hi))
    {
      fprintf(fptr,"%g ",cf->wv[i].spd);
    }
  }
  if (strcmp(fieldname,"wv_dir") == 0)
  {
    if (range_flag == 0)
    {
      fprintf(fptr,"%g ",cf->wv[i].dir);
    }
    else if (inrange((double)(cf->wv[i].dir),range_low,range_hi))
    {
      fprintf(fptr,"%g ",cf->wv[i].dir);
    }
  }
  if (strcmp(fieldname,"sigma0") == 0)
  {
    if (range_flag == 0)
    {
      fprintf(fptr,"%g ",cf->sigma0[i]);
    }
    else if (inrange((double)(cf->sigma0[i]),range_low,range_hi))
    {
      fprintf(fptr,"%g ",cf->sigma0[i]);
    }
  }
  if (strcmp(fieldname,"X") == 0)
  {
    if (range_flag == 0)
    {
      fprintf(fptr,"%g ",cf->XK[i]);
    }
    else if (inrange((double)(cf->XK[i]),range_low,range_hi))
    {
      fprintf(fptr,"%g ",cf->XK[i]);
    }
  }
  if (strcmp(fieldname,"Es") == 0)
  {
    if (range_flag == 0)
    {
      fprintf(fptr,"%g ",cf->Es[i]);
    }
    else if (inrange((double)(cf->Es[i]),range_low,range_hi))
    {
      fprintf(fptr,"%g ",cf->Es[i]);
    }
  }
  if (strcmp(fieldname,"En") == 0)
  {
    if (range_flag == 0)
    {
      fprintf(fptr,"%g ",cf->En[i]);
    }
    else if (inrange((double)(cf->En[i]),range_low,range_hi))
    {
      fprintf(fptr,"%g ",cf->En[i]);
    }
  }
  if (strcmp(fieldname,"var") == 0)
  {
    if (range_flag == 0)
    {
      fprintf(fptr,"%g ",cf->var_esn_slice[i]);
    }
    else if (inrange((double)(cf->var_esn_slice[i]),range_low,range_hi))
    {
      fprintf(fptr,"%g ",cf->var_esn_slice[i]);
    }
  }
  if (strcmp(fieldname,"azimuth") == 0)
  {
    if (range_flag == 0)
    {
      fprintf(fptr,"%g ",rtd*cf->azimuth[i]);
    }
    else if (inrange((double)(rtd*cf->azimuth[i]),range_low,range_hi))
    {
      fprintf(fptr,"%g ",rtd*cf->azimuth[i]);
    }
  }
  if (strcmp(fieldname,"incidence") == 0)
  {
    if (range_flag == 0)
    {
      fprintf(fptr,"%g ",rtd*cf->incidence[i]);
    }
    else if (inrange((double)(rtd*cf->incidence[i]),range_low,range_hi))
    {
      fprintf(fptr,"%g ",rtd*cf->incidence[i]);
    }
  }
  double alt,lon,lat;
  cf->centroid[i].GetAltLonGDLat(&alt,&lon,&lat);
  if (strcmp(fieldname,"alt") == 0)
  {
    if (range_flag == 0)
    {
      fprintf(fptr,"%g ",alt);
    }
    else if (inrange((double)(alt),range_low,range_hi))
    {
      fprintf(fptr,"%g ",alt);
    }
  }
  if (strcmp(fieldname,"lon") == 0)
  {
    if (range_flag == 0)
    {
      fprintf(fptr,"%g ",rtd*lon);
    }
    else if (inrange((double)(rtd*lon),range_low,range_hi))
    {
      fprintf(fptr,"%g ",rtd*lon);
    }
  }
  if (strcmp(fieldname,"lat") == 0)
  {
    if (range_flag == 0)
    {
      fprintf(fptr,"%g ",rtd*lat);
    }
    else if (inrange((double)(rtd*lat),range_low,range_hi))
    {
      fprintf(fptr,"%g ",rtd*lat);
    }
  }
  if (strcmp(fieldname,"range") == 0)
  {
    if (range_flag == 0)
    {
      fprintf(fptr,"%g ",cf->R[i]);
    }
    else if (inrange((double)(cf->R[i]),range_low,range_hi))
    {
      fprintf(fptr,"%g ",cf->R[i]);
    }
  }
  if (strcmp(fieldname,"GatGar") == 0)
  {
    if (range_flag == 0)
    {
      fprintf(fptr,"%g ",cf->GatGar[i]);
    }
    else if (inrange((double)(cf->GatGar[i]),range_low,range_hi))
    {
      fprintf(fptr,"%g ",cf->GatGar[i]);
    }
  }

}

int inrange(double value, double hi, double lo)

{

  if (value < lo || value > hi) return(0);
  else return(1);

}

*/

double get_parameter(
  char* fieldname,
  CheckFrame* cf,
  int i)
{
  if (strcmp(fieldname,"slicesPerSpot") == 0)
  {
    return((double)(cf->slicesPerSpot));
  }
  if (strcmp(fieldname,"pulseCount") == 0)
  {
    return((double)(cf->pulseCount));
  }
  if (strcmp(fieldname,"time") == 0)
  {
    return((double)(cf->time));
  }
  if (strcmp(fieldname,"rx") == 0)
  {
    return((double)(cf->rsat.Get(0)));
  }
  if (strcmp(fieldname,"ry") == 0)
  {
    return((double)(cf->rsat.Get(1)));
  }
  if (strcmp(fieldname,"rz") == 0)
  {
    return((double)(cf->rsat.Get(2)));
  }
  if (strcmp(fieldname,"vx") == 0)
  {
    return((double)(cf->vsat.Get(0)));
  }
  if (strcmp(fieldname,"vy") == 0)
  {
    return((double)(cf->vsat.Get(1)));
  }
  if (strcmp(fieldname,"vz") == 0)
  {
    return((double)(cf->vsat.Get(2)));
  }
  float roll,pitch,yaw;
  cf->attitude.GetRPY(&roll,&pitch,&yaw);
  if (strcmp(fieldname,"roll") == 0)
  {
    return((double)(roll));
  }
  if (strcmp(fieldname,"pitch") == 0)
  {
    return((double)(pitch));
  }
  if (strcmp(fieldname,"yaw") == 0)
  {
    return((double)(yaw));
  }
  if (strcmp(fieldname,"beamNumber") == 0)
  {
    return((double)(cf->beamNumber));
  }
  if (strcmp(fieldname,"orbitFrac") == 0)
  {
    return((double)(cf->orbitFrac));
  }
  if (strcmp(fieldname,"antennaAziTx") == 0)
  {
    return((double)(cf->antennaAziTx));
  }
  if (strcmp(fieldname,"antennaAziGi") == 0)
  {
    return((double)(cf->antennaAziGi));
  }
  if (strcmp(fieldname,"EsCal") == 0)
  {
    return((double)(cf->EsCal));
  }
  if (strcmp(fieldname,"deltaFreq") == 0)
  {
    return((double)(cf->deltaFreq));
  }
  if (strcmp(fieldname,"spinRate") == 0)
  {
    return((double)(cf->spinRate));
  }
  if (strcmp(fieldname,"commandedDoppler") == 0)
  {
    return((double)(cf->txDoppler));
  }
  if (strcmp(fieldname,"commandedDelay") == 0)
  {
    return((double)(cf->rxGateDelay));
  }
  if (strcmp(fieldname,"Xdoppler") == 0)
  {
    return((double)(cf->XdopplerFreq));
  }
  if (strcmp(fieldname,"XroundTripTime") == 0)
  {
    return((double)(cf->XroundTripTime));
  }
  if (strcmp(fieldname,"alpha") == 0)
  {
    return((double)(cf->alpha));
  }
  if (strcmp(fieldname,"EsnEcho") == 0)
  {
    return((double)(cf->EsnEcho));
  }
  if (strcmp(fieldname,"EsnNoise") == 0)
  {
    return((double)(cf->EsnNoise));
  }

  // Slices

  if (strcmp(fieldname,"idx") == 0)
  {
      return((double)(cf->idx[i]));
  }
  if (strcmp(fieldname,"measType") == 0)
  {
      return((double)(cf->measType[i]));
  }
  if (strcmp(fieldname,"wv_spd") == 0)
  {
      return((double)(cf->wv[i].spd));
  }
  if (strcmp(fieldname,"wv_dir") == 0)
  {
      return((double)(cf->wv[i].dir));
  }
  if (strcmp(fieldname,"sigma0") == 0)
  {
      return((double)(cf->sigma0[i]));
  }
  if (strcmp(fieldname,"X") == 0)
  {
      return((double)(cf->XK[i]));
  }
  if (strcmp(fieldname,"Es") == 0)
  {
      return((double)(cf->Es[i]));
  }
  if (strcmp(fieldname,"En") == 0)
  {
      return((double)(cf->En[i]));
  }
  if (strcmp(fieldname,"var") == 0)
  {
      return((double)(cf->var_esn_slice[i]));
  }
  if (strcmp(fieldname,"azimuth") == 0)
  {
      return((double)(rtd*cf->azimuth[i]));
  }
  if (strcmp(fieldname,"incidence") == 0)
  {
      return((double)(rtd*cf->incidence[i]));
  }
  double alt,lon,lat;
  cf->centroid[i].GetAltLonGDLat(&alt,&lon,&lat);
  if (strcmp(fieldname,"alt") == 0)
  {
      return(alt);
  }
  if (strcmp(fieldname,"lon") == 0)
  {
      return(rtd*lon);
  }
  if (strcmp(fieldname,"lat") == 0)
  {
      return(rtd*lat);
  }
  if (strcmp(fieldname,"range") == 0)
  {
      return((double)(cf->R[i]));
  }
  if (strcmp(fieldname,"GatGar") == 0)
  {
      return((double)(cf->GatGar[i]));
  }

  return(-1);
}

