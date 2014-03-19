//=========================================================//
// Copyright (C) 2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    check_inst_tracking
//
// SYNOPSIS
//    check_inst_tracking <configfile> <outfile> <L1A_file...>
//
// DESCRIPTION
// This program reads a L1A HDF file and then checks to see if the 
// range delays and doppler frequencies stored in the file agree with
// the our calculations of these quantities.
//
//
// OPERANDS
//    The following operands are supported:
//      <configfile> Name of a simulation configuration file which includes the
//                   appropraite SeaWinds parameters
//      <outfile>    Name of output file
//      <L1A_file ... >  List of L1A files.
//
// EXAMPLES
//    An example of a command line is:
//      % check_inst_tracking nominal.cfg CheckInst.out QS_S1A*
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
//    Bryan W. Stiles <mailto:Bryan.W.Stiles@jpl.nasa.gov>
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
#include <string.h>
#include <stdlib.h>
#include "hdf.h"
#include "mfhdf.h"
#include "Qscat.h"
#include "BufferedList.h"
#include "List.h"
#include "Sds.h"
#include "Misc.h"
#include "Ephemeris.h"
#include "ConfigList.h"
#include "L1A.h"
#include "ConfigSim.h"
#include "L1B.h"
#include "L1AToL1B.h"
#include "Tracking.h"
#include "QscatConfig.h"

//-----------//
// TEMPLATES //
//-----------//

// Class declarations needed for templates
// eliminates need to include the entire header file
class AngleInterval;

template class List<AngleInterval>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<StringPair>;
template class List<Meas>;
template class List<EarthPosition>;
template class List<MeasSpot>;
template class List<WindVectorPlus>;
template class List<long>;
template class List<OffsetList>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;

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

