//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    nbd_conclass
//
// SYNOPSIS
//    nbd_conclass [ -m minutes ] [ -r rain_rate ] <start_rev>
//        <end_rev> <output_base>
//
// DESCRIPTION
//    Makes some nbd and rain rate comparisons.
//
// OPTIONS
//    [ -m minutes ]    Time difference maximum.
//    [ -r rain_rate ]  The SSM/I rain rate to threshold.
//
// OPERANDS
//    <start_rev>    Duh.
//    <end_rev>      Duh again.
//    <output_base>  The output base.
//
// EXAMPLES
//    An example of a command line is:
//      % nbd_conclass -m 15 1500 1600 15min
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
// AUTHOR
//    James N. Huddleston (James.N.Huddleston@jpl.nasa.gov)
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
#include "Misc.h"
#include "ParTab.h"
#include "ArgDefs.h"
#include "ConfigList.h"
#include "PolyErrNo.h"
#include "PolyTable.h"
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Tracking.h"
#include "Tracking.C"
#include "GMF.h"
#include "ConfigSimDefs.h"
#include "L2B.h"
#include "ConfigSim.h"
#include "Interpolate.h"
#include "SeaPac.h"

//-----------//
// TEMPLATES //
//-----------//

template class List<StringPair>;
template class List<Meas>;
template class List<MeasSpot>;
template class List<OffsetList>;
template class List<long>;
template class List<OrbitState>;
template class BufferedList<OrbitState>;
template class List<EarthPosition>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;
template class List<WindVectorPlus>;
template class List<AngleInterval>;

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING    "m:r:"
#define AT_WIDTH     1624
#define CT_WIDTH     76
#define MIN_SAMPLES  20

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -m minutes ]", "[ -r rain_rate ]",
    "<start_rev>", "<end_rev>", "<output_base>", 0 };

// parameter (nbd,spd,dir,mle,prog); ssmi (0=total,1=clear,2=rain)
static unsigned long counts[16][16][16][16][3];
static double rain_rate_sum[16][16][16][16];

float range[4][2] = {
    { -4.0, 6.0 },    // NBD
    { 0.0, 30.0 },    // speed
    { 0.0, 90.0 },    // direction
    { -10.0, 0.0 }    // MLE
};

//--------------//
// MAIN PROGRAM //
//--------------//

