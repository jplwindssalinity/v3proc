//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
// CM Log
// $Log$
// 
//    Rev 1.1   06 Apr 1998 16:27:08   sally
// merged with SVT
// 
//    Rev 1.0   01 Apr 1998 13:36:50   sally
// Initial revision.
// 
// $Date$
// $Revision$
// $Author$
//
//=========================================================

#include <assert.h>
#include <malloc.h>
#include <string.h>

#include "DrvParamList.h"
#include "Itime.h"
#include "ParTab.h"
#include "PolyTable.h"
#include "TlmHdfFile.h"


static const char rcsid_DrvParamList_C[] =
    "@(#) $Header$";

#define PARAMETERLIST_BLOCK 8192

//===============
// DerivedParamList 
//===============

DerivedParamList::DerivedParamList()
:   ParameterList()
{
    return;

}//DerivedParamList::DerivedParamList

DerivedParamList::~DerivedParamList()
{
    return;

}//DerivedParamList::~DerivedParamList

ParameterList::StatusE
DerivedParamList::_copyAllValues(
unsigned int            maxNumExtracted,  // max number of extracted values
const ExtractResult*    extractResults,   // extract result array
int                     numParams,        // number in the array
TlmHdfFile*             tlmFile,          // current TLM file
int32                   thisIndex)        // current index in HDF
{
    long newCapacity = _pairCapacity + maxNumExtracted;
    int byteCount;
    int index=0;
    for (Parameter* param_ptr = GetHead();
                               param_ptr; param_ptr = GetNext(), index++)
    {
        // numParams should match number in ParameterList
        if (index >= numParams)
            return(_status = ERROR_TOO_FEW_PARAMETERS);

        // realloc memory to cover maximum extracted values
        byteCount = newCapacity * param_ptr->byteSize;
        param_ptr->data = (char *)realloc(param_ptr->data, byteCount);
        if (param_ptr->data == NULL)
            return(_status = ERROR_ALLOCATING_MEMORY);

        char* newData = param_ptr->data + _numPairs * param_ptr->byteSize;

        // simplest case: extract one value only
        if (maxNumExtracted == 1)
        {
            memcpy(newData, extractResults[index].dataBuf,
                                param_ptr->byteSize);
        }
        else
        {
            // this is the max number of values, copy the whole buffer
            if (maxNumExtracted ==
                    (unsigned int)extractResults[index].numExtracted)
            {
                memcpy(newData, extractResults[index].dataBuf,
                                maxNumExtracted * param_ptr->byteSize);
            }
            else if (maxNumExtracted >
                    (unsigned int)extractResults[index].numExtracted)
            {
                // space the time parameter evenly
                if (param_ptr->paramId == UTC_TIME)
                {
                    Itime *thisTime = (Itime*)(extractResults[index].dataBuf);
                    Itime nextTime;
                    static Itime each4PulsesTime;
                    // if this is the last frame, the previous
                    // each4PulsesTime will be used
                    if (param_ptr->extractFunc(tlmFile, param_ptr->sdsIDs,
                                           thisIndex+1, 1, 1, &nextTime) == 1)
                        each4PulsesTime = (nextTime - *thisTime) / 25;

                    Itime newTime = *thisTime;

                    for (unsigned int i = 0; i < maxNumExtracted; i++)
                    {
                        memcpy(newData, &newTime, param_ptr->byteSize);
                        newData += param_ptr->byteSize;
                        newTime += each4PulsesTime;
                    }
                }
                else
                {
                    int last = extractResults[index].numExtracted - 1;
                    if (last < 0)
                        return(_status = ERROR_TOO_FEW_VALUES);
                    memcpy(newData, extractResults[index].dataBuf,
                                       extractResults[index].numExtracted *
                                       param_ptr->byteSize);
                    newData += extractResults[index].numExtracted *
                                       param_ptr->byteSize;
                    char* lastData = extractResults[index].dataBuf +
                                       last * param_ptr->byteSize;
                    for (unsigned int i = extractResults[index].numExtracted;
                               i < maxNumExtracted; i++)
                    {
                        memcpy(newData, lastData, param_ptr->byteSize);
                        newData += param_ptr->byteSize;
                    }
                }
            }
            else
                return(_status = ERROR_TOO_MANY_VALUES);
        }
    }
    _pairCapacity = newCapacity;
    _numPairs += maxNumExtracted;
    return(_status = OK);

} // DerivedParamList::_copyAllValues

ParameterList::StatusE
DerivedParamList::HoldExtract(
TlmHdfFile*    tlmFile,
int32          startIndex)
{
    int numParams=0;
    Parameter* param_ptr=0;
    for (param_ptr = GetHead(); param_ptr; param_ptr = GetNext())
        numParams++;

    ExtractResult* extractResults = new ExtractResult [numParams];

    int index=0;
    int maxNumExtracted=0;
    for (param_ptr = GetHead(), index = 0; param_ptr;
                        param_ptr = GetNext(), index++)
    {
        extractResults[index].dataBuf = new char [25 * param_ptr->byteSize];
        extractResults[index].numExtracted =
                        param_ptr->extractFunc(tlmFile, param_ptr->sdsIDs,
                                           startIndex, 1, 1,
                                           extractResults[index].dataBuf);
        // if nothing is extracted with any parameter, stop
        if (extractResults[index].numExtracted <= 0)
        {
            maxNumExtracted = extractResults[index].numExtracted;
            break;
        }

        maxNumExtracted = MAX_OF_TWO(maxNumExtracted,
                               extractResults[index].numExtracted);
    }
    if (maxNumExtracted > 0)
    {
        // status is set inside _copyAllValues
        (void)_copyAllValues(maxNumExtracted, extractResults,
                              numParams, tlmFile, startIndex);
    }
    else if (maxNumExtracted < 0)
    {
        _status = ERROR_EXTRACTING_PARAMETER;
    }
    else if (maxNumExtracted == 0)
    {
        _status = ERROR_EXTRACTING_NOTHING;
    }

    for (param_ptr = GetHead(), index = 0; param_ptr;
                        param_ptr = GetNext(), index++)
    {
        delete [] extractResults[index].dataBuf;
    }
    delete [] extractResults;

    return(_status);

}//DerivedParamList::Extract
