//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		gmf
//
// SYNOPSIS
//		gmf [ -d ] [ -g gmf_file ] [ -r coef_file ] [ -w coef_file ]
//
// DESCRIPTION
//		Generates plots about a geophysical model function.
//
// OPTIONS
//		[ -d ]
//			Write terms in dB.
//		[ -g gmf_file ]
//			Gets the GMF information from a gmf table.
//		[ -r coef_file ]
//			Gets the GMF information from a coefficient file.
//		[ -w coef_file ]
//			Writes out a binary coefficient file.
//
// OPERANDS
//		None.
//
// EXAMPLES
//		An example of a command line is:
//			% gmf sass2.dat
//
// ENVIRONMENT
//		Not environment dependent.
//
// EXIT STATUS
//		The following exit values are returned:
//		0	Program executed successfully
//		>0	Program had an error
//
// NOTES
//		None.
//
// AUTHOR
//		James N. Huddleston
//		hudd@acid.jpl.nasa.gov
//----------------------------------------------------------------------

//-----------------------//
// Configuration Control //
//-----------------------//

static const char rcs_id[] =
	"@(#) $Id$";

//----------//
// INCLUDES //
//----------//

#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include "Misc.h"
#include "GMF.h"

//-----------//
// CONSTANTS //
//-----------//

#define QUOTES	'"'
#define OPTSTRING	"dg:r:w:"

#define POLS		2
#define INCS		67
#define SPDS		51

#define INC_STEP		2

#define RTD			(180.0 / M_PI)

//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

int term_vs_inc(const char* command, const char* gmf_file,
	double term[POLS][INCS][SPDS], char* name, unsigned char flag);
int term_vs_spd(const char* command, const char* gmf_file,
	double term[POLS][INCS][SPDS], char* name, unsigned char flag);

//------------------//
// OPTION VARIABLES //
//------------------//

unsigned char db_flag = 0;

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -d ]", "[ -g gmf_file ]", "[ -r coef_file ]",
	"[ -w coef_file ]", 0 };

const char* pol_map[] = { "V", "H" };

//--------------//
// MAIN PROGRAM //
//--------------//

int
main(
	int		argc,
	char*	argv[])
{
	//------------------------//
	// parse the command line //
	//------------------------//

	const char* command = no_path(argv[0]);

	if (argc == 1)
		usage(command, usage_array, 1);

	int c;
	char* read_gmf = 0;
	char* read_coef = 0;
	char* write_coef = 0;
	while ((c = getopt(argc, argv, OPTSTRING)) != -1)
	{
		switch(c)
		{
		case 'd':
			db_flag = 1;
			break;
		case 'g':
			read_gmf = optarg;
			break;
		case 'r':
			read_coef = optarg;
			break;
		case 'w':
			write_coef = optarg;
			break;
		case '?':
			usage(command, usage_array, 1);
			break;
		}
	}

	if (argc != optind)
		usage(command, usage_array, 1);

	if ((read_gmf && read_coef) ||
		(! read_gmf && ! read_coef))
	{
		fprintf(stderr, "%s: only one of [ -g, -r ] must be used\n", command);
		exit(1);
	}

	char* base;
	if (read_gmf)
		base = read_gmf;
	if (read_coef)
		base = read_coef;

	double a0[POLS][INCS][SPDS];
	double a1[POLS][INCS][SPDS];
	double a1_phase[POLS][INCS][SPDS];
	double a2[POLS][INCS][SPDS];
	double a2_phase[POLS][INCS][SPDS];

	//-----------------//
	// read in the gmf //
	//-----------------//

	if (read_gmf)
	{
		GMF gmf;
		if (! gmf.ReadOldStyle(read_gmf))
		{
			fprintf(stderr, "%s: error reading gmf file %s\n",
				command, read_gmf);
			exit(1);
		}

		//-------------------------------//
		// transform into A0, a1, and a2 //
		//-------------------------------//

		for (int pol = 0; pol < POLS; pol++)
		{
			for (int inc = 16; inc < INCS; inc += INC_STEP)
			{
				double dinc = (double)inc;
				for (int spd = 1; spd < SPDS; spd += 1)
				{
					double dspd = (double)spd;
					double A0, A1, A1_phase, A2, A2_phase;
					gmf.GetCoefs(pol, dinc, dspd, &A0, &A1, &A1_phase, &A2,
						&A2_phase);
					a0[pol][inc][spd] = A0;
					a1[pol][inc][spd] = A1 / A0;
					a1_phase[pol][inc][spd] = fmod(A1_phase * RTD + 360.0,
						360.0);
					if (a1_phase[pol][inc][spd] > 270.0)
						a1_phase[pol][inc][spd] -= 360.0;
					a2[pol][inc][spd] = A2 / A0;
					a2_phase[pol][inc][spd] = fmod(A2_phase * RTD + 360.0,
						360.0);
					if (a2_phase[pol][inc][spd] > 270.0)
						a2_phase[pol][inc][spd] -= 360.0;
				}
			}
		}
	}

	//-------------------//
	// read in the coefs //
	//-------------------//

	if (read_coef)
	{
		int ifd = open(read_coef, O_RDONLY);
		if (ifd == -1)
		{
			fprintf(stderr, "%s: error opening coefficient file %s\n",
				command, read_coef);
			exit(1);
		}
		int size = POLS * INCS * SPDS * sizeof(double);
		if (read(ifd, a0, size) != size ||
			read(ifd, a1, size) != size ||
			read(ifd, a1_phase, size) != size ||
			read(ifd, a2, size) != size ||
			read(ifd, a2_phase, size) != size)
		{
			fprintf(stderr, "%s: error reading coefficient file %s\n",
				command, read_coef);
			exit(1);
		}
		close(ifd);
	}

	//---------------------//
	// write out the coefs //
	//---------------------//

	if (write_coef)
	{
		int ofd = creat(write_coef, 0644);
		if (ofd == -1)
		{
			fprintf(stderr, "%s: error opening coefficient file %s\n",
				command, write_coef);
			exit(1);
		}
		int size = POLS * INCS * SPDS * sizeof(double);
		if (write(ofd, a0, size) != size ||
			write(ofd, a1, size) != size ||
			write(ofd, a1_phase, size) != size ||
			write(ofd, a2, size) != size ||
			write(ofd, a2_phase, size) != size)
		{
			fprintf(stderr, "%s: error writing coefficient file %s\n",
				command, write_coef);
			exit(1);
		}
		close(ofd);
	}

	term_vs_inc(command, base, a0, "a0", db_flag);
	term_vs_inc(command, base, a1, "a1", db_flag);
	term_vs_inc(command, base, a1_phase, "a1p", 0);
	term_vs_inc(command, base, a2, "a2", db_flag);
	term_vs_inc(command, base, a2_phase, "a2p", 0);

	term_vs_spd(command, base, a0, "a0", db_flag);
	term_vs_spd(command, base, a1, "a1", db_flag);
	term_vs_spd(command, base, a1_phase, "a1p", 0);
	term_vs_spd(command, base, a2, "a2", db_flag);
	term_vs_spd(command, base, a2_phase, "a2p", 0);

	return (0);
}

