//=========================================================//
// Copyright (C) 1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    analyze_et
//
// SYNOPSIS
//    analyze_et <sim_config_file> <echo_tracker_file> <output_file>
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
//    <echo_tracker_file>  The output of echo_tracker.
//    <output_file>    The output filename.
//
// EXAMPLES
//    An example of a command line is:
//    % analyze_et sws1b.cfg et.out xxx.out
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
#include "Spacecraft.h"
#include "Instrument.h"
#include "List.h"
#include "List.C"
#include "Tracking.h"
#include "Tracking.C"
#include "BufferedList.h"
#include "BufferedList.C"

//-----------//
// TEMPLATES //
//-----------//

template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;
template class List<StringPair>;
template class List<OrbitState>;
template class List<EarthPosition>;
template class BufferedList<OrbitState>;

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

const char* usage_array[] = { "<sim_config_file>", "<echo_tracker_output>",
    "<output_file>", 0};

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
    if (argc != 4)
        usage(command, usage_array, 1);

    int clidx = 1;
    const char* config_file = argv[clidx++];
    const char* echo_tracker_file = argv[clidx++];
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

/*
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
*/

    //------------//
    // open files //
    //------------//

    FILE* input_fp = fopen(echo_tracker_file, "r");
    if (input_fp == NULL)
    {
        fprintf(stderr, "%s: error opening input file %s\n", command,
            echo_tracker_file);
        exit(1);
    }

    char output_filename[2][1024];
    FILE* output_fp[2];
    for (int i = 0; i < 2; i++)
    {
        sprintf(output_filename[i], "%s.%d", output_file, i);
        output_fp[i] = fopen(output_filename[i], "w");
        if (output_fp == NULL)
        {
            fprintf(stderr, "%s: error opening output file %s\n", command,
                output_filename[i]);
            exit(1);
        }
    }

    //---------//
    // buffers //
    //---------//

    struct SpotTrack {
        double time;
        OrbitState orbit_state;
        Attitude attitude;
        unsigned int encoder;
        int beam_idx;
        float commanded_doppler;
        float commanded_rx_gate_delay;
        float commanded_rx_gate_width;
        float f_bb_expected;
        float f_bb_data;
    };
    SpotTrack spot_track[SPOTS];

    //-----------------//
    // conversion loop //
    //-----------------//

    Spacecraft spacecraft;
    Instrument instrument;
    Antenna* antenna = &(instrument.antenna);
    unsigned int encoder;
    int beam_idx;
    float f_bb_data, f_bb_expected;

    int spot_idx = 0;
    unsigned int last_orbit_step = -1;
    int done = 0;
    do
    {
        //----------------------//
        // check the spot index //
        //----------------------//

        if (spot_idx >= SPOTS)
        {
            fprintf(stderr, "%s: too many spots\n", command);
            exit(1);
        }

        //-----------------------------//
        // read an echo tracker record //
        //-----------------------------//

        SpotTrack* st = &(spot_track[spot_idx]);
        if (fread((void *)&(st->time), sizeof(double), 1, input_fp) != 1 ||
            fread((void *)&(st->orbit_step), sizeof(unsigned int), 1,
                input_fp) != 1 ||
            ! st->orbit_state.Read(input_fp) ||
            ! st->attitude.Read(input_fp) ||
            fread((void *)&(st->encoder), sizeof(unsigned int), 1,
                input_fp) != 1 ||
            fread((void *)&(st->beam_idx), sizeof(int), 1, input_fp) != 1 ||
            fread((void *)&(st->commanded_doppler), sizeof(float), 1,
                input_fp) != 1 ||
            fread((void *)&(st->commanded_rx_gate_delay), sizeof(float), 1,
                input_fp) != 1 ||
            fread((void *)&(st->commanded_rx_gate_width), sizeof(float), 1,
                input_fp) != 1 ||
            fread((void *)&(st->f_bb_expected), sizeof(float), 1,
                input_fp) != 1 ||
            fread((void *)&(st->f_bb_data), sizeof(float), 1, input_fp) != 1)
        {
            done = 1;
        }

        //-----------------------------//
        // check for end of orbit step //
        //-----------------------------//

        if (done ||
            (st->orbit_step != last_orbit_step && last_orbit_step != -1))
        {
            Attitude attitude = estimate_attitude();
            
            fprintf(output_fp, "%d %g %g %g\n", orbit_step, roll*rtd,
                pitch*rtd, yaw*rtd);
            last_orbit_step = st->orbit_step;
            spot_idx = 0;
        }

        //---------------------------//
        // write to the output files //
        //---------------------------//

        fprintf(output_fp[beam_idx], "%g %g %g\n", time, f_bb_expected,
            f_bb_data);

/*
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
                double f_bb_expected = tip.basebandFreq;
 
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

                spacecraft.orbitState.Write(output_fp);
                fwrite((void *)instrument.commandedDoppler, sizeof(float), 1,
                    output_fp);
                fwrite((void *)instrument.commandedRxGateDelay, sizeof(float),
                    1, output_fp);
                fwrite((void *)instrument.commandedRxGateWidth, sizeof(float),
                    1, output_fp);
                attitude.Write(output_fp);
                fwrite((void *)&f_bb_expected, sizeof(float), 1, output_fp);
                fwrite((void *)&f_bb_data, sizeof(float), 1, output_fp);

            }
        }
*/
    } while (! done);

    //-------------//
    // close files //
    //-------------//

    for (int i = 0; i < 2; i++)
    {
        fclose(output_fp[i]);
    }
    fclose(input_fp);

    return (0);
}

Attitude
estimate_attitude(
    int         spot_count,
    SpotTrack*  spot_track)
{
    Instrument instrument;
    Spacecraft spacecraft;
    for (int spot_idx = 0; spot_idx < spot_count; spot_idx++)
    {
        spacecraft.orbitState = spot_track[spot_idx].orbit_state;
        spacecraft.attitude = spot_track[spot_idx].attitude;
        unsigned int encoder = spot_track[spot_idx].encoder;
            fread((void *)&encoder, sizeof(unsigned int), 1, input_fp) != 1 ||
            fread((void *)&beam_idx, sizeof(int), 1, input_fp) != 1 ||
            fread((void *)&(instrument.commandedDoppler), sizeof(float), 1,
                input_fp) != 1 ||
            fread((void *)&(instrument.commandedRxGateDelay), sizeof(float), 1,
                input_fp) != 1 ||
            fread((void *)&(instrument.commandedRxGateWidth), sizeof(float), 1,
                input_fp) != 1 ||
            fread((void *)&f_bb_expected, sizeof(float), 1, input_fp) != 1 ||
            fread((void *)&f_bb_data, sizeof(float), 1, input_fp) != 1)
        {
            break;
        }

        //----------------//
        // set up antenna //
        //----------------//

        antenna->SetAzimuthWithEncoder(encoder);
        
    }
}
