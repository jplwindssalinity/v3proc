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
#include "L17.h"
#include "L20.h"
#include "Constants.h"
#include "Distributions.h"

//---------------------//
// ConfigSpacecraft    //
//---------------------//
int
ConfigSpacecraft(
	Spacecraft* spacecraft,
	ConfigList* config_list)
{
  //-------------------------------//
  //Read in Attitude Order Indices //
  //-------------------------------//

  int order1, order2, order3;
  if (! config_list->GetInt(ATTITUDE_ORDER_1_KEYWORD, &order1))
    return(0);
  if (! config_list->GetInt(ATTITUDE_ORDER_2_KEYWORD, &order2))
    return(0);
  if (! config_list->GetInt(ATTITUDE_ORDER_3_KEYWORD, &order3))
    return(0);

  //------------------------------//
  // Initialize Attitude          //
  //------------------------------//

	if (! spacecraft->attitude.SetOrder(order1, order2, order3))
	{
		fprintf(stderr, "Error setting attitude order (%d, %d, %d)\n",
			order1, order2, order3);
		return(0);
	}

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

	double epoch;
	if (! config_list->GetDouble(ORBIT_EPOCH_KEYWORD, &epoch))
		return(0);

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

	double mean_anomaly_at_epoch;
	if (! config_list->GetDouble(MEAN_ANOMALY_AT_EPOCH_KEYWORD,
		&mean_anomaly_at_epoch))
	{
		return(0);
	}

	spacecraft_sim->DefineOrbit(epoch, semi_major_axis, eccentricity,
		inclination, long_of_asc_node, arg_of_perigee, mean_anomaly_at_epoch);

	//-------------------------//
	// set up ephemeris period //
	//-------------------------//

	double eph_period;
	if (! config_list->GetDouble(EPHEMERIS_PERIOD_KEYWORD, &eph_period))
		return(0);
	spacecraft_sim->SetEphemerisPeriod(eph_period);

	return(1);
}

//----------------------------------//
// ConfigAttitudeControlModel       //
//----------------------------------//

