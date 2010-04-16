//==============================================================//
// Copyright (C) 1997-2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_configsim_c[] =
    "@(#) $Id$";

#include <sys/types.h>
#include <time.h>
#include "ConfigSim.h"
#include "ConfigSimDefs.h"
#include "OvwmConfigDefs.h"
#include "SpacecraftSim.h"
#include "XTable.h"
#include "BYUXTable.h"
#include "Misc.h"
#include "L00.h"
#include "L1A.h"
#include "L1AToL1B.h"
#include "L1B.h"
#include "L1BHdf.h"
#include "L2A.h"
#include "L2B.h"
#include "L2AToL2B.h"
#include "Constants.h"
#include "Distributions.h"
#include "Tracking.h"
#include "Kpm.h"
#include "EarthField.h"
#include "Kp.h"
#include "GenericGeom.h"
#include "ETime.h"




//-------------//
// LandMap     //
//-------------//
int ConfigLandMap(LandMap* lmap, ConfigList* config_list){

    char* landfile=config_list->Get(LANDMAP_FILE_KEYWORD); 
    config_list->DoNothingForMissingKeywords();
    char* landfiletype = config_list->Get(LANDMAP_TYPE_KEYWORD);
    config_list->ExitForMissingKeywords();
    char * nomstring="OLD_STYLE";
    if(!landfiletype){
      landfiletype=nomstring;
    }
    float lm_lat_start=0, lm_lon_start=0;
    int use_land;
    config_list->GetInt(USE_LANDMAP_KEYWORD, &use_land);
    if(use_land && strcasecmp(landfiletype,"LANDUSE")==0){
      config_list->GetFloat(LANDMAP_LAT_START_KEYWORD,&lm_lat_start);
      config_list->GetFloat(LANDMAP_LON_START_KEYWORD,&lm_lon_start);
    }

    if (!lmap->Initialize(landfile,use_land,landfiletype,lm_lon_start*dtr,lm_lat_start*dtr))
    {
      return(0);
    }
    return(1);
}
//------------------//
// ConfigSpacecraft //
//------------------//

int
ConfigSpacecraft(
    Spacecraft*  spacecraft,
    ConfigList*  config_list)
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
    SpacecraftSim*  spacecraft_sim,
    ConfigList*     config_list)
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

    if (! ConfigAttitude(config_list)){
      return(0);
    }
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

    if (! config_list->GetFloat(SIM_ROLL_BIAS_KEYWORD,
        &(spacecraft_sim->rollBias)))
        return(0);

    if (! config_list->GetFloat(SIM_PITCH_BIAS_KEYWORD,
        &(spacecraft_sim->pitchBias)))
        return(0);

    if (! config_list->GetFloat(SIM_YAW_BIAS_KEYWORD,
        &(spacecraft_sim->yawBias)))
        return(0);

    // Biases are specifed in degrees, but stored in radians.
    spacecraft_sim->rollBias *= dtr;
    spacecraft_sim->pitchBias *= dtr;
    spacecraft_sim->yawBias *= dtr;

    double epoch = 0.0;
    char* epoch_string = config_list->Get(ORBIT_EPOCH_KEYWORD);
    if (epoch_string != NULL)
    {
        ETime tmp_time;
        if (! tmp_time.FromCodeA(epoch_string))
        {
            fprintf(stderr, "ConfigSim: error parsing CodeA for %s (%s)\n",
                ORBIT_EPOCH_KEYWORD, epoch_string);
            return(0);
        }
        epoch = tmp_time.GetTime();
    }

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

    //-------------------------------//
    // optionally put the spacecraft //
    // at a specific place at the    //
    // epoch time                    //
    //-------------------------------//

    config_list->DoNothingForMissingKeywords();
    double epochlat,epochlon;
    int epochasc;
    if( config_list->GetDouble(ORBIT_EPOCH_LAT_KEYWORD,
			       &epochlat)){
       config_list->ExitForMissingKeywords();
       if(!config_list->GetDouble(ORBIT_EPOCH_LON_KEYWORD,
				  &epochlon)){
	 return(0);
       }
       if(!config_list->GetInt(ORBIT_EPOCH_ASC_KEYWORD,
				  &epochasc)){
	 return(0);
       }
       spacecraft_sim->LocationToOrbit(epochlon,epochlat,epochasc);
       if(g_velocity_frame==velocity_frame_inertial){
	 g_inertial_lat=epochlat*dtr;
         g_inertial_lon=epochlon*dtr;
         g_inertial_sctime=spacecraft_sim->GetEpoch();
       }
    }
    else if(g_velocity_frame==velocity_frame_inertial){
      fprintf(stderr,"Error: Inertial pointing requires ORBIT_EPOCH_XXX keywords\n");
      exit(1);
    }
    config_list->ExitForMissingKeywords();
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
ConfigAttitudeControlModel(
    AttDist*     attcntl,
    ConfigList*  config_list)
{
    char* string = config_list->Get(ATTITUDE_CONTROL_MODEL_KEYWORD);
    if (! string)
        return(0);

    if (strcasecmp(string, "NONE") == 0)
    {
        // By default mean, variance, and correlation length
        // are set to zero
        return(1);
    }
    else if (strcasecmp(string, "TIME_CORRELATED_GAUSSIAN") == 0)
    {
        float stdv, meanx, corrlength;
        if (! config_list->GetFloat(ROLL_CONTROL_STD_KEYWORD, &stdv))
            return(0);
        if (! config_list->GetFloat(ROLL_CONTROL_MEAN_KEYWORD, &meanx))
            return(0);
        if (! config_list->GetFloat(ROLL_CONTROL_CORRLENGTH_KEYWORD,
                &corrlength))
        {
            return(0);
        }

        attcntl->roll.SetVariance(stdv*stdv*dtr*dtr);
        attcntl->roll.SetMean(meanx*dtr);
        attcntl->roll.SetCorrelationLength(corrlength);
        attcntl->roll.SetSeed(get_seed(config_list, ROLL_CONTROL_SEED_KEYWORD,
            DEFAULT_ROLL_CONTROL_SEED));
        attcntl->roll.Initialize();

        if (! config_list->GetFloat(PITCH_CONTROL_STD_KEYWORD, &stdv))
            return(0);
        if (! config_list->GetFloat(PITCH_CONTROL_MEAN_KEYWORD, &meanx))
            return(0);
        if (! config_list->GetFloat(PITCH_CONTROL_CORRLENGTH_KEYWORD,
                &corrlength))
        {
            return(0);
        }

        attcntl->pitch.SetVariance(stdv*stdv*dtr*dtr);
        attcntl->pitch.SetMean(meanx*dtr);
        attcntl->pitch.SetCorrelationLength(corrlength);
        attcntl->pitch.SetSeed(get_seed(config_list,
            PITCH_CONTROL_SEED_KEYWORD, DEFAULT_PITCH_CONTROL_SEED));
        attcntl->pitch.Initialize();

        if (! config_list->GetFloat(YAW_CONTROL_STD_KEYWORD, &stdv))
            return(0);
        if (! config_list->GetFloat(YAW_CONTROL_MEAN_KEYWORD, &meanx))
            return(0);
        if (! config_list->GetFloat(YAW_CONTROL_CORRLENGTH_KEYWORD,
            &corrlength))
        {
            return(0);
        }

        attcntl->yaw.SetVariance(stdv*stdv*dtr*dtr);
        attcntl->yaw.SetMean(meanx*dtr);
        attcntl->yaw.SetCorrelationLength(corrlength);
        attcntl->yaw.SetSeed(get_seed(config_list,
            YAW_CONTROL_SEED_KEYWORD, DEFAULT_YAW_CONTROL_SEED));
        attcntl->yaw.Initialize();
    }
    else
    {
        fprintf(stderr, "No such Attitude Control Model. \n");
        fprintf(stderr, "Implemented models are:");
        fprintf(stderr, "TIME_CORRELATED_GAUSSIAN and NONE. \n");
        return(0);
    }

    return(1);
}

