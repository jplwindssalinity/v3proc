//=========================================================//
// Copyright (C) 1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    sim_l1a_echo_proc
//
// SYNOPSIS
//    sim_l1a_echo_proc <sim_config_file> <output_echo_file>
//
// DESCRIPTION
//    Reads a simulated L1A file and estimates frequency offsets (from
//    0 kHz baseband) of the spectral response of the echo by fitting
//    a gaussian to the echo energies.
//
// OPERANDS
//    The following operands are supported:
//      <sim_config_file>   The simulation configuration file.
//      <output_echo_file>  The output echo file name.
//
// EXAMPLES
//    An example of a command line is:
//      % sim_l1a_echo_proc qscat.cfg qscat.echo
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
#include <fcntl.h>
#include "Misc.h"
#include "ConfigList.h"
#include "L1A.h"
#include "ConfigSim.h"
#include "QscatConfig.h"
#include "InstrumentGeom.h"
#include "AccurateGeom.h"
#include "List.h"
#include "List.C"
#include "Tracking.h"
#include "Tracking.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "AngleInterval.h"
#include "echo_funcs.h"

//-----------//
// TEMPLATES //
//-----------//

template class List<Meas>;
template class List<long>;
template class List<WindVectorPlus>;
template class List<MeasSpot>;
template class List<OffsetList>;
template class List<StringPair>;
template class TrackerBase<unsigned char>;
template class List<OrbitState>;
template class BufferedList<OrbitState>;
template class List<EarthPosition>;
template class TrackerBase<unsigned short>;
template class List<AngleInterval>;

//-----------//
// CONSTANTS //
//-----------//

// if data is within KM_RANGE of land, flag it as land
#define KM_RANGE  100.0

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

const char* usage_array[] = { "<sim_config_file>", "<output_echo_file>", 0};