int 
ConfigAttitudeControlModel(SpacecraftSim* spacecraft_sim,
	ConfigList* config_list)
{
	GenericDist *roll, *pitch, *yaw;
	AttDist* ACEM;

	char* string;

        string=config_list->Get(ATTITUDE_CONTROL_MODEL_KEYWORD);
        if(! string)
		return(0);

	if(strcmp(string,"NONE")==0 || strcmp(string,"None")==0
	   || strcmp(string,"none")==0) return(1);


	else if(strcmp(string,"Gaussian")==0 || strcmp(string,"GAUSSIAN")==0
	   || strcmp(string,"gaussian")==0)
	{
	        roll=ConfigGaussian(ROLL_CONTROL_VARIANCE_KEYWORD,
			ROLL_CONTROL_MEAN_KEYWORD, config_list);
		if(roll==NULL)  return(0);
	        pitch=ConfigGaussian(PITCH_CONTROL_VARIANCE_KEYWORD,
			PITCH_CONTROL_MEAN_KEYWORD, config_list);
		if(pitch==NULL)   return(0);
	        yaw=ConfigGaussian(YAW_CONTROL_VARIANCE_KEYWORD,
			YAW_CONTROL_MEAN_KEYWORD, config_list);
		if(yaw==NULL)  return(0);

	}


	else if(strcmp(string,"Uniform")==0 || strcmp(string,"UNIFORM")==0
	   || strcmp(string,"uniform")==0)
	{
	        roll=ConfigUniform(ROLL_CONTROL_RADIUS_KEYWORD,
			ROLL_CONTROL_MEAN_KEYWORD, config_list);
		   if(roll==NULL) return(0);
	        pitch=ConfigUniform(PITCH_CONTROL_RADIUS_KEYWORD,
			PITCH_CONTROL_MEAN_KEYWORD, config_list);
		   if(pitch==NULL) return(0);
	        yaw=ConfigUniform(YAW_CONTROL_RADIUS_KEYWORD,
			YAW_CONTROL_MEAN_KEYWORD, config_list);
		   if(yaw==NULL)return(0);
	}

	
	else if(strcmp(string,"Gaussian_Random_Velocity")==0
           || strcmp(string,"GAUSSIAN_RANDOM_VELOCITY")==0
	   || strcmp(string,"gaussian_random_velocity")==0)
	{
	        roll=ConfigGaussianRandomVelocity(
		CONTROL_SAMPLE_RATE_KEYWORD, ROLL_CONTROL_BOUND_KEYWORD,
		ROLL_CONTROL_MEAN_KEYWORD, ROLL_CONTROL_VARIANCE_KEYWORD,
			config_list);
		if(roll==NULL) return(0);
	        pitch=ConfigGaussianRandomVelocity(
		CONTROL_SAMPLE_RATE_KEYWORD, PITCH_CONTROL_BOUND_KEYWORD,
		PITCH_CONTROL_MEAN_KEYWORD, PITCH_CONTROL_VARIANCE_KEYWORD,
			config_list);
		if(pitch==NULL) return(0);
	        yaw=ConfigGaussianRandomVelocity(
		CONTROL_SAMPLE_RATE_KEYWORD, YAW_CONTROL_BOUND_KEYWORD,
		YAW_CONTROL_MEAN_KEYWORD, YAW_CONTROL_VARIANCE_KEYWORD,
			config_list);
		if(yaw==NULL) return(0);

	}

	else if(strcmp(string,"Uniform_Random_Velocity")==0
           || strcmp(string,"UNIFORM_RANDOM_VELOCITY")==0
	   || strcmp(string,"uniform_random_velocity")==0)
	{
	        roll=ConfigUniformRandomVelocity(
		CONTROL_SAMPLE_RATE_KEYWORD, ROLL_CONTROL_BOUND_KEYWORD,
		ROLL_CONTROL_MEAN_KEYWORD, ROLL_CONTROL_RADIUS_KEYWORD,
			config_list);
		if(roll==NULL) return(0);
	        pitch=ConfigUniformRandomVelocity(
		CONTROL_SAMPLE_RATE_KEYWORD, PITCH_CONTROL_BOUND_KEYWORD,
		PITCH_CONTROL_MEAN_KEYWORD, PITCH_CONTROL_RADIUS_KEYWORD,
			config_list);
		if(pitch==NULL) return(0);
	        yaw=ConfigUniformRandomVelocity(
		CONTROL_SAMPLE_RATE_KEYWORD, YAW_CONTROL_BOUND_KEYWORD,
		YAW_CONTROL_MEAN_KEYWORD, YAW_CONTROL_RADIUS_KEYWORD,
			config_list);
		if(yaw==NULL) return(0);

	}


	else{
           fprintf(stderr,"No such Attitude Control Model. \n");
	   fprintf(stderr,"Implemented models are GAUSSIAN, UNIFORM, \n");
	   fprintf(stderr,"GAUSSIAN_RANDOM_VELOCITY,\n");
	   fprintf(stderr,"UNIFORM_RANDOM_VELOCITY, and NONE. \n");
	   return(0);
	}

        ACEM=new AttDist(roll,pitch,yaw);
        spacecraft_sim->SetAttCntlModel(ACEM);


	return(1);
}

//----------------------------------//
// ConfigAttitudeKnowledgeModel     //
//----------------------------------//

