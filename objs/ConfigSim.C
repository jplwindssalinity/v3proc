//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

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
#include "L1A.h"
#include "L1AToL1B.h"
#include "L1B.h"
#include "L2A.h"
#include "L2B.h"
#include "L2AToL2B.h"
#include "Constants.h"
#include "Distributions.h"
#include "Tracking.h"
#include "Kpm.h"
#include "EarthField.h"
#include "Kp.h"

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

	int sim_kprs_flag;
	if (! config_list->GetInt(SIM_KPRS_FLAG_KEYWORD, &sim_kprs_flag))
		return(0);
	spacecraft_sim->simKprsFlag = sim_kprs_flag;

	//-------------------------------//
	// configure the attitude models //
	//-------------------------------//

	if (! ConfigAttitudeControlModel(&(spacecraft_sim->attCntlDist),
			config_list))
	{
		return(0);
	}

	if (! ConfigAttitudeKnowledgeModel(&(spacecraft_sim->attKnowDist),
			config_list))
	{
		return(0);
	}
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
ConfigAttitudeControlModel(AttDist* attcntl,
	ConfigList* config_list)
{

	char* string;

	string=config_list->Get(ATTITUDE_CONTROL_MODEL_KEYWORD);
	if (! string)
		return(0);

	if (strcmp(string,"NONE")==0 || strcmp(string,"None")==0
		|| strcmp(string,"none")==0)
	{
		// By default mean, variance, and correlation length
		// are set to zero
		return(1);
	}

	else if (strcmp(string,"Time_Correlated_Gaussian")==0 ||
		strcmp(string,"TIME_CORRELATED_GAUSSIAN")==0
		|| strcmp(string,"time_correlated_gaussian")==0)
	{

		float std, mean, corrlength;

		if (! config_list->GetFloat(ROLL_CONTROL_STD_KEYWORD, &std))
			return(0);
		if (! config_list->GetFloat(ROLL_CONTROL_MEAN_KEYWORD, &mean))
			return(0);
		if (! config_list->GetFloat(ROLL_CONTROL_CORRLENGTH_KEYWORD,
				&corrlength))
		{
			return(0);
		}

		attcntl->roll.SetVariance(std*std*dtr*dtr);
		attcntl->roll.SetMean(mean*dtr);
		attcntl->roll.SetCorrelationLength(corrlength);
		attcntl->roll.SetSeed(ROLL_CONTROL_SEED);
		attcntl->roll.Initialize();

		if (! config_list->GetFloat(PITCH_CONTROL_STD_KEYWORD, &std))
			return(0);
		if (! config_list->GetFloat(PITCH_CONTROL_MEAN_KEYWORD, &mean))
			return(0);
		if (! config_list->GetFloat(PITCH_CONTROL_CORRLENGTH_KEYWORD,
				&corrlength))
		{
			return(0);
		}

		attcntl->pitch.SetVariance(std*std*dtr*dtr);
		attcntl->pitch.SetMean(mean*dtr);
		attcntl->pitch.SetCorrelationLength(corrlength);
		attcntl->pitch.SetSeed(PITCH_CONTROL_SEED);
		attcntl->pitch.Initialize();

		if (! config_list->GetFloat(YAW_CONTROL_STD_KEYWORD, &std))
			return(0);
		if (! config_list->GetFloat(YAW_CONTROL_MEAN_KEYWORD, &mean))
			return(0);
		if (! config_list->GetFloat(YAW_CONTROL_CORRLENGTH_KEYWORD,
			&corrlength))
		{
			return(0);
		}

		attcntl->yaw.SetVariance(std*std*dtr*dtr);
		attcntl->yaw.SetMean(mean*dtr);
		attcntl->yaw.SetCorrelationLength(corrlength);
		attcntl->yaw.SetSeed(YAW_CONTROL_SEED);
		attcntl->yaw.Initialize();
	}
	else
	{
		fprintf(stderr,"No such Attitude Control Model. \n");
		fprintf(stderr,"Implemented models are:");
		fprintf(stderr,"TIME_CORRELATED_GAUSSIAN and NONE. \n");
		return(0);
	}


	return(1);
}

//------------------------------//
// ConfigAttitudeKnowledgeModel //
//------------------------------//

int
ConfigAttitudeKnowledgeModel(AttDist* attknow,
	ConfigList* config_list)
{

	char* string;

	string=config_list->Get(ATTITUDE_KNOWLEDGE_MODEL_KEYWORD);
	if (! string)
		return(0);

	if (strcmp(string,"NONE")==0 || strcmp(string,"None")==0
		|| strcmp(string,"none")==0)
	{
		// By default mean, variance, and correlation length
		// are set to zero
		return(1);
	}

	else if (strcmp(string,"Time_Correlated_Gaussian")==0 ||
		strcmp(string,"TIME_CORRELATED_GAUSSIAN")==0
		|| strcmp(string,"time_correlated_gaussian")==0)
	{

		float std, mean, corrlength;

		if (! config_list->GetFloat(ROLL_KNOWLEDGE_STD_KEYWORD, &std))
			return(0);
		if (! config_list->GetFloat(ROLL_KNOWLEDGE_MEAN_KEYWORD, &mean))
			return(0);
		if (! config_list->GetFloat(ROLL_KNOWLEDGE_CORRLENGTH_KEYWORD,
			&corrlength))
		{
			return(0);
		}

		attknow->roll.SetVariance(std*std*dtr*dtr);
		attknow->roll.SetMean(mean*dtr);
		attknow->roll.SetCorrelationLength(corrlength);
		attknow->roll.SetSeed(ROLL_KNOWLEDGE_SEED);
		attknow->roll.Initialize();

		if (! config_list->GetFloat(PITCH_KNOWLEDGE_STD_KEYWORD, &std))
			return(0);
		if (! config_list->GetFloat(PITCH_KNOWLEDGE_MEAN_KEYWORD, &mean))
			return(0);
		if (! config_list->GetFloat(PITCH_KNOWLEDGE_CORRLENGTH_KEYWORD,
				&corrlength))
		{
			return(0);
		}

		attknow->pitch.SetVariance(std*std*dtr*dtr);
		attknow->pitch.SetMean(mean*dtr);
		attknow->pitch.SetCorrelationLength(corrlength);
		attknow->pitch.SetSeed(PITCH_KNOWLEDGE_SEED);
		attknow->pitch.Initialize();

		if (! config_list->GetFloat(YAW_KNOWLEDGE_STD_KEYWORD, &std))
			return(0);
		if (! config_list->GetFloat(YAW_KNOWLEDGE_MEAN_KEYWORD, &mean))
			return(0);
		if (! config_list->GetFloat(YAW_KNOWLEDGE_CORRLENGTH_KEYWORD,
				&corrlength))
		{
			return(0);
		}

		attknow->yaw.SetVariance(std*std*dtr*dtr);
		attknow->yaw.SetMean(mean*dtr);
		attknow->yaw.SetCorrelationLength(corrlength);
		attknow->yaw.SetSeed(YAW_KNOWLEDGE_SEED);
		attknow->yaw.Initialize();
	}
	else
	{
		fprintf(stderr,"No such Attitude Control Model. \n");
		fprintf(stderr,"Implemented models are:");
		fprintf(stderr,"TIME_CORRELATED_GAUSSIAN and NONE. \n");
		return(0);
	}


	return(1);
}