//------------------------------//
// ConfigAttitudeKnowledgeModel //
//------------------------------//

int
ConfigAttitudeKnowledgeModel(
    AttDist*     attknow,
    ConfigList*  config_list)
{
    char* string;

    string = config_list->Get(ATTITUDE_KNOWLEDGE_MODEL_KEYWORD);
    if (! string)
        return(0);

    if (strcasecmp(string, "NONE") == 0)
    {
        // By default mean, variance, and correlation length
        // are set to zero
        return(1);
    }
    else if (strcasecmp(string, "TIME_CORRELATED_GAUSSIAN") == 0)
    {

        float stdv, meanx, corrlength;

        if (! config_list->GetFloat(ROLL_KNOWLEDGE_STD_KEYWORD, &stdv))
            return(0);
        if (! config_list->GetFloat(ROLL_KNOWLEDGE_MEAN_KEYWORD, &meanx))
            return(0);
        if (! config_list->GetFloat(ROLL_KNOWLEDGE_CORRLENGTH_KEYWORD,
            &corrlength))
        {
            return(0);
        }

        attknow->roll.SetVariance(stdv*stdv*dtr*dtr);
        attknow->roll.SetMean(meanx*dtr);
        attknow->roll.SetCorrelationLength(corrlength);
        attknow->roll.SetSeed(get_seed(config_list,
            ROLL_KNOWLEDGE_SEED_KEYWORD, DEFAULT_ROLL_KNOWLEDGE_SEED));
        attknow->roll.Initialize();

        if (! config_list->GetFloat(PITCH_KNOWLEDGE_STD_KEYWORD, &stdv))
            return(0);
        if (! config_list->GetFloat(PITCH_KNOWLEDGE_MEAN_KEYWORD, &meanx))
            return(0);
        if (! config_list->GetFloat(PITCH_KNOWLEDGE_CORRLENGTH_KEYWORD,
                &corrlength))
        {
            return(0);
        }

        attknow->pitch.SetVariance(stdv*stdv*dtr*dtr);
        attknow->pitch.SetMean(meanx*dtr);
        attknow->pitch.SetCorrelationLength(corrlength);
        attknow->pitch.SetSeed(get_seed(config_list,
            PITCH_KNOWLEDGE_SEED_KEYWORD, DEFAULT_PITCH_KNOWLEDGE_SEED));
        attknow->pitch.Initialize();

        if (! config_list->GetFloat(YAW_KNOWLEDGE_STD_KEYWORD, &stdv))
            return(0);
        if (! config_list->GetFloat(YAW_KNOWLEDGE_MEAN_KEYWORD, &meanx))
            return(0);
        if (! config_list->GetFloat(YAW_KNOWLEDGE_CORRLENGTH_KEYWORD,
                &corrlength))
        {
            return(0);
        }

        attknow->yaw.SetVariance(stdv*stdv*dtr*dtr);
        attknow->yaw.SetMean(meanx*dtr);
        attknow->yaw.SetCorrelationLength(corrlength);
        attknow->yaw.SetSeed(get_seed(config_list,
            YAW_KNOWLEDGE_SEED_KEYWORD, DEFAULT_YAW_KNOWLEDGE_SEED));
        attknow->yaw.Initialize();
    }
    else
    {
        fprintf(stderr, "No such Attitude Control Model. \n");
        fprintf(stderr, "Implemented models are:");
        fprintf(stderr, "TIME_CORRELATED_GAUSSIAN and NONE. \n");
        return(0);
    }

    return(1);
}

/****************************************
//--------------------------------------//
// Configuration Routines for Specific  //
// Noise Distributions                  //
//--------------------------------------//

//----------------//
// ConfigGaussian //
//----------------//

Gaussian*
ConfigGaussian(const char* variance_keyword,
    const char* mean_keyword,
    ConfigList* config_list)
{
    double variance, meanx;

    if (! config_list->GetDouble(variance_keyword, &variance))
        return(NULL);
    variance*=dtr*dtr;
    if (! config_list->GetDouble(mean_keyword, &meanx))
        return(NULL);
    meanx*=dtr;
    Gaussian* new_g = new Gaussian((float)variance,(float)meanx);
    return(new_g);
}

    //---------------//
    // ConfigUniform //
    //---------------//

Uniform*
ConfigUniform(
    const char*        radius_keyword,
    const char*        mean_keyword,
    ConfigList*        config_list)
{
    double radius, meanx;

    if (! config_list->GetDouble(radius_keyword, &radius))
        return(NULL);
    radius*=dtr;
    if (! config_list->GetDouble(mean_keyword, &meanx))
        return(NULL);
    meanx*=dtr;
    Uniform* new_u = new Uniform((float)radius,float(meanx));
    return(new_u);
}

    //------------------------------//
    // ConfigGaussianRandomVelocity //
    //------------------------------//

RandomVelocity*
ConfigGaussianRandomVelocity(
    const char*        samprate_keyword,
    const char*        bound_keyword,
    const char*        mean_keyword,
    const char*        variance_keyword,
    ConfigList*        config_list)
{
    double variance, meanx, sample_rate, bound;
    GenericTimelessDist *velocity;

    if (! config_list->GetDouble(samprate_keyword, &sample_rate))
        return(NULL);

    if (! config_list->GetDouble(variance_keyword, &variance))
        return(NULL);
    variance*=dtr*dtr;
    if (! config_list->GetDouble(mean_keyword, &meanx))
        return(NULL);
    meanx*=dtr;
    if (! config_list->GetDouble(bound_keyword, &bound))
        return(NULL);
    bound*=dtr;
    velocity = new Gaussian((float)variance,0.0);
    RandomVelocity* new_rv = new RandomVelocity(velocity, (float)sample_rate, (float)bound, (float)meanx);
    return(new_rv);
}


    //-----------------------------//
    // ConfigUniformRandomVelocity //
    //-----------------------------//

RandomVelocity*
ConfigUniformRandomVelocity(
    const char*        samprate_keyword,
    const char*        bound_keyword,
    const char*        mean_keyword,
    const char*        radius_keyword,
    ConfigList*        config_list)
{
    double radius, meanx, sample_rate, bound;
    GenericTimelessDist *velocity;

    if (! config_list->GetDouble(samprate_keyword, &sample_rate))
        return(NULL);

    if (! config_list->GetDouble(radius_keyword, &radius))
        return(NULL);
    radius*=dtr;
    if (! config_list->GetDouble(mean_keyword, &meanx))
        return(NULL);
    meanx*=dtr;
    if (! config_list->GetDouble(bound_keyword, &bound))
        return(NULL);
    bound*=dtr;
    velocity = new Uniform((float)radius,0.0);
    RandomVelocity* new_rv = new RandomVelocity(velocity, (float)sample_rate,
        (float)bound, (float)meanx);
    return(new_rv);
}
********************************************/

//---------------//
// ConfigYahyaAntenna //
//---------------//