int 
ConfigAttitudeKnowledgeModel(SpacecraftSim* spacecraft_sim,
	ConfigList* config_list)
{
	GenericDist *roll, *pitch, *yaw;
	AttDist* AKEM;

	char* string;

        string=config_list->Get(ATTITUDE_KNOWLEDGE_MODEL_KEYWORD);
        if(! string)
		return(0);

	if(strcmp(string,"NONE")==0 || strcmp(string,"None")==0
	   || strcmp(string,"none")==0) return(1);


	else if(strcmp(string,"Gaussian")==0 || strcmp(string,"GAUSSIAN")==0
	   || strcmp(string,"gaussian")==0)
	{
	        roll=ConfigGaussian(ROLL_KNOWLEDGE_VARIANCE_KEYWORD,
			ROLL_KNOWLEDGE_MEAN_KEYWORD, config_list);
		if(roll==NULL) return(0);
	        pitch=ConfigGaussian(PITCH_KNOWLEDGE_VARIANCE_KEYWORD,
			PITCH_KNOWLEDGE_MEAN_KEYWORD, config_list);
		if(pitch==NULL) return(0);
	        yaw=ConfigGaussian(YAW_KNOWLEDGE_VARIANCE_KEYWORD,
			YAW_KNOWLEDGE_MEAN_KEYWORD, config_list);
		if(yaw==NULL) return(0);

	}


	else if(strcmp(string,"Uniform")==0 || strcmp(string,"UNIFORM")==0
	   || strcmp(string,"uniform")==0)
	{
	        roll=ConfigUniform(ROLL_KNOWLEDGE_RADIUS_KEYWORD,
			ROLL_KNOWLEDGE_MEAN_KEYWORD, config_list);
		if(roll==NULL) return(0);
	        pitch=ConfigUniform(PITCH_KNOWLEDGE_RADIUS_KEYWORD,
			PITCH_KNOWLEDGE_MEAN_KEYWORD, config_list);
		if(pitch==NULL) return(0);
	        yaw=ConfigUniform(YAW_KNOWLEDGE_RADIUS_KEYWORD,
			YAW_KNOWLEDGE_MEAN_KEYWORD, config_list);
		if(yaw==NULL) return(0);
	}

	
	else if(strcmp(string,"Gaussian_Random_Velocity")==0
           || strcmp(string,"GAUSSIAN_RANDOM_VELOCITY")==0
	   || strcmp(string,"gaussian_random_velocity")==0)
	{
	        roll=ConfigGaussianRandomVelocity(
		KNOWLEDGE_SAMPLE_RATE_KEYWORD, ROLL_KNOWLEDGE_BOUND_KEYWORD,
		ROLL_KNOWLEDGE_MEAN_KEYWORD, ROLL_KNOWLEDGE_VARIANCE_KEYWORD,
			config_list);
		if(roll==NULL) return(0);
	        pitch=ConfigGaussianRandomVelocity(
		KNOWLEDGE_SAMPLE_RATE_KEYWORD, PITCH_KNOWLEDGE_BOUND_KEYWORD,
		PITCH_KNOWLEDGE_MEAN_KEYWORD, PITCH_KNOWLEDGE_VARIANCE_KEYWORD,
			config_list);
		if(pitch==NULL) return(0);
	        yaw=ConfigGaussianRandomVelocity(
		KNOWLEDGE_SAMPLE_RATE_KEYWORD, YAW_KNOWLEDGE_BOUND_KEYWORD,
		YAW_KNOWLEDGE_MEAN_KEYWORD, YAW_KNOWLEDGE_VARIANCE_KEYWORD,
			config_list);
		if(yaw==NULL) return(0);

	}

	else if(strcmp(string,"Uniform_Random_Velocity")==0
           || strcmp(string,"UNIFORM_RANDOM_VELOCITY")==0
	   || strcmp(string,"uniform_random_velocity")==0)
	{
	        roll=ConfigUniformRandomVelocity(
		KNOWLEDGE_SAMPLE_RATE_KEYWORD, ROLL_KNOWLEDGE_BOUND_KEYWORD,
		ROLL_KNOWLEDGE_MEAN_KEYWORD, ROLL_KNOWLEDGE_RADIUS_KEYWORD,
			config_list);
		if(roll==NULL) return(0);
	        pitch=ConfigUniformRandomVelocity(
		KNOWLEDGE_SAMPLE_RATE_KEYWORD, PITCH_KNOWLEDGE_BOUND_KEYWORD,
		PITCH_KNOWLEDGE_MEAN_KEYWORD, PITCH_KNOWLEDGE_RADIUS_KEYWORD,
			config_list);
		if(pitch==NULL) return(0);
	        yaw=ConfigUniformRandomVelocity(
		KNOWLEDGE_SAMPLE_RATE_KEYWORD, YAW_KNOWLEDGE_BOUND_KEYWORD,
		YAW_KNOWLEDGE_MEAN_KEYWORD, YAW_KNOWLEDGE_RADIUS_KEYWORD,
			config_list);
		if(yaw==NULL) return(0);

	}


	else{
           fprintf(stderr,"No such Attitude Control Model. \n");
	   fprintf(stderr,"Implemented models are GAUSSIAN, UNIFORM, \n");
	   fprintf(stderr,"GAUSSIAN_RANDOM_VELOCITY,\n");
	   fprintf(stderr,"UNIFORM_RANDOM_VELOCITY, and NONE. \n");
	   return(0);
	}

        AKEM=new AttDist(roll,pitch,yaw);
        spacecraft_sim->SetAttKnowModel(AKEM);


	return(1);
}


