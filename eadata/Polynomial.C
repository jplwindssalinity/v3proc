//=========================================================
// Copyright  (C)1998, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.1   12 Feb 1998 16:49:34   sally
// add wrappers for "C" functions
// 
//    Rev 1.0   04 Feb 1998 14:16:50   daffer
// Initial checking
// Revision 1.3  1998/01/30 22:29:21  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================


#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "Polynomial.h"

Polynomial::Polynomial(
const char*   varName,     // IN
const char*   unitName,    // IN
const float*  array,       // IN: array of floats
int           num)         // IN: number of floats in array
: _factors(0), _order(-1)
{
    assert(varName!= 0 && unitName != 0 && array != 0 && num > 0);

    (void)strcpy(_varName, varName);
    (void)strcpy(_unitName, unitName);

    // save the polynomial
    _factors = new float[num];
    if (_factors == 0)
    {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    memcpy(_factors, array, num * sizeof(float));
    _order = num - 1;

}//Polynomial::Polynomial

Polynomial::~Polynomial()
{
    if (_factors)
        delete [] _factors;

}//Polynomial::~Polynomial

float
Polynomial::Apply(
float    x) const      // IN
{
    assert(_factors != 0);
    double result = _factors[0];
    for (int i=1; i <= _order; i++)
    {
        result += pow(x, i) * _factors[i];
    }
    // take care of the constant (X0)
    result += (_order < 1 ? x : 0);
    return((float) result);

}//Polynomial::Apply

double
Polynomial::Apply(
double    x) const      // IN
{
    assert(_factors != 0);
    double result = _factors[0];
    for (int i=1; i <= _order; i++)
    {
        result += pow(x, i) * _factors[i];
    }
    // take care of the constant (X0)
    result += (_order < 1 ? x : 0);
    return(result);

}//Polynomial::Apply

void
Polynomial::ApplyArray(
const float* inArray,       // IN: array for float DN
int    num,                 // IN: number of elements in array
float* outArray)const       // OUT: array for float EU
{
    assert(_factors != 0 && num > 0 && inArray != 0 && outArray != 0);

    for (int j=0; j < num; j++)
    {
        double result = _factors[0];
        for (int i=1; i <= _order; i++)
        {
            result += pow(inArray[j], i) * _factors[i];
        }
        // take care of the constant (X0)
        result += (_order < 1 ? inArray[j] : 0);
        outArray[j] = (float)result;
    }

}//Polynomial::ApplyArray

void
Polynomial::ApplyArray(
const double* inArray,             // IN: array for double DN
int           num,                 // IN: number of elements in array
double*       outArray)const       // OUT: array for double EU
{
    assert(_factors != 0 && num > 0 && inArray != 0 && outArray != 0);

    for (int j=0; j < num; j++)
    {
        double result = _factors[0];
        for (int i=1; i <= _order; i++)
        {
            result += pow(inArray[j], i) * _factors[i];
        }
        // take care of the constant (X0)
        result += (_order < 1 ? inArray[j] : 0);
        outArray[j] = result;
    }

}//Polynomial::ApplyArray

void
Polynomial::ApplyReplaceArray(
float*   array,          // IN: apply polynomial to array
int      num) const      // IN: number of elements in array
{
    assert(_factors != 0 && num > 0 && array != 0);
    for (int j=0; j < num; j++)
    {
        double result = _factors[0];
        for (int i=1; i <= _order; i++)
        {
            result += pow(array[j], i) * _factors[i];
        }
        // take care of the constant (X0)
        result += (_order < 1 ? array[j] : 0);
        float floatResult = (float)result;
        (void)memcpy(&array[j], &floatResult, sizeof(float));
    }

    return;

}//Polynomial::ApplyReplaceArray

void
Polynomial::ApplyReplaceArray(
double*  array,          // IN: apply polynomial to array
int      num) const      // IN: number of elements in array
{
    assert(_factors != 0 && num > 0 && array != 0);
    for (int j=0; j < num; j++)
    {
        double result = _factors[0];
        for (int i=1; i <= _order; i++)
        {
            result += pow(array[j], i) * _factors[i];
        }
        // take care of the constant (X0)
        result += (_order < 1 ? array[j] : 0);
        (void)memcpy(&array[j], &result, sizeof(double));
    }

    return;

}//Polynomial::ApplyReplaceArray
