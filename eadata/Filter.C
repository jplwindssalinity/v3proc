//=========================================================
// Copyright  (C)1995, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.3   20 Apr 1998 15:18:16   sally
// change List to EAList
// 
//    Rev 1.2   24 Mar 1998 15:57:00   sally
// de-warn for GNU
// 
//    Rev 1.1   23 Mar 1998 15:34:42   sally
// adapt to derived science data
// 
//    Rev 1.0   04 Feb 1998 14:15:24   daffer
// Initial checking
// Revision 1.3  1998/01/30 22:29:03  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

//------------------------------------------------------------------------
// Filter:  contains one FilTabEntry (could have two or more parameters)
//          one abbreviation
// FilterList: list of Filter
// FilterSet:  list of FilterList
//------------------------------------------------------------------------

#ifndef FILTER_C_INCLUDED
#define FILTER_C_INCLUDED

static const char rcs_id_Filter_C[] = "@(#) $Header$";

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "Filter.h"
#include "ParTab.h"

#define TOKENS "."

//================
// Filter methods 
//================

Filter::Filter(
    char*               abbr,
    const FilTabEntry*  table)
:   filterFunc(NULL), _filterTable(table)
{
    strcpy(filterAbbr, abbr);

    for (int k=0; k < NUM_MAX_FILTER_PARAM_ENTRIES; k++)
        parametersP[k] = 0;

    // look up the abbreviation
    int filter_index = 0;
    while (table[filter_index].filterName != NULL)
    {
        if (strcasecmp(abbr, table[filter_index].filterAbbr) == 0)
        {
            strcpy(filterAbbr, table[filter_index].filterAbbr);
            strcpy(filterName, table[filter_index].filterName);
            filterFunc = table[filter_index].filterFunc;

            for (int j=0; j < NUM_MAX_FILTER_PARAM_ENTRIES; j++)
                parametersP[j] = 0;
            for (unsigned long i=0;
                        i < table[filter_index].numParamEntries; i++)
            {
                const FilterParamEntry* paramEntry =
                              &(table[filter_index].paramEntries[i]);
                parametersP[i] = ParTabAccess::GetParameter(
                                            paramEntry->sourceId,
                                            paramEntry->paramId,
                                            paramEntry->unitId);
                if (parametersP[i] == 0)
                {
                    fprintf(stderr, "Error creating Filter '%s'\n", filterName);
                    exit(1);
                }

            }
            _filterIndex = filter_index;
            break;
        }
        filter_index++;
    } // while

    return;
}

Filter::~Filter()
{
    for (unsigned long i=0; i < _filterTable[_filterIndex].numParamEntries; i++)
    {
        if (parametersP[i])
            delete parametersP[i];
    }
    return;
}

//------
// Pass 
//------

char
Filter::Pass(
TlmHdfFile*    tlmFile,
int32          startIndex)
{
    //------------------
    // check the filter 
    //------------------

    return (filterFunc(this, tlmFile, startIndex));
}

//====================
// FilterList methods 
//====================

FilterList::FilterList(
char*               string,
const FilTabEntry*  table,
char*               badString)
:   EAList<Filter>()
{
    strcpy(listString, string);

    //--------------------
    // create each filter 
    //--------------------

    char filterAbbr[FILTER_ABBR_LENGTH];
    char* fromPtr = listString;
 
    do
    {
        char* toPtr = filterAbbr;
        while (*fromPtr != STRING_AND_CHAR && *fromPtr != '\0')
        {
            *toPtr = *fromPtr;
            fromPtr++;
            toPtr++;
        }
        *toPtr = '\0';
        Filter* f = new Filter(filterAbbr, table);
        if (f->filterFunc == NULL)
        {
            (void)strcpy(badString, filterAbbr);
            break;
        }
        Append(f);
    } while (*(fromPtr++) != '\0');

    return;
}

FilterList::~FilterList()
{
    // empty
    return;
}