//-------------//
// term_vs_inc //
//-------------//

int
term_vs_inc(
	const char*	command,
	const char*	gmf_file,
	double	term[POLS][INCS][SPDS],
	char*	name,
	unsigned char	flag)
{
	char filename[1024];
	FILE* ofp;

	for (int pol = 0; pol < POLS; pol++)
	{
		sprintf(filename, "%s.%s.inc.%s", gmf_file, pol_map[pol], name);
		ofp = fopen(filename, "w");
		if (ofp == NULL)
		{
			fprintf(stderr, "%s: error opening output file %s\n",
				command, filename);
			exit(1);
		}
		fprintf(ofp, "@ title %c%s, %s pol%c\n", QUOTES, name, pol_map[pol],
			QUOTES);
		fprintf(ofp, "@ xaxis label %cIncidence Angle (deg)%c\n", QUOTES,
			QUOTES);
		if (flag)
			fprintf(ofp, "@ yaxis label %c%s (dB)%c\n", QUOTES, name,
				QUOTES);
		else
			fprintf(ofp, "@ yaxis label %c%s%c\n", QUOTES, name,
				QUOTES);

		int spds[] = { 1, 3, 5, 8, 15, 20, 30, 40, -1 };
		int spd_idx = 0;
		while (spds[spd_idx] > 0)
		{
			fprintf(ofp, "@ legend string %d %c%d m/s%c\n", spd_idx, QUOTES,
				spds[spd_idx], QUOTES);
			for (int inc = 20; inc <= 60; inc += INC_STEP)
			{
				if (flag)
					fprintf(ofp, "%d %g\n", inc,
						10.0 * log10(term[pol][inc][spds[spd_idx]]));
				else
					fprintf(ofp, "%d %g\n", inc,
						term[pol][inc][spds[spd_idx]]);
			}
			spd_idx++;
			if (spds[spd_idx] > 0)
				fprintf(ofp, "&\n");
		}
	}
	fclose(ofp);
	return(1);
}

//-------------//
// term_vs_spd //
//-------------//

int
term_vs_spd(
	const char*	command,
	const char*	gmf_file,
	double	term[POLS][INCS][SPDS],
	char*	name,
	unsigned char	flag)
{
	char filename[1024];
	FILE* ofp;

	for (int pol = 0; pol < POLS; pol++)
	{
		sprintf(filename, "%s.%s.spd.%s", gmf_file, pol_map[pol], name);
		ofp = fopen(filename, "w");
		if (ofp == NULL)
		{
			fprintf(stderr, "%s: error opening output file %s\n",
				command, filename);
			exit(1);
		}
		fprintf(ofp, "@ title %c%s, %s pol%c\n", QUOTES, name, pol_map[pol],
			QUOTES);
		fprintf(ofp, "@ xaxis label %cWind Speed (m/s)%c\n", QUOTES,
			QUOTES);
		if (flag)
			fprintf(ofp, "@ yaxis label %c%s (dB)%c\n", QUOTES, name,
				QUOTES);
		else
			fprintf(ofp, "@ yaxis label %c%s%c\n", QUOTES, name,
				QUOTES);

		int incs[] = { 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60, 64, -1 };
		int inc_idx = 0;
		while (incs[inc_idx] > 0)
		{
			fprintf(ofp, "@ legend string %d %c%d deg%c\n", inc_idx, QUOTES,
				incs[inc_idx], QUOTES);
			for (int spd = 1; spd < SPDS; spd++)
			{
				if (flag)
					fprintf(ofp, "%d %g\n", spd,
						10.0 * log10(term[pol][incs[inc_idx]][spd]));
				else
					fprintf(ofp, "%d %g\n", spd, term[pol][incs[inc_idx]][spd]);
			}
			inc_idx++;
			if (incs[inc_idx] > 0)
				fprintf(ofp, "&\n");
		}
	}
	fclose(ofp);
	return(1);
}