int
ConfigYahyaAntenna(
    YahyaAntenna*     YahyaAnt,
    ConfigList*  config_list)
{
    //-----------------------//
    // configure Yahya antenna //
    //-----------------------//

    float alam,diam,flen,bw1,bw2,theta0,disp,blockage; 

    if (! config_list->GetFloat(ANTENNA_WAVELENGTH_KEYWORD, &alam))
    {
        fprintf(stderr,
            "Could not find antenna wavelength in config file\n");
        return(0);
    }
    if (! config_list->GetFloat(ANTENNA_DIAMETER_KEYWORD, &diam))
    {
        fprintf(stderr,
            "Could not find antenna diameter in config file\n");
        return(0);
    }
    if (! config_list->GetFloat(ANTENNA_FOCAL_LENGTH_KEYWORD, &flen))
    {
        fprintf(stderr,
            "Could not find antenna focal length in config file\n");
        return(0);
    }
    if (! config_list->GetFloat(ANTENNA_FEED_BW1_KEYWORD, &bw1))
    {
        fprintf(stderr,
            "Could not find antenna feed bw1 in config file\n");
        return(0);
    }
    if (! config_list->GetFloat(ANTENNA_FEED_BW2_KEYWORD, &bw2))
    {
        fprintf(stderr,
            "Could not find antenna feed bw2 in config file\n");
        return(0);
    }
    if (! config_list->GetFloat(ANTENNA_THETA0_KEYWORD, &theta0))
    {
        fprintf(stderr,
            "Could not find antenna theta0 in config file\n");
        return(0);
    }
    if (! config_list->GetFloat(ANTENNA_DISPLACEMENT_KEYWORD, &disp))
    {
        fprintf(stderr,
            "Could not find antenna displacement in config file\n");
        return(0);
    }
    if (! config_list->GetFloat(ANTENNA_BLOCKAGE_KEYWORD, &blockage))
    {
        fprintf(stderr,
            "Could not find antenna blockage in config file\n");
        return(0);
    }

    YahyaAnt->SetAlam(alam);
    YahyaAnt->SetDiam(diam);
    YahyaAnt->SetFlen(flen);
    YahyaAnt->SetBw1(bw1);
    YahyaAnt->SetBw2(bw2);
    YahyaAnt->SetTheta0(theta0);
    YahyaAnt->SetDisp(disp);
    YahyaAnt->SetBlockage(blockage);
    return(1);
}

//---------------//
// ConfigAntenna //
//---------------//

int
ConfigAntenna(
    Antenna*     antenna,
    ConfigList*  config_list)
{
    //-----------------------//
    // configure the antenna //
    //-----------------------//

    int number_of_beams;
    if (! config_list->GetInt(NUMBER_OF_BEAMS_KEYWORD, &number_of_beams))
    {
        fprintf(stderr, "Could not find number of beams in config file\n");
        return(0);
    }
    antenna->numberOfBeams = number_of_beams;

    double roll, pitch, yaw;
    if (! config_list->GetDouble(ANTENNA_PEDESTAL_ROLL_KEYWORD, &roll))
    {
        fprintf(stderr,
            "Could not find antenna pedestal roll in config file\n");
        return(0);
    }
    if (! config_list->GetDouble(ANTENNA_PEDESTAL_PITCH_KEYWORD, &pitch))
    {
        fprintf(stderr,
            "Could not find antenna pedestal pitch in config file\n");
        return(0);
    }
    if (! config_list->GetDouble(ANTENNA_PEDESTAL_YAW_KEYWORD, &yaw))
    {
        fprintf(stderr, "Could not find antenna pedestal yaw in config file\n");
        return(0);
    }

    //---------------------------------//
    // Convert from degrees to radians //
    //---------------------------------//

    roll *= dtr;
    pitch *= dtr;
    yaw *= dtr;
    Attitude att;
    att.Set(roll, pitch, yaw, 1, 2, 3);
    antenna->SetPedestalAttitude(&att);

    //----------------//
    // initialization //
    //----------------//

    double start_time;
    if (! config_list->GetDouble(ANTENNA_START_TIME_KEYWORD, &start_time))
        return(0);
    antenna->startTime = start_time;

    double start_azi;
    if (! config_list->GetDouble(ANTENNA_START_AZIMUTH_KEYWORD, &start_azi))
        return(0);
    antenna->startAzimuth = start_azi*dtr;

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
    Beam*        beam,
    int          beam_number,
    ConfigList*  config_list)
{
    char keyword[1024];
    char number[8];
    char tmp_char;

    sprintf(number, "%d", beam_number);

    substitute_string(BEAM_x_POLARIZATION_KEYWORD, "x", number, keyword);
    if (! config_list->GetChar(keyword, &tmp_char))
    {
        fprintf(stderr, "Missing keyword %s in config file\n",keyword);
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
        fprintf(stderr, "Beam %d polarization (%c) not recognized\n",
            beam_number,tmp_char);
        return(0);
    }

    substitute_string(BEAM_x_PATTERN_FILE_KEYWORD, "x", number, keyword);
    char* pattern_file = config_list->Get(keyword);
    if (pattern_file == NULL)
    {
        fprintf(stderr, "Could not find beam pattern file in config file\n");
        return(0);
    }
    if (! beam->ReadBeamPattern(pattern_file))
    {
        fprintf(stderr,
            "Error while reading beam %d pattern file\n",beam_number);
        return(0);
    }

    //-----------//
    // peak gain //
    //-----------//

    substitute_string(BEAM_x_PEAK_GAIN_KEYWORD, "x", number, keyword);
    double peak_gain;    // dB
    if (! config_list->GetDouble(keyword, &peak_gain))
        return(0);

    //----------------------------------------------------//
    // waveguide loss - subtract from peak gain (in dB's) //
    //----------------------------------------------------//

    substitute_string(BEAM_x_WAVEGUIDE_LOSS_KEYWORD, "x", number, keyword);
    double waveguide_loss;    // dB (divisive factor - positive means loss)
    if (! config_list->GetDouble(keyword, &waveguide_loss))
        return(0);

    beam->peakGain = pow(10.0, 0.1*(peak_gain - waveguide_loss));

    //-----------------------------------------------------------------//
    // Setup one mechanical boresight, or two electrical boresights.
    //-----------------------------------------------------------------//

    config_list->DoNothingForMissingKeywords();

    double look_angle;        // deg
    double azimuth_angle;    // deg

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
            fprintf(stderr,
                "Missing mechanical boresight azimuth in config file\n");
            return(0);
        }
    }
    else
    {
        substitute_string(BEAM_x_LOOK_ANGLE_KEYWORD, "x", number, keyword);
        if (! config_list->GetDouble(keyword, &look_angle))
        {
            fprintf(stderr, "Could not find beam look angle in config file\n");
            return(0);
        }
        look_angle *= dtr;

        substitute_string(BEAM_x_AZIMUTH_ANGLE_KEYWORD, "x", number, keyword);
        if (! config_list->GetDouble(keyword, &azimuth_angle))
        {
            fprintf(stderr,
                "Could not find beam azimuth angle in config file\n");
            return(0);
        }
        azimuth_angle *= dtr;

        beam->SetElectricalBoresight(look_angle, azimuth_angle);
    }

    config_list->ExitForMissingKeywords();

    return(1);
}

//--------------//
// ConfigXTable //
//--------------//

int
ConfigXTable(
    XTable*      xTable,
    ConfigList*  config_list,
    char*        read_write)
{
    /**** Find out if XTable is to be configured READ or WRITE ***/
    int read_flag =0;
    if (strcmp(read_write, "r") == 0)
        read_flag =1;
    else if (strcmp(read_write, "w")==0)
        read_flag =0;
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
        fprintf(stderr, "Could not find slice bandwidth in config file\n");
        return(0);
    }
    science_slice_bandwidth*=KHZ_TO_HZ;

    float guard_slice_bandwidth;
    if (! config_list->GetFloat(GUARD_SLICE_BANDWIDTH_KEYWORD,
        &guard_slice_bandwidth))
    {
        fprintf(stderr,
            "Could not find guard slice bandwidth in config file\n");
        return(0);
    }
    guard_slice_bandwidth*=KHZ_TO_HZ;

    /**** If mode is READ, read in xTable and make sure its parameters match
        those read from the config file *****/

    if (read_flag )
    {
        if (!xTable->Read())
        {
            fprintf(stderr, "Error reading xTable in ConfigXTable\n");
            return(0);
        }
        if (!xTable->CheckHeader(num_beams, num_science_slices,
                num_guard_slices_each_side, science_slice_bandwidth,
                guard_slice_bandwidth))
        {
            fprintf(stderr, "Header check failed in ConfigXTable\n");
            return(0);
        }
    }

    /***** If mode is WRITE, asign xTable parameters from parameters read from
        config file, and allocate it the arrays ****/

    else
    {
        xTable->numBeams = num_beams;
        xTable->numAzimuthBins = num_azimuths;
        xTable->numOrbitPositionBins = num_orbit_positions;
        xTable->numScienceSlices = num_science_slices;
        xTable->numGuardSlicesEachSide = num_guard_slices_each_side;
        xTable->scienceSliceBandwidth = science_slice_bandwidth;
        xTable->guardSliceBandwidth = guard_slice_bandwidth;
        xTable->numSlices = xTable->numScienceSlices +
            2 * xTable->numGuardSlicesEachSide;
        if (!xTable->Allocate())
        {
            fprintf(stderr, "ConfigXTable:Error allocating XTable object.\n");
            return(0);
        }
    }

    return(1);
}

