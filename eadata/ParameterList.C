//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.13   09 Sep 1998 15:06:52   sally
// take care of return value of -1 from extractFunc()
// 
//    Rev 1.12   04 Aug 1998 16:29:40   deliver
// pass polynomial table to ParameterList's HoldExtract
// 
//    Rev 1.11   27 Jul 1998 14:00:44   sally
// passing polynomial table to extraction function
// 
//    Rev 1.10   23 Jul 1998 16:14:52   sally
// pass polynomial table to extractFunc()
// 
//    Rev 1.9   08 Jun 1998 13:37:12   sally
// 
//    Rev 1.8   03 Jun 1998 10:10:34   sally
// change parameter names and types due to LP's changes
// 
//    Rev 1.7   01 May 1998 14:47:50   sally
// added HK2 file
// 
//    Rev 1.6   14 Apr 1998 16:43:54   sally
// move back to EA's old list
// 
//    Rev 1.5   06 Apr 1998 16:29:02   sally
// 
//    Rev 1.4   01 Apr 1998 13:36:14   sally
// for L1A Derived table
// 
//    Rev 1.3   24 Mar 1998 15:57:18   sally
// de-warn for GNU
// 
//    Rev 1.2   23 Mar 1998 15:39:54   sally
// adapt to derived science data
// 
//    Rev 1.1   19 Mar 1998 13:37:22   sally
//  added "days", "hours", "minutes" and "seconds" units
// 
//    Rev 1.0   04 Feb 1998 14:16:42   daffer
// Initial checking
// Revision 1.2  1998/01/30 22:29:20  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

#include <assert.h>
#include <malloc.h>
#include <string.h>

#include "ParameterList.h"
#include "Itime.h"
#include "ParTab.h"
#include "PolyTable.h"
#include "TlmHdfFile.h"


static const char rcs_parameter_list_id[] =
    "@(#) $Header$";

#define PARAMETERLIST_BLOCK 8192

//===============
// ParameterList 
//===============

ParameterList::ParameterList()
:   _numPairs(0), _pairCapacity(0), _status(OK)
{
    return;
}

ParameterList::~ParameterList()
{
    return;
}

ParameterList::StatusE
ParameterList::OpenParamDataSets(
TlmHdfFile*    tlmFile)
{
    for (Parameter* param_ptr = GetHead(); param_ptr; param_ptr = GetNext())
    {
        int32 dataType, dataStartIndex, dataLength, numDimensions;
        char tempString[BIG_SIZE];
        (void)strncpy(tempString, param_ptr->sdsNames, BIG_SIZE);
        char* oneSdsName=0;
        int i=0;
        char* lasts = 0;
        for (oneSdsName = (char*)safe_strtok(tempString, ",", &lasts);
                       oneSdsName;
                       oneSdsName = (char*)safe_strtok(0, ",", &lasts), i++)
        {
            param_ptr->sdsIDs[i] = HDF_FAIL;
            param_ptr->sdsIDs[i] = tlmFile->SelectDataset(
                                         oneSdsName, dataType, dataStartIndex,
                                         dataLength, numDimensions);
            if (param_ptr->sdsIDs[i] == HDF_FAIL)
                return(_status = ERROR_SELECTING_PARAMETER);
        }
    }
    return(_status = OK);

}//ParameterList::OpenParamDataSets

ParameterList::StatusE
ParameterList::CloseParamDataSets(
TlmHdfFile*    tlmFile)
{
    for (Parameter* param_ptr = GetHead(); param_ptr; param_ptr = GetNext())
    {
        for (int i=0; i < param_ptr->numSDSs; i++)
        {
            if (tlmFile->CloseDataset(param_ptr->sdsIDs[i]) == HDF_FAIL)
                return(_status = ERROR_DESELECTING_PARAMETER);
            param_ptr->sdsIDs[i] = HDF_FAIL;
        }
    }
    return(_status = OK);

}//ParameterList::CloseParamDataSets

//---------
// Extract 
//---------

