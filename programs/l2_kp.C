//==========================================================//
// Copyright (C) 1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		l2_kp
//
// SYNOPSIS
//		l2_kp [ -c config_file ] [ -a l2a_file ] [ -b l2b_file ]
//			[ -t truth_type ] [ -f truth_file ] [ -s low:high ]
//			[ -o output_base ]
//
// DESCRIPTION
//		Generates output files containing kp data.
//
// OPTIONS
//		[ -c config_file ]	Use the specified config file.
//		[ -a l2a_file ]		Use this l2a file.
//		[ -b l2b_file ]		Use this l2b file.
//		[ -t truth_type ]	This is the truth file type.
//		[ -f truth_file ]	This is the truth file.
//		[ -s low:high ]		The range of wind speeds.
//		[ -o output_base ]	The base name to use for output files.
//
//
// OPERANDS
//
// EXAMPLES
//		An example of a command line is:
//			% l2_kp qscat.cfg kp
//
// ENVIRONMENT
//		Not environment dependent.
//
// EXIT STATUS
//		The following exit values are returned:
//		1	Program executed successfully
//		>0	Program had an error
//
// NOTES
//		None.
//
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
#include <math.h>
#include "Misc.h"
#include "L2A.h"
#include "L2B.h"
#include "ConfigList.h"
#include "Ephemeris.h"
#include "ConfigSim.h"
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"

//-----------//
// TEMPLATES //
//-----------//

template class List<StringPair>;
template class List<EarthPosition>;
template class List<Meas>;
template class List<MeasSpot>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<long>;
template class List<OffsetList>;
template class List<WindVectorPlus>;

//-----------//
// CONSTANTS //
//-----------//

#define DEFAULT_LOW_SPEED	0.0
#define DEFAULT_HIGH_SPEED	30.0

//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

