//==============================================================//
// Copyright (C) 2000-2003, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    tiff
//
// SYNOPSIS
//    tiff [ -bdgs ] [ -c colormap ] [ -q # ] [ -x # ] [ -y # ]
//        <input_array> [ output_tiff ]
//
// DESCRIPTION
//    Converts a 2-D array into a TIFF.
//
// OPTIONS
//    [ -b ]       Bar. Generate a color bar strip too. input_array.bar.tiff
//    [ -d ]       Dump. Just output the raw data to stdout.
//    [ -g ]       Guess at the dimensions and ask.
//    [ -s ]       Scale. Linearly scale the array values to match
//                   the range of colors in the colormap.
//    [ -c colormap ]  Colormap. A colormap file to use. Colormap files
//                       are ASCII files containing (value, R, G, B) per line.
//                       A black to white colormap is the default.
//    [ -q # ]     Quality. A number from 0 to 100. Default is 100.
//    [ -x # ]     Specify the X dimension.
//    [ -y # ]     Specify the Y dimension.
//    [ output_tiff ]  The output TIFF file. If not specified, output
//                     file is input_array.tiff
//
// OPERANDS
//    <input_array>  The input array file.
//
// EXAMPLES
//    An example of a command line is:
//      % tiff -q 100 -c fire.cmap -x 500 array.dat
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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <tiffio.h>
#include "ColorMap.h"
#include "Array.h"
#include "Misc.h"
#include "List.h"

//-----------//
// TEMPLATES //
//-----------//

template class List<CMNode>;
template class SortableList<CMNode>;

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING       "bdgsc:q:x:y:"
#define TIFF_EXTENSION  "tiff"
#define COLORBAR_WIDTH  50

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

void  infowrite(int x_size, int y_size, const char* filename,
          FILE* fp = stdout);

//------------------//
// OPTION VARIABLES //
//------------------//

int opt_bar = 0;
int opt_dump = 0;
int opt_guess = 0;
int opt_scale = 0;
int opt_rel = 0;

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -bdgs ]", "[ -c colormap ]", "[ -q # ]",
    "[ -x # ]", "[ -y # ]", "<input_array>", "[ output_tiff ]", 0 };

