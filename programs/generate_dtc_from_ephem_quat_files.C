//==============================================================//
// Copyright (C) 1997-2013, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    generate_dtc_from_ephem_quat_files
//
// SYNOPSIS
//    generate_dtc_from_ephem_quat_files [ -b ] <sim_config_file> <DTC_base>
//
// DESCRIPTION
//    Generates a set of Doppler Tracking Constants for each beam,
//    based upon the parameters in the simulation configuration file
//    and the given Receiver Gate Constants.
//
// OPTIONS
//
// OPERANDS
//
// EXAMPLES
//
// ENVIRONMENT
//    Not environment dependent.
//
// EXIT STATUS
//    The following exit values are returned:
//       0  Program executed successfully
//      >0  Program had an error
//
// NOTES
//    None.
//
// AUTHORS
//    James N. Huddleston (hudd@casket.jpl.nasa.gov)
//    Bryan Stiles
//    Alex Fore
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
#include <vector>
#include "Misc.h"
#include "Ephemeris.h"
#include "Quat.h"
#include "ConfigList.h"
#include "QscatConfigDefs.h"
#include "ConfigSimDefs.h"
#include "Spacecraft.h"
#include "ConfigSim.h"
#include "QscatConfig.h"
#include "InstrumentGeom.h"
#include "Qscat.h"
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Tracking.h"
#include "Tracking.C"

//-----------//
// TEMPLATES //
//-----------//

// Class declarations needed for templates
// eliminates need to include the entire header file
class AngleInterval;

template class List<AngleInterval>;
template class BufferedList<OrbitState>;
template class BufferedList<QuatRec>;
template class List<OrbitState>;
template class List<QuatRec>;
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

#define DOPPLER_ORBIT_STEPS    256
#define DOPPLER_AZIMUTH_STEPS  90    // used for fitting

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

int opt_bias = 0;

const char usage_string[] = "-c config_file -e ephem.dat -q quats.dat -out_base outbase [-out_table out_dts_table] [-az_shift shift1 shift2] [-el_shift shift1 shift2]";

//--------------//
// MAIN PROGRAM //
//--------------//

