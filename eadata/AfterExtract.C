//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
// CM Log
// $Log$
// 
//    Rev 1.0   17 Apr 1998 16:51:50   sally
// Initial revision.
// 
// $Date$
// $Revision$
// $Author$
//
//=========================================================

#include "assert.h"
#include "Parameter.h"

int
EaLinearToLog(
int       numData,       // IN: number in linear data array
double*   linearData,    // IN: linear data array
double*   logData)       // OUT: log data array
{
    assert(numData > 0 && linearData != 0 && logData != 0);

    for (int i=0; i < numData; i++)
    {
        logData[i] = 10 * log10(linearData[i]);
    }
    return 1;

} // EaLinearToLog