int  x_size = -1;
int  y_size = -1;

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

    char* colormap_file = NULL;

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
            opt_bar = 1;
            break;
        case 'd':
            opt_dump = 1;
            break;
        case 'g':
            opt_guess = 1;
            break;
        case 's':
            opt_scale = 1;
            break;
        case 'c':
            colormap_file = optarg;
            break;
        case 'x':
            x_size = atoi(optarg);
            break;
        case 'y':
            y_size = atoi(optarg);
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    const char* array_file = NULL;
    const char* tiff_file = NULL;

    if (argc == optind || argc > optind + 2)
        usage(command, usage_array, 1);
    if (argc > optind)
        array_file = argv[optind++];
    if (argc > optind)
        tiff_file = argv[optind++];

    //------------------------------//
    // generate an output file name //
    //------------------------------//

    char filename[1024];
    if (tiff_file == NULL)
    {
        sprintf(filename, "%s.%s", array_file, TIFF_EXTENSION);
        tiff_file = filename;
    }

    //-------------------------------//
    // determine the array file size //
    //-------------------------------//

    struct stat stat_buf;
    if (stat(array_file, &stat_buf) != 0)
    {
        fprintf(stderr, "%s: error stating file %s\n", command, array_file);
        exit(1);
    }
    off_t filesize = stat_buf.st_size;
    int float_count_wo_hdr = (int)(filesize / 4);
    int float_count_w_hdr = (int)((filesize - 2 * sizeof(int)) / 4);
    int float_count = float_count_wo_hdr;

    //----------------------------//
    // guess the array dimensions //
    //----------------------------//

    if (opt_guess)
    {
        // i'm assuming there is no header here, otherwise, why guess?
        for (int try_x = 1; try_x < float_count_wo_hdr; try_x++)
        {
            int try_y = float_count_wo_hdr / try_x;
            if (try_x * try_y != float_count_wo_hdr)
                continue;
            printf("%d x %d\n", try_x, try_y);
        }
        exit(0);
    }

    //---------------------//
    // open the array file //
    //---------------------//

    FILE* ifp = fopen(array_file, "r");
    if (ifp == NULL)
    {
        fprintf(stderr, "%s: error opening array file %s\n", command,
            array_file);
        exit(1);
    }

    if (x_size == -1 && y_size == -1)
    {
        //----------------------------------//
        // array dimensions must be in file //
        //----------------------------------//

        if (fread(&x_size, sizeof(int), 1, ifp) != 1 ||
            fread(&y_size, sizeof(int), 1, ifp) != 1)
        {
            fprintf(stderr,
                "%s: error reading array dimensions from array file %s\n",
                command, array_file);
            exit(1);
        }
        float_count = float_count_w_hdr;
    }
    else if (x_size == -1 && y_size != -1)
    {
        x_size = (int)(float_count_wo_hdr / y_size);
    }
    else if (y_size == -1 && x_size != -1)
    {
        y_size = (int)(float_count_wo_hdr / x_size);
    }

    //----------------------//
    // check the dimensions //
    //----------------------//

    if (x_size * y_size != float_count)
    {
        fprintf(stderr, "%s: invalid array dimensions for array file %s\n",
            command, array_file);
        fprintf(stderr, "    (%d x %d)\n", x_size, y_size);
        exit(1);
    }

    //--------------------//
    // read the color map //
    //--------------------//

    ColorMap colormap;
    if (colormap_file != NULL)
    {
        if (! colormap.Read(colormap_file))
        {
            fprintf(stderr, "%s: error reading colormap file %s\n", command,
                colormap_file);
            exit(1);
        }
    }
    else
    {
        colormap.BlackToWhite(0.0, 1.0);
        opt_scale = 1;
    }

    //-----------------------------//
    // allocate for the input file //
    //-----------------------------//

    float** array = (float **)make_array(sizeof(float), 2, x_size, y_size);
    if (array == NULL)
    {
        fprintf(stderr, "%s: error allocating array (%d x %d)\n", command,
            x_size, y_size);
        exit(1);
    }

    //---------------------//
    // read the input file //
    //---------------------//

    int x, y;
    for (x = 0; x < x_size; x++)
    {
        if (fread(*(array + x), sizeof(float), y_size, ifp) !=
            (unsigned int)y_size)
        {
            fprintf(stderr, "%s: error reading array file %s\n", command,
                array_file);
            exit(1);
        }
    }

    //---------------//
    // dump the data //
    //---------------//

    if (opt_dump) {
        for (int x = 0; x < x_size; x++) {
            for (int y = 0; y < y_size; y++) {
                printf("%g\n", array[x][y]);
            }
        }
    }

    //----------------------//
    // find the map extrema //
    //----------------------//

    CMNode* node = colormap.GetHead();
    float min_map = node->value;
    node = colormap.GetTail();
    float max_map = node->value;

    //----------------------//
    // scale the input file //
    //----------------------//

    if (opt_scale)
    {
        //------------------------//
        // find the array extrema //
        //------------------------//

        float min_value = array[0][0];
        float max_value = min_value;
        for (x = 0; x < x_size; x++)
        {
            for (y = 0; y < y_size; y++)
            {
                if (array[x][y] < min_value)
                    min_value = array[x][y];
                if (array[x][y] > max_value)
                    max_value = array[x][y];
            }
        }

        //-------//
        // scale //
        //-------//

        float m = (max_map - min_map) / (max_value - min_value);
        float b = min_map - m * min_value;
        for (int x = 0; x < x_size; x++)
        {
            for (int y = 0; y < y_size; y++)
            {
                array[x][y] = m * array[x][y] + b;
            }
        }
    }

    //---------------------------------//
    // open the tiff file and set tags //
    //---------------------------------//

    TIFF* tif_fp = TIFFOpen(tiff_file, "w");
    TIFFSetField(tif_fp, TIFFTAG_IMAGELENGTH, (uint32)y_size);
    TIFFSetField(tif_fp, TIFFTAG_IMAGEWIDTH, (uint32)x_size);
    TIFFSetField(tif_fp, TIFFTAG_SAMPLESPERPIXEL, (uint32)3);
    TIFFSetField(tif_fp, TIFFTAG_BITSPERSAMPLE, (uint32)8);
    TIFFSetField(tif_fp, TIFFTAG_PLANARCONFIG, (uint32)PLANARCONFIG_SEPARATE);
    TIFFSetField(tif_fp, TIFFTAG_PHOTOMETRIC, (uint16)PHOTOMETRIC_RGB);
    TIFFSetField(tif_fp, TIFFTAG_ORIENTATION, (uint16)5);

    unsigned char *buf = NULL;

    buf = (unsigned char *)_TIFFmalloc(x_size);
    if (buf == NULL)
    {
        fprintf(stderr, "%s: error allocating scanline (%d)\n", command,
            x_size);
        exit(1);
    }

    for (c = 0; c < 3; c++) {
        for (y = 0; y < y_size; y++) {
            for (x = 0; x < x_size; x++) {
                unsigned char color[3];
                colormap.ConvertToColor(array[x][y_size - 1 - y], color);
                buf[x] = color[c];
            }
            TIFFWriteScanline(tif_fp, buf, y, c);
        }
    }

    _TIFFfree(buf);
    TIFFClose(tif_fp);

    //-------------//
    // free memory //
    //-------------//

    free_array(array, 2, x_size, y_size);

    if (! opt_dump) {
        printf("TIFF Written...\n");
        infowrite(x_size, y_size, tiff_file);
    }

    //----------------------//
    // generate a color bar //
    //----------------------//

/*
    if (opt_bar)
    {
        sprintf(filename, "%s.bar.%s", array_file, TIFF_EXTENSION);
        x_size = COLORBAR_WIDTH;
        y_size /= 2;    // half the image height
        array = (float **)make_array(sizeof(float), 2, x_size, y_size);
        if (array == NULL)
        {
            fprintf(stderr, "%s: error allocating colorbar array (%d x %d)\n",
                command, x_size, y_size);
            exit(1);
        }
        for (int y = 0; y < y_size; y++)
        {
            float fraction = (float)y / (float)(y_size - 1);
            float value = min_map + fraction * (max_map - min_map);
            for (int x = 0; x < x_size; x++)
            {
                array[x][y] = value;
            }
        }
        if (! write_tiff(array, x_size, y_size, &colormap, filename))
        {
            fprintf(stderr, "%s: error writing colorbar\n", command);
            infowrite(x_size, y_size, filename, stderr);
            exit(1);
        }
        free_array(array, 2, x_size, y_size);
        fprintf(stderr, "\nColorbar Written...\n");
        infowrite(x_size, y_size, filename);
    }
*/
    return (0);
}

//-----------//
// infowrite //
//-----------//

void
infowrite(
    int          x_size,
    int          y_size,
    const char*  filename,
    FILE*        fp)
{
    fprintf(fp, "   X Size : %d\n", x_size);
    fprintf(fp, "   Y Size : %d\n", y_size);
    fprintf(fp, "     File : %s\n", filename);
    return;
}
