//==============================================================//
// Copyright (C) 1998-1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    gmf_plot
//
// SYNOPSIS
//    gmf_plot [ -ips ] <gmf_file> [output_base]
//
// DESCRIPTION
//    Generates plots about a geophysical model function.
//
// OPTIONS
//    The following options are supported:
//      [ -i ]  Interactive.
//      [ -p ]  Indicates the gmf file is polarimetric.
//      [ -s ]  Don't show query for interactive (silent).
//
// OPERANDS
//    The following operands are supported:
//      <gmf_file>     The geophysical model function file.
//      [output_base]  The base name for output files.  This is not
//                       needed if the interactive option is
//                       specified.
//
// EXAMPLES
//    An example of a command line is:
//      % gmf_plot -p pkmod.dat pkmod.plot
//      % gmf_plot -i nscat2.dat
//      % gmf_plot -is nscat2.dat < data.in > data.out
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

#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include "Meas.h"
#include "Misc.h"
#include "Beam.h"
#include "GMF.h"
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "AngleInterval.h"
#include "Wind.h"

// these are needed because Meas (which this program needs) uses L2AHdf
// (which this program doesn't need)
#include "Tracking.h"
#include "Tracking.C"

//-----------//
// TEMPLATES //
//-----------//

template class List<Meas>;
template class List<WindVectorPlus>;
template class List<MeasSpot>;
template class List<OffsetList>;
template class List<OrbitState>;
template class List<long>;
template class BufferedList<OrbitState>;
template class List<EarthPosition>;
template class List<AngleInterval>;
template class List<AngleInterval>;

// these are needed because Meas (which this program needs) uses L2AHdf
// (which this program doesn't need)
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;

//-----------//
// CONSTANTS //
//-----------//

#define QUOTES     '"'
#define OPTSTRING  "ips"

#define POLS  2
#define INCS  67
#define SPDS  51

#define INC_STEP  2

//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

int  gen_plot(const char* output_base, GMF* gmf, Meas::MeasTypeE meas_type,
         float inc, int use_log);
int  interact(GMF* gmf, int silent);
int  string_query(FILE* fp, char* query, const char* choices[],
         int* current_idx, int silent);
int  float_query(FILE* fp, char* query, float* current_value, int silent);

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -ips ]", "<gmf_file>", "[output_base]", 0 };

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

    int opt_pol = 0;
    int opt_interactive = 0;
    int opt_silent = 0;

    //------------------------//
    // parse the command line //
    //------------------------//

    const char* command = no_path(argv[0]);

    extern int optind;
    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
        case 'i':
            opt_interactive = 1;
            break;
        case 'p':
            opt_pol = 1;
            break;
        case 's':
            opt_silent = 1;
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    const char* gmf_file = NULL;
    const char* output_base = NULL;
    if (opt_interactive)
    {
        if (argc != optind + 1)
            usage(command, usage_array, 1);
        gmf_file = argv[optind++];
    }
    else
    {
        if (argc != optind + 2)
            usage(command, usage_array, 1);
        gmf_file = argv[optind++];
        output_base = argv[optind++];
    }

    //-----------------//
    // read in the gmf //
    //-----------------//

    GMF gmf;
    if (opt_pol)
    {
        if (! gmf.ReadPolarimetric(gmf_file))
        {
            fprintf(stderr, "%s: error reading polarimetric GMF file %s\n",
                command, gmf_file);
            exit(1);
        }
    }
    else
    {
        if (! gmf.ReadOldStyle(gmf_file))
        {
            fprintf(stderr, "%s: error reading GMF file %s\n", command,
                gmf_file);
            exit(1);
        }
    }

    if (opt_interactive)
    {
        //--------------------------//
        // do the interactive thing //
        //--------------------------//

        interact(&gmf, opt_silent);
        printf("\n");
    }
    else
    {
        //--------------------//
        // generate the plots //
        //--------------------//

        gen_plot(output_base, &gmf, Meas::VV_MEAS_TYPE, 46.0, 1);
        gen_plot(output_base, &gmf, Meas::HH_MEAS_TYPE, 46.0, 1);

        gen_plot(output_base, &gmf, Meas::VV_MEAS_TYPE, 54.0, 1);
        gen_plot(output_base, &gmf, Meas::HH_MEAS_TYPE, 54.0, 1);

        if (opt_pol)
        {
            gen_plot(output_base, &gmf, Meas::VH_MEAS_TYPE, 46.0, 1);
            gen_plot(output_base, &gmf, Meas::HV_MEAS_TYPE, 46.0, 1);
            gen_plot(output_base, &gmf, Meas::VV_HV_CORR_MEAS_TYPE, 46.0, 0);
            gen_plot(output_base, &gmf, Meas::HH_VH_CORR_MEAS_TYPE, 46.0, 0);

            gen_plot(output_base, &gmf, Meas::VH_MEAS_TYPE, 54.0, 1);
            gen_plot(output_base, &gmf, Meas::HV_MEAS_TYPE, 54.0, 1);
            gen_plot(output_base, &gmf, Meas::VV_HV_CORR_MEAS_TYPE, 54.0, 0);
            gen_plot(output_base, &gmf, Meas::HH_VH_CORR_MEAS_TYPE, 54.0, 0);
        }
    }

    return (0);
}

//----------//
// gen_plot //
//----------//

