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
//		[ -1 l1b_file ]		Use this l1b file (then l2a is not used)
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
//			% l2_kp -c qscat.cfg -o l2
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
#include "L1B.h"
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

#define	INPUT_SOURCE_KEYWORD	"INPUT_SOURCE"
#define WIND_SOURCE_KEYWORD		"WIND_SOURCE"
#define XAXIS_KEYWORD			"XAXIS"
#define POLARIZATION_KEYWORD	"POLARIZATION"
#define WIND_SPD_MIN_KEYWORD	"WIND_SPD_MIN"
#define WIND_SPD_MAX_KEYWORD	"WIND_SPD_MAX"
#define WIND_SPD_BINS_KEYWORD	"WIND_SPD_BINS"
#define WIND_RELAZ_MIN_KEYWORD	"WIND_RELAZ_MIN"
#define WIND_RELAZ_MAX_KEYWORD	"WIND_RELAZ_MAX"
#define WIND_RELAZ_BINS_KEYWORD	"WIND_RELAZ_BINS"
#define CTD_MIN_KEYWORD			"CTD_MIN"
#define CTD_MAX_KEYWORD			"CTD_MAX"
#define CTD_BINS_KEYWORD		"CTD_BINS"

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

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "<kp config file>", "<sim config file>",
	"<output_base>", 0};

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

	//------------------------//
	// parse the command line //
	//------------------------//

	const char* command = no_path(argv[0]);

	if (argc != 4)
		usage(command, usage_array, 1);

	int clidx = 1;
	const char* kpcfg_file = argv[clidx++];
	const char* config_file = argv[clidx++];
	const char* output_base = argv[clidx++];

	//------------------------//
	// read in Kp config file //
	//------------------------//

	ConfigList kpcfg_list;
	if (! kpcfg_list.Read(kpcfg_file))
	{
		fprintf(stderr, "%s: error reading config file %s\n",
			command, kpcfg_file);
		exit(1);
	}

	//-------------------------//
	// read in sim config file //
	//-------------------------//

	ConfigList config_list;
	if (! config_list.Read(config_file))
	{
		fprintf(stderr, "%s: error reading sim config file %s\n",
			command, config_file);
		exit(1);
	}

	kpcfg_list.ExitForMissingKeywords();
	config_list.ExitForMissingKeywords();

	const char* input_source = kpcfg_list.Get(INPUT_SOURCE_KEYWORD);
	const char* wind_source = kpcfg_list.Get(WIND_SOURCE_KEYWORD);

	//---------------------//
	// Open and read files //
	//---------------------//

	L1B l1b;
	L2A l2a;
	char* l1b_file = NULL;
	char* l2a_file = NULL;

	if (strcmp(input_source,"1B") == 0)
	{
		l1b_file = config_list.Get(L1B_FILE_KEYWORD);
		l1b.SetFilename(l1b_file);
		if (! l1b.OpenForReading())
		{
			fprintf(stderr, "%s: error opening L1B file %s\n",
				command, l1b_file);
			exit(1);
		}
	}
	else if (strcmp(input_source,"2A") == 0)
	{
		l2a_file = config_list.Get(L2A_FILE_KEYWORD);
		l2a.SetFilename(l2a_file);
		if (! l2a.OpenForReading())
		{
			fprintf(stderr, "%s: error opening L2A file %s\n",
				command, l2a_file);
			exit(1);
		}
		if (! l2a.ReadHeader())
		{
			fprintf(stderr, "%s: error reading Level 2A header\n", command); 
			exit(1);
		}
	}
	else
	{
		fprintf(stderr,"%s: Invalid input source = %s\n",
			command, input_source);
		exit(-1);
	}

	L2B l2b;
	WindField truth;
	WindField* windfield = NULL;
	char* l2b_file = NULL;
	char* truth_file = NULL;
	char* truth_type = NULL;

	if (strcmp(wind_source,"retrieved") == 0)
	{
		l2b_file = config_list.Get(L2B_FILE_KEYWORD);
		l2b.SetFilename(l2b_file);
		if (! l2b.OpenForReading())
		{
			fprintf(stderr, "%s: error opening L2B file %s\n",
				command, l2b_file);
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
		windfield = NULL;
	}
	else if (strcmp(wind_source,"truth") == 0)
	{
		truth_file = config_list.Get(WINDFIELD_FILE_KEYWORD);
		truth_type = config_list.Get(WINDFIELD_TYPE_KEYWORD);
		if (! truth.ReadType(truth_file, truth_type))
		{
			fprintf(stderr, "%s: error accessing truth file %s\n",
				command, truth_file);
			exit(-1);
		}
		windfield = &truth;
	}
	else
	{
		fprintf(stderr,"%s: Invalid wind source = %s\n",
			command, wind_source);
		exit(-1);
	}

	//------------------//
	// Set polarization //
	//------------------//

	char pol_char;
	PolE pol;
	kpcfg_list.GetChar(POLARIZATION_KEYWORD,&pol_char);
	if (pol_char == 'V' || pol_char == 'v')
	{
		pol = V_POL;
	}
	else if (pol_char == 'H' || pol_char == 'h')
	{
		pol = H_POL;
	}
	else
	{
		fprintf(stderr,"%s: Invalid polarization = %c\n",
			command, pol_char);
		exit(-1);
	}
	

	//---------------------------//
	// Get pertinent header data //
	//---------------------------//

	int cross_track_bins;
	int along_track_bins;
	if (l2a_file)
	{
		cross_track_bins = l2a.header.crossTrackBins;
		along_track_bins = l2a.header.alongTrackBins;
	}
	else if (l2b_file)
	{
		cross_track_bins = l2b.frame.swath.GetCrossTrackBins();
		along_track_bins = l2b.frame.swath.GetAlongTrackBins();
	}
	else
	{
		cross_track_bins = 0;
		along_track_bins = 0;
	}

	float ws_min,ws_max;
	int ws_bins;
	kpcfg_list.GetFloat(WIND_SPD_MIN_KEYWORD,&ws_min);
	kpcfg_list.GetFloat(WIND_SPD_MAX_KEYWORD,&ws_max);
	kpcfg_list.GetInt(WIND_SPD_BINS_KEYWORD,&ws_bins);
	float wrelaz_min,wrelaz_max;
	int wrelaz_bins;
	kpcfg_list.GetFloat(WIND_RELAZ_MIN_KEYWORD,&wrelaz_min);
	kpcfg_list.GetFloat(WIND_RELAZ_MAX_KEYWORD,&wrelaz_max);
	kpcfg_list.GetInt(WIND_RELAZ_BINS_KEYWORD,&wrelaz_bins);
	float ctd_min,ctd_max;
	int ctd_bins;
	kpcfg_list.GetFloat(CTD_MIN_KEYWORD,&ctd_min);
	kpcfg_list.GetFloat(CTD_MAX_KEYWORD,&ctd_max);
	kpcfg_list.GetInt(CTD_BINS_KEYWORD,&ctd_bins);

	//-----------------------------------------------//
	// create indices to track independent variables //
	//-----------------------------------------------//

	Index ws_idx;
	ws_idx.SpecifyEdges(ws_min,ws_max,ws_bins);
	Index wrelaz_idx;
	wrelaz_idx.SpecifyEdges(wrelaz_min,wrelaz_max,wrelaz_bins);
	Index ctd_idx;
	ctd_idx.SpecifyEdges(ctd_min,ctd_max,ctd_bins);

	//-------------------------------------------//
	// Assign the primary index and case indices //
	//-------------------------------------------//

	Index x_idx,c1_idx,c2_idx;
	const char* xaxis = kpcfg_list.Get(XAXIS_KEYWORD);
	int caseid;
	if (strcmp(xaxis,"wind_speed") == 0)
	{
		x_idx = ws_idx;
		c1_idx = wrelaz_idx;
		c2_idx = ctd_idx;
		caseid = 1;
	}
	else if (strcmp(xaxis,"wind_relaz") == 0)
	{
		x_idx = wrelaz_idx;
		c1_idx = ws_idx;
		c2_idx = ctd_idx;
		caseid = 2;
	}
	else if (strcmp(xaxis,"ctd") == 0)
	{
		x_idx = ctd_idx;
		c1_idx = ws_idx;
		c2_idx = wrelaz_idx;
		caseid = 3;
	}
	else
	{
		fprintf(stderr,"%s: Invalid xaxis index = %s\n",
			command, xaxis);
		exit(-1);
	}

	//---------------//
	// create arrays //
	//---------------//

	float** avg = (float**)make_array(sizeof(float),2,c1_idx.GetBins(),
					x_idx.GetBins());
	float** var = (float**)make_array(sizeof(float),2,c1_idx.GetBins(),
					x_idx.GetBins());
	float** error = (float**)make_array(sizeof(float),2,c1_idx.GetBins(),
					x_idx.GetBins());
	int** count = (int**)make_array(sizeof(int),2,c1_idx.GetBins(),
					x_idx.GetBins());
	if (avg == NULL || count == NULL ||
		var == NULL || error == NULL)
	{
		fprintf(stderr, "%s: error generating accumulation arrays\n", command);
		exit(1);
	}

	int i,j;
	for (i=0; i < c1_idx.GetBins(); i++)
	for (j=0; j < x_idx.GetBins(); j++)
	{
		avg[i][j] = 0.0;
		var[i][j] = 0.0;
		error[i][j] = 0.0;
		count[i][j] = 0;
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

	//-----------------------------//
	// 2-Pass Variance Calculation //
	//-----------------------------//

	LonLat lon_lat;
	WindVector wv;
	long count_all = 0;
	float avg_sigma0 = 0.0;
	MeasList* ml = NULL;
	if (l1b_file) ml = new MeasList;

	while(1)
	{
		Meas* m;
		if (l1b_file)
		{
			ml->FreeContents();
			if (! l1b.ReadDataRec()) break;
//			ml = l1b.frame.spotList.mergeLists();
			MeasSpot* spot;
			l1b.frame.spotList.GotoHead();
			while ((spot = l1b.frame.spotList.RemoveCurrent()))
			{
				ml->AppendList(spot);
			}
		}
		else
		{
			if (! l2a.ReadDataRec()) break;
			ml = &(l2a.frame.measList);
		}

		for (m = ml->GetHead(); m; m = ml->GetNext())
		{
			//-----------------------------//
			// Accumulate running averages //
			//-----------------------------//

			float s = m->value;

			// all data
			count_all++;
			avg_sigma0 += 1.0/count_all * (s - avg_sigma0);

			//------------------------------------------------------------//
			// Bin each value of the primary index, sorting with the case //
			// indices.													  //
			//------------------------------------------------------------//

			if (pol != m->pol) continue;

			lon_lat.Set(m->centroid);

			if (windfield)
			{
				if (! windfield->InterpolatedWindVector(lon_lat, &wv))
				{
					printf("Error accessing true wind field in %s\n",command);
					exit(-1);
				}
			}
			else
			{
				printf("Error: no wind field to use in %s\n",command);
				exit(-1);
			}

			if (caseid == 1 && x_idx.GetNearestIndex(wv.spd,&j) &&
				c1_idx.GetNearestIndex(wv.dir,&i))
			{
				count[i][j]++;
				avg[i][j] += 1.0/count[i][j] * (s - avg[i][j]);
			}
		}
	}

	if (l1b_file)
	{
		l1b.RewindFile();
	}
	else
	{
		l2a.RewindFile();
		if (! l2a.ReadHeader())
		{
			fprintf(stderr, "%s: error reading Level 2A header\n", command); 
			exit(1);
		}
	}

	double diff = 0.0;
	double var_all = 0.0;
	double error_all = 0.0;
	while(1)
	{
		ml->FreeContents();
		Meas* m;
		if (l1b_file)
		{
			if (! l1b.ReadDataRec()) break;
//			ml = l1b.frame.spotList.mergeLists();
			MeasSpot* spot;
			l1b.frame.spotList.GotoHead();
			while ((spot = l1b.frame.spotList.RemoveCurrent()))
			{
				ml->AppendList(spot);
			}
		}
		else
		{
			if (! l2a.ReadDataRec()) break;
			ml = &(l2a.frame.measList);
		}

		for (m = ml->GetHead(); m; m = ml->GetNext())
		{
			//----------------------//
			// Accumulate variances //
			//----------------------//

			float s = m->value;

			// all data
			diff = s - avg_sigma0;
			var_all += diff*diff;
			error_all += diff;

			//------------------------------------------------------------//
			// Bin each value of the primary index, sorting with the case //
			// indices.													  //
			//------------------------------------------------------------//

			if (pol != m->pol) continue;

			lon_lat.Set(m->centroid);

			if (truth_file)
			{
				if (! truth.InterpolatedWindVector(lon_lat, &wv))
				{
					printf("Error accessing true wind field in %s\n",command);
					exit(-1);
				}
			}
			else
			{
				wv.spd = 0;
				wv.dir = 0;
			}


			if (caseid == 1 && x_idx.GetNearestIndex(wv.spd,&j) &&
				c1_idx.GetNearestIndex(wv.dir,&i))
			{
				diff = s - avg[i][j];
				var[i][j] += diff*diff;
				error[i][j] += diff;
			}
		}
	}

	//----------------------------//
	// Finish computing variances //
	// Convert to Kp			  //
	//----------------------------//

	var_all = (var_all - error_all*error_all/count_all)/(count_all-1);
	float Kp_all = 10*log(1.0 + sqrt(var_all)/avg_sigma0)/log(10.0);
	printf("Overall Kp, count = %g dB, %ld\n",Kp_all,count_all);

/*
	// binned by cross track distance
	for (i=0; i < cross_track_bins; i++)
	{
		if (count_ctd[i] > 1)
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
		if (count_atd[i] > 1)
		{
		var_atd[i] = (var_atd[i] - error_atd[i]*error_atd[i]/count_atd[i]) /
			(count_atd[i] - 1);
		var_atd[i] = 10*log(1.0 + sqrt(var_atd[i])/avg_atd[i])/log(10.0);
		}
		fprintf(ofp_kp,"%g\n",var_atd[i]);
	}
*/

	for (i=0; i < c1_idx.GetBins(); i++)
	for (j=0; j < x_idx.GetBins(); j++)
	{
		if (count[i][j] > 1)
		{
			var[i][j] = (var[i][j] - error[i][j]*error[i][j]/count[i][j]) /
				(count[i][j] - 1);
			var[i][j] = 10*log(1.0 + sqrt(var[i][j])/avg[i][j])/log(10.0);
		}
		float x,c1;
		if (x_idx.IndexToValue(j,&x) && c1_idx.IndexToValue(i,&c1))
		{
			fprintf(ofp_kp,"%g %g %g %g %d\n",c1,x,var[i][j],avg[i][j],
				count[i][j]);
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
