//=========================================================//
// Copyright (C) 2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef POINTLIST_H
#define POINTLIST_H

static const char rcs_id_point_list_h[] =
    "@(#) $Id$";

#include <stdio.h>
#include "List.h"

//======================================================================
// CLASSES
//    Point, PointList
//======================================================================

//======================================================================
// CLASS
//    Point
//
// DESCRIPTION
//    The Point class holds an along track and cross track index.
//======================================================================

class Point
{
public:
    Point();
    Point(int cti, int ati);

    int cti;
    int ati;
};

//======================================================================
// CLASS
//    PointList
//
// DESCRIPTION
//    The PointList holds a list of points. It also assists with things
//    like adding neighbors within a window to the list.
//======================================================================

class PointList : public List<Point>
{
public:

    //--------------//
    // construction //
    //--------------//

    PointList();
    ~PointList();

    void  SetRange(int cti_min, int cti_max, int ati_min, int ati_max);
    int   AddWindow(Point* p, int window_size);
    int   AddWindow(int cti, int ati, int window_size);

private:
    int  _minCti;
    int  _maxCti;
    int  _minAti;
    int  _maxAti;
};

#endif
