//=========================================================
// Copyright  (C)1995, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.0   26 Apr 1999 15:50:46   sally
// Initial revision.
// Revision 1.2  1999/04/25 22:19:53  sally
// *** empty log message ***
//
// Revision 1.1  1999/04/25 05:07:48  sally
// Initial revision
//
// 
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

#ifndef MEMORY_FROM_FILE_H
#define MEMORY_FROM_FILE_H

static const char rcs_id_MemoryFromFile_h[] =
     "@(#) $Header$";

#include "EAList.h"
#include "CommonDefs.h"

class MemoryFromFileBlock
{
public:
    // these 3 operators are required for SortedList
    friend inline int operator==(const MemoryFromFileBlock& a,
                                 const MemoryFromFileBlock& b);
    friend inline int operator<(const MemoryFromFileBlock& a,
                                const MemoryFromFileBlock& b);
    friend inline int operator>=(const MemoryFromFileBlock& a,
                                 const MemoryFromFileBlock& b);

    enum MemoryCompareStatus
    {
        MEMORY_MATCHED = 0,       // addr is in this block and data matched
        MEMORY_NOT_MATCHED,   // addr is in this block and data not matched
        MEMORY_LARGER_THAN_END,   // addr is larger than end addr
        MEMORY_SMALLER_THAN_START // addr is small than start addr
    };

    MemoryFromFileBlock(unsigned int   startAddr,
                        char*          memData,
                        unsigned int   byteLen);

    virtual ~MemoryFromFileBlock();

    MemoryCompareStatus     Compare(unsigned int   addr,
                                    char           data,
                                    FILE*          outputFP);

    unsigned int            GetStartAddr(void) { return _startAddr; }
    unsigned int            GetByteLen(void) { return _byteLen; }
    const char*             GetData(void) { return _memData; }

protected:
    unsigned int  _startAddr;
    char*         _memData;
    unsigned int  _byteLen;
};

inline int operator==(const MemoryFromFileBlock& a,
                      const MemoryFromFileBlock& b)
{
    return(a._startAddr == b._startAddr ? 1 : 0);
}

inline int operator<(const MemoryFromFileBlock& a,
                      const MemoryFromFileBlock& b)
{
    return(a._startAddr < b._startAddr ? 1 : 0);
}

inline int operator>=(const MemoryFromFileBlock& a,
                      const MemoryFromFileBlock& b)
{
    return(a._startAddr >= b._startAddr ? 1 : 0);
}


class MemoryFromFileList : public SortedList<MemoryFromFileBlock>
{
public:
    MemoryFromFileList(const char*    memoryFilename);

    virtual ~MemoryFromFileList();

            //------------------------------------------------------
            // return 1 if matched;
            //        0 if address not in memory file;
            //       -1 if address is in memory file but not matched.
            //------------------------------------------------------
    int     Compare(unsigned int   addr,
                    char           data,
                    FILE*          outputFP);

    const char*  GetFilename(void) { return _filename; }

protected:
    char    _filename[BIG_SIZE];
};

class CDSMemoryFromFileList : public MemoryFromFileList
{
public:
    enum CDSMemoryFromFileStatusE
    {
        CDS_FILE_MEMORY_OK,
        CDS_FILE_MEMORY_OPEN_FAIL,
        CDS_FILE_MEMORY_READ_ADDR_FAIL,
        CDS_FILE_MEMORY_READ_DATA_FAIL
    };

    CDSMemoryFromFileList(const char*    cdsMemoryFilename);

    virtual ~CDSMemoryFromFileList() { };

    CDSMemoryFromFileStatusE     GetStatus(void) { return _status; }

protected:
    CDSMemoryFromFileStatusE     _status;
};

class SESMemoryFromFileList : public MemoryFromFileList
{
public:
    SESMemoryFromFileList(const char*    sesMemoryFilename);

    virtual ~SESMemoryFromFileList() { };
};

#endif
