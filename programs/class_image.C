//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    class_image
//
// SYNOPSIS
//    class_image <output_base> <rev_count> <rev_spacing> <val_file...>
//
// DESCRIPTION
//    Generates average value images on a 0.5 x 0.5 degree grid.
//
// OPTIONS
//    None.
//
// OPERANDS
//    <output_base>      The output base.
//    <rev_count>        The number of revs to average.
//    <rev_spacing>      The number of revs to pass for each output
//                         image.
//    <val_file...>      The input classification values.  These MUST
//                         be in rev order and MUST have the rev number
//                         as the first part of the filename.
//
// EXAMPLES
//    An example of a command line is:
//      % class_image 30.10.class 30 10 *.val
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
//    James N. Huddleston (James.N.Huddleston@jpl.nasa.gov)
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
#include "List.h"
#include "List.C"
#include "Misc.h"
#include "Array.h"
#include "Index.h"

//-----------//
// TEMPLATES //
//-----------//

class Datum
{
public:
    Datum();
    ~Datum();

    unsigned short  rev;
    short           val;
};

Datum::Datum()
:   rev(0), val(0)
{
    return;
}

Datum::~Datum()
{
    return;
}

class DatumList : public List<Datum>
{
public:
    DatumList();
    ~DatumList();
};

DatumList::DatumList()
{
    return;
}

DatumList::~DatumList()
{
    return;
}

template class List<Datum>;

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING        ""

#define FLOAT_IMAGE_HEADER  "imf "
#define CHAR_IMAGE_HEADER   "imc "

#define SHORT_SCALE      8000.0
#define SHORT_BIAS       0.0
#define SHORT_MIN        -32767
#define SHORT_MAX        32767

#define CHAR_SCALE      (199.0/6.0)
#define CHAR_BIAS       2.0
#define CHAR_MIN        0
#define CHAR_MAX        199

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "<output_base>", "<rev_count>",
    "<rev_spacing>", "<val_file...>", 0 };

//--------------//
// MAIN PROGRAM //
//--------------//

