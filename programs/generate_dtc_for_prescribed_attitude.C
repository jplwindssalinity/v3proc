//==============================================================//
// Copyright (C) 1997-2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    generate_dtc
//
// SYNOPSIS
//    generate_dtc [ -b ] <sim_config_file> <DTC_base>
//
// DESCRIPTION
//    Generates a set of Doppler Tracking Constants for each beam,
//    based upon the parameters in the simulation configuration file
//    and the given Receiver Gate Constants.
//
// OPTIONS
//    [ -b ]  Bias. Use the biases in the attitude when calculating
//              the DTC. This will simulate the effect of postlaunch
//              echo centering.
//
// OPERANDS
//    The following operands are supported:
//      <sim_config_file>  The sim_config_file needed listing all
//                           input parameters, input files, and
//                           output files.
//
//      <DTC_base>         The DTC output base.
//
// EXAMPLES
//    An example of a command line is:
//      % generate_constants sws1b.cfg dtc.dat
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
// AUTHOR
//    Bryan Stiles (Bryan.W.Stiles@jpl.nasa.gov)
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
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Misc.h"
#include "InstrumentGeom.h"
#include "ConfigList.h"
#include "L2A.h"
#include "ConfigSim.h"
#include "L2B.h"
#include "L2AToL2B.h"
#include "Tracking.h"
#include "Tracking.C"
#include "Array.h"
#include "Meas.h"
#include "SpacecraftSim.h"
#include "Spacecraft.h"
#include "QscatConfig.h"

using std::list;
using std::map; 

//-----------//
// TEMPLATES //
//-----------//
// Class declarations needed for templates
// eliminates need to include the entire header file
class AngleInterval;

template class List<StringPair>;
template class List<Meas>;
template class List<EarthPosition>;
template class List<WindVectorPlus>;
template class List<MeasSpot>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<off_t>;
template class List<OffsetList>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;
template class List<AngleInterval>;
template class std::list<string>;
template class std::map<string,string,Options::ltstr>;


//-----------//
// CONSTANTS //
//-----------//

#define DOPPLER_ORBIT_STEPS    256
#define DOPPLER_AZIMUTH_STEPS  90    // used for fitting

#define OPTSTRING  "b"

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

const char* usage_array[] = { "[ -b ]", "<config_file>", "<DTC_base>", 
"<pitch_bias>","<pitch_magnitude>","<pitch_cycles_per_orbit>","<pitch_phase>",
"<roll_bias>","<roll_magnitude>","< roll_cycles_per_orbit>","<roll_phase>",
"<yaw_bias>","<yaw_magnitude>","<yaw_cycles_per_orbit>","<yaw_phase>",
    0 };


int opt_bias = 0;

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
    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
        case 'b':
            opt_bias = 0;
	    fprintf(stderr,"Optbias disabled\n");
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    if (argc != optind + 14)
        usage(command, usage_array, 1);

    const char* config_file = argv[optind++];
    const char* dtc_base = argv[optind++];
    float pitch_bias=atof(argv[optind++])*dtr;
    float pitch_mag=atof(argv[optind++])*dtr;
    float pitch_freq=atof(argv[optind++]);
    float pitch_phase=atof(argv[optind++])*dtr;
    float roll_bias=atof(argv[optind++])*dtr;
    float roll_mag=atof(argv[optind++])*dtr;
    float roll_freq=atof(argv[optind++]);
    float roll_phase=atof(argv[optind++])*dtr;
    float yaw_bias=atof(argv[optind++])*dtr;
    float yaw_mag=atof(argv[optind++])*dtr;
    float yaw_freq=atof(argv[optind++]);
    float yaw_phase=atof(argv[optind++])*dtr;

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

    //-----------//
    // variables //
    //-----------//

    OrbitState* orbit_state = &(spacecraft.orbitState);
    Attitude* attitude = &(spacecraft.attitude);

    double orbit_period = spacecraft_sim.GetPeriod();
    double orbit_step_size = orbit_period / (double)DOPPLER_ORBIT_STEPS;
    double azimuth_step_size = two_pi / (double)DOPPLER_AZIMUTH_STEPS;

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

        //------------------------------//
        // start at an equator crossing //
        //------------------------------//

        double start_time =
            spacecraft_sim.FindNextArgOfLatTime(spacecraft_sim.GetEpoch(),
                EQX_ARG_OF_LAT, EQX_TIME_TOLERANCE);
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
            double time = start_time +
                orbit_step_size * ((double)orbit_step + 0.5);

            //-----------------------//
            // locate the spacecraft //
            //-----------------------//

            spacecraft_sim.UpdateOrbit(time, &spacecraft);

            //------------------------//
            // assign attitude        //
            //------------------------//

            float orbit_phase=(double)(orbit_step+0.5)*2*pi/DOPPLER_ORBIT_STEPS;
            float roll=roll_bias+roll_mag*cos(roll_freq*orbit_phase+roll_phase);
            float pitch=pitch_bias+pitch_mag*cos(pitch_freq*orbit_phase+pitch_phase);
            float yaw=yaw_bias+yaw_mag*cos(yaw_freq*orbit_phase+yaw_phase);
            attitude->SetRoll(roll);
            attitude->SetPitch(pitch);
            attitude->SetYaw(yaw);


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

                unsigned short encoder =
                    qscat.sas.AzimuthToEncoder(cds_azimuth);

                //------------------------------//
                // calculate receiver gate info //
                //------------------------------//

                CdsBeamInfo* cds_beam_info = qscat.GetCurrentCdsBeamInfo();
                unsigned char rx_gate_delay_dn;
                float rx_gate_delay_fdn;
                range_tracker->GetRxGateDelay(orbit_step, encoder,
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

                qscat.IdealCommandedDoppler(&spacecraft, NULL, 1); // 1 means to use spacecraft attitude to compute Doppler

                // constants are used to calculate the actual Doppler
                // frequency to correct for, but IdealCommandedDoppler
                // sets the commanded Doppler (ergo -)
                dop_com[azimuth_step] = -qscat.ses.txDoppler;
            }

            //------------------------//
            // fit doppler parameters //
            //------------------------//

            double a, p, c;
            azimuth_fit(DOPPLER_AZIMUTH_STEPS, dop_com, &a, &p, &c);


	    float mindop =-590000;
            float maxdop = 590000;
            if(2*a>maxdop-mindop) a=(maxdop-mindop)/2;
            if(c+a>maxdop) c=maxdop-a;
            if(c-a<mindop) c=mindop+a;
#define DEBUG
#ifdef DEBUG 
            float z=orbit_state->rsat.Get(2);
            printf("GEN_DTC OrbitPhase=%g Roll=%g Pitch=%g Yaw=%g a=%g p=%g c=%g dop0=%g dop90=%g dop180=%g dop270=%g z=%g\n",orbit_phase*rtd,roll*rtd,pitch*rtd,yaw*rtd,a,p,c,dop_com[1],dop_com[DOPPLER_AZIMUTH_STEPS/4],dop_com[DOPPLER_AZIMUTH_STEPS/2],dop_com[(DOPPLER_AZIMUTH_STEPS*3)/4],z);
#endif
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

    //----------------//
    // free the array //
    //----------------//

    free_array((void *)terms, 2, DOPPLER_ORBIT_STEPS, 3);

    return (0);
}
