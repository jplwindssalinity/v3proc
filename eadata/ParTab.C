//=========================================================
// Copyright  (C)1995, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.11   13 Oct 1998 15:34:20   sally
// added L1B file
// 
//    Rev 1.10   03 Jun 1998 10:10:12   sally
// change parameter names and types due to LP's changes
// 
//    Rev 1.9   01 May 1998 14:46:56   sally
// add HK2 file
// 
//    Rev 1.8   20 Apr 1998 10:22:44   sally
// change for WindSwatch
// 
//    Rev 1.7   17 Apr 1998 16:50:48   sally
// add L2A and L2B file formats
// 
//    Rev 1.6   30 Mar 1998 15:14:02   sally
// added L2A parameter table
// 
//    Rev 1.5   27 Mar 1998 09:59:56   sally
// added L1A Derived data
// 
//    Rev 1.4   24 Mar 1998 15:57:16   sally
// de-warn for GNU
// 
//    Rev 1.3   23 Mar 1998 15:37:30   sally
// adapt to derived science data
// 
//    Rev 1.2   20 Feb 1998 10:59:02   sally
// L1 to L1A
// 
//    Rev 1.1   12 Feb 1998 16:47:42   sally
// add start and end time
// Revision 1.3  1998/02/03 00:12:33  sally
// change NRT to L1AP
//
// Revision 1.2  1998/01/30 22:29:19  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

#include <malloc.h>
#include <memory.h>
#include <string.h>

#include "Parameter.h"
#include "ParTab.h"
#include "Itime.h"
#include "HdfFile.h"

static const char rcs_id[] = "@(#) $Id$";

//**********************************************************************
// GetSourceId (source_string)
//
// RETURN: The source id based on the source string (HK2, L1A, etc.)
//**********************************************************************

SourceIdE
ParTabAccess::GetSourceId(
    const char  *source_string)
{
    SourceIdE sourceId;

    if (strcasecmp((char *)source_string, SOURCE_L1A_STRING) == 0)
        sourceId = SOURCE_L1A;
    else if (strcasecmp((char *)source_string, SOURCE_HK2_STRING) == 0)
        sourceId = SOURCE_HK2;
    else if (strcasecmp((char *)source_string, SOURCE_L1AP_STRING) == 0)
        sourceId = SOURCE_L1AP;
    else if (strcasecmp((char *)source_string, SOURCE_L1A_DERIVED_STRING) == 0)
        sourceId = SOURCE_L1A_DERIVED;
    else if (strcasecmp((char *)source_string, SOURCE_L1B_STRING) == 0)
        sourceId = SOURCE_L1B;
    else if (strcasecmp((char *)source_string, SOURCE_L2A_STRING) == 0)
        sourceId = SOURCE_L2A;
    else if (strcasecmp((char *)source_string, SOURCE_L2B_STRING) == 0)
        sourceId = SOURCE_L2B;
    else
        sourceId = SOURCE_UNKNOWN;

    return sourceId;
}

const char*
ParTabAccess::GetSdsNames(
SourceIdE   sourceId,
ParamIdE    paramId)
{
    int i;

    // get the proper parameter table
    const ParTabEntry* paramTable;
    int tableSize;
    if (! _GetParTab(sourceId, paramTable, tableSize))
        return NULL;

    // search for the entry
    for (i = 0; i < tableSize; i++)
    {
        if (paramTable[i].paramId == paramId)
            return(paramTable[i].sdsNames);
    }

    return 0;

}//ParTabAccess::GetSdsNames

//**********************************************************************
// GetParameter (sourceId, paramId, unitId)
//
// RETURN: A pointer to a new Parameter having the specified sourceId,
//          paramId, and unitId
//**********************************************************************

Parameter*
ParTabAccess::GetParameter(
    SourceIdE   sourceId,
    ParamIdE    paramId,
    UnitIdE     unitId)
{
    int i;

    // get the proper parameter table
    const ParTabEntry* paramTable;
    int tableSize;
    if (! _GetParTab(sourceId, paramTable, tableSize))
    {
        return NULL;
    }

    // search for the entry
    unsigned long entry_index=0;
    char found=FALSE;
    for (i = 0; i < tableSize; i++)
    {
        if (paramTable[i].paramId == paramId)
        {
            found = TRUE;
            entry_index = i;
            break;
        }
    }
    if (found != TRUE)
    {
        return NULL;
    }

    // search for the unit
    found = FALSE;
    unsigned long unit_index=0;
    for (i = 0; i < (int)paramTable[entry_index].numUnitEntries; i++)
    {
        if (paramTable[entry_index].unitEntries[i].unitId == unitId)
        {
            found = TRUE;
            unit_index = i;
            break;
        }
    }
    if (found != TRUE)
    {
        return NULL;
    }

    Parameter *param = ParamAssign(sourceId, paramTable, entry_index,
        unit_index);
    return param;
}