//-----------------//
// ConfigBYUXTable //
//-----------------//

int
ConfigBYUXTable(
    BYUXTable*   BYUX,
    ConfigList*  config_list)
{
    char* ibeam_file = config_list->Get(XFACTOR_INNER_BEAM_FILE_KEYWORD);
    char* obeam_file = config_list->Get(XFACTOR_OUTER_BEAM_FILE_KEYWORD);
    if (! (BYUX->Read(ibeam_file, obeam_file)))
        return(0);

    return(1);
}

//----------------//
// ConfigFbbTable //
//----------------//

int
ConfigFbbTable(
    FbbTable*    fbb_table,
    ConfigList*  config_list)
{
    char* fbb_ib_file = config_list->Get(FBB_INNER_BEAM_FILE_KEYWORD);
    char* fbb_ob_file = config_list->Get(FBB_OUTER_BEAM_FILE_KEYWORD);
    if (! fbb_table->Read(fbb_ib_file, fbb_ob_file))
        return(0);

    return(1);
}

//-----------//
// ConfigL00 //
//-----------//

int
ConfigL00(
    L00*         l00,
    ConfigList*  config_list)
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

    //-------------------------//
    // configure the l00 frame //
    //-------------------------//

    if (! l00->frame.Allocate(number_of_beams, antenna_cycles_per_frame,
        total_slices))
    {
        return(0);
    }

    //-----------------------------------------//
    // use the frame size to allocate a buffer //
    //-----------------------------------------//

    if (! l00->AllocateBuffer())
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
    L1A*         l1a,
    ConfigList*  config_list)
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

    //-------------------------//
    // configure the l1a frame //
    //-------------------------//

    if (! l1a->frame.Allocate(number_of_beams, antenna_cycles_per_frame,
        total_slices))
    {
        return(0);
    }

    //-----------------------------------------//
    // use the frame size to allocate a buffer //
    //-----------------------------------------//

    if (! l1a->AllocateBuffer())
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
    L1B*         l1b,
    ConfigList*  config_list)
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

//--------------//
// ConfigL1BHdf //
//--------------//

int
ConfigL1BHdf(
    L1BHdf*      l1bHdf,
    ConfigList*  config_list)
{
    //-------------------------------//
    // configure the l1b HDF product //
    //-------------------------------//
    float science_slice_bandwidth;
    if (! config_list->GetFloat(SCIENCE_SLICE_BANDWIDTH_KEYWORD,
        &science_slice_bandwidth))
    {
        fprintf(stderr, "Could not find slice bandwidth in config file\n");
        return(0);
    }
    l1bHdf->configBandwidth = science_slice_bandwidth * KHZ_TO_HZ;

    float txPulseWidth;
    if (! config_list->GetFloat(TX_PULSE_WIDTH_KEYWORD, &txPulseWidth))
    {
        fprintf(stderr, "Could not find txPulseWidth in config file\n");
        return(0);
    }
    l1bHdf->configTxPulseWidth = txPulseWidth * MS_TO_S;

    return(1);

} // ConfigL1BHdf

//----------------//
// ConfigL1AToL1B //
//----------------//

