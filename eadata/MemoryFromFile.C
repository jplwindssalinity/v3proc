//=========================================================
// Copyright  (C)1995, California Institute of Technology.
// U.S. Government sponsorship under
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.1   07 May 1999 13:11:02   sally
// add memory check for CDS and SES
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

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "MemoryFromFile.h"
#include "SafeString.h"

MemoryFromFileBlock::MemoryFromFileBlock(
unsigned int     startAddr,
char*            memData,
unsigned int     byteLen)
: _startAddr(startAddr), _byteLen(byteLen)
{
    assert(memData != 0);
    _memData = new char[byteLen];
    assert(_memData != 0);
    (void)memcpy(_memData, memData, byteLen);
}

MemoryFromFileBlock::~MemoryFromFileBlock()
{
    delete [] _memData;
    _memData = 0;
    _memData = 0;
}

MemoryFromFileBlock::MemoryCompareStatus
MemoryFromFileBlock::Compare(
unsigned int   addr,
char           data,
FILE*          outputFP)
{
    if (addr < _startAddr)
        return MEMORY_SMALLER_THAN_START;
    else if (addr >= (_startAddr + _byteLen))
        return MEMORY_LARGER_THAN_END;
    else if (_memData[addr - _startAddr] == data)
        return MEMORY_MATCHED;
    else
    {
        fprintf(outputFP, "Address: 0X%08X: File : 0X%02X, TLM: 0X%02X\n",
                             addr, _memData[addr - _startAddr], data);
        return MEMORY_NOT_MATCHED;
    }

}//MemoryFromFileBlock::Compare

MemoryFromFileList::MemoryFromFileList(
const char*       memoryFilename)
: SortedList<MemoryFromFileBlock>()
{
    (void)strncpy(_filename, memoryFilename, BIG_SIZE - 1);
    _filename[BIG_SIZE - 1] = '\0';
}

MemoryFromFileList::~MemoryFromFileList()
{
}

//------------------------------------------------------
// return 1 if matched;
//        0 if address not in memory file;
//       -1 if address is in memory file but not matched.
//------------------------------------------------------
int
MemoryFromFileList::Compare(
unsigned int   addr,
char           data,
FILE*          outputFP)
{
    for (MemoryFromFileBlock* memoryBlock = GetHead(); memoryBlock;
                      memoryBlock = GetNext())
    {
        MemoryFromFileBlock::MemoryCompareStatus compareStatus =
                                 memoryBlock->Compare(addr, data, outputFP);
        switch(compareStatus)
        {
            case MemoryFromFileBlock::MEMORY_MATCHED:
                return 1;
            case MemoryFromFileBlock::MEMORY_SMALLER_THAN_START:
                return 0;
            case MemoryFromFileBlock::MEMORY_NOT_MATCHED:
                return -1;
            default:   // MEMORY_LARGER_THAN_END, continue to match
                break;
        }
    }
    return 0;

} // MemoryFromFileList::Compare

