//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.4   17 Apr 1998 16:51:12   sally
// add L2A and L2B file formats
// 
//    Rev 1.3   27 Mar 1998 10:00:00   sally
// added L1A Derived data
// 
//    Rev 1.2   23 Mar 1998 15:39:44   sally
// adapt to derived science data
// 
//    Rev 1.1   20 Feb 1998 10:59:12   sally
// L1 to L1A
// 
//    Rev 1.0   04 Feb 1998 14:16:38   daffer
// Initial checking
// Revision 1.3  1998/02/03 00:12:33  sally
// change NRT to L1AP
//
// Revision 1.2  1998/01/30 22:29:20  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

#include <stdio.h>
#include <string.h>
#include "Parameter.h"

static const char rcs_id_Parameter_C[] =
    "@(#) $Header$";

const char *source_id_map[] =
{
    SOURCE_UNKNOWN_STRING,
    SOURCE_L1A_STRING,
    SOURCE_L1AP_STRING,
    SOURCE_L1A_DERIVED_STRING,
    SOURCE_HK2_STRING,
    SOURCE_L2A_STRING,
    SOURCE_L2B_STRING
};

// constructor
Parameter::Parameter()
:   paramId(PARAM_UNKNOWN), sourceId(SOURCE_UNKNOWN),
    sdsIDs(0), numSDSs(0), unitId(UNIT_UNKNOWN),
    dataType(DATA_UNKNOWN), needPolynomial(0), byteSize(0),
    extractFunc(NULL), printFunc(NULL), data(NULL), held(0)
{
    paramName[0] = '\0';
    measurable[0] = '\0';
    sdsNames[0] = '\0';
    unitName[0] = '\0';
    byteSize = 0;
    return;
} // Parameter::Parameter

// copy constructor
Parameter::Parameter(
const Parameter&        other)
:   paramId(other.paramId), sourceId(other.sourceId),
    unitId(other.unitId), dataType(other.dataType),
    needPolynomial(other.needPolynomial), byteSize(other.byteSize),
    extractFunc(other.extractFunc), printFunc(other.printFunc),
    data(0),    // data is specifically not copied
    held(other.held)
{
    (void) strcpy (paramName, other.paramName);
    (void) strcpy (measurable, other.measurable);
    (void) strcpy (sdsNames, other.sdsNames);
    (void) strcpy (unitName, other.unitName);
    numSDSs = other.numSDSs;
    sdsIDs = new int32[numSDSs];
    for (int i=0; i < numSDSs; i++)
    {
        sdsIDs[i] = other.sdsIDs[i];
    }
    return;

}//Parameter::Parameter

// destructor
Parameter::~Parameter()
{
    if (sdsIDs) delete [] sdsIDs;
    if (data) delete data;
    numSDSs = 0;
    sdsIDs =0;

}//Parameter::~Parameter

// assign operator
Parameter&
Parameter::operator= (
    const Parameter&    other)
{
    paramId = other.paramId;
    sourceId = other.sourceId;
    unitId = other.unitId;
    dataType = other.dataType;
    needPolynomial = other.needPolynomial;
    byteSize = other.byteSize;
    extractFunc = other.extractFunc;
    printFunc = other.printFunc;
    data = 0;   // data is specifically not copied
    held = other.held;
    (void) strcpy (paramName, other.paramName);
    (void) strcpy (measurable, other.measurable);
    (void) strcpy (sdsNames, other.sdsNames);
    (void) strcpy (unitName, other.unitName);
    return (*this);
}
