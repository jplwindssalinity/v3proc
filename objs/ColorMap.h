//=========================================================//
// Copyright (C) 2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef COLORMAP_H
#define COLORMAP_H

static const char rcs_id_color_map_h[] =
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

    enum ColorIndexE { RED = 0, GREEN = 1, BLUE = 2 };

    //--------------//
    // construction //
    //--------------//

    CMNode();
    ~CMNode();

    //--------------//
    // input/output //
    //--------------//

    void  GetRGBArray(unsigned char target_color[3]);
    void  SetRGB(unsigned char r, unsigned char g, unsigned char b);
    void  GetRGB(unsigned char* r, unsigned char* g, unsigned char* b);
    void  SetHSV(float h, float s, float v);
    void  GetHSV(float* h, float* s, float* v);
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

#endif