IotBoolean
FilterList::OpenParamDataSets(
TlmHdfFile*    tlmFile)
{
    for (Filter* filterP = GetHead(); filterP; filterP = GetNext())
    {
        // go thru all parameter in the "filterP" and open all data sets
        // return FALSE right away if any one fails
        Parameter* paramP=0;
        for (int i=0; i < NUM_MAX_FILTER_PARAM_ENTRIES; i++)
        {
            if ((paramP = filterP->parametersP[i]) == 0)
                break;
            char tempString[BIG_SIZE];
            (void)strncpy(tempString, paramP->sdsNames, BIG_SIZE);
            int32 dataType, dataStartIndex, dataLength, numDimensions;
            char *oneSdsName = (char*)strtok(tempString, ",");
            if (oneSdsName)
            {
                paramP->sdsIDs[0] = HDF_FAIL;
                paramP->sdsIDs[0] = tlmFile->SelectDataset(
                                         oneSdsName, dataType, dataStartIndex,
                                         dataLength, numDimensions);
                if (paramP->sdsIDs[0] == HDF_FAIL)
                    return(0);
            }
            else
                return 0;
        }
    }
    return(1);
 
}//FilterList::OpenParamDataSets
 
IotBoolean
FilterList::CloseParamDataSets(
TlmHdfFile*    tlmFile)
{
    IotBoolean closeOK = 1;
    for (Filter* filterP = GetHead(); filterP; filterP = GetNext())
    {
        // go thru all parameter in the "filterP" and close all data sets
        // continue to close even any one fails
        Parameter* paramP=0;
        for (int i=0; i < NUM_MAX_FILTER_PARAM_ENTRIES; i++)
        {
            if ((paramP = filterP->parametersP[i]) == 0)
                break;
            if (tlmFile->CloseDataset(paramP->sdsIDs[0]) == HDF_FAIL)
                closeOK = 0;
            paramP->sdsIDs[0] = HDF_FAIL;
        }
    }
    return(closeOK);
 
}//FilterList::CloseParamDataSets
 

//------
// Pass 
//------

char
FilterList::Pass(
TlmHdfFile*    tlmFile,
int32          startIndex)
{
    //-----------------
    // AND each filter 
    //-----------------

    Filter* filter = GetHead();
    while (filter != NULL)
    {
        if (! filter->Pass(tlmFile, startIndex))
            return (0);
        else
            filter = GetNext();
    }
    return (1);
}

//===================
// FilterSet methods 
//===================

FilterSet::FilterSet(
const char*         string,
const FilTabEntry*  table,
char*               badString)
:   EAList<FilterList>()
{
    (void)strcpy(setString, string);

    //-------------------------
    // create each filter list 
    //-------------------------
    char listString[FILTER_LIST_STRING_LENGTH];
    char* fromPtr = setString;

    do
    {
        char* toPtr = listString;
        while (*fromPtr != STRING_OR_CHAR && *fromPtr != '\0')
        {
            *toPtr = *fromPtr;
            fromPtr++;
            toPtr++;
        }
        *toPtr = '\0';
        if (strlen(listString) > 0)
        {
            FilterList* flist = new FilterList(listString, table, badString);
            if (badString[0] != '\0')
                return;
            Append(flist);
        }
    } while (*(fromPtr++) != '\0');

    return;

}//FilterSet::FilterSet

FilterSet::FilterSet()
:   EAList<FilterList>()
{
    setString[0] = '\0';
    return;
}

FilterSet::~FilterSet()
{
    // empty
    return;
}

IotBoolean
FilterSet::OpenParamDataSets(
TlmHdfFile*    tlmFileP)
{
    // go thru all filterlist in the filter set, open all the datasets
    // return FALSE right away if any one fails
    for (FilterList* filterListP = GetHead(); filterListP;
                                           filterListP = GetNext())
    {
        if (filterListP->OpenParamDataSets(tlmFileP) == 0)
            return(0);
    }
    return(1);
    
}//FilterSet::OpenParamDataSets

IotBoolean
FilterSet::CloseParamDataSets(
TlmHdfFile*    tlmFileP)
{
    IotBoolean closeOK = 1;
    // go thru all filterlist in the filter set, close all the datasets
    // continue to close even if any one fails
    for (FilterList* filterListP = GetHead(); filterListP;
                                           filterListP = GetNext())
    {
        if (filterListP->OpenParamDataSets(tlmFileP) == 0)
            closeOK = 0;
    }
    return(closeOK);
    
}//FilterSet::CloseParamDataSets

//------
// Pass 
//------
char
FilterSet::Pass(
TlmHdfFile*    tlmFile,
int32          startIndex)
{
    //---------------------
    // OR each filter list 
    //---------------------

    FilterList* flist = GetHead();

    //--------------------------------------------
    // if the set is empty, always pass
    //--------------------------------------------
    if (flist == NULL)
        return (1);

    while (flist != NULL)
    {
        if (flist->Pass(tlmFile, startIndex))
            return (1);
        else
            flist = GetNext();
    }
    return (0);

}//FilterSet::Pass

#endif
