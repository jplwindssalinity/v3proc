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
//			[ -o output_base ] [ -w within ] [-a] [-i subtitle str]
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
//		[ -s low_spd:high_spd ]		The range of wind speeds.
//      [ -r low_lat:high_lat ]  The range of latitudes.
//		[ -o output_base ]	The base name to use for output files.
//		[ -w within ]		The angle to use for within.
//		[ -a ]				Autoscale plots
//		[ -i subtitle string]
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
#include "Array.h"

//-----------//
// TEMPLATES //
//-----------//

class AngleInterval;
template class List<StringPair>;
template class List<EarthPosition>;
template class List<WindVectorPlus>;
template class List<AngleInterval>;

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING				"c:l:t:f:r:s:o:w:a:i:"
#define ARRAY_SIZE				1024

#define DEFAULT_LOW_LAT         -90.0
#define DEFAULT_HIGH_LAT        90.0
#define DEFAULT_LOW_SPEED		0.0
#define DEFAULT_HIGH_SPEED		30.0
#define DEFAULT_WITHIN_ANGLE	45.0
#define DEFAULT_OUTPUT_BASE		"metrics"
#define DEFAULT_XMIN			-1000.0
#define DEFAULT_XMAX			1000.0
#define DEFAULT_YMIN			0.0
#define DEFAULT_YMAX			1.0
#define RMS_SPD_MIN				0.0
#define RMS_SPD_MAX				5.0
#define RMS_DIR_MIN				0.0
#define RMS_DIR_MAX				50.0
#define RMS_DIR_MAX2			120.0
#define BIAS_SPD_MIN			-3.0
#define BIAS_SPD_MAX			3.0
#define BIAS_DIR_MIN			-10.0
#define BIAS_DIR_MAX			10.0
#define BIAS_DIR_MIN2			-80.0
#define BIAS_DIR_MAX2			80.0
#define SKILL_MIN				0.3
#define SKILL_MAX				1.0

#define DIRECTION_BINS			72		// 5 degree bins

//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

enum PlotFlagE    { NORMAL, COUNT };
enum RangeFlagE   { USE_LIMITS, AUTOSCALE };

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

int xmgr_control(FILE* ofp, const char* title, const char* subtitle,
        const char* x_label, const char* y_label);

int plot_thing(const char* extension, const char* title, const char* x_axis,
		const char* y_axis, float* data = NULL, float* secondary = NULL);

int rad_to_deg(float* data);

int plot_density(const char* extension, const char* title,
        const char* x_axis, const char* y_axis);


//------------------//
// OPTION VARIABLES //
//------------------//

int lat_range_opt = 0;
int autoscale_opt = 0;

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -c config_file ]", "[ -l l2b_file ]",
	"[ -t truth_type ]", "[ -f truth_file ]", "[ -s low_spd:high_spd ]",
    "[ -r low_lat:high_lat ]", "[ -o output_base ]", "[ -w within ]",
    "[ -a ]", "[ -i subtitle ]", 0 };

// not always evil...
float*         ctd_array = NULL;
float*         value_array = NULL;
float*         value_2_array = NULL;
float*         err_array = NULL;
float*         std_dev_array = NULL;
int*           count_array = NULL;
int            cross_track_bins = 0;

float*         dir_array = NULL;
unsigned int*  uint_array = NULL;
unsigned int*  uint_2_array = NULL;

const char*    command = NULL;
char*          l2b_file = NULL;
char*          output_base = NULL;
char*          subtitle_str = NULL;

