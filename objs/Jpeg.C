//=========================================================//
// Copyright (C) 2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

static const char rcs_id_jpeg_c[] =
    "@(#) $Id$";

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <jpeglib.h>
#include "Jpeg.h"

//================//
// JPEG functions //
//================//

int
write_jpeg(
    float**      array,
    int          x_size,
    int          y_size,
    ColorMap*    colormap,
    int          quality,
    const char*  output_file)
{
    //--------------------//
    // create a scan line //
    //--------------------//

    int size = 3 * x_size * sizeof(JSAMPLE);
    JSAMPLE* scan_line = (JSAMPLE *)malloc(size);
    if (scan_line == NULL)
        return(0);

    //-----------------//
    // generate a JPEG //
    //-----------------//

    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    JSAMPROW row_pointer[1];
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    //-----------------//
    // set output file //
    //-----------------//

    FILE* ofp = fopen(output_file, "w");
    if (ofp == NULL)
        return(0);

    jpeg_stdio_dest(&cinfo, ofp);

    //----------------------------//
    // set compression parameters //
    //----------------------------//

    cinfo.image_width = x_size;
    cinfo.image_height = y_size;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE);

    //----------//
    // compress //
    //----------//

    jpeg_start_compress(&cinfo, TRUE);

    int height_idx = y_size - 1;
    while (cinfo.next_scanline < cinfo.image_height)
    {
        // convert scanline
        // this is done in reverse order to make the image match
        // the x-y plotting convention instead of the image convention
        row_pointer[0] = scan_line;
        for (int width_idx = 0; width_idx < x_size; width_idx++)
        {
            unsigned char color[3];
            colormap->ConvertToColor(array[width_idx][height_idx], color);
            for (int i = 0; i < 3; i++)
            {
                int i_idx = width_idx * 3 + i;
                scan_line[i_idx] = color[i];
            }
        }
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
        height_idx--;
    }
    jpeg_finish_compress(&cinfo);
    fclose(ofp);
    jpeg_destroy_compress(&cinfo);

    //-------------//
    // free memory //
    //-------------//

    free(scan_line);

    return(1);
}
