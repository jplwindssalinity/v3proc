//=========================================================
// Copyright  (C)1995, California Institute of Technology.
// U.S. Government sponsorship under
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.1   07 May 1999 13:11:10   sally
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
//
//=========================================================

#include <assert.h>

#include "MemoryFromTlm.h"
#include "ParTab.h"
#include "Parameter.h"
#include "TlmHdfFile.h"

MemoryFromTlmBlock::MemoryFromTlmBlock(
unsigned int   startAddr,
char*          memData,
unsigned int   byteLen)
: _startAddr(startAddr), _memData(0), _byteLen(byteLen)
{
    assert(memData != 0);
    _memData = new char[byteLen];
    assert(_memData != 0);
    (void)memcpy(_memData, memData, byteLen);

} // MemoryFromTlmBlock::MemoryFromTlmBlock

MemoryFromTlmBlock::~MemoryFromTlmBlock()
{
    delete [] _memData;
}

MemoryFromTlmList::MemoryFromTlmList(
TlmHdfFile*  tlmHdfFile)
: SortedList<MemoryFromTlmBlock>(),
  _tlmHdfFile(tlmHdfFile)
{
} // MemoryFromTlmList::MemoryFromTlmList


CDSMemoryFromTlmList::CDSMemoryFromTlmList(
TlmHdfFile*  tlmHdfFile)
: MemoryFromTlmList(tlmHdfFile),
  _memoryAddrParamP(0), _memoryDataParamP(0),
  _status(CDS_TLM_MEMORY_OK)
{
    _memoryAddrParamP = ParTabAccess::GetParameter(
                             SOURCE_L1A, CDS_MEMORY_DUMP_ADDR, UNIT_DEC_ADDR);
    if (tlmHdfFile->OpenParamDatasets(_memoryAddrParamP) != HdfFile::OK)
    {
        _status = CDS_TLM_MEMORY_PARAM_OPEN_FAIL;
        return;
    }

    _memoryDataParamP = ParTabAccess::GetParameter(
                             SOURCE_L1A, CDS_MEMORY_DUMP_DATA, UNIT_DN);
    if (tlmHdfFile->OpenParamDatasets(_memoryDataParamP) != HdfFile::OK)
    {
        _status = CDS_TLM_MEMORY_PARAM_OPEN_FAIL;
        return;
    }

    for (int index=0; index < tlmHdfFile->GetDataLength(); index++)
    {
        unsigned int startAddr=0;
        if ( ! _memoryAddrParamP->extractFunc(tlmHdfFile,
                                          _memoryAddrParamP->sdsIDs,
                                          index, 1, 1, &startAddr, 0))
        {
            fprintf(stderr,
                      "Extracting Memory Address (index %d) from %s failed\n",
                      index, tlmHdfFile->GetFileName());
            _status = CDS_TLM_MEMORY_PARAM_EXTRACT_FAIL;
            return;
        }
        char memData[16];
        if ( ! _memoryDataParamP->extractFunc(tlmHdfFile,
                                          _memoryDataParamP->sdsIDs,
                                          index, 1, 1, memData, 0))
        {
            fprintf(stderr,
                      "Extracting Memory Data (index %d) from %s failed\n",
                      index, tlmHdfFile->GetFileName());
            _status = CDS_TLM_MEMORY_PARAM_EXTRACT_FAIL;
            return;
        }
        MemoryFromTlmBlock* newMemory =
                new MemoryFromTlmBlock(startAddr, memData, 16);
        SortedList<MemoryFromTlmBlock>::AddSorted(newMemory);
    }

}//CDSMemoryFromTlmList::CDSMemoryFromTlmList

CDSMemoryFromTlmList::~CDSMemoryFromTlmList()
{
    if (_memoryAddrParamP)
    {
        (void)_tlmHdfFile->CloseParamDatasets(_memoryAddrParamP);
        _memoryAddrParamP = 0;
    }

    if (_memoryDataParamP)
    {
        (void)_tlmHdfFile->CloseParamDatasets(_memoryDataParamP);
        _memoryDataParamP = 0;
    }

} // CDSMemoryFromTlmList::~CDSMemoryFromTlmList

SESMemoryFromTlmList::SESMemoryFromTlmList(
TlmHdfFile*  tlmHdfFile)
: MemoryFromTlmList(tlmHdfFile),
  _memoryAddrParamP(0), _memoryDataParamP(0),
  _status(SES_TLM_MEMORY_OK)
{
    _memoryAddrParamP = ParTabAccess::GetParameter(
                             SOURCE_L1A, SES_MEMORY_DUMP_ADDR, UNIT_DEC_ADDR);
    if (tlmHdfFile->OpenParamDatasets(_memoryAddrParamP) != HdfFile::OK)
    {
        _status = SES_TLM_MEMORY_PARAM_OPEN_FAIL;
        return;
    }

    _memoryDataParamP = ParTabAccess::GetParameter(
                             SOURCE_L1A, SES_MEMORY_DUMP_DATA, UNIT_DN);
    if (tlmHdfFile->OpenParamDatasets(_memoryDataParamP) != HdfFile::OK)
    {
        _status = SES_TLM_MEMORY_PARAM_OPEN_FAIL;
        return;
    }

    for (int index=0; index < tlmHdfFile->GetDataLength(); index++)
    {
        unsigned short startAddr=0;
        if ( ! _memoryAddrParamP->extractFunc(tlmHdfFile,
                                          _memoryAddrParamP->sdsIDs,
                                          index, 1, 1, &startAddr, 0))
        {
            fprintf(stderr,
                      "Extracting Memory Address (index %d) from %s failed\n",
                      index, tlmHdfFile->GetFileName());
            _status = SES_TLM_MEMORY_PARAM_EXTRACT_FAIL;
            return;
        }
        char memData[4];
        if ( ! _memoryDataParamP->extractFunc(tlmHdfFile,
                                          _memoryDataParamP->sdsIDs,
                                          index, 1, 1, &memData, 0))
        {
            fprintf(stderr,
                      "Extracting Memory Data (index %d) from %s failed\n",
                      index, tlmHdfFile->GetFileName());
            _status = SES_TLM_MEMORY_PARAM_EXTRACT_FAIL;
            return;
        }
        MemoryFromTlmBlock* newMemory =
                new MemoryFromTlmBlock((unsigned int)startAddr, memData, 4);
        SortedList<MemoryFromTlmBlock>::AddSorted(newMemory);
    }

}//SESMemoryFromTlmList::SESMemoryFromTlmList

SESMemoryFromTlmList::~SESMemoryFromTlmList()
{
    if (_memoryAddrParamP)
    {
        (void)_tlmHdfFile->CloseParamDatasets(_memoryAddrParamP);
        _memoryAddrParamP = 0;
    }

    if (_memoryDataParamP)
    {
        (void)_tlmHdfFile->CloseParamDatasets(_memoryDataParamP);
        _memoryDataParamP = 0;
    }

} // SESMemoryFromTlmList::~SESMemoryFromTlmList