int
ConfigL1AToL1B(
    L1AToL1B*    l1a_to_l1b,
    ConfigList*  config_list)
{
    //-------------------------//
    // output simga0 to stdout //
    //-------------------------//

    config_list->DoNothingForMissingKeywords();
    int output_sigma0_to_stdout;
    if (! config_list->GetInt(OUTPUT_SIGMA0_TO_STDOUT_KEYWORD,
        &output_sigma0_to_stdout))
    {
        output_sigma0_to_stdout = 0;    // default value
    }
    l1a_to_l1b->outputSigma0ToStdout = output_sigma0_to_stdout;

    l1a_to_l1b->simVs1BCheckfile =
        config_list->Get(ONEB_CHECKFILE_KEYWORD);
    // Remove any pre-existing check file
    FILE* fptr = fopen(l1a_to_l1b->simVs1BCheckfile, "w");
    if (fptr != NULL) fclose(fptr);

    config_list->ExitForMissingKeywords();

    //--------------------------------------//
    // Read in the land map file            //
    //--------------------------------------//

    char* landfile = config_list->Get(LANDMAP_FILE_KEYWORD);
    int use_land;
    config_list->GetInt(USE_LANDMAP_KEYWORD, &use_land);
    if (! l1a_to_l1b->landMap.Initialize(landfile,use_land))
    {
        fprintf(stderr, "Cannot Initialize Land Map\n");
        exit(0);
    }

    //-------------------//
    // k-factor/x-factor //
    //-------------------//

    int use_kfactor;
    if (! config_list->GetInt(USE_KFACTOR_KEYWORD, &use_kfactor))
        use_kfactor = 0; // default value
    l1a_to_l1b->useKfactor = use_kfactor;

    int use_BYU_xfactor;
    if (! config_list->GetInt(USE_BYU_XFACTOR_KEYWORD, &use_BYU_xfactor))
        use_BYU_xfactor = 0; // default value
    l1a_to_l1b->useBYUXfactor = use_BYU_xfactor;

    /****** Exactly one of these must be true ***/
    if (use_kfactor + use_BYU_xfactor != 1)
    {
        fprintf(stderr,
            "ConfigL1AToL1B:X computation incorrectly specified.\n");
        return(0);
    }
    if (use_kfactor)
    {
        if (!ConfigXTable(&(l1a_to_l1b->kfactorTable),config_list, "r"))
            return(0);
    }
        else if (use_BYU_xfactor){
      if (!ConfigBYUXTable(&(l1a_to_l1b->BYUX),config_list))
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
    L2A*         l2a,
    ConfigList*  config_list)
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
    L2B*         l2b,
    ConfigList*  config_list)
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
    L2AToL2B*    l2a_to_l2b,
    ConfigList*  config_list)
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

    config_list->DoNothingForMissingKeywords();
    
        if (! config_list->GetInt("NEURAL_NET_TRAIN_ATI", &tmp_int))
	  l2a_to_l2b->ann_train_ati=0;
        else l2a_to_l2b->ann_train_ati=tmp_int;
	if (! config_list->GetFloat(INCLINATION_KEYWORD, &tmp_float))
	  l2a_to_l2b->orbitInclination=0;
        else l2a_to_l2b->orbitInclination=tmp_float*pi/180;

	if (! config_list->GetFloat("NEURAL_NET_TRAIN_DIRECTION_OFFSET", &tmp_float))
	  l2a_to_l2b->ann_train_diroff=0;
        else l2a_to_l2b->ann_train_diroff=tmp_float*pi/180;

        if (! config_list->GetInt(USE_SIGMA0_SPATIAL_WEIGHTS_KEYWORD, &tmp_int))
            return(0);
        l2a_to_l2b->useSigma0Weights = tmp_int;

        if (! config_list->GetFloat(SSW_CORRELATION_LENGTH_KEYWORD, &tmp_float))
            return(0);
        l2a_to_l2b->sigma0WeightCorrLength = tmp_float;
        
    l2a_to_l2b->ann_sigma0_corr_file = config_list->Get(NEURAL_NET_SIG0_CORR_FILE);
    if (l2a_to_l2b->ann_sigma0_corr_file) {
        // construct the neural net
        if(!l2a_to_l2b->s0corr_mlp.Read(l2a_to_l2b->ann_sigma0_corr_file)){
          fprintf(stderr,"L2AToL2B::ConvertAndWrite: Error: Unable to read netfile %s\n",l2a_to_l2b->ann_sigma0_corr_file);
          return 0;
        }
    }

    l2a_to_l2b->ann_error_est_file = config_list->Get(NEURAL_NET_ERR_EST_FILE);
    if (l2a_to_l2b->ann_error_est_file) {
        // construct the neural net
        if(!l2a_to_l2b->errEst_mlp.Read(l2a_to_l2b->ann_error_est_file)){
          fprintf(stderr,"L2AToL2B::ConvertAndWrite: Error: Unable to read netfile %s\n",l2a_to_l2b->ann_error_est_file);
          return 0;
        }
    }

    config_list->ExitForMissingKeywords();

    char* wr_method = config_list->Get(WIND_RETRIEVAL_METHOD_KEYWORD);
    if (wr_method == NULL)
        return(0);
    if (! l2a_to_l2b->SetWindRetrievalMethod(wr_method))
        return(0);

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
        // configure nudging flags //
        //------------------------//

        if (! config_list->GetInt(SMART_NUDGE_FLAG_KEYWORD, &tmp_int))
            return(0);
        l2a_to_l2b->smartNudgeFlag = tmp_int;

        config_list->DoNothingForMissingKeywords();
        if (! config_list->GetInt("ARRAY_NUDGING", &tmp_int))
	  tmp_int=0;
        l2a_to_l2b->arrayNudgeFlag = tmp_int;

        if(l2a_to_l2b->arrayNudgeFlag && l2a_to_l2b->smartNudgeFlag){
	  fprintf(stderr,"Use either SMART or ARRAY nudging but not both!\n");
	  exit(1);
	}

        config_list->ExitForMissingKeywords();
        //-----------------------//
        // configure nudge field //
        //-----------------------//

        char* nudge_type = config_list->Get(NUDGE_WINDFIELD_TYPE_KEYWORD);
        if (nudge_type == NULL)
            return(0);

        if (strcasecmp(nudge_type, "SV") == 0)
        {
            if (!config_list->GetFloat(NUDGE_WINDFIELD_LAT_MIN_KEYWORD,
                                       &l2a_to_l2b->nudgeField.lat_min) ||
                !config_list->GetFloat(NUDGE_WINDFIELD_LAT_MAX_KEYWORD,
                                       &l2a_to_l2b->nudgeField.lat_max) ||
                !config_list->GetFloat(NUDGE_WINDFIELD_LON_MIN_KEYWORD,
                                       &l2a_to_l2b->nudgeField.lon_min) ||
                !config_list->GetFloat(NUDGE_WINDFIELD_LON_MAX_KEYWORD,
                                       &l2a_to_l2b->nudgeField.lon_max))
            {
              fprintf(stderr, "ConfigNudgeWindField: SV can't determine range of lat and lon\n");
              return(0);
            }
        }
	float dummy;
        int fixed_speed_set=0;
	config_list->DoNothingForMissingKeywords();
	if (config_list->GetFloat(TRUTH_WIND_FIXED_SPEED_KEYWORD, &dummy))
	  {
	    fixed_speed_set=1;
	  }
	config_list->ExitForMissingKeywords();
        char* nudge_windfield = config_list->Get(NUDGE_WINDFIELD_FILE_KEYWORD);
        if (nudge_windfield == NULL)
            return(0);
       
        if (l2a_to_l2b->smartNudgeFlag)
        {
	  if(fixed_speed_set){
	    fprintf(stderr,"Cannot use SMART NUDGE WITH FIXED TRUTH SPEED\n");
	  }
            if (!config_list->GetFloat(NUDGE_WINDFIELD_LAT_MIN_KEYWORD,
                                       &l2a_to_l2b->nudgeVctrField.latMin) ||
                !config_list->GetFloat(NUDGE_WINDFIELD_LAT_MAX_KEYWORD,
                                       &l2a_to_l2b->nudgeVctrField.latMax) ||
                !config_list->GetFloat(NUDGE_WINDFIELD_LON_MIN_KEYWORD,
                                       &l2a_to_l2b->nudgeVctrField.lonMin) ||
                !config_list->GetFloat(NUDGE_WINDFIELD_LON_MAX_KEYWORD,
                                       &l2a_to_l2b->nudgeVctrField.lonMax))
            {
              fprintf(stderr, "Config NudgeVectorField:  can't determine range of lat and lon\n");
              return(0);
            }
            l2a_to_l2b->nudgeVctrField.lonMax*=dtr;
            l2a_to_l2b->nudgeVctrField.lonMin*=dtr;
            l2a_to_l2b->nudgeVctrField.latMax*=dtr;
            l2a_to_l2b->nudgeVctrField.latMin*=dtr;
            if (! l2a_to_l2b->nudgeVctrField.ReadVctr(nudge_windfield))
                return(0);
        }
        else if(l2a_to_l2b->arrayNudgeFlag){
	  if(fixed_speed_set){
	    fprintf(stderr,"Cannot use ARRAY NUDGE WITH FIXED TRUTH SPEED\n");
	  }
	  l2a_to_l2b->ReadNudgeArray(nudge_windfield);
        }
        else
        {
            if (! l2a_to_l2b->nudgeField.ReadType(nudge_windfield, nudge_type))
                return(0);

            //--------------------------//
            // Scale or Fix Wind Speeds //
            //--------------------------//

            config_list->DoNothingForMissingKeywords();
            float fixed_speed;
	    if (config_list->GetFloat(TRUTH_WIND_FIXED_SPEED_KEYWORD, &fixed_speed))
	      {
		l2a_to_l2b->nudgeField.FixSpeed(fixed_speed);
	      }
            float scale;
            if (config_list->GetFloat(TRUTH_WIND_SPEED_MULTIPLIER_KEYWORD,
                &scale))
            {
                l2a_to_l2b->nudgeField.ScaleSpeed(scale);
            }
            config_list->ExitForMissingKeywords();
        }

        //----------------//
        // threshold flag //
        //----------------//

        if (! config_list->GetInt(USE_NUDGING_THRESHOLD_KEYWORD, &tmp_int))
            return(0);
        l2a_to_l2b->useNudgeThreshold = tmp_int;

        if(l2a_to_l2b->useNudgeThreshold)
        {
            if (! config_list->GetFloat(NEAR_SWATH_NUDGE_THRESHOLD_KEYWORD,
                &tmp_float))
            {
                return(0);
            }
            l2a_to_l2b->nudgeThresholds[0]=tmp_float;
            if (! config_list->GetFloat(FAR_SWATH_NUDGE_THRESHOLD_KEYWORD,
                &tmp_float))
            {
                return(0);
            }
            l2a_to_l2b->nudgeThresholds[1] = tmp_float;
        }
        else
        {
            config_list->DoNothingForMissingKeywords();
            if (! config_list->GetInt(USE_STREAM_NUDGING_KEYWORD, &tmp_int))
            {
                l2a_to_l2b->useNudgeStream = 0;  // default
            }
            else
            {
                l2a_to_l2b->useNudgeStream = tmp_int;
                if (! config_list->GetFloat(STREAM_THRESHOLD_KEYWORD,
                    &tmp_float))
                {
                    l2a_to_l2b->streamThreshold = 0.0; // default
                }
                else
                {
                    l2a_to_l2b->streamThreshold = tmp_float;
                }
            }
            config_list->ExitForMissingKeywords();
        }
    }

    if (! config_list->GetInt(USE_NARROW_MEDIAN_FILTER_KEYWORD, &tmp_int))
        return(0);
    l2a_to_l2b->useNMF = tmp_int;

    if (! config_list->GetInt(USE_RANDOM_RANK_INIT_KEYWORD, &tmp_int))
        return(0);
    l2a_to_l2b->useRandomInit = tmp_int;

    config_list->DoNothingForMissingKeywords();
    if ( config_list->GetInt(USE_HURRICANE_NUDGE_KEYWORD, &tmp_int) && tmp_int)
    {
        config_list->ExitForMissingKeywords();
        l2a_to_l2b->useHurricaneNudgeField = tmp_int;
        char* hurricane_file
            = config_list->Get(HURRICANE_WINDFIELD_FILE_KEYWORD);
        if (hurricane_file == NULL)
            return(0);
        if (! l2a_to_l2b->hurricaneField.ReadHurricane(hurricane_file))
            return(0);
        if (! config_list->GetFloat(HURRICANE_RADIUS_KEYWORD,&tmp_float))
            return(0);
        l2a_to_l2b->hurricaneRadius = tmp_float; // km
        float lat, lon;
        if (! config_list->GetFloat(HURRICANE_CENTER_LATITUDE_KEYWORD,
            &tmp_float))
        {
            return(0);
        }
        lat = tmp_float * dtr;
        if (! config_list->GetFloat(HURRICANE_CENTER_LONGITUDE_KEYWORD,
            &tmp_float))
        {
            return(0);
        }
        lon = tmp_float * dtr;
        l2a_to_l2b->hurricaneCenter.SetAltLonGDLat(0.0, lon, lat);
    }
    else
    {
        l2a_to_l2b->useHurricaneNudgeField = 0;
    }

    // configure Kprc
    config_list->ExitForMissingKeywords();
    int sim_kprc = 0;
    float kprc_value = 0.0;
    config_list->GetInt(SIM_KPRC_KEYWORD, &sim_kprc);
    if (sim_kprc)
    {
        config_list->GetFloat(KPRC_VALUE_KEYWORD, &kprc_value);
    }

    // convert from dB
    float std = pow(10.0, 0.1 * (kprc_value)) - 1.0;
    l2a_to_l2b->kprc.SetVariance(std*std);
    l2a_to_l2b->kprc.SetMean(0.0);
    l2a_to_l2b->kprc.SetSeed(get_seed(config_list, KPRC_SEED_KEYWORD,
        DEFAULT_KPRC_SEED));
    return(1);
}

