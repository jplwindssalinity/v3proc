//=========================================================//
// Copyright (C) 2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef JPEG_H
#define JPEG_H

static const char rcs_id_jpeg_h[] =
    "@(#) $Id$";

#include <stdio.h>
#include "List.h"

//======================================================================
// CLASSES
//    ColorMap, CMNode
//======================================================================

//======================================================================
// CLASS
//    CMNode
//
// DESCRIPTION
//    The CMNode object contains one mapping between a floating point
//    value and RGB
//======================================================================

class CMNode
{
public:

    //--------------//
    // construction //
    //--------------//

    CMNode();
    ~CMNode();

    //--------------//
    // input/output //
    //--------------//

    void  SetColor(unsigned char target_color[3]);
    int   InterpolateColor(CMNode* next_node, float target_value,
              unsigned char target_color[3]);

    //-----------//
    // operators //
    //-----------//

    int  operator==(CMNode c2);
    int  operator<(CMNode c2);
    int  operator>=(CMNode c2);

    //-----------//
    // variables //
    //-----------//

    float          value;
    unsigned char  color[3];    // red, green, blue
};

//======================================================================
// CLASS
//    ColorMap
//
// DESCRIPTION
//    The ColorMap object holds a mapping between float point values
//    and 24 bit color
//======================================================================

class ColorMap : public SortableList<CMNode>
{
public:

    //--------------//
    // construction //
    //--------------//

    ColorMap();
    ~ColorMap();

    void  FreeContents();

    //--------------//
    // input/output //
    //--------------//

    int  Read(const char* filename);
    int  BlackToWhite(float min_value, float max_value);

    //--------//
    // access //
    //--------//

    int  ConvertToColor(float value, unsigned char* color);

protected:
};

//================//
// JPEG functions //
//================//

int  write_jpeg(float** array, int x_size, int y_size, ColorMap* colormap,
         int quality, const char* output_file);

#endif
