//=========================================================//
// Copyright (C) 1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    echo_tracker
//
// SYNOPSIS
//    echo_tracker <sim_config_file> <output_file>
//
// DESCRIPTION
//    Calculated the predicted and measured baseband frequency of the
//    peak two-way antenna pattern as a function of orbit time and
//    azimuth angle.
//
// OPTIONS
//    None.
//
// OPERANDS
//    The following operands are supported:
//    <sim_config_file>    The simulation configuration file.
//    <output_file>    The output filename.
//
// EXAMPLES
//    An example of a command line is:
//    % echo_tracker sws1b.cfg et.out
//
// ENVIRONMENT
//    Not environment dependent.
//
// EXIT STATUS
//    The following exit values are returned:
//    0    Program executed successfully
//    >0    Program had an error
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
#include <stdlib.h>
#include "Misc.h"
#include "ConfigList.h"
#include "L1A.h"
#include "ConfigSim.h"
#include "InstrumentGeom.h"
#include "Matrix.h"
#include "Ephemeris.h"
#include "List.h"
#include "List.C"
#include "Tracking.h"
#include "Tracking.C"
#include "BufferedList.h"
#include "BufferedList.C"

//-----------//
// TEMPLATES //
//-----------//

template class List<Meas>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;
template class List<WindVectorPlus>;
template class List<MeasSpot>;
template class List<long>;
template class List<OffsetList>;
template class List<StringPair>;
template class List<OrbitState>;
template class BufferedList<OrbitState>;
template class List<EarthPosition>;

//-----------//
// CONSTANTS //
//-----------//

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