PlotFlagE      plot_flag = NORMAL;
RangeFlagE     range_flag = USE_LIMITS;
float          xy_limits[4] = {DEFAULT_XMIN, DEFAULT_XMAX,
                   DEFAULT_YMIN, DEFAULT_YMAX};

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
    float low_lat = DEFAULT_LOW_LAT * dtr;
    float high_lat = DEFAULT_HIGH_LAT * dtr;
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
				fprintf(stderr, "%s: error determining speed range %s\n",
					command, optarg);
				exit(1);
			}
			break;
        case 'r':
            if (sscanf(optarg, "%f:%f", &low_lat, &high_lat) != 2)
            {
                fprintf(stderr, "%s: error determining latitude range %s\n",
                    command, optarg);
                exit(1);
            }
            low_lat *= dtr;
            high_lat *= dtr;
            lat_range_opt = 1;
		case 'o':
			output_base = optarg;
			break;
		case 'w':
			within_angle = atof(optarg);
			break;
		case 'a':
			autoscale_opt = 1;
			break;
		case 'i':
			subtitle_str = optarg;
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

	if (! output_base)
		output_base = DEFAULT_OUTPUT_BASE;

	//-----------------------//
	// read in level 2B file //
	//-----------------------//

	L2B l2b;
	l2b.SetInputFilename(l2b_file);
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
	if (! truth.ReadType(truth_file, truth_type))
    {
        fprintf(stderr, "%s: error reading true wind field from file %s\n",
            command, truth_file);
        exit(1);
    }

    //---------------------//
    // clear bad latitudes //
    //---------------------//

    if (lat_range_opt)
    {
        int lat_erase = swath->DeleteLatitudesOutside(low_lat, high_lat);
        printf("Removing %d vectors out of latitude range\n", lat_erase);
    }

	//---------------//
	// create arrays //
	//---------------//

	cross_track_bins = swath->GetCrossTrackBins();
	ctd_array = new float[cross_track_bins];
	value_array = new float[cross_track_bins];
	value_2_array = new float[cross_track_bins];
	std_dev_array = new float[cross_track_bins];
	err_array = new float[cross_track_bins];
	count_array = new int[cross_track_bins];

	dir_array = new float[DIRECTION_BINS];
    uint_array = new unsigned int[DIRECTION_BINS];
    uint_2_array = new unsigned int[DIRECTION_BINS];

	char title[1024];

	//--------------------//
	// generate ctd array //
	//--------------------//

	if (! swath->CtdArray(l2b.header.crossTrackResolution, ctd_array))
	{
		fprintf(stderr, "%s: error generating CTD array\n", command);
		exit(1);
	}

	//--------------------//
	// generate dir array //
	//--------------------//

	if (! swath->DirArray(DIRECTION_BINS, dir_array))
	{
		fprintf(stderr, "%s: error generating dir array\n", command);
		exit(1);
	}

	//==========//
	// SELECTED //
	//==========//

	//----------------------------------//
	// selected rms speed error vs. ctd //
	//----------------------------------//

	if (! swath->RmsSpdErrVsCti(&truth, value_array, std_dev_array, err_array,
		value_2_array, count_array, low_speed, high_speed))
	{
		fprintf(stderr, "%s: error calculating selected RMS speed error\n",
			command);
		exit(1);
	}
	xy_limits[2] = RMS_SPD_MIN;
	xy_limits[3] = RMS_SPD_MAX;
	sprintf(title, "Selected RMS Speed Error vs. CTD (%g - %g m/s)",
		low_speed, high_speed);
	plot_thing("sel_rms_spd_err", title, "Cross Track Distance (km)",
		"RMS Speed Error (m/s)", value_array, std_dev_array);

	//-----------------------------//
	// selected speed bias vs. ctd //
	//-----------------------------//

	xy_limits[2] = BIAS_SPD_MIN;
	xy_limits[3] = BIAS_SPD_MAX;
	sprintf(title, "Selected Speed Bias vs. CTD (%g - %g m/s)", low_speed,
		high_speed);
	plot_thing("sel_spd_bias", title, "Cross Track Distance (km)",
		"Speed Bias (m/s)", value_2_array);

	//-------------------//
	// WVC count vs. ctd //
	//-------------------//

    range_flag = AUTOSCALE;
    plot_flag = COUNT;
	sprintf(title, "Number of WVC vs. CTD (%g - %g m/s)", low_speed,
        high_speed);
	plot_thing("wvc", title, "Cross Track Distance (km)", "Number of WVC");
    range_flag = USE_LIMITS;
    plot_flag = NORMAL;

	//--------------------------------------//
	// selected rms direction error vs. ctd //
	//--------------------------------------//

	if (! swath->RmsDirErrVsCti(&truth, value_array, std_dev_array, err_array,
		value_2_array, count_array, low_speed, high_speed))
	{
		fprintf(stderr, "%s: error calculating selected RMS direction error\n",
			command);
		exit(1);
	}
	rad_to_deg(value_array);
	rad_to_deg(std_dev_array);
	xy_limits[2] = RMS_DIR_MIN;
	xy_limits[3] = RMS_DIR_MAX2;
	sprintf(title, "Selected RMS Direction Error vs. CTD (%g - %g m/s)",
		low_speed, high_speed);
	plot_thing("sel_rms_dir_err", title, "Cross Track Distance (km)",
		"RMS Direction Error (deg)", value_array, std_dev_array);

	//---------------------------------//
	// selected direction bias vs. ctd //
	//---------------------------------//

	rad_to_deg(value_2_array);
	xy_limits[2] = BIAS_DIR_MIN2;
	xy_limits[3] = BIAS_DIR_MAX2;
	sprintf(title, "Selected Direction Bias vs. CTD (%g - %g m/s)", low_speed,
		high_speed);
	plot_thing("sel_dir_bias", title, "Cross Track Distance (km)",
		"Direction Bias (deg)", value_2_array);

	//---------------//
	// skill vs. ctd //
	//---------------//

	if (! swath->SkillVsCti(&truth, value_array, count_array, low_speed,
		high_speed))
	{
		fprintf(stderr, "%s: error calculating skill\n", command);
		exit(1);
	}
	xy_limits[2] = SKILL_MIN;
	xy_limits[3] = SKILL_MAX;
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
	xy_limits[2] = SKILL_MIN;
	xy_limits[3] = SKILL_MAX;
	sprintf(title, "Within %.0f vs. CTD (%g - %g m/s)", within_angle,
		low_speed, high_speed);
	plot_thing("within", title, "Cross Track Distance (km)", "Within");

	//----------------------------------------//
	// Vector Correlation Coefficient vs. ctd //
	//----------------------------------------//

    if (! swath->VectorCorrelationVsCti(&truth, value_array, count_array,
        low_speed, high_speed))
	{
		fprintf(stderr, "%s: error calculating vector correlation\n", command);
		exit(1);
	}
	xy_limits[2] = 0.0;
	xy_limits[3] = 2.0;
	sprintf(title, "Vector Correlation  vs. CTD (%g - %g m/s)", low_speed,
        high_speed);
	plot_thing("vector_correlation", title, "Cross Track Distance (km)",
        "Vector Correlation Coefficient");

	//--------------------//
	// avg nambig vs. ctd //
	//--------------------//

	if (! swath->AvgNambigVsCti(&truth, value_array, low_speed, high_speed))
	{
		fprintf(stderr, "%s: error calculating average number of ambigs\n",
			command);
		exit(1);
	}
	xy_limits[2] = 0;
	xy_limits[3] = 5;
	sprintf(title, "Average Number of Ambiguities vs. CTD (%g - %g m/s)",
        low_speed, high_speed);
	plot_thing("avg_nambig", title, "Cross Track Distance (km)",
		"Number of Ambiguities");

	//=========//
	// NEAREST //
	//=========//

	swath->SelectNearest(&truth);

    //------------------------------------//
    // nearest and true direction density //
    //------------------------------------//

	if (! swath->DirectionDensity(&truth, uint_array, uint_2_array,
		low_speed, high_speed, DIRECTION_BINS))
	{
		fprintf(stderr, "%s: error calculating nearest direction densities\n",
			command);
		exit(1);
	}
    sprintf(title, "Direction Density (%g - %g m/s)", low_speed, high_speed);
    plot_density("dir_den", title, "Relative Wind Direction (deg)",
        "Density");

	//---------------------------------//
	// nearest rms speed error vs. ctd //
	//---------------------------------//

	if (! swath->RmsSpdErrVsCti(&truth, value_array, std_dev_array, err_array,
		value_2_array, count_array, low_speed, high_speed))
	{
		fprintf(stderr, "%s: error calculating nearest RMS speed error\n",
			command);
		exit(1);
	}
	xy_limits[2] = RMS_SPD_MIN;
	xy_limits[3] = RMS_SPD_MAX;
	sprintf(title, "Nearest RMS Speed Error vs. CTD (%g - %g m/s)",
		low_speed, high_speed);
	plot_thing("near_rms_spd_err", title, "Cross Track Distance (km)",
		"RMS Speed Error (m/s)", value_array, std_dev_array);

	//----------------------------//
	// nearest speed bias vs. ctd //
	//----------------------------//

	xy_limits[2] = BIAS_SPD_MIN;
	xy_limits[3] = BIAS_SPD_MAX;
	sprintf(title, "Nearest Speed Bias vs. CTD (%g - %g m/s)", low_speed,
		high_speed);
	plot_thing("near_spd_bias", title, "Cross Track Distance (km)",
		"Speed Bias (m/s)", value_2_array);

	//-------------------------------------//
	// nearest rms direction error vs. ctd //
	//-------------------------------------//

	if (! swath->RmsDirErrVsCti(&truth, value_array, std_dev_array, err_array,
		value_2_array, count_array, low_speed, high_speed))
	{
		fprintf(stderr, "%s: error calculating nearest RMS direction error\n",
			command);
		exit(1);
	}
	rad_to_deg(value_array);
	rad_to_deg(std_dev_array);
	xy_limits[2] = RMS_DIR_MIN;
	xy_limits[3] = RMS_DIR_MAX;
	sprintf(title, "Nearest RMS Direction Error vs. CTD (%g - %g m/s)",
		low_speed, high_speed);
	plot_thing("near_rms_dir_err", title, "Cross Track Distance (km)",
		"RMS Direction Error (deg)", value_array, std_dev_array);

	//--------------------------------//
	// nearest direction bias vs. ctd //
	//--------------------------------//

	rad_to_deg(value_2_array);
	xy_limits[2] = BIAS_DIR_MIN;
	xy_limits[3] = BIAS_DIR_MAX;
	sprintf(title, "Nearest Direction Bias vs. CTD (%g - %g m/s)", low_speed,
		high_speed);
	plot_thing("near_dir_bias", title, "Cross Track Distance (km)",
		"Direction Bias (deg)", value_2_array);

	//-------------//
	// free arrays //
	//-------------//

	delete[] value_array;
	delete[] value_2_array;
	delete[] std_dev_array;
	delete[] err_array;
	delete[] ctd_array;
	delete[] count_array;
    delete[] uint_array;
    delete[] uint_2_array;

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
    if (range_flag == USE_LIMITS && autoscale_opt == 0)
	{
		fprintf(ofp, "@ world xmin %f\n", xy_limits[0]);
		fprintf(ofp, "@ world xmax %f\n", xy_limits[1]);
		fprintf(ofp, "@ world ymin %f\n", xy_limits[2]);
		fprintf(ofp, "@ world ymax %f\n", xy_limits[3]);
		fprintf(ofp, "@ xaxis tick major 400\n");
		fprintf(ofp, "@ xaxis tick minor 200\n");
		fprintf(ofp, "@ yaxis tick major %f\n",
            (xy_limits[3] - xy_limits[2]) / 10);
		fprintf(ofp, "@ yaxis tick minor %f\n",
            (xy_limits[3] - xy_limits[2]) / 20);
	}
	fprintf(ofp, "@ xaxis tick major grid on\n");
	fprintf(ofp, "@ xaxis tick minor grid on\n");
	fprintf(ofp, "@ yaxis tick major grid on\n");
	fprintf(ofp, "@ yaxis tick minor grid on\n");
	fprintf(ofp, "@ xaxis tick major linestyle 2\n");
	fprintf(ofp, "@ xaxis tick minor linestyle 2\n");
	fprintf(ofp, "@ yaxis tick major linestyle 2\n");
	fprintf(ofp, "@ yaxis tick minor linestyle 2\n");
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
	const char*		y_axis,
	float*			data,
	float*			secondary)
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

	char sub_title[1024];
	if (subtitle_str)
	{
		sprintf(sub_title,"%s %s", l2b_file, subtitle_str);
	}
	else
	{
		sprintf(sub_title,"%s", l2b_file);
	}

	xmgr_control(ofp, title, sub_title, x_axis, y_axis);

	//------//
	// plot //
	//------//

	float* plot_data;
	if (data)
		plot_data = data;
	else
		plot_data = value_array;

	for (int i = 0; i < cross_track_bins; i++)
	{
		if (count_array[i] > 1)
		{
			if (secondary)
			{
				fprintf(ofp, "%g %g %g\n", ctd_array[i], plot_data[i],
					secondary[i]);
			}
            else if (plot_flag == COUNT)
            {
				fprintf(ofp, "%g %d\n", ctd_array[i], count_array[i]);
            }
			else
			{
				fprintf(ofp, "%g %g %d\n", ctd_array[i], plot_data[i],
					count_array[i]);
			}
		}
	}
	fclose(ofp);

	return(1);
}

