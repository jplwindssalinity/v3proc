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

//--------//
// polcoe //
//--------//

int
polcoe(
	double		x[],
	double		y[],
	int			n,
	double*		cof)
{
	//---------------------------//
	// allocate temporary arrays //
	//---------------------------//

	double* s = (double *)malloc((n+1) * sizeof(double));
	if (s == NULL)
		return(0);

	int i,j,k;
	double phi,ff,b;

	for (i=0; i <= n; i++)
	{
		s[i] = 0.0;
		cof[i] = 0.0;
	}

	s[n] = -x[0];
	for (i=1; i <= n; i++)
	{
		for (j=n-i; j <= n-1; j++)
		{
			s[j] -= x[i]*s[j+1];
		}
		s[n] -= x[i];
	}

	for (j=0; j <= n; j++)
	{
		phi = n+1;
		for (k=n; k >= 1; k--)
		{
			phi = k*s[k] + x[j]*phi;
		}
		ff = y[j]/phi;
		b = 1.0;
		for (k=n; k >= 0; k--)
		{
			cof[k] += b*ff;
			b = s[k] + x[j]*b;
		}
	}

	free(s);
	return(1);

}