//**********************************************************************
// GetParameter (sourceId, paramString)
//
// RETURN: A pointer to a new Parameter having the specified source id
//          and paramString (paramId.unitId)
//**********************************************************************

Parameter*
ParTabAccess::GetParameter(
    SourceIdE   sourceId,
    char*       paramString)
{
    ParamIdE    paramId;
    UnitIdE     unitId;
    if (sscanf(paramString, "%d.%d", (int*)&paramId, (int*)&unitId) != 2)
        return (NULL);
    return (GetParameter(sourceId, paramId, unitId));
}

//**********************************************************************
// GetParameter (sourceId, paramName, unitName)
//
// RETURN: A pointer to a new Parameter having the specified sourceId,
//          paramName, and unitName
//**********************************************************************

Parameter*
ParTabAccess::GetParameter(
    SourceIdE   sourceId,
    char        *paramName,
    char        *unitName)
{
    int i;

    // get the proper parameter table
    const ParTabEntry* paramTable;
    int tableSize;
    if (! _GetParTab(sourceId, paramTable, tableSize))
    {
        return NULL;
    }

    // search for the entry
    unsigned long entry_index=0;
    char found = FALSE;
    for (i = 0; i < tableSize; i++)
    {
        if (strcmp(paramTable[i].paramName, paramName) == 0)
        {
            found = TRUE;
            entry_index = i;
            break;
        }
    }
    if (found != TRUE)
    {
        return NULL;
    }

    // search for the unit
    found = FALSE;
    unsigned long unit_index=0;
    for (i = 0; i < (int)paramTable[entry_index].numUnitEntries; i++)
    {
        if (strcmp(paramTable[entry_index].unitEntries[i].unitName,
            unitName) == 0)
        {
            found = TRUE;
            unit_index = i;
            break;
        }
    }
    if (found != TRUE)
    {
        return NULL;
    }

    Parameter *param = ParamAssign(sourceId, paramTable, entry_index,
        unit_index);
    return param;
}

//**********************************************************************
// GetParamUnits(sourceId, paramName, units, unitsSize)
//
// RETURN: FALSE on failure, TRUE on success
//   SETS: units to contain an array of unit strings
//         unitsSize to contain the size of the unit array
//**********************************************************************

char
ParTabAccess::GetParamUnits(
    SourceIdE   sourceId,
    char*       paramName,
    char**&     units,
    int&        unitsSize)
{
    const ParTabEntry* tableEntry = GetParTabEntry(sourceId, paramName);
    if (tableEntry == NULL)
    {
        return(FALSE);
    }

    // allocate array of char*
    units = (char**) malloc ((int) tableEntry->numUnitEntries *
            sizeof(char*));

    if (units == NULL)
    {
        return(FALSE);
    }

    for (unsigned long i = 0; i < tableEntry->numUnitEntries; i++)
    {
        units[i] = (char*) malloc(strlen(tableEntry->unitEntries[i].unitName)
                                    + 1);
        if (units[i] == NULL)
        {
            for (unsigned long j = 0; j < i; j++)
            {
                free(units[j]);
            }
            free((char *)units);
            return(FALSE);
        }
        else
        {
            (void) strcpy(units[i], tableEntry->unitEntries[i].unitName);
        }
    }
    unitsSize = (int) tableEntry->numUnitEntries;

    return(TRUE);
} // GetParamUnits

//**********************************************************************
// ParamAssign(sourceId, paramTable, entry_index, unit_index)
//
// RETURN: Pointer to a new Parameter containing information specified
//          in entry_index/unit_index; NULL on error
//**********************************************************************

