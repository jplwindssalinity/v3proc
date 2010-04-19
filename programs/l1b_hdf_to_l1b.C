//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

//----------------------------------------------------------------------
// NAME
//		test_hdf_l1b.C
//
// SYNOPSIS
//		test_hdf_l1b config_file
//
// DESCRIPTION
//		Generates output files containing ASCII output of a wind swatch
//		given a L1B HDF file.
//
// OPTIONS
//		The following keywords in the config file are used:
//      required: L1B_HDF_FILE, L1B_FILE
//      optional: LANDMAP_FILE, EPHEMERIS_FILE, SIM_LAT/LON_MIN/MAX
//
// OPERANDS
//		None.
//
// EXAMPLES
//		An example of a command line is:
//			% test_hdf_l1b -c sally.cfg -l L1B_100.file -o l1b.out
//
// ENVIRONMENT
//		Not environment dependent.
//
// EXIT STATUS
//		The following exit values are returned:
//		0	Program executed successfully
//		>0	Program had an error
//
// NOTES
//		None.
//
// AUTHOR
//		Sally Chou
//		Sally.H.Chou@jpl.nasa.gov
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
#include <stdlib.h>
#include "Misc.h"
#include "Ephemeris.h"
#include "ConfigList.h"
#include "L1A.h"
#include "ConfigSim.h"
//#include "L1BHdf.h"
#include "L1AToL1B.h"
#include "Tracking.h"
#include "Tracking.C"
#include "QscatConfig.h"
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"


using std::list;
using std::map;

//-----------//
// TEMPLATES //
//-----------//

class AngleInterval;

template class List<AngleInterval>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<StringPair>;
template class List<Meas>;
template class List<EarthPosition>;
template class List<MeasSpot>;
template class List<WindVectorPlus>;
template class List<off_t>;
template class List<OffsetList>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;
template class std::list<string>;
template class std::map<string,string,Options::ltstr>;

//-----------//
// CONSTANTS //
//-----------//

//#define OPTSTRING				"c:l:o:e:m:"


//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "config_file", 0 };





void landFlagL1BFrame(L1BFrame* f, LandMap* l){
  MeasSpotList* msl= &(f->spotList);
  for(MeasSpot* spot=msl->GetHead();spot;spot=msl->GetNext()){
    for(Meas* meas=spot->GetHead();meas;meas=spot->GetNext()){
      double alt,gdlat,lon;
      meas->centroid.GetAltLonGDLat(&alt,&lon,&gdlat);
      meas->landFlag=l->IsLand(lon,gdlat);
    }
  }
}

//------------------------------------------------------------//
// Routine for extrapolating the ends of an ephemeris file    //
//------------------------------------------------------------//
#ifdef EXTEND_EPHEM
void ExtendEphemerisEnds(char* infile, char* outfile, float timestep, 
    int nsteps, char *output_file, const char *command){
	//----------------------------//
	// open the input ephem file //
	//---------------------------//

	FILE* ephem_fp = fopen(infile,"r");
	if (ephem_fp == NULL)
	{
		fprintf(stderr, "%s: error opening ephem file %s\n", command,
			infile);
		exit(1);
	}

	//------------------//
	// open output file //
	//------------------//

	FILE* output_fp = fopen(outfile, "w");
	if (output_fp == NULL)
	{
		fprintf(stderr, "%s: error opening output file %s\n", command,
			output_file);
		exit(1);
	}

        OrbitState os;
        OrbitState lastos;
        int first_time = 1;
        while (os.Read(ephem_fp))
	{
          lastos=os;
          if (first_time) {
             for(int c=-nsteps;c<0;c++){
	      os.time=lastos.time+c*timestep;
              os.rsat=lastos.rsat+lastos.vsat*(c*timestep);
              os.Write(output_fp);
	    }
            first_time = 0;
          }
	  lastos.Write(output_fp);
	}
        fclose(ephem_fp);

	for(int c=1;c<=nsteps;c++){
	  os.time=lastos.time+c*timestep;
	  os.rsat=lastos.rsat+lastos.vsat*(c*timestep);
	  os.Write(output_fp);
	}
        fclose(output_fp);

}
#endif
//--------------//
// MAIN PROGRAM //
//--------------//

