//=========================================================
// Copyright  (C)1998, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.2   20 Apr 1998 15:19:50   sally
// change List to EAList
// 
//    Rev 1.1   12 Feb 1998 16:49:26   sally
// add wrappers for "C" functions
// 
//    Rev 1.0   04 Feb 1998 14:16:48   daffer
// Initial checking
// Revision 1.5  1998/01/30 23:08:48  daffer
// *** empty log message ***
//
// Revision 1.4  1998/01/30 22:57:57  daffer
// *** empty log message ***
//
// Revision 1.3  1998/01/30 22:28:38  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================


#ifndef POLYTABLE_H
#define POLYTABLE_H

#include "EAList.h"
#include "Polynomial.h"
#include "PolyErrNo.h"

//--------------------------------------------------------
// This object reads a polynomial table from a file
// and create a list of Polynomials.
//
// File format:
//   element_var_name  unit_name a bX cX2 dX3 ...
//
//--------------------------------------------------------

static const char rcs_id_PolyTable_h[] =
    "@(#) $Header$";

class PolynomialTable : public EAList<Polynomial>
{
public:
    PolynomialTable(const char*            filename,   // IN
                    EA_PolynomialErrorNo&  status);    // OUT: return status

    virtual ~PolynomialTable();

    const Polynomial*   SelectPolynomial(
                           const char*   varName,          // IN
                           const char*   unitName);        // IN

}; //PolynomialTable

#endif //POLYTABLE_H