//--------------//
// plot_density //
//--------------//

int
plot_density(
    const char*  extension,
    const char*  title,
    const char*  x_axis,
    const char*  y_axis)
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

	char sub_title[1024];
	if (subtitle_str)
	{
		sprintf(sub_title, "%s %s", l2b_file, subtitle_str);
	}
	else
	{
		sprintf(sub_title, "%s", l2b_file);
	}

	xmgr_control(ofp, title, sub_title, x_axis, y_axis);

    //-------//
    // count //
    //-------//

    unsigned int count = 0;
    unsigned int count_2 = 0;
    for (int i = 0; i < DIRECTION_BINS; i++)
    {
        count += uint_array[i];
        count_2 += uint_2_array[i];
    }

    //-----------//
    // normalize //
    //-----------//

    double width = dir_array[1] - dir_array[0];
    double scale = 1.0 / (width * (double)count);
    double scale_2 = 1.0 / (width * (double)count_2);

	//------//
	// plot //
	//------//

	for (int i = 0; i < DIRECTION_BINS; i++)
	{
        fprintf(ofp, "%g %g %g\n", dir_array[i] * rtd,
            scale * (double)uint_array[i],
            scale_2 * (double)uint_2_array[i]);
	}
	fclose(ofp);

	return(1);
}

//------------//
// rad_to_deg //
//------------//

int
rad_to_deg(
	float*		data)
{
	for (int i = 0; i < cross_track_bins; i++)
	{
		data[i] *= rtd;
	}
	return(1);
}