//------------------------------------------//
// Configuration Routines for Specific      //
// Noise Distributions                      //
//------------------------------------------//

	//----------------------------------//
	// ConfigGaussian                   //
	//----------------------------------//

Gaussian* 
ConfigGaussian(const char* variance_keyword,
	const char* mean_keyword,
	ConfigList* config_list)
{
	double variance, mean;

 	if(! config_list->GetDouble(variance_keyword,
	   &variance)) return(NULL);
 	if(! config_list->GetDouble(mean_keyword,
	   &mean)) return(NULL);
	Gaussian* new_g = new Gaussian((float)variance,(float)mean);
	return(new_g);

}

	//----------------------------------//
	// ConfigUniform                    //
	//----------------------------------//

Uniform* 
ConfigUniform(const char* radius_keyword,
	const char* mean_keyword,
	ConfigList* config_list)
{
	double radius, mean;

	if(! config_list->GetDouble(radius_keyword,
	  &radius)) return(NULL);
	if(! config_list->GetDouble(mean_keyword,
	  &mean)) return(NULL);
	Uniform* new_u = new Uniform((float)radius,float(mean));
	return(new_u);

}

	//--------------------------------------------//
	// ConfigGaussianRandomVelocity               //
	//--------------------------------------------//

RandomVelocity* 
ConfigGaussianRandomVelocity(const char* samprate_keyword,
	const char* bound_keyword, const char* mean_keyword, 
	const char* variance_keyword,
	ConfigList* config_list)
{
	double variance, mean, sample_rate, bound;
	GenericTimelessDist *velocity;

	if(! config_list->GetDouble(samprate_keyword,
	   &sample_rate)) return(NULL);


 	if(! config_list->GetDouble(variance_keyword,
	   &variance)) return(NULL);
 	if(! config_list->GetDouble(mean_keyword,
	   &mean)) return(NULL);
 	if(! config_list->GetDouble(bound_keyword,
	   &bound)) return(NULL);
	velocity = new Gaussian((float)variance,0.0);
	RandomVelocity* new_rv = new RandomVelocity(velocity, (float)sample_rate, (float)bound, (float)mean);
	return(new_rv);
}


	//------------------------------------------//
	// ConfigUniformRandomVelocity              //
	//------------------------------------------//

