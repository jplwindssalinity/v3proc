//==============================================================//
// Copyright (C) 1997-2001, California Institute of Technology. //
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
//    James N. Huddleston (hudd@casket.jpl.nasa.gov)
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
#include "QscatConfigDefs.h"
#include "ConfigSimDefs.h"
#include "Spacecraft.h"
#include "ConfigSim.h"
#include "QscatConfig.h"
#include "InstrumentGeom.h"
#include "List.h"
#include "List.C"
#include "Tracking.h"
#include "Tracking.C"
#include "BufferedList.h"
#include "BufferedList.C"

//-----------//
// TEMPLATES //
//-----------//

// Class declarations needed for templates
// eliminates need to include the entire header file
class AngleInterval;

template class List<AngleInterval>;
template class List<Meas>;
template class List<WindVectorPlus>;
template class List<long>;
template class TrackerBase<unsigned short>;
template class List<MeasSpot>;
template class List<OffsetList>;
template class List<StringPair>;
template class TrackerBase<unsigned char>;
template class List<OrbitState>;
template class BufferedList<OrbitState>;
template class List<EarthPosition>;

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

const char* usage_array[] = { "<sim_config_file>", "<DTC_base>", 0};

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
            opt_bias = 1;
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    if (argc != optind + 2)
        usage(command, usage_array, 1);

    const char* config_file = argv[optind++];
    const char* dtc_base = argv[optind++];

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

    unsigned int cds_encoder_offset_dn;
    double sas_encoder_offset;
    switch (qscat.sas.encoderElectronics)
    {
        case ENCODER_A:
            cds_encoder_offset_dn = CDS_ENCODER_A_OFFSET;
            sas_encoder_offset = SAS_ENCODER_A_OFFSET * dtr;
            break;
        case ENCODER_B:
            cds_encoder_offset_dn = CDS_ENCODER_B_OFFSET;
            sas_encoder_offset = SAS_ENCODER_B_OFFSET * dtr;
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

        unsigned int cds_beam_offset_dn;
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

            //------------------------------------------------//
            // if the bias is requested, set the attitude too //
            //------------------------------------------------//

            if (opt_bias)
            {
                spacecraft_sim.UpdateAttitude(time, &spacecraft);
            }

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

                qscat.IdealCommandedDoppler(&spacecraft);

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
