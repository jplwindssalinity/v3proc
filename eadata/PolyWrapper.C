//=========================================================
// Copyright  (C)1998, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
// CM Log
// $Log$
// 
//    Rev 1.0   12 Feb 1998 16:50:18   sally
// Initial revision.
//
// $Date$
// $Revision$
// $Author$
//
//=========================================================

static const char rcs_id_PolyWrapper_C[] =
    "@(#) $Header$";


#include "PolyWrapper.h"
#include "PolyTable.h"
#include "Polynomial.h"

/*----------------------------------------------------
 * Create a polynomial table: this must be done
 * before any operations on the polynomials.
 * Returns EA polynomial error number.
 *----------------------------------------------------*/
EA_PolynomialErrorNo
EA_CreatePolyTable(
const char*      filename,     /* IN: table filename */
EA_PolyTableP*   polyTableP)   /* OUT: ptr to the new polynomial table */
{
    /*--------------------------------------------------------------*/
    /* create the polynomial table.  If the creation fails, delete 
       the table and return NULL; else return the pointer to user   */
    /*--------------------------------------------------------------*/
    EA_PolynomialErrorNo status = EA_POLY_OK;
    PolynomialTable* newPolyTableP = new PolynomialTable(filename, status);
    if (status != EA_POLY_OK)
    {
        delete newPolyTableP;
        newPolyTableP = 0;
    }
    *polyTableP = (EA_PolyTableP) newPolyTableP;
    return(status);

} /* EA_CreatePolyTable */


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
float            arrayEU[])    /* OUT: array of EU */
{
    /* make sure the table pointer is not NULL */
    if (polytableP == 0)
        return(EA_POLY_NULL_TABLE_POINTER);

    const Polynomial* polynomial = 
          ((PolynomialTable*)polytableP)->SelectPolynomial(varName, unitName);
    if (polynomial == 0)
        return(EA_POLY_NO_SUCH_POLYNOMIAL);

    polynomial->ApplyArray(arrayDN, numInArray, arrayEU);

    return(EA_POLY_OK);

} /* EA_ApplyFloatPolynomial */


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
double           arrayEU[])    /* OUT: array of EU */
{
    /* make sure the table pointer is not NULL */
    if (polytableP == 0)
        return(EA_POLY_NULL_TABLE_POINTER);

    const Polynomial* polynomial = 
          ((PolynomialTable*)polytableP)->SelectPolynomial(varName, unitName);
    if (polynomial == 0)
        return(EA_POLY_NO_SUCH_POLYNOMIAL);

    polynomial->ApplyArray(arrayDN, numInArray, arrayEU);

    return(EA_POLY_OK);

} /* EA_ApplyDoublePolynomial */


/*----------------------------------------------------
 * Destroy the polynomial table created previously.
 * Returns EA polynomial error number.
 *----------------------------------------------------*/
EA_PolynomialErrorNo
EA_DestroyPolyTable(
EA_PolyTableP    polytableP)   /* IN: polynomial table ptr */
{
    /* make sure the table pointer is not NULL */
    if (polytableP == 0)
        return(EA_POLY_NULL_TABLE_POINTER);

    delete ((PolynomialTable*)polytableP);
    return(EA_POLY_OK);

} /* EA_DestroyPolyTable */