int
main(
    int    argc,
    char*  argv[])
{
    //------------//
    // initialize //
    //------------//

    int minutes = 180;    // three hours
    float rain_threshold = 0.0;

    //------------------------//
    // parse the command line //
    //------------------------//

    const char* command = no_path(argv[0]);

    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
        case 'm':
            minutes = atoi(optarg);
            break;
        case 'r':
            rain_threshold = atof(optarg);
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    if (argc < optind + 3)
        usage(command, usage_array, 1);

    int start_rev = atoi(argv[optind++]);
    int end_rev = atoi(argv[optind++]);
    const char* output_base = argv[optind++];

    //-------------------//
    // open output files //
    //-------------------//

    char filename[2048];
    sprintf(filename, "%s.conclass", output_base);
    FILE* conclass_ofp = fopen(filename, "w");
    if (conclass_ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            filename);
        exit(1);
    }

    //--------------------//
    // process rev by rev //
    //--------------------//

    unsigned char nbd_array[AT_WIDTH][CT_WIDTH];
    unsigned char spd_array[AT_WIDTH][CT_WIDTH];
    unsigned char dir_array[AT_WIDTH][CT_WIDTH];
    unsigned char mle_array[AT_WIDTH][CT_WIDTH];

    for (int rev = start_rev; rev <= end_rev; rev++)
    {
        //---------------//
        // read nbd file //
        //---------------//

        char nbd_file[1024];
        sprintf(nbd_file, "%d.nbd", rev);
        FILE* ifp = fopen(nbd_file, "r");
        if (ifp == NULL)
        {
            fprintf(stderr, "%s: error opening NBD file %s\n", command,
                nbd_file);
            exit(1);
        }
        fread(nbd_array, sizeof(char), CT_WIDTH * AT_WIDTH, ifp);
        fread(spd_array, sizeof(char), CT_WIDTH * AT_WIDTH, ifp);
        fread(dir_array, sizeof(char), CT_WIDTH * AT_WIDTH, ifp);
        fread(mle_array, sizeof(char), CT_WIDTH * AT_WIDTH, ifp);
        fclose(ifp);

        //----------------//
        // read rain file //
        //----------------//

        char rain_file[1024];
        sprintf(rain_file, "/export/svt11/hudd/ssmi/%d.rain", rev);
        unsigned char rain_rate[AT_WIDTH][CT_WIDTH];
        unsigned char time_dif[AT_WIDTH][CT_WIDTH];
        ifp = fopen(rain_file, "r");
        if (ifp == NULL)
        {
            fprintf(stderr, "%s: error opening rain file %s\n", command,
                rain_file);
            exit(1);
        }
        fread(rain_rate, sizeof(char), CT_WIDTH * AT_WIDTH, ifp);
        fread(time_dif, sizeof(char), CT_WIDTH * AT_WIDTH, ifp);
        fclose(ifp);

        //---------------------------------------//
        // eliminate rain data out of time range //
        //---------------------------------------//

        for (int ati = 0; ati < AT_WIDTH; ati++)
        {
            for (int cti = 0; cti < CT_WIDTH; cti++)
            {
                int co_time = time_dif[ati][cti] * 2 - 180;
                if (abs(co_time) > minutes)
                    rain_rate[ati][cti] = 255;
            }
        }

        //-------------------------//
        // generate conclass table //
        //-------------------------//

        for (int ati = 0; ati < AT_WIDTH; ati++)
        {
            for (int cti = 0; cti < CT_WIDTH; cti++)
            {
                if (rain_rate[ati][cti] >= 250 ||
                    spd_array[ati][cti] == 255 ||
                    dir_array[ati][cti] == 255 ||
                    mle_array[ati][cti] == 255)
                {
                    continue;
                }
                double nbd = (double)nbd_array[ati][cti] / 20.0 - 6.0;
                double spd = (double)spd_array[ati][cti] / 5.0;
                double dir = (double)dir_array[ati][cti] / 2.0;
                double mle = (double)mle_array[ati][cti] / 8.0 - 30.0;

                // nbd -4 to 6
                int inbd = (int)((nbd +  4.0) * 14.0 / 10.0 + 0.5);
                // spd 0 to 30
                int ispd = (int)((spd +  0.0) * 15.0 / 30.0 + 0.5);
                // dir 0 to 90
                int idir = (int)((dir +  0.0) * 15.0 / 90.0 + 0.5);
                // mle -10 to 0
                int imle = (int)((mle + 10.0) * 15.0 / 10.0 + 0.5);

                if (inbd < 0) inbd = 0;
                if (inbd > 14) inbd = 14;

                // hack in missing nbd element
                if (nbd_array[ati][cti] == 255) inbd = 15;

                if (ispd < 0) ispd = 0;
                if (ispd > 15) ispd = 15;
                if (idir < 0) idir = 0;
                if (idir > 15) idir = 15;
                if (imle < 0) imle = 0;
                if (imle > 15) imle = 15;

                float rr = (float)rain_rate[ati][cti] * 0.1;
                if (rr == 0.0)
                    counts[inbd][ispd][idir][imle][1]++;
                if (rr > rain_threshold)
                    counts[inbd][ispd][idir][imle][2]++;
                rain_rate_sum[inbd][ispd][idir][imle] += rr;
                counts[inbd][ispd][idir][imle][0]++;
            }
        }
    }

    //---------------------//
    // write conclass file //
    //---------------------//

    unsigned short conclass[16][16][16][16];
    for (int i = 0; i < 16; i++)
    {
        for (int j = 0; j < 16; j++)
        {
            for (int k = 0; k < 16; k++)
            {
                for (int l = 0; l < 16; l++)
                {
                    conclass[i][j][k][l] = 0;
                    if (counts[i][j][k][l][0] > 0)
                    {
                        double clear_prob = (double)counts[i][j][k][l][1] /
                            (double)counts[i][j][k][l][0];
                        double rain_prob = (double)counts[i][j][k][l][2] /
                            (double)counts[i][j][k][l][0];

                        int iclear = (int)(clear_prob * 200.0 + 0.5);
                        int irain = (int)(rain_prob * 200.0 + 0.5);
                        int iprob = irain * 256 + iclear;

/*
                        int iprob = (int)(prob_dif * 200.0 + 0.5);
                        if (iprob < 0) iprob = 0;
                        if (iprob > 200) iprob = 200;
*/

                        // avg rain rate
                        double avg_rain_rate = rain_rate_sum[i][j][k][l] /
                            (double)counts[i][j][k][l][0];
                        double rain_rate_dn = avg_rain_rate * 25.0;
                        if (rain_rate_dn < 0) rain_rate_dn = 0;
                        if (rain_rate_dn > 255) rain_rate_dn = 255;

                        conclass[i][j][k][l] = (unsigned short)iprob;
//                        conclass[i][j][k][l] = (unsigned short)rain_rate_dn;
                    }
                }
            }
        }
    }
    fwrite(conclass, sizeof(short), 16 * 16 * 16 * 16, conclass_ofp);

    //-------------//
    // close files //
    //-------------//

    fclose(conclass_ofp);

    return (0);
}