Parameter
*ParTabAccess::ParamAssign(
    SourceIdE           sourceId,
    const ParTabEntry*  paramTable,
    unsigned long       entry_index,
    unsigned long       unit_index)
{
    const ParTabEntry *tableEntry;
    tableEntry = &(paramTable[entry_index]);

    Parameter *param = new Parameter();
    if (param == NULL)
    {
        return NULL;
    }

    param->paramId = tableEntry->paramId;
    (void) strncpy(param->paramName, tableEntry->paramName, STRING_LEN);
    param->sourceId = sourceId;
    (void) strncpy(param->measurable, tableEntry->measurable, STRING_LEN);
    param->unitId = tableEntry->unitEntries[unit_index].unitId;
    (void) strncpy(param->unitName,
        tableEntry->unitEntries[unit_index].unitName, STRING_LEN);
    param->sdsIDs = 0;
    param->numSDSs = 0;
    (void) strncpy(param->sdsNames, tableEntry->sdsNames, BIG_SIZE);
    char tempString[BIG_SIZE];
    (void)strncpy(tempString, tableEntry->sdsNames, BIG_SIZE);
    char* oneSdsName=0;
    for (oneSdsName = (char*)strtok(tempString, ","); oneSdsName;
               oneSdsName = (char*)strtok(0, ","))
    {
        (param->numSDSs)++;
    }
    param->sdsIDs = new int32[param->numSDSs];
    for (int i=0; i < param->numSDSs; i++)
    {
        param->sdsIDs[i] = HDF_FAIL;
    }
    param->dataType = tableEntry->unitEntries[unit_index].dataType;
    param->needPolynomial = tableEntry->unitEntries[unit_index].needPolynomial;
    switch (param->dataType)
    {
    case DATA_UNKNOWN:
        param->byteSize = 0;
        break;
    case DATA_CHAR1:
    case DATA_UINT1:
    case DATA_INT1:
        param->byteSize = 1;
        break;
    case DATA_INT1_76:
        param->byteSize = 76;
        break;
    case DATA_INT1_810:
        param->byteSize = 810;
        break;
    case DATA_CHAR2:
    case DATA_UINT2:
    case DATA_INT2:
        param->byteSize = 2;
        break;
    case DATA_CHAR3:
        param->byteSize = 3;
        break;
    case DATA_CHAR4:
    case DATA_UINT4:
    case DATA_INT4:
    case DATA_FLOAT4:
        param->byteSize = 4;
        break;
    case DATA_ITIME:
        param->byteSize = 6;
        break;
    case DATA_UINT2_4:
    case DATA_FLOAT8:
        param->byteSize = 8;
        break;
    case DATA_UINT2_5:
        param->byteSize = 10;
        break;
    case DATA_CHAR13:
        param->byteSize = 13;
        break;
    case DATA_CHAR16:
    case DATA_UINT4_4:
        param->byteSize = 16;
        break;
    case DATA_UINT2_12:
        param->byteSize = 24;
        break;
    case DATA_CHAR28:
        param->byteSize = 28;
        break;
    case DATA_CHAR32:
    case DATA_UINT2_2_8:
        param->byteSize = 32;
        break;
    case DATA_UINT4_12:
        param->byteSize = 48;
        break;
    case DATA_UINT1_49:
        param->byteSize = 49;
        break;
    case DATA_UINT2_25:
        param->byteSize = 50;
        break;
    case DATA_UINT2_76:
        param->byteSize = 152;
        break;
    case DATA_UINT4_25:
    case DATA_FLOAT4_25:
        param->byteSize = 100;
        break;
    case DATA_UINT2_100:
    case DATA_INT2_100:
        param->byteSize = 200;
        break;
    case DATA_FLOAT4_76:
        param->byteSize = 304;
        break;
    case DATA_UINT4_100:
    case DATA_FLOAT4_100:
        param->byteSize = 400;
        break;
    case DATA_FLOAT4_76_4:
        param->byteSize = 1216;
        break;
    case DATA_UINT2_810:
        param->byteSize = 1620;
        break;
    case DATA_UINT2_100_12:
        param->byteSize = 2400;
        break;
    case DATA_FLOAT4_100_8:
        param->byteSize = 3200;
        break;
    case DATA_FLOAT4_810:
        param->byteSize = 3240;
        break;
    case DATA_UINT4_100_12:
        param->byteSize = 4800;
        break;
    default:
        delete param;
        return 0;
        break;
    }
    param->extractFunc = tableEntry->unitEntries[unit_index].extractFunc;
    param->printFunc = tableEntry->unitEntries[unit_index].printFunc;
    param->data = NULL;

    return param;
} // ParamAssign