#define OPTSTRING			"c:a:b:t:f:s:o:"

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -c config_file ]", "[ -a l2a_file ]",
	"[ -b l2b_file]", "[ -t truth_type ]", "[ -f truth_file ]",
	"[ -s low:high ]",
	"[ -o output_base ]", 0 };

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
	char* l2a_file = NULL;
	char* l2b_file = NULL;
	char* truth_type = NULL;
	char* truth_file = NULL;
	float low_speed = DEFAULT_LOW_SPEED;
	float high_speed = DEFAULT_HIGH_SPEED;
	char* output_base = NULL;

	//------------------------//
	// parse the command line //
	//------------------------//

	const char* command = no_path(argv[0]);

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
		case 'a':
			l2a_file = optarg;
			break;
		case 'b':
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
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

	//---------------------//
	// check for arguments //
	//---------------------//

	if (! l2a_file)
	{
		l2a_file = config_list.Get(L2A_FILE_KEYWORD);
		if (l2a_file == NULL)
		{
			fprintf(stderr, "%s: must specify L2A file\n", command);
			exit(1);
		}
	}

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

	//--------------------//
	// Open level 2A file //
	//--------------------//

	L2A l2a;
	l2a.SetFilename(l2a_file);
	if (! l2a.OpenForReading())
	{
		fprintf(stderr, "%s: error opening L2A file %s\n", command, l2a_file);
		exit(1);
	}

	if (! l2a.ReadHeader())
	{
		fprintf(stderr, "%s: error reading Level 2A header\n", command); 
		exit(1);
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

	//----------------------------//
	// read in "truth" wind field //
	//----------------------------//

	WindField truth;
	truth.ReadType(truth_file, truth_type);

	//---------------//
	// create arrays //
	//---------------//

	int cross_track_bins = l2b.frame.swath.GetCrossTrackBins();
	int along_track_bins = l2b.frame.swath.GetAlongTrackBins();

	float* avg_ctd = (float*)calloc(cross_track_bins,sizeof(float));
	float* var_ctd = (float*)calloc(cross_track_bins,sizeof(float));
	float* error_ctd = (float*)calloc(cross_track_bins,sizeof(float));
	int* count_ctd = (int*)calloc(cross_track_bins,sizeof(int));

	float* avg_atd = (float*)calloc(along_track_bins,sizeof(float));
	float* var_atd = (float*)calloc(along_track_bins,sizeof(float));
	float* error_atd = (float*)calloc(along_track_bins,sizeof(float));
	int* count_atd = (int*)calloc(along_track_bins,sizeof(int));

	char title[1024];

	//--------------------//
	// generate ctd array //
	//--------------------//

	float* ctd_array = new float[cross_track_bins];
	if (! l2b.frame.swath.CtdArray(l2b.header.crossTrackResolution, ctd_array))
	{
		fprintf(stderr, "%s: error generating CTD array\n", command);
		exit(1);
	}

	Index ws_idx;
	ws_idx.SpecifyEdges(low_speed,high_speed,30);
	float* avg_tws = ws_idx.MakeFloatArray();
	float* var_tws = ws_idx.MakeFloatArray();
	float* error_tws = ws_idx.MakeFloatArray();
	int* count_tws = ws_idx.MakeIntArray();
	if (avg_tws == NULL || count_tws == NULL ||
		var_tws == NULL || error_tws == NULL)
	{
		fprintf(stderr, "%s: error generating true WS arrays\n", command);
		exit(1);
	}

	//--------------------------------//
	// create and configure ephemeris //
	//--------------------------------//

	Ephemeris ephemeris;
	if (! ConfigEphemeris(&ephemeris, &config_list))
	{
		fprintf(stderr, "%s: error configuring ephemeris\n", command);
		exit(1);
	}

	//-------------------//
	// open output files //
	//-------------------//

	if (! output_base)
	{
		fprintf(stderr, "%s: must specify L2A file\n", command);
		exit(1);
	}
    char filename[1024];
    sprintf(filename, "%s.%s", output_base, "kp");
    FILE* ofp_kp = fopen(filename, "w");
    if (ofp_kp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            filename);
        exit(1);
    }

	//----------------//
	// loop and write //
	//----------------//

	LonLat lon_lat;
	WindVector true_wv;
	long count = 0;
	float avg_sigma0 = 0.0;
	int i;
	while (l2a.ReadDataRec())
	{
		MeasList* ml = &(l2a.frame.measList);
		for (Meas* m = ml->GetHead(); m; m = ml->GetNext())
		{
			//-----------------------------//
			// Accumulate running averages //
			//-----------------------------//

			float s = m->value;

			// all data
			count++;
			avg_sigma0 += 1.0/count * (s - avg_sigma0);

			// binned by cross track distance
			count_ctd[l2a.frame.cti]++;
			avg_ctd[l2a.frame.cti] += 1.0/count_ctd[l2a.frame.cti] *
				(s - avg_ctd[l2a.frame.cti]);

			// binned by along track distance
			count_atd[l2a.frame.ati]++;
			avg_atd[l2a.frame.ati] += 1.0/count_atd[l2a.frame.ati] *
				(s - avg_atd[l2a.frame.ati]);

			// binned by true wind speed
			if (m->beamIdx == 0)
			{
			lon_lat.Set(m->centroid);
			if (! truth.InterpolatedWindVector(lon_lat, &true_wv))
			{
				printf("Error accessing true wind field in %s\n",command);
				exit(-1);
			}
			if (true_wv.dir-m->eastAzimuth > 50*dtr && true_wv.dir-m->eastAzimuth < 55*dtr)
			if (ws_idx.GetNearestIndex(true_wv.spd, &i))
			{
				count_tws[i]++;
				avg_tws[i] += 1.0/count_tws[i] *
					(s - avg_tws[i]);
			}
			}
		}
	}

	l2a.RewindFile();
	if (! l2a.ReadHeader())
	{
		fprintf(stderr, "%s: error reading Level 2A header\n", command); 
		exit(1);
	}

	double diff = 0.0;
	double var_all = 0.0;
	double error_all = 0.0;
	while (l2a.ReadDataRec())
	{
		MeasList* ml = &(l2a.frame.measList);
		for (Meas* m = ml->GetHead(); m; m = ml->GetNext())
		{
			//----------------------//
			// Accumulate variances //
			//----------------------//

			float s = m->value;

			// all data
			diff = s - avg_sigma0;
			var_all += diff*diff;
			error_all += diff;

			// binned by cross track distance
			diff = s - avg_ctd[l2a.frame.cti];
			var_ctd[l2a.frame.cti] += diff*diff;
			error_ctd[l2a.frame.cti] += diff;

			// binned by along track distance
			diff = s - avg_atd[l2a.frame.ati];
			var_atd[l2a.frame.ati] += diff*diff;
			error_atd[l2a.frame.ati] += diff;

			// binned by true wind speed
			if (m->beamIdx == 0)
			{
			lon_lat.Set(m->centroid);
			if (! truth.InterpolatedWindVector(lon_lat, &true_wv))
			{
				printf("Error accessing true wind field in %s\n",command);
				exit(-1);
			}
			if (true_wv.dir-m->eastAzimuth > 50*dtr && true_wv.dir-m->eastAzimuth < 55*dtr)
			if (ws_idx.GetNearestIndex(true_wv.spd, &i))
			{
				diff = s - avg_tws[i];
				var_tws[i] += diff*diff;
				error_tws[i] += diff;
			}
			}
		}
	}

	//----------------------------//
	// Finish computing variances //
	// Convert to Kp			  //
	//----------------------------//

	var_all = (var_all - error_all*error_all/count)/(count-1);
	float Kp_all = 10*log(1.0 + sqrt(var_all)/avg_sigma0)/log(10.0);
	printf("Overall Kp, count = %g dB, %d\n",Kp_all,count);

