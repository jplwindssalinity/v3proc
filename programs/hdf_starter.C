//=========================================================//
// Copyright (C) 2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    hdf_starter
//
// SYNOPSIS
//    hdf_starter [ -o output_file ] <L1A_file...>
//
// DESCRIPTION
//    This is a sample program which shows you how to read from
//    an HDF file. More importantly, it is a starter program which
//    you can modify to create your own programs. I have tried to
//    make the code as HDF-ish as possible. In other words, you will
//    be calling *actual* HDF routines and learn a bit about HDF as
//    you go. Don't panic! It really isn't all that complicated.
//    What does this sample program do? It will extract all 12
//    slices, and create an average slice profile. Not very useful,
//    but a decent enough example for showing how things work. Along
//    the way I will be doing a lot of the "basics" such as checking
//    frame and pulse quality flags and avoiding the calibration
//    loopback and load measurements.
//
// OPTIONS
//    The following options are supported:
//      [ -o output_file ]  The output file. If not specified, the
//                            output will be written to standard output.
//
// OPERANDS
//    The following operands are supported:
//      <L1A_file...>  A list of L1A files.
//
// EXAMPLES
//    An example of a command line is:
//      % hdf_starter -o average_cal_profile.dat /seapac/disk1/L1A/data/QS_S1A*
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
//    James N. Huddleston <mailto:James.N.Huddleston@jpl.nasa.gov>
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
#include "hdf.h"
#include "mfhdf.h"

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING  "o:"

//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

const char* no_path(const char* string);
void usage(const char* command, const char* option_array[],
    const int exit_value);
int32 SDnametoid(int32 sd_id, char* sds_name, float64* scale_factor = NULL);

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -o output_file ]", "<L1A_file...>", 0 };

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
    // no_path is my own function that simply gets the name of the
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
    extern char* optarg;
    extern int optind;

    //--------------------
    // By default, we will write to standard output. If a file
    // name is given, we will use that instead.
    //--------------------

    FILE* ofp = stdout;
    char* output_file = NULL;

    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
        case 'o':
            output_file = optarg;
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    if (argc <= optind)
        usage(command, usage_array, 1);

    int start_file_idx = optind;
    int end_file_idx = argc;

    //----------------------//
    // open the output file //
    //----------------------//

    if (output_file != NULL)
    {
        // only bother trying to open the file if there is one
        ofp = fopen(output_file, "w");
        if (ofp == NULL)
        {
            fprintf(stderr, "%s: error opening file %s\n", command,
                output_file);
            exit(1);
        }
    }

    //--------------------
    // These are some variables for calculating the average slice profile.
    //--------------------

    double slice_sum[12];
    for (int i = 0; i < 12; i++)
    {
        slice_sum[i] = 0.0;
    }
    unsigned long count = 0;

    //------------------------------//
    // loop through the input files //
    //------------------------------//

    for (int file_idx = start_file_idx; file_idx < end_file_idx; file_idx++)
    {
        char* hdf_input_filename = argv[file_idx];

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
        // If we want, we can get information about the attribute.
        // This part of the code is just for show. You'll notice that I
        // don't use any of the information I get.
        //--------------------

        char attr_name[MAX_NC_NAME];
        int32 data_type, n_values;
        if (SDattrinfo(sd_id, attr_index_l1a_actual_frame, attr_name,
            &data_type, &n_values) == FAIL)
        {
            fprintf(stderr,
                "%s: error getting attribute info for l1a_actual_frames\n",
                command);
            exit(1);
        }

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
        // straightforward. Actually, I've made a helper function,
        // cleverly called "SDnametoid" that makes both function
        // calls for you. Let's all enjoy use happy function!
        //
        // ***UPDATE***
        // I have updated the "SDnametoid" function to do even more!
        // Now, if you pass it a pointer to a float64, SDnametoid will
        // look up the scale factor for you. Yep, that's right: you
        // won't need to read the SIS and hard code in scale factors.
        // It would look like this...
        // float64 sigma0_sf;
        // int32 sigma0_sds_id = SDnametoid(sd_id, "sigam0", &sigma0_sf);
        // If the scale factor is zero, something went wrong.
        //--------------------

        int32 power_dn_sds_id = SDnametoid(sd_id, "power_dn");
        int32 frame_err_status_sds_id = SDnametoid(sd_id, "frame_err_status");
        int32 frame_qual_flag_sds_id = SDnametoid(sd_id, "frame_qual_flag");
        int32 pulse_qual_flag_sds_id = SDnametoid(sd_id, "pulse_qual_flag");
        int32 frame_inst_status_sds_id = SDnametoid(sd_id,
            "frame_inst_status");
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

        int32 power_dn_start[3] = { 0, 0, 0 };
        int32 power_dn_edges[3] = { 1, 100, 12 };

        for (int frame_idx = 0; frame_idx < frame_count; frame_idx++)
        {
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
                continue;
            }

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
                continue;
            }

            //--------------------
            // power_dn is a [N][100][12] array, where N is the
            // total number of frames in the file, 100 is the number
            // of pulses per frame, and 12 is the number of slices
            // per pulse. Since I'm reading one frame at a time,
            // I just need a [100][12] array. The power_dn_edges
            // array indicates how much data to read (1 frame, 100
            // pulses, 12 slices). The power_dn_start array was
            // initialized with all zeroes { 0, 0, 0 }.
            // The only part of that we will want to change is the
            // starting frame which should increment as we go
            // throught the frames one by one. So, I'll set it to the
            // frame index.
            //--------------------

            power_dn_start[0] = frame_idx;
            uint32 power_dn[100][12];
            if (SDreaddata(power_dn_sds_id, power_dn_start, NULL,
                power_dn_edges, (VOIDP)power_dn) == FAIL)
            {
                fprintf(stderr,
                    "%s: error reading SD data for power_dn\n",
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
            // Here is my bogus application: averaging the science
            // slices to get an average profile.
            //--------------------

            for (int pulse_idx = 0; pulse_idx < 100; pulse_idx++)
            {
                if (pulse_idx == loopback_index || pulse_idx == load_index)
                    continue;

                for (int slice_idx = 0; slice_idx < 12; slice_idx++)
                {
                    slice_sum[slice_idx] +=
                        (double)power_dn[pulse_idx][slice_idx];
                }
                count++;
            }
        }

        //--------------------
        // We are almost done with this file. But first, we MUST
        // end our access to all of the SDSs.
        //--------------------

        if (SDendaccess(power_dn_sds_id) == FAIL ||
            SDendaccess(frame_err_status_sds_id) == FAIL ||
            SDendaccess(frame_qual_flag_sds_id) == FAIL ||
            SDendaccess(frame_inst_status_sds_id) == FAIL ||
            SDendaccess(true_cal_pulse_pos_sds_id) == FAIL)
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

    //--------------------
    // write out my "results"
    //--------------------

    for (int slice_idx = 0; slice_idx < 12; slice_idx++)
    {
        fprintf(ofp, "%d %g\n", slice_idx,
            slice_sum[slice_idx] / (double)count);
    }

    //-----------------------//
    // close the output file //
    //-----------------------//

    if (output_file != NULL)
    {
        fclose(ofp);
    }

    return (0);
}

