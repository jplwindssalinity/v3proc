//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

//----------------------------------------------------------------------
// NAME
//		test_hdf_l1b.C
//
// SYNOPSIS
//		test_hdf_l1b [ -c config_file ] [ -l l1b_hdf_file ]
//			[ -o output_file ] [ -m landmapfile ]
//
// DESCRIPTION
//		Generates output files containing ASCII output of a wind swatch
//		given a L1B HDF file.
//
// OPTIONS
//		[ -c config_file ]	Use the specified config file.
//		[ -l l1b_hdf_file ]		Use this HDF l1b file.
//		[ -o output_file ]	The name to use for output file.
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

#define OPTSTRING				"c:l:o:e:m:"


//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -c config_file ]", "[ -l l1b_hdf_file ]",
	"[ -o output_file ] [-e ephemeris_file]", "[-m landmapfile]", 0 };

// not always evil...
const char*		command = NULL;
char*			l1b_hdf_file = NULL;
char*			output_file = NULL;
FILE*           output_fp = stdout;
char*			ephemeris_file = NULL;
char*  landmapfile = NULL;




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
void ExtendEphemerisEnds(char* infile, char* outfile, float timestep, int nsteps){
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

	char* config_file = NULL;
	ConfigList config_list;
        LandMap landmap;
        landmap.Initialize(NULL,0);
	l1b_hdf_file = NULL;
	output_file = NULL;

	//------------------------//
	// parse the command line //
	//------------------------//

	command = no_path(argv[0]);

	if (argc == 1)
		usage(command, usage_array, 1);

	int c;
	while ((c = getopt(argc, argv, OPTSTRING)) != -1)
	{
		switch(c)
		{
		case 'c':
			config_file = optarg;
			if (! config_list.Read(config_file))
			{
				fprintf(stderr, "%s: error reading config file %s\n",
					command, config_file);
				exit(1);
			}
			break;
		case 'l':
			l1b_hdf_file = optarg;
			break;
                case 'm':
		  landmapfile = optarg;
                  landmap.Initialize(landmapfile,1);
		  break;
		case 'o':
            output_file = optarg;
			break;
		case 'e':
            ephemeris_file = optarg;
			break;
		case '?':
			usage(command, usage_array, 1);
			break;
		}
	}


	//---------------------//
	// check for arguments //
	//---------------------//

	if (! l1b_hdf_file)
	{
		l1b_hdf_file = config_list.Get(L1B_HDF_FILE_KEYWORD);
		if (l1b_hdf_file == NULL)
		{
			fprintf(stderr, "%s: must specify HDF L1B file\n", command);
			exit(1);
		}
	}

	//-----------------------//
	// read in HDF 1B file   //
	//-----------------------//
	L1B l1b(l1b_hdf_file);
	
	// Prepare to write
    if (output_file != 0)
    {
        if (l1b.OpenForWriting(output_file) == 0)
        {
            fprintf(stderr, "%s: cannot open %s for output\n",
                               argv[0], output_file);
            exit(1);
        }
      

       // opens temporary file to write ephemeris to  
       // this is later copied to ephemeris_file with extrapolated ephemeris added at both ends
       if (ephemeris_file != NULL)
        {
//        	if ((l1b.ephemeris_fp = fopen("l1bhdf_to_l1b_tmpfile", "w")) == NULL) {
        	if ((l1b.ephemeris_fp = fopen(ephemeris_file, "w")) == NULL) {
	            fprintf(stderr, "%s: cannot open l1bhdf_to_l1b_tmpfile for output\n",
                               argv[0]);
    	        exit(1);
        	}
        }

		//-----------------------//
		// write out as SVT L1B  //
		//-----------------------//


       int frames_written = 0;
       while (l1b.frame.ReadPureHdfFrame()) 
	    {
	      if (!l1b.WriteEphemerisRec()) {
	            fprintf(stderr, "%s: writing to %s failed.\n",
	                               argv[0], ephemeris_file);
	            exit(1);
	        }


	      // HACK-- remove measurements south of LOWER_LAT_LIM lat
	      # define LOWER_LAT_LIM       -50.0
          MeasSpotList* msl= &(l1b.frame.spotList);
          for(MeasSpot* spot=msl->GetHead();spot;){
            double alt,gdlat,lon;
            spot->scOrbitState.rsat.GetAltLonGDLat(&alt,&lon,&gdlat);
            if (gdlat*180/M_PI < LOWER_LAT_LIM) {
//                printf("lat = %lf\n", gdlat*180/M_PI);
                MeasSpot* toDelete = msl->RemoveCurrent();
                delete toDelete;
                spot = msl->GetCurrent();
            } else {
                spot=msl->GetNext();
            }
          }
          if (l1b.frame.spotList.NodeCount() == 0)
          {
            if ( !(l1b.frame.frame_i % 20) )
                  printf("Skipped frame %d because satellite is south of %lf latitude\n", l1b.frame.frame_i, LOWER_LAT_LIM);
            continue;
          }
          // END HACK
          
            // hack
//	        frames_written++;
//	        if (frames_written > 1200)
//	           break;      // so we stop writing everything and exit
//	        if (frames_written > 500)
//	        {
//	           if ( !(l1b.frame.frame_i % 20) )
//    	           printf("writing ephemeris for frame %d; frames_written = %d\n", l1b.frame.frame_i, frames_written);
//	           continue;   // so we continue writing the ephemeris after we've written 500 frames
//	        }
	        // end hack

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
    }

    float extension_step=54; // units are seconds
    float extension_num=100; // number of extra steps on each side

    // This may not be needed.
//   if (ephemeris_file != NULL)
//        {
//          fflush(l1b.ephemeris_fp);
//          ExtendEphemerisEnds("l1bhdf_to_l1b_tmpfile",ephemeris_file,extension_step,extension_num);
//	}

	return 0;
	
	
	
	
	#ifdef OLD
	
    HdfFile::StatusE rc;
    L1BHdf  l1bHdf(l1b_hdf_file, rc);
    if (rc != HdfFile::OK)
    {
        fprintf(stderr, "%s: cannot open HDF %s for input\n",
                               argv[0], l1b_hdf_file);
        exit(1);
    }
    fprintf(stdout, "%s: %s has %d records\n",
                               argv[0], l1b_hdf_file, l1bHdf.GetDataLength());

    //--------------------------------------------
    // configure L1B HDF object from config list
    //--------------------------------------------
    if ( ! ConfigL1BHdf(&l1bHdf, &config_list))
    {
        fprintf(stderr, "%s: config L1B HDF failed\n", argv[0]);
        exit(1);
    }
    
    if (output_file != 0)
    {
        if (l1bHdf.OpenForWriting(output_file) == 0)
        {
            fprintf(stderr, "%s: cannot open %s for output\n",
                               argv[0], output_file);
            exit(1);
        }
    }

	//-----------------------//
	// write out as SVT L1B  //
	//-----------------------//
    while (l1bHdf.ReadL1BHdfDataRec())
    {
        if ( ! l1bHdf.WriteDataRec())
        {
            fprintf(stderr, "%s: writing to %s failed.\n",
                               argv[0], output_file);
            exit(1);
        }
    }

    rc = l1bHdf.HdfFile::GetStatus();
    if (rc != HdfFile::OK && rc != HdfFile::NO_MORE_DATA)
    {
        fprintf(stderr, "%s: reading HDF %s failed before EOF is reached\n",
                           argv[0], l1b_hdf_file);
            exit(1);
    }

	return (0);
	#endif

} // main
