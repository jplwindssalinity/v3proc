//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

//----------------------------------------------------------------------
// NAME
//		l2ab_obj_comp
//
// SYNOPSIS
//		l2ab_obj_comp <sim_config_file> <output base> <l2b file 1> <l2b file 2>
//
// DESCRIPTION
//		Simulates the SeaWinds 1b ground processing of Level 2A to
//		Level 2B data.  This program retrieves wind from measurements.
//
// OPTIONS
//		None.
//
// OPERANDS
//		The following operand is supported:
//		<sim_config_file>		The sim_config_file needed listing
//								all input parameters, input files, and
//								output files.
//		<output base name>
//
// EXAMPLES
//		An example of a command line is:
//			% l2ab_obj_comp sws1b.cfg objplot
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
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Misc.h"
#include "ConfigList.h"
#include "L2A.h"
#include "ConfigSim.h"
#include "L2B.h"
#include "L2AToL2B.h"
#include "Tracking.h"
#include "Tracking.C"


//-----------//
// TEMPLATES //
//-----------//

template class List<StringPair>;
template class List<Meas>;
template class List<EarthPosition>;
template class List<WindVectorPlus>;
template class List<MeasSpot>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<long>;
template class List<OffsetList>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;

//-----------//
// CONSTANTS //
//-----------//

#define ANGTOL	45

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

const char* usage_array[] = { "<sim_config_file>", "<output base>",
							 	"<l2b_file_pe>", "<l2b_file_gs>", 0};

//--------------------//
// Report handler     //
// runs if SIGUSR1 is //
// recieved.          //
//--------------------//

int global_frame_number=0;