/*
	// binned by cross track distance
	for (i=0; i < cross_track_bins; i++)
	{
		if (count_ctd[i] > 0)
		{
			var_ctd[i] = (var_ctd[i] - error_ctd[i]*error_ctd[i]/count_ctd[i]) /
				(count_ctd[i] - 1);
			var_ctd[i] = 10*log(1.0 + sqrt(var_ctd[i])/avg_ctd[i])/log(10.0);
		}
		fprintf(ofp_kp,"%g\n",var_ctd[i]);
		
	}
	// binned by along track distance
	for (i=0; i < along_track_bins; i++)
	{
		if (count_atd[i] > 0)
		{
		var_atd[i] = (var_atd[i] - error_atd[i]*error_atd[i]/count_atd[i]) /
			(count_atd[i] - 1);
		var_atd[i] = 10*log(1.0 + sqrt(var_atd[i])/avg_atd[i])/log(10.0);
		}
		fprintf(ofp_kp,"%g\n",var_atd[i]);
	}
*/

	// binned by true wind speed
	for (i=0; i < ws_idx.GetBins(); i++)
	{
		if (count_tws[i] > 0)
		{
			var_tws[i] = (var_tws[i] - error_tws[i]*error_tws[i]/count_tws[i]) /
				(count_tws[i] - 1);
			var_tws[i] = 10*log(1.0 + sqrt(var_tws[i])/avg_tws[i])/log(10.0);
		}
		float tws;
		if (ws_idx.IndexToValue(i,&tws))
		{
			fprintf(ofp_kp,"%g %g %d\n",tws,var_tws[i],count_tws[i]);
		}
	}

	//------------------------------//
	// Generate XMGR plotting files //
	//------------------------------//

	//-----------------//
	// close the files //
	//-----------------//

	fclose(ofp_kp);
	l2a.Close();
	l2b.Close();

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

/**

//------------//
// plot_thing //
//------------//

int
plot_thing(
	const char*		output_base,
	const char*		extension,
	const char*		title,
	const char*		subtitle,
	const char*		x_axis,
	const char*		y_axis,
	Index			idx,
	int				count_array,
	float*			out_array)
{
	char filename[1024];
	sprintf(filename, "%s.%s", output_base, extension);
	FILE* ofp = fopen(filename, "w");
	if (ofp == NULL)
	{
		fprintf(stderr, "Error opening output file %s\n", filename);
		exit(1);
	}

	xmgr_control(ofp, title, subtitle, x_axis, y_axis);

	for (int i = 0; i < idx.GetBins(); i++)
	{
		if (count_array[i] > 0)
		{
			float x;
			idx.IndexToMidValue(i,&x);
			fprintf(ofp, "%g %g %d\n", x, out_array[i], count_array[i]);
		}
	}
	fclose(ofp);

	return(1);
}

**/

/**
	//-------------------------//
	// rms speed error vs. ctd //
	//-------------------------//

	if (! l2b.frame.swath.RmsSpdErrVsCti(&truth, value_array, count_array,
		low_speed, high_speed))
	{
		fprintf(stderr, "%s: error calculating RMS speed error\n", command);
		exit(1);
	}

	sprintf(title, "RMS Speed Error vs. CTD (%g - %g m/s)", low_speed,
		high_speed);
	plot_thing("rms_spd_err", title, "Cross Track Distance (km)",
		"RMS Speed Error (m/s)");
**/
