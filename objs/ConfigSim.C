//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_configsim_c[] =
	"@(#) $Id$";

#include "ConfigSim.h"
#include "InstrumentSim.h"
#include "SpacecraftSim.h"
#include "Misc.h"

//------------------//
// ConfigSpacecraft //
//------------------//

int
ConfigSpacecraft(
	Spacecraft*		spacecraft,
	ConfigList*		config_list)
{
	//-------------------//
	// nothing to do yet //
	//-------------------//

	spacecraft;
	config_list;
	return(1);
}

//---------------------//
// ConfigSpacecraftSim //
//---------------------//

int
ConfigSpacecraftSim(
	SpacecraftSim*	spacecraft_sim,
	ConfigList*		config_list)
{
	//------------------------------------//
	// configure the spacecraft simulator //
	//------------------------------------//
 
	double semi_major_axis;
	if (! config_list->GetDouble(SEMI_MAJOR_AXIS_KEYWORD, &semi_major_axis))
		return(0);
 
	double eccentricity;
	if (! config_list->GetDouble(ECCENTRICITY_KEYWORD, &eccentricity))
		return(0);
 
	double inclination;
	if (! config_list->GetDouble(INCLINATION_KEYWORD, &inclination))
		return(0);
 
	double long_of_asc_node;
	if (! config_list->GetDouble(LONG_OF_ASC_NODE_KEYWORD, &long_of_asc_node))
		return(0);
 
	double arg_of_perigee;
	if (! config_list->GetDouble(ARGUMENT_OF_PERIGEE_KEYWORD, &arg_of_perigee))
		return(0);
 
	double mean_anomaly;
	if (! config_list->GetDouble(MEAN_ANOMALY_KEYWORD, &mean_anomaly))
		return(0);
 
	spacecraft_sim->DefineOrbit(semi_major_axis, eccentricity, inclination,
		long_of_asc_node, arg_of_perigee, mean_anomaly);

	//-------------------------//
	// set up ephemeris period //
	//-------------------------//

	double eph_period;
	if (! config_list->GetDouble(EPHEMERIS_PERIOD_KEYWORD, &eph_period))
		return(0);
	spacecraft_sim->SetEphemerisPeriod(eph_period);
 
	return(1);
}

//------------------//
// ConfigInstrument //
//------------------//

int
ConfigInstrument(
	Instrument*		instrument,
	ConfigList*		config_list)
{
	//-----------------------//
	// configure the antenna //
	//-----------------------//

	if (! ConfigAntenna(&(instrument->antenna), config_list))
		return(0);

	return(1);
}

//---------------------//
// ConfigInstrumentSim //
//---------------------//

int
ConfigInstrumentSim(
	InstrumentSim*	instrument_sim,
	ConfigList*		config_list)
{
	//------------------------------------//
	// configure the instrument simulator //
	//------------------------------------//

	double tmp_double;

	if (! config_list->GetDouble(PRI_PER_BEAM_KEYWORD, &tmp_double))
		return(0);
	instrument_sim->SetPriPerBeam(tmp_double);
	
	if (! config_list->GetDouble(BEAM_B_TIME_OFFSET_KEYWORD, &tmp_double))
		return(0);
	instrument_sim->SetBeamBTimeOffset(tmp_double);

	//---------------------------------//
	// configure the antenna simulator //
	//---------------------------------//

	if (! ConfigAntennaSim(&(instrument_sim->antennaSim), config_list))
		return(0);

	return(1);
}

//---------------//
// ConfigAntenna //
//---------------//

int
ConfigAntenna(
	Antenna*		antenna,
	ConfigList*		config_list)
{
	//-----------------------//
	// configure the antenna //
	//-----------------------//

	int number_of_beams;
	if (! config_list->GetInt(NUMBER_OF_BEAMS_KEYWORD, &number_of_beams))
		return(0);
	antenna->numberOfBeams = number_of_beams;

	int encoder_bits;
	if (! config_list->GetInt(NUMBER_OF_ENCODER_BITS_KEYWORD, &encoder_bits))
		return(0);
	antenna->SetNumberOfEncoderBits(encoder_bits);

	//---------------------//
	// configure each beam //
	//---------------------//

	for (int beam_number = 0; beam_number < antenna->numberOfBeams;
		beam_number++)
	{
		if (! ConfigBeam((antenna->beam + beam_number), beam_number,
			config_list))
		{
			return(0);
		}
	}

	return(1);
}

//------------------//
// ConfigAntennaSim //
//------------------//

int
ConfigAntennaSim(
	AntennaSim*		antenna_sim,
	ConfigList*		config_list)
{
	//---------------------------------//
	// configure the antenna simulator //
	//---------------------------------//
 
	double spin_rate;
	if (! config_list->GetDouble(SPIN_RATE_KEYWORD, &spin_rate))
		return(0);
	antenna_sim->SetSpinRate(spin_rate);

	return(1);
}

//------------//
// ConfigBeam //
//------------//

int
ConfigBeam(
	Beam*			beam,
	int				beam_number,
	ConfigList*		config_list)
{
	char keyword[1024];
	char number[8];
	double tmp_double;

	sprintf(number, "%d", beam_number);

	substitute_string(BEAM_x_LOOK_ANGLE_KEYWORD, "x", number, keyword);
	if (! config_list->GetDouble(keyword, &tmp_double))
		return(0);
	beam->lookAngle = tmp_double * dtr;

	substitute_string(BEAM_x_AZIMUTH_ANGLE_KEYWORD, "x", number, keyword);
	if (! config_list->GetDouble(keyword, &tmp_double))
		return(0);
	beam->azimuthAngle = tmp_double * dtr;

	// The beam frame is defined by the look angle and the azimuth angle.
	beam->beamFrame.Set(0,beam->lookAngle,beam->azimuthAngle,3,2,1);

	return(1);
}

//---------------//
// ConfigL00File //
//---------------//

int
ConfigL00File(
	L00File*		l00_file,
	ConfigList*		config_list)
{
	//------------------------//
	// configure the l00 file //
	//------------------------//

	char* l00_filename = config_list->Get(L00_FILE_KEYWORD);
	if (l00_filename == NULL)
		return(0);
	l00_file->SetFilename(l00_filename);

	return(1);
}

//-----------------//
// ConfigWindField //
//-----------------//

int
ConfigWindField(
	WindField*		windfield,
	ConfigList*		config_list)
{
	//--------------------------//
	// configure the wind field //
	//--------------------------//

	char* windfield_filename = config_list->Get(WINDFIELD_FILE_KEYWORD);
	if (windfield_filename == NULL)
		return(0);
	if (! windfield->ReadVap(windfield_filename))
		return(0);
	return(1);
}

//-----------//
// ConfigGMF //
//-----------//

int
ConfigGMF(
	GMF*			gmf,
	ConfigList*		config_list)
{
	//-------------------//
	// configure the gmf //
	//-------------------//

	char* gmf_filename = config_list->Get(GMF_FILE_KEYWORD);
	if (gmf_filename == NULL)
		return(0);
	if (! gmf->ReadOldStyle(gmf_filename))
		return(0);
	return(1);
}