const char* usage_array[] = { "<configfile>", "output_file","<L1A_file...>", 0 };

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
    // no_path is a helper function that simply gets the name of the
    // executable without its path. That way, error messages will
    // say "hdf_program: error doing something" rather than
    // "/home/users/dork/bin/hdf_program: error doing something".
    // I think it just looks nicer.
    //
    // usage is another helper function. It will format and spit out
    // usage messages for you (and then exit with the specified exit
    // code).
    //--------------------

    const char* command = no_path(argv[0]);

    //--------------------
    // By default, we will write to standard output. If a file
    // name is given, we will use that instead.
    //--------------------

    FILE* ofp = stdout;


    if (argc < 4)
        usage(command, usage_array, 1);

    int clidx=1;
    char* config_file = argv[clidx++];
    char* out_file = argv[clidx++];
    int start_file_idx = clidx;
    int end_file_idx = argc;

    //----------------------//
    // open the output file //
    //----------------------//

    // only bother trying to open the file if there is one
    ofp = fopen(out_file, "w");
    if (ofp == NULL)
      {
	fprintf(stderr, "%s: error opening file %s\n", command,
                out_file);
	exit(1);
      }



    //---------------------//
    // read in config file //
    //---------------------//

    ConfigList config_list;
    if (! config_list.Read(config_file))
    {
        fprintf(stderr, "%s: error reading sim config file %s\n",
            command, config_file);
        exit(1);
    }

    //----------------------------//
    // create and configure QSCAT //
    //----------------------------//

    Qscat qscat;
    if (! ConfigQscat(&qscat, &config_list))
    {
        fprintf(stderr, "%s: error configuring QSCAT\n", command);
        exit(1);
    }

    //------------------------------//
    // loop through the input files //
    //------------------------------//

    for (int file_idx = start_file_idx; file_idx < end_file_idx; file_idx++)
    {
        char* hdf_input_filename = argv[file_idx];

	int file_offset=2000000*(file_idx-start_file_idx);
        //--------------------
        // hdf_input_filename contains the name of... well... the HDF
        // input file. Now that we have the filename, let's do some HDF!
        //
        // First, we'll need to open the file for reading. Actually,
        // with HDF there are different ways to open a file depending
        // on what you want to do with it. In this case, we want to get
        // at the Scientific Data Sets (SDSs for short), so we'll use
        // SDstart. If you want to access the Vdata (the frame time
        // string), you'll need to open the file a different way; ask me
        // for help with that.
        //
        // Anyway, here we go...
        //--------------------

        int32 sd_id = SDstart(hdf_input_filename, DFACC_READ);
        if (sd_id == FAIL)
        {
            fprintf(stderr, "%s: error opening HDF file %s for reading\n",
                command, hdf_input_filename);
            exit(1);
        }

        //--------------------
        // That wasn't too hard was it? Now, let's determine the
        // number of frames in the file. That bit of information is
        // in the header -- also called the metadata or file attributes.
        // To get an attribute, we first need to look up its
        // index using SDfindattr.
        //--------------------

        int32 attr_index_l1a_actual_frame = SDfindattr(sd_id,
            "l1a_actual_frames");

        //--------------------
        // ...and we can read it. To be "good" I probably should
        // use the data type and the number of values to allocate
        // the right amount of memory for the attribute. But, I'm lazy
        // and I know that this particular attribute won't be longer
        // than 1024 bytes.
        //--------------------

        char data[1024];
        if (SDreadattr(sd_id, attr_index_l1a_actual_frame, data) == FAIL)
        {
            fprintf(stderr,
                "%s: error reading attribute for l1a_actual_frames\n",
                command);
            exit(1);
        }

        //--------------------
        // Attributes can contain lots of different types of data.
        // The GDS made theirs all strings. The first line contains
        // the type of data, the second line contains the number of
        // elements, and the rest is the attribute value. We'll
        // use sscanf to throw out the first two lines and get the
        // frame count from the third line. Yes, it is a strange
        // looking use of scanf; if you are interested in the details,
        // check out the man page.
        //--------------------

        int frame_count = 0;
        if (sscanf(data, " %*[^\n] %*[^\n] %d", &frame_count) != 1)
        {
            fprintf(stderr, "%s: error parsing l1a_actual_frame attribute\n",
                command);
            fprintf(stderr, "%s\n", data);
            exit(1);
        }


        //--------------------
        // Now, let's get some real data. In this case we need to
        // get an SDS ID for each parameter that we want. However,
        // HDF only allows us to look up an SDS index from the
        // name. Then we have to look up the ID using the index.
        // It's a little tedious, I know, but it still is fairly
        // straightforward. Actually, James has made a helper function,
        // cleverly called "SDnametoid" that makes both function
        // calls for you. Let's all enjoy use happy function!
        //
        // ***UPDATE***
        // James has updated the "SDnametoid" function to do even more!
        // Now, if you pass it a pointer to a float64, SDnametoid will
        // look up the scale factor for you. Yep, that's right: you
        // won't need to read the SIS and hard code in scale factors.
        // It would look like this...
        // float64 sigma0_sf;
        // int32 sigma0_sds_id = SDnametoid(sd_id, "sigam0", &sigma0_sf);
        // If the scale factor is zero, something went wrong.
        //--------------------

        int32 range_gate_a_delay_sds_id= SDnametoid(sd_id, 
						    "range_gate_a_delay");
        int32 range_gate_b_delay_sds_id = SDnametoid(sd_id, 
						    "range_gate_b_delay");
	int32 doppler_shift_command_1_sds_id = SDnametoid(sd_id, 
						    "doppler_shift_command_1");
        int32 doppler_shift_command_2_sds_id = SDnametoid(sd_id, 
						    "doppler_shift_command_2");
        int32 antenna_position_sds_id = SDnametoid(sd_id,"antenna_position");
        int32 doppler_orbit_step_sds_id = SDnametoid(sd_id,
						     "doppler_orbit_step");
        int32 prf_orbit_step_change_sds_id = SDnametoid(sd_id,
						     "prf_orbit_step_change");
        int32 frame_err_status_sds_id = SDnametoid(sd_id, "frame_err_status");
        int32 frame_qual_flag_sds_id = SDnametoid(sd_id, "frame_qual_flag");
        int32 pulse_qual_flag_sds_id = SDnametoid(sd_id, "pulse_qual_flag");
        int32 frame_inst_status_sds_id = SDnametoid(sd_id,"frame_inst_status");
        int32 true_cal_pulse_pos_sds_id = SDnametoid(sd_id,
						     "true_cal_pulse_pos");

        //--------------------
        // Here comes the real fun: reading the data. There is a
        // bit of explaining that I need to do first. The SDreaddata
        // function allows you to specify what portion of data you
        // want. Actually, it REQUIRES you to specify what portion
        // of data you want. The arguments start, stride, and edges
        // describe the starting location, the number of elements to
        // skip after each read, and the number of elements to be
        // read, respectively, for each dimension. As Greg will
        // undoubtedly tell you, we don't ever want to skip any data;
        // so, set stride to NULL. (If you really want to skip data
        // I'd be happy to give you a copy of the HDF documentation.)
        // The start and edges arguments should be self-explanatory
        // based on the examples below. I have chosen to do this
        // processing on a frame-by-frame basis so that it doesn't
        // hog tons of memory. Please adopt a similar "resource-kind"
        // programming style; other people would like to execute
        // their programs too.
        //--------------------

        //--------------------
        // Oh, one more thing. Don't worry about the following
        // start and edges variables right now. They'll make
        // sense in a minute 'cause I'll 'splain 'em.
        //--------------------

        int32 generic_1d_start[1] = { 0 };
        int32 generic_1d_edges[1] = { 1 };

        int32 pulse_qual_flag_start[2] = { 0, 0 };
        int32 pulse_qual_flag_edges[2] = { 1, 13 };

        int32 antenna_position_start[2] = { 0, 0 };
        int32 antenna_position_edges[2] = { 1, 100 };

	int last_frame_bad=1;
        
        for (int frame_idx = 0; frame_idx < frame_count; frame_idx++)
        {
	    int frame_offset= file_offset+100*frame_idx;
            //--------------------
            // The generic_1d_start array is used to indicate the
            // starting frame number for all our one-dimensional
            // extraction needs. This will make more sense as
            // you read more.
            //--------------------

            generic_1d_start[0] = frame_idx;

            //--------------------
            // OK. Here's the plan. We're going to do a few checks
            // as we go and ignore frames that have errors in them.
            // We will check the frame error status, frame quality
            // flag, and pulse quality flags. Let's go!
            //--------------------

            //--------------------
            // In the L1A file, the frame_err_status is stored as an
            // array of size [N] where N is the number of frames in the
            // file. The L1A SIS says that frame_err_status is of size
            // [13000], but they really mean the number of frames, N.
            // The storage type is uint32 (unsigned 32 bit integer).
            // Since we only want to read one frame at a time, the
            // edges array needs to contain a 1 indicating one frame.
            // The start array simply needs to contain the starting
            // frame index (frame_idx). Notice that I set up the
            // edges arrays before this loop. That way, they only get
            // set once. The start array, however, needs to get set
            // every time. And, yes, it is kind of weird to define
            // an array of size one. Later, when we need to extract
            // multidimensional parameters, we'll use longer arrays.
            //--------------------

#define ALL_FRAMES_GOOD
#ifndef ALL_FRAMES_GOOD
            uint32 frame_err_status;
            if (SDreaddata(frame_err_status_sds_id, generic_1d_start,
                NULL, generic_1d_edges, (VOIDP)&frame_err_status) == FAIL)
            {
                fprintf(stderr,
                    "%s: error reading SD data for frame_err_status (ID=%d)\n",
                    command, (int)frame_err_status_sds_id);
                exit(1);
            }

            //--------------------
            // good frames have a frame_err_status of zero
            //--------------------

            if (frame_err_status != 0)
            {
                //--------------------
                // There is something evil in this frame. Tell the
                // user and go on to the next frame.
                //--------------------

                fprintf(stderr,
                    "%s: frame %d is evil. (error status = 0x%08x)\n",
                    command, frame_idx, (unsigned int)frame_err_status);
                last_frame_bad=1;
                continue;
            }

            //--------------------
            // The frame quality flag is similar to the frame error
            // status.
            //--------------------

            uint16 frame_qual_flag;
            if (SDreaddata(frame_qual_flag_sds_id, generic_1d_start,
                NULL, generic_1d_edges, (VOIDP)&frame_qual_flag) == FAIL)
            {
                fprintf(stderr,
                    "%s: error reading SD data for frame_qual_flag\n",
                    command);
                exit(1);
            }

            if (frame_qual_flag != 0)
            {
                //--------------------
                // This frame it just not up to our high standards
                // of "quality". No, I don't know what that means.
                // Let's dump it anyway.
                //--------------------

                fprintf(stderr,
                    "%s: frame %d is evil. (quality flag = %0x)\n", command,
                    frame_idx, frame_qual_flag);
		last_frame_bad=1;
                continue;
            }
#endif // end ifndef ALL_FRAMES_GOOD
            //--------------------
            // The pulse quality flag is a bit more complicated. If
            // you look at the L1A SIS, you will see that the pulse
            // quality flag is a two-dimensional array of size
            // [N][13] (remember that 13000 is really N). Since we
            // are reading one frame at a time, we  will be
            // reading a [13] array. We want to read 1 frame, 13
            // bytes so the edges array should be { 1, 13 }. And,
            // we want to start reading at the current frame and
            // with byte 0 so the start array should be { frame_idx, 0 }.
            // The 0 has already been set, so we only need to deal with
            // the frame_idx.
            //--------------------

            pulse_qual_flag_start[0] = frame_idx;
            uint8 pulse_qual_flag[13];
            if (SDreaddata(pulse_qual_flag_sds_id, pulse_qual_flag_start,
                NULL, pulse_qual_flag_edges, (VOIDP)&pulse_qual_flag) == FAIL)
            {
                fprintf(stderr,
                    "%s: error reading SD data for pulse_qual_flag\n",
                    command);
                exit(1);
            }

            //--------------------
            // Good pulses have their bit set to 0.
            //--------------------

            int bad_pulses = 0;
            for (int i = 0; i < 13; i++)
            {
                if (pulse_qual_flag[i] != 0)
                {
                    bad_pulses = 1;
                }
            }
            if (bad_pulses)
            {
                //--------------------
                // At least one of the pulses has gone bad. It is
                // likely to be due to the SES reset. We'll be
                // conservative (a.k.a. lazy) and ignore the entire
                // frame.
                //--------------------

                fprintf(stderr, "%s: frame %d is evil. (bad pulse)\n",
                    command, frame_idx);
		last_frame_bad=1;
                continue;
            }

            //--------------------
            // antenna_position is a [N][100] array, where N is the
            // total number of frames in the file, 100 is the number
            // of pulses per frame. Since I'm reading one frame at a time,
            // I just need a [100] array. The power_dn_edges
            // array indicates how much data to read (1 frame, 100
            // pulses). The power_dn_start array was
            // initialized with all zeroes { 0, 0 }.
            // The only part of that we will want to change is the
            // starting frame which should increment as we go
            // throught the frames one by one. So, I'll set it to the
            // frame index.
            //--------------------

            antenna_position_start[0] = frame_idx;
            uint16 antenna_position[100];
            if (SDreaddata(antenna_position_sds_id, antenna_position_start,
			   NULL, antenna_position_edges, 
			   (VOIDP)antenna_position) == FAIL)
            {
                fprintf(stderr,
                    "%s: error reading SD data for antenna_position\n",
                    command);
                exit(1);
            }

            //------------------------------------
            // Obtain various other parameters
            //------------------------------------

            uint8 range_gate_a_delay;
            if (SDreaddata(range_gate_a_delay_sds_id, generic_1d_start,
                NULL, generic_1d_edges, (VOIDP)&range_gate_a_delay) == FAIL)
            {
                fprintf(stderr, "%s: error reading range_gate_a_delay\n",
                    command);
                exit(1);
            }

	    float range_gate_delay_1=range_gate_a_delay*
	      RX_GATE_DELAY_CMD_RESOLUTION;

            uint8 range_gate_b_delay;
            if (SDreaddata(range_gate_b_delay_sds_id, generic_1d_start,
                NULL, generic_1d_edges, (VOIDP)&range_gate_b_delay) == FAIL)
            {
                fprintf(stderr, "%s: error reading range_gate_b_delay\n",
                    command);
                exit(1);
            }
	    float range_gate_delay_2=range_gate_b_delay*
	      RX_GATE_DELAY_CMD_RESOLUTION;

	    //----------------------------------------------------//
            // Doppler shifts are stored in the HDF file in the   //
            // following STUPID format:                           //
            //                                                    //
            // for doppler_shift_command_1 the LEAST significant  //
            // 16 bits of the unsigned 32 bit value contains the  //
            // SIGNED 16-bit doppler shift dn value (1 dn=2 Khz)  //
            //                                                    //
            // for doppler_shift_command_2 the same format but    //
            // while for doppler_shift_command_1 bits 16-31 are   //
	    // all zero, for doppler_shift_command_2 bit 16 is one//
            //----------------------------------------------------//
           

            int16 doppler_shift_command_1;
            uint32 doppler_word;
            if (SDreaddata(doppler_shift_command_1_sds_id, generic_1d_start,
                NULL, generic_1d_edges, (VOIDP)&doppler_word) 
		== FAIL)
            {
                fprintf(stderr, "%s: error reading doppler_shift_command_1\n",
                    command);
                exit(1);
            }
            
	    // Extract 16 bit signed number
            doppler_shift_command_1=(int16)(doppler_word);  

	    float doppler_shift_1=doppler_shift_command_1*
	      TX_FREQUENCY_CMD_RESOLUTION;

            int16 doppler_shift_command_2;
            if (SDreaddata(doppler_shift_command_2_sds_id, generic_1d_start,
                NULL, generic_1d_edges, (VOIDP)&doppler_word) 
		== FAIL)
            {
                fprintf(stderr, "%s: error reading doppler_shift_command_2\n",
                    command);
                exit(1);
            }

	    // Extract 16-bit signed number
	    doppler_shift_command_2=(int16)(doppler_word);

	    float doppler_shift_2=doppler_shift_command_2*
	      TX_FREQUENCY_CMD_RESOLUTION;

            uint8 doppler_orbit_step;
            if (SDreaddata(doppler_orbit_step_sds_id, generic_1d_start,
                NULL, generic_1d_edges, (VOIDP)&doppler_orbit_step) == FAIL)
            {
                fprintf(stderr, "%s: error reading doppler_orbit_step\n",
                    command);
                exit(1);
            }

            uint8 prf_orbit_step_change;
            if (SDreaddata(prf_orbit_step_change_sds_id, generic_1d_start,
                NULL, generic_1d_edges, (VOIDP)&prf_orbit_step_change) == FAIL)
            {
                fprintf(stderr, "%s: error reading prf_orbit_step_change\n",
                    command);
                exit(1);
            }

            //--------------------
            // You might recall that SeaWinds does this thing called
            // the calibration loopback. We want to detect and ignore
            // them. Greg will probably want to detect and keep them.
            // The variable true_cal_pulse_pos is the key to all of this.
            //--------------------

            int8 true_cal_pulse_pos;
            if (SDreaddata(true_cal_pulse_pos_sds_id, generic_1d_start,
                NULL, generic_1d_edges, (VOIDP)&true_cal_pulse_pos) == FAIL)
            {
                fprintf(stderr, "%s: error reading true_cal_pulse_pos\n",
                    command);
                exit(1);
            }

            int loopback_index = true_cal_pulse_pos - 1;
            int load_index = true_cal_pulse_pos;

            //--------------------
            // Use the retrieved parameters to perform tracking and compare
            // slices to get an average profile.
            //--------------------

            // qscat->cds.time=frame_time[0];
            // qscat.cds.orbitTime=orbit_time[0];
        
            for (int spot_idx = 0; spot_idx < 100; spot_idx++)
            {
	      int pulse_number=frame_offset+spot_idx;
	      if ( spot_idx==loopback_index || spot_idx==load_index )
		{
		  continue;
		}

              // set up beam index
	      int beam_idx = spot_idx % NUMBER_OF_QSCAT_BEAMS;
	      qscat.cds.currentBeamIdx = beam_idx;

	      //----------------//
	      // set orbit step //
	      //----------------//

	      unsigned int orbit_step = doppler_orbit_step;
	      if (prf_orbit_step_change != 255 &&
		  spot_idx < prf_orbit_step_change)
		{
		  if (orbit_step == 0)
		    orbit_step = ORBIT_STEPS - 1;
		  else
		    orbit_step--;
		}
	      qscat.cds.orbitStep = orbit_step;

	      //-------------//
	      // set antenna //
	      //-------------//
	      unsigned short held_encoder = *(antenna_position + spot_idx);
	      qscat.cds.heldEncoder = held_encoder;
	      qscat.SetEncoderAzimuth(held_encoder, 1);

	      //----------------------------//
	      // range and Doppler tracking //
	      //----------------------------//
	      SetDelayAndFrequency(NULL, &qscat);
	      float calc_dop=qscat.ses.txDoppler;
              float calc_rgd=qscat.ses.rxGateDelay;

              // write to output file
              if(!last_frame_bad && spot_idx<2){
		if(spot_idx==0){
                  int delta_rgd_dn=qscat.cds.rxGateDelayDn-range_gate_a_delay;
		  float delta_rgd=delta_rgd_dn*RX_GATE_DELAY_CMD_RESOLUTION;
                  int delta_dop_dn=qscat.cds.txDopplerDn
		    -doppler_shift_command_1;
		  float delta_dop=delta_dop_dn*TX_FREQUENCY_CMD_RESOLUTION;
		  fprintf(ofp,"%d %g %g %d %g %g %g %d %g\n",pulse_number,
			  range_gate_delay_1,calc_rgd,delta_rgd_dn,delta_rgd, 
			  doppler_shift_1,calc_dop,delta_dop_dn,delta_dop);
		}
		else{
                  int delta_rgd_dn=qscat.cds.rxGateDelayDn-range_gate_b_delay;
		  float delta_rgd=delta_rgd_dn*RX_GATE_DELAY_CMD_RESOLUTION;
                  int delta_dop_dn=qscat.cds.txDopplerDn
		    -doppler_shift_command_2;
		  float delta_dop=delta_dop_dn*TX_FREQUENCY_CMD_RESOLUTION;
		  fprintf(ofp,"%d %g %g %d %g %g %g %d %g\n",pulse_number,
			  range_gate_delay_2,calc_rgd,delta_rgd_dn,delta_rgd, 
			  doppler_shift_2,calc_dop,delta_dop_dn,delta_dop);
		}
	      }
            }
	    last_frame_bad=0;
        }

        //--------------------
        // We are almost done with this file. But first, we MUST
        // end our access to all of the SDSs.
        //--------------------

        if (SDendaccess(antenna_position_sds_id) == FAIL ||
            SDendaccess(frame_err_status_sds_id) == FAIL ||
            SDendaccess(frame_qual_flag_sds_id) == FAIL ||
            SDendaccess(frame_inst_status_sds_id) == FAIL ||
            SDendaccess(true_cal_pulse_pos_sds_id) == FAIL ||
	    SDendaccess(range_gate_a_delay_sds_id) == FAIL ||
	    SDendaccess(range_gate_b_delay_sds_id) == FAIL ||
	    SDendaccess(doppler_shift_command_1_sds_id) == FAIL ||
	    SDendaccess(doppler_shift_command_2_sds_id) == FAIL ||
	    SDendaccess(doppler_orbit_step_sds_id) == FAIL ||
	    SDendaccess(prf_orbit_step_change_sds_id) == FAIL 
	    )
        {
            fprintf(stderr, "%s: error ending SD access\n", command);
            exit(1);
        }

        //--------------------
        // Finally, we can say goodbye to this file. Buh-bye!
        //--------------------

        if (SDend(sd_id) == FAIL)
        {
            fprintf(stderr, "%s: error ending SD\n", command);
            exit(1);
        }
    }

    //-----------------------//
    // close the output file //
    //-----------------------//

    fclose(ofp);

    return (0);
}



