/*=========================================================
   Copyright  (C)1998, California Institute of Technology. 
   U.S. Government sponsorship under 
   NASA Contract NAS7-1260 is acknowledged
  
   CM Log
   $Log$
// 
//    Rev 1.0   12 Feb 1998 16:50:14   sally
// Initial revision.
  
   $Date$
   $Revision$
   $Author$
  
  =========================================================*/


#ifndef POLYWRAPPER_H
#define POLYWRAPPER_H

static const char rcs_id_PolyWrapper_h[] =
    "@(#) $Header$";

#include "PolyErrNo.h"

typedef void *       EA_PolyTableP;

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------
 * Create a polynomial table: this must be done
 * before any operations on the polynomials.
 * Returns EA polynomial error number.
 *----------------------------------------------------*/
EA_PolynomialErrorNo
EA_CreatePolyTable(
const char*      filename,     /* IN: table filename */
EA_PolyTableP*   polyTableP);  /* OUT: ptr to the new polynomial table */


/*----------------------------------------------------
 * Apply polynomial to an array of float DN.
 * User must provide space for the array of float EU.
 * Returns EA polynomial error number.
 *----------------------------------------------------*/
EA_PolynomialErrorNo
EA_ApplyFloatPolynomial(
EA_PolyTableP    polytableP,   /* IN: polynomial table ptr */
const char*      varName,      /* IN: variable name */
const char*      unitName,     /* IN: unit name */
const float      arrayDN[],    /* IN: array of DN */
int              numInArray,   /* IN: number in arrayDN and arrayEU */
float            arrayEU[]);   /* OUT: array of EU: user provides space */


/*----------------------------------------------------
 * Apply polynomial to an array of double DN.
 * User must provide space for the array of double EU.
 * Returns EA polynomial error number.
 *----------------------------------------------------*/
EA_PolynomialErrorNo
EA_ApplyDoublePolynomial(
EA_PolyTableP    polytableP,   /* IN: polynomial table ptr */
const char*      varName,      /* IN: variable name */
const char*      unitName,     /* IN: unit name */
const double     arrayDN[],    /* IN: array of DN */
int              numInArray,   /* IN: number in arrayDN and arrayEU */
double           arrayEU[]);   /* OUT: array of EU: user provides space */


/*----------------------------------------------------
 * Destroy the polynomial table created previously.
 * Returns EA polynomial error number.
 *----------------------------------------------------*/
EA_PolynomialErrorNo
EA_DestroyPolyTable(
EA_PolyTableP    polytableP);  /* IN: polynomial table ptr */

#ifdef __cplusplus
}
#endif

#endif /* POLYWRAPPER_H */