ParameterList::StatusE
ParameterList::Extract(
TlmHdfFile*    tlmFile,
int32          startIndex)
{
    if (_numPairs >= _pairCapacity)
    {
        // more memory is needed
        if ( ! _ReallocData())
        {
            _status = ParameterList::ERROR_ALLOCATING_MEMORY;
            return (_status);
        }
    }
    for (Parameter* param_ptr = GetHead(); param_ptr; param_ptr = GetNext())
    {
        if ( ! param_ptr->extractFunc(tlmFile, param_ptr->sdsIDs,
                 startIndex, 1, 1,
                 param_ptr->data + _numPairs * param_ptr->byteSize, 0))
        {
            return (_status=ERROR_EXTRACTING_PARAMETER);
        }
    }

    // all parameters were extracted, so add to the number of pairs
    _numPairs++;

    return(_status);

} // ParameterList::Extract

//-------------
// HoldExtract 
//-------------
ParameterList::StatusE
ParameterList::HoldExtract(
TlmHdfFile*       tlmFile,
int32             startIndex,
PolynomialTable*  polyTable)           // not used
{
    assert (tlmFile != 0);
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

    //----------------------------------------
    // extract as many parameters as possible 
    //----------------------------------------

    int new_param_count = 0;
    Parameter* param_ptr=0;
    for (param_ptr = GetHead(); param_ptr; param_ptr = GetNext())
    {
        int rc = param_ptr->extractFunc(tlmFile,
                 param_ptr->sdsIDs, startIndex, 1, 1,
                 param_ptr->data + _numPairs * param_ptr->byteSize, polyTable);
        switch(rc)
        {
            case -1:
                return(_status = ERROR_EXTRACTING_PARAMETER);
            case 0:
                break;
            default:
                param_ptr->held = 1;
                if (param_ptr->paramId != UTC_TIME)
                    new_param_count++;  // only count non-time parameters
                break;
        }
    }

    if (! new_param_count)
        return(_status);    // no newly extracted parameters (except time?)

    for (param_ptr = GetHead(); param_ptr; param_ptr = GetNext())
    {
        if (! param_ptr->held)
            return(_status);    // there is at least one missing parameter
    }

    // all parameters are held, at least one newly extracted parameter

    //------------------------------------
    // copy current data as new held data 
    //------------------------------------

    for (param_ptr = GetHead(); param_ptr; param_ptr = GetNext())
    {
        char* start = param_ptr->data + _numPairs * param_ptr->byteSize;
        memcpy(start + param_ptr->byteSize, start, param_ptr->byteSize);
    }
    _numPairs++;

    return(_status);

}//ParameterList::HoldExtract

//----------
// PrePrint 
//----------

ParameterList::StatusE
ParameterList::PrePrint()
{
    for (Parameter* param_ptr = GetHead(); param_ptr; param_ptr = GetNext())
    {
        //-----------------------------------
        // set units for AUTOTIME parameters 
        //-----------------------------------

        if (param_ptr->unitId == UNIT_AUTOTIME)
            _SetTimeUnits(param_ptr);

        //-----------------
        // adjust the time 
        //-----------------

        if (param_ptr->paramId == UTC_TIME)
            _status = _AdjustTime(param_ptr);
    }
    return (_status);
}

//-------
// Print 
//-------

ParameterList::StatusE
ParameterList::Print(
    FILE*   fp)
{
    for (unsigned long i = 0; i < _numPairs; i++)
    {
        Parameter* first_param = GetHead();
        first_param->printFunc(fp,
            first_param->data + i * first_param->byteSize);
        for (Parameter* param_ptr = GetNext(); param_ptr;
            param_ptr = GetNext())
        {
            fprintf(fp, "   ");
            param_ptr->printFunc(fp,
                param_ptr->data + i * param_ptr->byteSize);
        }
        fprintf(fp, "\n");
    }
    return(_status);
}

//------------
// PrintACEgr 
//------------