const char* usage_array[] = { "<sim_config_file>", "<output_file>", 0};

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
        fprintf(stderr, "%s: error reading sim config file %s\n", command,
        config_file);
        exit(1);
    }

    //----------------------------------//
    // create and configure L1A product //
    //----------------------------------//

    L1A l1a;
    if (! ConfigL1A(&l1a, &config_list))
    {
        fprintf(stderr, "%s: error configuring Level 1A Product\n", command);
        exit(1);
    }
    L1AFrame* frame = &(l1a.frame);

    //---------------------------------//
    // create and configure spacecraft //
    //---------------------------------//

    Spacecraft spacecraft;
    if (! ConfigSpacecraft(&spacecraft, &config_list))
    {
        fprintf(stderr, "%s: error configuring spacecraft\n", command);
        exit(1);
    }

    //---------------------------------//
    // create and configure instrument //
    //---------------------------------//

    Instrument instrument;
    if (! ConfigInstrument(&instrument, &config_list))
    {
        fprintf(stderr, "%s: error configuring instrument\n", command);
        exit(1);
    }
    Antenna* antenna = &(instrument.antenna);
    double Gn = instrument.noise_receiverGain;
    double Bn = instrument.noiseBandwidth;
    double Ge = instrument.echo_receiverGain;
    double Be = instrument.GetTotalSignalBandwidth();
    double Bs = instrument.scienceSliceBandwidth;

    float signal_energy[10];    // technically, this should be dynamic
    float slice_number[10] = { 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0,
        9.0 };    // and so should this

    //--------------------------------//
    // create and configure ephemeris //
    //--------------------------------//

    Ephemeris ephemeris;
    if (! ConfigEphemeris(&ephemeris, &config_list))
    {
        fprintf(stderr, "%s: error configuring ephemeris\n", command);
        exit(1);
    }

    //------------//
    // open files //
    //------------//

    l1a.OpenForReading();
    FILE* output_fp = fopen(output_file, "w");
    if (output_fp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            output_file);
        exit(1);
    }

    //-----------------//
    // conversion loop //
    //-----------------//

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

        float roll = l1a.frame.attitude.GetRoll();
        float pitch = l1a.frame.attitude.GetPitch();
        float yaw = l1a.frame.attitude.GetYaw();
        spacecraft.attitude.SetRPY(roll, pitch, yaw);

        unsigned int orbit_ticks = frame->orbitTicks;

        //--------------------------//
        // loop for each beam cycle //
        //--------------------------//

        for (int beam_cycle = 0; beam_cycle < frame->antennaCyclesPerFrame;
            beam_cycle++)
        {
            //--------------------//
            // loop for each beam //
            //--------------------//
    
            for (int beam_idx = 0; beam_idx < antenna->numberOfBeams;
                beam_idx++)
            {
                int spot_idx = beam_cycle * antenna->numberOfBeams + beam_idx;

                int base_slice_idx = spot_idx * frame->slicesPerSpot;
                antenna->currentBeamIdx = beam_idx;
                Beam* beam = antenna->GetCurrentBeam();

                if (spot_idx == frame->priOfOrbitTickChange)
                    orbit_ticks++;
                unsigned int orbit_step =
                    beam->dopplerTracker.OrbitTicksToStep(orbit_ticks,
                    instrument.orbitTicksPerOrbit);
    
                //-------------------------//
                // calculate the spot time //
                //-------------------------//
    
                double time = frame->time + beam_cycle * antenna->priPerBeam +
                    beam->timeOffset;

                //---------------------------//
                // calculate the orbit state //
                //---------------------------//
 
                if (! ephemeris.GetOrbitState(time, EPHEMERIS_INTERP_ORDER,
                    &(spacecraft.orbitState)))
                {
                    return(0);
                }
 
                //--------------------------------------//
                // set up the spacecraft and instrument //
                //--------------------------------------//
 
                antenna->SetAzimuthWithEncoder(
                    (unsigned int)frame->antennaPosition[spot_idx]);
                instrument.orbitTicks = beam->rangeTracker.OrbitStepToTicks(
                    orbit_step, instrument.orbitTicksPerOrbit);
 
                SetRangeAndDoppler(&spacecraft, &instrument);

                //-------------------------------------------//
                // calculate the expected baseband frequency //
                //-------------------------------------------//
                // baseband frequency using tracking constants and attitude
 
                Attitude* attitude = &(spacecraft.attitude);
                CoordinateSwitch antenna_frame_to_gc =
                    AntennaFrameToGC(&(spacecraft.orbitState), attitude,
                    antenna);
 
                double center_look, center_azim;
                if (! GetTwoWayPeakGain2(&antenna_frame_to_gc, &spacecraft,
                    beam, instrument.antenna.actualSpinRate, &center_look,
                    &center_azim))
                {
                    return(0);
                }
 
                Vector3 vector;
                vector.SphericalSet(1.0, center_look, center_azim);
                TargetInfoPackage tip;
                if (! TargetInfo(&antenna_frame_to_gc, &spacecraft,
                    &instrument, vector, &tip))
                {
                    return(0);
                }
                float f_bb_expected = tip.basebandFreq;
 
                //-----------------------//
                // get the azimuth angle //
                //-----------------------//
 
                unsigned int encoder =
                    frame->antennaPosition[spot_idx];

                //----------------------------------//
                // calculate the signal only energy //
                //----------------------------------//
 
                double En = frame->spotNoise[spot_idx];
                double Es = 0.0;
                for (int slice_idx = 0; slice_idx < frame->slicesPerSpot;
                    slice_idx++)
                {
                    Es += frame->science[base_slice_idx + slice_idx];
                }
 
                double npsd = ((En / (Gn * Bn)) - (Es / (Ge * Bn))) /
                    (1.0 - Be / Bn);
 
                for (int slice_idx = 0; slice_idx < frame->slicesPerSpot;
                    slice_idx++)
                {
                    signal_energy[slice_idx] =
                        frame->science[base_slice_idx + slice_idx] -
                        Ge * Bs * npsd;
                }
 
                //----------------------------------//
                // fit a quadratic to find the peak //
                //----------------------------------//
 
                Matrix u, v;
                Vector w, coefs;
                coefs.Allocate(3);
                u.SVDFit(slice_number, signal_energy, NULL,
                    frame->slicesPerSpot, &coefs, 3, &u, &v, &w);

                double c[3];
                coefs.GetElement(0, &(c[0]));
                coefs.GetElement(1, &(c[1]));
                coefs.GetElement(2, &(c[2]));
                float peak_slice = -c[1] / (2.0 * c[2]);
                if (peak_slice < 0.0 || peak_slice > frame->slicesPerSpot)
                    continue;
 
                int near_slice_idx = (int)(peak_slice + 0.5);
                float f1, bw;
                instrument.GetSliceFreqBw(near_slice_idx, &f1, &bw);
                float f_bb_data = f1 + bw * (peak_slice -
                    (float)near_slice_idx + 0.5);
 
                //--------//
                // output //
                //--------//

                fwrite((void *)&time, sizeof(double), 1, output_fp);
                fwrite((void *)&orbit_step, sizeof(unsigned int), 1,
                    output_fp);
                spacecraft.orbitState.Write(output_fp);
                attitude->Write(output_fp);
                fwrite((void *)&encoder, sizeof(unsigned int), 1, output_fp);
                fwrite((void *)&beam_idx, sizeof(int), 1, output_fp);
                fwrite((void *)&(instrument.commandedDoppler), sizeof(float),
                    1, output_fp);
                fwrite((void *)&(instrument.commandedRxGateDelay),
                    sizeof(float), 1, output_fp);
                fwrite((void *)&(instrument.commandedRxGateWidth),
                    sizeof(float), 1, output_fp);
                fwrite((void *)&f_bb_expected, sizeof(float), 1, output_fp);
                fwrite((void *)&f_bb_data, sizeof(float), 1, output_fp);
            }
        }
    } while (1);

    l1a.Close();
    fclose(output_fp);
    return (0);
}
