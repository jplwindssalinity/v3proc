//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
// CM Log
// $Log$
// 
//    Rev 1.7   05 Apr 1999 13:46:10   sally
// fix for receiver gain
// 
//    Rev 1.6   18 Aug 1998 10:57:10   sally
// make L1ADrvExtract return any number of values
// 
//    Rev 1.5   28 Jul 1998 10:47:54   sally
// added pulse pattern for CBM
// .
// 
//    Rev 1.4   27 Jul 1998 13:59:08   sally
// passing polynomial table to extraction function
// 
//    Rev 1.3   23 Jul 1998 16:12:40   sally
// pass polynomial table to extractFunc()
// 
//    Rev 1.2   08 Jun 1998 13:36:04   sally
// fix for processing REQI
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
#include <stdlib.h>
#include <string.h>

#include "DrvParamList.h"
#include "Itime.h"
#include "ParTab.h"
#include "PolyTable.h"
#include "TlmHdfFile.h"


static const char rcsid_DrvParamList_C[] =
    "@(#) $Header$";

#define PARAMETERLIST_BLOCK        8192
#define MAX_NUM_DERIVED_VALUES     100

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

void
DerivedParamList:: _getThisPlusTime(
TlmHdfFile*    tlmFile,        // current TLM file
Parameter*     param_ptr,
char*          newData,
char*          thisBuf,
int            paramIndex,
int            k)
{
    Itime *thisTime = (Itime*) thisBuf;
    Itime nextTime;
    // extract the next frame's time
    // if this is the last frame, the previous
    // eachPulsesTime will be used
    static Itime eachPulseTime;
    if (param_ptr->extractFunc(tlmFile, param_ptr->sdsIDs,
                                   paramIndex+1, 1, 1, &nextTime,
                                   0) == 1)
        eachPulseTime = (nextTime - *thisTime) / MAX_NUM_DERIVED_VALUES;

    Itime newTime = *thisTime + eachPulseTime * (long)k;
    memcpy(newData, &newTime, param_ptr->byteSize);

} // DerivedParamList:: _getThisPlusTime

ParameterList::StatusE
DerivedParamList::_copyAllValues(
const DerivedExtractResult*    extractResults, // extract result array
int                            numParams,      // number in the array
TlmHdfFile*                    tlmFile,        // current TLM file
int32                          thisIndex)      // current index in HDF
{
    //--------------------------------------
    // make sure enough memory is allocated
    //--------------------------------------
    if (_numPairs + 1 >= _pairCapacity)     // +1 is for held area
    {
        // more memory is needed
        if ( ! _ReallocData())
        {
            _status = ParameterList::ERROR_ALLOCATING_MEMORY;
            return (_status);
        }
    }

    int index=0;
    Parameter* param_ptr=0;

    int haveValidData = 0;

    // go through all 100 pulses one by one
    for (int k=0; k < MAX_NUM_DERIVED_VALUES; k++)
    {
        haveValidData = 0;
        for (param_ptr = GetHead(), index = 0; param_ptr;
                                  param_ptr = GetNext(), index++)
        {
            // numParams should match number in ParameterList
            if (index >= numParams)
                return(_status = ERROR_TOO_FEW_PARAMETERS);

            // check if this pulse contains valid data
            if (param_ptr->paramId != UTC_TIME &&
                   extractResults[index].validDataMap[k])
            {
                haveValidData = 1;
                break;
            }
        }
        // if no parameter contains valid data, skip to next pulse
        if ( ! haveValidData) continue;

        //--------------------------------------------------------
        // OK, at least one parameter contains valid data
        // the other parameters have to:
        //   (a) duplicate the previous one if there is old data
        //   (b) write 0 if there isn't
        //--------------------------------------------------------
        _numPairs++;
        for (param_ptr = GetHead(), index = 0; param_ptr;
                                 param_ptr = GetNext(), index++)
        {
            char* newData = param_ptr->data +
                             (_numPairs - 1) * param_ptr->byteSize;
            if (param_ptr->paramId == UTC_TIME)
            {
                _getThisPlusTime(tlmFile, param_ptr,
                       newData, extractResults[index].dataBuf, thisIndex, k);
                continue;
            }

            char* thisBuf = extractResults[index].dataBuf +
                                  k * param_ptr->byteSize;
            if (extractResults[index].validDataMap[k])
            {
                (void)memcpy(newData, thisBuf, param_ptr->byteSize);
            }
            // if there was no data yet, set it to 0
            else if (newData == param_ptr->data)
            {
                (void)memset(newData, 0, param_ptr->byteSize);
            }
            // set it to previous value
            else
            {
                (void)memcpy(newData, newData - param_ptr->byteSize,
                                        param_ptr->byteSize);
            }
        }

    }

    return(_status = OK);

} // DerivedParamList::_copyAllValues

ParameterList::StatusE
DerivedParamList::HoldExtract(
TlmHdfFile*       tlmFile,
int32             startIndex,
PolynomialTable*  polyTable)
{
    int numParams=0;
    Parameter* param_ptr=0;
    for (param_ptr = GetHead(); param_ptr; param_ptr = GetNext())
        numParams++;

    DerivedExtractResult* extractResults =
                    new DerivedExtractResult [numParams];
    int index=0;
    for (index = 0; index < numParams; index++)
        extractResults[index].dataBuf = 0;

    int maxNumExtracted = 0;
    index = 0;
    for (param_ptr = GetHead(); param_ptr;
                        param_ptr = GetNext(), index++)
    {
        extractResults[index].dataBuf =
               new char [MAX_NUM_DERIVED_VALUES * (param_ptr->byteSize)];
        if (param_ptr->paramId == UTC_TIME)
            extractResults[index].numExtracted =
                        param_ptr->extractFunc(tlmFile,
                                       param_ptr->sdsIDs,
                                       startIndex, 1, 1,
                                       (void*)extractResults[index].dataBuf,
                                       polyTable);
        else
            extractResults[index].numExtracted =
                        param_ptr->extractFunc(tlmFile,
                                       param_ptr->sdsIDs,
                                       startIndex, 1, 1,
                                       (void*)&(extractResults[index]),
                                       polyTable);
int k = extractResults[index].numExtracted;
        // if nothing is extracted with any parameter, stop
        if (extractResults[index].numExtracted < 0)
        {
                fprintf(stderr, "Extracting Parameter %s[%s] failed\n",
                             param_ptr->paramName, param_ptr->unitName);
        }

        if (param_ptr->paramId != UTC_TIME)
            maxNumExtracted = MAX_OF_TWO(maxNumExtracted,
                               extractResults[index].numExtracted);
    }
    if (maxNumExtracted > 0)
    {
        // status is set inside _copyAllValues
        (void)_copyAllValues(extractResults, numParams, tlmFile, startIndex);
    }
    else if (maxNumExtracted < 0)
    {
        _status = ERROR_EXTRACTING_PARAMETER;
    }
    else if (maxNumExtracted == 0)
    {
        _status = ERROR_EXTRACTING_NOTHING;
    }

    for (index = 0; index < numParams; index++)
    {
        if (extractResults[index].dataBuf)
            delete [] extractResults[index].dataBuf;
    }
    delete [] extractResults;

    return(_status);

}//DerivedParamList::HoldExtract
