//=========================================================//
// Copyright (C) 1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    l1a_echo_proc
//
// SYNOPSIS
//    l1a_echo_proc <sim_config_file> <echo_data_file>
//
// DESCRIPTION
//    Reads the simulated L1A file and estimates frequency offsets
//    (from 0 kHz baseband) of the spectral response of the echo.
//
// OPERANDS
//    The following operands are supported:
//      <sim_config_file>  The simulation configuration file.
//      <echo_data_file>   The echo data output file.
//
// EXAMPLES
//    An example of a command line is:
//      % l1a_echo_proc qscat.cfg echo.dat
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
#include "Tracking.h"
#include "BufferedList.h"
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

#define KM_RANGE  100.0
#define POINTS    4

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

const char* usage_array[] = { "<sim_config_file>", "<echo_data_file>", 0};

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
    const char* output_file = argv[clidx++];

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

    int ofd = creat(output_file, 0644);
    if (ofd == -1)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            output_file);
        exit(1);
    }

    //-------------------//
    // step through data //
    //-------------------//

    int last_orbit_step = -1;
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

        //-----------------------------//
        // write out an ephemeris line //
        //-----------------------------//

        write_ephemeris(ofd, frame->gcX, frame->gcY, frame->gcZ, frame->velX,
            frame->velY, frame->velZ, roll, pitch, yaw);

        //------------------------------//
        // write out an orbit time line //
        //------------------------------//

        write_orbit_time(ofd, frame->orbitTicks);

        //--------------------//
        // step through spots //
        //--------------------//

        for (int spot_idx = 0; spot_idx < frame->spotsPerFrame; spot_idx++)
        {
            // determine the spot time
//            double time = frame->time + spot_idx * qscat.ses.pri;

            // determine beam and beam index
            int beam_idx = spot_idx % NUMBER_OF_QSCAT_BEAMS;
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

            //----------------------//
            // write out orbit step //
            //----------------------//

            if ((int)orbit_step != last_orbit_step)
            {
                write_orbit_step(ofd, orbit_step);
                last_orbit_step = orbit_step;
            }

            // set the encoder for tracking
            unsigned short held_encoder = *(frame->antennaPosition + spot_idx);
            qscat.cds.heldEncoder = held_encoder;

            // get the ideal encoder for output
            unsigned short ideal_encoder = qscat.cds.EstimateIdealEncoder();

            //-------------------------------//
            // do range and Doppler tracking //
            //-------------------------------//

//            qscat.cds.useRgc = 1;
//            qscat.cds.useDtc = 1;

            qscat.SetEncoderAzimuth(held_encoder, 1);
            qscat.SetOtherAzimuths(&spacecraft);

            SetDelayAndFrequency(&spacecraft, &qscat);

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
            int land_flag = 0;
            for (int i = 0; i < 4; i++)
            {
                LonLat use_lon_lat;
                use_lon_lat.Set(lon, lat);
                use_lon_lat.ApproxApplyDelta(dlon_km[i], dlat_km[i]);
                if (qscat_sim.landMap.IsLand(&use_lon_lat))
                {
                    land_flag = 1;
                    break;
                }
            }

            //----------------------//
            // threshold spot power //
            //----------------------//

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

            //---------------//
            // find the peak //
            //---------------//

            float meas_spec_peak_slice, meas_spec_peak_freq;
            if (! find_peak(&qscat, slice_number, signal_energy,
                frame->slicesPerSpot, &meas_spec_peak_slice,
                &meas_spec_peak_freq))
            {
                continue;
            }

/*
static int zz = 0;
char fn[1024];
if (ideal_encoder > 3641 && ideal_encoder < 4551)
{
sprintf(fn, "xxx.%05d.%d", zz, beam_idx);
FILE* fp = fopen(fn, "w");
for (int i = 0; i < 10; i++)
{
//  fprintf(fp, "%g %g %g %g\n", slice_number[i], signal_energy[i],
//    c[2]*i*i + c[1]*i + c[0], frame->science[base_slice_idx + i]);
  fprintf(fp, "%g %g %g\n", slice_number[i], signal_energy[i],
    frame->science[base_slice_idx + i]);
}
fprintf(fp, "&\n");
fprintf(fp, "%g 0.0\n", meas_spec_peak_slice);
fclose(fp);
zz++;
}
*/
            //-------//
            // write //
            //-------//

            write_spot(ofd, beam_idx, qscat.ses.txDoppler,
                qscat.ses.rxGateDelay, ideal_encoder,
                qscat.sas.antenna.txCenterAzimuthAngle, meas_spec_peak_freq,
                total_signal_energy, land_flag);
        }

        in_first_frame = 0;

    } while (1);

    l1a.Close();

    return (0);
}