//**********************************************************************
// GetParTabEntry (sourceId, paramName)
//
// RETURN: Pointer to the table entry corresponding to the parameter
//**********************************************************************

const ParTabEntry
*ParTabAccess::GetParTabEntry(
SourceIdE       sourceId,       // SOURCE_HK2, SOURCE_L1A or SOURCE_L1AP
const char*     paramName)      // the parameter name
{
    // get the proper parameter table
    const ParTabEntry* paramTable;
    int tableSize;
    if (! _GetParTab(sourceId, paramTable, tableSize))
    {
        return NULL;
    }

    // search for the entry
    unsigned long entry_index;
    char found=FALSE;
    int i=0;
    for (i = 0; i < tableSize; i++)
    {
        if (strcmp(paramTable[i].paramName, paramName) == 0)
        {
            found = TRUE;
            entry_index = i;
            break;
        }
    }
    if (found != TRUE)
    {
        return (NULL);
    }
    else
    {
        return (&paramTable[i]);
    }
} //ParTabAccess::GetParTabEntry

//**********************************************************************
// _GetParTab (sourceId, paramTable, tableSize)
//
// EFFECT: Sets paramTable and tableSize
// RETURN: TRUE if sourceId ok; else FALSE
//*************************************************************

char
ParTabAccess::_GetParTab(
SourceIdE   sourceId,           // SOURCE_HK2, SOURCE_L1A or SOURCE_L1AP
const ParTabEntry*& paramTable, // RETURN: pointer to parameter table
int&            tableSize)      // RETURN: size of parameter table
{
    switch (sourceId)
    {
        case SOURCE_HK2:
            paramTable = (ParTabEntry *)HK2ParTab;
            tableSize = HK2ParTabSize;
            return (TRUE);
        case SOURCE_L1A:
        case SOURCE_L1AP:
            paramTable = (ParTabEntry *)L1AParTab;
            tableSize = L1AParTabSize;
            return (TRUE);

        case SOURCE_L1A_DERIVED:
            paramTable = (ParTabEntry *)L1ADerivedParTab;
            tableSize = L1ADerivedTabSize;
            return (TRUE);

        case SOURCE_L1B:
            paramTable = (ParTabEntry *)L1BParTab;
            tableSize = L1BParTabSize;
            return (TRUE);

        case SOURCE_L2A:
            paramTable = (ParTabEntry *)L2AParTab;
            tableSize = L2AParTabSize;
            return (TRUE);

        case SOURCE_L2B:
            paramTable = (ParTabEntry *)L2BParTab;
            tableSize = L2BParTabSize;
            return (TRUE);

        default:
            return (FALSE);
    }
} //ParTabAccess::_GetParTab

//static
int
ParTabAccess::GetParamMaxLen(
SourceIdE source)   // SOURCE_HK2, SOURCE_L1A or SOURCE_L1AP
{
    // get the proper parameter table
    const ParTabEntry* paramTable;
    int tableSize;
    if (! _GetParTab(source, paramTable, tableSize))
    {
        return 0;
    }

    // find the maximum param string len
    int maxLen = 0;
    for (int i = 0; i < tableSize; i++)
    {
        maxLen = MAX_OF_TWO (maxLen, (int)strlen(paramTable[i].paramName));
    }
    return (maxLen);

}//ParTabAccess::GetParamMaxLen

//static
int
ParTabAccess::GetUnitMaxLen(
SourceIdE source)   // SOURCE_HK2, SOURCE_L1A or SOURCE_L1AP
{
    // get the proper parameter table
    const ParTabEntry* paramTable;
    int tableSize;
    if (! _GetParTab(source, paramTable, tableSize))
    {
        return 0;
    }

    // find the maximum param string len
    int maxLen = 0;
    for (int i = 0; i < tableSize; i++)
    {
        for (int j = 0; j < (int)paramTable[i].numUnitEntries; j++)
            maxLen = MAX_OF_TWO (maxLen,
                        (int)strlen(paramTable[i].unitEntries[j].unitName));
    }
    return (maxLen);

}//ParTabAccess::GetUnitMaxLen
