//==============================================================//
// Copyright (C) 1997-2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#ifndef BASEFILE_H
#define BASEFILE_H

static const char rcs_id_basefile_h[] =
    "@(#) $Id$";

#include <stdio.h>

//======================================================================
// CLASSES
//    BaseFile
//======================================================================


//======================================================================
// CLASS
//    BaseFile
//
// DESCRIPTION
//    The BaseFile object is used to easily handle opens, closes,
//    reads, and writes for an input and an output stream.
//======================================================================

class BaseFile
{
public:

    //--------------//
    // construction //
    //--------------//

    BaseFile();
    ~BaseFile();

    //---------------------//
    // setting and getting //
    //---------------------//

    int    SetInputFilename(const char* filename);
    int    SetOutputFilename(const char* filename);
    char*  GetInputFilename() { return(_inputFilename); };
    char*  GetOutputFilename() { return(_outputFilename); };
    FILE*  GetInputFp() { return(_inputFp); };
    FILE*  GetOutputFp() { return(_outputFp); };
    void   SetInputFp(FILE* fp) { _inputFp = fp; };
    void   SetOutputFp(FILE* fp) { _outputFp = fp; };

    //--------------//
    // input/output //
    //--------------//

    int  OpenForReading();
    int  OpenForReading(const char* filename);
    int  OpenForWriting();
    int  OpenForWriting(const char* filename);

    int  RewindInputFile();
    int  Read(char* buffer, size_t bytes);
    int  Write(char* buffer, size_t bytes);

    int  CloseInputFile();
    int  CloseOutputFile();
    int  Close();

    //--------//
    // status //
    //--------//

    int  EndOfFile() { return(feof(_inputFp)); };
    int  InputError() { return(ferror(_inputFp)); };
    int  OutputError() { return(ferror(_outputFp)); };

protected:

    //-----------//
    // variables //
    //-----------//

    char*  _inputFilename;
    char*  _outputFilename;
    FILE*  _inputFp;
    FILE*  _outputFp;
};

#endif
