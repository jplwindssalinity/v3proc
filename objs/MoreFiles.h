//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef MOREFILES_H
#define MOREFILES_H

static const char rcs_id_morefiles_h[] =
    "@(#) $Id$";

#include <stdio.h>
#include "List.h"

//======================================================================
// CLASSES
//    MoreFiles
//======================================================================

//======================================================================
// CLASS
//    FilePack
//
// DESCRIPTION
//    The FilePack object holds information about a single file.  It is
//    used by MoreFiles.
//======================================================================

class FilePack
{
public:
    FilePack();
    ~FilePack();

    int  SetFilenameAndMode(const char* file_name, const char* file_mode);

    char*  filename;
    char*  mode;
    FILE*  fp;
};

//======================================================================
// CLASS
//    MoreFiles
//
// DESCRIPTION
//    The MoreFiles object allows you to have many files (virtually)
//    open.  Only works for writing now.
//======================================================================

typedef FilePack*  FileToken;

class MoreFiles
{
public:
    MoreFiles();
    ~MoreFiles();

    FileToken  Open(const char* filename, const char* mode);
    int        Close(FileToken token);
    FILE*      TokenToFp(FileToken token);
    int        Hold();

protected:
    int             _OpenHold(FilePack* filepack);

    List<FilePack>  _fullList;
    List<FilePack>  _openList;
};

#endif
