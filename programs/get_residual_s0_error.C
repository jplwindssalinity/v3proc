//=========================================================//
// Copyright (C) 1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    get_residual_sigma0_error
//
// SYNOPSIS
//    l2a_s0 <config_file> <l2a_file> <l2bfile> <output_file>
//
// DESCRIPTION
//    Reads in a Level 2A file and 2B file and writes out the following 16 values:
//    A % comes before the last string to allow easy reading by MATLAB
//    s0_residual_error, s0_model, MeasType_Int  Incidence_Angle,
//    East_Azimuth  Sigma-0 Wind_Speed Wind_Dir 
//    A B C Latitude 
//    Longitude CTI ATI % MeasType_String 
//
// OPTIONS
//    [ -c cti ]  Restrict output to the given cross track index.
//    [ -a ati ]  Restrict output to the given along track index.
//    [ -d D ]  decimate by factor D
//
// OPERANDS
//    The following operand is supported:
//    <l2a_file>     The Level 2A input file.
//    <l2b_file>     The Level 2B input file.
//    <config_file>     Configuration file.
//    <output_file>  The output file.
//
// EXAMPLES
//    An example of a command line is:
//      % l2a_s0 -c 5 l2a.dat l2a.s0
//
// ENVIRONMENT
//    Not environment dependent.
//
// EXIT STATUS
//    The following exit values are returned:
//       0  Program executed successfully
//      >0  Program had an error
//
// NOTES
//    None.
//
// AUTHORS
//    James N. Huddleston (hudd@casket.jpl.nasa.gov)
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
#include <unistd.h>
#include <stdlib.h>
#include "ConfigList.h"
#include "ConfigSimDefs.h"
#include "ConfigSim.h"
#include "Wind.h"
#include "L2B.h"
#include "GMF.h"
#include "Misc.h"
#include "L2A.h"
#include "List.h"
#include "BufferedList.h"
#include "Tracking.h"
#include "Qscat.h"

using std::list;
using std::map;
//-----------//
// TEMPLATES //
//-----------//

template class List<EarthPosition>;
template class List<Meas>;
template class List<MeasSpot>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<off_t>;
template class List<OffsetList>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;
template class List<StringPair>;
template class List<WindVectorPlus>;
template class List<AngleInterval>;
template class std::list<string>;
template class std::map<string,string,Options::ltstr>;

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING  "c:a:d:"

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

int g_ati_opt = 0;
int g_cti_opt = 0;
int decim_opt = 0;

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -c cti ]", "[ -a ati ]", "[-d DecimationFactor]", "<config_file>", "<l2a_file>", "<l2b_file>",
    "<output_file>", 0};

//--------------//
// MAIN PROGRAM //
//--------------//

int
main(
    int    argc,
    char*  argv[])
{
    //------------------------//
    // parse the command line //
    //------------------------//

    int cti = 0;
    int ati = 0;
    int decimation_factor=0;

    const char* command = no_path(argv[0]);
    extern int optind;
    extern char *optarg;
    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
        case 'a':
            g_ati_opt = 1;
            ati = atoi(optarg)-1;  // to agree when HDF along track index
            break;
        case 'c':
            g_cti_opt = 1;
            cti = atoi(optarg);
            break;
       case 'd':
            decim_opt = 1;
            decimation_factor = atoi(optarg);
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    if (argc != optind + 4)
        usage(command, usage_array, 1);

    const char* config_file = argv[optind++];
    const char* l2a_file = argv[optind++];
    const char* l2b_file = argv[optind++];
    const char* output_file = argv[optind++];

    ConfigList config_list;
    if (! config_list.Read(config_file))
      {
	fprintf(stderr, "%s: error reading config file %s\n", command, config_file);
	exit(1);
      }
    
    //---------------------//
    // Config GMF          //
    //---------------------//
    GMF gmf;
    if(!ConfigGMF(&gmf,&config_list)){
      fprintf(stderr,"%s: error configuring GMF\n",command);
      exit(1);
    }

    //------------------------//
    // open the Level 2A file //
    //------------------------//

    L2A l2a;
    if (! l2a.OpenForReading(l2a_file))
    {
        fprintf(stderr, "%s: error opening Level 2A file %s\n", command,
            l2a_file);
        exit(1);
    }

    L2B l2b;

    
    if (! l2b.SmartRead(l2b_file))
      {
	fprintf(stderr, "%s: error reading L2B data record from file %s\n",
                command,l2b_file);
	exit(1);
      }
    
    //------------------//
    // open output file //
    //------------------//

    FILE* output_fp = fopen(output_file, "w");
    if (output_fp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            output_file);
        exit(1);
    }

    //----------------//
    // loop and write //
    //----------------//

    int mno=0;
    while (l2a.ReadDataRec())
    {
        if (g_ati_opt && l2a.frame.ati != ati)
            continue;

        if (g_cti_opt && l2a.frame.cti != cti)
            continue;

        if (g_ati_opt && l2a.frame.ati > ati) 
	  break;

        //---- Get wind info
        WVC* wvc=l2b.frame.swath.GetWVC(l2a.frame.cti,l2a.frame.ati);
        if(! wvc || !wvc->selected) continue;
        float dir=wvc->selected->dir;
        float spd=wvc->selected->spd;

        

        MeasList* ml = &(l2a.frame.measList);
        LonLat lonlat = ml->AverageLonLat();
        for (Meas* m = ml->GetHead(); m; m = ml->GetNext())
        {

	    // estimate model sigma0

	    float chi=dir-m->eastAzimuth+pi;
            float models0;
            if(!gmf.GetInterpolatedValue(m->measType,m->incidenceAngle,spd,chi,&models0)){
		fprintf(stderr,"%s: GMF interpolarion failed.\n",command);
                exit(1);
	      }

            if(!decim_opt || mno%decimation_factor==0){
            fprintf(output_fp, "%g %g %d %g %g %g %g %g %g %g %g %g %g %d %d %% %s\n",
		    m->value-models0, models0, (int)m->measType, m->incidenceAngle * rtd,
		    m->eastAzimuth * rtd, m->value,spd,dir*rtd,m->A,m->B,m->C,lonlat.latitude*rtd,lonlat.longitude*rtd,l2a.frame.cti,l2a.frame.ati,
		    meas_type_map[m->measType]);
	    }
	    mno++;
        }
    }

    //-----------------//
    // close the files //
    //-----------------//

    fclose(output_fp);
    l2a.Close();

    return (0);
}