/****************************************
//--------------------------------------//
// Configuration Routines for Specific	//
// Noise Distributions					//
//--------------------------------------//

//----------------//
// ConfigGaussian //
//----------------//

Gaussian*
ConfigGaussian(const char* variance_keyword,
	const char* mean_keyword,
	ConfigList* config_list)
{
	double variance, mean;

	if (! config_list->GetDouble(variance_keyword, &variance))
		return(NULL);
	variance*=dtr*dtr;
	if (! config_list->GetDouble(mean_keyword, &mean))
		return(NULL);
	mean*=dtr;
	Gaussian* new_g = new Gaussian((float)variance,(float)mean);
	return(new_g);
}

	//---------------//
	// ConfigUniform //
	//---------------//

Uniform*
ConfigUniform(
	const char*		radius_keyword,
	const char*		mean_keyword,
	ConfigList*		config_list)
{
	double radius, mean;

	if (! config_list->GetDouble(radius_keyword, &radius))
		return(NULL);
	radius*=dtr;
	if (! config_list->GetDouble(mean_keyword, &mean))
		return(NULL);
	mean*=dtr;
	Uniform* new_u = new Uniform((float)radius,float(mean));
	return(new_u);
}

	//------------------------------//
	// ConfigGaussianRandomVelocity //
	//------------------------------//

RandomVelocity*
ConfigGaussianRandomVelocity(
	const char*		samprate_keyword,
	const char*		bound_keyword,
	const char*		mean_keyword,
	const char*		variance_keyword,
	ConfigList*		config_list)
{
	double variance, mean, sample_rate, bound;
	GenericTimelessDist *velocity;

	if (! config_list->GetDouble(samprate_keyword, &sample_rate))
		return(NULL);

	if (! config_list->GetDouble(variance_keyword, &variance))
		return(NULL);
	variance*=dtr*dtr;
	if (! config_list->GetDouble(mean_keyword, &mean))
		return(NULL);
	mean*=dtr;
	if (! config_list->GetDouble(bound_keyword, &bound))
		return(NULL);
	bound*=dtr;
	velocity = new Gaussian((float)variance,0.0);
	RandomVelocity* new_rv = new RandomVelocity(velocity, (float)sample_rate, (float)bound, (float)mean);
	return(new_rv);
}


	//-----------------------------//
	// ConfigUniformRandomVelocity //
	//-----------------------------//

RandomVelocity*
ConfigUniformRandomVelocity(
	const char*		samprate_keyword,
	const char*		bound_keyword,
	const char*		mean_keyword,
	const char*		radius_keyword,
	ConfigList*		config_list)
{
	double radius, mean, sample_rate, bound;
	GenericTimelessDist *velocity;

	if (! config_list->GetDouble(samprate_keyword, &sample_rate))
		return(NULL);

	if (! config_list->GetDouble(radius_keyword, &radius))
		return(NULL);
	radius*=dtr;
	if (! config_list->GetDouble(mean_keyword, &mean))
		return(NULL);
	mean*=dtr;
	if (! config_list->GetDouble(bound_keyword, &bound))
		return(NULL);
	bound*=dtr;
	velocity=new Uniform((float)radius,0.0);
	RandomVelocity* new_rv = new RandomVelocity(velocity, (float)sample_rate,
		(float)bound, (float)mean);
	return(new_rv);
}
********************************************/

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

	float system_temperature;		// K
	if (! config_list->GetFloat(SYSTEM_TEMPERATURE_KEYWORD,&system_temperature))
		return(0);
	instrument->systemTemperature = system_temperature;

	float base_transmit_freq;	// GHz
	if (! config_list->GetFloat(BASE_TRANSMIT_FREQUENCY_KEYWORD,
		&base_transmit_freq))
	{
		return(0);
	}
	instrument->baseTransmitFreq = base_transmit_freq * GHZ_TO_HZ;

	float s_bw;
	if (! config_list->GetFloat(SCIENCE_SLICE_BANDWIDTH_KEYWORD, &s_bw))
		return(0);
	instrument->scienceSliceBandwidth = s_bw * KHZ_TO_HZ;

	int s_count;
	if (! config_list->GetInt(SCIENCE_SLICES_PER_SPOT_KEYWORD, &s_count))
		return(0);
	instrument->scienceSlicesPerSpot = s_count;

	float g_bw;
	if (! config_list->GetFloat(GUARD_SLICE_BANDWIDTH_KEYWORD, &g_bw))
		return(0);
	instrument->guardSliceBandwidth = g_bw * KHZ_TO_HZ;

	int g_count;
	if (! config_list->GetInt(GUARD_SLICES_PER_SIDE_KEYWORD, &g_count))
		return(0);
	instrument->guardSlicesPerSide = g_count;

	float noise_bandwidth;
	if (! config_list->GetFloat(NOISE_BANDWIDTH_KEYWORD, &noise_bandwidth))
		return(0);
	instrument->noiseBandwidth = noise_bandwidth * KHZ_TO_HZ;

	float transmit_power;
	/**** parameter in config file should be in Watts ***/
	if (! config_list->GetFloat(TRANSMIT_POWER_KEYWORD, &transmit_power))
		return(0);
	instrument->transmitPower = transmit_power;

	float echo_receiver_gain;
	/**** parameter in config file should be in dB ***/
	if (! config_list->GetFloat(ECHO_RECEIVER_GAIN_KEYWORD,
		&echo_receiver_gain))
	{
		return(0);
	}
	echo_receiver_gain=(float)pow(10.0,0.1*echo_receiver_gain);
	instrument->echo_receiverGain = echo_receiver_gain;

	float noise_receiver_gain;
	/**** parameter in config file should be in dB ***/
	if (! config_list->GetFloat(NOISE_RECEIVER_GAIN_KEYWORD,
		&noise_receiver_gain))
	{
		return(0);
	}
	noise_receiver_gain=(float)pow(10.0,0.1*noise_receiver_gain);
	instrument->noise_receiverGain = noise_receiver_gain;

	float system_loss;
	/**** parameter in config file should be in dB ***/
	if (! config_list->GetFloat(SYSTEM_LOSS_KEYWORD, &system_loss))
		return(0);
	system_loss=(float)pow(10.0,0.1*system_loss);
	instrument->systemLoss = system_loss;

	//--------------//
	// orbit period //
	//--------------//

	unsigned int orbit_ticks;
	if (! config_list->GetUnsignedInt(ORBIT_TICKS_PER_ORBIT_KEYWORD,
		&orbit_ticks))
	{
		return(0);
	}
	instrument->orbitTicksPerOrbit = orbit_ticks;

	//-------//
	// FLAGS //
	//-------//

	int sim_kpc_flag;
	if (! config_list->GetInt(SIM_KPC_FLAG_KEYWORD, &sim_kpc_flag))
		return(0);
	instrument->simKpcFlag = sim_kpc_flag;

	int sim_corr_kpm_flag;
	if (! config_list->GetInt(SIM_CORR_KPM_FLAG_KEYWORD, &sim_corr_kpm_flag))
		return(0);
	instrument->simCorrKpmFlag = sim_corr_kpm_flag;

	int sim_uncorr_kpm_flag;
	if (! config_list->GetInt(SIM_UNCORR_KPM_FLAG_KEYWORD,&sim_uncorr_kpm_flag))
		return(0);
	instrument->simUncorrKpmFlag = sim_uncorr_kpm_flag;

	int sim_kpri_flag;
	if (! config_list->GetInt(SIM_KPRI_FLAG_KEYWORD, &sim_kpri_flag))
		return(0);
	instrument->simKpriFlag = sim_kpri_flag;

	double corr_kpmdB;
	if (! config_list->GetDouble(CORR_KPM_KEYWORD, &corr_kpmdB))
		return(0);
	// convert to real units and un-normalize
	instrument->corrKpm = pow(10.0,0.1*corr_kpmdB) - 1.0;

	// don't waste time generating zero variance rv's
	if (corr_kpmdB == 0.0) instrument->simCorrKpmFlag = 0;

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

	float kp_ptgr, ptgr_bias, ptgr_corrlength;
	if (! config_list->GetFloat(PTGR_NOISE_KP_KEYWORD, &kp_ptgr))
	{
		fprintf(stderr,"Could not find PtGr noise variance in config file\n");
		return(0);
	}
	if (! config_list->GetFloat(PTGR_NOISE_BIAS_KEYWORD, &ptgr_bias))
	{
		fprintf(stderr,"Could not find PtGr noise mean in config file\n");
		return(0);
	}

	if (! config_list->GetFloat(PTGR_NOISE_CORRLENGTH_KEYWORD,
		&ptgr_corrlength))
	{
		fprintf(stderr,
			"Could not find PtGr noise correlation length in config file\n");
		return(0);
	}
	kp_ptgr=pow(10,0.1*kp_ptgr)-1.0;
	instrument_sim->ptgrNoise.SetVariance(kp_ptgr*kp_ptgr);
	ptgr_bias=pow(10,0.1*ptgr_bias)-1.0;
	instrument_sim->ptgrNoise.SetMean(ptgr_bias);
	instrument_sim->ptgrNoise.SetCorrelationLength(ptgr_corrlength);
	instrument_sim->ptgrNoise.SetSeed(PTGR_SEED);
	instrument_sim->ptgrNoise.Initialize();

	int uniform_sigma_field;

	config_list->WarnForMissingKeywords();
	if (! config_list->GetInt(UNIFORM_SIGMA_FIELD_KEYWORD,
		&uniform_sigma_field))
	{
		uniform_sigma_field=0;		// default value
	}
	instrument_sim->uniformSigmaField=uniform_sigma_field;

	int output_X_to_stdout;
	if (! config_list->GetInt(OUTPUT_X_TO_STDOUT_KEYWORD,
		&output_X_to_stdout))
	{
		output_X_to_stdout=0; // default value
	}
	instrument_sim->outputXToStdout=output_X_to_stdout;

	int use_kfactor;
	if (! config_list->GetInt(USE_KFACTOR_KEYWORD, &use_kfactor))
		use_kfactor=0; // default value
	instrument_sim->useKfactor=use_kfactor;

	int create_xtable;
	if (! config_list->GetInt(CREATE_XTABLE_KEYWORD, &create_xtable))
		create_xtable=0; // default value
	instrument_sim->createXtable=create_xtable;

	int compute_xfactor;
	if (! config_list->GetInt(COMPUTE_XFACTOR_KEYWORD, &compute_xfactor))
		compute_xfactor=0; // default value
	instrument_sim->computeXfactor=compute_xfactor;

	int range_gate_clipping;
	if (! config_list->GetInt(RANGE_GATE_CLIPPING_KEYWORD, &range_gate_clipping))
		range_gate_clipping=0; // default value
	instrument_sim->rangeGateClipping=range_gate_clipping;

	int apply_doppler_error;
	if (! config_list->GetInt(APPLY_DOPPLER_ERROR_KEYWORD, &apply_doppler_error))
		apply_doppler_error=0; // default value
	instrument_sim->applyDopplerError=apply_doppler_error;

	config_list->DoNothingForMissingKeywords();
	instrument_sim->simVs1BCheckfile =
		config_list->Get(SIM_CHECKFILE_KEYWORD);

	config_list->ExitForMissingKeywords();

	float system_temperature;
	if (! config_list->GetFloat(SYSTEM_TEMPERATURE_KEYWORD,&system_temperature))
	{
		fprintf(stderr,"Could not find system temperature in config file\n");
		return(0);
	}

	/****** You cannot use and create the XTable simultaneously. ***/
	if (create_xtable && use_kfactor)
	{
		fprintf(stderr,
			"ConfigInstrumentSim: Cannot use kfactor AND create Xtable\n");
		return(0);
	}

	/*** To create an X table you NEED a uniform sigma0 field. ***/
	if (create_xtable && !uniform_sigma_field)
	{
		fprintf(stderr,
			"ConfigInstrumentSim: Cannot create an Xtable without a uniform sigma0 field\n");
		return(0);
	}

	/*** To create an X table SYSTEM_TEMPERATURE MUST be zero so that **/
	/*** Pn_slice will be zero and will NOT corrupt the X table **/
	if (create_xtable && system_temperature!=0.0)
	{
		fprintf(stderr,
			"ConfigInstrumentSim: Cannot create an Xtable with a nonzero system temperature! \n");
		return(0);
	}

	if (create_xtable)
	{
		if (!ConfigXTable(&(instrument_sim->xTable),config_list,"w"))
			return(0);
	}
	else if (use_kfactor)
	{
		if (!ConfigXTable(&(instrument_sim->kfactorTable),config_list,"r"))
			return(0);
	}
        
	if (compute_xfactor){
	  int num_look_steps;
	  if (! config_list->GetInt(NUM_LOOK_STEPS_KEYWORD, &num_look_steps))
		return(0);
	  instrument_sim->numLookStepsPerSlice=num_look_steps;
	  
	  float azimuth_integration_range;
	  if (! config_list->GetFloat(AZIMUTH_INTEGRATION_RANGE_KEYWORD,
				      &azimuth_integration_range))
	    {
	      return(0);
	    }
	  instrument_sim->azimuthIntegrationRange=azimuth_integration_range*dtr;

	  float azimuth_step_size;
	  if (! config_list->GetFloat(AZIMUTH_STEP_SIZE_KEYWORD,
				      &azimuth_step_size))
	    {
	      return(0);
	    }
	  instrument_sim->azimuthStepSize=azimuth_step_size*dtr;
	}

        if(apply_doppler_error){
          float doppler_bias;
	  if (! config_list->GetFloat(DOPPLER_BIAS_KEYWORD,
				      &doppler_bias))
	    {
	      return(0);
	    }
	  instrument_sim->dopplerBias=doppler_bias*KHZ_TO_HZ;
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
	{
		return(0);
	}
	instrument_sim->azimuthIntegrationRange=azimuth_integration_range*dtr;

	float azimuth_step_size;
	if (! config_list->GetFloat(AZIMUTH_STEP_SIZE_KEYWORD,
			&azimuth_step_size))
	{
		return(0);
	}
	instrument_sim->azimuthStepSize=azimuth_step_size*dtr;


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
	double tmp_double;

	//-----------------------//
	// configure the antenna //
	//-----------------------//

	int number_of_beams;
	if (! config_list->GetInt(NUMBER_OF_BEAMS_KEYWORD, &number_of_beams))
	{
		fprintf(stderr,"Could not find number of beams in config file\n");
		return(0);
	}
	antenna->numberOfBeams = number_of_beams;

	if (! config_list->GetDouble(PRI_PER_BEAM_KEYWORD, &tmp_double))
	{
		fprintf(stderr,"Could not find PRI per beam in config file\n");
		return(0);
	}
	antenna->priPerBeam = (float)number_of_beams *
		quantize(tmp_double / (float)number_of_beams, PRF_CLOCK_RESOLUTION);

	int tmp_int;
	if (! config_list->GetInt(NUMBER_OF_ENCODER_BITS_KEYWORD, &tmp_int))
		return(0);

	unsigned int values = 1 << tmp_int;
	antenna->SetNumberOfEncoderValues(values);

	double roll, pitch, yaw;
	if (! config_list->GetDouble(ANTENNA_PEDESTAL_ROLL_KEYWORD, &roll))
	{
		fprintf(stderr,"Could not find antenna pedestal roll in config file\n");
		return(0);
	}
	if (! config_list->GetDouble(ANTENNA_PEDESTAL_PITCH_KEYWORD, &pitch))
	{
		fprintf(stderr,"Could not find antenna pedestal pitch in config file\n");
		return(0);
	}
	if (! config_list->GetDouble(ANTENNA_PEDESTAL_YAW_KEYWORD, &yaw))
	{
		fprintf(stderr,"Could not find antenna pedestal yaw in config file\n");
		return(0);
	}
        
	//---------------------------------//
        // Convert from degrees to radians //
	//---------------------------------//
        roll*=dtr;
        pitch*=dtr;
        yaw*=dtr;
	Attitude att;
	att.Set(roll, pitch, yaw, 1, 2, 3);
	antenna->SetPedestalAttitude(&att);

	//---------//
	// encoder //
	//---------//

	if (! config_list->GetInt(ENCODER_A_OFFSET_KEYWORD, &tmp_int))
		return(0);
	antenna->encoderAOffsetDn = tmp_int;

	if (! config_list->GetDouble(ENCODER_DELAY_KEYWORD, &tmp_double))
		return(0);
	antenna->encoderDelay = tmp_double;

	//------------//
	// spin rates //
	//------------//

	if (! config_list->GetDouble(COMMANDED_SPIN_RATE_KEYWORD, &tmp_double))
		return(0);
	antenna->commandedSpinRate = tmp_double * rpm_to_radps;

	if (! config_list->GetDouble(ACTUAL_SPIN_RATE_KEYWORD, &tmp_double))
		return(0);
	antenna->actualSpinRate = tmp_double * rpm_to_radps;

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
		fprintf(stderr,"Missing keyword %s in config file\n",keyword);
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
		fprintf(stderr,"Beam %d polarization (%c) not recognized\n",
			beam_number,tmp_char);
		return(0);
	}

	double pulse_width;		// ms
	substitute_string(BEAM_x_PULSE_WIDTH_KEYWORD, "x", number, keyword);
	if (! config_list->GetDouble(keyword, &pulse_width))
	{
		fprintf(stderr,"Could not find beam pulse width in config file\n");
		return(0);
	}
	beam->txPulseWidth = quantize(pulse_width * MS_TO_S,
		TX_PULSE_WIDTH_RESOLUTION);

	double gate_width;		// ms
	substitute_string(BEAM_x_RECEIVER_GATE_WIDTH_KEYWORD, "x", number, keyword);
	if (! config_list->GetDouble(keyword, &gate_width))
	{
		fprintf(stderr,"Could not find beam receiver gate width in config file\n");
		return(0);
	}
	beam->rxGateWidth = quantize(gate_width * MS_TO_S,
		RX_GATE_WIDTH_RESOLUTION);

	substitute_string(BEAM_x_PATTERN_FILE_KEYWORD, "x", number, keyword);
	char* pattern_file = config_list->Get(keyword);
	if (pattern_file == NULL)
	{
		fprintf(stderr,"Could not find beam pattern file in config file\n");
		return(0);
	}
	if (! beam->ReadBeamPattern(pattern_file))
	{
		fprintf(stderr,"Error while reading beam %d pattern file\n",beam_number);
		return(0);
	}

	//-----------------------------------------------------------------//
	// Setup one mechanical boresight, or two electrical boresights.
	//-----------------------------------------------------------------//

	config_list->DoNothingForMissingKeywords();

	double look_angle;		// deg
	double azimuth_angle;	// deg

	if (config_list->GetDouble(MECH_LOOK_ANGLE_KEYWORD, &look_angle))
	{
		look_angle *= dtr;
		if (config_list->GetDouble(MECH_AZIMUTH_ANGLE_KEYWORD, &azimuth_angle))
		{
			azimuth_angle *= dtr;
			beam->SetMechanicalBoresight(look_angle, azimuth_angle);
		}
		else
		{
			fprintf(stderr,"Missing mechanical boresight azimuth in config file\n");
			return(0);
		}
	}
	else
	{
		substitute_string(BEAM_x_LOOK_ANGLE_KEYWORD, "x", number, keyword);
		if (! config_list->GetDouble(keyword, &look_angle))
		{
			fprintf(stderr,"Could not find beam look angle in config file\n");
			return(0);
		}
		look_angle *= dtr;

		substitute_string(BEAM_x_AZIMUTH_ANGLE_KEYWORD, "x", number, keyword);
		if (! config_list->GetDouble(keyword, &azimuth_angle))
		{
			fprintf(stderr,"Could not find beam azimuth angle in config file\n");
			return(0);
		}
		azimuth_angle *= dtr;

		beam->SetElectricalBoresight(look_angle, azimuth_angle);
	}

	config_list->ExitForMissingKeywords();

	// ms
	substitute_string(BEAM_x_TIME_OFFSET_KEYWORD, "x", number, keyword);
	if (! config_list->GetDouble(keyword, &tmp_double))
	{
		fprintf(stderr,"Could not find beam time offset in config file\n");
		return(0);
	}
	beam->timeOffset = quantize(tmp_double * MS_TO_S, PRF_CLOCK_RESOLUTION);

	//----------------//
	// Range Tracking //
	//----------------//

	int use_rgc;
	if (! config_list->GetInt(USE_RGC_KEYWORD, &use_rgc))
	{
		fprintf(stderr,"Could not find use RGC flag in config file\n");
		return(0);
	}
	beam->useRangeTracker = use_rgc;
	if (use_rgc)
	{
		substitute_string(BEAM_x_RGC_FILE_KEYWORD, "x", number, keyword);
		char* rgc_file = config_list->Get(keyword);
		if (rgc_file == NULL)
		{
			fprintf(stderr,"Could not find RGC file name in config file\n");
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
		fprintf(stderr,"Could not find use DTC flag in config file\n");
		return(0);
	}
	beam->useDopplerTracker = use_dtc;
	if (use_dtc)
	{
		substitute_string(BEAM_x_DTC_FILE_KEYWORD, "x", number, keyword);
		char* dtc_file = config_list->Get(keyword);
		if (dtc_file == NULL)
		{
			fprintf(stderr,"Could not find DTC file name in config file\n");
			return(0);
		}

		if (! beam->dopplerTracker.ReadBinary(dtc_file))
		{
			fprintf(stderr, "ConfigBeam: error reading DTC file %s\n",
				dtc_file);
			return(0);
		}
	}

	//-------------------------//
	// parameters for tracking //
	//-------------------------//

	if (use_rgc || use_dtc)
	{
		substitute_string(BEAM_x_PEAK_OFFSET_DN_KEYWORD, "x", number, keyword);
		int tmp_int;
		if (! config_list->GetInt(keyword, &tmp_int))
			return(0);
		beam->sasBeamOffsetDn = (unsigned int)tmp_int;
	}

	return(1);
}

