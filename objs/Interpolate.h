//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef INTERPOLATE_H
#define INTERPOLATE_H

static const char rcs_id_interpolate_h[] =
	"@(#) $Id$";

//======================================================================
// DESCRIPTION
//		Interpolation functions
//		Polynomial fitting
//======================================================================

// for polint, n is the order+1
int polint(double xa[], double ya[], int n, double x, double* y);

// for polcoe, n is the order
int polcoe( double x[], double y[], int	n, double* cof);

#endif
