//==============================================================//
// Copyright (C) 1997-2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    generate_rgc
//
// SYNOPSIS
//    generate_rgc [ -f ] <sim_config_file> <RGC_base>
//
// DESCRIPTION
//    Generates a set of Receiver Gate Constants for each beam based
//    upon the parameters in the simulation configuration file.
//
// OPTIONS
//      [ -f ]  Make the range fixed over azimuth.
//
// OPERANDS
//    The following operands are supported:
//      <sim_config_file>  The sim_config_file needed listing
//                           all input parameters.
//
//      <RGC_base>         The RGC output base.
//
// EXAMPLES
//    An example of a command line is:
//      % generate_constants sws1b.cfg rgc.dat
//
// ENVIRONMENT
//    Not environment dependent.
//
// EXIT STATUS
//    The following exit values are returned:
//       0   Program executed successfully
//      >0  Program had an error
//
// NOTES
//    None.
//
// AUTHOR
//    James N. Huddleston
//    hudd@casket.jpl.nasa.gov
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
#include "Qscat.h"
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
template class List<MeasSpot>;
template class List<OffsetList>;
template class List<StringPair>;
template class TrackerBase<unsigned char>;
template class List<OrbitState>;
template class BufferedList<OrbitState>;
template class TrackerBase<unsigned short>;
template class List<EarthPosition>;

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING  "f"

#define RANGE_ORBIT_STEPS    256
#define RANGE_AZIMUTH_STEPS  90    // used for fitting

#define EQX_TIME_TOLERANCE   0.1

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

const char* usage_array[] = { "[ -f ]", "<sim_config_file>", "<RGC_base>",
    0 };