//--------------//
// ConfigXTable //
//--------------//

int
ConfigXTable(
	XTable*			xTable,
	ConfigList*		config_list,
	char*			read_write)
{
	/**** Find out if XTable is to be configured READ or WRITE ***/
	int read=0;
	if (strcmp(read_write,"r") == 0)
		read=1;
	else if (strcmp(read_write,"w")==0)
		read=0;
	else
	{
		fprintf(stderr, "ConfigXTable: Bad read_write parameter");
		return(0);
	}

	/**** Get XTable Filename *****/

	char * xtable_filename= config_list->Get(XTABLE_FILENAME_KEYWORD);
	if (xtable_filename == NULL)
		return(0);

	xTable->SetFilename(xtable_filename);


	/**** Read header parameters for XTable object ****/

	int num_beams;
	if (! config_list->GetInt(NUMBER_OF_BEAMS_KEYWORD,&num_beams))
		return(0);

	int num_science_slices;
	if (! config_list->GetInt(SCIENCE_SLICES_PER_SPOT_KEYWORD,
		&num_science_slices))
	{
		return(0);
	}

	int num_guard_slices_each_side;
	if (! config_list->GetInt(GUARD_SLICES_PER_SIDE_KEYWORD,
		&num_guard_slices_each_side))
	{
		return(0);
	}

	int num_azimuths;
	if (! config_list->GetInt(XTABLE_NUM_AZIMUTHS_KEYWORD,&num_azimuths))
		return(0);

	int num_orbit_positions;
	if (! config_list->GetInt(XTABLE_NUM_ORBIT_STEPS_KEYWORD,
		&num_orbit_positions))
	{
		return(0);
	}

	float science_slice_bandwidth;
	if (! config_list->GetFloat(SCIENCE_SLICE_BANDWIDTH_KEYWORD,
		&science_slice_bandwidth))
	{
		fprintf(stderr,"Could not find slice bandwidth in config file\n");
		return(0);
	}
	science_slice_bandwidth*=KHZ_TO_HZ;

	float guard_slice_bandwidth;
	if (! config_list->GetFloat(GUARD_SLICE_BANDWIDTH_KEYWORD,
		&guard_slice_bandwidth))
	{
		fprintf(stderr,"Could not find guard slice bandwidth in config file\n");
		return(0);
	}
	guard_slice_bandwidth*=KHZ_TO_HZ;

	/**** If mode is READ, read in xTable and make sure its parameters match
		those read from the config file *****/

	if (read)
	{
		if (!xTable->Read())
		{
			fprintf(stderr,"Error reading xTable in ConfigXTable\n");
			return(0);
		}
		if (!xTable->CheckHeader(num_beams, num_science_slices,
				num_guard_slices_each_side, science_slice_bandwidth,
				guard_slice_bandwidth))
		{
			fprintf(stderr,"Header check failed in ConfigXTable\n");
			return(0);
		}
	}

	/***** If mode is WRITE, asign xTable parameters from parameters read from
		config file, and allocate it the arrays ****/

	else
	{
		xTable->numBeams=num_beams;
		xTable->numAzimuthBins=num_azimuths;
		xTable->numOrbitPositionBins=num_orbit_positions;
		xTable->numScienceSlices=num_science_slices;
		xTable->numGuardSlicesEachSide=num_guard_slices_each_side;
		xTable->scienceSliceBandwidth=science_slice_bandwidth;
		xTable->guardSliceBandwidth=guard_slice_bandwidth;
		xTable->numSlices=xTable->numScienceSlices +
			2 * xTable->numGuardSlicesEachSide;
		if (!xTable->Allocate())
		{
			fprintf(stderr,"ConfigXTable:Error allocating XTable object.\n");
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
	l00->SetInputFilename(l00_filename);
	l00->SetOutputFilename(l00_filename);

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
// ConfigL1A //
//-----------//

int
ConfigL1A(
	L1A*			l1a,
	ConfigList*		config_list)
{
	//---------------------------//
	// configure the l1a product //
	//---------------------------//

	char* l1a_filename = config_list->Get(L1A_FILE_KEYWORD);
	if (l1a_filename == NULL)
		return(0);
	l1a->SetInputFilename(l1a_filename);
	l1a->SetOutputFilename(l1a_filename);

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

	if (! l1a->AllocateBuffer(number_of_beams, antenna_cycles_per_frame,
		total_slices))
	{
		return(0);
	}

	//-------------------------//
	// configure the l1a frame //
	//-------------------------//

	if (! l1a->frame.Allocate(number_of_beams, antenna_cycles_per_frame,
		total_slices))
	{
		return(0);
	}

	return(1);
}

//-----------//
// ConfigL1B //
//-----------//

int
ConfigL1B(
	L1B*			l1b,
	ConfigList*		config_list)
{
	//---------------------------//
	// configure the l1b product //
	//---------------------------//

	char* l1b_filename = config_list->Get(L1B_FILE_KEYWORD);
	if (l1b_filename == NULL)
		return(0);
	l1b->SetInputFilename(l1b_filename);
	l1b->SetOutputFilename(l1b_filename);

	return(1);
}

//----------------//
// ConfigL1AToL1B //
//----------------//

int
ConfigL1AToL1B(
	L1AToL1B*		l1a_to_l1b,
	ConfigList*		config_list)
{
	//-------------------------//
	// output simga0 to stdout //
	//-------------------------//

	config_list->DoNothingForMissingKeywords();
	int output_sigma0_to_stdout;
	if (! config_list->GetInt(OUTPUT_SIGMA0_TO_STDOUT_KEYWORD,
		&output_sigma0_to_stdout))
	{
		output_sigma0_to_stdout = 0;	// default value
	}
	l1a_to_l1b->outputSigma0ToStdout = output_sigma0_to_stdout;

	l1a_to_l1b->simVs1BCheckfile =
		config_list->Get(ONEB_CHECKFILE_KEYWORD);

	config_list->ExitForMissingKeywords();

	//----------//
	// k-factor //
	//----------//

	int use_kfactor;
	if (! config_list->GetInt(USE_KFACTOR_KEYWORD, &use_kfactor))
		use_kfactor=0; // default value
	l1a_to_l1b->useKfactor=use_kfactor;
	if (use_kfactor)
	{
		if (!ConfigXTable(&(l1a_to_l1b->kfactorTable),config_list,"r"))
			return(0);
	}

	//------------------//
	// spot compositing //
	//------------------//

	int spot_comp;
	if (! config_list->GetInt(USE_SPOT_COMPOSITES_KEYWORD, &spot_comp))
		spot_comp = 0;
	l1a_to_l1b->useSpotCompositing = spot_comp;

	//---------------------------//
	// gain threshold for slices //
	//---------------------------//

	float gain;
	if (! config_list->GetFloat(SLICE_GAIN_THRESHOLD_KEYWORD, &gain))
		return(0);
	l1a_to_l1b->sliceGainThreshold = pow(10.0, 0.1 * gain);

	//-----------------------------------------------//
	// maximum number of slices to use in processing //
	//-----------------------------------------------//

	int max_slices;
	if (! config_list->GetInt(PROCESS_MAX_SLICES_KEYWORD, &max_slices))
		return(0);
	l1a_to_l1b->processMaxSlices = max_slices;

	return(1);
}

//-----------//
// ConfigL2A //
//-----------//

int
ConfigL2A(
	L2A*			l2a,
	ConfigList*		config_list)
{
	//---------------------------//
	// configure the L2A product //
	//---------------------------//

	char* l2a_filename = config_list->Get(L2A_FILE_KEYWORD);
	if (l2a_filename == NULL)
		return(0);
	l2a->SetInputFilename(l2a_filename);
	l2a->SetOutputFilename(l2a_filename);

	return(1);
}

//-----------//
// ConfigL2B //
//-----------//

int
ConfigL2B(
	L2B*			l2b,
	ConfigList*		config_list)
{
	//---------------------------//
	// configure the l2b product //
	//---------------------------//

	char* l2b_filename = config_list->Get(L2B_FILE_KEYWORD);
	if (l2b_filename == NULL)
		return(0);
	l2b->SetInputFilename(l2b_filename);
	l2b->SetOutputFilename(l2b_filename);
	
	return(1);
}

//----------------//
// ConfigL2AToL2B //
//----------------//

int
ConfigL2AToL2B(
	L2AToL2B*		l2a_to_l2b,
	ConfigList*		config_list)
{
	int tmp_int;
	float tmp_float;
	if (! config_list->GetInt(MEDIAN_FILTER_WINDOW_SIZE_KEYWORD, &tmp_int))
		return(0);
	l2a_to_l2b->medianFilterWindowSize = tmp_int;

	if (! config_list->GetInt(MEDIAN_FILTER_MAX_PASSES_KEYWORD, &tmp_int))
		return(0);
	l2a_to_l2b->medianFilterMaxPasses = tmp_int;

	if (! config_list->GetInt(MAX_RANK_FOR_NUDGING_KEYWORD, &tmp_int))
		return(0);
	l2a_to_l2b->maxRankForNudging = tmp_int;

	if (! config_list->GetInt(USE_MANY_AMBIGUITIES_KEYWORD, &tmp_int))
		return(0);
	l2a_to_l2b->useManyAmbiguities = tmp_int;

	if (! config_list->GetInt(USE_AMBIGUITY_WEIGHTS_KEYWORD, &tmp_int))
		return(0);
	l2a_to_l2b->useAmbiguityWeights = tmp_int;

	if (! config_list->GetInt(USE_PEAK_SPLITTING_KEYWORD, &tmp_int))
		return(0);
	l2a_to_l2b->usePeakSplitting = tmp_int;

	if (! config_list->GetInt(USE_H1_FLAG_KEYWORD, &tmp_int))
		return(0);
	l2a_to_l2b->useH1Flag = tmp_int;

	if( l2a_to_l2b->usePeakSplitting && l2a_to_l2b->useManyAmbiguities)
	{
		fprintf(stderr,
			"Cannot use ManyAmbiguities and PeakSplitting at the same time.\n");
		return(0);
	}
	if(l2a_to_l2b->usePeakSplitting)
	{
		if (! config_list->GetFloat(ONE_PEAK_WIDTH_KEYWORD, &tmp_float))
			return(0);
		l2a_to_l2b->onePeakWidth = tmp_float*dtr;
		if (! config_list->GetFloat(TWO_PEAK_SEPARATION_THRESHOLD_KEYWORD,
			&tmp_float))
		{
			return(0);
		}
		l2a_to_l2b->twoPeakSep = tmp_float*dtr;
		if (! config_list->GetFloat(SCALED_PROBABILITY_THRESHOLD_KEYWORD,
			&tmp_float))
		{
			return(0);
		}
		l2a_to_l2b->probThreshold = tmp_float;
	}

	//---------//
	// nudging //
	//---------//

	int use_nudging;
	if (! config_list->GetInt(USE_NUDGING_KEYWORD, &use_nudging))
		return(0);

	l2a_to_l2b->useNudging = use_nudging;
	if (use_nudging)
	{
        //------------------------//
        // configure nudging flag //
        //------------------------//

        if (! config_list->GetInt(SMART_NUDGE_FLAG_KEYWORD, &tmp_int))
            return(0);
        l2a_to_l2b->smartNudgeFlag = tmp_int;

        //-----------------------//
        // configure nudge field //
        //-----------------------//

		char* nudge_type = config_list->Get(NUDGE_WINDFIELD_TYPE_KEYWORD);
		if (nudge_type == NULL)
			return(0);
 
		char* nudge_windfield = config_list->Get(NUDGE_WINDFIELD_FILE_KEYWORD);
		if (nudge_windfield == NULL)
			return(0);

		if (! l2a_to_l2b->nudgeField.ReadType(nudge_windfield, nudge_type))
			return(0);
	}

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

	//--------------------------------//
	// configure the number of angles //
	//--------------------------------//

	int tmp_int;
	if (! config_list->GetInt(GMF_PHI_COUNT_KEYWORD, &tmp_int))
		return(0);
	if (! gmf->SetPhiCount(tmp_int))
		return(0);

	//---------------------------//
	// configure retrieval flags //
	//---------------------------//

	if (! config_list->GetInt(RETRIEVE_USING_KPC_FLAG_KEYWORD, &tmp_int))
		return(0);
	gmf->retrieveUsingKpcFlag = tmp_int;

	if (! config_list->GetInt(RETRIEVE_USING_KPM_FLAG_KEYWORD, &tmp_int))
		return(0);
	gmf->retrieveUsingKpmFlag = tmp_int;

	if (! config_list->GetInt(RETRIEVE_USING_KPRI_FLAG_KEYWORD, &tmp_int))
		return(0);
	gmf->retrieveUsingKpriFlag = tmp_int;

	if (! config_list->GetInt(RETRIEVE_USING_KPRS_FLAG_KEYWORD, &tmp_int))
		return(0);
	gmf->retrieveUsingKprsFlag = tmp_int;

	if (! config_list->GetInt(RETRIEVE_USING_LOGVAR_KEYWORD, &tmp_int))
		return(0);
	gmf->retrieveUsingLogVar = tmp_int;

    //------------------------//
    // configure nudging flag //
    //------------------------//

    if (! config_list->GetInt(SMART_NUDGE_FLAG_KEYWORD, &tmp_int))
        return(0);
    gmf->smartNudgeFlag = tmp_int;

	return(1);
}

//----------//
// ConfigKp //
//----------//

int
ConfigKp(
	Kp*				kp,
	ConfigList*		config_list)
{
	//---------------//
	// configure Kpm //
	//---------------//

	char* kpm_filename = config_list->Get(KPM_FILE_KEYWORD);
	if (kpm_filename == NULL)
		return(0);

	if (! kp->kpm.ReadTable(kpm_filename))
	{
		fprintf(stderr,"Error reading Kpm table from %s\n",kpm_filename);
		return(0);
	}

	//----------------//
	// configure Kpri //
	//----------------//

	double kp_ptgr;
	if (! config_list->GetDouble(PTGR_NOISE_KP_KEYWORD,&kp_ptgr))
		return(0);
	kp_ptgr=pow(10,0.1*kp_ptgr)-1.0;
	if (! kp->kpri.SetKpPtGr(kp_ptgr))
	{
		fprintf(stderr,"Error setting KpPtGr\n");
		return(0);
	}

	//----------------//
	// configure Kprs //
	//----------------//

	char* kprs_filename=config_list->Get(KPRS_FILE_KEYWORD);
	if (kprs_filename == NULL)
		return(0);
	if (strcmp(kprs_filename,"NONE") != 0
		&& strcmp(kprs_filename,"none") != 0
		&& strcmp(kprs_filename,"None") != 0)
	{
		if (! kp->kprs.Read(kprs_filename))
		{
			fprintf(stderr,"Error reading Kprs from %s\n",kprs_filename);
			return(0);
		}
	}

	return(1);
}

//----------------//
// ConfigKpmField //
//----------------//

int
ConfigKpmField(
	KpmField*			kpmField,
	ConfigList*		config_list)
{
	//------------------------//
	// configure the KpmField //
	//------------------------//

	config_list->DoNothingForMissingKeywords();

	char* kpm_filename = config_list->Get(KPM_FIELD_FILE_KEYWORD);

	if (kpm_filename == NULL)
	{
		// No file specified, so use an uncorrelated field.
		// KpmField is automatically initialized with _corrLength = 0.0.
		fprintf(stderr,"Using uncorrelated Kpm\n");
	}
	else if (! kpmField->corr.Read(kpm_filename))
	{
		// No file present (or wrong format) so build a field and write it
		// to the indicated file name (overwriting anything in the file).

		fprintf(stderr,"Error reading KpmField from %s\n",kpm_filename);
		return(0);
	}

	config_list->ExitForMissingKeywords();

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
	// configure L1B //
	//---------------//

	if (! ConfigL1B(&(grid->l1b), config_list))
		return(0);

	//---------------//
	// configure L2A //
	//---------------//

	if (! ConfigL2A(&(grid->l2a), config_list))
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
	config_list->DoNothingForMissingKeywords();
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
		fprintf(stderr,"Using explicit spacecraft start time\n");
	}
	else
	{
		double ephemeris_period = spacecraft_sim->GetEphemerisPeriod();
		double first_time = MIN(*instrument_start_time, *grid_start_time);
		*spacecraft_start_time = first_time -
			ephemeris_period * (EPHEMERIS_INTERP_ORDER + 2) - EPHEMERIS_BUFFER;
	}

	//---------------------//
	// spacecraft end time //
	//---------------------//

	if (config_list->GetDouble(SPACECRAFT_END_TIME_KEYWORD,
		spacecraft_end_time))
	{
		// nothing to do -- woo hoo once more!
		fprintf(stderr,"Using explicit spacecraft end time\n");
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