int
main(
    int    argc,
    char*  argv[])
{
    //------------//
    // initialize //
    //------------//

    //------------------------//
    // parse the command line //
    //------------------------//

    const char* command = no_path(argv[0]);

    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    if (argc < optind + 4)
        usage(command, usage_array, 1);

    const char* output_base = argv[optind++];
    int rev_count = atoi(argv[optind++]);
    int rev_spacing = atoi(argv[optind++]);
    int start_file_idx = optind;
    int end_file_idx = argc;

    //-----------------//
    // determine sizes //
    //-----------------//

    int lon_bins = 720;
    int lat_bins = 360;
    float lon_min = 0.0;
    float lon_max = 360.0;
    float lat_min = -90.0;
    float lat_max = 90.0;

    //-----------------------//
    // generate output array //
    //-----------------------//

    DatumList*** accum_image = (DatumList***)make_array(sizeof(DatumList*), 2,
        lon_bins, lat_bins);
    if (accum_image == NULL)
    {
        fprintf(stderr,
            "%s: error allocating array (%d x %d) for accumulator\n",
            command, lon_bins, lat_bins);
        exit(1);
    }

    for (int lon_idx = 0; lon_idx < lon_bins; lon_idx++)
    {
        for (int lat_idx = 0; lat_idx < lat_bins; lat_idx++)
        {
            DatumList* dl = new DatumList();
            if (dl == NULL)
            {
                fprintf(stderr, "%s: error allocating DatumList\n", command);
                exit(1);
            }
            accum_image[lon_idx][lat_idx] = dl;
        }
    }

    Index lon_index;
    lon_index.SpecifyWrappedCenters(lon_min, lon_max, lon_bins);
    Index lat_index;
    lat_index.SpecifyEdges(lat_min, lat_max, lat_bins);

    unsigned char** output_image =
        (unsigned char**)make_array(sizeof(unsigned char), 2, lon_bins,
        lat_bins);

    //-------------------------//
    // accumulate file by file //
    //-------------------------//

    int revs_in = 0;
    int last_rev_output = 0;
    for (int file_idx = start_file_idx; file_idx < end_file_idx; file_idx++)
    {
        const char* file = argv[file_idx];
printf("%s\n", file);
        FILE* ifp = fopen(file, "r");
        if (ifp == NULL)
        {
            fprintf(stderr, "%s: error opening file %s\n", command, file);
            exit(1);
        }

        //--------------------------//
        // determine the rev number //
        //--------------------------//

        int current_rev;
        char* ptr = strrchr(file, (int)"/");
        if (ptr == NULL)
            ptr = (char *)file;
        else
            ptr += 1;
        if (sscanf(ptr, "%d", &current_rev) != 1)
        {
            fprintf(stderr, "%s: error determining rev number from file %s\n",
                command, file);
            exit(1);
        }
        int min_rev = current_rev - rev_count;

        //-------------------//
        // remove "old" data //
        //-------------------//

        for (int lon_idx = 0; lon_idx < lon_bins; lon_idx++)
        {
            for (int lat_idx = 0; lat_idx < lat_bins; lat_idx++)
            {
                DatumList* dl = accum_image[lon_idx][lat_idx];
                for (Datum* d = dl->GetHead(); d != NULL; )
                {
                    if (d->rev <= min_rev)
                    {
                        Datum* old_d = dl->RemoveCurrent();
                        delete old_d;
                        d = dl->GetCurrent();
                    }
                    else
                        d = dl->GetNext();
                }
            }
        }

        //---------------//
        // skip the apts //
        //---------------//

        do
        {
            float lon, lat, val;
            if (fread((void *)&lon, sizeof(float), 1, ifp) != 1 ||
                fread((void *)&lat, sizeof(float), 1, ifp) != 1 ||
                fread((void *)&val, sizeof(float), 1, ifp) != 1)
            {
                if (feof(ifp))
                    break;
                fprintf(stderr, "%s: error reading from file %s\n", command,
                    file);
                exit(1);
            }

            int lon_idx;
            lon_index.GetNearestWrappedIndex(lon, &lon_idx);
            int lat_idx;
            lat_index.GetNearestIndex(lat, &lat_idx);

            Datum* datum = new Datum;
            datum->rev = (unsigned short)current_rev;
            float fval = val * SHORT_SCALE + SHORT_BIAS;
            if (fval > SHORT_MAX)
                fval = SHORT_MAX;
            if (fval < SHORT_MIN)
                fval = SHORT_MIN;
            datum->val = (short)(fval);    // this should be rounded better!
            DatumList* dl = accum_image[lon_idx][lat_idx];
            if (! dl->Append(datum))
            {
                fprintf(stderr, "%s: error adding Datum to DatumList\n",
                    command);
                exit(1);
            }
        } while (1);
        fclose(ifp);

        //----------------------//
        // check rev quantities //
        //----------------------//

        revs_in++;
        // make sure we at least have read the number of revs to average
        if (revs_in < rev_count)
            continue;

        if (current_rev < last_rev_output + rev_spacing)
            continue;

        //--------------------------//
        // generate an output image //
        //--------------------------//

        for (int i = 0; i < lon_bins; i++)
        {
            for (int j = 0; j < lat_bins; j++)
            {
                output_image[i][j] = 0;
            }
        }

        for (int lon_idx = 0; lon_idx < lon_bins; lon_idx++)
        {
            for (int lat_idx = 0; lat_idx < lat_bins; lat_idx++)
            {
                DatumList* dl = accum_image[lon_idx][lat_idx];
                if (dl == NULL)
                    continue;
                double sum = 0.0;
                int count = 0;
                for (Datum* d = dl->GetHead(); d != NULL; d = dl->GetNext())
                {
                    sum += (double)d->val;
                    count++;
                }
                if (count == 0)
                    continue;
                float avg = sum / count;
                float outval = ((avg - SHORT_BIAS) / SHORT_SCALE +
                    CHAR_BIAS) * CHAR_SCALE;
                if (outval > 199.0)
                    outval = 199.0;
                if (outval < 0.0)
                    outval = 0.0;
                output_image[lon_idx][lat_idx] = (unsigned char)(outval + 0.5);
            }
        }
        char filename[1024];
        sprintf(filename, "%s.%05d", output_base, current_rev);
        FILE* ofp = fopen(filename, "w");
        if (ofp == NULL)
        {
            fprintf(stderr, "%s: error opening output file %s\n", command,
                filename);
            exit(1);
        }
        char* im_string = CHAR_IMAGE_HEADER;
        fwrite(im_string, 4, 1, ofp);
        fwrite(&lon_bins, 4, 1, ofp);
        fwrite(&lat_bins, 4, 1, ofp);
        for (int lon_idx = 0; lon_idx < lon_bins; lon_idx++)
        {
            fwrite(output_image[lon_idx], sizeof(unsigned char),
                lat_bins, ofp);
        }
        fclose(ofp);
        printf("-> %s\n", filename);
        last_rev_output = current_rev;
    }

    return (0);
}