RandomVelocity* 
ConfigUniformRandomVelocity(const char* samprate_keyword,
	const char* bound_keyword, const char* mean_keyword, 
	const char* radius_keyword,
	ConfigList* config_list)
{
	double radius, mean, sample_rate, bound;
	GenericTimelessDist *velocity;

	if(! config_list->GetDouble(samprate_keyword,
	   &sample_rate)) return(NULL);


 	if(! config_list->GetDouble(radius_keyword,
	   &radius)) return(NULL);
 	if(! config_list->GetDouble(mean_keyword,
	   &mean)) return(NULL);
 	if(! config_list->GetDouble(bound_keyword,
	   &bound)) return(NULL);
	velocity=new Uniform((float)radius,0.0);
	RandomVelocity* new_rv = new RandomVelocity(velocity, (float)sample_rate, (float)bound, (float)mean);
	return(new_rv);
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

	//--------------------//
	// configure RF stuff //
	//--------------------//

	float chirp_rate;	// kHz/ms
	if (! config_list->GetFloat(CHIRP_RATE_KEYWORD, &chirp_rate))
		return(0);
	instrument->chirpRate = chirp_rate * KHZ_PER_MS_TO_HZ_PER_S;

	float chirp_start_m;	// kHz/ms
	if (! config_list->GetFloat(CHIRP_START_M_KEYWORD, &chirp_start_m))
		return(0);
	instrument->chirpStartM = chirp_start_m * KHZ_PER_MS_TO_HZ_PER_S;

	float chirp_rate_b;		// kHz
	if (! config_list->GetFloat(CHIRP_START_B_KEYWORD, &chirp_rate_b))
		return(0);
	instrument->chirpStartB = chirp_rate_b * KHZ_TO_HZ;

	float system_delay;		// us
	if (! config_list->GetFloat(SYSTEM_DELAY_KEYWORD, &system_delay))
		return(0);
	instrument->systemDelay = system_delay * US_TO_S;

	float receiver_gate_width;	// ms
	if (! config_list->GetFloat(RECEIVER_GATE_WIDTH_KEYWORD,
		&receiver_gate_width))
		return(0);
	instrument->receiverGateWidth = receiver_gate_width * MS_TO_S;

	float base_transmit_freq;	// GHz
	if (! config_list->GetFloat(BASE_TRANSMIT_FREQUENCY_KEYWORD,
		&base_transmit_freq))
		return(0);
	instrument->baseTransmitFreq = base_transmit_freq * GHZ_TO_HZ;

	float slice_bandwidth;
	if (! config_list->GetFloat(SLICE_BANDWIDTH_KEYWORD, &slice_bandwidth))
		return(0);
	instrument->sliceBandwidth = slice_bandwidth * KHZ_TO_HZ;

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
	//--------------------------------//
	// initialize start and end times //
	//--------------------------------//

	double start_time;
	if (! config_list->GetDouble(INSTRUMENT_START_TIME_KEYWORD, &start_time))
		return(0);
	instrument_sim->startTime = start_time;

	double end_time;
	if (! config_list->GetDouble(INSTRUMENT_END_TIME_KEYWORD, &end_time))
		return(0);
	instrument_sim->endTime = end_time;

	int slices_per_spot;
	if (! config_list->GetInt(L00_SLICES_PER_SPOT_KEYWORD, &slices_per_spot))
		return(0);
	instrument_sim->slicesPerSpot = slices_per_spot;
      
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

	double spin_rate;
	if (! config_list->GetDouble(SPIN_RATE_KEYWORD, &spin_rate))
		return(0);
	antenna->spinRate = spin_rate * rpm_to_radps;

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
		printf("Beam %d polarization (%c) not recognized\n",
			beam_number,tmp_char);
		return(0);
	}

	double pulse_width;		// ms
	substitute_string(BEAM_x_PULSE_WIDTH_KEYWORD, "x", number, keyword);
	if (! config_list->GetDouble(keyword, &pulse_width))
		return(0);
	beam->pulseWidth = pulse_width * MS_TO_S;

	double look_angle;		// deg
	substitute_string(BEAM_x_LOOK_ANGLE_KEYWORD, "x", number, keyword);
	if (! config_list->GetDouble(keyword, &look_angle))
		return(0);
	look_angle *= dtr;

	double azimuth_angle;	// deg
	substitute_string(BEAM_x_AZIMUTH_ANGLE_KEYWORD, "x", number, keyword);
	if (! config_list->GetDouble(keyword, &azimuth_angle))
		return(0);
	azimuth_angle *= dtr;

	substitute_string(BEAM_x_PATTERN_FILE_KEYWORD, "x", number, keyword);
	char* pattern_file = config_list->Get(keyword);
	if (pattern_file == NULL)
		return(0);
	if (! beam->ReadBeamPattern(pattern_file))
	{
		printf("Error while reading beam %d pattern file\n",beam_number);
		return(0);
	}

	beam->SetElectricalBoresight(look_angle, azimuth_angle);

	// ms
	substitute_string(BEAM_x_TIME_OFFSET_KEYWORD, "x", number, keyword);
	if (! config_list->GetDouble(keyword, &tmp_double))
		return(0);
	beam->timeOffset = tmp_double * MS_TO_S;

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

	int number_of_beams;
	if (! config_list->GetInt(NUMBER_OF_BEAMS_KEYWORD, &number_of_beams))
		return(0);

	int antenna_cycles_per_frame;
	if (! config_list->GetInt(L00_ANTENNA_CYCLES_PER_FRAME_KEYWORD,
		&antenna_cycles_per_frame))
	{
		return(0);
	}

	int slices_per_spot;
	if (! config_list->GetInt(L00_SLICES_PER_SPOT_KEYWORD, &slices_per_spot))
		return(0);

	if (! l00->AllocateBuffer(number_of_beams, antenna_cycles_per_frame,
		slices_per_spot))
	{
		return(0);
	}

	//-------------------------//
	// configure the l00 frame //
	//-------------------------//

	if (! l00->frame.Allocate(number_of_beams, antenna_cycles_per_frame,
		slices_per_spot))
	{
		return(0);
	}

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

	int number_of_beams;
	if (! config_list->GetInt(NUMBER_OF_BEAMS_KEYWORD, &number_of_beams))
		return(0);

	int antenna_cycles_per_frame;
	if (! config_list->GetInt(L00_ANTENNA_CYCLES_PER_FRAME_KEYWORD,
		&antenna_cycles_per_frame))
	{
		return(0);
	}

	int slices_per_spot;
	if (! config_list->GetInt(L00_SLICES_PER_SPOT_KEYWORD, &slices_per_spot))
		return(0);

	if (! l10->AllocateBuffer(number_of_beams, antenna_cycles_per_frame,
		slices_per_spot))
	{
		return(0);
	}

	//-------------------------//
	// configure the l10 frame //
	//-------------------------//

	if (! l10->frame.Allocate(number_of_beams, antenna_cycles_per_frame,
		slices_per_spot))
	{
		return(0);
	}

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

