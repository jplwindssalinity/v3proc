//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

//----------------------------------------------------------------------
// NAME
//		get_DopRan_scan
//
// SYNOPSIS
//		get_DopRan_scan <sim_config_file> <beam number> <time>
//
// DESCRIPTION
//		Generates plottable Ideal Doppler and Range file for one scan.
//              writes to stdout
//
// OPTIONS
//		None.
//
// OPERANDS
//		The following operands are supported:
//		<sim_config_file>	The sim_config_file needed listing
//								all input parameters, input files, and
//								output files.
//
//		<beam_number>		0=inner 1=outer
//
// EXAMPLES
//		An example of a command line is:
//			% get_DopRan_scan X.cfg 0 0
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
//		Bryan W. Stiles
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
#include "Tracking.h"
#include "Tracking.C"
#include "QscatConfig.h"

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
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;

//-----------//
// CONSTANTS //
//-----------//

#define EQX_TIME_TOLERANCE	0.1

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

const char* usage_array[] = { "<sim_config_file>", "<beam_no>", "<time>", 0};

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

	if (argc != 4)
		usage(command, usage_array, 1);

	int arg_idx = 1;
	const char* config_file = argv[arg_idx++];
        int beam_no =atoi(argv[arg_idx++]);
        float inst_time =atof(argv[arg_idx++]);
        float azimuth;

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

	//----------------------------------------------//
	// create a spacecraft and spacecraft simulator //
	//----------------------------------------------//

	Spacecraft spacecraft;
	if (! ConfigSpacecraft(&spacecraft, &config_list))
	{
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

	//-----------------------------------------------//
	// create QSCAT and QSCAT simulator              //
	//-----------------------------------------------//

	Qscat qscat;
	if (! ConfigQscat(&qscat, &config_list))
	{
		fprintf(stderr, "%s: error configuring qscat\n", command);
		exit(1);
	}

	QscatSim qscat_sim;
	if (! ConfigQscatSim(&qscat_sim, &config_list))
	{
		fprintf(stderr, "%s: error configuring qscat simulator\n",
			command);
		exit(1);
	}

	//----------------------------//
	// create a Level 0.0 product //
	//----------------------------//

	L00 l00;
	if (! ConfigL00(&l00, &config_list))
	{
		fprintf(stderr, "%s: error configuring Level 0.0\n", command);
		exit(1);
	}
	//--------------------//
	// read the windfield //
	//--------------------//

	WindField windfield;
	if (! ConfigWindField(&windfield, &config_list))
	{
		fprintf(stderr, "%s: error configuring wind field\n", command);
		exit(1);
	}

	//-------------------------------------//
	// read the geophysical model function //
	//-------------------------------------//

	GMF gmf;
	if (! ConfigGMF(&gmf, &config_list))
	{
		fprintf(stderr, "%s: error configuring GMF\n", command);
		exit(1);
	}

	//--------------//
	// configure Kp //
	//--------------//
 
	Kp kp;
	if (! ConfigKp(&kp, &config_list))
	{
		fprintf(stderr, "%s: error configuring Kp\n", command);
		exit(1);
	}

	//------------------//
	// Setup a KpmField //
	//------------------//

	KpmField kpmField;
	if (! ConfigKpmField(&kpmField, &config_list))
	{
		fprintf(stderr, "%s: error configuring KpmField\n", command);
		exit(1);
	}

	//---------------------//
	// configure the times //
	//---------------------//

	double instrument_start_time, instrument_end_time;
	double spacecraft_start_time, spacecraft_end_time;


	qscat_sim.startTime = inst_time;
	spacecraft_start_time=-100.0;
        instrument_start_time=inst_time;
        instrument_end_time=6000.0;
        spacecraft_end_time=6100.0;

	//------------------//
	// set the eqx time //
	//------------------//

	double eqx_time =
		spacecraft_sim.FindPrevArgOfLatTime(instrument_start_time,
			EQX_ARG_OF_LAT, EQX_TIME_TOLERANCE);
	qscat.cds.SetEqxTime(eqx_time);

	//------------//
	// initialize //
	//------------//

	if (! qscat_sim.Initialize(&qscat))
	{
		fprintf(stderr, "%s: error initializing QSCAT simulator\n",
			command);
		exit(1);
	}

	if (! spacecraft_sim.Initialize(0.0))
	{
		fprintf(stderr, "%s: error initializing spacecraft simulator\n",
			command);
		exit(1);
	}


	//----------------------//
	// cycle through events //
	//----------------------//

	SpacecraftEvent spacecraft_event;
	spacecraft_event.time = spacecraft_start_time;

	float instrument_event_time;
	instrument_event_time = instrument_start_time;

	int spacecraft_done = 0;
	int instrument_done = 0;

	//-------------------------//
	// start with first events //
	//-------------------------//

	spacecraft_sim.DetermineNextEvent(&spacecraft_event);

	//---------------------//
	// loop through events //
	//---------------------//

	for (;;)
	{
		//---------------------------------------//
		// process spacecraft event if necessary //
		//---------------------------------------//

		if (! spacecraft_done)
		{
			if (spacecraft_event.time > spacecraft_end_time)
			{
				spacecraft_done = 1;
				continue;
			}
			if (spacecraft_event.time <= instrument_event_time ||
				instrument_done)
			{
				//------------------------------//
				// process the spacecraft event //
				//------------------------------//

				switch(spacecraft_event.eventId)
				{
				case SpacecraftEvent::EQUATOR_CROSSING:
					qscat.cds.SetEqxTime(spacecraft_event.time);
					break;
				default:
					break;
				}
				spacecraft_sim.DetermineNextEvent(&spacecraft_event);
			}
		}

		//---------------------------------------//
		// process instrument event if necessary //
		//---------------------------------------//

		if (! instrument_done)
		{
			if (instrument_event_time > instrument_end_time)
			{
				instrument_done = 1;
				continue;
			}
			if (instrument_event_time <= spacecraft_event.time ||
				spacecraft_done)
			{
			  //------------------------------//
			  // process the instrument event //
			  //------------------------------//


			  // process spacecraft stuff
			  spacecraft_sim.UpdateOrbit(instrument_event_time,
						     &spacecraft);
			  spacecraft_sim.UpdateAttitude(instrument_event_time, &spacecraft);
		  
			  // process instrument stuff
			  qscat.cds.SetTime(instrument_event_time);

			  qscat.cds.currentBeamIdx = beam_no;
			  for(int a=0;a<360;a+=10){
			    azimuth=a*dtr;
			    qscat.sas.antenna.azimuthAngle = azimuth;
			    double rtt=IdealRtt(&spacecraft,&qscat);
			    qscat.sas.antenna.azimuthAngle -= rtt *
			      qscat.sas.antenna.spinRate/2.0;
			    SetDelayAndFrequency(&spacecraft,&qscat);                    
			    double pulse_width = qscat.ses.txPulseWidth;
			    SesBeamInfo* ses_beam_info=qscat.ses.GetCurrentBeamInfo(beam_no);
			    
			    rtt=qscat.ses.rxGateDelay +
			      (ses_beam_info->rxGateWidth-pulse_width)/2.0;
			    double range=rtt/2.0*speed_light_kps;
			  
			    printf("%d %g %g %g %g\n",beam_no,instrument_event_time,azimuth*rtd,-(qscat.ses.txDoppler),range);


			  
			  }					
			  instrument_event_time=instrument_end_time+100;
					
			}
		}

		//---------------//
		// check if done //
		//---------------//

		if (instrument_done && spacecraft_done)
			break;
	}


	return (0);
}

