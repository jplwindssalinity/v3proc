//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_l2aframe_c[] =
	"@(#) $Id$";

#include <memory.h>
#include <math.h>
#include "L2AFrame.h"
#include "Constants.h"
#include "Misc.h"

//===========//
// L2AHeader //
//===========//

L2AHeader::L2AHeader()
:	crossTrackResolution(0.0), alongTrackResolution(0.0), crossTrackBins(0),
	alongTrackBins(0), zeroIndex(0), startTime(0.0)
{
	return;
}

L2AHeader::~L2AHeader()
{
	return;
}

//-----------------//
// L2AHeader::Read //
//-----------------//

int
L2AHeader::Read(
	FILE*	fp)
{
	if (fread(&crossTrackResolution, sizeof(float), 1, fp) != 1 ||
		fread(&alongTrackResolution, sizeof(float), 1, fp) != 1 ||
		fread(&crossTrackBins, sizeof(int), 1, fp) != 1 ||
		fread(&alongTrackBins, sizeof(int), 1, fp) != 1 ||
		fread(&zeroIndex, sizeof(int), 1, fp) != 1 ||
		fread(&startTime, sizeof(double), 1, fp) != 1)
	{
		return(0);
	}
	return(1);
}

//------------------//
// L2AHeader::Write //
//------------------//

int
L2AHeader::Write(
	FILE*	fp)
{
	if (fwrite(&crossTrackResolution, sizeof(float), 1, fp) != 1 ||
		fwrite(&alongTrackResolution, sizeof(float), 1, fp) != 1 ||
		fwrite(&crossTrackBins, sizeof(int), 1, fp) != 1 ||
		fwrite(&alongTrackBins, sizeof(int), 1, fp) != 1 ||
		fwrite(&zeroIndex, sizeof(int), 1, fp) != 1 ||
		fwrite(&startTime, sizeof(double), 1, fp) != 1)
	{
		return(0);
	}
	return(1);
}

//------------------------//
// L2AHeader::WriteAscii  //
//------------------------//

int
L2AHeader::WriteAscii(
	FILE*	fp)
{
        fprintf(fp,"#################################################\n");
        fprintf(fp,"######                                     ######\n");
        fprintf(fp,"######             L2A  Header             ######\n");
        fprintf(fp,"######                                     ######\n");
        fprintf(fp,"#################################################\n");
        fprintf(fp,"\n");
        fprintf(fp,"CrossTrackRes: %g AlongTrackRes: %g\n",
		crossTrackResolution, alongTrackResolution);
        fprintf(fp,"CrossTrackBins: %d AlongTrackBins: %d\n",
                crossTrackBins,alongTrackBins);
        fprintf(fp,"StartTime: %g ZeroIndex: %d\n",startTime,zeroIndex);
	return(1);
}

//==========//
// L2AFrame //
//==========//

L2AFrame::L2AFrame()
{
	return;
}

L2AFrame::~L2AFrame()
{
	return;
}

//----------------//
// L2AFrame::Read //
//----------------//

int
L2AFrame::Read(
	FILE*	fp)
{
	if (fread((void *)&rev, sizeof(unsigned int), 1, fp) != 1 ||
		fread((void *)&ati, sizeof(int), 1, fp) != 1 ||
		fread((void *)&cti, sizeof(unsigned char), 1, fp) != 1 ||
		measList.Read(fp) != 1)
	{
		return(0);
	}

	return(1);
}

//-----------------//
// L2AFrame::Write //
//-----------------//

int
L2AFrame::Write(
	FILE*	fp)
{
	if (fwrite((void *)&rev, sizeof(unsigned int), 1, fp) != 1 ||
		fwrite((void *)&ati, sizeof(int), 1, fp) != 1 ||
		fwrite((void *)&cti, sizeof(unsigned char), 1, fp) != 1 ||
		measList.Write(fp) != 1)
	{
		return(0);
	}

	return(1);
}

//----------------------//
// L2AFrame::WriteAscii //
//----------------------//

int
L2AFrame::WriteAscii(
	FILE*	fp)
{
        fprintf(fp,"\n#################################################\n");
        fprintf(fp,"######                                     ######\n");
        fprintf(fp,"######         L2A  Data Record            ######\n");
        fprintf(fp,"######                                     ######\n");
        fprintf(fp,"#################################################\n");
        fprintf(fp,"\n");
        fprintf(fp,"Rev: %d AlongTrackIndex: %d CrossTrackIndex: %d\n\n",
		rev,ati,(int)cti);
	if ( measList.WriteAscii(fp) != 1)
	{
		return(0);
	}

	return(1);
}

//------------------//
// L2AFrame::ReadGS //
//------------------//

#define MAX_COLS	76
#define MAX_BINS	1000

