//=========================================================//
// Copyright (C) 2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

static const char rcs_id_color_map_c[] =
    "@(#) $Id$";

#include <stdio.h>
#include <malloc.h>
#include <math.h>
#include <string.h>
#include "ColorMap.h"
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

//---------------------//
// CMNode::GetRGBArray //
//---------------------//

void
CMNode::GetRGBArray(
    unsigned char  target_color[3])
{
    target_color[0] = color[0];
    target_color[1] = color[1];
    target_color[2] = color[2];
    return;
}

//----------------//
// CMNode::SetRGB //
//----------------//

void
CMNode::SetRGB(
    unsigned char  r,
    unsigned char  g,
    unsigned char  b)
{
    color[RED] = r;
    color[GREEN] = g;
    color[BLUE] = b;
    return;
}

//----------------//
// CMNode::GetRGB //
//----------------//

void
CMNode::GetRGB(
    unsigned char*  r,
    unsigned char*  g,
    unsigned char*  b)
{
    *r = color[RED];
    *g = color[GREEN];
    *b = color[BLUE];
    return;
}

//----------------//
// CMNode::SetHSV //
//----------------//

void
CMNode::SetHSV(
    float  h,
    float  s,
    float  v)
{
    while (h < 0.0)
        h += 360.0;
    while (h > 360.0)
        h -= 360.0;

    if (s == 0.0)
    {
        // achromatic (grey)
        color[RED] = color[GREEN] = color[BLUE]
            = (unsigned char)(255.0 * v + 0.5);
        return;
    }

    h /= 60.0;    // sector 0 to 5
    int i = (int)floor(h);
    float f = h - (float)i;   // factorial part of h
    float p = v * (1.0 - s);
    float q = v * (1.0 - s * f);
    float t = v * (1.0 - s * (1.0 - f));

    float fr, fg, fb;
    switch (i)
    {
    case 0:
        fr = v; fg = t; fb = p;
        break;
    case 1:
        fr = q; fg = v; fb = p;
        break;
    case 2:
        fr = p; fg = v; fb = t;
        break;
    case 3:
        fr = p; fg = q; fb = v;
        break;
    case 4:
        fr = t; fg = p; fb = v;
        break;
    default:
        fr = v; fg = p; fb = q;
        break;
    }
    color[RED] = (unsigned char)(255.0 * fr + 0.5);
    color[GREEN] = (unsigned char)(255.0 * fg + 0.5);
    color[BLUE] = (unsigned char)(255.0 * fb + 0.5);

    return;
}

//----------------//
// CMNode::GetHSV //
//----------------//

void
CMNode::GetHSV(
    float*  h,
    float*  s,
    float*  v)
{
    unsigned char min = 255;
    unsigned char max = 0;
    for (int i = 0; i < 3; i++)
    {
        if (color[i] < min)
            min = color[i];
        if (color[i] > max)
            max = color[i];
    }
    *v = (float)max / 255.0;
    float delta = (float)(max - min);
    if (max != 0)
    {
        *s = delta / (float)max;
    }
    else
    {
        // r = g = b = 0                // s = 0, v is undefined
        *s = 0.0;
        *h = -1.0;
        return;
    }

    if (color[RED] == max)
        *h = ((float)color[GREEN] - (float)color[BLUE]) / delta;
    else if(color[GREEN] == max)
        *h = 2.0 + ((float)color[BLUE] - (float)color[RED]) / delta;
    else
        *h = 4.0 + ((float)color[RED] - (float)color[GREEN]) / delta;

    *h *= 60.0;
    if (*h < 0.0)
        *h += 360.0;

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

    char line[1024];

    while (fgets(line, 1024, fp) != NULL)
    {
        float value;
        int r, g, b, chars_read;
        if (sscanf(line, " %g %d %d %d%n",
            &value, &r, &g, &b, &chars_read) != 4)
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

        // wipe out the carriage return
        char* rest_of_line = line + chars_read;
        char* end = strrchr(rest_of_line, '\n');
        if (end != NULL)
            *end = '\0';

        // loop over modifiers
        char* mod = strtok(rest_of_line, " ");
        while (mod != NULL)
        {
            float value;
            if (sscanf(mod, "h+%g", &value) == 1)
            {
                float h, s, v;
                new_node->GetHSV(&h, &s, &v);
                h += value;
                new_node->SetHSV(h, s, v);
            }
            if (sscanf(mod, "h-%g", &value) == 1)
            {
                float h, s, v;
                new_node->GetHSV(&h, &s, &v);
                h -= value;
                new_node->SetHSV(h, s, v);
            }
            if (sscanf(mod, "v*%g", &value) == 1)
            {
                float h, s, v;
                new_node->GetHSV(&h, &s, &v);
                v *= value;
                new_node->SetHSV(h, s, v);
            }
            if (sscanf(mod, "s*%g", &value) == 1)
            {
                float h, s, v;
                new_node->GetHSV(&h, &s, &v);
                s *= value;
                new_node->SetHSV(h, s, v);
            }
            mod = strtok(NULL, " ");
        }

        AddSorted(new_node);
    }

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
        node->GetRGBArray(color);
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
    node->GetRGBArray(color);
    return(1);
}
