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

int polint(double xa[], double ya[], int n, double x, double* y);
int polcoe( double x[], double y[], int	n, double* cof);

#endif
