//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		generate_kfactor
//
// SYNOPSIS
//    generate_kfactor [ -tT trueX_file ] [-r trueX_file] [-e estimatedX_file] 
//           <sim_config_file>    <output_file> 
// DESCRIPTION
//		Generates a Table of Kfactor values indexed by beam, 
//		azimuth, orbit position, and slice number. (XTABLE format)
//
// OPTIONS  
//              -T           Create true_x table only, write to output file
//                           No Kfactor table is created.
//
//		-t filename  Create true_x table, write to filename 
//              -r filename  Read true_x from filename
//              -e filename  Create estimated_x table , write to filename
//
// OPERANDS
//		The following operands are supported:
//		<sim_config_file>	The sim_config_file needed listing
//				        all input parameters, input files, and
//								output files.
//
//		<output_file>	        The K-factor output file.
//
//
//
// EXAMPLES
//		An example of a command line is:
//			% generate_kfactor -t true_x.dat
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
//		Bryan Stiles
//		bstiles@warukaze.jpl.nasa.gov
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
#include "Misc.h"
#include "ConfigList.h"
#include "List.h"
#include "List.C"
#include "Spacecraft.h"
#include "ConfigSim.h"
#include "Tracking.h"
#include "InstrumentGeom.h"
#include "BufferedList.h"
#include "BufferedList.C"
#include "XTable.h"

//-----------//
// TEMPLATES //
//-----------//

template class List<EarthPosition>;
template class List<StringPair>;
template class List<Meas>;
template class List<WindVectorPlus>;
template class List<MeasSpot>;
template class List<long>;
template class List<OffsetList>;
template class List<OrbitState>;
template class BufferedList<OrbitState>;

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