ParameterList::StatusE
ParameterList::PrintACEgr(
    FILE*   fp,
    Itime   start,
    Itime   end,
    char*   comments)
{
    // --- title ---
    Parameter* xaxis_param = GetHead();
    Parameter* y1_param = GetNext();
    fprintf(fp, "@ title \"%s", y1_param->paramName);
    Parameter* param_ptr=0;
    for (param_ptr = GetNext(); param_ptr; param_ptr = GetNext())
        fprintf(fp, ", %s", param_ptr->paramName);
    fprintf(fp, " vs. %s from %s", xaxis_param->paramName,
        source_id_map[xaxis_param->sourceId]);
    if (comments)
        fprintf(fp, "  {%s}", comments);
    fprintf(fp, "\"\n");
 
    // --- subtitle ---
    char codea_1[CODEA_TIME_LEN], codea_2[CODEA_TIME_LEN];
    start.ItimeToCodeA(codea_1);
    end.ItimeToCodeA(codea_2);
    fprintf(fp, "@ subtitle \"From %s to %s\"\n", codea_1, codea_2);
 
    // --- x axis label ---
    fprintf(fp, "@ xaxis label \"%s (%s)\"\n", xaxis_param->measurable,
        xaxis_param->unitName);
 
    // --- y axis label ---
    fprintf(fp, "@ yaxis label \"%s (%s)", y1_param->measurable,
        y1_param->unitName);
    for (param_ptr = GetIndex(2); param_ptr; param_ptr = GetNext())
        fprintf(fp, ", %s (%s)", param_ptr->measurable, param_ptr->unitName);
    fprintf(fp, "\"\n");
 
    // --- delimeter line ---
    fprintf(fp, "#--------------------------------------------------\n");

    // --- print the data ---
    Print(fp);

    return (_status);
}

//--------------
// _ReallocData 
//--------------

char
ParameterList::_ReallocData(void)
{
    long newCapacity = _pairCapacity + PARAMETERLIST_BLOCK;
    int byteCount;
    for (Parameter* param_ptr = GetHead(); param_ptr; param_ptr = GetNext())
    {
        byteCount = newCapacity * param_ptr->byteSize;

        if (param_ptr->data == NULL)
            param_ptr->data = (char *)malloc(byteCount);
        else
            param_ptr->data = (char *)realloc(param_ptr->data, byteCount);

        if (param_ptr->data == NULL)
            return (0);
    }
    _pairCapacity = newCapacity;
    return(1);
}

//---------------
// _GetFirstTime 
//---------------

Itime
ParameterList::_GetFirstTime(
    Parameter*  target_param)
{
    if (_numPairs < 1)
        return (INVALID_TIME);

    BYTE6 time_buffer;
    memcpy(time_buffer, target_param->data, 6);
    Itime itime;
    itime.Char6ToItime((const char*)time_buffer);
    return (itime);
}

//--------------
// _GetLastTime 
//--------------

Itime
ParameterList::_GetLastTime(
    Parameter*  target_param)
{
    if (_numPairs < 1)
        return (INVALID_TIME);

    BYTE6 time_buffer;
    memcpy(time_buffer, target_param->data +
        (_numPairs - 1) * target_param->byteSize, 6);
    Itime itime;
    itime.Char6ToItime((const char*)time_buffer);
    return (itime);
}

//--------------
// _GetDuration 
//--------------

int
ParameterList::_GetDuration(
    Parameter*  target_param,
    Itime*      duration)
{
    // determine the duration
    Itime first_time = _GetFirstTime(target_param);
    if (first_time == INVALID_TIME)
        return (0);
    Itime last_time = _GetLastTime(target_param);
    if (last_time == INVALID_TIME)
        return (0);
    *duration = last_time - first_time;
    return (1);
}

//---------------
// _SetTimeUnits 
//---------------

ParameterList::StatusE
ParameterList::_SetTimeUnits(
    Parameter*  target_param)
{
    Itime duration;
    if (! _GetDuration(target_param, &duration))
        return (ParameterList::ERROR_CONVERTING_TIME);

    // select the proper units
    UnitIdE unitId;
    if (duration.sec < USE_SECONDS_DURATION)
        unitId = UNIT_SECONDS;
    else if (duration.sec < USE_MINUTES_DURATION)
        unitId = UNIT_MINUTES;
    else if (duration.sec < USE_HOURS_DURATION)
        unitId = UNIT_HOURS;
    else
        unitId = UNIT_DAYS;

    // look up the time parameter
    SourceIdE sourceId = target_param->sourceId;
    Parameter *time_param = ParTabAccess::GetParameter(sourceId,
        UTC_TIME, unitId);
    assert(time_param != 0);
 
    // copy the print function, unit name, and unit id
    target_param->printFunc = time_param->printFunc;
    strcpy(target_param->unitName, time_param->unitName);
    target_param->unitId = time_param->unitId;
 
    // delete the time parameter
    delete time_param;

    return (ParameterList::OK);
}

