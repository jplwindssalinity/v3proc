//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_configsim_c[] =
	"@(#) $Id$";

#include "ConfigSim.h"
#include "ConfigSimDefs.h"
#include "InstrumentSim.h"
#include "InstrumentSimAccurate.h"
#include "SpacecraftSim.h"
#include "XTable.h"
#include "Misc.h"
#include "L00.h"
#include "L10.h"
#include "L10ToL15.h"
#include "L15.h"
#include "L17.h"
#include "L20.h"
#include "Constants.h"
#include "Distributions.h"
#include "Tracking.h"

//------------------//
// ConfigSpacecraft //
//------------------//

int
ConfigSpacecraft(
	Spacecraft* spacecraft,
	ConfigList* config_list)
{
	//--------------------------------//
	// Read in Attitude Order Indices //
	//--------------------------------//

	int order1, order2, order3;
	if (! config_list->GetInt(ATTITUDE_ORDER_1_KEYWORD, &order1))
		return(0);
	if (! config_list->GetInt(ATTITUDE_ORDER_2_KEYWORD, &order2))
		return(0);
	if (! config_list->GetInt(ATTITUDE_ORDER_3_KEYWORD, &order3))
		return(0);

	//---------------------//
	// Initialize Attitude //
	//---------------------//

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

//----------------------------//
// ConfigAttitudeControlModel //
//----------------------------//

int
ConfigAttitudeControlModel(SpacecraftSim* spacecraft_sim,
	ConfigList* config_list)
{
	GenericDist *roll, *pitch, *yaw;
	AttDist* ACEM;

	char* string;

	string=config_list->Get(ATTITUDE_CONTROL_MODEL_KEYWORD);
	if (! string)
		return(0);

	if(strcmp(string,"NONE")==0 || strcmp(string,"None")==0
		|| strcmp(string,"none")==0) return(1);


	else if(strcmp(string,"Gaussian")==0 || strcmp(string,"GAUSSIAN")==0
			|| strcmp(string,"gaussian")==0)
	{
		roll=ConfigGaussian(ROLL_CONTROL_VARIANCE_KEYWORD,
			ROLL_CONTROL_MEAN_KEYWORD, config_list);
		if(roll==NULL)
			return(0);
		pitch=ConfigGaussian(PITCH_CONTROL_VARIANCE_KEYWORD,
			PITCH_CONTROL_MEAN_KEYWORD, config_list);
		if(pitch==NULL)
			return(0);
		yaw=ConfigGaussian(YAW_CONTROL_VARIANCE_KEYWORD,
			YAW_CONTROL_MEAN_KEYWORD, config_list);
		if(yaw==NULL)
			return(0);
	}
	else if(strcmp(string,"Uniform")==0 || strcmp(string,"UNIFORM")==0
			|| strcmp(string,"uniform")==0)
	{
		roll=ConfigUniform(ROLL_CONTROL_RADIUS_KEYWORD,
			ROLL_CONTROL_MEAN_KEYWORD, config_list);
		if(roll==NULL)
			return(0);
		pitch=ConfigUniform(PITCH_CONTROL_RADIUS_KEYWORD,
			PITCH_CONTROL_MEAN_KEYWORD, config_list);
		if(pitch==NULL)
			return(0);
		yaw=ConfigUniform(YAW_CONTROL_RADIUS_KEYWORD,
			YAW_CONTROL_MEAN_KEYWORD, config_list);
		if(yaw==NULL)
			return(0);
	}
	else if(strcmp(string,"Gaussian_Random_Velocity")==0
			|| strcmp(string,"GAUSSIAN_RANDOM_VELOCITY")==0
			|| strcmp(string,"gaussian_random_velocity")==0)
	{
		roll=ConfigGaussianRandomVelocity(CONTROL_SAMPLE_RATE_KEYWORD,
			ROLL_CONTROL_BOUND_KEYWORD, ROLL_CONTROL_MEAN_KEYWORD,
			ROLL_CONTROL_VARIANCE_KEYWORD, config_list);
		if(roll==NULL)
			return(0);
		pitch=ConfigGaussianRandomVelocity(CONTROL_SAMPLE_RATE_KEYWORD,
			PITCH_CONTROL_BOUND_KEYWORD, PITCH_CONTROL_MEAN_KEYWORD,
			PITCH_CONTROL_VARIANCE_KEYWORD, config_list);
		if(pitch==NULL)
			return(0);
		yaw=ConfigGaussianRandomVelocity(CONTROL_SAMPLE_RATE_KEYWORD,
			YAW_CONTROL_BOUND_KEYWORD, YAW_CONTROL_MEAN_KEYWORD,
			YAW_CONTROL_VARIANCE_KEYWORD, config_list);
		if(yaw==NULL)
			return(0);
	}
	else if(strcmp(string,"Uniform_Random_Velocity")==0
			|| strcmp(string,"UNIFORM_RANDOM_VELOCITY")==0
			|| strcmp(string,"uniform_random_velocity")==0)
	{
		roll=ConfigUniformRandomVelocity(
		CONTROL_SAMPLE_RATE_KEYWORD, ROLL_CONTROL_BOUND_KEYWORD,
		ROLL_CONTROL_MEAN_KEYWORD, ROLL_CONTROL_RADIUS_KEYWORD, config_list);
		if(roll==NULL)
			return(0);
		pitch=ConfigUniformRandomVelocity(
		CONTROL_SAMPLE_RATE_KEYWORD, PITCH_CONTROL_BOUND_KEYWORD,
		PITCH_CONTROL_MEAN_KEYWORD, PITCH_CONTROL_RADIUS_KEYWORD,
			config_list);
		if(pitch==NULL)
			return(0);
		yaw=ConfigUniformRandomVelocity(
		CONTROL_SAMPLE_RATE_KEYWORD, YAW_CONTROL_BOUND_KEYWORD,
		YAW_CONTROL_MEAN_KEYWORD, YAW_CONTROL_RADIUS_KEYWORD,
			config_list);
		if(yaw==NULL) return(0);

	}
	else
	{
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

//------------------------------//
// ConfigAttitudeKnowledgeModel //
//------------------------------//

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

	if (strcmp(string,"NONE")==0 || strcmp(string,"None")==0
		|| strcmp(string,"none")==0)
	{
		return(1);
	}
	else if (strcmp(string,"Gaussian")==0 || strcmp(string,"GAUSSIAN")==0
			|| strcmp(string,"gaussian")==0)
	{
		roll=ConfigGaussian(ROLL_KNOWLEDGE_VARIANCE_KEYWORD,
			ROLL_KNOWLEDGE_MEAN_KEYWORD, config_list);
		if (roll==NULL) return(0);
		pitch=ConfigGaussian(PITCH_KNOWLEDGE_VARIANCE_KEYWORD,
			PITCH_KNOWLEDGE_MEAN_KEYWORD, config_list);
		if (pitch==NULL) return(0);
		yaw=ConfigGaussian(YAW_KNOWLEDGE_VARIANCE_KEYWORD,
			YAW_KNOWLEDGE_MEAN_KEYWORD, config_list);
		if (yaw==NULL) return(0);

	}
	else if (strcmp(string,"Uniform")==0 || strcmp(string,"UNIFORM")==0
	   || strcmp(string,"uniform")==0)
	{
	        roll=ConfigUniform(ROLL_KNOWLEDGE_RADIUS_KEYWORD,
			ROLL_KNOWLEDGE_MEAN_KEYWORD, config_list);
		if (roll==NULL) return(0);
	        pitch=ConfigUniform(PITCH_KNOWLEDGE_RADIUS_KEYWORD,
			PITCH_KNOWLEDGE_MEAN_KEYWORD, config_list);
		if (pitch==NULL) return(0);
	        yaw=ConfigUniform(YAW_KNOWLEDGE_RADIUS_KEYWORD,
			YAW_KNOWLEDGE_MEAN_KEYWORD, config_list);
		if (yaw==NULL) return(0);
	}
	else if (strcmp(string,"Gaussian_Random_Velocity")==0
           || strcmp(string,"GAUSSIAN_RANDOM_VELOCITY")==0
	   || strcmp(string,"gaussian_random_velocity")==0)
	{
	        roll=ConfigGaussianRandomVelocity(
		KNOWLEDGE_SAMPLE_RATE_KEYWORD, ROLL_KNOWLEDGE_BOUND_KEYWORD,
		ROLL_KNOWLEDGE_MEAN_KEYWORD, ROLL_KNOWLEDGE_VARIANCE_KEYWORD,
			config_list);
		if (roll==NULL) return(0);
	        pitch=ConfigGaussianRandomVelocity(
		KNOWLEDGE_SAMPLE_RATE_KEYWORD, PITCH_KNOWLEDGE_BOUND_KEYWORD,
		PITCH_KNOWLEDGE_MEAN_KEYWORD, PITCH_KNOWLEDGE_VARIANCE_KEYWORD,
			config_list);
		if (pitch==NULL) return(0);
	        yaw=ConfigGaussianRandomVelocity(
		KNOWLEDGE_SAMPLE_RATE_KEYWORD, YAW_KNOWLEDGE_BOUND_KEYWORD,
		YAW_KNOWLEDGE_MEAN_KEYWORD, YAW_KNOWLEDGE_VARIANCE_KEYWORD,
			config_list);
		if (yaw==NULL) return(0);

	}

	else if (strcmp(string,"Uniform_Random_Velocity")==0
           || strcmp(string,"UNIFORM_RANDOM_VELOCITY")==0
	   || strcmp(string,"uniform_random_velocity")==0)
	{
	        roll=ConfigUniformRandomVelocity(
		KNOWLEDGE_SAMPLE_RATE_KEYWORD, ROLL_KNOWLEDGE_BOUND_KEYWORD,
		ROLL_KNOWLEDGE_MEAN_KEYWORD, ROLL_KNOWLEDGE_RADIUS_KEYWORD,
			config_list);
		if (roll==NULL) return(0);
	        pitch=ConfigUniformRandomVelocity(
		KNOWLEDGE_SAMPLE_RATE_KEYWORD, PITCH_KNOWLEDGE_BOUND_KEYWORD,
		PITCH_KNOWLEDGE_MEAN_KEYWORD, PITCH_KNOWLEDGE_RADIUS_KEYWORD,
			config_list);
		if (pitch==NULL) return(0);
	        yaw=ConfigUniformRandomVelocity(
		KNOWLEDGE_SAMPLE_RATE_KEYWORD, YAW_KNOWLEDGE_BOUND_KEYWORD,
		YAW_KNOWLEDGE_MEAN_KEYWORD, YAW_KNOWLEDGE_RADIUS_KEYWORD,
			config_list);
		if (yaw==NULL) return(0);

	}
	else
	{
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


//-------------------------------------//
// Configuration Routines for Specific //
// Noise Distributions                 //
//-------------------------------------//

//----------------//
// ConfigGaussian //
//----------------//

Gaussian*
ConfigGaussian(const char* variance_keyword,
	const char* mean_keyword,
	ConfigList* config_list)
{
	double variance, mean;

	if(! config_list->GetDouble(variance_keyword,
	   &variance)) return(NULL);
        variance*=dtr*dtr;
	if(! config_list->GetDouble(mean_keyword,
	   &mean)) return(NULL);
	mean*=dtr;
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
	radius*=dtr;
	if(! config_list->GetDouble(mean_keyword,
	  &mean)) return(NULL);
        mean*=dtr;
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
        variance*=dtr*dtr;
	if(! config_list->GetDouble(mean_keyword,
	   &mean)) return(NULL);
	mean*=dtr;
	if(! config_list->GetDouble(bound_keyword,
	   &bound)) return(NULL);
        bound*=dtr;
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
	radius*=dtr;
	if(! config_list->GetDouble(mean_keyword,
	   &mean)) return(NULL);
	mean*=dtr;
	if(! config_list->GetDouble(bound_keyword, &bound))
		return(NULL);
	bound*=dtr;
	velocity=new Uniform((float)radius,0.0);
	RandomVelocity* new_rv = new RandomVelocity(velocity, (float)sample_rate,
		(float)bound, (float)mean);
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
	{
		printf("Could not find chirp rate in config file\n");
		return(0);
	}
	instrument->chirpRate = chirp_rate * KHZ_PER_MS_TO_HZ_PER_S;

	float chirp_start_m;	// kHz/ms
	if (! config_list->GetFloat(CHIRP_START_M_KEYWORD, &chirp_start_m))
	{
		printf("Could not find chirp start_m in config file\n");
		return(0);
	}
	instrument->chirpStartM = chirp_start_m * KHZ_PER_MS_TO_HZ_PER_S;

	float chirp_rate_b;		// kHz
	if (! config_list->GetFloat(CHIRP_START_B_KEYWORD, &chirp_rate_b))
	{
		printf("Could not find chirp start_b in config file\n");
		return(0);
	}
	instrument->chirpStartB = chirp_rate_b * KHZ_TO_HZ;

	float system_temperature;		// K
	if (! config_list->GetFloat(SYSTEM_TEMPERATURE_KEYWORD,&system_temperature))
	{
		printf("Could not find system temperature in config file\n");
		return(0);
	}
	instrument->systemTemperature = system_temperature;

	float base_transmit_freq;	// GHz
	if (! config_list->GetFloat(BASE_TRANSMIT_FREQUENCY_KEYWORD,
		&base_transmit_freq))
	{
		printf("Could not find base transmit freq in config file\n");
		return(0);
	}
	instrument->baseTransmitFreq = base_transmit_freq * GHZ_TO_HZ;

	float s_bw;
	if (! config_list->GetFloat(SCIENCE_SLICE_BANDWIDTH_KEYWORD, &s_bw))
	{
		printf("Could not find slice bandwidth in config file\n");
		return(0);
	}
	instrument->scienceSliceBandwidth = s_bw * KHZ_TO_HZ;

	int s_count;
	if (! config_list->GetInt(SCIENCE_SLICES_PER_SPOT_KEYWORD, &s_count))
	{
		printf("Could not find slices per spot in config file\n");
		return(0);
	}
	instrument->scienceSlicesPerSpot = s_count;

	float g_bw;
	if (! config_list->GetFloat(GUARD_SLICE_BANDWIDTH_KEYWORD, &g_bw))
	{
		printf("Could not find guard slice bandwidth in config file\n");
		return(0);
	}
	instrument->guardSliceBandwidth = g_bw * KHZ_TO_HZ;

	int g_count;
	if (! config_list->GetInt(GUARD_SLICES_PER_SIDE_KEYWORD, &g_count))
	{
		printf("Could not find guard slices per spot in config file\n");
		return(0);
	}
	instrument->guardSlicesPerSide = g_count;

	float noise_bandwidth;
	if (! config_list->GetFloat(NOISE_BANDWIDTH_KEYWORD, &noise_bandwidth))
	{
		printf("Could not find noise bandwidth in config file\n");
		return(0);
	}
	instrument->noiseBandwidth = noise_bandwidth * KHZ_TO_HZ;

	float transmit_power;
	/**** parameter in config file should be in Watts ***/
	if (! config_list->GetFloat(TRANSMIT_POWER_KEYWORD, &transmit_power))
	{
		printf("Could not find transmit power in config file\n");
		return(0);
	}
	instrument->transmitPower = transmit_power;

	float echo_receiver_gain;
	/**** parameter in config file should be in dB ***/
	if (! config_list->GetFloat(ECHO_RECEIVER_GAIN_KEYWORD,
		&echo_receiver_gain))
	{
		printf("Could not find echo receiver gain in config file\n");
		return(0);
	}
	echo_receiver_gain=(float)pow(10.0,0.1*echo_receiver_gain);
	instrument->echo_receiverGain = echo_receiver_gain;

	float noise_receiver_gain;
	/**** parameter in config file should be in dB ***/
	if (! config_list->GetFloat(NOISE_RECEIVER_GAIN_KEYWORD,
		&noise_receiver_gain))
	{
		printf("Could not find noise receiver gain in config file\n");
		return(0);
	}
	noise_receiver_gain=(float)pow(10.0,0.1*noise_receiver_gain);
	instrument->noise_receiverGain = noise_receiver_gain;

	float system_loss;
	/**** parameter in config file should be in dB ***/
	if (! config_list->GetFloat(SYSTEM_LOSS_KEYWORD, &system_loss))
	{
		printf("Could not find system loss in config file\n");
		return(0);
	}
	system_loss=(float)pow(10.0,0.1*system_loss);
	instrument->systemLoss = system_loss;

	int use_kpc;
	if (! config_list->GetInt(USE_KPC_KEYWORD, &use_kpc))
	{
		printf("Could not find use Kpc flag in config file\n");
		return(0);
	}
	instrument->useKpc = use_kpc;

	int use_kpm;
	if (! config_list->GetInt(USE_KPM_KEYWORD, &use_kpm))
	{
		printf("Could not find use Kpm flag in config file\n");
		return(0);
	}
	instrument->useKpm = use_kpm;

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
	// Configure the Antenna Simulator//
	//--------------------------------//

	if (! ConfigAntennaSim(&(instrument_sim->antennaSim), config_list))
		return(0);

	//-----------------------//
	// initialize PTGR noise //
	//-----------------------//

	float ptgr_var, ptgr_mean;
	if (! config_list->GetFloat(PTGR_NOISE_VARIANCE_KEYWORD, &ptgr_var))
	{
		printf("Could not find PtGr noise variance in config file\n");
		return(0);
	}
	if (! config_list->GetFloat(PTGR_NOISE_MEAN_KEYWORD, &ptgr_mean))
	{
		printf("Could not find PtGr noise mean in config file\n");
		return(0);
	}

	instrument_sim->ptgrNoise.SetVariance(ptgr_var);
	instrument_sim->ptgrNoise.SetMean(ptgr_mean);

	int uniform_sigma_field;

	config_list->WarnForMissingKeywords();
	if (! config_list->GetInt(UNIFORM_SIGMA_FIELD_KEYWORD,
		&uniform_sigma_field))
	{
		uniform_sigma_field=0;		// default value
	}
	instrument_sim->uniformSigmaField=uniform_sigma_field;

	int output_Pr_to_stdout;
	if (! config_list->GetInt(OUTPUT_PR_TO_STDOUT_KEYWORD,
		&output_Pr_to_stdout))
	{
		output_Pr_to_stdout=0; // default value
	}
	instrument_sim->outputPrToStdout=output_Pr_to_stdout;

	int use_kfactor;
	if (! config_list->GetInt(USE_KFACTOR_KEYWORD, &use_kfactor))
		use_kfactor=0; // default value
	instrument_sim->useKfactor=use_kfactor;


	int create_xtable;
	if (! config_list->GetInt(CREATE_XTABLE_KEYWORD, &create_xtable))
		create_xtable=0; // default value
	instrument_sim->createXtable=create_xtable;

	config_list->ExitForMissingKeywords();

	float system_temperature;
	if (! config_list->GetFloat(SYSTEM_TEMPERATURE_KEYWORD,&system_temperature))
	{
		printf("Could not find system temperature in config file\n");
		return(0);
	}

	/****** You cannot use and create the XTable simultaneously. ***/
	if(create_xtable && use_kfactor)
	{
		fprintf(stderr,
			"ConfigInstrumentSim: Cannot use kfactor AND create Xtable\n");
		return(0);
	}

	/*** To create an X table you NEED a uniform sigma0 field. ***/
	if(create_xtable && !uniform_sigma_field)
	{
		fprintf(stderr,
			"ConfigInstrumentSim: Cannot create an Xtable without a uniform sigma0 field\n");
		return(0);
	}

	/*** To create an X table SYSTEM_TEMPERATURE MUST be zero so that **/
        /*** Pn_slice will be zero and will NOT corrupt the X table      **/
	if(create_xtable && system_temperature!=0.0)
	{
		fprintf(stderr,
			"ConfigInstrumentSim: Cannot create an Xtable with a
nonzero system temperature! \n");
		return(0);
	}

	if(create_xtable)
	{
		if(!ConfigXTable(&(instrument_sim->xTable),config_list,"w"))
			return(0);
	}
	else if(use_kfactor)
	{
		if(!ConfigXTable(&(instrument_sim->kfactorTable),config_list,"r"))
			return(0);
	}

	return(1);
}

//-----------------------------//
// ConfigInstrumentSimAccurate //
//-----------------------------//

int
ConfigInstrumentSimAccurate(
	InstrumentSimAccurate*	instrument_sim,
	ConfigList*				config_list)
{
        if (! ConfigInstrumentSim(instrument_sim, config_list))
	        return(0);

	int num_look_steps;
	if (! config_list->GetInt(NUM_LOOK_STEPS_KEYWORD, &num_look_steps))
		return(0);
	instrument_sim->numLookStepsPerSlice=num_look_steps;

        float azimuth_integration_range;
	if (! config_list->GetFloat(AZIMUTH_INTEGRATION_RANGE_KEYWORD,
				  &azimuth_integration_range))
		return(0);
	instrument_sim->azimuthIntegrationRange=azimuth_integration_range*dtr;

        float azimuth_step_size;
	if (! config_list->GetFloat(AZIMUTH_STEP_SIZE_KEYWORD,
				  &azimuth_step_size))
		return(0);
	instrument_sim->azimuthStepSize=azimuth_step_size*dtr;


        return(1);
}

//------------------//
// ConfigAntennaSim //
//------------------//

int
ConfigAntennaSim(
	 AntennaSim* antenna_sim,
	 ConfigList* config_list)
{
	double start_time;
	if (! config_list->GetDouble(ANTENNA_START_TIME_KEYWORD, &start_time))
		return(0);
	antenna_sim->startTime = start_time;
	double start_azi;
	if (! config_list->GetDouble(ANTENNA_START_AZIMUTH_KEYWORD, &start_azi))
		return(0);
	antenna_sim->startAzimuth = start_azi*dtr;
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
	{
		printf("Could not find number of beams in config file\n");
		return(0);
	}
	antenna->numberOfBeams = number_of_beams;

	double pri_per_beam;
	if (! config_list->GetDouble(PRI_PER_BEAM_KEYWORD, &pri_per_beam))
	{
		printf("Could not find PRI per beam in config file\n");
		return(0);
	}
	antenna->priPerBeam = pri_per_beam;

	int encoder_bits;
	if (! config_list->GetInt(NUMBER_OF_ENCODER_BITS_KEYWORD, &encoder_bits))
	{
		printf("Could not find number of encoder bits in config file\n");
		return(0);
	}
	unsigned int values = 1 << encoder_bits;
	antenna->SetNumberOfEncoderValues(values);

	double roll, pitch, yaw;
	if (! config_list->GetDouble(ANTENNA_PEDESTAL_ROLL_KEYWORD, &roll))
	{
		printf("Could not find antenna pedestal roll in config file\n");
		return(0);
	}
	if (! config_list->GetDouble(ANTENNA_PEDESTAL_PITCH_KEYWORD, &pitch))
	{
		printf("Could not find antenna pedestal pitch in config file\n");
		return(0);
	}
	if (! config_list->GetDouble(ANTENNA_PEDESTAL_YAW_KEYWORD, &yaw))
	{
		printf("Could not find antenna pedestal yaw in config file\n");
		return(0);
	}
	Attitude att;
	att.Set(roll, pitch, yaw, 1, 2, 3);
	antenna->SetPedestalAttitude(&att);

	double spin_rate;
	if (! config_list->GetDouble(SPIN_RATE_KEYWORD, &spin_rate))
	{
		printf("Could not find spin rate in config file\n");
		return(0);
	}
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
	{
		printf("Missing keyword %s in config file\n",keyword);
		return(0);
	}
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
	{
		printf("Could not find beam pulse width in config file\n");
		return(0);
	}
	beam->pulseWidth = pulse_width * MS_TO_S;

	double gate_width;		// ms
	substitute_string(BEAM_x_RECEIVER_GATE_WIDTH_KEYWORD, "x", number, keyword);
	if (! config_list->GetDouble(keyword, &gate_width))
	{
		printf("Could not find beam receiver gate width in config file\n");
		return(0);
	}
	beam->rxGateWidth = gate_width * MS_TO_S;

	double look_angle;		// deg
	substitute_string(BEAM_x_LOOK_ANGLE_KEYWORD, "x", number, keyword);
	if (! config_list->GetDouble(keyword, &look_angle))
	{
		printf("Could not find beam look angle in config file\n");
		return(0);
	}
	look_angle *= dtr;

	double azimuth_angle;	// deg
	substitute_string(BEAM_x_AZIMUTH_ANGLE_KEYWORD, "x", number, keyword);
	if (! config_list->GetDouble(keyword, &azimuth_angle))
	{
		printf("Could not find beam azimuth angle in config file\n");
		return(0);
	}
	azimuth_angle *= dtr;

	substitute_string(BEAM_x_PATTERN_FILE_KEYWORD, "x", number, keyword);
	char* pattern_file = config_list->Get(keyword);
	if (pattern_file == NULL)
	{
		printf("Could not find beam pattern file in config file\n");
		return(0);
	}
	if (! beam->ReadBeamPattern(pattern_file))
	{
		printf("Error while reading beam %d pattern file\n",beam_number);
		return(0);
	}

	beam->SetElectricalBoresight(look_angle, azimuth_angle);

	// ms
	substitute_string(BEAM_x_TIME_OFFSET_KEYWORD, "x", number, keyword);
	if (! config_list->GetDouble(keyword, &tmp_double))
	{
		printf("Could not find beam time offset in config file\n");
		return(0);
	}
	beam->timeOffset = tmp_double * MS_TO_S;

	//----------------//
	// Range Tracking //
	//----------------//

	int use_rgc;
	if (! config_list->GetInt(USE_RGC_KEYWORD, &use_rgc))
	{
		printf("Could not find use RGC flag in config file\n");
		return(0);
	}
	beam->useRangeTracker = use_rgc;
	if (use_rgc)
	{
		substitute_string(BEAM_x_RGC_FILE_KEYWORD, "x", number, keyword);
		char* rgc_file = config_list->Get(keyword);
		if (rgc_file == NULL)
		{
			printf("Could not find RGC file name in config file\n");
			return(0);
		}

		if (! beam->rangeTracker.ReadBinary(rgc_file))
		{
			fprintf(stderr, "ConfigBeam: error reading RGC file %s\n",
				rgc_file);
			return(0);
		}
	}

	//------------------//
	// Doppler Tracking //
	//------------------//

	int use_dtc;
	if (! config_list->GetInt(USE_DTC_KEYWORD, &use_dtc))
	{
		printf("Could not find use DTC flag in config file\n");
		return(0);
	}
	beam->useDopplerTracker = use_dtc;
	if (use_dtc)
	{
		substitute_string(BEAM_x_DTC_FILE_KEYWORD, "x", number, keyword);
		char* dtc_file = config_list->Get(keyword);
		if (dtc_file == NULL)
		{
			printf("Could not find DTC file name in config file\n");
			return(0);
		}

		if (! beam->dopplerTracker.ReadBinary(dtc_file))
		{
			fprintf(stderr, "ConfigBeam: error reading DTC file %s\n",
				dtc_file);
			return(0);
		}
	}

	return(1);
}

//--------------//
// ConfigXTable //
//--------------//

int
ConfigXTable(
	     XTable*      xTable,
	     ConfigList*  config_list,
	     char* read_write)
{

  /**** Find out if XTable is to be configured READ or WRITE ***/
  int read=0;
  if(strcmp(read_write,"r") == 0)  read=1;
  else if(strcmp(read_write,"w")==0) read=0;
  else{
    fprintf(stderr, "ConfigXTable: Bad read_write parameter");
    return(0);
  }

  /**** Get XTable Filename  *****/

  char * xtable_filename= config_list->Get(XTABLE_FILENAME_KEYWORD);
  if (xtable_filename == NULL)
	{
	printf("Could not find XTable filename in config file\n");
    return(0);
	}
  xTable->SetFilename(xtable_filename);


  /**** Read header parameters for XTable object ****/

  int num_beams;
  if(! config_list->GetInt(NUMBER_OF_BEAMS_KEYWORD,&num_beams))
	{
	printf("Could not find number of beams in config file\n");
    return(0);
	}

  float pri_per_beam, antenna_spin_rate;
  if(! config_list->GetFloat(PRI_PER_BEAM_KEYWORD,&pri_per_beam))
	{
	printf("Could not find PRI per beam in config file\n");
    return(0);
	}
  if(! config_list->GetFloat(SPIN_RATE_KEYWORD,&antenna_spin_rate))
	{
	printf("Could not find spin rate in config file\n");
    return(0);
	}
  int num_azimuths=int((60.0/antenna_spin_rate)/pri_per_beam);

  int num_science_slices;
  if(! config_list->GetInt(SCIENCE_SLICES_PER_SPOT_KEYWORD,&num_science_slices))
	{
	printf("Could not find slices per spot in config file\n");
    return(0);
	}

  int num_guard_slices_each_side;
  if(! config_list->GetInt(GUARD_SLICES_PER_SIDE_KEYWORD,&num_guard_slices_each_side))
	{
	printf("Could not find guard slices per side in config file\n");
    return(0);
	}

  float science_slice_bandwidth;
  if(! config_list->GetFloat(SCIENCE_SLICE_BANDWIDTH_KEYWORD,&science_slice_bandwidth))
	{
	printf("Could not find slice bandwidth in config file\n");
    return(0);
	}
  science_slice_bandwidth*=KHZ_TO_HZ;

  float guard_slice_bandwidth;
  if(! config_list->GetFloat(GUARD_SLICE_BANDWIDTH_KEYWORD,&guard_slice_bandwidth))
	{
	printf("Could not find guard slice bandwidth in config file\n");
    return(0);
	}
  guard_slice_bandwidth*=KHZ_TO_HZ;


  /**** If mode is READ, read in xTable and make sure its parameters match
        those read from the config file                               *****/

  if(read){
    if(!xTable->Read())
	{
		printf("Error reading xTable in ConfigXTable\n");
		return(0);
	}
    if(!xTable->CheckHeader(num_beams, num_science_slices,
			   num_guard_slices_each_side, science_slice_bandwidth,
			   guard_slice_bandwidth))
	{
		printf("Header check failed in ConfigXTable\n");
    	return(0);
	}
  }

  /***** If mode is WRITE, asign xTable parameters from parameters read from
         config file, and allocate it the arrays                         ****/

  else{
    xTable->numBeams=num_beams;
    xTable->numAzimuthBins=num_azimuths;
    xTable->numScienceSlices=num_science_slices;
    xTable->numGuardSlicesEachSide=num_guard_slices_each_side;
    xTable->scienceSliceBandwidth=science_slice_bandwidth;
    xTable->guardSliceBandwidth=guard_slice_bandwidth;
    xTable->numSlices=xTable->numScienceSlices+2*xTable->numGuardSlicesEachSide;
    if (!xTable->Allocate()){
      fprintf(stderr,"Error allocating Xtable object/n");
      return(0);
    }

  }

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

	int s_count;
	if (! config_list->GetInt(SCIENCE_SLICES_PER_SPOT_KEYWORD, &s_count))
		return(0);

	int g_count;
	if (! config_list->GetInt(GUARD_SLICES_PER_SIDE_KEYWORD, &g_count))
		return(0);

	int total_slices = s_count + 2 * g_count;

	if (! l00->AllocateBuffer(number_of_beams, antenna_cycles_per_frame,
		total_slices))
	{
		return(0);
	}

	//-------------------------//
	// configure the l00 frame //
	//-------------------------//

	if (! l00->frame.Allocate(number_of_beams, antenna_cycles_per_frame,
		total_slices))
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

	int s_count;
	if (! config_list->GetInt(SCIENCE_SLICES_PER_SPOT_KEYWORD, &s_count))
		return(0);

	int g_count;
	if (! config_list->GetInt(GUARD_SLICES_PER_SIDE_KEYWORD, &g_count))
		return(0);

	int total_slices = s_count + 2 * g_count;

	if (! l10->AllocateBuffer(number_of_beams, antenna_cycles_per_frame,
		total_slices))
	{
		return(0);
	}

	//-------------------------//
	// configure the l10 frame //
	//-------------------------//

	if (! l10->frame.Allocate(number_of_beams, antenna_cycles_per_frame,
		total_slices))
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

//----------------//
// ConfigL10ToL15 //
//----------------//

int
ConfigL10ToL15(
	L10ToL15*			l10tol15,
	ConfigList*		config_list)
{
	config_list->WarnForMissingKeywords();
	int output_sigma0_to_stdout;
	if (! config_list->GetInt(OUTPUT_SIGMA0_TO_STDOUT_KEYWORD,
		&output_sigma0_to_stdout))
	{
		output_sigma0_to_stdout=0; // default value
	}
	l10tol15->outputSigma0ToStdout=output_sigma0_to_stdout;

	int use_kfactor;
	if (! config_list->GetInt(USE_KFACTOR_KEYWORD, &use_kfactor))
		use_kfactor=0; // default value
	l10tol15->useKfactor=use_kfactor;

	if(use_kfactor)
	{
		if(!ConfigXTable(&(l10tol15->kfactorTable),config_list,"r"))
			return(0);
	}

	config_list->ExitForMissingKeywords();

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

	ephemeris->SetMaxNodes(200);		// this should be calculated

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

	char* windfield_type = config_list->Get(WINDFIELD_TYPE_KEYWORD);
	if (windfield_type == NULL)
		return(0);

	char* windfield_filename = config_list->Get(WINDFIELD_FILE_KEYWORD);
	if (windfield_filename == NULL)
		return(0);

	if (! windfield->ReadType(windfield_filename, windfield_type))
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
	// configure l15 //
	//---------------//

	if (! ConfigL15(&(grid->l15), config_list))
		return(0);

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

	grid->Allocate(ct_res, at_res, 2000.0, 3000.0);

	return(1);
}

//---------------//
// ConfigControl //
//---------------//

#define EPHEMERIS_BUFFER		300			// 5 minutes

int
ConfigControl(
	SpacecraftSim*	spacecraft_sim,
	ConfigList*		config_list,
	double*			grid_start_time,
	double*			grid_end_time,
	double*			instrument_start_time,
	double*			instrument_end_time,
	double*			spacecraft_start_time,
	double*			spacecraft_end_time)
{
	config_list->WarnForMissingKeywords();
	double orbit_period = spacecraft_sim->GetPeriod();

	//-----------------//
	// grid start time //
	//-----------------//

	double time_in_rev;
	if (config_list->GetDouble(TIME_IN_REV_KEYWORD, &time_in_rev))
	{
		*grid_start_time = spacecraft_sim->FindPrevArgOfLatTime(time_in_rev,
			SOUTH_ARG_OF_LAT, EQX_TIME_TOLERANCE);
	}
	else
	{
		fprintf(stderr, "ConfigControl: can't determine grid start time\n");
		exit(1);
	}

	//---------------//
	// grid end time //
	//---------------//

	double grid_lat_range, grid_time_range;
	if (config_list->GetDouble(GRID_LATITUDE_RANGE_KEYWORD, &grid_lat_range))
	{
		grid_time_range = orbit_period * grid_lat_range / 360.0;
		*grid_end_time = *grid_start_time + grid_time_range;
	}
	else if (config_list->GetDouble(GRID_TIME_RANGE_KEYWORD, &grid_time_range))
	{
		*grid_end_time = *grid_start_time + grid_time_range;
	}
	else
	{
        fprintf(stderr, "ConfigControl: can't determine grid end time\n");
        exit(1);
	}

	//-----------------------//
	// instrument start time //
	//-----------------------//

	double ins_buf;
	if (config_list->GetDouble(INSTRUMENT_START_TIME_KEYWORD,
		instrument_start_time))
	{
		// nothing to do -- woo hoo!
	}
	else if (config_list->GetDouble(INSTRUMENT_TIME_BUFFER_KEYWORD, &ins_buf))
	{
		*instrument_start_time = *grid_start_time - ins_buf;
	}
	else
	{
		fprintf(stderr,
			"ConfigControl: can't determine instrument start time\n");
		exit(1);
	}

	//---------------------//
	// instrument end time //
	//---------------------//

	if (config_list->GetDouble(INSTRUMENT_END_TIME_KEYWORD,
		instrument_end_time))
	{
		// nothing to do -- woo hoo again!
	}
	else if (config_list->GetDouble(INSTRUMENT_TIME_BUFFER_KEYWORD, &ins_buf))
	{
		*instrument_end_time = *grid_end_time + ins_buf;
	}
	else
	{
		fprintf(stderr,
			"ConfigControl: can't determine instrument end time\n");
		exit(1);
	}

	//-----------------------//
	// spacecraft start time //
	//-----------------------//

	if (config_list->GetDouble(SPACECRAFT_START_TIME_KEYWORD,
		spacecraft_start_time))
	{
		// nothing to do -- woo hoo once more!
	}
	else
	{
		double ephemeris_period = spacecraft_sim->GetEphemerisPeriod();
		*spacecraft_start_time = *grid_start_time -
			ephemeris_period * (EPHEMERIS_INTERP_ORDER + 2) - EPHEMERIS_BUFFER;
	}

	//---------------------//
	// spacecraft end time //
	//---------------------//

	if (config_list->GetDouble(SPACECRAFT_END_TIME_KEYWORD,
		spacecraft_end_time))
	{
		// nothing to do -- woo hoo once more!
	}
	else
	{
		double last_time = MAX(*instrument_end_time, *grid_end_time);
		double ephemeris_period = spacecraft_sim->GetEphemerisPeriod();
		*spacecraft_end_time = last_time +
			ephemeris_period * (EPHEMERIS_INTERP_ORDER + 2) + EPHEMERIS_BUFFER;
	}

	config_list->ExitForMissingKeywords();

	return(1);
}