//--------------------
// The following are three helper functions. Normally they would be
// put into a library, but if you leave them here you won't have to
// worry about linking.
//--------------------

//---------//
// no_path //
//---------//

const char*
no_path(
    const char*  string)
{
    const char* last_slash = strrchr(string, '/');
    if (! last_slash)
        return(string);
    return(last_slash + 1);
}

#define LINE_LENGTH  78

//-------//
// usage //
//-------//

void
usage(
    const char*  command,
    const char*  option_array[],
    const int    exit_value)
{
    fprintf(stderr, "usage: %s", command);
    int skip = 11;
    int position = 7 + strlen(command);
    for (int i = 0; option_array[i]; i++)
    {
        int length = 1 + strlen(option_array[i]);
        position += length;
        if (position > LINE_LENGTH)
        {
            fprintf(stderr, "\n%*s", skip, " ");
            position = skip + length;
        }
        fprintf(stderr, " %s", option_array[i]);
    }
    fprintf(stderr, "\n");
    exit(exit_value);
}

//------------//
// SDnametoid //
//------------//

int32
SDnametoid(
    int32     sd_id,
    char*     sds_name,
    float64*  scale_factor)
{
    //------------------------------//
    // convert the name to an index //
    //------------------------------//

    int32 sds_index = SDnametoindex(sd_id, sds_name);
    if (sds_index == FAIL)
    {
        fprintf(stderr, "SDnametoid: error converting SD name (%s) to index\n",
            sds_name);
        exit(1);
    }

    //-------------------------------//
    // select that sd, and get an id //
    //-------------------------------//

    int32 sds_id = SDselect(sd_id, sds_index);
    if (sds_id == FAIL)
    {
        fprintf(stderr, "SDnametoid: error converting SD index (%ld) to ID\n",
            sds_index);
        exit(1);
    }

    //----------------------//
    // get the scale factor //
    //----------------------//

    if (scale_factor != NULL)
    {
        float64 cal_error, offset, offset_error;
        int32 data_type;
        if (SDgetcal(sds_id, scale_factor, &cal_error, &offset,
            &offset_error, &data_type) == FAIL)
        {
            *scale_factor = 0.0;
        }
    }

    return (sds_id);
}