int
main(
	int		argc,
	char*	argv[])
{
	//-----------//
	// variables //
	//-----------//
    const char*		command = NULL;
    char*			l1b_hdf_file = NULL;
    char*			output_file = NULL;
    char*			ephemeris_file = NULL;
    char*           landmapfile = NULL;
    float           lat_min, lon_min, lat_max, lon_max;

	char* config_file = NULL;
	ConfigList config_list;
        LandMap landmap;
        landmap.Initialize(NULL,0);

	//------------------------//
	// parse the command line //
	//------------------------//

	command = no_path(argv[0]);

	if (argc != 2)
		usage(command, usage_array, 1);
		
	config_file = argv[1];
	if (! config_list.Read(config_file))
	{
		fprintf(stderr, "%s: error reading config file %s\n",
			command, config_file);
		exit(1);
	}


	//---------------------//
	// check for config file parameters //
	//---------------------//

	l1b_hdf_file = config_list.Get(L1B_HDF_FILE_KEYWORD);
	if (l1b_hdf_file == NULL) {
		fprintf(stderr, "%s: config file must specify L1B_HDF_FILE\n", command);
		exit(1);
	} else {
	   printf("Using l1b HDF file: %s\n", l1b_hdf_file);
	}
	
	output_file = config_list.Get(L1B_FILE_KEYWORD);
	if (output_file == NULL) {
		fprintf(stderr, "%s: config file must specify L1B_FILE\n", command);
		exit(1);
	} else {
	   printf("Using l1b file: %s\n", output_file);
	}
	
	landmapfile = config_list.Get(LANDMAP_FILE_KEYWORD);
	if (landmapfile != NULL) {
	   landmap.Initialize(landmapfile,1);
	   printf("Using landmap file: %s\n", landmapfile);
	}

	ephemeris_file = config_list.Get(EPHEMERIS_FILE_KEYWORD);
	if (ephemeris_file != NULL) {
        printf("Using ephemeris file: %s\n", ephemeris_file);
	}
	
	// get the box which all data should be in
	#define GET_BOX_BOUNDS(keyword, var, default) \
        if (config_list.Get(keyword)) \
            config_list.GetFloat(keyword, &var); \
        else \
            var = default;
            
    GET_BOX_BOUNDS("SIM_LAT_MIN", lat_min, -90)
    GET_BOX_BOUNDS("SIM_LAT_MAX", lat_max, 90)
    GET_BOX_BOUNDS("SIM_LON_MIN", lon_min, 0)
    GET_BOX_BOUNDS("SIM_LON_MAX", lon_max, 360)
    
    printf("Converting data inside of box: lat min = %.1lf, lat max = %.1lf, lon min = %.1lf, lon max = %.1lf\n",
        lat_min, lat_max, lon_min, lon_max);

	//-----------------------//
	// read in HDF 1B file   //
	//-----------------------//
	L1B l1b(l1b_hdf_file);
	
	// Prepare to write
    if (l1b.OpenForWriting(output_file) == 0) {
        fprintf(stderr, "%s: cannot open l1b file %s for output\n",
                            argv[0], output_file);
        exit(1);
    }
    

    if (ephemeris_file != NULL)
    {
        #ifdef EXTEND_EPHEM
        // opens temporary file to write ephemeris to  
        // this is later copied to ephemeris_file with extrapolated ephemeris added at both ends
        if ((l1b.ephemeris_fp = fopen("l1bhdf_to_l1b_tmpfile", "w")) == NULL) {
        #else
        if ((l1b.ephemeris_fp = fopen(ephemeris_file, "w")) == NULL) {
        #endif
            fprintf(stderr, "%s: cannot open %s for output\n",
                            argv[0], ephemeris_file);
	        exit(1);
        }
    }

	//-----------------------//
	// write out as SVT L1B  //
	//-----------------------//


//    int frames_written = 0;
    while (l1b.frame.ReadPureHdfFrame()) 
    {
        // we want to write out the entire ephemeris regardless of whether it is 
        // in the bounding box given
        if (!l1b.WriteEphemerisRec()) {
            fprintf(stderr, "%s: writing ephemeris to %s failed.\n",
                               argv[0], ephemeris_file);
            exit(1);
        }

        // delete measurements outside of the bounding box
        MeasSpotList* msl= &(l1b.frame.spotList);
        for(MeasSpot* spot=msl->GetHead();spot; ) {
            double alt,lat,lon;
//            spot->scOrbitState.rsat.GetAltLonGDLat(&alt,&lon,&lat);
            if (spot->NodeCount() != 0) {
                Meas *m = spot->GetHead();
//                printf("meas * = %x, num meases = %d\n", m, spot->NodeCount());
                m->centroid.GetAltLonGDLat(&alt,&lon,&lat);
                if (lat*180/M_PI < lat_min || lat*180/M_PI > lat_max ||
                    lon*180/M_PI < lon_min || lon*180/M_PI > lon_max) {
                    // we are outside of the box, so delete the point
                    MeasSpot* toDelete = msl->RemoveCurrent();
                    delete toDelete;
                    spot = msl->GetCurrent();
                } else {
                    // inside of the box- keep measurement
                    spot=msl->GetNext();
                }
            } else {
                MeasSpot* toDelete = msl->RemoveCurrent();
                delete toDelete;
                spot = msl->GetCurrent();
            }
        }
        if (l1b.frame.spotList.NodeCount() == 0) {
            if ( !(l1b.frame.frame_i % 20) )
                printf("Skipped data in frame %d of %d because it is outside of given box (ephemeris still written)\n",
                    l1b.frame.frame_i, l1b.frame.num_l1b_frames);
            continue;
        }
        
        // hack-- write out only part of the file
//        frames_written++;
//        if (frames_written > 1200)
//           break;      // so we stop writing everything and exit
//        if (frames_written > 500)
//        {
//           if ( !(l1b.frame.frame_i % 20) )
//	           printf("writing ephemeris for frame %d; frames_written = %d\n", l1b.frame.frame_i, frames_written);
//           continue;   // so we continue writing the ephemeris after we've written 500 frames
//        }
        // end hack

        // write measurements (we will only get this far if we are in the bounding box)
        landFlagL1BFrame(&(l1b.frame),&landmap);
        if (l1b.WriteDataRec()) {
           if ( !(l1b.frame.frame_i % 20) )
                fprintf(stderr, "Successfully wrote %d frames of %d\n", 
                    l1b.frame.frame_i, l1b.frame.num_l1b_frames);
        }
        else {
            fprintf(stderr, "%s: writing to %s failed.\n",
                               argv[0], output_file);
            exit(1);
        }
    }
    

    #ifdef EXTEND_EPHEM
    float extension_step=54; // units are seconds
    float extension_num=100; // number of extra steps on each side
    if (ephemeris_file != NULL) {
          fflush(l1b.ephemeris_fp);
          ExtendEphemerisEnds("l1bhdf_to_l1b_tmpfile",ephemeris_file,extension_step,extension_num, output_file, command);
	}
	#endif

	return 0;
	
} // main
