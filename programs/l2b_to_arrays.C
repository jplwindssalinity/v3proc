//==============================================================//
// Copyright (C) 1997-2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    l2b_to_arrays.C
//
// SYNOPSIS
//    l2b_to_arrays <l2b_file> <out_file> [rank] [config_file]
//
// DESCRIPTION
//    Writes l2b WVCs in array format (with cross-track and along-track index).
//    Use rank to select what to output. If rank=7, truth will be output, 
//        requires a config file to determine truth wind field.
//    
//    Output format (Matlab script to read output):
//    fid=fopen(filename,'r','l');
//    ati1=fread(fid,[1,1],'int32');  % first along-track index
//    nati=fread(fid,[1,1],'int32');  % number of along-track indices
//    ncti=fread(fid,[1,1],'int32');  % number of cross-track indices
//    spd=fread(fid,[ncti,nati],'float');  % speed array
//    dir=fread(fid,[ncti,nati],'float')*180/pi;  % direction array
//    lat=fread(fid,[ncti,nati],'float')*180/pi;  % latitude array
//    lon=fread(fid,[ncti,nati],'float')*180/pi;  % longitude array
//    fclose(fid);
//    
//    Empty cells have spd=-1 and dir,lat,lon = 0
//
//
// OPTIONS
//    None.
//
// OPERANDS
//    The following operands are supported:
//      <l2b_file>     The l2b file to read from
//      <out_file>     The name of the file to write to
//      [rank]         0 (default): selected
//                     -2: nearest to truth (requires config file)
//                     -1: nearest
//                     5: nudge
//                     6: hdfdirth
//                     7: truth (requires config file)
//                     1-4: rank-1
//      [config_file]  The config_file to read from. Currently only used for truth
//
// EXAMPLES
//    An example of a command line is:
//    l2b_to_arrays l2b_ku_S3_CENTROID.dat S3array.dat
//    l2b_to_arrays l2b_ku_S3_CENTROID.dat trutharray.dat 7 dfs_ku_dfs_12rpm.rdf 
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
//    Bryan W Stiles
//    Alexandra H Chau, modified 9/24/09
//      added rank=7 to get truth, fixed rank=-1 (nearest) bug, and  
//      updated the comments
//    Alexandra H Chau, modified 4/1/10
//      added rank = -2 to get nearest to truth
//    Brent Williams, modified 6/28/10
//      added output rain flag
//    Alexandra H Chau, modified 8/9/10
//      fixed bug in getting nearest to truth (rank=-2) -- was sometimes crashing with bus error
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
#include <unistd.h>
#include "ConfigList.h"
#include "ConfigSimDefs.h"
#include "Misc.h"
#include "Wind.h"
#include "L2B.h"
#include "List.h"
#include "Array.h"

using std::list;
using std::map;

//-----------//
// TEMPLATES //
//-----------//

template class List<StringPair>;
template class List<EarthPosition>;
template class List<WindVectorPlus>;
template class List<AngleInterval>;
template class std::list<string>;
template class std::map<string,string,Options::ltstr>;

//-----------//
// CONSTANTS //
//-----------//

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

