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
#include "L00.h"
#include "L10.h"
#include "L15.h"

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

	double pri_per_beam;
	if (! config_list->GetDouble(PRI_PER_BEAM_KEYWORD, &pri_per_beam))
		return(0);
	antenna->priPerBeam = pri_per_beam;

	int encoder_bits;
	if (! config_list->GetInt(NUMBER_OF_ENCODER_BITS_KEYWORD, &encoder_bits))
		return(0);
	antenna->SetNumberOfEncoderBits(encoder_bits);

	double roll, pitch, yaw;
	if (! config_list->GetDouble(ANTENNA_PEDESTAL_ROLL_KEYWORD, &roll))
		return(0);
	if (! config_list->GetDouble(ANTENNA_PEDESTAL_PITCH_KEYWORD, &pitch))
		return(0);
	if (! config_list->GetDouble(ANTENNA_PEDESTAL_YAW_KEYWORD, &yaw))
		return(0);
	Attitude att;
	att.Set(roll, pitch, yaw, 1, 2, 3);
	antenna->SetPedestalAttitude(&att);

	//---------------------//
	// configure each beam //
	//---------------------//

	for (int beam_idx = 0; beam_idx < antenna->numberOfBeams; beam_idx++)
	{
		if (! ConfigBeam((antenna->beam + beam_idx), beam_idx + 1,
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
	char tmp_char;

	sprintf(number, "%d", beam_number);

	substitute_string(BEAM_x_POLARIZATION_KEYWORD, "x", number, keyword);
	if (! config_list->GetChar(keyword, &tmp_char))
		return(0);
	switch (tmp_char)
	{
	case 'V':
	case 'v':
		beam->polarization = V_POL;
		break;
	case 'H':
	case 'h':
		beam->polarization = H_POL;
		break;
	default:
		return(0);
	}

	double look_angle;
	substitute_string(BEAM_x_LOOK_ANGLE_KEYWORD, "x", number, keyword);
	if (! config_list->GetDouble(keyword, &look_angle))
		return(0);
	look_angle *= dtr;
	
	double azimuth_angle;
	substitute_string(BEAM_x_AZIMUTH_ANGLE_KEYWORD, "x", number, keyword);
	if (! config_list->GetDouble(keyword, &azimuth_angle))
		return(0);
	azimuth_angle *= dtr;

	beam->SetBeamGeometry(look_angle, azimuth_angle);

	substitute_string(BEAM_x_TIME_OFFSET_KEYWORD, "x", number, keyword);
	if (! config_list->GetDouble(keyword, &tmp_double))
		return(0);
	beam->timeOffset = tmp_double;

	return(1);
}

//-----------//
// ConfigL00 //
//-----------//

int
ConfigL00(
	L00*			l00,
	ConfigList*		config_list)
{
	//---------------------------//
	// configure the l00 product //
	//---------------------------//

	char* l00_filename = config_list->Get(L00_FILE_KEYWORD);
	if (l00_filename == NULL)
		return(0);
	l00->SetFilename(l00_filename);

	return(1);
}

//-----------//
// ConfigL10 //
//-----------//

int
ConfigL10(
	L10*			l10,
	ConfigList*		config_list)
{
	//---------------------------//
	// configure the l10 product //
	//---------------------------//

	char* l10_filename = config_list->Get(L10_FILE_KEYWORD);
	if (l10_filename == NULL)
		return(0);
	l10->SetFilename(l10_filename);

	return(1);
}

//-----------//
// ConfigL15 //
//-----------//

int
ConfigL15(
	L15*			l15,
	ConfigList*		config_list)
{
	//---------------------------//
	// configure the l15 product //
	//---------------------------//

	char* l15_filename = config_list->Get(L15_FILE_KEYWORD);
	if (l15_filename == NULL)
		return(0);
	l15->SetFilename(l15_filename);

	return(1);
}

//-----------------//
// ConfigEphemeris //
//-----------------//

int
ConfigEphemeris(
	Ephemeris*		ephemeris,
	ConfigList*		config_list)
{
	char* ephemeris_filename = config_list->Get(EPHEMERIS_FILE_KEYWORD);
	if (ephemeris_filename == NULL)
		return(0);
	ephemeris->SetFile(ephemeris_filename);

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
