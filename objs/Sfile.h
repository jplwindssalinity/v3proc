//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef SFILE_H
#define SFILE_H

static const char rcs_id_sfile_h[] =
    "@(#) $Id$";

#include <stdio.h>
#include "List.h"

//======================================================================
// CLASSES
//    SFILE
//======================================================================

//======================================================================
// CLASS
//    SFILE
//
// DESCRIPTION
//    SFILE's are like FILE's but they allow you to virtually keep
//    open an unlimited number of files.  The only thing you need
//    to remember to do is to convert the SFILE* to a FILE*
//    immediately before accessing the file.  Do that every time.
//======================================================================

class OpenList;

class SFILE
{
public:
    SFILE();
    ~SFILE();

    int    Open(const char* filename, const char* mode);
    int    Hold();
    int    Close();
    int    Append();
    FILE*  fp();

protected:
    enum DirectionE { UNKNOWN, WRITE, READ };

    int           _ReOpen();
    int           _HoldAFile();

    char*         _filename;
    char*         _mode;
    FILE*         _fp;

    DirectionE    _direction;
    long          _readOffset;

    static OpenList  _openList;
};

SFILE*  sfopen(const char* filename, const char* mode);
int     sfclose(SFILE* sf);

//======================================================================
// CLASS
//    OpenList
//
// DESCRIPTION
//    The OpenList object holds the list of open SFILE's.  It needs
//    them so that if you are trying to write to a file that is on
//    hold, it can find another file to put on hold so it can give
//    you access to your file.
//======================================================================

class OpenList : public List<SFILE>
{
public:
    OpenList();
    ~OpenList();
};

#endif
