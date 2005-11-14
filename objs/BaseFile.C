//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_basefile_c[] =
    "@(#) $Id$";

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include "BaseFile.h"

//==========//
// BaseFile //
//==========//

BaseFile::BaseFile()
:   _inputFilename(NULL), _outputFilename(NULL), _inputFp(NULL),
    _outputFp(NULL)
{
    return;
}

BaseFile::~BaseFile()
{
    CloseInputFile();
    CloseOutputFile();
    free(_inputFilename);
    free(_outputFilename);
    return;
}

//----------------------------//
// BaseFile::SetInputFilename //
//----------------------------//

int
BaseFile::SetInputFilename(
    const char*  filename)
{
    if (_inputFilename != NULL)
        free(_inputFilename);
    _inputFilename = strdup(filename);
    if (_inputFilename == NULL)
        return(0);
    return(1);
}

//-----------------------------//
// BaseFile::SetOutputFilename //
//-----------------------------//

int
BaseFile::SetOutputFilename(
    const char*  filename)
{
    if (_outputFilename != NULL)
        free(_outputFilename);
    _outputFilename = strdup(filename);
    if (_outputFilename == NULL)
        return(0);
    return(1);
}

//--------------------------//
// BaseFile::OpenForReading //
//--------------------------//

int
BaseFile::OpenForReading()
{
    if (_inputFilename == NULL)
        return(0);

    _inputFp = fopen(_inputFilename, "r");
    if (_inputFp == NULL)
        return(0);

    return(1);
}

int
BaseFile::OpenForReading(
    const char*  filename)
{
    if (! SetInputFilename(filename))
        return(0);

    if (! OpenForReading())
        return(0);

    return(1);
}

//--------------------------//
// BaseFile::OpenForWriting //
//--------------------------//

int
BaseFile::OpenForWriting()
{
    if (_outputFilename == NULL)
        return(0);

    _outputFp = fopen(_outputFilename, "w");
    if (_outputFp == NULL)
        return(0);

    return(1);
}

int
BaseFile::OpenForWriting(
    const char*  filename)
{
    if (! SetOutputFilename(filename))
        return(0);

    if (! OpenForWriting())
        return(0);

    return(1);
}

//---------------------------//
// BaseFile::RewindInputFile //
//---------------------------//

int
BaseFile::RewindInputFile()
{
    rewind(_inputFp);
    return(1);
}

//----------------//
// BaseFile::Read //
//----------------//

int
BaseFile::Read(
    char*   buffer,
    size_t  bytes)
{
    if (fread(buffer, bytes, 1, _inputFp) != 1)
        return(0);
    return(1);
}

//-----------------//
// BaseFile::Write //
//-----------------//

int
BaseFile::Write(
    char*   buffer,
    size_t  bytes)
{
    if (fwrite(buffer, bytes, 1, _outputFp) != 1)
        return(0);
    return(1);
}

//--------------------------//
// BaseFile::CloseInputFile //
//--------------------------//

int
BaseFile::CloseInputFile()
{
    if (_inputFp != NULL && _inputFp != stdin)
    {
        // only needs to be closed if it exists and is not stdin
        fclose(_inputFp);
    }
    _inputFp = NULL;
    return(1);
}

//---------------------------//
// BaseFile::CloseOutputFile //
//---------------------------//

int
BaseFile::CloseOutputFile()
{
    if (_outputFp != NULL && _outputFp != stdout)
    {
        // only needs to be closed if it exists and is not stdout
        fclose(_outputFp);
    }
    _outputFp = NULL;
    return(1);
}

//-----------------//
// BaseFile::Close //
//-----------------//

int
BaseFile::Close()
{
    CloseInputFile();
    CloseOutputFile();
    return(1);
}
