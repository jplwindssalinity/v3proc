//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
// CM Log
// $Log$
// 
//    Rev 1.3   01 Sep 1998 16:39:14   sally
// change operator== to Match in order to pass outFP
// 
//    Rev 1.2   28 Aug 1998 16:30:54   sally
// use REQA's file number to match REQQ file 
// 
//    Rev 1.1   28 Aug 1998 14:53:46   sally
// make the ReqqRecordList a Sorted List
// 
//    Rev 1.0   28 Aug 1998 11:21:22   sally
// Initial revision.
// 
// $Date$
// $Revision$
// $Author$
//
//=========================================================

#ifndef REQQ_RECORD_H
#define REQQ_RECORD_H

#include <stdio.h>

#include "EAList.h"
#include "Itime.h"
#include "Reqq.h"

static const char rcs_id_reqq_record_h[] =
    "@(#) $Header$";

#define EA_REQQ_SHORT_STRING_LEN    20

//============
// Reqq record class 
//============

class ReqqRecord
{
public:

    friend inline int operator==(const ReqqRecord&, const ReqqRecord&);
    friend inline int operator>=(const ReqqRecord&, const ReqqRecord&);
    friend inline int operator>(const ReqqRecord&, const ReqqRecord&);
    friend inline int operator<(const ReqqRecord&, const ReqqRecord&);

    ReqqRecord();   // input REQQ file pointer

    virtual ~ReqqRecord();

    int       Match(const ReqqRecord& other, FILE* outFP);

    int       _commandNumber;
    char      _sensorName[EA_REQQ_SHORT_STRING_LEN];
    char      _mnemonic[EA_REQQ_SHORT_STRING_LEN];
    char      _timeString[CODEA_TIME_LEN];
    char      _pathString[EA_REQQ_SHORT_STRING_LEN];
    char      _gammaString[EA_REQQ_SHORT_STRING_LEN];
    char      _param1String[EA_REQQ_SHORT_STRING_LEN];
    char      _param2String[EA_REQQ_SHORT_STRING_LEN];
    char      _checksumString[EA_REQQ_SHORT_STRING_LEN];

};

class ReqqRecordList : public SortedList<ReqqRecord>
{
public:
    ReqqRecordList();

    virtual ~ReqqRecordList();

    virtual ReqqStatusE    Read(const char* reqqFile,   // REQQ filename
                                FILE*       outFP);

    int                    GetFileNumber(void) const { return _fileNumber; };
    char                   GetFileVersion(void) const { return _fileVersion; };

    virtual ReqqStatusE   _ReadHeader(FILE*  fp, // read header line
                                      FILE*  outFP);

    virtual ReqqStatusE   _ReadRecord(FILE*  fp, // read & create a record
                                      FILE*  outFP);

    int           _fileNumber;
    char          _fileVersion;
};

class ReqaRecordList : public ReqqRecordList
{
public:

    ReqaRecordList();
    virtual ~ReqaRecordList();
    int                   Match(const ReqqRecordList& reqq, FILE* outFP);

protected:
    virtual ReqqStatusE   _ReadHeader(FILE*  fp, // read header line
                                      FILE*  outFP);
    virtual ReqqStatusE   _ReadRecord(FILE*  fp ,// read & create a record
                                      FILE*  outFP);
};

inline int operator==(const ReqqRecord& a, const ReqqRecord& b)
    { return(a._commandNumber == b._commandNumber); }
inline int operator>=(const ReqqRecord& a, const ReqqRecord& b)
    { return(a._commandNumber >= b._commandNumber); }
inline int operator>(const ReqqRecord& a, const ReqqRecord& b)
    { return(a._commandNumber > b._commandNumber); }
inline int operator<(const ReqqRecord& a, const ReqqRecord& b)
    { return(a._commandNumber < b._commandNumber); }

#endif // REQQ_RECORD_H