//-----------------//
// ConfigEphemeris //
//-----------------//

int
ConfigEphemeris(
    Ephemeris*   ephemeris,
    ConfigList*  config_list)
{
    char* ephemeris_filename = config_list->Get(EPHEMERIS_FILE_KEYWORD);
    if (ephemeris_filename == NULL)
        return(0);
    ephemeris->SetInputFile(ephemeris_filename);

    ephemeris->SetMaxNodes(30000);        // this should be calculated

    return(1);
}

//-----------------//
// ConfigWindField //
//-----------------//

int
ConfigWindField(
    WindField*   windfield,
    ConfigList*  config_list)
{
    //--------------------------//
    // configure the wind field //
    //--------------------------//

    char* windfield_type = config_list->Get(TRUTH_WIND_TYPE_KEYWORD);
    if (windfield_type == NULL)
    {
        fprintf(stderr, "ConfigWindField: can't determine windfield type\n");
        return(0);
    }

    if (strcasecmp(windfield_type, "SV") == 0)
    {
        if (!config_list->GetFloat(WIND_FIELD_LAT_MIN_KEYWORD, &windfield->lat_min) ||
            !config_list->GetFloat(WIND_FIELD_LAT_MAX_KEYWORD, &windfield->lat_max) ||
            !config_list->GetFloat(WIND_FIELD_LON_MIN_KEYWORD, &windfield->lon_min) ||
            !config_list->GetFloat(WIND_FIELD_LON_MAX_KEYWORD, &windfield->lon_max))
        {
          fprintf(stderr, "ConfigWindField: SV can't determine range of lat and lon\n");
          return(0);
        }
    }

    char* windfield_filename = config_list->Get(TRUTH_WIND_FILE_KEYWORD);
    if (windfield_filename == NULL)
    {
        fprintf(stderr, "ConfigWindField: can't determine windfield file\n");
        return(0);
    }

    if (! windfield->ReadType(windfield_filename, windfield_type))
    {
        fprintf(stderr, "ConfigWindField: can't read windfield\n");
        fprintf(stderr, "  (%s of type %s)\n", windfield_filename,
            windfield_type);
        return(0);
    }

    //--------------------------//
    // use as fixed wind speed? //
    //--------------------------//

    config_list->DoNothingForMissingKeywords();
    float fixed_speed;
    if (config_list->GetFloat(TRUTH_WIND_FIXED_SPEED_KEYWORD, &fixed_speed))
    {
        windfield->FixSpeed(fixed_speed);
    }
    float fixed_direction;
    if (config_list->GetFloat(TRUTH_WIND_FIXED_DIRECTION_KEYWORD, &fixed_direction))
    {
        fixed_direction *= dtr;
        windfield->FixDirection(fixed_direction);
    }
    int tmp_int;
    if (config_list->GetInt(TRUTH_WIND_RANDOM_DIRECTION_KEYWORD, &tmp_int))
    {
        windfield->RandomDirection(tmp_int);
    }
    config_list->ExitForMissingKeywords();

    //--------------------//
    // Scale Wind Speeds? //
    //--------------------//

    config_list->DoNothingForMissingKeywords();
    float scale;
    if (config_list->GetFloat(TRUTH_WIND_SPEED_MULTIPLIER_KEYWORD, &scale))
    {
        windfield->ScaleSpeed(scale);
        fprintf(stderr, "Warning: scaling all wind speeds by %g\n", scale);
    }
    config_list->ExitForMissingKeywords();

    return(1);
}

int  

ConfigRainField(RainField* rainfield, ConfigList* config_list){
    if (! config_list->GetInt(USE_3D_RAIN_MODEL_KEYWORD, &rainfield->flag_3d))
        return(0);

    if (!rainfield->flag_3d) {

      char* rainfield_filename1 = config_list->Get(TRUTH_RAIN_FILE_INNERH_KEYWORD);
      if (rainfield_filename1 == NULL)
      {
          fprintf(stderr, "ConfigRainField: can't determine rainfield file\n");
          return(0);
      }

      char* rainfield_filename2 = config_list->Get(TRUTH_RAIN_FILE_OUTERV_KEYWORD);
      if (rainfield_filename2 == NULL)
      {
          fprintf(stderr, "ConfigRainField: can't determine rainfield file\n");
          return(0);
      }

      if (!config_list->GetFloat(RAIN_FIELD_LAT_MIN_KEYWORD, &rainfield->lat_min) ||
          !config_list->GetFloat(RAIN_FIELD_LAT_MAX_KEYWORD, &rainfield->lat_max) ||
          !config_list->GetFloat(RAIN_FIELD_LON_MIN_KEYWORD, &rainfield->lon_min) ||
          !config_list->GetFloat(RAIN_FIELD_LON_MAX_KEYWORD, &rainfield->lon_max))
      {
          fprintf(stderr, "ConfigRainField: can't determine range of lat and lon\n");
          return(0);
      }
      if(!rainfield->ReadSVBinary(rainfield_filename1,rainfield_filename2)){
          fprintf(stderr, "ConfigRainField: can't read rainfield file\n");
          return(0);      
      }

    } else if (rainfield->flag_3d) {

      char* rainfield_filename1 = config_list->Get(RAIN_REFL_ATTN_FILE_KEYWORD);
      if (rainfield_filename1 == NULL)
      {
          fprintf(stderr, "ConfigRainField: can't determine rainfield file\n");
          return(0);
      }

      char* rainfield_filename2 = config_list->Get(RAIN_SPLASH_FILE_KEYWORD);
      if (rainfield_filename2 == NULL)
      {
          fprintf(stderr, "ConfigRainField: can't determine rainfield file\n");
          return(0);
      }

      if (!config_list->GetFloat(RAIN_FIELD_LAT_MIN_KEYWORD, &rainfield->lat_min) ||
          !config_list->GetFloat(RAIN_FIELD_LAT_MAX_KEYWORD, &rainfield->lat_max) ||
          !config_list->GetFloat(RAIN_FIELD_LON_MIN_KEYWORD, &rainfield->lon_min) ||
          !config_list->GetFloat(RAIN_FIELD_LON_MAX_KEYWORD, &rainfield->lon_max))
      {
          fprintf(stderr, "ConfigRainField: can't determine range of lat and lon\n");
          return(0);
      }

      if(!rainfield->ReadSV3DData(rainfield_filename1,rainfield_filename2)){
          fprintf(stderr, "ConfigRainField: can't read rainfield file\n");
          return(0);      
      }

      // calculate constant from reflectivity to sigma0
      float base_tx_frequency;   // GHz
      if (! config_list->GetFloat(BASE_TRANSMIT_FREQUENCY_KEYWORD,
          &base_tx_frequency))
      {
          return(0);      
      }
      float lambda = speed_light_kps/base_tx_frequency/1.e6;
      rainfield->const_ZtoSigma = pow(pi,5)*DIELECTRIC_WATER_CONST_SQ/pow(lambda,4)/1.e18; // last factor convert Z from mm^6/m^3 to m^6/m^3

    }

    return(1);
}
//-----------//
// ConfigGMF //
//-----------//

