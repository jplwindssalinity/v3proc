//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

//----------------------------------------------------------------------
// NAME
//		l2b_metrics
//
// SYNOPSIS
//		l2b_metrics [ -c config_file ] [ -l l2b_file ]
//			[ -t truth_type ] [ -f truth_file ] [ -s low:high ]
//			[ -o output_base ] [ -w within ]
//
// DESCRIPTION
//		Generates output files containing wind retrieval metrics
//		as a function of cross track distance for given ranges of
//		wind speed.
//
// OPTIONS
//		[ -c config_file ]	Use the specified config file.
//		[ -l l2b_file ]		Use this l2b file.
//		[ -t truth_type ]	This is the truth file type.
//		[ -f truth_file ]	This is the truth file.
//		[ -s low:high ]		The range of wind speeds.
//		[ -o output_base ]	The base name to use for output files.
//		[ -w within ]		The angle to use for within.
//
// OPERANDS
//		None.
//
// EXAMPLES
//		An example of a command line is:
//			% l2b_metrics -c qscat.cfg -s 5.0:6.0
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

#include <stdio.h>
#include "ConfigList.h"
#include "Misc.h"
#include "ConfigSimDefs.h"
#include "L2B.h"
#include "Constants.h"
#include "List.h"
#include "List.C"

//-----------//
// TEMPLATES //
//-----------//

template class List<StringPair>;
template class List<EarthPosition>;
template class List<WindVectorPlus>;

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING				"c:l:t:f:s:o:w:"
#define ARRAY_SIZE				1024

#define DEFAULT_LOW_SPEED		0.0
#define DEFAULT_HIGH_SPEED		30.0
#define DEFAULT_WITHIN_ANGLE	45.0

//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

int xmgr_control(FILE* ofp, const char* title, const char* subtitle,
		const char* x_label, const char* y_label);

int plot_thing(const char* extension, const char* title, const char* x_axis,
		const char* y_axis);

int rad_to_deg();

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -c config_file ]", "[ -l l2b_file ]",
	"[ -t truth_type ]", "[ -f truth_file ]", "[ -s low:high ]",
	"[ -o output_base ]", "[ -w within ]", 0 };

// not always evil...
float*			ctd_array = NULL;
float*			value_array = NULL;
int*			count_array = NULL;
int				cross_track_bins = 0;
const char*		command = NULL;
char*			l2b_file = NULL;
char*			output_base = NULL;

//--------------//
// MAIN PROGRAM //
//--------------//

