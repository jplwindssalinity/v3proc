//=========================================================
// Copyright  (C)1995, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.2   20 Apr 1998 15:18:18   sally
// change List to EAList
// 
//    Rev 1.1   20 Feb 1998 10:56:56   sally
// L1 to L1A
// 
//    Rev 1.0   04 Feb 1998 14:15:26   daffer
// Initial checking
// Revision 1.3  1998/01/30 22:28:18  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

#ifndef FILTER_H
#define FILTER_H

static const char rcs_id_filter_h[] = "@(#) $Header$";

#include <stdio.h>
#include <string.h>

#include "EAList.h"
#include "TlmHdfFile.h"

#define FILTER_ABBR_LENGTH          8
#define FILTER_NAME_LENGTH          64
#define FILTER_LIST_STRING_LENGTH   64
#define FILTER_SET_STRING_LENGTH    128

#define STRING_OR_CHAR              '+'
#define STRING_AND_CHAR             '.'

#define NUM_MAX_FILTER_PARAM_ENTRIES  3

class Filter;
typedef char (*FilterFunc) (Filter*, TlmHdfFile*, int32);

//=============
// FilTabEntry 
//=============

struct FilterParamEntry
{
    SourceIdE          sourceId;
    ParamIdE           paramId;
    UnitIdE            unitId;
};

struct FilTabEntry
{
    char*              filterAbbr;
    char*              filterName;
    FilterFunc         filterFunc;

    unsigned long      numParamEntries;
    FilterParamEntry   paramEntries[NUM_MAX_FILTER_PARAM_ENTRIES];
};

//========
// Filter 
//========

class Filter
{
public:
    friend int operator==(const Filter&, const Filter&);

    Filter(char* abbr, const FilTabEntry* table);
    virtual ~Filter();
    char    Pass(TlmHdfFile* tlmFile, int32 startIndex);

    char        filterAbbr[FILTER_ABBR_LENGTH]; // an abbreviation
    char        filterName[FILTER_NAME_LENGTH]; // the name of the filter
    FilterFunc  filterFunc;                     // the filter function
    Parameter*  parametersP[NUM_MAX_FILTER_PARAM_ENTRIES];

    const FilTabEntry* _filterTable;
    int                _filterIndex;
};

inline int operator==(const Filter& a, const Filter& b)
{
    return(strcmp(a.filterName, b.filterName) == 0 ? 1 : 0);
}

//============
// FilterList 
//============

class FilterList : public EAList<Filter>
{
public:
    friend int operator==(const Filter&, const Filter&);

    // badString: string returned to user if it is bad
    FilterList(char* string, const FilTabEntry* table, char* badString);

    virtual ~FilterList();

    IotBoolean   OpenParamDataSets(TlmHdfFile*);
    IotBoolean   CloseParamDataSets(TlmHdfFile*);

    char         Pass(TlmHdfFile* tlmFile, int32 startIndex);

    char         listString[FILTER_LIST_STRING_LENGTH];

}; // Parameter

inline int operator==(const FilterList& a, const FilterList& b)
{
    return(strcmp(a.listString, b.listString) == 0 ? 1 : 0);
}

//===========
// FilterSet 
//===========

class FilterSet : public EAList<FilterList>
{
public:
    enum StatusE
    {
        OK,
        ERROR_STRING_EXISTS
    };

    FilterSet();

    // badString: string returned to user if it is bad
    FilterSet(const char* string, const FilTabEntry* table, char* badString);

    virtual ~FilterSet();

    IotBoolean   OpenParamDataSets(TlmHdfFile*);
    IotBoolean   CloseParamDataSets(TlmHdfFile*);

    char         Pass(TlmHdfFile* tlmFile, int32 startIndex);

    char         setString[FILTER_SET_STRING_LENGTH];
};

extern const FilTabEntry HkdtFilTab[];
 
extern const FilTabEntry L1AFilTab[];

#endif //FILTER_H