int
ConfigGMF(
    GMF*         gmf,
    ConfigList*  config_list)
{
    //-------------------//
    // configure the gmf //
    //-------------------//

    char* gmf_filename = config_list->Get(GMF_FILE_KEYWORD);
    if (gmf_filename == NULL)
        return(0);
    
    char* gmf_format = config_list->Get(GMF_FILE_FORMAT_KEYWORD);
    if (gmf_format == NULL)
        return(0);
    
    if (strcasecmp(gmf_format, "OLD_STYLE") == 0)
    {
        if (! gmf->ReadOldStyle(gmf_filename))
            return(0);
    }
    else if (strcasecmp(gmf_format, "QUIKSCAT") == 0)
    {
        if (! gmf->ReadQScatStyle(gmf_filename))
            return(0);
    }
    else if (strcasecmp(gmf_format, "C_BAND") == 0)
    {
      char* cgmf_filename = config_list->Get(C_BAND_GMF_FILE_KEYWORD);
      if (cgmf_filename == NULL)
        return(0);
      if (! gmf->ReadCBand(cgmf_filename))
	return(0);
    }
    else if (strcasecmp(gmf_format, "KU_AND_C") == 0)
    {

      char* cgmf_filename = config_list->Get(C_BAND_GMF_FILE_KEYWORD);
      if (cgmf_filename == NULL)
        return(0);
      if (! gmf->ReadKuAndC(gmf_filename,cgmf_filename))
            return(0);
    }
    else if (strcasecmp(gmf_format, "POLARIMETRIC") == 0)
    {
        if (! gmf->ReadPolarimetric(gmf_filename))
            return(0);
    }
   else if (strcasecmp(gmf_format, "HIGH_WIND") == 0)
    {
        if (! gmf->ReadHighWind(gmf_filename))
            return(0);
    }
    else
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

    config_list->DoNothingForMissingKeywords();

    if ( config_list->GetInt(RETRIEVE_OVER_ICE_KEYWORD, &tmp_int))
        gmf->retrieveOverIce = tmp_int;
    else
        gmf->retrieveOverIce = 0;

    if ( config_list->GetInt(RETRIEVE_OVER_COAST_KEYWORD, &tmp_int))
        gmf->retrieveOverCoast = tmp_int;
    else
        gmf->retrieveOverCoast = 0;

    if ( config_list->GetInt(RETRIEVE_USING_CRITERIA_FLAG_KEYWORD, &tmp_int))
        gmf->retrieveUsingCriteriaFlag = tmp_int;
    else
        gmf->retrieveUsingCriteriaFlag = 1;

    if ( config_list->GetInt(OBJECTIVE_FUNCTION_METHOD_KEYWORD, &tmp_int))
        gmf->objectiveFunctionMethod = tmp_int;
    else
        gmf->objectiveFunctionMethod = 0;

    config_list->ExitForMissingKeywords();

    float tmp_float;
    if (! config_list->GetFloat(REQUIRED_AZIMUTH_DIVERSITY_KEYWORD,
        &tmp_float))
    {
            return(0);
    }
    gmf->minimumAzimuthDiversity = tmp_float*dtr;

    // Otherwise it uses the default value initialized by the constructor

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
    Kp*          kp,
    ConfigList*  config_list)
{
    //---------------//
    // configure Kpm //
    //---------------//

    char* kpm_filename = config_list->Get(KPM_FILE_KEYWORD);
    if (kpm_filename == NULL)
        return(0);

    if (! kp->kpm.ReadTable(kpm_filename))
    {
        fprintf(stderr, "Error reading Kpm table from %s\n",kpm_filename);
        return(0);
    }

    //----------------//
    // configure Kpri //
    //----------------//

    double kp_ptgr;
    if (! config_list->GetDouble(PTGR_NOISE_KP_KEYWORD,&kp_ptgr))
        return(0);
    kp_ptgr = pow(10,0.1*kp_ptgr)-1.0;
    if (! kp->kpri.SetKpPtGr(kp_ptgr))
    {
        fprintf(stderr, "Error setting KpPtGr\n");
        return(0);
    }

    //----------------//
    // configure Kprs //
    //----------------//
    // Try to read the Kprs file. If it can't be read, check to see
    // if it is needed. If yes, bail; if not, warn and continue.

    config_list->MemorizeLogFlag();
    config_list->DoNothingForMissingKeywords();

    // gather information
    char* kprs_filename = config_list->Get(KPRS_FILE_KEYWORD);
    int retrieve_using_kprs = 0;   // assume it is not needed
    config_list->GetInt(RETRIEVE_USING_KPRS_FLAG_KEYWORD,
        &retrieve_using_kprs);

    if (kprs_filename == NULL || strcasecmp(kprs_filename, "NONE") != 0)
    {
        // No file specified (keyword missing or set to "none")
        // See if we care. No, really. See!
        if (retrieve_using_kprs == 1)
        {
            // we care!
            fprintf(stderr, "ConfigKp: missing Kprs file\n");
            exit(1);
        }
    }
    else
    {
        // file specified. read it.
        if (! kp->kprs.Read(kprs_filename))
        {
            fprintf(stderr, "ConfigKp: error reading Kprs file %s\n",
                kprs_filename);
            if (retrieve_using_kprs == 1)
            {
                // we need that file. bail.
                exit(1);
            }
            else
            {
                fprintf(stderr,
                    "  It doesn't appear to be needed. Continuing.\n");
            }
        }
    }
    config_list->RestoreLogFlag();

    return(1);
}

//----------------//
// ConfigKpmField //
//----------------//