int
main(
	int		argc,
	char*	argv[])
{
	//-----------//
	// variables //
	//-----------//

	char* config_file = NULL;
	ConfigList config_list;
	l2b_file = NULL;
	char* truth_type = NULL;
	char* truth_file = NULL;
	float low_speed = DEFAULT_LOW_SPEED;
	float high_speed = DEFAULT_HIGH_SPEED;
	float within_angle = DEFAULT_WITHIN_ANGLE;
	output_base = NULL;

	//------------------------//
	// parse the command line //
	//------------------------//

	command = no_path(argv[0]);

	if (argc == 1)
		usage(command, usage_array, 1);

	int c;
	while ((c = getopt(argc, argv, OPTSTRING)) != -1)
	{
		switch(c)
		{
		case 'c':
			config_file = optarg;
			if (! config_list.Read(config_file))
			{
				fprintf(stderr, "%s: error reading config file %s\n",
					command, config_file);
				exit(1);
			}
			break;
		case 'l':
			l2b_file = optarg;
			break;
		case 't':
			truth_type = optarg;
			break;
		case 'f':
			truth_file = optarg;
			break;
		case 's':
			if (sscanf(optarg, "%f:%f", &low_speed, &high_speed) != 2)
			{
				fprintf(stderr, "%s: error determine speed range %s\n",
					command, optarg);
				exit(1);
			}
			break;
		case 'o':
			output_base = optarg;
			break;
		case 'w':
			within_angle = atof(optarg);
			break;
		case '?':
			usage(command, usage_array, 1);
			break;
		}
	}

	//---------------------//
	// check for arguments //
	//---------------------//

	if (! l2b_file)
	{
		l2b_file = config_list.Get(L2B_FILE_KEYWORD);
		if (l2b_file == NULL)
		{
			fprintf(stderr, "%s: must specify L2B file\n", command);
			exit(1);
		}
	}

	if (! truth_type)
	{
		truth_type = config_list.Get(WINDFIELD_TYPE_KEYWORD);
		if (truth_type == NULL)
		{
			fprintf(stderr, "%s: must specify truth windfield type\n",
				command);
			exit(1);
		}
	}

	if (! truth_file)
	{
		truth_file = config_list.Get(WINDFIELD_FILE_KEYWORD);
		if (truth_file == NULL)
		{
			fprintf(stderr, "%s: must specify truth windfield file\n",
				command);
			exit(1);
		}
	}

	//-----------------------//
	// read in level 2B file //
	//-----------------------//

	L2B l2b;
	l2b.SetFilename(l2b_file);
	if (! l2b.OpenForReading())
	{
		fprintf(stderr, "%s: error opening L2B file %s\n", command, l2b_file);
		exit(1);
	}

	if (! l2b.ReadHeader())
	{
		fprintf(stderr, "%s: error reading L2B header from file %s\n",
			command, l2b_file);
		exit(1);
	}

	if (! l2b.ReadDataRec())
	{
		fprintf(stderr, "%s: error reading L2B swath from file %s\n",
			command, l2b_file);
		exit(1);
	}

	WindSwath* swath = &(l2b.frame.swath);

	//----------------------------//
	// read in "truth" wind field //
	//----------------------------//

	WindField truth;
	truth.ReadType(truth_file, truth_type);

	//---------------//
	// create arrays //
	//---------------//

	cross_track_bins = swath->GetCrossTrackBins();
	ctd_array = new float[cross_track_bins];
	value_array = new float[cross_track_bins];
	count_array = new int[cross_track_bins];

	char title[1024];

	//--------------------//
	// generate ctd array //
	//--------------------//

	if (! swath->CtdArray(l2b.header.crossTrackResolution, ctd_array))
	{
		fprintf(stderr, "%s: error generating CTD array\n", command);
		exit(1);
	}

	//==========//
	// SELECTED //
	//==========//

	//----------------------------------//
	// selected rms speed error vs. ctd //
	//----------------------------------//

	if (! swath->RmsSpdErrVsCti(&truth, value_array, count_array, low_speed,
		high_speed))
	{
		fprintf(stderr, "%s: error calculating selected RMS speed error\n",
			command);
		exit(1);
	}
	sprintf(title, "Selected RMS Speed Error vs. CTD (%g - %g m/s)",
		low_speed, high_speed);
	plot_thing("sel_rms_spd_err", title, "Cross Track Distance (km)",
		"RMS Speed Error (m/s)");

	//--------------------------------------//
	// selected rms direction error vs. ctd //
	//--------------------------------------//

	if (! swath->RmsDirErrVsCti(&truth, value_array, count_array, low_speed,
		high_speed))
	{
		fprintf(stderr, "%s: error calculating selected RMS direction error\n",
			command);
		exit(1);
	}
	rad_to_deg();
	sprintf(title, "Selected RMS Direction Error vs. CTD (%g - %g m/s)",
		low_speed, high_speed);
	plot_thing("sel_rms_dir_err", title, "Cross Track Distance (km)",
		"RMS Direction Error (deg)");

	//-----------------------------//
	// selected speed bias vs. ctd //
	//-----------------------------//

	if (! swath->SpdBiasVsCti(&truth, value_array, count_array, low_speed,
		high_speed))
	{
		fprintf(stderr, "%s: error calculating selected speed bias\n", command);
		exit(1);
	}
	sprintf(title, "Selected Speed Bias vs. CTD (%g - %g m/s)", low_speed,
		high_speed);
	plot_thing("sel_spd_bias", title, "Cross Track Distance (km)",
		"Speed Bias (m/s)");

	//---------------//
	// skill vs. ctd //
	//---------------//

	if (! swath->SkillVsCti(&truth, value_array, count_array, low_speed,
		high_speed))
	{
		fprintf(stderr, "%s: error calculating skill\n", command);
		exit(1);
	}
	sprintf(title, "Skill vs. CTD (%g - %g m/s)", low_speed, high_speed);
	plot_thing("skill", title, "Cross Track Distance (km)", "Skill");

	//----------------//
	// within vs. ctd //
	//----------------//

	if (! swath->WithinVsCti(&truth, value_array, count_array, low_speed,
		high_speed, within_angle * dtr))
	{
		fprintf(stderr, "%s: error calculating within\n", command);
		exit(1);
	}
	sprintf(title, "Within %.0f vs. CTD (%g - %g m/s)", within_angle,
		low_speed, high_speed);
	plot_thing("within", title, "Cross Track Distance (km)", "Within");

	//=========//
	// NEAREST //
	//=========//

	swath->SelectNearest(&truth);

	//---------------------------------//
	// nearest rms speed error vs. ctd //
	//---------------------------------//

	if (! swath->RmsSpdErrVsCti(&truth, value_array, count_array, low_speed,
		high_speed))
	{
		fprintf(stderr, "%s: error calculating nearest RMS speed error\n",
			command);
		exit(1);
	}
	sprintf(title, "Nearest RMS Speed Error vs. CTD (%g - %g m/s)",
		low_speed, high_speed);
	plot_thing("near_rms_spd_err", title, "Cross Track Distance (km)",
		"RMS Speed Error (m/s)");

	//-------------------------------------//
	// nearest rms direction error vs. ctd //
	//-------------------------------------//

	if (! swath->RmsDirErrVsCti(&truth, value_array, count_array, low_speed,
		high_speed))
	{
		fprintf(stderr, "%s: error calculating nearest RMS direction error\n",
			command);
		exit(1);
	}
	rad_to_deg();
	sprintf(title, "Nearest RMS Direction Error vs. CTD (%g - %g m/s)",
		low_speed, high_speed);
	plot_thing("near_rms_dir_err", title, "Cross Track Distance (km)",
		"RMS Direction Error (deg)");

	//----------------------------//
	// nearest speed bias vs. ctd //
	//----------------------------//

	if (! swath->SpdBiasVsCti(&truth, value_array, count_array, low_speed,
		high_speed))
	{
		fprintf(stderr, "%s: error calculating nearest speed bias\n", command);
		exit(1);
	}
	sprintf(title, "Nearest Speed Bias vs. CTD (%g - %g m/s)", low_speed,
		high_speed);
	plot_thing("near_spd_bias", title, "Cross Track Distance (km)",
		"Speed Bias (m/s)");

	//-------------//
	// free arrays //
	//-------------//

	delete[] value_array;
	delete[] ctd_array;
	delete[] count_array;

	return (0);
}