void report(int sig_num){
  fprintf(stderr,"l2ab_obj_comp: Starting frame number %d\n",
	  global_frame_number);
  return;
}

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
	int use_l2a = 0;
	int clidx = 1;
	const char* config_file = NULL;
	const char* output_base = NULL;
	const char* l2b1_file = NULL;
	const char* l2b2_file = NULL;
    const char* truth_type = NULL;
    const char* truth_file = NULL;
 

	if (argc == 3)
	{
		use_l2a = 1;
		clidx = 1;
		config_file = argv[clidx++];
		output_base = argv[clidx++];
	}
	else if (argc == 5)
	{
		clidx = 1;
		config_file = argv[clidx++];
		output_base = argv[clidx++];
		l2b1_file = argv[clidx++];
		l2b2_file = argv[clidx++];
	}
	else
	{
		usage(command, usage_array, 1);
	}

        //------------------------//
        // tell how far you have  //
        // gotten if you recieve  //
        // the siguser1 signal    //
        //------------------------//

        sigset(SIGUSR1,&report);

	//---------------------//
	// read in config file //
	//---------------------//

	ConfigList config_list;
	if (! config_list.Read(config_file))
	{
		fprintf(stderr, "%s: error reading sim config file %s\n",
			command, config_file);
		exit(1);
	}

	//------------------//
	// setup truth file //
	//------------------//

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

    //----------------------------//
    // read in "truth" wind field //
    //----------------------------//

    WindField truth;
    truth.ReadType(truth_file, truth_type);

	//-------------------------------------//
	// create and configure level products //
	//-------------------------------------//

	L2A l2a;
	L2B l2b1,l2b2;
	if (use_l2a)
	{
		if (! ConfigL2A(&l2a, &config_list))
		{
			fprintf(stderr, "%s: error configuring Level 2A Product\n",
					 command);
			exit(1);
		}
	}
	else
	{
		l2b1.SetInputFilename(l2b1_file);
		l2b2.SetInputFilename(l2b2_file);
	}

	//-------------------------------------//
	// read the geophysical model function //
	//-------------------------------------//
 
	GMF gmf;
	if (! ConfigGMF(&gmf, &config_list))
	{
		fprintf(stderr, "%s: error configuring GMF\n", command);
		exit(1);
	}

	//--------------//
	// configure Kp //
	//--------------//

	Kp kp;
	if (! ConfigKp(&kp, &config_list))
	{
		fprintf(stderr, "%s: error configuring Kp\n", command);
		exit(1);
	}

	//-----------------------------//
	// open files and read headers //
	//-----------------------------//

	if (use_l2a)
	{
		if (! l2a.OpenForReading())
    	{
        	fprintf(stderr,"%s: error opening L2A file %s\n",
					command, l2a.GetInputFilename());
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
    	if (! l2b1.OpenForReading())
    	{
        	fprintf(stderr, "%s: error opening L2B file %s\n",
					command, l2b1_file);
        	exit(1);
    	}
    	if (! l2b1.ReadHeader())
    	{
        	fprintf(stderr, "%s: error reading L2B header from file %s\n",
            	command, l2b1_file);
        	exit(1);
    	}
    	if (! l2b1.ReadDataRec())
    	{
        	fprintf(stderr, "%s: error reading L2B swath from file %s\n",
            	command, l2b1_file);
        	exit(1);
    	}

    	if (! l2b2.OpenForReading())
    	{
        	fprintf(stderr, "%s: error opening L2B file %s\n",
					command, l2b2_file);
        	exit(1);
    	}
    	if (! l2b2.ReadHeader())
    	{
        	fprintf(stderr, "%s: error reading L2B header from file %s\n",
            	command, l2b2_file);
        	exit(1);
    	}
    	if (! l2b2.ReadDataRec())
    	{
        	fprintf(stderr, "%s: error reading L2B swath from file %s\n",
            	command, l2b2_file);
        	exit(1);
    	}

	}

	int count = 1;
	int graphnumber = 1;
	FILE* ofp = NULL;
	char filename[1024];
	int total_wvc = 0;
	int total_gs_1 = 0;
	int total_gs_2 = 0;
	int total_gs_3 = 0;
	int total_gs_4 = 0;
	int total_pe_1 = 0;
	int total_pe_2 = 0;
	int total_pe_3 = 0;
	int total_pe_4 = 0;
	int total_missed_pe = 0;
	int total_missed_gs = 0;
	int total_missed_both = 0;
	float frac_gs_1,frac_gs_2,frac_gs_3,frac_gs_4;
	float frac_pe_1,frac_pe_2,frac_pe_3,frac_pe_4;
	float frac_missed_pe,frac_missed_gs,frac_missed_both;
	int cti = 0;
	int ati = 0;
	WVC* wvc1 = NULL;
	WVC* wvc2 = NULL;

	//---------------//
	// scanning loop //
	//---------------//

	for (;;)
	{
		if (use_l2a)
		{
	        global_frame_number++;

			//-----------------------------//
			// read a level 2A data record //
			//-----------------------------//

			if (! l2a.ReadDataRec())
			{
				switch (l2a.GetStatus())
				{
				case L2A::OK:		// end of file
					break;
				case L2A::ERROR_READING_FRAME:
					fprintf(stderr, "%s: error reading Level 2A data\n",
							command);
					exit(1);
					break;
				case L2A::ERROR_UNKNOWN:
					fprintf(stderr, "%s: unknown error reading Level 2A data\n",
						command);
					exit(1);
					break;
				default:
					fprintf(stderr, "%s: unknown status (???)\n", command);
					exit(1);
				}
				break;		// done, exit do loop
			}

//			if (l2a.frame.cti < 60 || l2a.frame.cti > 70) continue;

			//---------//
			// process //
			//---------//

	    	MeasList* meas_list = &(l2a.frame.measList);

    		//-----------------------------------//
    		// check for missing wind field data //
    		//-----------------------------------//

    		int any_zero = 0;
    		for (Meas* meas = meas_list->GetHead(); meas;
			 	meas = meas_list->GetNext())
    		{
        		if (! meas->value)
        		{
            		any_zero = 1;
            		break;
        		}
    		}
    		if (any_zero)
    		{
        		continue;
    		}

    		//-----------------------------------//
    		// check for wind retrieval criteria //
    		//-----------------------------------//

    		if (! gmf.CheckRetrieveCriteria(meas_list))
    		{
        		continue;
    		}

    		//---------------//
    		// retrieve wind //
    		//---------------//

	    	wvc1 = new WVC();
			if (! gmf.RetrieveWinds(meas_list, &kp, wvc1))
       		{
           		delete wvc1;
           		continue;
       		}
 
	    	wvc2 = new WVC();
			if (! gmf.GSRetrieveWinds(meas_list, &kp, wvc2))
       		{
           		delete wvc1;
           		delete wvc2;
           		continue;
       		}

		    wvc1->lonLat = meas_list->AverageLonLat();
		    wvc2->lonLat = meas_list->AverageLonLat();
		}
		else
		{	// use l2b ambiguities
			cti++;
			if (cti >= l2b1.frame.swath.GetCrossTrackBins())
			{
				cti = 0;
				ati++;
				if (ati >= l2b1.frame.swath.GetAlongTrackBins())
				{
					break;
				}
			}

			wvc1 = l2b1.frame.swath.swath[cti][ati];
			wvc2 = l2b2.frame.swath.swath[cti][ati];

			if (wvc1 == NULL || wvc2 == NULL) continue;
		}
 
		total_wvc++;

    	//------------------------------------------//
    	// Compute number of mismatched ambiguities //
    	//------------------------------------------//

		int mismatched = 0;
		WindVectorPlus *wvp1, *wvp2;
		for (wvp2 = wvc2->ambiguities.GetHead(); wvp2;
			 wvp2 = wvc2->ambiguities.GetNext())
		{
			int ok = 0;
			for (wvp1 = wvc1->ambiguities.GetHead(); wvp1;
				 wvp1 = wvc1->ambiguities.GetNext())
			{
				if (angle_diff(wvp1->dir,wvp2->dir) < dtr*ANGTOL)
				{
					ok = 1;
				}
			}
			if (! ok)
			{
				mismatched++;
			}
		}
		if (mismatched == 1) total_gs_1++;
		if (mismatched == 2) total_gs_2++;
		if (mismatched == 3) total_gs_3++;
		if (mismatched == 4) total_gs_4++;

		mismatched = 0;
		for (wvp1 = wvc1->ambiguities.GetHead(); wvp1;
			 wvp1 = wvc1->ambiguities.GetNext())
		{
			int ok = 0;
			for (wvp2 = wvc2->ambiguities.GetHead(); wvp2;
				 wvp2 = wvc2->ambiguities.GetNext())
			{
				if (angle_diff(wvp1->dir,wvp2->dir) < dtr*ANGTOL)
				{
					ok = 1;
				}
			}
			if (! ok)
			{
				mismatched++;
			}
		}
		if (mismatched == 1) total_pe_1++;
		if (mismatched == 2) total_pe_2++;
		if (mismatched == 3) total_pe_3++;
		if (mismatched == 4) total_pe_4++;

		frac_gs_1 = (float)total_gs_1 / (float)total_wvc;
		frac_gs_2 = (float)total_gs_2 / (float)total_wvc;
		frac_gs_3 = (float)total_gs_3 / (float)total_wvc;
		frac_gs_4 = (float)total_gs_4 / (float)total_wvc;

		frac_pe_1 = (float)total_pe_1 / (float)total_wvc;
		frac_pe_2 = (float)total_pe_2 / (float)total_wvc;
		frac_pe_3 = (float)total_pe_3 / (float)total_wvc;
		frac_pe_4 = (float)total_pe_4 / (float)total_wvc;

    	//----------------------------------//
    	// Compute number of missed truth's //
    	//----------------------------------//

		WindVector true_wv;
        if (! truth.InterpolatedWindVector(wvc1->lonLat, &true_wv))
		{
			fprintf(stderr,"Error interpolating true wind field\n");
			exit(1);
		}
		while (true_wv.dir < 0) true_wv.dir += two_pi;

		int missed_pe = 1;
		for (wvp1 = wvc1->ambiguities.GetHead(); wvp1;
			 wvp1 = wvc1->ambiguities.GetNext())
		{
			if (angle_diff(wvp1->dir,true_wv.dir) < dtr*ANGTOL)
			{
				missed_pe = 0;
			}
		}
		if (missed_pe)
		{
			total_missed_pe++;
		}

		int missed_gs = 1;
		for (wvp2 = wvc2->ambiguities.GetHead(); wvp2;
			 wvp2 = wvc2->ambiguities.GetNext())
		{
			if (angle_diff(wvp2->dir,true_wv.dir) < dtr*ANGTOL)
			{
				missed_gs = 0;
			}
		}
		if (missed_gs)
		{
			total_missed_gs++;
		}

		if (missed_gs && missed_pe)
		{
			total_missed_both++;
		}

		frac_missed_pe = (float)total_missed_pe / (float)total_wvc;
		frac_missed_gs = (float)total_missed_gs / (float)total_wvc;
		frac_missed_both = (float)total_missed_both / (float)total_wvc;

		if (total_wvc % 1000 == 0)
		{
			printf("%d %g %g %g %g   %g %g %g %g   %g %g %g\n",total_wvc,
					frac_gs_1,frac_gs_2,frac_gs_3,frac_gs_4,
					frac_pe_1,frac_pe_2,frac_pe_3,frac_pe_4,
					frac_missed_pe,frac_missed_gs,frac_missed_both);
		}

		int output = 0;
		if ((missed_gs) && (!missed_pe))
		{
			output = 1;
		}

//		if (wvc1->ambiguities.NodeCount() == wvc2->ambiguities.NodeCount())
//		{
//			for (wvp1 = wvc1->ambiguities.GetHead(),
//				 wvp2 = wvc2->ambiguities.GetHead(); wvp1;
//				 wvp1 = wvc1->ambiguities.GetNext(),
//				 wvp2 = wvc2->ambiguities.GetNext())
//			{
//				if (fabs(wrap_angle_near(wvp1->dir - wvp2->dir, 0.0)) >
//					 dtr*ANGTOL)
//				{
//					output = 1;
//				}
//			}
//		}
//		else
//		{
//			num_mismatched++;
//			output = 1;
//		}

		if (output == 1 && use_l2a)
		{
			if (graphnumber == 1)
			{
				sprintf(filename,"%s.%d",output_base,count);
				ofp = fopen(filename,"w");
				if (ofp == NULL)
				{
					fprintf(stderr,"Error opening %s\n",filename);
					exit(-1);
				}
			}
			else
			{
			    fprintf(ofp, "&\n");
			}

			fprintf(ofp,"@WITH G%d\n",graphnumber-1);
			fprintf(ofp,"@G%d ON\n",graphnumber-1);

			gmf.WriteObjectiveCurve(ofp);
			gmf.AppendSolutions(ofp,wvc1);
			gmf.AppendSolutions(ofp,wvc2);
		    fprintf(ofp, "&\n");
	        fprintf(ofp, "%g %g\n", true_wv.dir * rtd, 0.5);
//	        printf("%g %g\n", true_wv.dir * rtd, 0.0);

			if (graphnumber == 10)
			{
				graphnumber = 1;
				fclose(ofp);
				count++;
			}
			else
			{
				graphnumber++;
			}
		}

		if (use_l2a)
		{
			delete wvc1;
			delete wvc2;
		}
	}

	printf("%d %g %g %g %g   %g %g %g %g   %g %g %g\n",total_wvc,
			frac_gs_1,frac_gs_2,frac_gs_3,frac_gs_4,
			frac_pe_1,frac_pe_2,frac_pe_3,frac_pe_4,
			frac_missed_pe,frac_missed_gs,frac_missed_both);

	l2a.Close();

	return (0);
}
