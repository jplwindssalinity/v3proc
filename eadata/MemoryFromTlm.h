//=========================================================
// Copyright  (C)1995, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.1   07 May 1999 13:11:14   sally
// add memory check for CDS and SES
// Revision 1.2  1999/04/25 05:07:20  sally
// *** empty log message ***
//
// Revision 1.1  1999/04/25 03:40:22  sally
// Initial revision
//
// 
// $Date$
// $Revision$
// $Author$
//
//=========================================================

#ifndef MEMORY_FROM_TLM_H
#define MEMORY_FROM_TLM_H

static const char rcs_id_MemoryFromTlm_h[] =
     "@(#) $Header$";

#include "EAList.h"

class TlmHdfFile;
class Parameter;

class MemoryFromTlmBlock
{
public:
    // these 3 operators are required for SortedList
    friend inline int operator==(const MemoryFromTlmBlock& a,
                                 const MemoryFromTlmBlock& b);
    friend inline int operator<(const MemoryFromTlmBlock& a,
                                 const MemoryFromTlmBlock& b);
    friend inline int operator>=(const MemoryFromTlmBlock& a,
                                 const MemoryFromTlmBlock& b);

    MemoryFromTlmBlock(unsigned int   startAddr,
                       char*          memData,
                       unsigned int   byteLen);

    virtual ~MemoryFromTlmBlock();

    unsigned int       GetStartAddr(void) { return _startAddr; }
    unsigned int       GetByteLen(void) { return _byteLen; }
    const char*        GetData(void) { return _memData; }

protected:
    unsigned int  _startAddr;
    char*         _memData;
    unsigned int  _byteLen;
};

inline int operator==(const MemoryFromTlmBlock& a,
                      const MemoryFromTlmBlock& b)
{
    return(a._startAddr == b._startAddr ? 1 : 0);
}

inline int operator<(const MemoryFromTlmBlock& a,
                      const MemoryFromTlmBlock& b)
{
    return(a._startAddr < b._startAddr ? 1 : 0);
}

inline int operator>=(const MemoryFromTlmBlock& a,
                      const MemoryFromTlmBlock& b)
{
    return(a._startAddr >= b._startAddr ? 1 : 0);
}

class MemoryFromTlmList : public SortedList<MemoryFromTlmBlock>
{
public:
    MemoryFromTlmList(TlmHdfFile*  tlmHdfFile);

    virtual ~MemoryFromTlmList() { };

protected:
    TlmHdfFile*               _tlmHdfFile;

};

class CDSMemoryFromTlmList : public MemoryFromTlmList
{
public:
    enum CDSMemoryFromTlmStatusE
    {
        CDS_TLM_MEMORY_OK,
        CDS_TLM_MEMORY_PARAM_OPEN_FAIL,
        CDS_TLM_MEMORY_PARAM_EXTRACT_FAIL
    };

    CDSMemoryFromTlmList(TlmHdfFile*  tlmHdfFile);

    virtual ~CDSMemoryFromTlmList();

    CDSMemoryFromTlmStatusE  GetStatus(void) { return _status; }

protected:
    Parameter*                _memoryAddrParamP;
    Parameter*                _memoryDataParamP;
    CDSMemoryFromTlmStatusE   _status;
};

class SESMemoryFromTlmList : public MemoryFromTlmList
{
public:
    enum SESMemoryFromTlmStatusE
    {
        SES_TLM_MEMORY_OK,
        SES_TLM_MEMORY_PARAM_OPEN_FAIL,
        SES_TLM_MEMORY_PARAM_EXTRACT_FAIL
    };

    SESMemoryFromTlmList(TlmHdfFile*  tlmHdfFile);

    virtual ~SESMemoryFromTlmList();

    SESMemoryFromTlmStatusE  GetStatus(void) { return _status; }

protected:
    Parameter*                _memoryAddrParamP;
    Parameter*                _memoryDataParamP;
    SESMemoryFromTlmStatusE   _status;
};

#endif
