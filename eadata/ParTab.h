//=========================================================
// Copyright  (C)1995, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.10   07 Oct 1999 14:00:12   sally
// added L2Ahr file type
// 
//    Rev 1.9   25 May 1999 14:05:52   sally
// add L2Ax for Bryan Stiles
// 
//    Rev 1.8   13 Oct 1998 15:34:22   sally
// added L1B file
// 
//    Rev 1.7   01 May 1998 14:47:00   sally
// add HK2 file
// 
//    Rev 1.6   17 Apr 1998 16:51:08   sally
// add L2A and L2B file formats
// 
//    Rev 1.5   27 Mar 1998 09:59:58   sally
// added L1A Derived data
// 
//    Rev 1.4   23 Mar 1998 15:39:40   sally
// adapt to derived science data
// 
//    Rev 1.3   19 Mar 1998 13:37:14   sally
//  added "days", "hours", "minutes" and "seconds" units
// 
//    Rev 1.2   20 Feb 1998 10:59:08   sally
// L1 to L1A
// 
//    Rev 1.1   12 Feb 1998 16:47:44   sally
// add start and end time
// Revision 1.3  1998/02/03 00:12:33  sally
// change NRT to L1AP
//
// Revision 1.2  1998/01/30 22:28:34  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

#ifndef PARTAB_H
#define PARTAB_H

static const char rcs_id_partab_h[] =
    "@(#) $Id$";

#include "CommonDefs.h"
#include "Parameter.h"

#define NUM_MAX_UNIT_ENTRIES    8

struct UnitEntry
{
    UnitIdE         unitId;
    char*           unitName;
    DataTypeE       dataType;
    IotBoolean      needPolynomial;
    ExtractFunc     extractFunc;
    PrintFunc       printFunc;
};
 
struct ParTabEntry
{
    ParamIdE        paramId;
    char*           paramName;
    SourceIdE       sourceId;
    char*           measurable;
    char*           sdsNames;     // HDF SDS name
    unsigned long   numUnitEntries;
    UnitEntry       unitEntries[NUM_MAX_UNIT_ENTRIES];
};

extern const ParTabEntry HK2ParTab[];
extern const int HK2ParTabSize;
 
extern const ParTabEntry L1AParTab[];
extern const int L1AParTabSize;
 
extern const ParTabEntry L1ADerivedParTab[];
extern const int L1ADerivedTabSize;
 
extern const ParTabEntry L1BParTab[];
extern const int L1BParTabSize;
 
extern const ParTabEntry L2AParTab[];
extern const int L2AParTabSize;
 
extern const ParTabEntry L2AxParTab[];
extern const int L2AxParTabSize;

extern const ParTabEntry L2AhrParTab[];
extern const int L2AhrParTabSize;
 
extern const ParTabEntry L2BParTab[];
extern const int L2BParTabSize;
 

class ParTabAccess
{
public:

    static SourceIdE GetSourceId(
                const char* source_string);

    // user must "delete Parameter" when done for all GetParameter funcs

    static const char* GetSdsNames(
                SourceIdE sourceId,
                ParamIdE paramId);

    static Parameter* GetParameter(
                SourceIdE sourceId,
                ParamIdE paramId, 
                UnitIdE unitId);

    static Parameter* GetParameter(
                SourceIdE sourceId,
                char*   paramString);

    static Parameter* GetParameter(
                SourceIdE sourceId,
                char *paramName,
                char *unitName);

    static char GetParamUnits(
                SourceIdE sourceId,
                char*   paramName,
                char**& units,      // RETURN: array of unit strings
                int&    unitsSize); // RETURN: size of units array);

    static Parameter* ParamAssign(
                SourceIdE           sourceId,
                const ParTabEntry*  parTab,
                unsigned long       entry_index,
                unsigned long       unit_index);

                // get the param names' max string len
    static int  GetParamMaxLen(
                SourceIdE source);  // SOURCE_HK2, SOURCE_L1A or SOURCE_NRT

                // get the unit names' max string len
    static int  GetUnitMaxLen(
                SourceIdE source);  // SOURCE_HK2, SOURCE_L1A or SOURCE_NRT

    static const ParTabEntry* GetParTabEntry(
                SourceIdE       sourceId,
                const char*     paramName);

private:
    static char _GetParTab(
                SourceIdE source,   // SOURCE_HK2, SOURCE_L1A or SOURCE_NRT
                const ParTabEntry*& parTab, // RETURN: ptr to param table
                int& parTabSize);   // RETURN: size of parameter table

}; //ParTabAccess

#endif //PARTAB_H
