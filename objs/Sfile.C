//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

static const char rcs_id_morefiles_c[] =
    "@(#) $Id$";

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "Sfile.h"
#include "List.h"

//------------//
// initialize //
//------------//

OpenList SFILE::_openList = OpenList();

//--------//
// sfopen //
//--------//

SFILE*
sfopen(
    const char*  filename,
    const char*  mode)
{
    //-----------------//
    // create an SFILE //
    //-----------------//

    SFILE* sf = new SFILE();
    if (sf == NULL)
        return(NULL);

    //---------------//
    // open the file //
    //---------------//

    if (! sf->Open(filename, mode))
    {
        delete sf;
        return(NULL);
    }

    return(sf);
}

//---------//
// sfclose //
//---------//

int
sfclose(
    SFILE*  sf)
{
    //----------------//
    // close the file //
    //----------------//

    if (! sf->Close())
        return(0);

    //-------------------//
    // destroy the SFILE //
    //-------------------//

    delete sf;

    return(1);
}

//=======//
// SFILE //
//=======//

SFILE::SFILE()
:   _filename(NULL), _mode(NULL), _fp(NULL), _direction(UNKNOWN),
    _readOffset(0)
{
    return;
}

SFILE::~SFILE()
{
    if (_filename)
        free(_filename);
    if (_mode)
        free(_mode);
    if (_fp)
        fclose(_fp);

    return;
}

//-------------//
// SFILE::Open //
//-------------//
// open a file, put it on the open list

int
SFILE::Open(
    const char*  filename,
    const char*  mode)
{
    //---------------//
    // open the file //
    //---------------//

    if (_fp != NULL)
        return(0);

    do
    {
        _fp = fopen(filename, mode);
        if (_fp != NULL)
            break;    // success
    } while (_HoldAFile());

    if (_fp == NULL)
        return(0);

    //--------------------------------//
    // remember the filename and mode //
    //--------------------------------//

    _filename = strdup(filename);
    if (_filename == NULL)
        return(0);

    _mode = strdup(mode);
    if (_mode == NULL)
        return(0);

    //-----------------------------//
    // determine read/write status //
    //-----------------------------//

    if (strchr(_mode, (int)'r') != NULL)
        _direction = READ;
    else if (strchr(_mode, (int)'w') != NULL)
        _direction = WRITE;
    else if (strchr(_mode, (int)'a') != NULL)
        _direction = WRITE;

    //------------------------//
    // add it to the OpenList //
    //------------------------//

    if (! Append())
        return(0);

    return(1);
}

//-------------//
// SFILE::Hold //
//-------------//
// close a file, remove from open list, keep info

int
SFILE::Hold()
{
    //---------------------------------//
    // if reading, remember the offset //
    //---------------------------------//

    if (_direction == READ)
    {
        _readOffset = ftell(_fp);
    }

    //----------------//
    // close the file //
    //----------------//

    if (_fp != NULL)
        fclose(_fp);
    _fp = NULL;

    //-----------------------------//
    // remove it from the OpenList //
    //-----------------------------//

    for (SFILE* sf = _openList.GetHead(); sf; sf = _openList.GetNext())
    {
        if (sf == this)
        {
            _openList.RemoveCurrent();
            return(1);
        }
    }

    return(0);    // wasn't in the OpenList error
}

//--------------//
// SFILE::Close //
//--------------//
// close a file, remove from the open list, get rid of info

int
SFILE::Close()
{
    if (! Hold())
        return(0);

    //----------------------------//
    // free the filename and mode //
    //----------------------------//

    if (_filename != NULL)
        free(_filename);

    if (_mode != NULL)
        free(_mode);

    //-----------------------//
    // clear other variables //
    //-----------------------//

    _direction = UNKNOWN;
    _readOffset = 0;

    return(0);
}

//---------------//
// SFILE::Append //
//---------------//

int
SFILE::Append()
{
    return (_openList.Append(this));
}

//-----------//
// SFILE::fp //
//-----------//

FILE*
SFILE::fp()
{
    //----------------------------------------------//
    // if the file is open, just return the pointer //
    //----------------------------------------------//

    if (_fp != NULL)
        return(_fp);

    //--------------------------------------------//
    // open the file, holding others as necessary //
    //--------------------------------------------//

    do
    {
        if (_ReOpen())
            return(_fp);
    } while (_HoldAFile());

    return(NULL);
}

//----------------//
// SFILE::_ReOpen //
//----------------//

int
SFILE::_ReOpen()
{
    switch (_direction)
    {
    case WRITE:
        // open for appending
        _fp = fopen(_filename, "a");
        if (_fp == NULL)
            return(0);
        break;
    case READ:
        // open for reading, seek to last place
        _fp = fopen(_filename, "r");
        if (_fp == NULL)
            return(0);
        if (fseek(_fp, _readOffset, SEEK_SET) != 0)
            return(0);
        break;
    default:
        return(0);
        break;
    }
    return(1);
}

//-------------------//
// SFILE::_HoldAFile //
//-------------------//

int
SFILE::_HoldAFile()
{
    // put the head file on hold
    SFILE* sf = _openList.GetHead();
    if (sf == NULL)
        return(0);

    return(sf->Hold());
}

//==========//
// OpenList //
//==========//

OpenList::OpenList()
{
    return;
}

OpenList::~OpenList()
{
    return;
}
