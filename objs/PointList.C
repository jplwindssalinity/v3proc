//=========================================================//
// Copyright (C) 2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

static const char rcs_id_point_list_c[] =
    "@(#) $Id$";

#include "PointList.h"

//=======//
// Point //
//=======//

Point::Point()
:   cti(0), ati(0)
{
    return;
}

Point::Point(
    int  new_cti,
    int  new_ati)
{
    cti = new_cti;
    ati = new_ati;
    return;
}

//===========//
// PointList //
//===========//

PointList::PointList()
{
    return;
}

PointList::~PointList()
{
    return;
}

//---------------------//
// PointList::SetRange //
//---------------------//

void
PointList::SetRange(
    int  cti_min,
    int  cti_max,
    int  ati_min,
    int  ati_max)
{
    _minCti = cti_min;
    _maxCti = cti_max;
    _minAti = ati_min;
    _maxAti = ati_max;
    return;
}

//----------------------//
// PointList::AddWindow //
//----------------------//

int
PointList::AddWindow(
    Point*  p,
    int     window_size)
{
    return (AddWindow(p->cti, p->ati, window_size));
}
//----------------------//
// PointList::AddWindow //
//----------------------//

int
PointList::AddWindow(
    int  cti_center,
    int  ati_center,
    int  window_size)
{
    int half_window = window_size / 2;
    
    int cti_min = cti_center - half_window;
    if (cti_min < _minCti)
        cti_min = _minCti;

    int cti_max = cti_center + half_window + 1;
    if (cti_max > _maxCti)
        cti_max = _maxCti;
    
    int ati_min = ati_center - half_window;
    if (ati_min < _minAti)
        ati_min = _minAti;

    int ati_max = ati_center + half_window + 1;
    if (ati_max > _maxAti)
        ati_max = _maxAti;
    
    int count = 0;
    for (int cti = cti_min; cti < cti_max; cti++)
    {
        for (int ati = ati_min; ati < ati_max; ati++)
        {
            Point* p = new Point(cti, ati);
            Append(p);
            count++;
        }
    }
    return(count);
}