//-------------
// _AdjustTime 
//-------------

ParameterList::StatusE
ParameterList::_AdjustTime(
    Parameter*  target_param)
{
    Itime duration;
    if (! _GetDuration(target_param, &duration))
        return (ParameterList::ERROR_CONVERTING_TIME);

    Itime first_time = _GetFirstTime(target_param);
    if (first_time == INVALID_TIME)
        return (ParameterList::ERROR_CONVERTING_TIME);

    // select the since time
    Itime sinceTime;
    switch(target_param->unitId)
    {
        case UNIT_DAYS:
            if (duration.sec / SEC_PER_28_DAYS > 1) // more than 28 days
                sinceTime = first_time.StartOfYear();
            else
                sinceTime = first_time.StartOfMonth();
            break;
        case UNIT_HOURS:
            sinceTime = first_time.StartOfDay();
            break;
        case UNIT_MINUTES:
            sinceTime = first_time.StartOfHour();
            break;
        case UNIT_SECONDS:
            sinceTime = first_time.StartOfMinute();
            break;
        default:
            sinceTime = INVALID_TIME;
    }

    // adjust by the since time
    Itime itime;
    char *address;
    for (unsigned long i = 0; i < _numPairs; i++)
    {
        address = target_param->data + i * target_param->byteSize;
        itime.Char6ToItime((const char*)address);
        itime -= sinceTime;
        itime.ItimeToChar6(address);
    }

    // correct the units string
    char codea[CODEA_TIME_LEN];
    sinceTime.ItimeToCodeA(codea);
    strcat(target_param->unitName, " since ");
    strcat(target_param->unitName, codea);

    return (ParameterList::OK);
}

ParameterList::StatusE
ParameterList::ApplyPolynomialTable(
PolynomialTable*    polyTable)
{
    assert(polyTable != 0);

    for (Parameter* param_ptr = GetHead(); param_ptr; param_ptr = GetNext())
    {
        if ( ! param_ptr->needPolynomial)
            continue;

        char tempString[BIG_SIZE];
        (void)strncpy(tempString, param_ptr->sdsNames, BIG_SIZE);
        char *oneSdsName=0, *lasts = 0;
        oneSdsName = (char*)safe_strtok(tempString, ",", &lasts);
        if (oneSdsName == 0)
        {
            fprintf(stderr, "Missing SDS name\n");
            return(ERROR_MISSING_SDS_NAME);
        }

        // is this parameter in the polynomial table?
        const Polynomial* polynomial = polyTable->SelectPolynomial(
                                 oneSdsName, param_ptr->unitName);

        // it is in the polynomial table. need to apply to the data
        if (polynomial)
        {
            switch(param_ptr->dataType)
            {
                case DATA_FLOAT4:
                    polynomial->ApplyReplaceArray((float*)param_ptr->data,
                                                   (int)_numPairs);
                    break;
                case DATA_FLOAT8:
                    polynomial->ApplyReplaceArray((double*)param_ptr->data,
                                                   (int)_numPairs);
                    break;
                default:
                    return(_status = ERROR_APPLY_POLYNOMIAL_TO_NON_FLOAT);
            }
        }
        else
        {
            fprintf(stderr, "%s[%s] needs polynomial\n",
                                param_ptr->paramName, param_ptr->unitName);
            return(_status = ERROR_PARAMETER_NEED_POLYNOMIAL);
        }
    }
    return(_status = OK);

}//ParameterList::ApplyPolynomialTable

IotBoolean
ParameterList::NeedPolynomial(void)
{
    IotBoolean rc = 0;
    for (Parameter* paramP = GetHead(); paramP; paramP = GetNext())
    {
        if (paramP->needPolynomial)
        {
            fprintf(stderr, "%s: needs polynomial\n", paramP->paramName);
            rc = 1;
        }
    }
    return(rc);

}//ParameterList::NeedPolynomial
