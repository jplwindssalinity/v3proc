//=========================================================//
// Copyright  (C)1996, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//
// CM Log
//
// $Log$
// 
//    Rev 1.1   01 May 1998 13:18:14   daffer
// Added pvcs keywords
//
// $Date$
// $Revision$
// $Author$
//
//=========================================================//

static const char rcs_id[] =
    "@(#) $Id$";

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "TlmFile.h"

//=========//
// TlmFile //
//=========//

TlmFile::TlmFile(
    const char*     filename)
:   _fd(-1), _status(OK), _filename(0), _fileTime(INVALID_TIME)
{
    _status = OK;
    _DupFilename(filename);
    return;
}

TlmFile::~TlmFile()
{
    if (_filename)
        free(_filename);
    return;
}

//-------------//
// GetDataHdrs //
//-------------//
// since this is only valid for L1, don't make others define it
TlmFile::ContentsE
TlmFile::GetDataHdrs(
    char*   data_hdrs)
{
    data_hdrs;  // make compiler happy
    return(TlmFile::NOTHING);
}

//--------------//
// _DupFilename //
//--------------//
// duplicate the filename string for a local copy

TlmFile::StatusE
TlmFile::_DupFilename(
    const char*     filename)
{
    _filename = strdup(filename);
    if (! _filename)
        _status = ERROR_ALLOCATING_FILENAME;
    return(_status);
}

//-------------//
// _SetOffsets //
//-------------//
// set the first and last data records offsets.
// if either offset cannot be set, both offsets are set to -1.

TlmFile::StatusE
TlmFile::_SetOffsets(
    const int       data_rec_size,
    const off_t     first_offset,
    const off_t     last_offset,
    ExtractFunc     time_func,
    const Itime     start_time,
    const Itime     end_time,
    off_t*          first_used_offset,
    off_t*          last_used_offset,
    Itime*          first_used_time,
    Itime*          last_used_time)
{
    // initialize
    *first_used_offset = -1;
    *last_used_offset = -1;
    *first_used_time = INVALID_TIME;
    *last_used_time = INVALID_TIME;

    // check the times
    if (start_time != INVALID_TIME && end_time != INVALID_TIME &&
        start_time > end_time)
    {
        return(_status);
    }

    // allocate for a data record
    char* data_rec = (char *)malloc(data_rec_size);
    if (! data_rec)
        return(_status = ERROR_ALLOCATING_TIMESEARCH);

    // initialize the original offsets
    off_t current_first_offset = first_offset;
    off_t current_last_offset = last_offset;

    // search for the start time
    off_t new_first_offset = _BinarySearch(start_time, data_rec_size,
        data_rec, current_first_offset, current_last_offset, time_func,
        SEARCH_START, first_used_time);
    if (new_first_offset == -1)
    {
        free(data_rec);
        return(_status);
    }
    current_first_offset = new_first_offset;    // make the 2nd search faster

    // search for the end time
    off_t new_last_offset = _BinarySearch(end_time, data_rec_size, data_rec,
        current_first_offset, current_last_offset, time_func, SEARCH_END,
        last_used_time);
    if (new_last_offset == -1)
    {
        free(data_rec);
        return(_status);
    }

    *first_used_offset = new_first_offset;
    *last_used_offset = new_last_offset;

    free(data_rec);
    return(_status);
}

//---------------//
// _BinarySearch //
//---------------//

off_t
TlmFile::_BinarySearch(
    const Itime     target_time,
    int             data_rec_size,
    char*           data_rec,
    off_t           first_offset,
    off_t           last_offset,
    ExtractFunc     time_func,
    int             target_type,
    Itime*          found_time)
{
    //-------------------------//
    // first do a simple check //
    //-------------------------//

    // determine the first and last time
    Itime first_time, last_time;
    if (_GetTime(first_offset, data_rec_size, data_rec, time_func, &first_time)
        != OK)
    {
        return(-1);
    }
    if (_GetTime(last_offset - data_rec_size, data_rec_size, data_rec,
        time_func, &last_time) != OK)
    {
        return(-1);
    }

    // check based on required target_type
    if (target_type == SEARCH_START)
    {
        if (first_time >= target_time || target_time == INVALID_TIME)
        {
            *found_time = first_time;
            return(first_offset);
        }
        if (last_time < target_time)
            return(-1);
    }
    else if (target_type == SEARCH_END)
    {
        if (last_time <= target_time || target_time == INVALID_TIME)
        {
            *found_time = last_time;
            return(last_offset);
        }
        if (first_time > target_time)
            return(-1);
    }
    else
    {
        _status = UNKNOWN_CONDITION;
        return(-1);
    }

    //-------------------------------------------//
    // if simple check fails, do a binary search //
    //-------------------------------------------//

    int low = 0;
    int mid;
    int high = (last_offset - first_offset) / data_rec_size - 1;
    off_t offset;
    Itime time;
    while ((mid = (low + high) / 2) != low)
    {
        offset = first_offset + mid * data_rec_size;
        if (_GetTime(offset, data_rec_size, data_rec, time_func, &time) != OK)
            return(-1);
        if (time > target_time)
            high = mid;
        else
            low = mid;
    }

    // answer is either low or high, select the right one
    off_t low_offset = first_offset + low * data_rec_size;
    Itime low_time;
    if (_GetTime(low_offset, data_rec_size, data_rec, time_func, &low_time)
        != OK)
    {
        return(-1);
    }

    off_t high_offset = first_offset + high * data_rec_size;
    Itime high_time;
    if (_GetTime(high_offset, data_rec_size, data_rec, time_func, &high_time)
        != OK)
    {
        return(-1);
    }

    if (target_type == SEARCH_START)
    {
        if (target_time > high_time)
            return(-1);
        else if (target_time > low_time)
        {
            offset = high_offset;
            *found_time = high_time;
        }
        else
        {
            offset = low_offset;
            *found_time = low_time;
        }
    }
    else if (target_type == SEARCH_END)
    {
        if (target_time < low_time)
            return(-1);
        else if (target_time < high_time)
        {
            offset = low_offset;
            *found_time = low_time;
        }
        else
        {
            offset = high_offset;
            *found_time = high_time;
        }
    }
    else
    {
        _status = UNKNOWN_CONDITION;
        return(-1);
    }

    return(offset);
}

//----------//
// _GetTime //
//----------//

TlmFile::StatusE
TlmFile::_GetTime(
    off_t           offset,
    int             data_rec_size,
    char*           data_rec_buffer,
    ExtractFunc     time_func,
    Itime*          time)
{
    if (lseek(_fd, offset, SEEK_SET) != offset)
        return(_status = ERROR_SEEKING_TIMESEARCH);
    if (read(_fd, data_rec_buffer, data_rec_size) != data_rec_size)
        return(_status = ERROR_READING_TIMESEARCH);
    if (! time_func(data_rec_buffer, (char *)time))
        return(_status = ERROR_EXTRACTING_TIMESEARCH);
    return(_status);
}