int
L2AFrame::ReadGS(
	FILE*	fp)
{
	static int at_valid_row = 0;
	static int min_row_number = 0;

	static int sigma0_in_row = 0;
	static int current_col = 0;
	static int row_number = 0;

	int col_number;
	int dummy;

	unsigned int sigma0_in_cell;

	int done = 0;
	do
	{
		//-------------------------------//
		// get to a row with sigma0 data //
		//-------------------------------//

		while (! at_valid_row)
		{
			if (fread((void *)&dummy, sizeof(int), 1, fp) != 1 ||
				fread((void *)&row_number, sizeof(int), 1, fp) != 1 ||
				fread((void *)&sigma0_in_row, sizeof(int), 1, fp) != 1 ||
				fread((void *)&dummy, sizeof(int), 1, fp) != 1)
			{
				return(0);
			}
			current_col = 0;
			if (sigma0_in_row)
				at_valid_row = 1;
		}

		//---------------------//
		// get to a valid cell //
		//---------------------//

		while (current_col < MAX_COLS)
		{
			if (fread((void *)&dummy, sizeof(int), 1, fp) != 1 ||
				fread((void *)&col_number, sizeof(int), 1, fp) != 1 ||
				fread((void *)&sigma0_in_cell, sizeof(unsigned int), 1,
					fp) != 1 ||
				fread((void *)&dummy, sizeof(int), 1, fp) != 1)
			{
				return(0);
			}
			current_col++;

			if (sigma0_in_cell)
			{
				done = 1;
				break;
			}
		}
		if (current_col == MAX_COLS)
			at_valid_row = 0;
	} while (! done);

	if (min_row_number == 0)
		min_row_number = row_number;

	ati = row_number - min_row_number;
	cti = col_number;

	//---------------//
	// read the cell //
	//---------------//

	float lat[MAX_BINS];
	float lon[MAX_BINS];
	float sigma0[MAX_BINS];
	float alpha[MAX_BINS];
	float beta[MAX_BINS];
	float gamma[MAX_BINS];
	float inc[MAX_BINS];
	float azi[MAX_BINS];
	float atten[MAX_BINS];
	int land_flag[MAX_BINS];
	int ice_flag[MAX_BINS];
	int co_flag[MAX_BINS];
	int amsr[MAX_BINS];
	int qual1[MAX_BINS];
	int qual2[MAX_BINS];
	int mode[MAX_BINS];
	int beam[MAX_BINS];
	int col[MAX_BINS];
	int num[MAX_BINS];

	sigma0_in_cell += 2;		// hack for FORTRAN delimeters

	if (fread((void *)&lat, sizeof(float), sigma0_in_cell, fp) !=
			sigma0_in_cell ||
		fread((void *)&lon, sizeof(float), sigma0_in_cell, fp) !=
			sigma0_in_cell ||
		fread((void *)&sigma0, sizeof(float), sigma0_in_cell, fp) !=
			sigma0_in_cell ||
		fread((void *)&alpha, sizeof(float), sigma0_in_cell, fp) !=
			sigma0_in_cell ||
		fread((void *)&beta, sizeof(float), sigma0_in_cell, fp) !=
			sigma0_in_cell ||
		fread((void *)&gamma, sizeof(float), sigma0_in_cell, fp) !=
			sigma0_in_cell ||
		fread((void *)&inc, sizeof(float), sigma0_in_cell, fp) !=
			sigma0_in_cell ||
		fread((void *)&azi, sizeof(float), sigma0_in_cell, fp) !=
			sigma0_in_cell ||
		fread((void *)&atten, sizeof(float), sigma0_in_cell, fp) !=
			sigma0_in_cell ||
		fread((void *)&land_flag, sizeof(int), sigma0_in_cell, fp) !=
			sigma0_in_cell ||
		fread((void *)&ice_flag, sizeof(int), sigma0_in_cell, fp) !=
			sigma0_in_cell ||
		fread((void *)&co_flag, sizeof(int), sigma0_in_cell, fp) !=
			sigma0_in_cell ||
		fread((void *)&amsr, sizeof(int), sigma0_in_cell, fp) !=
			sigma0_in_cell ||
		fread((void *)&qual1, sizeof(int), sigma0_in_cell, fp) !=
			sigma0_in_cell ||
		fread((void *)&qual2, sizeof(int), sigma0_in_cell, fp) !=
			sigma0_in_cell ||
		fread((void *)&mode, sizeof(int), sigma0_in_cell, fp) !=
			sigma0_in_cell ||
		fread((void *)&beam, sizeof(int), sigma0_in_cell, fp) !=
			sigma0_in_cell ||
		fread((void *)&col, sizeof(int), sigma0_in_cell, fp) !=
			sigma0_in_cell ||
		fread((void *)&num, sizeof(int), sigma0_in_cell, fp) !=
			sigma0_in_cell)
	{
		return(0);
	}

	//------------------//
	// transfer to Meas //
	//------------------//

	measList.FreeContents();

	for (unsigned int i = 1; i < sigma0_in_cell - 1; i++)
	{
		Meas* m = new Meas();
		m->value = pow(10.0, sigma0[i] * 0.1);
		if (qual1[i])
			m->value *= -1.0;
		double gd_lon = lon[i] * dtr;
		double gd_lat = lat[i] * dtr;
		double gd_alt = 0.0;
		m->centroid.SetAltLonGDLat(gd_alt, gd_lon, gd_lat);
		m->pol = (PolE)(2 - beam[i]);
		m->eastAzimuth = CWNTOCCWE(azi[i] * dtr) + pi;
		m->incidenceAngle = inc[i] * dtr;
		m->beamIdx = beam[i] - 1;
		m->A = alpha[i];
		m->B = beta[i];
		m->C = gamma[i];

		if (! measList.Append(m))
			return(0);
	}

	return(1);
}
