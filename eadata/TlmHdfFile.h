//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.6   15 Mar 1999 14:19:44   sally
// add some methods for getting user's start and end indexes
// 
//    Rev 1.5   01 May 1998 14:47:56   sally
// added HK2 file
// 
//    Rev 1.4   17 Apr 1998 16:51:26   sally
// add L2A and L2B file formats
// 
//    Rev 1.3   03 Mar 1998 13:26:42   sally
// support range
// 
//    Rev 1.2   17 Feb 1998 14:46:28   sally
// add Range()
// 
//    Rev 1.1   12 Feb 1998 16:47:52   sally
// add start and end time
// Revision 1.2  1998/01/30 22:28:45  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

#ifndef TLMHDFFILE_H
#define TLMHDFFILE_H

static const char rcs_id_TlmHdfFile_h[] =
    "@(#) $Header$";

#include <string.h>

#include "CommonDefs.h"
#include "HdfFile.h"
#include "Itime.h"
#include "Parameter.h"

//-------------------------------------------------------------------
// TlmHdfFile:
//  This is an abstract class used as a generic Telemetry File.
//  The file is assumed to have same length of data in all datasets.
//-------------------------------------------------------------------

class TlmHdfFile : public HdfFile
{
public:

    friend int operator==(const TlmHdfFile&, const TlmHdfFile&);
    friend int operator>=(const TlmHdfFile&, const TlmHdfFile&);
    friend int operator< (const TlmHdfFile&, const TlmHdfFile&);

    TlmHdfFile(
           const char*           filename,                 // IN
           SourceIdE             sourceType,               // IN
           HdfFile::StatusE&     returnStatus);            // OUT


    virtual ~TlmHdfFile();

    SourceIdE           GetSource() { return _sourceType; }

    virtual Itime       GetFirstDataTime(void) { return(INVALID_TIME); }
    virtual Itime       GetLastDataTime(void) { return(INVALID_TIME); }

    int32               GetUserStartIndex(void) { return _userStartIndex; }
    int32               GetUserEndIndex(void) { return _userEndIndex; }

                        //----------------------------------------
                        // return TRUE if more data,
                        // and nextIndex contains the next index
                        //----------------------------------------
    virtual StatusE     GetNextIndex(int32& nextIndex);  // IN/OUT

    virtual StatusE     Range(FILE* ofp)= 0;

protected:

    virtual StatusE     _setFileIndices(void)=0;

    SourceIdE   _sourceType;

    int32       _userNextIndex;    // next index in the file(subclass fills in)
    int32       _userStartIndex;   // user's start index (subclass fills in)
    int32       _userEndIndex;     // user's end index (subclass fills in

};

inline int operator==(const TlmHdfFile& a, const TlmHdfFile& b)
{
    return(strcmp(a._filename, b._filename) == 0 ? 1 : 0);
}

inline int operator>=(const TlmHdfFile& a, const TlmHdfFile& b)
{
    return(strcmp(a._filename, b._filename) >= 0 ? 1 : 0);
}

inline int operator<(const TlmHdfFile& a, const TlmHdfFile& b)
{
    return(strcmp(a._filename, b._filename) < 0 ? 1 : 0);
}

#endif