// approximate ranges used for land flag check
float dlon_km[4] = { KM_RANGE, 0.0, -KM_RANGE, 0.0 };
float dlat_km[4] = { 0.0, KM_RANGE, 0.0, -KM_RANGE };

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
    if (argc != 3)
        usage(command, usage_array, 1);

    int clidx = 1;
    const char* config_file = argv[clidx++];
    const char* output_echo_file = argv[clidx++];

    //---------------------//
    // read in config file //
    //---------------------//

    ConfigList config_list;
    if (! config_list.Read(config_file))
    {
        fprintf(stderr, "%s: error reading configuration file %s\n",
            command, config_file);
        exit(1);
    }

    //---------------------------------------//
    // create and configure Level 1A product //
    //---------------------------------------//

    L1A l1a;
    if (! ConfigL1A(&l1a, &config_list))
    {
        fprintf(stderr, "%s: error configuring Level 1A Product\n", command);
        exit(1);
    }

    //---------------------------------//
    // create and configure spacecraft //
    //---------------------------------//

    Spacecraft spacecraft;
    if (! ConfigSpacecraft(&spacecraft, &config_list))
    {
        fprintf(stderr, "%s: error configuring spacecraft\n", command);
        exit(1);
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

    //-----------//
    // variables //
    //-----------//

    double signal_energy[10];
    double slice_number[10] = { 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0,
        9.0 };

    //-----------//
    // predigest //
    //-----------//

    L1AFrame* frame = &(l1a.frame);
    Antenna* antenna = &(qscat.sas.antenna);
    double Bn = qscat.ses.noiseBandwidth;
    double Be = qscat.ses.GetTotalSignalBandwidth();
    double Bs = qscat.ses.scienceSliceBandwidth;

    //--------------------//
    // open Level 1A file //
    //--------------------//

    if (! l1a.OpenForReading())
    {
        fprintf(stderr, "%s: error opening L1A file\n", command);
        exit(1);
    }

    //------------------//
    // open output file //
    //------------------//

    int ofd = creat(output_echo_file, 0644);
    if (ofd == -1)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            output_echo_file);
        exit(1);
    }

    //-------------------//
    // step through data //
    //-------------------//

    EchoInfo echo_info;

    int in_first_frame = 1;
    do
    {
        //-----------------------------//
        // read a level 1A data record //
        //-----------------------------//

        if (! l1a.ReadDataRec())
        {
            switch (l1a.GetStatus())
            {
            case L1A::OK:    // end of file
                break;
            case L1A::ERROR_READING_FRAME:
                fprintf(stderr, "%s: error reading Level 1A data\n", command);
                exit(1);
                break;
            case L1A::ERROR_UNKNOWN:
                fprintf(stderr, "%s: unknown error reading Level 1A data\n",
                    command);
                exit(1);
                break;
            default:
                fprintf(stderr, "%s: unknown status (???)\n", command);
                exit(1);
            }
            break;    // done, exit do loop
        }

        //--------//
        // unpack //
        //--------//

        frame->Unpack(l1a.buffer);

        //--------------------//
        // set the spacecraft //
        //--------------------//

        spacecraft.orbitState.rsat.Set(frame->gcX, frame->gcY, frame->gcZ);
        spacecraft.orbitState.vsat.Set(frame->velX, frame->velY, frame->velZ);

        Attitude* attitude = &(frame->attitude);
        float roll = attitude->GetRoll();
        float pitch = attitude->GetPitch();
        float yaw = attitude->GetYaw();
        spacecraft.attitude.SetRPY(roll, pitch, yaw);

        //------------------------//
        // estimate the spin rate //
        //------------------------//

        unsigned int theta_max =
            *(frame->antennaPosition + frame->spotsPerFrame - 1);
        unsigned int theta_min =
            *(frame->antennaPosition + 0);
        while (theta_max < theta_min)
        {
            theta_max += ENCODER_N;
        }
        unsigned int delta_theta = theta_max - theta_min;
        double delta_theta_rad = two_pi * (double)delta_theta /
            (double)ENCODER_N;
        double delta_t = (double)(frame->spotsPerFrame - 1) * qscat.ses.pri;
        double omega = delta_theta_rad / delta_t;
        qscat.sas.antenna.spinRate = omega;

        //-----------------------------------------------//
        // set the ephemeris, orbit time, and orbit step //
        //-----------------------------------------------//

        echo_info.gcX = frame->gcX;
        echo_info.gcY = frame->gcY;
        echo_info.gcZ = frame->gcZ;
        echo_info.velX = frame->velX;
        echo_info.velY = frame->velY;
        echo_info.velZ = frame->velZ;
        echo_info.roll = roll;
        echo_info.pitch = pitch;
        echo_info.yaw = yaw;
        echo_info.orbitTicks = frame->orbitTicks;
        echo_info.orbitStep = frame->orbitStep;
        echo_info.priOfOrbitStepChange = frame->priOfOrbitStepChange;

        //--------------------//
        // step through spots //
        //--------------------//

        for (int spot_idx = 0; spot_idx < frame->spotsPerFrame; spot_idx++)
        {
            echo_info.flag[spot_idx] = EchoInfo::OK;

            // determine beam index and beam
            int beam_idx = spot_idx % NUMBER_OF_QSCAT_BEAMS;
            echo_info.beamIdx[spot_idx] = (unsigned char)beam_idx;
            qscat.cds.currentBeamIdx = beam_idx;
            Beam* beam = qscat.GetCurrentBeam();

            // determine starting slice index
            int base_slice_idx = spot_idx * frame->slicesPerSpot;

            // determine orbit step
            qscat.cds.orbitTime = frame->orbitTicks;
            unsigned int orbit_step = frame->orbitStep;
            if (frame->priOfOrbitStepChange != 255 &&
                spot_idx < frame->priOfOrbitStepChange)
            {
                if (orbit_step == 0)
                    orbit_step = ORBIT_STEPS - 1;
                else
                    orbit_step--;
            }
            qscat.cds.orbitStep = orbit_step;

            // set the encoder for tracking
            unsigned short held_encoder = *(frame->antennaPosition + spot_idx);
            qscat.cds.heldEncoder = held_encoder;

            // get the ideal encoder for output
            unsigned short ideal_encoder = qscat.cds.EstimateIdealEncoder();
            echo_info.idealEncoder[spot_idx] = ideal_encoder;

            //-------------------------------//
            // do range and Doppler tracking //
            //-------------------------------//

            qscat.SetEncoderAzimuth(held_encoder, 1);
            qscat.SetOtherAzimuths(&spacecraft);
            echo_info.txCenterAzimuthAngle[spot_idx] =
                qscat.sas.antenna.txCenterAzimuthAngle;

            SetDelayAndFrequency(&spacecraft, &qscat);

            echo_info.txDoppler[spot_idx] = qscat.ses.txDoppler;
            echo_info.rxGateDelay[spot_idx] = qscat.ses.rxGateDelay;

            //------------------------------//
            // skip pulses from first frame //
            //------------------------------//

            if (in_first_frame)
                continue;

            //-------------------------//
            // skip calibration pulses //
            //-------------------------//

            if (spot_idx == frame->calPosition - 2 ||
                spot_idx == frame->calPosition - 1)
            {
                echo_info.flag[spot_idx] = EchoInfo::CAL_OR_LOAD_PULSE;
                continue;
            }

            //-----------//
            // flag land //
            //-----------//

            CoordinateSwitch antenna_frame_to_gc =
                AntennaFrameToGC(&(spacecraft.orbitState), attitude, antenna,
                antenna->groundImpactAzimuthAngle);
            double look, azim;
            if (! beam->GetElectricalBoresight(&look, &azim))
                return(0);
            Vector3 vector;
            vector.SphericalSet(1.0, look, azim);
            QscatTargetInfo qti;
            if (! qscat.TargetInfo(&antenna_frame_to_gc, &spacecraft,
                vector, &qti))
            {
                fprintf(stderr, "%s: error finding target information\n",
                    command);
                exit(1);
            }
            double alt, lon, lat;
            if (! qti.rTarget.GetAltLonGCLat(&alt, &lon, &lat))
            {
                fprintf(stderr, "%s: error finding alt/lon/lat\n", command);
                exit(1);
            }
            for (int i = 0; i < 4; i++)
            {
                LonLat use_lon_lat;
                use_lon_lat.Set(lon, lat);
                use_lon_lat.ApproxApplyDelta(dlon_km[i], dlat_km[i]);
                if (qscat_sim.landMap.IsLand(&use_lon_lat))
                {
                    echo_info.flag[spot_idx] = EchoInfo::LAND;
                    break;
                }
            }

            //-------------------------------//
            // determine total signal energy //
            //-------------------------------//

            double En = frame->spotNoise[spot_idx];
            double Es = 0.0;
            for (int slice_idx = 0; slice_idx < frame->slicesPerSpot;
                slice_idx++)
            {
                Es += frame->science[base_slice_idx + slice_idx];
            }

            double rho = 1.0;
            double beta = qscat.ses.rxGainNoise / qscat.ses.rxGainEcho;
            double alpha = Bn/Be*beta;
            double noise = Bs / Be * (rho / beta * En - Es) /
                (alpha * rho / beta - 1.0);

            double total_signal_energy = 0.0;
            for (int slice_idx = 0; slice_idx < frame->slicesPerSpot;
                slice_idx++)
            {
                signal_energy[slice_idx] =
                    frame->science[base_slice_idx + slice_idx] -
                    noise;
                total_signal_energy += signal_energy[slice_idx];
            }
            echo_info.totalSignalEnergy[spot_idx] = total_signal_energy;

            //---------------//
            // find the peak //
            //---------------//

            float meas_spec_peak_slice, meas_spec_peak_freq;
            if (! gaussian_fit(&qscat, slice_number, signal_energy,
                frame->slicesPerSpot, &meas_spec_peak_slice,
                &meas_spec_peak_freq))
            {
                echo_info.flag[spot_idx] = EchoInfo::BAD_PEAK;
                continue;
            }
            echo_info.measSpecPeakFreq[spot_idx] = meas_spec_peak_freq;
        }

        //---------------------//
        // write out the frame //
        //---------------------//

        if (! echo_info.Write(ofd))
        {
            fprintf(stderr, "%s: error writing frame to output echo file %s\n",
                command, output_echo_file);
            exit(1);
        }

        in_first_frame = 0;

    } while (1);

    l1a.Close();
    close(ofd);

    return (0);
}
