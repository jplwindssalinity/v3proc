//=========================================================//
// Copyright (C) 2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    super_gmf
//
// SYNOPSIS
//    super_gmf <gmf_file> <output_base>
//
// DESCRIPTION
//    Generates plots about a geophysical model function. This
//    program requires user input when running.
//
// OPTIONS
//    None.
//
// OPERANDS
//    <gmf_file>     The geophysical model function file.
//    <output_base>  The base name for output files.
//
// EXAMPLES
//    An example of a command line is:
//      % super_gmf sass2.dat sass2
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
//    James N. Huddleston <mailto:James.N.Huddleston@jpl.nasa.gov>
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
#include "GMF.h"
#include <unistd.h>
#include "List.h"
#include "List.C"
#include "AngleInterval.h"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Tracking.h"
#include "Tracking.C"

//-----------//
// TEMPLATES //
//-----------//
 
template class List<WindVectorPlus>;
template class List<Meas>;
template class List<AngleInterval>;
template class List<MeasSpot>;
template class List<EarthPosition>;
template class List<long>;
template class List<OffsetList>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING  ""
#define QUOTES     '"'

/*

#define POLS       2
#define INCS       67
#define SPDS       51

#define INC_STEP   2
*/

//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

Meas::MeasTypeE  query_pol();
float            query_inc();
float            query_spd();

int a0_vs_wind_speed(GMF* gmf, const char* output_base);
int s0_vs_azimuth_angle(GMF* gmf, const char* output_base);

/*
int term_vs_inc(const char* command, const char* gmf_file,
    double term[POLS][INCS][SPDS], char* name, unsigned char flag);
int term_vs_spd(const char* command, const char* gmf_file,
    double term[POLS][INCS][SPDS], char* name, unsigned char flag);
*/

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "<gmf_file>", "<output_base>", 0 };

