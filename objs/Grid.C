//==============================================================//
// Copyright (C) 1997-1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_grid_c[] =
    "@(#) $Id$";

#include <stdio.h>
#include <malloc.h>
#include "Grid.h"
#include "Meas.h"
#include "Sigma0.h"


//======//
// Grid //
//======//

Grid::Grid()
:   _crosstrack_res(0.0), _alongtrack_res(0.0), _crosstrack_size(0.0),
    _alongtrack_size(0.0), _crosstrack_bins(0), _alongtrack_bins(0),
    _start_time(0.0), _end_time(0.0), _max_vati(0), _ati_start(0),
    _ati_offset(0), _orbit_period(0.0), _grid(NULL)
{
    return;
}

Grid::~Grid()
{
    if (_grid != NULL)
    {
        free_array(_grid, 2, _crosstrack_bins, _alongtrack_bins);
    }
    return;
}

//--------------------//
// Grid::SetStartTime //
//--------------------//

int
Grid::SetStartTime(
    double  start_time)
{
    _start_time = start_time;
    l2a.header.startTime = _start_time;

    if (ephemeris.GetPosition(_start_time,EPHEMERIS_INTERP_ORDER,
        &_start_position) == 0)
    {
        fprintf(stderr,
            "Grid::SetStartTime: Grid start time is out of ephemeris range\n");
        exit(1);
    }

    _start_position = _start_position.Nadir();
    return(1);
}

//------------------//
// Grid::SetEndTime //
//------------------//

int
Grid::SetEndTime(
    double  end_time)
{
    _end_time = end_time;
//    l2a.header.endTime = _end_time;

    //--------------------------------------------------------------//
    // Determine corresponding along track index (in virtual grid).
    //--------------------------------------------------------------//

    EarthPosition r;
    if (ephemeris.GetPosition(_end_time,EPHEMERIS_INTERP_ORDER,&r) == 0)
    {
        fprintf(stderr,
            "Grid::SetEndTime: Grid end time is out of ephemeris range\n");
        exit(1);
    }

    float ctd, atd;
    if (ephemeris.GetSubtrackCoordinates(r, _start_position, _start_time,
        _end_time, &ctd, &atd) == 0)
    {
        fprintf(stderr,
            "Grid::SetEndTime: No subtrack coordinates for grid end time\n");
        exit(1);
    }

    _max_vati = (int) (atd/_alongtrack_res);    // virtual along track index.

    if (_max_vati < 0)
    {
        fprintf(stderr,
            "Grid::SetEndTime: Grid ends before grid starts -> no output\n");
    }

    return(1);
}

//----------------//
// Grid::Allocate //
//----------------//

int
Grid::Allocate(
    double  crosstrack_res,
    double  alongtrack_res,
    double  crosstrack_size,
    double  alongtrack_size)
{
    _alongtrack_res = alongtrack_res;
    _crosstrack_res = crosstrack_res;
    _alongtrack_size = alongtrack_size;
    _crosstrack_size = crosstrack_size;
    _alongtrack_bins = (int) (_alongtrack_size / _alongtrack_res);
    _crosstrack_bins = (int) (_crosstrack_size / _crosstrack_res);

    // Put data into L2A object for use in header.
    l2a.header.crossTrackResolution = _crosstrack_res;
    l2a.header.alongTrackResolution = _alongtrack_res;
    l2a.header.crossTrackBins = _crosstrack_bins;
    l2a.header.alongTrackBins = _alongtrack_bins;
    l2a.header.zeroIndex = (int) (_crosstrack_size/2.0/_crosstrack_res + 0.5);

    //
    // Set circular buffer tracking parameters to the beginning of the
    // virtual grid.
    //

    _ati_start = 0;
    _ati_offset = 0;

    _grid = (OffsetListList**)make_array(sizeof(OffsetListList), 2,
        _crosstrack_bins, _alongtrack_bins);
    if (_grid == NULL)
        return(0);

    // Make an empty OffsetListList object,
    // and use to initialize each grid element
    OffsetListList empty_list;
    for (int i=0; i < _crosstrack_bins; i++)
    {
        for (int j=0; j < _alongtrack_bins; j++)
        {
            _grid[i][j] = empty_list;
        }
    }

    return(1);
}

//-----------//
// Grid::Add //
//-----------//
// Add the measurement to the grid using an offset list.
// meas = pointer to the Meas object to grid
// meas_time = time at which the measurement was made
// spot_id = unique integer for each measurement spot - used for compositing.
// do_composite = 1 if slices should be composited, 0 if not.

