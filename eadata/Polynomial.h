//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.1   12 Feb 1998 16:49:36   sally
// add wrappers for "C" functions
// 
//    Rev 1.0   04 Feb 1998 14:16:52   daffer
// Initial checking
// Revision 1.2  1998/01/30 22:28:40  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

#ifndef POLYNOMIAL_H
#define POLYNOMIAL_H

static const char rcs_id_polynomial_h[] =
    "@(#) $Header$";

#include <string.h>

#include "CommonDefs.h"

//-********************************************************************
//     define polynomial operation
//-********************************************************************

class Polynomial
{
public:
    friend int operator==(const Polynomial&, const Polynomial&);

    Polynomial(const char*   varName,    // IN
               const char*   unitName,   // IN
               const float*  array,      // IN: array of numbers
               int           num);       // IN: number of elements in array
    virtual ~Polynomial();

    const char*  GetVarName(void) const  { return _varName; }
    const char*  GetUnitName(void) const { return _unitName; }

    float        Apply(float x)const;    // IN: apply polynomial to x
    double       Apply(double x)const;   // IN: apply polynomial to x

                 // NOTE: user must provide space for outArray
    void         ApplyArray(
                       const float* inArray, // IN: array for float DN
                       int          num,     // IN: number of elements in array
                       float*       outArray)const;// OUT: array for float EU

                 // NOTE: user must provide space for outArray
    void         ApplyArray(
                       const double* inArray, // IN: array for double DN
                       int           num,     // IN: number of elements in array
                       double*       outArray)const;// OUT: array for double EU

    void         ApplyReplaceArray(
                       float* array,     // OUT: apply polynomial to array
                       int    num)const; // IN: number of elements in array
    void         ApplyReplaceArray(
                       double* array,    // OUT: apply polynomial to array
                       int     num)const;// IN: number of elements in array

protected:
    char       _varName[STRING_LEN];
    char       _unitName[STRING_LEN];
    float*     _factors;
    int        _order;
};

inline int operator==(const Polynomial& a, const Polynomial& b)
{
    return(strcmp(a._varName, b._varName) == 0 &&
           strcmp(a._unitName, b._unitName) == 0 ? 1 : 0);
}

#endif //POLYNOMIAL_H
