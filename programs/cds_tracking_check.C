//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    rgc_delay_errors
//
// SYNOPSIS
//    rgc_delay_errors <sim_config_file> <rgc_errs> <dtc_errs>
//
// DESCRIPTION
//    Generates plottable error files between the actual delay and the
//    delay calculated by the RGC.
//
// OPTIONS
//    None.
//
// OPERANDS
//    The following operands are supported:
//      <sim_config_file>  The sim_config_file needed listing all
//                           input parameters, input files, and output
//                           files.
//
//      <rgc_errs>         A plottable error file for the RGC.
//
//      <dtc_errs>         A plottable error file for the DTC.
//
// EXAMPLES
//    An example of a command line is:
//      % constant_errors sws1b.cfg tlm.dat rgc.errs dtc.errs
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
#include "Misc.h"
#include "ConfigList.h"
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

//-----------//
// CONSTANTS //
//-----------//

#define EQX_TIME_TOLERANCE  0.1

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

const char* usage_array[] = { "<sim_config_file>", "<tlm_file>", "<rgc_errs>",
    "<dtc_errs>", 0};

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

    if (argc != 5)
        usage(command, usage_array, 1);

    int arg_idx = 1;
    const char* config_file = argv[arg_idx++];
    const char* tlm_file = argv[arg_idx++];
    const char* rgc_err_file = argv[arg_idx++];
    const char* dtc_err_file = argv[arg_idx++];

    //--------------------------------//
    // read in simulation config file //
    //--------------------------------//

    ConfigList config_list;
    if (! config_list.Read(config_file))
    {
        fprintf(stderr, "%s: error reading sim config file %s\n", command,
            config_file);
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

    //----------------------//
    // open the output file //
    //----------------------//

    FILE* rgc_err_fp = fopen(rgc_err_file, "w");
    if (rgc_err_fp == NULL)
    {
        fprintf(stderr, "%s: error opening RGC error file %s\n", command,
            rgc_err_file);
        exit(1);
    }

    FILE* dtc_err_fp = fopen(dtc_err_file, "w");
    if (dtc_err_fp == NULL)
    {
        fprintf(stderr, "%s: error opening DTC error file %s\n", command,
            dtc_err_file);
        exit(1);
    }

    FILE* ifp = fopen(tlm_file, "r");
    if (ifp == NULL)
    {
        fprintf(stderr, "%s: error opening tlm file %s\n", command,
            tlm_file);
        exit(1);
    }

    for (int frame_idx = 0; ;frame_idx++)
    {
        //--------------//
        // read in data //
        //--------------//

        int raw_orbit_step, step_change;
        int retval = fscanf(ifp, " %d %d", &raw_orbit_step, &step_change);
        if (retval == -1)
            break;

        int encoder[100];
        for (int i = 0; i < 100; i++)
        {
            fscanf(ifp, " %d", &(encoder[i]));
        }

        float delay_1, delay_2, doppler_1, doppler_2;
        fscanf(ifp, " %f %f %f %f", &delay_1, &delay_2, &doppler_1, &doppler_2);

        unsigned char delay_1_dn = (unsigned char)(delay_1 / 1000.0 /
            RX_GATE_DELAY_CMD_RESOLUTION + 0.5);
        unsigned char delay_2_dn = (unsigned char)(delay_2 / 1000.0 /
            RX_GATE_DELAY_CMD_RESOLUTION + 0.5);
        short doppler_1_dn = (short)(doppler_1) / 2;
        short doppler_2_dn = (short)(doppler_2) / 2;

		//---------//
		// process //
		//---------//

        for (int i = 0; i < 100; i++)
        {
            qscat.cds.currentBeamIdx = i % 2;
            CdsBeamInfo* cds_beam_info = qscat.GetCurrentCdsBeamInfo();
            RangeTracker* range_tracker = &(cds_beam_info->rangeTracker);
            DopplerTracker* doppler_tracker = &(cds_beam_info->dopplerTracker);

            unsigned short orbit_step;
            if (step_change != 255 && i + 1 < step_change)
            {
                if (raw_orbit_step == 0)
                    orbit_step = 255;
                else
                    orbit_step = raw_orbit_step - 1;
            }
            else
                orbit_step = raw_orbit_step;

            //-------------------------//
            // calculate the RGC delay //
            //-------------------------//

            qscat.cds.previousEncoder = encoder[i];
            unsigned short use_encoder = qscat.cds.EstimateEncoder();

            unsigned char rx_gate_delay_dn;
            float rx_gate_delay_fdn;
            range_tracker->GetRxGateDelay(orbit_step, use_encoder,
                cds_beam_info->rxGateWidthDn, qscat.cds.txPulseWidthDn,
                &rx_gate_delay_dn, &rx_gate_delay_fdn);

            //-------------------------------//
            // calculate the Doppler command //
            //-------------------------------//

            short doppler_dn;
            doppler_tracker->GetCommandedDoppler(orbit_step, use_encoder,
                rx_gate_delay_dn, rx_gate_delay_fdn, &doppler_dn);

            int delay_dif, freq_dif;
            switch (i)
            {
                case 0:
                    delay_dif = (int)rx_gate_delay_dn - (int)delay_1_dn;
                    fprintf(rgc_err_fp, "%d %d %d %d\n", frame_idx, delay_dif,
                        rx_gate_delay_dn, delay_1_dn);

                    freq_dif = (int)doppler_dn - (int)doppler_1_dn;
                    fprintf(dtc_err_fp, "%d %d %d %d\n", frame_idx, freq_dif,
                        doppler_dn, doppler_1_dn);
                    break;
                case 1:
                    delay_dif = (int)rx_gate_delay_dn - (int)delay_2_dn;
                    fprintf(rgc_err_fp, "%d %d %d %d\n", frame_idx, delay_dif,
                        rx_gate_delay_dn, delay_2_dn);

                    freq_dif = (int)doppler_dn - (int)doppler_2_dn;
                    fprintf(dtc_err_fp, "%d %d %d %d\n", frame_idx, freq_dif,
                        doppler_dn, doppler_2_dn);
                    break;
                default:
//                    printf("%d\n", rx_gate_delay_dn);
                    break;
            }
		}
	};

	//-------------------//
	// close error files //
	//-------------------//

	fclose(rgc_err_fp);
	fclose(dtc_err_fp);
	fclose(ifp);

	return (0);
}