CDSMemoryFromFileList::CDSMemoryFromFileList(
const char*     cdsMemoryFilename)
: MemoryFromFileList(cdsMemoryFilename),
  _status(CDS_FILE_MEMORY_OK)
{
    FILE* inputFP = fopen(cdsMemoryFilename, "r");
    if (inputFP == NULL)
    {
        _status = CDS_FILE_MEMORY_OPEN_FAIL;
        return;
    }
    char line[BIG_4K_SIZE];
    char* readPtr;
    
    while ((readPtr = fgets(line, BIG_4K_SIZE, inputFP)) != NULL)
    {
        if (readPtr != line)
        {
            if (feof(inputFP))
                return;
            else
            {
                _status = CDS_FILE_MEMORY_READ_ADDR_FAIL;
                return;
            }
       
        }
        // take out "\n"
        line[strlen(line) - 1] = '\0';
        // skip blank lines, and comment lines
        if (*line == '\0' || *line == '#' || *line == '!')
            continue;

        unsigned int startAddr=0;
        char* lasts=0;
        
        char* stringPtr = safe_strtok(line, " ", &lasts);
        if (sscanf(stringPtr, "%x", &startAddr) != 1)
        {
            _status = CDS_FILE_MEMORY_READ_ADDR_FAIL;
            return;
        }
        unsigned short twoBytes;
        unsigned char oneByte;
        unsigned int byteLen=0;
        char memData[BIG_4K_SIZE];

        char* memPtr = memData;
        while ((stringPtr = safe_strtok(0, " ", &lasts)) != 0)
        {
            if (sscanf(stringPtr, "%hX", &twoBytes) != 1)
            {
                _status = CDS_FILE_MEMORY_READ_DATA_FAIL;
                return;
            }
            oneByte = (unsigned char) twoBytes;
            memcpy(memPtr, &oneByte, 1);
            memPtr++;
            byteLen++;
        }
        MemoryFromFileBlock* newMemory =
                  new MemoryFromFileBlock(startAddr, memData, byteLen);
        SortedList<MemoryFromFileBlock>::AddSorted(newMemory);
    }

#ifdef DEBUG
for (MemoryFromFileBlock* block = GetHead(); block != 0;
                          block = GetNext())
{
    printf("Start: %08X", block->GetStartAddr());
    const char* data = block->GetData();
    for (unsigned int i = 0; i < block->GetByteLen(); i++, data++)
    {
        printf(" %02X", (unsigned short) *data);
    }
    printf("\n");
}
#endif

    fclose(inputFP);

} // CDSMemoryFromFileList::CDSMemoryFromFileList

SESMemoryFromFileList::SESMemoryFromFileList(
const char*     sesMemoryFilename)
: MemoryFromFileList(sesMemoryFilename),
  _status(SES_FILE_MEMORY_OK)
{
    FILE* inputFP = fopen(sesMemoryFilename, "r");
    if (inputFP == NULL)
    {
        _status = SES_FILE_MEMORY_OPEN_FAIL;
        return;
    }
    char line[BIG_4K_SIZE];
    char* readPtr;
    
    while ((readPtr = fgets(line, BIG_4K_SIZE, inputFP)) != NULL)
    {
        if (readPtr != line)
        {
            if (feof(inputFP))
                return;
            else
            {
                _status = SES_FILE_MEMORY_READ_ADDR_FAIL;
                return;
            }
       
        }
        // take out "\n"
        line[strlen(line) - 1] = '\0';
        // skip blank lines, and comment lines
        if (*line == '\0' || *line == '#' || *line == '!')
            continue;

        unsigned int startAddr=0;
        char* lasts=0;
        
        char* stringPtr = safe_strtok(line, " ", &lasts);
        if (sscanf(stringPtr, "%x", &startAddr) != 1)
        {
            _status = SES_FILE_MEMORY_READ_ADDR_FAIL;
            return;
        }
        unsigned short twoBytes;
        unsigned char oneByte;
        unsigned int byteLen=0;
        char memData[BIG_4K_SIZE];

        char* memPtr = memData;
        while ((stringPtr = safe_strtok(0, " ", &lasts)) != 0)
        {
            if (sscanf(stringPtr, "%hX", &twoBytes) != 1)
            {
                _status = SES_FILE_MEMORY_READ_DATA_FAIL;
                return;
            }
            oneByte = (unsigned char) twoBytes;
            memcpy(memPtr, &oneByte, 1);
            memPtr++;
            byteLen++;
        }
        MemoryFromFileBlock* newMemory =
                  new MemoryFromFileBlock(startAddr, memData, byteLen);
        SortedList<MemoryFromFileBlock>::AddSorted(newMemory);
    }

#ifdef DEBUG
for (MemoryFromFileBlock* block = GetHead(); block != 0;
                          block = GetNext())
{
    printf("Start: %08X", block->GetStartAddr());
    const char* data = block->GetData();
    for (unsigned int i = 0; i < block->GetByteLen(); i++, data++)
    {
        printf(" %02X", (unsigned short) *data);
    }
    printf("\n");
}
#endif

    fclose(inputFP);

} // SESMemoryFromFileList::SESMemoryFromFileList
