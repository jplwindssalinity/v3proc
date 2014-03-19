//==============================================================//
// Copyright (C) 2000-2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    jpeg
//
// SYNOPSIS
//    jpeg [ -bgs ] [ -c colormap ] [ -q # ] [ -x # ] [ -y # ]
//        <input_array> [ output_jpeg ]
//
// DESCRIPTION
//    Converts a 2-D array into a JPEG.
//
// OPTIONS
//    [ -b ]       Bar. Generate a color bar strip too. input_array.bar.jpeg
//    [ -g ]       Guess at the dimensions and ask.
//    [ -s ]       Scale. Linearly scale the array values to match
//                   the range of colors in the colormap.
//    [ -c colormap ]  Colormap. A colormap file to use. Colormap files
//                       are ASCII files containing (value, R, G, B) per line.
//                        A black to white colormap is the default.
//    [ -q # ]     Quality. A number from 0 to 100. Default is 100.
//    [ -x # ]     Specify the X dimension.
//    [ -y # ]     Specify the Y dimension.
//    [ output_jpeg ]  The output JPEG file. If not specified, output
//                     file is input_array.jpeg
//
// OPERANDS
//    <input_array>  The input array file.
//
// EXAMPLES
//    An example of a command line is:
//      % jpeg -q 100 -c fire.cmap -x 500 array.dat
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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include "Jpeg.h"
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

#define OPTSTRING       "bgsc:q:x:y:"
#define JPEG_EXTENSION  "jpeg"
#define COLORBAR_WIDTH  50

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

void  infowrite(int x_size, int y_size, int quality, const char* filename,
          FILE* fp = stdout);

//------------------//
// OPTION VARIABLES //
//------------------//

int opt_bar = 0;
int opt_guess = 0;
int opt_scale = 0;
int opt_rel = 0;

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -bgs ]", "[ -c colormap ]", "[ -q # ]",
    "[ -x # ]", "[ -y # ]", "<input_array>", "[ output_jpeg ]", 0 };

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
    int   quality = 100;

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
        case 'g':
            opt_guess = 1;
            break;
        case 's':
            opt_scale = 1;
            break;
        case 'c':
            colormap_file = optarg;
            break;
        case 'q':
            quality = atoi(optarg);
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

    if (argc < optind + 1)
        usage(command, usage_array, 1);

    const char* array_file = argv[optind++];
    const char* jpeg_file = NULL;
    if (optind < argc)
        jpeg_file = argv[optind++];

    //------------------------------//
    // generate an output file name //
    //------------------------------//

    char filename[1024];
    if (jpeg_file == NULL)
    {
        sprintf(filename, "%s.%s", array_file, JPEG_EXTENSION);
        jpeg_file = filename;
    }

    //-------------------------------//
    // determine the array file size //
    //-------------------------------//

    struct stat buf;
    if (stat(array_file, &buf) != 0)
    {
        fprintf(stderr, "%s: error stating file %s\n", command, array_file);
        exit(1);
    }
    off_t filesize = buf.st_size;
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

    for (int x = 0; x < x_size; x++)
    {
        if (fread(*(array + x), sizeof(float), y_size, ifp) !=
            (unsigned int)y_size)
        {
            fprintf(stderr, "%s: error reading array file %s\n", command,
                array_file);
            exit(1);
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
        for (int x = 0; x < x_size; x++)
        {
            for (int y = 0; y < y_size; y++)
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

    //----------------//
    // write the JPEG //
    //----------------//

    if (! write_jpeg(array, x_size, y_size, &colormap, quality, jpeg_file))
    {
        fprintf(stderr, "%s: error writing JPEG\n", command);
        infowrite(x_size, y_size, quality, jpeg_file, stderr);
        exit(1);
    }

    //-------------//
    // free memory //
    //-------------//

    free_array(array, 2, x_size, y_size);

    printf("JPEG Written...\n");
    infowrite(x_size, y_size, quality, jpeg_file);

    //----------------------//
    // generate a color bar //
    //----------------------//

    if (opt_bar)
    {
        sprintf(filename, "%s.bar.%s", array_file, JPEG_EXTENSION);
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
        if (! write_jpeg(array, x_size, y_size, &colormap, quality, filename))
        {
            fprintf(stderr, "%s: error writing colorbar\n", command);
            infowrite(x_size, y_size, quality, filename, stderr);
            exit(1);
        }
        free_array(array, 2, x_size, y_size);
        fprintf(stderr, "\nColorbar Written...\n");
        infowrite(x_size, y_size, quality, filename);
    }
    return (0);
}

//-----------//
// infowrite //
//-----------//

void
infowrite(
    int          x_size,
    int          y_size,
    int          quality,
    const char*  filename,
    FILE*        fp)
{
    fprintf(fp, "   X Size : %d\n", x_size);
    fprintf(fp, "   Y Size : %d\n", y_size);
    fprintf(fp, "  Quality : %d\n", quality);
    fprintf(fp, "     File : %s\n", filename);
    return;
}