//--------------//
// xmgr_control //
//--------------//

#define QUOTE	'"'

int
xmgr_control(
	FILE*			ofp,
	const char*		title,
	const char*		subtitle,
	const char*		x_label,
	const char*		y_label)
{
	fprintf(ofp, "@ title %c%s%c\n", QUOTE, title, QUOTE);
	fprintf(ofp, "@ subtitle %c%s%c\n", QUOTE, subtitle, QUOTE);
	fprintf(ofp, "@ xaxis label %c%s%c\n", QUOTE, x_label, QUOTE);
	fprintf(ofp, "@ yaxis label %c%s%c\n", QUOTE, y_label, QUOTE);
	return(1);
}

//------------//
// plot_thing //
//------------//

int
plot_thing(
	const char*		extension,
	const char*		title,
	const char*		x_axis,
	const char*		y_axis)
{
	char filename[1024];
	sprintf(filename, "%s.%s", output_base, extension);
	FILE* ofp = fopen(filename, "w");
	if (ofp == NULL)
	{
		fprintf(stderr, "%s: error opening output file %s\n", command,
			filename);
		exit(1);
	}

	xmgr_control(ofp, title, l2b_file, x_axis, y_axis);

	for (int i = 0; i < cross_track_bins; i++)
	{
		if (count_array[i] > 0)
		{
			fprintf(ofp, "%g %g %d\n", ctd_array[i], value_array[i],
				count_array[i]);
		}
	}
	fclose(ofp);

	return(1);
}

//------------//
// rad_to_deg //
//------------//

int
rad_to_deg()
{
	for (int i = 0; i < cross_track_bins; i++)
	{
		value_array[i] *= rtd;
	}
	return(1);
}
