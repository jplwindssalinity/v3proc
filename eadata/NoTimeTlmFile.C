//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
// CM Log
// $Log$
// 
//    Rev 1.2   01 May 1998 14:46:54   sally
// add HK2 file
// 
//    Rev 1.1   20 Apr 1998 10:22:14   sally
// change for WindSwatch
// 
// $Date$
// $Revision$
// $Author$
//
//=========================================================

static const char rcs_id[] =
    "@(#) $Header$";

#include <stdio.h>

#include "NoTimeTlmFile.h"
#include "Parameter.h"

//========//
// NoTimeTlmFile //
//========//

NoTimeTlmFile::NoTimeTlmFile(
const char*     filename,
StatusE&        returnStatus)
:   TlmHdfFile(filename, SOURCE_L2A, returnStatus)
{
    // check TlmHdfFile construction
    if (_status != HdfFile::OK)
        return;

    // set the start and end indices according to user's start and end time
    if (_setFileIndices() != OK)
    {
        returnStatus = _status;
        return;
    }

    _userNextIndex = _userStartIndex;

    returnStatus = _status;
    return;

}//NoTimeTlmFile::NoTimeTlmFile

NoTimeTlmFile::~NoTimeTlmFile()
{
    return;

} //NoTimeTlmFile::~NoTimeTlmFile

TlmHdfFile::StatusE
NoTimeTlmFile::Range(
    FILE*   ofp)
{
    fprintf(ofp, "%s : %s\n", source_id_map[_sourceType], _filename);
    fprintf(ofp, "  %ld Data Records\n", _dataLength);
    return(_status);

}//NoTimeTlmFile::Range

//-----------------//
// _setFileIndices //
//-----------------//
// set the first and last data records offsets.
// if either offset cannot be set, both offsets are set to -1.

NoTimeTlmFile::StatusE
NoTimeTlmFile::_setFileIndices(void)
{
    _status = OK;

    // find the effective start index
    _userStartIndex = 0;

    // find the effective end index
    _userEndIndex = _dataLength - 1;

    return(_status = OK);

}//NoTimeTlmFile::_setFileIndices
