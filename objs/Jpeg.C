//=========================================================//
// Copyright (C) 2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

static const char rcs_id_jpeg_c[] =
    "@(#) $Id$";

#include <stdio.h>
#include <malloc.h>
#include </opt/local/include/jpeglib.h>
#include "Jpeg.h"
#include "Array.h"
#include "List.h"

//========//
// CMNode //
//========//

CMNode::CMNode()
{
    return;
}

CMNode::~CMNode()
{
    return;
}

//------------------//
// CMNode::SetColor //
//------------------//

void
CMNode::SetColor(
    unsigned char  target_color[3])
{
    target_color[0] = color[0];
    target_color[1] = color[1];
    target_color[2] = color[2];
    return;
}

//--------------------------//
// CMNode::InterpolateColor //
//--------------------------//

int
CMNode::InterpolateColor(
    CMNode*        next_node,
    float          target_value,
    unsigned char  target_color[3])
{
    if (next_node == NULL)
        return(0);

    float dx = next_node->value - value;
    for (int i = 0; i < 3; i++)
    {
        float m = ((float)next_node->color[i] - (float)color[i]) / dx;
        float y = m * (target_value - (float)value) + (float)color[i];
        int c = (int)(y + 0.5);
        if (c < 0) c = 0;
        if (c > 255) c = 255;
        target_color[i] = (unsigned char)c;
    }
    return(1);
}

//------------//
// CMNode::== //
//------------//

int
CMNode::operator==(CMNode c2)
{
    if (value == c2.value)
        return(1);
    else
        return(0);
}

//-----------//
// CMNode::< //
//-----------//

int
CMNode::operator<(CMNode c2)
{
    if (value < c2.value)
        return(1);
    else
        return(0);
}

//------------//
// CMNode::>= //
//------------//

int
CMNode::operator>=(CMNode c2)
{
    if (value >= c2.value)
        return(1);
    else
        return(0);
}

//==========//
// ColorMap //
//==========//

ColorMap::ColorMap()
{
    return;
}

ColorMap::~ColorMap()
{
	FreeContents();
    return;
}

//------------------------//
// ColorMap::FreeContents //
//------------------------//

void
ColorMap::FreeContents()
{
    CMNode* node;
    GotoHead();
    while ((node = RemoveCurrent()) != NULL)
        delete node;
    return;
}

//----------------//
// ColorMap::Read //
//----------------//

int
ColorMap::Read(
    const char*  filename)
{
    FILE* fp = fopen(filename, "r");
    if (fp == NULL)
        return(0);

    do
    {
        float value;
        int r, g, b;
        if (fscanf(fp, " %g %d %d %d", &value, &r, &g, &b) != 4)
        {
            if (feof(fp))
                break;
            else
                return(0);
        }

        CMNode* new_node = new CMNode();
        if (new_node == NULL)
            return(0);
        new_node->value = value;
        new_node->color[0] = (unsigned char)r;
        new_node->color[1] = (unsigned char)g;
        new_node->color[2] = (unsigned char)b;
        AddSorted(new_node);
    } while (1);

    return(1);
}

//------------------------//
// ColorMap::BlackToWhite //
//------------------------//

int
ColorMap::BlackToWhite(
    float  min_value,
    float  max_value)
{
    CMNode* new_node = new CMNode();
    if (new_node == NULL)
        return(0);
    new_node->value = min_value;
    new_node->color[0] = 0;
    new_node->color[1] = 0;
    new_node->color[2] = 0;
    AddSorted(new_node);

    new_node = new CMNode();
    if (new_node == NULL)
        return(0);
    new_node->value = max_value;
    new_node->color[0] = 255;
    new_node->color[1] = 255;
    new_node->color[2] = 255;
    AddSorted(new_node);

    return(1);
}

//--------------------------//
// ColorMap::ConvertToColor //
//--------------------------//

int
ColorMap::ConvertToColor(
    float           value,
    unsigned char*  color)
{
    //------------------------------//
    // check for head-side clipping //
    //------------------------------//

    CMNode* node = GetHead();
    if (node == NULL)
        return(0);

    if (value < node->value)
    {
        // value is less than the specified range
        // clip it
        node->SetColor(color);
        return(1);
    }

    //-------------------------//
    // check for interpolation //
    //-------------------------//

    node = GetNext();
    while (node != NULL)
    {
        if (node->value > value)
        {
            // value is in between two node
            CMNode* prev_node = GetPrev();
            return(prev_node->InterpolateColor(node, value, color));
        }
        node = GetNext();
    }

    //---------------------------------//
    // only tail-side clipping is left //
    //---------------------------------//

    node = GetTail();
    node->SetColor(color);
    return(1);
}

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