int
ConfigKpmField(
    KpmField*    kpmField,
    ConfigList*  config_list)
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
        fprintf(stderr, "Using uncorrelated Kpm\n");
    }
    else if (! kpmField->corr.Read(kpm_filename))
    {
        // No file present (or wrong format) so build a field and write it
        // to the indicated file name (overwriting anything in the file).

        fprintf(stderr, "Error reading KpmField from %s\n",kpm_filename);
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
    Grid*        grid,
    ConfigList*  config_list)
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

    double ct_size;
    if (! config_list->GetDouble(GRID_WINDOW_CROSSTRACK_SIZE_KEYWORD, &ct_size))
        return(0);

    double at_size;
    if (! config_list->GetDouble(GRID_WINDOW_ALONGTRACK_SIZE_KEYWORD, &at_size))
        return(0);

    grid->Allocate(ct_res, at_res, ct_size, at_size);


    char* methstr=config_list->Get(REGRID_METHOD_KEYWORD);
    if(strcasecmp(methstr,"centroid")==0){
      grid->method=Grid::CENTROID;
    }
    else if(strcasecmp(methstr,"overlap")==0){
      grid->method=Grid::OVERLAP;
      config_list->GetFloat(REGRID_OVERLAP_FACTOR_KEYWORD,&(grid->overlapFactor));
    }
    else{
      fprintf(stderr,"ConfigGrid:Bad regrid method");
      return(0);
    }
    return(1);
}

//---------------//
// ConfigControl //
//---------------//

#define EPHEMERIS_BUFFER        300            // 5 minutes

int
ConfigControl(
    SpacecraftSim*  spacecraft_sim,
    ConfigList*     config_list,
    double*         grid_start_time,
    double*         grid_end_time,
    double*         instrument_start_time,
    double*         instrument_end_time,
    double*         spacecraft_start_time,
    double*         spacecraft_end_time)
{
    config_list->DoNothingForMissingKeywords();
    double orbit_period = spacecraft_sim->GetPeriod();

    //-----------------//
    // grid start time //
    //-----------------//

    char* grid_start_time_string =
        config_list->Get(GRID_START_TIME_KEYWORD);
    char* time_in_rev_string = config_list->Get(TIME_IN_REV_KEYWORD);
    if (grid_start_time_string != NULL)
    {
        ETime tmp_time;
        if (! tmp_time.FromCodeA(grid_start_time_string))
        {
            fprintf(stderr, "ConfigControl: error parsing CodeA for %s (%s)\n",
                GRID_START_TIME_KEYWORD, grid_start_time_string);
            exit(1);
        }
        *grid_start_time = tmp_time.GetTime();
    }
    else if (time_in_rev_string != NULL)
    {
        ETime tmp_time;
        if (! tmp_time.FromCodeA(time_in_rev_string))
        {
            fprintf(stderr, "ConfigControl: error parsing CodeA for %s (%s)\n",
                TIME_IN_REV_KEYWORD, time_in_rev_string);
            exit(1);
        }
        double time_in_rev = tmp_time.GetTime();
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
    char* grid_end_time_string =
        config_list->Get(GRID_END_TIME_KEYWORD);
    if (grid_end_time_string != NULL)
    {
        ETime tmp_time;
        if (! tmp_time.FromCodeA(grid_end_time_string))
        {
            fprintf(stderr, "ConfigControl: error parsing CodeA for %s (%s)\n",
                GRID_END_TIME_KEYWORD, grid_end_time_string);
            exit(1);
        }
        *grid_end_time = tmp_time.GetTime();
    }
    else if (config_list->GetDouble(GRID_LATITUDE_RANGE_KEYWORD, &grid_lat_range))
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
    char* instrument_start_time_string =
        config_list->Get(INSTRUMENT_START_TIME_KEYWORD);
    if (instrument_start_time_string != NULL)
    {
        ETime tmp_time;
        if (! tmp_time.FromCodeA(instrument_start_time_string))
        {
            fprintf(stderr, "ConfigControl: error parsing CodeA for %s (%s)\n",
                INSTRUMENT_START_TIME_KEYWORD, instrument_start_time_string);
            exit(1);
        }
        *instrument_start_time = tmp_time.GetTime();
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

    char* instrument_end_time_string =
        config_list->Get(INSTRUMENT_END_TIME_KEYWORD);
    if (instrument_end_time_string != NULL)
    {
        ETime tmp_time;
        if (! tmp_time.FromCodeA(instrument_end_time_string))
        {
            fprintf(stderr, "ConfigControl: error parsing CodeA for %s (%s)\n",
                INSTRUMENT_END_TIME_KEYWORD, instrument_end_time_string);
            exit(1);
        }
        *instrument_end_time = tmp_time.GetTime();
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

    char* spacecraft_start_time_string =
        config_list->Get(SPACECRAFT_START_TIME_KEYWORD);
    if (spacecraft_start_time_string != NULL)
    {
        ETime tmp_time;
        if (! tmp_time.FromCodeA(spacecraft_start_time_string))
        {
            fprintf(stderr, "ConfigControl: error parsing CodeA for %s (%s)\n",
                SPACECRAFT_START_TIME_KEYWORD, spacecraft_start_time_string);
            exit(1);
        }
        *spacecraft_start_time = tmp_time.GetTime();
        fprintf(stderr, "Using explicit spacecraft start time\n");
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

    char* spacecraft_end_time_string =
        config_list->Get(SPACECRAFT_END_TIME_KEYWORD);
    if (spacecraft_end_time_string != NULL)
    {
        ETime tmp_time;
        if (! tmp_time.FromCodeA(spacecraft_end_time_string))
        {
            fprintf(stderr, "ConfigControl: error parsing CodeA for %s (%s)\n",
                SPACECRAFT_END_TIME_KEYWORD, spacecraft_end_time_string);
            exit(1);
        }
        *spacecraft_end_time = tmp_time.GetTime();
        fprintf(stderr, "Using explicit spacecraft end time\n");
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

//----------------//
// ConfigAttitude //
//----------------//
// this odd function doesn't configure an object, but sets a
// global variable to the appropriate function for geodetic,
// geocentric, or constant RADEC pointing
//


int
ConfigAttitude(
    ConfigList*  config_list)
{
    char *attitude = config_list->Get(ATTITUDE_KEYWORD);
    if (attitude == NULL)
        return(0);

    if (strcasecmp("geocentric", attitude) == 0)
    {
        g_velocity_frame = velocity_frame_geocentric;
        return (1);
    }
    else if (strcasecmp("geodetic", attitude) == 0)
    {
        g_velocity_frame = velocity_frame_geodetic;
        return (1);
    }

    else if (strcasecmp("inertial", attitude) == 0)
    {
      g_velocity_frame = velocity_frame_inertial;
      // Bryan Stiles Sept 16 2009, this new case (see GenericGeom.C for 
      // velocity_frame_inertial function is implemented to handle the case where
      // the spacecraft is spinning about a constant vector in inertial coordinates
      //
      // In the event that this case occurs some setup is performed in
      // ConfigSpacecraftSim to determine the set the desired inertial vector from
      // a target lat,lon and time in the config file
      // This same lat and lon is used as was used for orbit setup. This means that
      // the ORBIT_EPOCH_LAT and ORBIT_EPOCH_LON keywords must be set. If you
      // attempt to use the older mean_anomaly_at_epoch, epoch_time setup
      // an error message is put out to stderr and execution stops.
      return (1);
    }
    return (0);
}

//-----------------//
// helper function //
//-----------------//

long
get_seed(
    ConfigList*  config_list,
    const char*  keyword,
    long         default_seed)
{
    // remember the config list setting
    config_list->MemorizeLogFlag();

    // be quiet
    config_list->DoNothingForMissingKeywords();

    long seed;
    int randomize_seeds;
    if (config_list->GetInt(RANDOMIZE_SEEDS_KEYWORD, &randomize_seeds) &&
        randomize_seeds)
    {
        // use a crummy random number generator and the current time
        int r = rand();
        seed = time(NULL) * r;
        printf("Initializing %s to %ld\n", keyword, seed);
    }
    else if (config_list->GetLong(keyword, &seed))
    {
        // use the seed from the config file
        ;
    }
    else
    {
        // use the default seed
        seed = default_seed;
    }

    // restore the config list setting
    config_list->RestoreLogFlag();
    return(seed);
}