/*
const char* pol_map[] = { "V", "H" };
*/

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

    const char* command = no_path(argv[0]);
    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1) {
        switch(c) {
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    if (argc != optind + 2) {
        usage(command, usage_array, 1);
    }
    char* gmf_file = argv[optind++];
    char* output_base = argv[optind++];

/*
    double a0[POLS][INCS][SPDS];
    double a1[POLS][INCS][SPDS];
    double a1_phase[POLS][INCS][SPDS];
    double a2[POLS][INCS][SPDS];
    double a2_phase[POLS][INCS][SPDS];
    double a3[POLS][INCS][SPDS];
    double a3_phase[POLS][INCS][SPDS];
    double a4[POLS][INCS][SPDS];
    double a4_phase[POLS][INCS][SPDS];
*/

    //-----------------//
    // read in the gmf //
    //-----------------//

    GMF gmf;
    if (! gmf.ReadOldStyle(gmf_file)) {
        fprintf(stderr, "%s: error reading gmf file %s\n",
            command, gmf_file);
        exit(1);
    }

    //-------------------------------//
    // determine what the user wants //
    //-------------------------------//

    printf("Generate a plot of:\n");
    printf(" [1] A0 versus wind speed\n");
    printf(" [2] Sigma-0 versus azimuth angle\n");

    int choice;
    if (scanf("%d", &choice) != 1) {
        exit(0);
    }

    switch (choice) {
    case 1:
        a0_vs_wind_speed(&gmf, output_base);
        break;
    case 2:
        s0_vs_azimuth_angle(&gmf, output_base);
        break;
    default:
        break;
    }

    return(0);
}

//------------------//
// a0_vs_wind_speed //
//------------------//

int
a0_vs_wind_speed(
    GMF*         gmf,
    const char*  output_base)
{
    // polarization
    Meas::MeasTypeE meas_type = query_pol();

    // incidence angle
    float inc = query_inc();

    // speed range
    printf("Min speed (in m/s): ");
    float min_speed;
    scanf(" %f", &min_speed);
    printf("Max speed (in m/s): ");
    float max_speed;
    scanf(" %f", &max_speed);
    printf("Speed step (in m/s): ");
    float speed_step;
    scanf(" %f", &speed_step);

    // filename
    printf("Filename: ");
    char filename[1024];
    scanf("%s", filename);

    FILE* ofp = fopen(filename, "w");
    fprintf(ofp, "@ xaxis label %cWind Speed(m/s)%c\n", QUOTES, QUOTES);
    fprintf(ofp, "@ yaxis label %cA0 (dB)%c\n", QUOTES, QUOTES);

    float a0, a1, a1p, a2, a2p, a3, a3p, a4, a4p;
    for (float spd = min_speed; spd < max_speed + speed_step / 2.0;
    spd += speed_step) {
        if (! gmf->GetCoefs(meas_type, inc, spd, &a0, &a1, &a1p, &a2, &a2p, &a3,
        &a3p, &a4, &a4p)) {
            fprintf(stderr, "error getting coefficients\n");
            exit(1);
        }
        fprintf(ofp, "%g %g\n", spd, 10.0 * log10(a0));
    }

    fclose(ofp);

    return(0);
}

//---------------------//
// s0_vs_azimuth_angle //
//---------------------//

int
s0_vs_azimuth_angle(
    GMF*         gmf,
    const char*  output_base)
{
    Meas::MeasTypeE meas_type = query_pol();
    float inc = query_inc();
    float spd = query_spd();

    // filename
    printf("Filename: ");
    char filename[1024];
    scanf("%s", filename);

    FILE* ofp = fopen(filename, "w");
    fprintf(ofp, "@ xaxis label %cAzimuth Angle (deg)%c\n", QUOTES, QUOTES);
    fprintf(ofp, "@ yaxis label %cSigma-0 (dB)%c\n", QUOTES, QUOTES);

    for (float az = 0.0; az <= 360; az += 5) {
        float chi = az * dtr;
        float s0;
        if (! gmf->GetInterpolatedValue(meas_type, inc, spd, chi, &s0)) {
            fprintf(stderr, "error getting sigma-0 value\n");
            exit(1);
        }
        fprintf(ofp, "%g %g\n", az, 10.0 * log10(s0));
    }

    fclose(ofp);

    return(0);
}


/*
        //---------------------------------------//
        // transform into A0, a1, a2, a3, and a4 //
        //---------------------------------------//

        for (int pol_idx = 0; pol_idx < POLS; pol_idx++)
        {
            PolE pol = (PolE) pol_idx;
            Meas::MeasTypeE met = PolToMeasType(pol);
            for (int inc = 16; inc < INCS; inc += INC_STEP)
            {
                for (int spd = 1; spd < SPDS; spd += 1)
                {
                    double dspd = (double)spd;
                    float xa0, xa1, xa1p, xa2, xa2p, xa3, xa3p, xa4, xa4p;
                    gmf.GetCoefs(met, (float)inc * dtr, dspd, &xa0, &xa1,
                        &xa1p, &xa2, &xa2p, &xa3, &xa3p, &xa4, &xa4p);
                    a0[pol][inc][spd] = xa0;
                    a1[pol][inc][spd] = xa1 / xa0;
                    a1_phase[pol][inc][spd] = fmod(xa1p * RTD + 360.0, 360.0);
                    if (a1_phase[pol][inc][spd] > 270.0)
                        a1_phase[pol][inc][spd] -= 360.0;
                    a2[pol][inc][spd] = xa2 / xa0;
                    a2_phase[pol][inc][spd] = fmod(xa2p * RTD + 360.0, 360.0);
                    if (a2_phase[pol][inc][spd] > 270.0)
                        a2_phase[pol][inc][spd] -= 360.0;
                    a3[pol][inc][spd] = xa3 / xa0;
                    a3_phase[pol][inc][spd] = fmod(xa3p * RTD + 360.0, 360.0);
                    if (a3_phase[pol][inc][spd] > 270.0)
                        a3_phase[pol][inc][spd] -= 360.0;
                    a4[pol][inc][spd] = xa4 / xa0;
                    a4_phase[pol][inc][spd] = fmod(xa4p * RTD + 360.0, 360.0);
                    if (a4_phase[pol][inc][spd] > 270.0)
                        a4_phase[pol][inc][spd] -= 360.0;
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
            read(ifd, a2_phase, size) != size ||
            read(ifd, a3, size) != size ||
            read(ifd, a3_phase, size) != size ||
            read(ifd, a4, size) != size ||
            read(ifd, a4_phase, size) != size)
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
            write(ofd, a2_phase, size) != size ||
            write(ofd, a3, size) != size ||
            write(ofd, a3_phase, size) != size ||
            write(ofd, a4, size) != size ||
            write(ofd, a4_phase, size) != size)
        {
            fprintf(stderr, "%s: error writing coefficient file %s\n",
                command, write_coef);
            exit(1);
        }
        close(ofd);
    }

    term_vs_inc(command, base, a0, "a0", 1);
    term_vs_inc(command, base, a1, "a1", 0);
    term_vs_inc(command, base, a1_phase, "a1p", 0);
    term_vs_inc(command, base, a2, "a2", 0);
    term_vs_inc(command, base, a2_phase, "a2p", 0);
    term_vs_inc(command, base, a3, "a3", 0);
    term_vs_inc(command, base, a3_phase, "a3p", 0);
    term_vs_inc(command, base, a4, "a4", 0);
    term_vs_inc(command, base, a4_phase, "a4p", 0);

    term_vs_spd(command, base, a0, "a0", 1);
    term_vs_spd(command, base, a1, "a1", 0);
    term_vs_spd(command, base, a1_phase, "a1p", 0);
    term_vs_spd(command, base, a2, "a2", 0);
    term_vs_spd(command, base, a2_phase, "a2p", 0);
    term_vs_spd(command, base, a3, "a3", 0);
    term_vs_spd(command, base, a3_phase, "a3p", 0);
    term_vs_spd(command, base, a4, "a4", 0);
    term_vs_spd(command, base, a4_phase, "a4p", 0);

    return (0);
}

//-------------//
// term_vs_inc //
//-------------//

int
term_vs_inc(
    const char*    command,
    const char*    gmf_file,
    double    term[POLS][INCS][SPDS],
    char*    name,
    unsigned char    flag)
{
    char filename[1024];
    FILE* ofp = NULL;

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
                if (flag && term[pol][inc][spds[spd_idx]] > 0.0)
                    fprintf(ofp, "%d %g\n", inc,
                        10.0 * log10(term[pol][inc][spds[spd_idx]]));
                else if (! flag)
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
    const char*    command,
    const char*    gmf_file,
    double    term[POLS][INCS][SPDS],
    char*    name,
    unsigned char    flag)
{
    char filename[1024];
    FILE* ofp = NULL;

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

        int incs[] = { 20, 24, 28, 32, 36, 40, 44, 48, 52, 54, 56, 60, 64, -1 };
        int inc_idx = 0;
        while (incs[inc_idx] > 0)
        {
            fprintf(ofp, "@ legend string %d %c%d deg%c\n", inc_idx, QUOTES,
                incs[inc_idx], QUOTES);
            for (int spd = 1; spd < SPDS; spd++)
            {
                if (flag && term[pol][incs[inc_idx]][spd] > 0.0)
                    fprintf(ofp, "%d %g\n", spd,
                        10.0 * log10(term[pol][incs[inc_idx]][spd]));
                else if (! flag)
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
*/

Meas::MeasTypeE
query_pol()
{
    Meas::MeasTypeE meas_type = Meas::NONE;
    while (meas_type == Meas::NONE) {
        printf("Polarization (H, V): ");
        char pol_char;
        scanf(" %c", &pol_char);
        if (pol_char == 'V' || pol_char == 'v') {
            meas_type = Meas::VV_MEAS_TYPE;
        } else if (pol_char == 'H' || pol_char == 'h') {
            meas_type = Meas::HH_MEAS_TYPE;
        } else {
            printf("Invalid entry, try again.\n");
        }
    }
    return meas_type;
}

float
query_inc()
{
    printf("Incidence angle (in degrees): ");
    float inc;
    scanf(" %f", &inc);
    inc *= dtr;
    return inc;
}

float
query_spd()
{
    printf("Speed (in m/s): ");
    float spd;
    scanf(" %f", &spd);
    return spd;
}
