//=========================================================//
// Copyright (C) 1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    gmf_avg
//
// SYNOPSIS
//    gmf_avg <gmf_file>
//
// DESCRIPTION
//    Generates a table of sigma_0 (db) vs. incidence angle
//
// OPTIONS
//    None.
//
// OPERANDS
//    <gmf_file>  The geophysical model function file.
//
// EXAMPLES
//    An example of a command line is:
//      % gmf_avg nscat2.dat
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
#include "Misc.h"
#include "Beam.h"
#include "GMF.h"
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "AngleInterval.h"
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
template class TrackerBase<unsigned short>;
template class TrackerBase<unsigned char>;

//-----------//
// CONSTANTS //
//-----------//

float pdf[] = { 0.0, 0.0142402, 0.0281764, 0.0415154, 0.053985, 0.0653434,
    0.0753864, 0.083955, 0.090936, 0.096267, 0.099935, 0.101974, 0.102459,
    0.101501, 0.099245, 0.095857, 0.091515, 0.086408, 0.080725, 0.074646,
    0.068344, 0.061972, 0.055666, 0.049544, 0.043698, 0.038201, 0.033104,
    0.02844, 0.024226, 0.020463, 0.017141, 0.01424, 0.011733, 0.00959,
    0.007775, 0.006253, 0.004989, 0.003949, 0.003102, 0.002417, 0.001868,
    0.001433, 0.001092, 0.000824, 0.000618, 0.00046, 0.000339, 0.000249,
    0.000181, 0.00013, 9.3e-05, 6.6e-05, 4.7e-05, 3.3e-05, 2.3e-05, 1.6e-05,
    1e-05, 7e-06, 5e-06, 3e-06 };

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

const char* usage_array[] = { "<gmf_file>", 0 };

const char* pol_map[] = { "V", "H" };

//--------------//
// MAIN PROGRAM //
//--------------//

int
main(
    int        argc,
    char*    argv[])
{
    //------------------------//
    // parse the command line //
    //------------------------//

    const char* command = no_path(argv[0]);

    if (argc != 2)
        usage(command, usage_array, 1);

    const char* gmf_file = argv[1];

    //-----------------//
    // read in the gmf //
    //-----------------//

    GMF gmf;
    if (! gmf.ReadOldStyle(gmf_file))
    {
        fprintf(stderr, "%s: error reading gmf file %s\n",
            command, gmf_file);
        exit(1);
    }

    for (int pol_idx = 0; pol_idx < 2; pol_idx++)
    {
        printf("# polarization = %s\n", pol_map[pol_idx]);
        for (int inc_idx = 18; inc_idx < 64; inc_idx += 2)
        {
            float inc = (float)inc_idx * dtr;
            Meas::MeasTypeE mt;
            if (pol_idx == V_POL)
                mt = Meas::VV_MEAS_TYPE;
            else if (pol_idx == H_POL)
                mt = Meas::HH_MEAS_TYPE;
            else
                mt = Meas::NONE;

            float sum = 0.0;
            float pdf_sum = 0.0;
            for (int spd_idx = 0; spd_idx <= 30; spd_idx += 1)
            {
                float spd = (float)spd_idx;
                int pdf_idx = spd_idx * 2;
                for (int dir_idx = 0; dir_idx < 72; dir_idx += 1)
                {
                    float dir = (float)dir_idx * 5.0 * dtr;
                    float value;
                    gmf.GetNearestValue(mt, inc, spd, dir, &value);
                    sum += value * pdf[pdf_idx];
                    pdf_sum += pdf[pdf_idx];
                }
            }
            float avg = sum / pdf_sum;
            printf("%g %g\n", inc * rtd, avg);
        }
        printf("&\n");
    }

    return (0);
}