//-----------//
// ConfigL17 //
//-----------//

int
ConfigL17(
	L17*			l17,
	ConfigList*		config_list)
{
	//---------------------------//
	// configure the l17 product //
	//---------------------------//

	char* l17_filename = config_list->Get(L17_FILE_KEYWORD);
	if (l17_filename == NULL)
		return(0);
	l17->SetFilename(l17_filename);

	return(1);
}

//----------------//
// ConfigL17ToL20 //
//----------------//

int
ConfigL17ToL20(
	L17ToL20*		l17_to_l20,
	ConfigList*		config_list)
{
	double tmp;

	if (! config_list->GetDouble(SPD_TOLERANCE_KEYWORD, &tmp))
		return(0);
	l17_to_l20->spdTolerance = (float)tmp;

	if (! config_list->GetDouble(PHI_STEP_KEYWORD, &tmp))
		return(0);
	l17_to_l20->phiStep = (float)tmp * dtr;

	if (! config_list->GetDouble(PHI_BUFFER_KEYWORD, &tmp))
		return(0);
	l17_to_l20->phiBuffer = (float)tmp * dtr;

	if (! config_list->GetDouble(PHI_MAX_SMOOTHING_KEYWORD, &tmp))
		return(0);
	l17_to_l20->phiMaxSmoothing = (float)tmp * dtr;

	return(1);
}

//-----------//
// ConfigL20 //
//-----------//

int
ConfigL20(
	L20*			l20,
	ConfigList*		config_list)
{
	//---------------------------//
	// configure the l20 product //
	//---------------------------//

	char* l20_filename = config_list->Get(L20_FILE_KEYWORD);
	if (l20_filename == NULL)
		return(0);
	l20->SetFilename(l20_filename);

	int tmp_int;
	if (! config_list->GetInt(MEDIAN_FILTER_WINDOW_SIZE_KEYWORD, &tmp_int))
		return(0);
	l20->medianFilterWindowSize = tmp_int;

	if (! config_list->GetInt(MEDIAN_FILTER_MAX_PASSES_KEYWORD, &tmp_int))
		return(0);
	l20->medianFilterMaxPasses = tmp_int;

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
	ephemeris->SetInputFile(ephemeris_filename);

	ephemeris->SetMaxNodes(50);		// this should be calculated

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

//------------//
// ConfigGrid //
//------------//

int
ConfigGrid(
	Grid*			grid,
	ConfigList*		config_list)
{
	//---------------//
	// configure l17 //
	//---------------//

	if (! ConfigL17(&(grid->l17), config_list))
		return(0);

	//---------------------//
	// configure ephemeris //
	//---------------------//

	if (! ConfigEphemeris(&(grid->ephemeris), config_list))
		return(0);

	//------------------------//
	// configure rest of grid //
	//------------------------//

	double ct_res;
	if (! config_list->GetDouble(CROSSTRACK_RESOLUTION_KEYWORD, &ct_res))
		return(0);

	double at_res;
	if (! config_list->GetDouble(ALONGTRACK_RESOLUTION_KEYWORD, &at_res))
		return(0);

	grid->Allocate(ct_res, at_res, 2000.0, 5000.0);

	double start_time;
	if (! config_list->GetDouble(ALONGTRACK_START_TIME_KEYWORD, &start_time))
		return(0);

	grid->SetStartTime(start_time);

	return(1);
}