int
Grid::Add(
    Meas*   meas,
    double  meas_time,
    long    spot_id,
    int     do_composite)
{
    //----------------------------------//
    // calculate the subtrack distances //
    //----------------------------------//

    float ctd, atd;
    if (ephemeris.GetSubtrackCoordinates(meas->centroid, _start_position,
        _start_time, meas_time, &ctd, &atd) == 0)
    {
        return(0);  // Couldn't find a grid position, so dump this measurement.
    }

    //
    // Compute grid indices, noting that the cross track grid starts on the left
    // side at cti = 0.
    //

    int cti = (int) ((ctd + _crosstrack_size/2.0)/_crosstrack_res + 0.5);
    int vati = (int) (atd/_alongtrack_res);    // virtual along track index.

    if ((cti >= _crosstrack_bins) || (cti < 0))
    {
        fprintf(stderr, "Grid::Add: crosstrack index = %d out of range\n",
            cti);
        return(0);
    }

    //
    // Negative vati means the point falls before the defined grid start.
    // For this special case, Add() returns success, but dumps the measurement.
    // If vati falls inside the defined grid, but in a portion that has been
    // already output, then an error message is generated (see below).
    //

    if (vati < 0)
    {
//        delete meas;
        return(1);
    }

    if (vati > _max_vati)
    {    // beyond end of grid, so do nothing.
        return(1);
    }

    //
    // Note that reverse shifting is not implemented.
    // Thus, if vati falls before the earliest row in memory, it is considered
    // out of range instead of trying to back up the buffer.  This limitation
    // is imposed by BufferedList.
    //

    if (vati < _ati_offset)
    {
        fprintf(stderr, "Grid::Add: alongtrack index = %d out of range\n",
            vati);
        return(0);
    }

    //
    // Determine if the along track index is in memory or not.
    // If not, read and write rows until it is in range.
    //

    while (vati - _ati_offset >= _alongtrack_bins)
    {
        // vati is beyond latest row, so need to shift the grid buffer
        ShiftForward(do_composite);
    }

    //
    // Convert the along track index into the virtual buffer (vati) to an
    // index into the grid in memory (ati).
    //

    int ati = (vati - _ati_offset + _ati_start) % _alongtrack_bins;

    // convert the measurement to an offset

    long* offset = new long;
    *offset = meas->offset;

    OffsetList* offsetlist = _grid[cti][ati].GetHead();

    if (do_composite == 1)
    {
        // Composite slices that fall in the same grid location and spot.
        // Scan for an offset list with the same spot id.
        while (offsetlist != NULL)
        {
            if (offsetlist->spotId == spot_id)
            {
                break;
            }
            offsetlist = _grid[cti][ati].GetNext();
        }

        if (offsetlist == NULL)
        {
            // Append a new offsetlist (with the new offset) for the new spot
            offsetlist = new OffsetList;
            offsetlist->Append(offset);
            offsetlist->spotId = spot_id;
            _grid[cti][ati].Append(offsetlist);
        }
        else
        {
            // Append the offset in the list with the matching spot id.
            offsetlist->Append(offset);
        }
    }
    else
    {
        // Put all slices in the first (and only) offset list at this grid loc.
        if (offsetlist == NULL)
        {
            // Need to create a sublist and attach to the grid square
            offsetlist = new OffsetList;
            if (offsetlist == NULL)
            {
                fprintf(stderr, "Grid::Add: error allocating memory\n");
                exit(1);
            }
            _grid[cti][ati].Append(offsetlist);
        }
        offsetlist->Append(offset);
    }

    //printf("%d %d %f %f\n",cti,vati,ctd,atd);
    return(1);
}

//
// Grid::ShiftForward
//
// Shift the grid of measurements by one alongtrack row.
// The earliest row is written out to the output file and its storage
// is reset to receive another row.  The circular buffer tracking indices
// are updated appropriately so that the newly freed row is mapped to
// the next along track index.
// If requested, slices are composited before output.
// The slices to be composited should all be together in a sublist of their
// grid location.
//
// Inputs:
//    do_composite = flag set to 1 if compositing is desired, 0 otherwise.

int
Grid::ShiftForward(
    int  do_composite)
{
    //---------------------------//
    // remember the L1B location //
    //---------------------------//

    FILE* fp = l1b.GetInputFp();
    long offset = ftell(fp);
    if (offset == -1)
        return(0);

    MeasList spot_measList;

    // Write out the earliest row of measurement lists.
    for (int i=0; i < _crosstrack_bins; i++)
    {
        //----------------------------------------//
        // convert each offset list to a MeasList //
        //----------------------------------------//

        l2a.frame.measList.FreeContents();

        if (do_composite == 1)
        {
            for (OffsetList* offsetlist = _grid[i][_ati_start].GetHead();
                offsetlist; offsetlist = _grid[i][_ati_start].GetNext())
            {
                // each sublist is composited before output
                offsetlist->MakeMeasList(fp, &spot_measList);
                Meas* meas = new Meas;
                if (! meas->Composite(&spot_measList))
                {
                    fprintf(stderr, "Grid::ShiftForward: Error compositing\n");
                    delete meas;
                    return(0);
                }
                spot_measList.FreeContents();
                if (! l2a.frame.measList.Append(meas))
                {
                    fprintf(stderr,
                        "Grid::ShiftForward: Error forming list for output\n");
                    delete meas;
                    return(0);
                }
            }
        }
        else
        {
            OffsetList* offsetlist = _grid[i][_ati_start].GetHead();
            if (offsetlist != NULL)
            {
                offsetlist->MakeMeasList(fp, &(l2a.frame.measList));
            }
        }

        //----------------------------------//
        // complete and write the L2A frame //
        //----------------------------------//

        l2a.frame.rev = 0;
        l2a.frame.cti = i;
        l2a.frame.ati = _ati_offset;
        if (l2a.frame.measList.GetHead() != NULL)
        {
            l2a.WriteDataRec();
        }

        //----------------------//
        // free the offset list //
        //----------------------//

        _grid[i][_ati_start].FreeContents();
    }

    // Update buffer indices.
    _ati_start = (_ati_start + 1) % _alongtrack_bins;
    _ati_offset++;

    //----------------------//
    // restore L1B location //
    //----------------------//

    if (fseek(fp, offset, SEEK_SET) == -1)
        return(0);

    return(1);
}

//-------------//
// Grid::Flush //
//-------------//
// Write out all the grid rows in memory.

int
Grid::Flush(
    int  do_composite)
{
    for (int i = 0; i < _alongtrack_bins; i++)
    {
        ShiftForward(do_composite);
    }

    return(1);
}
