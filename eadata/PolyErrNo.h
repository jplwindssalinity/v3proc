/*=========================================================
   Copyright  (C)1998, California Institute of Technology. 
   U.S. Government sponsorship under 
   NASA Contract NAS7-1260 is acknowledged
  
   CM Log
   $Log$
// 
//    Rev 1.0   12 Feb 1998 16:50:20   sally
// Initial revision.
  
   $Date$
   $Revision$
   $Author$
  
  ========================================================= */


#ifndef POLYERRNO_H
#define POLYERRNO_H

static const char rcs_id_PolyErrNo_h[] =
    "@(#) $Header$";

typedef enum
{
    EA_POLY_OK,
    EA_POLY_FILE_OPEN_FAILED,
    EA_POLY_FILE_NOT_ENOUGH_ARGS,
    EA_POLY_OUT_OF_MEMORY,
    EA_POLY_ERR_READ_COEFFICIENTS,
    EA_POLY_NULL_TABLE_POINTER,
    EA_POLY_NO_SUCH_POLYNOMIAL

} EA_PolynomialErrorNo;

#endif //POLYERRNO_H