int opt_fixed = 0;     // by default, delay can change as a function of azimuth

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
    extern int optind;

    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
        case 'f':
            opt_fixed = 1;
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    if (argc != optind + 2)
        usage(command, usage_array, 1);

    const char* config_file = argv[optind++];
    const char* rgc_base = argv[optind++];

    //--------------------------------//
    // read in simulation config file //
    //--------------------------------//

    ConfigList config_list;
    if (! config_list.Read(config_file))
    {
        fprintf(stderr, "%s: error reading configuration file %s\n",
            command, config_file);
        exit(1);
    }

    //----------------------------------//
    // force RGC and DTC to not be read //
    //----------------------------------//

    config_list.StompOrAppend(USE_RGC_KEYWORD, "0");
    config_list.StompOrAppend(USE_DTC_KEYWORD, "0");
    config_list.StompOrAppend(ORBIT_TICKS_PER_ORBIT_KEYWORD, "0");
    config_list.StompOrAppend(USE_KFACTOR_KEYWORD, "0");

    //----------------------------------------------//
    // create a spacecraft and spacecraft simulator //
    //----------------------------------------------//

    Spacecraft spacecraft;
    if (! ConfigSpacecraft(&spacecraft, &config_list))
    {
        fprintf(stderr, "%s: error configuring spacecraft\n", command);
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
        fprintf(stderr, "%s: error configuring QSCAT simulator\n", command);
        exit(1);
    }

    //----------------//
    // allocate terms //
    //----------------//

    // terms are [0] = amplitude, [1] = phase, [2] = bias
    double** terms;
    terms = (double **)make_array(sizeof(double), 2, RANGE_ORBIT_STEPS, 3);

    //------------//
    // initialize //
    //------------//

    double orbit_period = spacecraft_sim.GetPeriod();
    double orbit_step_size = orbit_period / (double)RANGE_ORBIT_STEPS;
    double azimuth_step_size = two_pi / (double)RANGE_AZIMUTH_STEPS;
    unsigned int orbit_ticks_per_orbit =
        (unsigned int)(orbit_period * ORBIT_TICKS_PER_SECOND + 0.5);
    printf("%s %d\n", ORBIT_TICKS_PER_ORBIT_KEYWORD, orbit_ticks_per_orbit);
    qscat.cds.CmdOrbitTicksPerOrbit(orbit_ticks_per_orbit);

    //----------------------------//
    // select encoder information //
    //----------------------------//

    unsigned int encoder_offset_dn;
    switch (qscat.sas.encoderElectronics)
    {
        case ENCODER_A:
            encoder_offset_dn = CDS_ENCODER_A_OFFSET;
            break;
        case ENCODER_B:
            encoder_offset_dn = CDS_ENCODER_B_OFFSET;
            break;
        default:
            fprintf(stderr, "%s: unknown encoder electronics\n", command);
            exit(1);
    }
    double encoder_offset = (double)encoder_offset_dn * two_pi /
        (double)ENCODER_N;

    //-------------------------------------------//
    // determine spin rate in radians per second //
    //-------------------------------------------//

    double assumed_spin_rate = qscat.cds.GetAssumedSpinRate();
    assumed_spin_rate *= rpm_to_radps;

    //--------------------//
    // step through beams //
    //--------------------//

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

        //------------------------//
        // allocate range tracker //
        //------------------------//

        qscat.cds.currentBeamIdx = beam_idx;
        Beam* beam = qscat.GetCurrentBeam();
        CdsBeamInfo* cds_beam_info = qscat.GetCurrentCdsBeamInfo();
        if (! cds_beam_info->rangeTracker.Allocate(RANGE_ORBIT_STEPS))
        {
            fprintf(stderr, "%s: error allocating range tracker\n", command);
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
            fprintf(stderr, "%s: error initializing QSCAT simulator\n",
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

        for (int orbit_step = 0; orbit_step < RANGE_ORBIT_STEPS; orbit_step++)
        {
            //--------------------//
            // calculate the time //
            //--------------------//

            // addition of 0.5 centers s/c on orbit_step
            double time = start_time +
                orbit_step_size * ((double)orbit_step + 0.5);

            //-----------------------//
            // locate the spacecraft //
            //-----------------------//

            spacecraft_sim.UpdateOrbit(time, &spacecraft);

            //----------------------//
            // step through azimuth //
            //----------------------//

            double rtt[RANGE_AZIMUTH_STEPS];

            for (int azimuth_step = 0; azimuth_step < RANGE_AZIMUTH_STEPS;
                azimuth_step++)
            {
                //--------------------------------//
                // calculate azimuth angle to use //
                //--------------------------------//

                // The table needs to be built for the CDS algorithm,
                // but we need to determine the actual antenna azimuth
                // angle.  The following code starts from the CDS azimuth,
                // backtracks to the original sampled encoder and then
                // calculates the actual antenna azimuth at the ground
                // impact time.  This method of doing the calculation will
                // allow for things like changes in the antenna spin rate
                // which will affect the actual antenna azimuth but not
                // the CDS estimation (which uses hardcoded spin rates).

                // start with the azimuth angle to be used by the CDS
                double azimuth = azimuth_step_size * (double)azimuth_step;

                // set the antenna azimuth for calculating the centering offset
                qscat.sas.antenna.SetEncoderAzimuthAngle(azimuth);

                // subtract the beam offset
                azimuth -= cds_beam_offset;

                // subtract an estimate of the centering offset
                // uses an estimate of the round trip time as
                // the previous pulses round trip time
                OrbitState* orbit_state = &(spacecraft.orbitState);
                Attitude* attitude = &(spacecraft.attitude);
                Antenna* antenna = &(qscat.sas.antenna);
                CoordinateSwitch antenna_frame_to_gc =
                    AntennaFrameToGC(orbit_state, attitude, antenna, azimuth);
                double look, az;
                GetPeakSpatialResponse2(&antenna_frame_to_gc, &spacecraft,
                    beam, antenna->spinRate, &look, &az);
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

                // subtract the encoder offset
                azimuth -= encoder_offset;

                // subtract the internal (sampling) delay angle
                double cds_pri = (double)qscat.cds.priDn / 10.0;
                azimuth -= (cds_pri * assumed_spin_rate);

                // add the actual sampling delay angle
                azimuth += (qscat.ses.pri * qscat.sas.antenna.spinRate);

                // and get to the center of the transmit pulse so that
                // the two-way gain product is formed correctly

                azimuth += (qscat.ses.txPulseWidth *
                    qscat.sas.antenna.spinRate / 2.0);

                //-------------------------//
                // set the antenna azimuth //
                //-------------------------//

                qscat.sas.antenna.SetEncoderAzimuthAngle(azimuth);
                qscat.SetOtherAzimuths(&spacecraft);

                //-------------------------------------------//
                // calculate the ideal round trip time in ms //
                //-------------------------------------------//
                // for an explanation of why T_GRID and T_RC are
                // here, check with Rod's memo

                rtt[azimuth_step] = (qscat.IdealRtt(&spacecraft) + T_GRID +
                    T_RC) * S_TO_MS;
            }

            //--------------------//
            // fit rtt parameters //
            //--------------------//

            double a, p, c;
            azimuth_fit(RANGE_AZIMUTH_STEPS, rtt, &a, &p, &c);

            if (opt_fixed)
            {
                // zero the amplitude and the phase
                a = 0.0;
                p = 0.0;
            }

            *(*(terms + orbit_step) + 0) = a;
            *(*(terms + orbit_step) + 1) = p;
            *(*(terms + orbit_step) + 2) = c;
        }

        //-----------//
        // set delay //
        //-----------//

        cds_beam_info->rangeTracker.SetRoundTripTime(terms);

        //---------------------------------------//
        // write out the receiver gate constants //
        //---------------------------------------//

        char filename[1024];
        sprintf(filename, "%s.%d", rgc_base, beam_idx + 1);
        if (! cds_beam_info->rangeTracker.WriteBinary(filename))
        {
            fprintf(stderr, "%s: error writing RGC file %s\n", command,
                filename);
            exit(1);
        }
    }

    //----------------//
    // free the array //
    //----------------//

    free_array((void *)terms, 2, RANGE_ORBIT_STEPS, 3);

    return (0);
}