int
gen_plot(
    const char*      output_base,
    GMF*             gmf,
    Meas::MeasTypeE  meas_type,
    float            inc,
    int              use_log)
{
    float c_speed_table[] = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 10.0,
        15.0, 20.0, 30.0, 40.0, 50.0, -1.0 };
    float r_speed_table[] = { 2.0, 4.0, 6.0, 8.0, 10.0, 12.0, 14.0, 16.0, 18.0,
        20.0, 22.0, 24.0, 26.0, 28.0, 30.0, -1.0 };

    char filename[1024];
    int inc_int = (int)(inc + 0.5);
    sprintf(filename, "%s.%02d.%s", output_base, inc_int,
        meas_type_map[(int)meas_type]);

    FILE* ofp = fopen(filename, "w");
    if (ofp == NULL)
        return(0);

    fprintf(ofp, "@ legend on\n");
    fprintf(ofp, "@ title %c%s,  %d degree incidence angle%c\n",
        QUOTES, meas_type_map[(int)meas_type], inc_int, QUOTES);
    int legend_string = 0;

    float* speed_table;
    switch (meas_type)
    {
    case Meas::VV_HV_CORR_MEAS_TYPE:
    case Meas::HH_VH_CORR_MEAS_TYPE:
        speed_table = r_speed_table;
        break;
    default:
        speed_table = c_speed_table;
        break;
    }

    for (int spd_idx = 0; speed_table[spd_idx] > 0; spd_idx++)
    {
        float spd = speed_table[spd_idx];
        fprintf(ofp, "@ legend string %d %c%g m/s%c\n", legend_string, QUOTES,
            spd, QUOTES);
        fprintf(ofp, "@ s%d linewidth 3\n", legend_string);
        legend_string++;
        for (int chi_d = 0; chi_d <= 360; chi_d++)
        {
            float chi = (float)chi_d * dtr;
            float value;
            gmf->GetInterpolatedValue(meas_type, inc * dtr, spd, chi, &value);
            if (use_log)
                value = 10.0 * log10(value);

            fprintf(ofp, "%g %g\n", chi * rtd, value);
        }
        if (speed_table[spd_idx + 1] > 0.0)
            fprintf(ofp, "&\n");
    }
    fclose(ofp);
    return(1);
}

//----------//
// interact //
//----------//

int
interact(
    GMF*  gmf,
    int   silent)
{
    int meas_type = (int)Meas::VV_MEAS_TYPE;
    float incidence_angle = 50.0;
    float speed = 7.0;
    float dir = 45.0;
    float mdir = 45.0;
    do
    {
        if (! string_query(stdin, "Measurement type", meas_type_map,
            &meas_type, silent))
        {
            return(1);
        }
        if (! float_query(stdin, "Incidence angle", &incidence_angle, silent))
        {
            return(1);
        }
        if (! float_query(stdin, "Wind speed", &speed, silent))
        {
            return(1);
        }
        if (! float_query(stdin, "Wind direction CCW from E", &dir, silent))
        {
            return(1);
        }
        if (! float_query(stdin, "Measurement direction CCW from E", &mdir,
            silent))
        {
            return(1);
        }
        float chi = (dir - mdir) * dtr + pi;

        float value;
        if (! gmf->GetInterpolatedValue((Meas::MeasTypeE)meas_type,
            incidence_angle * dtr, speed, chi, &value))
        {
            printf("Error looking up in GMF.\n");
        }
        else
        {
            printf("%g", value);
            if (value > 0.0)
            {
                double db_val = 10.0 * log10(value);
                printf(" (%g dB)", db_val);
            }
            printf("\n\n");
        }

    } while (1);

    return(1);
}

//--------------//
// string_query //
//--------------//
// 1 = got value
// 0 = done

#define MAX_STRING_LENGTH  1024

int
string_query(
    FILE*        fp,
    char*        query,
    const char*  choices[],
    int*         current_idx,
    int          silent)
{
    char string[MAX_STRING_LENGTH];
    do
    {
        if (! silent)
            printf("%s (%s): ", query, choices[*current_idx]);

        if (fgets(string, MAX_STRING_LENGTH, fp) == NULL)
            return(0);    // ctrl D ?

        if (strlen(string) == 1)
            return(1);    // use default

        for (int idx = 0; choices[idx] != NULL; idx++)
        {
            if (strncasecmp(string, choices[idx], strlen(string) - 1) == 0)
            {
                *current_idx = idx;
                return(1);    // got valid value
            }
        }

        printf("Input must be one of:");
        for (int idx = 0; choices[idx] != NULL; idx++)
        {
            printf(" %s", choices[idx]);
        }
        printf("\n");
    } while (1);
    return(0);
}

//-------------//
// float_query //
//-------------//
// 1 = got value
// 0 = done

int
float_query(
    FILE*   fp,
    char*   query,
    float*  current_value,
    int     silent)
{
    char string[MAX_STRING_LENGTH];
    do
    {
        if (! silent)
            printf("%s (%g): ", query, *current_value);

        if (fgets(string, MAX_STRING_LENGTH, fp) == NULL)
            return(0);    // ctrl D ?

        if (strlen(string) == 1)
            return(1);    // use default

        float value;
        if (sscanf(string, " %g", &value) == 1)
        {
            *current_value = value;
            return(1);    // got valid value
        }

        printf("Input must be a number.\n");
    } while (1);
    return(0);
}