const char* usage_array[] = { "[-terT]" "<sim_config_file>", 
"<output_file>", 0};

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
        char* config_file=NULL;
        char* trueX_file=NULL;
        char* read_trueX_file=NULL;
        char* estX_file=NULL;
        char* output_file=NULL;


        int trueX_only=0;
        
	if (argc < 2)
		usage(command, usage_array, 1);

	int arg_idx = 1;
        
        int num_operands=0;

        while(arg_idx < argc){
          if(argv[arg_idx][0]!='-'){
            switch(num_operands){
	    case 0:
	      config_file=argv[arg_idx];
	      num_operands++;
	      break;
	    case 1:
	      output_file=argv[arg_idx];
	      num_operands++;
	      break;
	    default:
	      usage(command, usage_array, 1);
	    }
	  }
	  else if(strlen(argv[arg_idx])==2) {
	    switch(argv[arg_idx][1]){
	    case 'T':
	      trueX_only=1;
              break;
	      
	    case 't':
	      arg_idx++;
	      trueX_file=argv[arg_idx];
	      break;
	
	    case 'e':
	      arg_idx++;
	      estX_file=argv[arg_idx];
	      break;

	    case 'r':
	      arg_idx++;
	      read_trueX_file=argv[arg_idx];
	      break;


	    default:
	      fprintf(stderr,"%s: Bad option %s\n",command,argv[arg_idx]);
	      exit(1);
	      
	    }
	  }
	  else {
	    fprintf(stderr,"%s: Bad option %s\n",command,argv[arg_idx]);
	    exit(1);
	  }
	  arg_idx++;
	}
	
        if (num_operands !=2)  usage(command, usage_array, 1);

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

	int num_azimuths;
	if(! config_list.GetInt(XTABLE_NUM_AZIMUTHS_KEYWORD,&num_azimuths))
	  return(0);

        int num_orbit_position_bins;
	if(! config_list.GetInt(XTABLE_NUM_ORBIT_STEPS_KEYWORD,
				 &num_orbit_position_bins))
	  return(0);

	//-------------------------------------//
	// forcefeed several config parameters //
	//-------------------------------------//
 
	config_list.StompOrAppend(USE_RGC_KEYWORD, "0");
	config_list.StompOrAppend(USE_DTC_KEYWORD, "0");
	config_list.StompOrAppend(USE_KPC_KEYWORD, "0");
	config_list.StompOrAppend(USE_KPM_KEYWORD, "0");
	config_list.StompOrAppend(USE_KFACTOR_KEYWORD, "0");
        config_list.StompOrAppend(ATTITUDE_CONTROL_MODEL_KEYWORD,"NONE");
	config_list.StompOrAppend(CREATE_XTABLE_KEYWORD, "1");
	config_list.StompOrAppend(SYSTEM_TEMPERATURE_KEYWORD, "0");
	config_list.StompOrAppend(UNIFORM_SIGMA_FIELD_KEYWORD, "1");
	config_list.StompOrAppend(PTGR_NOISE_MEAN_KEYWORD, "0");
	config_list.StompOrAppend(PTGR_NOISE_STD_KEYWORD, "0");


	//----------------------------------------------//
	// create a spacecraft and spacecraft simulator //
	//----------------------------------------------//

	Spacecraft spacecraft;
	if (! ConfigSpacecraft(&spacecraft, &config_list)){
	  fprintf(stderr, "%s: error configuring spacecraft simulator\n",
		  command);
	  exit(1);
	}
	
	SpacecraftSim spacecraft_sim;
	if (! ConfigSpacecraftSim(&spacecraft_sim, &config_list))
	  {
	    fprintf(stderr, "%s: error configuring spacecraft simulator\n",
		    command);
	    exit(1);
	  }
	spacecraft_sim.LocationToOrbit(0.0, 0.0, 1);

	//-----------------------------------------------//
	// create an instrument                          //
	//-----------------------------------------------//

	Instrument instrument;
	if (! ConfigInstrument(&instrument, &config_list))
	  {
	    fprintf(stderr, "%s: error configuring instrument\n", command);
	    exit(1);
	  }
	Antenna* antenna = &(instrument.antenna);

	//------------------------------------------------//
	// Either read true_X from file or configure      //
	// an InstrumentSimAccurate object to compute it  //
	//------------------------------------------------//
  
        InstrumentSimAccurate instrument_sim_acc; 
	// only used if trueX is calculated

        XTable *trueX, *estX;
	XTable trueX_from_file; // only used if trueX is read in.

	if(read_trueX_file!=NULL){
	  if(!trueX_from_file.SetFilename(argv[1]))
	    {
	      fprintf(stderr,"%s :Bad filename for trueX table\n",command);
	      exit(1);
	    }
	  if(!trueX_from_file.Read())
	    {
	      fprintf(stderr,"%s :Error reading trueX table\n",command);
	      exit(1);
	    } 
	  trueX=&trueX_from_file;
	}
        else{
	   if(trueX_only) trueX_file=output_file;
           if(trueX_file != NULL){
	     config_list.StompOrAppend(XTABLE_FILENAME_KEYWORD, trueX_file);
	   }
           else{
	     config_list.StompOrAppend(XTABLE_FILENAME_KEYWORD, "dummy");
	   }
	   if (!ConfigInstrumentSimAccurate(&instrument_sim_acc, &config_list))
	     {
		fprintf(stderr, "%s: error configuring instrument simulator\n",
			command);
		exit(1);
	     }
           trueX=&(instrument_sim_acc.xTable);
	}

        //----------------------------------------------------//
        // Configure Standard Simulator for computing estX    //
        //----------------------------------------------------//

	InstrumentSim instrument_sim;
	if(estX_file != NULL){
	  config_list.StompOrAppend(XTABLE_FILENAME_KEYWORD, estX_file);
	}
	else{
	  config_list.StompOrAppend(XTABLE_FILENAME_KEYWORD, "dummy");
	}

	if (! ConfigInstrumentSim(&instrument_sim, &config_list))
	{
		fprintf(stderr, "%s: error configuring instrument simulator\n",
			command);
		exit(1);
	}

	estX=&(instrument_sim.xTable);

	//------------------------------//
	// start at an equator crossing //
	//------------------------------//

	double start_time =
		spacecraft_sim.FindNextArgOfLatTime(spacecraft_sim.GetEpoch(),
			EQX_ARG_OF_LAT, EQX_TIME_TOLERANCE);
	instrument.SetEqxTime(start_time);

	//------------//
	// initialize //
	//------------//

	if (! instrument_sim.Initialize(&(instrument.antenna)))
	{
		fprintf(stderr, "%s: error initializing instrument simulator\n",
			command);
		exit(1);
	}

	if (! instrument_sim_acc.Initialize(&(instrument.antenna)))
	{
		fprintf(stderr, "%s: error initializing instrument simulator\n",
			command);
		exit(1);
	}

	if (! spacecraft_sim.Initialize(start_time))
	{
		fprintf(stderr, "%s: error initializing spacecraft simulator\n",
			command);
		exit(1);
	}

	//-----------//
	// variables //
	//-----------//


	//--------------------//
	// loop through orbit //
	//--------------------//

	L00 l00_dummy;
	if (! ConfigL00(&l00_dummy, &config_list))
	{
		fprintf(stderr, "%s: error configuring Level 0.0\n", command);
		exit(1);
	}
	double orbit_period = spacecraft_sim.GetPeriod();
	double orbit_step_size = orbit_period / (double)num_orbit_position_bins;
	double azimuth_step_size = two_pi / (double)num_azimuths;

	for (int orbit_step = 0; orbit_step < num_orbit_position_bins; orbit_step++)
	{
		//--------------------//
		// calculate the time //
		//--------------------//

		// addition of 0.5 centers on orbit_step
		double time = start_time +
			orbit_step_size * ((double)orbit_step);

		//-----------------------//
		// locate the spacecraft //
		//-----------------------//

		spacecraft_sim.UpdateOrbit(time, &spacecraft);

		//-------------------------//
		// set the instrument time //
		//-------------------------//

		instrument.SetTime(time);

		//--------------------//
		// step through beams //
		//--------------------//

		for (int beam_idx = 0; beam_idx < antenna->numberOfBeams;
			beam_idx++)
		{
			antenna->currentBeamIdx = beam_idx;

			//--------------------------------//
			// calculate baseband frequencies //
			//--------------------------------//


			for (int azimuth_step = 0; azimuth_step < num_azimuths;
				azimuth_step++)
			{
				antenna->azimuthAngle = azimuth_step_size *
					(double)azimuth_step;

				if(read_trueX_file == NULL){
				  instrument_sim_acc.ScatSim(&spacecraft,
					       &instrument,NULL,NULL,NULL,
					       &(l00_dummy.frame));
				}
				if(trueX_only == NULL){
				  instrument_sim.ScatSim(&spacecraft,
					       &instrument,NULL,NULL,NULL,
					       &(l00_dummy.frame));
				}			
			}

		}
	}

        XTable kfactor;
        if(!trueX_only){
	  /**** Calculate Kfactor table ***/
        
	  if(!MakeKfactorTable(trueX,estX,&kfactor)){
	    fprintf(stderr,"%s :Error calculating kfactor table\n",command);
	    exit(1);
	  }

	  /**** Write Kfactor Table ***/
	  if(!kfactor.SetFilename(output_file)){
	    exit(1); 
	  }

	  if(!kfactor.Write()){
	    fprintf(stderr,"%s :Error writing Kfactor table\n",command);
	    exit(1);
	  }
		  
	}
	if(trueX_file!=NULL){
	  if(!trueX->Write()){
	    fprintf(stderr,"%s :Error writing Kfactor table\n",command);
	    exit(1);
	  }
	}

	if(estX_file!=NULL){
	  if(!estX->Write()){
	    fprintf(stderr,"%s :Error writing Kfactor table\n",command);
	    exit(1);
	  }
	}
	exit (0);
}

