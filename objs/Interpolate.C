//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_interpolate_c[] =
	"@(#) $Id$";

#include <math.h>
#include <malloc.h>

//--------//
// polint //
//--------//

int
polint(
	double		xa[],
	double		ya[],
	int			n,
	double		x,
	double*		y)
{
	//---------------------------//
	// allocate temporary arrays //
	//---------------------------//

	double* c = (double *)malloc(n * sizeof(double));
	if (c == NULL)
		return(0);

	double* d = (double *)malloc(n * sizeof(double));
	if (d == NULL)
		return(0);

	//-------------------------------------------//
	// find the index of the closest table entry //
	//-------------------------------------------//

	int ns = 0;
	double dif = fabs(x - xa[0]);
	for (int i = 0; i < n; i++)
	{
		double dift = fabs(x - xa[i]);
		if (dift < dif)
		{
			ns = i;
			dif = dift;
		}

		// initialize
		c[i] = ya[i];
		d[i] = ya[i];
	}

	double value = ya[ns--];
	for (int m = 1; m < n; m++)
	{
		for (int i = 0; i < n - m; i++)
		{
			double ho = xa[i] - x;
			double hp = xa[i+m] - x;
			double w = c[i+1] - d[i];
			double den = ho - hp;
			if (den == 0.0)
				return(0);		// error
			den = w / den;
			d[i] = hp * den;
			c[i] = ho * den;
		}

		double adj;
		if (2 * ns < (n - m - 1))
			adj = c[ns+1];
		else
			adj = d[ns--];
		value += adj;
	}

	free(c);
	free(d);

	*y = value;
	return(1);
}