const char* usage_array[] = { "<l2b_file>", "<out_file>", "[rank (-1=near, 0=sel, 5= nudge, 6= hdfdirth, 7=truth, -2=near truth)]", "[config_file (required for truth)]", 0};

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
    if (argc < 3 || argc > 5 )
        usage(command, usage_array, 1);

    int clidx = 1;
    const char* in_file = argv[clidx++];
    const char* out_file = argv[clidx++];
    int rank=0;
    if(argc>=4) rank=atoi(argv[clidx++]);
    const char* config_file = NULL;
    ConfigList config_list;
    if(argc>=5) // config_file parameter exists
    {
      config_file = argv[clidx++];
      if (! config_list.Read(config_file))
      {
	fprintf(stderr, "%s: error reading config file %s\n", command, config_file);
	exit(1);
      }
    }  
   
    fprintf(stderr,"rank is %d\n",rank);

    //------------------//
    // read in l2b file //
    //------------------//

    L2B l2b;

    
    if (! l2b.SmartRead(in_file))
      {
	fprintf(stderr, "%s: error reading L2B data record from file %s\n",
                command,in_file);
	exit(1);
      }
    
    //-------------------------------------------------------------------//
    // if rank = 7 (output truth) or -2, read truth parameters and field //
    //-------------------------------------------------------------------//
    if (rank==7 || rank==-2)
    {
      if (config_file)
      {
	// fprintf(stderr, "%s: config_file is %s\n", command, config_file); // for debugging
	char* truth_type = NULL;
	char* truth_file = NULL;
	// read truth type
	truth_type = config_list.Get(TRUTH_WIND_TYPE_KEYWORD);
	if (truth_type == NULL)
	{
	  fprintf(stderr, "%s: must specify truth windfield type \n", command);
	  exit(1);
	}
	// read truth file name
	truth_file = config_list.Get(TRUTH_WIND_FILE_KEYWORD);
	if (truth_file == NULL)
	{
	  fprintf(stderr, "%s: must specify truth windfield file in %s \n", command, config_file);
	  exit(1);
	}
	// read truth file
	WindField truth;
	// Check boundaries
	if (strcasecmp(truth_type,"SV") == 0)
	{
	  if (!config_list.GetFloat(WIND_FIELD_LAT_MIN_KEYWORD, &truth.lat_min) ||
	      !config_list.GetFloat(WIND_FIELD_LAT_MAX_KEYWORD, &truth.lat_max) ||
	      !config_list.GetFloat(WIND_FIELD_LON_MIN_KEYWORD, &truth.lon_min) ||
	      !config_list.GetFloat(WIND_FIELD_LON_MAX_KEYWORD, &truth.lon_max))
	  {
	    fprintf(stderr, "ConfigWindField: SV can't determine range of lat and lon\n");
	    return(0);
	  }
	}
	// Read truth
	if (! truth.ReadType(truth_file, truth_type))
	{
	  fprintf(stderr, "%s: error reading true wind field from file %s\n", command, truth_file);
	  exit(1);
	}
	// Scale wind speeds
	config_list.DoNothingForMissingKeywords();
	float scale;
	if (config_list.GetFloat(TRUTH_WIND_SPEED_MULTIPLIER_KEYWORD, &scale))
	{
	  truth.ScaleSpeed(scale);
	  fprintf(stderr, "Warning: scaling all wind speeds by %g\n", scale);
	}
	config_list.ExitForMissingKeywords();
	// use as fixed wind speed? 
	config_list.DoNothingForMissingKeywords();
	float fixed_speed;
	if (config_list.GetFloat(TRUTH_WIND_FIXED_SPEED_KEYWORD, &fixed_speed))
	{
	  truth.FixSpeed(fixed_speed);
	}
	float fixed_direction;
	if (config_list.GetFloat(TRUTH_WIND_FIXED_DIRECTION_KEYWORD, &fixed_direction))
	{
	  fixed_direction *= dtr;
	  truth.FixDirection(fixed_direction);
	}
	config_list.ExitForMissingKeywords();

	if (rank==7)
	  // MAKE TRUTH SELECTED (set selected field to be truth)
	  l2b.frame.swath.SelectTruth(&truth);
	else if (rank==-2)
	  // Make truth the nudge field (set nudge field to be the truth)
	  l2b.frame.swath.GetNudgeVectors(&truth);
      }
      else // config_file does not exist
      {
	fprintf(stderr, "%s: error -- no config file was specified, no truth available\n", command);
	exit(1);
      }
    }
    
    //------------------------//
    // Prepare array to write //
    //------------------------//

    int ncti=l2b.frame.swath.GetCrossTrackBins();
    int nati=l2b.frame.swath.GetAlongTrackBins();


    float ** spd=(float**)make_array(sizeof(float),2,nati,ncti);
    float ** dir=(float**)make_array(sizeof(float),2,nati,ncti);
    float ** lat=(float**)make_array(sizeof(float),2,nati,ncti);
    float ** lon=(float**)make_array(sizeof(float),2,nati,ncti);

    unsigned int ** flg=(unsigned int**)make_array(sizeof(unsigned int),2,nati,ncti);
    float ** impact=(float**)make_array(sizeof(float),2,nati,ncti);    

    int found_valid=0;
    int first_valid_ati=0, nvalid_ati=0;
    for(int a=0;a<nati;a++){
      for(int c=0;c<ncti;c++){
	    
	//fprintf(stderr,"Starting %d %d",a,c);
	    WVC* wvc=l2b.frame.swath.GetWVC(c,a);

            bool good = true;
            if(wvc){
	      WindVectorPlus* wvp;
	      
	      switch (rank)
	      {
	      case -2: // nearest to truth (already set nudgeWV to truth above)
		// fall through to rank = -1, select nearest to nudge
	      case -1: // nearest
		if(wvc->nudgeWV)
		  wvp=wvc->GetNearestToDirection(wvc->nudgeWV->dir);
		else wvp=NULL;
		break;
	      case 0: // selected
		wvp=wvc->selected;
		break;
	      case 5: // nudge
		wvp=wvc->nudgeWV;
		break;
	      case 6: // hdfdirth
		wvp=(WindVectorPlus*)wvc->specialVector;
		break;
	      case 7: // truth
		wvp=wvc->selected; // selected was changed to truth above
		break;
	      case 1: // get ambiguities (1-4)
	      case 2:
	      case 3:
	      case 4:
		wvp=wvc->ambiguities.GetByIndex(rank-1);
		break;
	      default: // invalid rank
		wvp=NULL;
		break;
	      }

	      /* This was the original code, has bug if rank ==-1
	      if(rank==-1 && wvc){
                if(wvc->nudgeWV)
		  wvp=wvc->GetNearestToDirection(wvc->nudgeWV->dir);
		else wvp=NULL;
		}
	      if(rank<=0) wvp=wvc->selected;
              else if(rank<5) wvp=wvc->ambiguities.GetByIndex(rank-1);
	      else if(rank==5) wvp=wvc->nudgeWV;
	      else wvp=(WindVectorPlus*)wvc->specialVector; */

	      if(wvp){
		spd[a][c]=wvp->spd;
		dir[a][c]=wvp->dir;
		lat[a][c]=wvc->lonLat.latitude;
		lon[a][c]=wvc->lonLat.longitude;
		flg[a][c]=wvc->qualFlag;//find correct object
                impact[a][c]=wvc->rainImpact;
	      }
	      else good =false;
	      nvalid_ati=a+1-first_valid_ati;
	      if(!found_valid){
		found_valid=1;
		first_valid_ati=a;
	      }
	    }
	    else good =false;

	    if(good==false){
              spd[a][c]=-1;
              dir[a][c]=0;
	      lat[a][c]=0;
	      lon[a][c]=0;
	      flg[a][c]=0;
              impact[a][c]=0;
	    }
	    //fprintf(stderr, "here %d %d\n",a,c);
      } // end c loop
    } // end a loop


    //-----------------//
    // Write the array //
    //-----------------//

    FILE* wfp=fopen(out_file,"w");
    if(!wfp){
      fprintf(stderr,"Cannot open file %s for writing\n",out_file);
      exit(1);
    }
    int f=first_valid_ati;
    int n=nvalid_ati;
    if(fwrite(&first_valid_ati,sizeof(int),1,wfp)!=1  ||
       fwrite(&nvalid_ati,sizeof(int),1,wfp)!=1  ||
       fwrite(&ncti,sizeof(int),1,wfp)!=1  ||
       !write_array(wfp,&spd[f],sizeof(float),2,n,ncti) ||
       !write_array(wfp,&dir[f],sizeof(float),2,n,ncti) ||
       !write_array(wfp,&lat[f],sizeof(float),2,n,ncti) ||
       !write_array(wfp,&lon[f],sizeof(float),2,n,ncti) ||
       !write_array(wfp,&flg[f],sizeof(unsigned int),2,n,ncti)  ||
       !write_array(wfp,&impact[f],sizeof(float),2,n,ncti)){
      fprintf(stderr,"Error writing to file %s\n",out_file);
      exit(1);
    }
    free_array(spd,2,nati,ncti);
    free_array(dir,2,nati,ncti);
    free_array(lat,2,nati,ncti);
    free_array(lon,2,nati,ncti);
    free_array(flg,2,nati,ncti);
    free_array(impact,2,nati,ncti);
    fclose(wfp);
    return (0);
}