int
main(
    int    argc,
    char*  argv[])
{
    //------------------------//
    // parse the command line //
    //------------------------//

    const char* command = no_path(argv[0]);
    char*       out_dts_table = NULL;
    char*       config_file   = NULL;
    char*       ephem_file    = NULL;
    char*       quat_file     = NULL;
    char*       dtc_base      = NULL;
    
    int    start_rev   = 0;
    double az_shift[2] = {0.0,0.0};
    double el_shift[2] = {0.0,0.0};
    
    int optind = 1;
    while( (optind < argc) && (argv[optind][0]=='-') ) {
      std::string sw = argv[optind];
      if( sw == "-out_table" ) {
        out_dts_table = argv[++optind];
      } else if( sw == "-s" ) {
        start_rev = atoi(argv[++optind]);
      } else if( sw == "-az_shift" ) {
        az_shift[0] = dtr*atof(argv[++optind]);
        az_shift[1] = dtr*atof(argv[++optind]);
      } else if( sw == "-el_shift" ) {
        el_shift[0] = dtr*atof(argv[++optind]);
        el_shift[1] = dtr*atof(argv[++optind]);
      } else if( sw == "-out_base" ) {
        dtc_base = argv[++optind];
      } else if( sw == "-c" ) {
        config_file = argv[++optind];
      } else if( sw == "-e" ) {
        ephem_file = argv[++optind];
      } else if( sw == "-q" ) {
        quat_file = argv[++optind];
      } else {
        fprintf(stderr,"%s: %s\n",command,&usage_string[0]);
        exit(1);
      }
      ++optind;
    }
    
    //printf("%f %f %f %f\n",az_shift[0],az_shift[1],el_shift[0],el_shift[1]);
    
    if( !config_file || !dtc_base || !ephem_file || !quat_file ) {
      fprintf(stderr,"%s: %s\n",command,&usage_string[0]);
      exit(1);    
    }

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

    //--------------------------------------//
    // force RGC to be read, don't read DTC //
    //--------------------------------------//

    config_list.StompOrAppend(USE_RGC_KEYWORD, "1");
    config_list.StompOrAppend(USE_DTC_KEYWORD, "0");
    config_list.StompOrAppend(USE_KFACTOR_KEYWORD, "0");

    //--------------------------------------------------------//
    // zero out the standard deviations if biases are desired //
    //--------------------------------------------------------//

    if (opt_bias)
    {
        config_list.StompOrAppend(ROLL_CONTROL_STD_KEYWORD, "0.0");
        config_list.StompOrAppend(PITCH_CONTROL_STD_KEYWORD, "0.0");
        config_list.StompOrAppend(YAW_CONTROL_STD_KEYWORD, "0.0");
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
    spacecraft_sim.LocationToOrbit(0.0, 0.0, 1);

    //----------------------------------------//
    // select geodetic or geocentric attitude //
    //----------------------------------------//

    if (! ConfigAttitude(&config_list))
    {
        fprintf(stderr, "%s: using default attitude reference\n", command);
    }

    //--------------------------------------//
    // create a QSCAT and a QSCAT simulator //
    //--------------------------------------//

    Qscat qscat;
    if (! ConfigQscat(&qscat, &config_list))
    {
        fprintf(stderr, "%s: error configuring QSCAT\n", command);
        exit(1);
    }

    QscatSim qscat_sim;
    if (! ConfigQscatSim(&qscat_sim, &config_list))
    {
        fprintf(stderr, "%s: error configuring instrument simulator\n",
            command);
        exit(1);
    }

    //----------------//
    // allocate terms //
    //----------------//

    // terms are [0] = amplitude, [1] = phase, [2] = bias
    double** terms;
    terms = (double **)make_array(sizeof(double), 2, DOPPLER_ORBIT_STEPS, 3);

    // Open ephem and quat files
    Ephemeris ephem(ephem_file,10000000);
    QuatFile  quats(quat_file,10000000);
    
    // Determine period from ephem file and start time at Asc node
    std::vector< double > asc_node_times;    
    OrbitState this_os;
    OrbitState prev_os;
    
    ephem.GetNextOrbitState( &prev_os );
    while(ephem.GetNextOrbitState( &this_os )) {
      
      double this_z  = this_os.rsat.GetZ();
      double prev_z  = prev_os.rsat.GetZ();
      double this_vz = this_os.vsat.GetZ();
      
      if( this_z>=0 && prev_z < 0 && this_vz >= 0 ) {
        // linear interpolation to z==0 time;
        double asc_node_time = prev_os.time 
                             + ( this_os.time - prev_os.time ) 
                             * ( 0.0 - prev_z ) / ( this_z - prev_z );
        
        asc_node_times.push_back( asc_node_time );
      }
      prev_os = this_os;
    }
    
    //------------------------------//
    // start at an equator crossing //
    //------------------------------//
    double start_time, orbit_period;
    if( start_rev == -1 ) {
      // use last node-to-node orbit in ephem / quat files
      start_time = asc_node_times[asc_node_times.size()-2];
      orbit_period = asc_node_times[asc_node_times.size()-1]-asc_node_times[asc_node_times.size()-2];
    } else if ( start_rev< asc_node_times.size()-1) {
      start_time = asc_node_times[start_rev];
      orbit_period = asc_node_times[start_rev+1]-asc_node_times[start_rev];
    } else {
      fprintf(stderr,"Error: bad value for start_rev\n");
      exit(1);
    }
    //printf("orbit_period: %f\n",orbit_period);

    //-----------//
    // variables //
    //-----------//

    OrbitState* orbit_state = &(spacecraft.orbitState);
    Attitude* attitude = &(spacecraft.attitude);

    //double orbit_period = spacecraft_sim.GetPeriod();
    double orbit_step_size = orbit_period / (double)DOPPLER_ORBIT_STEPS;
    double azimuth_step_size = two_pi / (double)DOPPLER_AZIMUTH_STEPS;
    unsigned int orbit_ticks_per_orbit =
        (unsigned int)(orbit_period * ORBIT_TICKS_PER_SECOND + 0.5);
    //printf("%s %d\n", ORBIT_TICKS_PER_ORBIT_KEYWORD, orbit_ticks_per_orbit);
    qscat.cds.CmdOrbitTicksPerOrbit(orbit_ticks_per_orbit);


    //----------------------------//
    // select encoder information //
    //----------------------------//

    unsigned int cds_encoder_offset_dn = 0;
    double sas_encoder_offset = 0.0;
    switch (qscat.sas.encoderElectronics)
    {
        case ENCODER_A:
            cds_encoder_offset_dn = qscat.cds.encoderAOffset;
            sas_encoder_offset = qscat.sas.encoderAOffset * dtr;
            break;
        case ENCODER_B:
            cds_encoder_offset_dn = qscat.cds.encoderBOffset;
            sas_encoder_offset = qscat.sas.encoderBOffset * dtr;
            break;
        default:
            fprintf(stderr, "%s: unknown encoder electronics\n", command);
            exit(1);
    }
    double cds_encoder_offset = (double)cds_encoder_offset_dn * two_pi /
        (double)ENCODER_N;

    //-------------------------------------------//
    // determine spin rate in radians per second //
    //-------------------------------------------//

    double assumed_spin_rate = qscat.cds.GetAssumedSpinRate();
    assumed_spin_rate *= rpm_to_radps;

    //-----------------//
    // loop over beams //
    //-----------------//
    
    FILE* ofp_dts_table = NULL;
    if( out_dts_table ) {
      ofp_dts_table = fopen(out_dts_table,"w");
      int tmp = DOPPLER_ORBIT_STEPS;
      fwrite(&tmp,sizeof(int),1,ofp_dts_table);
      tmp = DOPPLER_AZIMUTH_STEPS;
      fwrite(&tmp,sizeof(int),1,ofp_dts_table);
    }
    for (int beam_idx = 0; beam_idx < NUMBER_OF_QSCAT_BEAMS; beam_idx++)
    {
        //---------------------------//
        // get the beam offset angle //
        //---------------------------//

        unsigned int cds_beam_offset_dn = 0;
        switch (beam_idx)
        {
            case 0:
                cds_beam_offset_dn = BEAM_A_OFFSET;
                break;
            case 1:
                cds_beam_offset_dn = BEAM_B_OFFSET;
                break;
            default:
                fprintf(stderr, "%s: too many beams\n", command);
                exit(1);
                break;
        }
        double cds_beam_offset = (double)cds_beam_offset_dn * two_pi /
            (double)ENCODER_N;

        //--------------------------//
        // allocate Doppler tracker //
        //--------------------------//

        qscat.cds.currentBeamIdx = beam_idx;
        Beam* beam = qscat.GetCurrentBeam();

        CdsBeamInfo* cds_beam_info = qscat.GetCurrentCdsBeamInfo();
        DopplerTracker* doppler_tracker = &(cds_beam_info->dopplerTracker);
        RangeTracker* range_tracker = &(cds_beam_info->rangeTracker);

        if (! doppler_tracker->Allocate(DOPPLER_ORBIT_STEPS))
        {
            fprintf(stderr, "%s: error allocating Doppler tracker\n", command);
            exit(1);
        }

       qscat.cds.SetEqxTime(start_time);

        //------------//
        // initialize //
        //------------//

        if (! qscat_sim.Initialize(&qscat))
        {
            fprintf(stderr, "%s: error initializing the QSCAT simulator\n",
                command);
            exit(1);
        }

        if (! spacecraft_sim.Initialize(start_time))
        {
            fprintf(stderr, "%s: error initializing spacecraft simulator\n",
                command);
            exit(1);
        }

        //--------------------//
        // loop through orbit //
        //--------------------//

        for (int orbit_step = 0; orbit_step < DOPPLER_ORBIT_STEPS; orbit_step++)
        {
            //--------------------//
            // calculate the time //
            //--------------------//

            // addition of 0.5 centers on orbit_step
            double time = start_time + orbit_step_size * ((double)orbit_step + 0.5);

            //-----------------------//
            // locate the spacecraft //
            //-----------------------//

            spacecraft_sim.UpdateOrbit(time, &spacecraft);
            qscat.cds.SetTime( time );
            
            //------------------------------------------------//
            // if the bias is requested, set the attitude too //
            //------------------------------------------------//

            if (opt_bias)
            {
                spacecraft_sim.UpdateAttitude(time, &spacecraft);
            }

            //---Modified to use input ephem and quaterion file------------
            // Overwrite spacecraft attitude and orbitstate with that
            // interpolated from ephem and quaterion files
            Quat this_quat;

            // Interpolate quaternion to this time
            quats.GetQuat( time, &this_quat );
            
            // Convert to attitude angles and overwrite those in spacecraft object
            this_quat.GetAttitudeGS( &(spacecraft.attitude) );
            
            //spacecraft.attitude.SetPitch(spacecraft.attitude.GetPitch()+4.0*dtr);
            
            // Interpolate ephem and overwrite that in spacecraft object
            ephem.GetOrbitState( time, EPHEMERIS_INTERP_ORDER, &(spacecraft.orbitState) );
            //--------------------------------------------------------------


            //----------------------//
            // step through azimuth //
            //----------------------//

            double dop_com[DOPPLER_AZIMUTH_STEPS];
            for (int azimuth_step = 0; azimuth_step < DOPPLER_AZIMUTH_STEPS;
                azimuth_step++)
            {
                //--------------------------------//
                // calculate azimuth angle to use //
                //--------------------------------//

                // The table needs to be built for the CDS algorithm,
                // but we need to determine the actual antenna azimuth
                // angle in order to do the correct calculations.  The
                // following code starts from the CDS azimuth value,
                // backtracks to the original sampled encoder and then
                // calculates the actual antenna azimuth at the ground
                // impact time.  This method of doing the calculation will
                // allow for things like changes in the antenna spin rate
                // which will affect the actual antenna azimuth but not
                // the CDS estimation (which uses hardcoded spin rates).

                // start with the azimuth angle to be used by the CDS
                double cds_azimuth = azimuth_step_size * (double)azimuth_step;
                double azimuth = cds_azimuth;

                // subtract the beam offset
                azimuth -= cds_beam_offset;

                // subtract an estimate of the centering offset
                // uses an estimate of the round trip time as
                // the previous pulses round trip time
                qscat.sas.antenna.SetEncoderAzimuthAngle(cds_azimuth);
                Antenna* antenna = &(qscat.sas.antenna);
                CoordinateSwitch antenna_frame_to_gc =
                    AntennaFrameToGC(orbit_state, attitude, antenna, azimuth);
                double look, az;
                if (! GetPeakSpatialResponse2(&antenna_frame_to_gc,
                    &spacecraft, beam, antenna->spinRate, &look, &az))
                {
                    fprintf(stderr,
                        "%s: error finding peak spatial response\n", command);
                    exit(1);
                }

                double gain_c;
                qscat.SpatialResponse( &antenna_frame_to_gc,
                                       &spacecraft, 
                                       look, az, &gain_c, 1 );

                look += el_shift[beam_idx];
                az   += asin(sin(az_shift[beam_idx])/sin(look));

                double gain_3db;
                qscat.SpatialResponse( &antenna_frame_to_gc,
                                       &spacecraft, 
                                       look, az, &gain_3db, 1 );
                
                //printf("%12.6f %12.6f %12.6f\n",look,az,10*log10( gain_3db / gain_c ) );
                
                Vector3 vector;
                vector.SphericalSet(1.0, look, az);

                QscatTargetInfo qti;
                if (! qscat.TargetInfo(&antenna_frame_to_gc, &spacecraft,
                    vector, &qti))
                {
                    fprintf(stderr, "%s: error finding round trip time\n",
                        command);
                    exit(1);
                }

                // then apply to the azimuth angle
                double delay = (qti.roundTripTime + qscat.ses.txPulseWidth) /
                    2.0;
                azimuth -= (delay * assumed_spin_rate);

                // subtract the cds encoder offset
                azimuth -= cds_encoder_offset;

                // subtract the internal (sampling) delay angle
                double cds_pri = MS_TO_S * (double)qscat.cds.priDn / 10.0;
                azimuth -= (cds_pri * assumed_spin_rate);

                // add the actual sampling delay angle
                azimuth += (qscat.ses.pri * qscat.sas.antenna.spinRate);

                // and get to the center of the transmit pulse so that
                // the two-way gain product is formed correctly
                azimuth += (qscat.ses.txPulseWidth *
                    qscat.sas.antenna.spinRate / 2.0);

                // apply the sas encoder offset
                azimuth += sas_encoder_offset;

                //---------------------------//
                // set the Tx center azimuth //
                //---------------------------//

                qscat.sas.antenna.SetTxCenterAzimuthAngle(azimuth);

                //-------------------------//
                // set the antenna azimuth //
                //-------------------------//

                qscat.sas.antenna.SetEncoderAzimuthAngle(azimuth);

                //-----------------------------------------------------//
                // determine the encoder value to use in the algorithm //
                //-----------------------------------------------------//

                unsigned short encoder = qscat.sas.AzimuthToEncoder(cds_azimuth);
                unsigned short orbstep = qscat.cds.SetAndGetOrbitStep();
                //------------------------------//
                // calculate receiver gate info //
                //------------------------------//

                CdsBeamInfo* cds_beam_info = qscat.GetCurrentCdsBeamInfo();
                unsigned char rx_gate_delay_dn;
                float rx_gate_delay_fdn;
                range_tracker->GetRxGateDelay(orbstep, encoder,
                    cds_beam_info->rxGateWidthDn, qscat.cds.txPulseWidthDn,
                    &rx_gate_delay_dn, &rx_gate_delay_fdn);
                // set it with the exact value to eliminate quant. effects
                qscat.ses.CmdRxGateDelayFdn(rx_gate_delay_fdn);

                //-------------------//
                // coordinate system //
                //-------------------//

                antenna_frame_to_gc = AntennaFrameToGC(orbit_state, attitude,
                    antenna, qscat.sas.antenna.txCenterAzimuthAngle);

                if (! GetPeakSpatialResponse2(&antenna_frame_to_gc,
                    &spacecraft, beam, antenna->spinRate, &look, &azimuth))
                {
                    fprintf(stderr,
                        "%s: error finding peak spatial response\n", command);
                    exit(1);
                }
                vector.SphericalSet(1.0, look, azimuth);
                
                //--------------------------------//
                // calculate corrective frequency //
                //--------------------------------//
                // 1 means to use spacecraft attitude to compute Doppler
                qscat.IdealCommandedDoppler(&spacecraft, NULL, 1); 
                
                // constants are used to calculate the actual Doppler
                // frequency to correct for, but IdealCommandedDoppler
                // sets the commanded Doppler (ergo -)
                dop_com[azimuth_step] = -qscat.ses.txDoppler;
            }
            
            if(out_dts_table) fwrite(&dop_com,sizeof(double),DOPPLER_AZIMUTH_STEPS,ofp_dts_table);
            
            //------------------------//
            // fit doppler parameters //
            //------------------------//

            double a, p, c;
            azimuth_fit(DOPPLER_AZIMUTH_STEPS, dop_com, &a, &p, &c);
            *(*(terms + orbit_step) + 0) = a;
            *(*(terms + orbit_step) + 1) = p;
            *(*(terms + orbit_step) + 2) = c;
        }

        //-------------//
        // set Doppler //
        //-------------//

        doppler_tracker->SetTerms(terms);

        //------------------------------------------//
        // write out the doppler tracking constants //
        //------------------------------------------//

        char filename[1024];
        sprintf(filename, "%s.%d", dtc_base, beam_idx + 1);
        if (! doppler_tracker->WriteBinary(filename))
        {
            fprintf(stderr, "%s: error writing DTC file %s\n", command,
                filename);
            exit(1);
        }
    }
    
    if( out_dts_table ) fclose(ofp_dts_table);
    
    //----------------//
    // free the array //
    //----------------//

    free_array((void *)terms, 2, DOPPLER_ORBIT_STEPS, 3);

    return (0);
}
