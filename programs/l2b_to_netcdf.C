//==============================================================//
// Copyright (C) 1998-2010, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    l2b_to_netcdf
//
// SYNOPSIS
//    l2b_to_netcdf <input_file> <output_file> 
//
// DESCRIPTION
//    Reads frames from a L2B file and
//    writes them to an NetCDF file
//
// OPTIONS
//
// AUTHOR
//    Thomas Werne
//    werne@jpl.nasa.gov
//----------------------------------------------------------------------

//-----------------------//
// Configuration Control //
//-----------------------//

static const char rcs_id[] =
    "@(#) $Id$";

//----------//
// INCLUDES //
//----------//

#include </usr/include/netcdf.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "Misc.h"
#include "L2B.h"
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Tracking.h"
#include "Tracking.C"


//--------//
// MACROS //
//--------//
#define STRINGIFY(x) #x
#define S(x) STRINGIFY(x)

#define ERR(call) \
    if (call) { \
        perror("Error in " __FILE__ " @ " S(__LINE__)); \
        exit(EXIT_FAILURE); \
    }
    
#define NCERR(call) \
{ \
    int error; \
    if ((error = call) != NC_NOERR) { \
        fprintf(stderr, "Error %d in %s @ %d: %s\n", error, \
                __FILE__, __LINE__, nc_strerror(error)); \
        exit(EXIT_FAILURE); \
    } \
}

enum {
    USABLE_MASK = 0x01,
    RAIN_MASK   = 0x02,
    BEAM_MASK   = 0x04,
    LAND_MASK   = 0x10,
    ICE_MASK    = 0x20,
    VALID_MASK  = 0x40
};

//-----------//
// TEMPLATES //
//-----------//

class AngleInterval;
template class List<AngleInterval>;
template class List<EarthPosition>;
template class List<WindVectorPlus>;
template class List<long>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;


typedef enum {
    LATITUDE = 0,
    LONGITUDE,
    SEL_SPEED,
    SEL_DIRECTION,
    RAIN_IMPACT,
    DIVERGENCE,
    CURL,
    WIND_STRESS,
    WIND_STRESS_DIVERGENCE,
    WIND_STRESS_CURL,
    TIME,
    FLAGS,

    first_extended_variable,

    SEL_OBJ = first_extended_variable,
    NUDGE_SPEED,
    NUDGE_DIRECTION,
    AMBIG_SPEED,
    AMBIG_DIRECTION,
    AMBIG_OBJ,
    N_IN_FORE,
    N_IN_AFT,
    N_OUT_FORE,
    N_OUT_AFT,
    num_variables
} variables;

typedef enum {
    FILL_VALUE = 0,
    VALID_MIN,
    VALID_MAX,
    LONG_NAME,
    num_standard_attrs
} standard_attrs;

typedef struct {
    const char *command;
    const char *input_file;
    const char *output_file;
    char extended;
} l2b_to_netcdf_config;

typedef struct {
    const char *name;
    int type;
    union {
        float f;
        int i;
        const char *s;
        unsigned char c;
    } value;
} attribute;

typedef struct {
    const char *name;
    int type;
    int ndims;
    int *dims;
    int id;
    int nattrs;
    attribute *attrs;
} netcdf_variable;


int parse_commandline(int argc, char **argv, l2b_to_netcdf_config *config);


//--------------//
// MAIN PROGRAM //
//--------------//

int
main(
    int    argc,
    char*  argv[])
{
    

    l2b_to_netcdf_config config;

    //------------------------//
    // parse the command line //
    //------------------------//

    if (parse_commandline(argc, argv, &config) != 0) {
        return -1;
    }

    //-------------------//
    // create L2B object //
    //-------------------//

    L2B l2b;

    //---------------------//
    // open the input file //
    //---------------------//

    if (! l2b.OpenForReading(config.input_file))
    {
        fprintf(stderr, "%s: error opening input file %s\n", config.command,
            config.input_file);
        exit(1);
    }

    //---------------------//
    // copy desired frames //
    //---------------------//

    if (! l2b.ReadHeader())
    {
        fprintf(stderr, "%s: error reading from input file %s\n", config.command,
            config.input_file);
        exit(1);
    }
    if (! l2b.ReadDataRec())
    {
        fprintf(stderr, "%s: error reading from input file %s\n", config.command,
            config.input_file);
        exit(1);
    }


    /********************************************
     * Build NetCDF DB                          *
     *******************************************/

    /* Here we're going to store a set of attributes, and a swath of
     * wind vector cell ambiguities.  The format of the swath is:
     * a 2-dimensional matrix (cross-track x along_track); each 
     * point is a variable length array (ambiguities); each element
     * of the array is a (u,v) two-float compounddata type.
     */
    int ncid, tmp;

    int max_ambiguities = l2b.frame.swath.GetMaxAmbiguityCount();
    int cross_track_dim_id, along_track_dim_id, ambiguities_dim_id;

    // Initialize the NetCDF DB
    NCERR(nc_create(config.output_file, (NC_NETCDF4 | NC_NOFILL) & 
                ~NC_CLASSIC_MODEL, &ncid));

    // Create the data dimensions
    NCERR(nc_def_dim(ncid, "along_track", 
                (size_t)l2b.frame.swath.GetAlongTrackBins(), 
                &along_track_dim_id));
    NCERR(nc_def_dim(ncid, "cross_track", 
                (size_t)l2b.frame.swath.GetCrossTrackBins(), 
                &cross_track_dim_id));
    if (config.extended) {
        NCERR(nc_def_dim(ncid, "ambiguities",
                (size_t)max_ambiguities, &ambiguities_dim_id));
    }

    /*************************************************************
     *
     * Declare the NetCDF variables
     *
     ************************************************************/

    int dimensions[3];

    /* We're being sneaky here.  We can pass dimensions to both two-
     * and three-dimensional variables, since they all use the same
     * first two dimensions.
     */
    dimensions[0] = along_track_dim_id;
    dimensions[1] = cross_track_dim_id;
    dimensions[2] = ambiguities_dim_id;

    netcdf_variable varlist[num_variables];
   
    varlist[LATITUDE].name  = "latitude";
    varlist[LATITUDE].type  = NC_FLOAT;
    varlist[LATITUDE].ndims = 2;
    varlist[LATITUDE].dims = dimensions; 
    varlist[LATITUDE].nattrs = num_standard_attrs + 2;
    ERR((varlist[LATITUDE].attrs = (typeof varlist[LATITUDE].attrs)
                malloc(varlist[LATITUDE].nattrs * 
                    (sizeof *varlist[LATITUDE].attrs))) == NULL);

    varlist[LATITUDE].attrs[FILL_VALUE].name = "FillValue";
    varlist[LATITUDE].attrs[FILL_VALUE].type = varlist[LATITUDE].type;
    varlist[LATITUDE].attrs[FILL_VALUE].value.f = 0.0f;
    varlist[LATITUDE].attrs[VALID_MIN].name = "valid_min";
    varlist[LATITUDE].attrs[VALID_MIN].type = varlist[LATITUDE].type;
    varlist[LATITUDE].attrs[VALID_MIN].value.f = -90.0f;
    varlist[LATITUDE].attrs[VALID_MAX].name = "valid_max";
    varlist[LATITUDE].attrs[VALID_MAX].type = varlist[LATITUDE].type;
    varlist[LATITUDE].attrs[VALID_MAX].value.f =  90.0f;
    varlist[LATITUDE].attrs[LONG_NAME].name = "long_name";
    varlist[LATITUDE].attrs[LONG_NAME].type = NC_CHAR;
    varlist[LATITUDE].attrs[LONG_NAME].value.s = "latitude";
    varlist[LATITUDE].attrs[num_standard_attrs].name = "units";
    varlist[LATITUDE].attrs[num_standard_attrs].type = NC_CHAR;
    varlist[LATITUDE].attrs[num_standard_attrs].value.s = "degrees_north";
    varlist[LATITUDE].attrs[num_standard_attrs + 1].name = "scale_factor";
    varlist[LATITUDE].attrs[num_standard_attrs + 1].type = NC_FLOAT;
    varlist[LATITUDE].attrs[num_standard_attrs + 1].value.f = 1.0f;

    varlist[LONGITUDE].name  = "longitude";
    varlist[LONGITUDE].type  = NC_FLOAT;
    varlist[LONGITUDE].ndims = 2;
    varlist[LONGITUDE].dims = dimensions; 
    varlist[LONGITUDE].nattrs = num_standard_attrs + 2;
    ERR((varlist[LONGITUDE].attrs = (typeof varlist[LONGITUDE].attrs)
                malloc(varlist[LONGITUDE].nattrs * 
                    (sizeof *varlist[LONGITUDE].attrs))) == NULL);

    varlist[LONGITUDE].attrs[FILL_VALUE].name = "FillValue";
    varlist[LONGITUDE].attrs[FILL_VALUE].type = varlist[LONGITUDE].type;
    varlist[LONGITUDE].attrs[FILL_VALUE].value.f = 0.0f;
    varlist[LONGITUDE].attrs[VALID_MIN].name = "valid_min";
    varlist[LONGITUDE].attrs[VALID_MIN].type = varlist[LONGITUDE].type;
    varlist[LONGITUDE].attrs[VALID_MIN].value.f = 0.0f;
    varlist[LONGITUDE].attrs[VALID_MAX].name = "valid_max";
    varlist[LONGITUDE].attrs[VALID_MAX].type = varlist[LONGITUDE].type;
    varlist[LONGITUDE].attrs[VALID_MAX].value.f = 360.0f;
    varlist[LONGITUDE].attrs[LONG_NAME].name = "long_name";
    varlist[LONGITUDE].attrs[LONG_NAME].type = NC_CHAR;
    varlist[LONGITUDE].attrs[LONG_NAME].value.s = "latitude";
    varlist[LONGITUDE].attrs[num_standard_attrs].name = "units";
    varlist[LONGITUDE].attrs[num_standard_attrs].type = NC_CHAR;
    varlist[LONGITUDE].attrs[num_standard_attrs].value.s = "degrees_east";
    varlist[LONGITUDE].attrs[num_standard_attrs + 1].name = "scale_factor";
    varlist[LONGITUDE].attrs[num_standard_attrs + 1].type = NC_FLOAT;
    varlist[LONGITUDE].attrs[num_standard_attrs + 1].value.f = 1.0f;

    varlist[SEL_SPEED].name  = "wind_speed";
    varlist[SEL_SPEED].type  = NC_FLOAT;
    varlist[SEL_SPEED].ndims = 2;
    varlist[SEL_SPEED].dims = dimensions; 
    varlist[SEL_SPEED].nattrs = num_standard_attrs + 2;
    ERR((varlist[SEL_SPEED].attrs = (typeof varlist[SEL_SPEED].attrs)
                malloc(varlist[SEL_SPEED].nattrs * 
                    (sizeof *varlist[SEL_SPEED].attrs))) == NULL);

    varlist[SEL_SPEED].attrs[FILL_VALUE].name = "FillValue";
    varlist[SEL_SPEED].attrs[FILL_VALUE].type = varlist[SEL_SPEED].type;
    varlist[SEL_SPEED].attrs[FILL_VALUE].value.f = 0.0f;
    varlist[SEL_SPEED].attrs[VALID_MIN].name = "valid_min";
    varlist[SEL_SPEED].attrs[VALID_MIN].type = varlist[SEL_SPEED].type;
    varlist[SEL_SPEED].attrs[VALID_MIN].value.f = 0.0f;
    varlist[SEL_SPEED].attrs[VALID_MAX].name = "valid_max";
    varlist[SEL_SPEED].attrs[VALID_MAX].type = varlist[SEL_SPEED].type;
    varlist[SEL_SPEED].attrs[VALID_MAX].value.f = HUGE_VAL;
    varlist[SEL_SPEED].attrs[LONG_NAME].name = "long_name";
    varlist[SEL_SPEED].attrs[LONG_NAME].type = NC_CHAR;
    varlist[SEL_SPEED].attrs[LONG_NAME].value.s = "wind_speed";
    varlist[SEL_SPEED].attrs[num_standard_attrs].name = "units";
    varlist[SEL_SPEED].attrs[num_standard_attrs].type = NC_CHAR;
    varlist[SEL_SPEED].attrs[num_standard_attrs].value.s = "m s-1";
    varlist[SEL_SPEED].attrs[num_standard_attrs + 1].name = "scale_factor";
    varlist[SEL_SPEED].attrs[num_standard_attrs + 1].type = NC_FLOAT;
    varlist[SEL_SPEED].attrs[num_standard_attrs + 1].value.f = 1.0f;
    
    varlist[SEL_DIRECTION].name  = "wind_to_direction";
    varlist[SEL_DIRECTION].type  = NC_FLOAT;
    varlist[SEL_DIRECTION].ndims = 2;
    varlist[SEL_DIRECTION].dims = dimensions; 
    varlist[SEL_DIRECTION].nattrs = num_standard_attrs + 2;
    ERR((varlist[SEL_DIRECTION].attrs = (typeof varlist[SEL_DIRECTION].attrs)
                malloc(varlist[SEL_DIRECTION].nattrs * 
                    (sizeof *varlist[SEL_DIRECTION].attrs))) == NULL);

    varlist[SEL_DIRECTION].attrs[FILL_VALUE].name = "FillValue";
    varlist[SEL_DIRECTION].attrs[FILL_VALUE].type = varlist[SEL_DIRECTION].type;
    varlist[SEL_DIRECTION].attrs[FILL_VALUE].value.f = 0.0f;
    varlist[SEL_DIRECTION].attrs[VALID_MIN].name = "valid_min";
    varlist[SEL_DIRECTION].attrs[VALID_MIN].type = varlist[SEL_DIRECTION].type;
    varlist[SEL_DIRECTION].attrs[VALID_MIN].value.f = 0.0f;
    varlist[SEL_DIRECTION].attrs[VALID_MAX].name = "valid_max";
    varlist[SEL_DIRECTION].attrs[VALID_MAX].type = varlist[SEL_DIRECTION].type;
    varlist[SEL_DIRECTION].attrs[VALID_MAX].value.f = 360.0f;
    varlist[SEL_DIRECTION].attrs[LONG_NAME].name = "long_name";
    varlist[SEL_DIRECTION].attrs[LONG_NAME].type = NC_CHAR;
    varlist[SEL_DIRECTION].attrs[LONG_NAME].value.s = "wind_to_direction";
    varlist[SEL_DIRECTION].attrs[num_standard_attrs].name = "units";
    varlist[SEL_DIRECTION].attrs[num_standard_attrs].type = NC_CHAR;
    varlist[SEL_DIRECTION].attrs[num_standard_attrs].value.s = "degrees";
    varlist[SEL_DIRECTION].attrs[num_standard_attrs + 1].name = "scale_factor";
    varlist[SEL_DIRECTION].attrs[num_standard_attrs + 1].type = NC_FLOAT;
    varlist[SEL_DIRECTION].attrs[num_standard_attrs + 1].value.f = 1.0f;
   
    varlist[RAIN_IMPACT].name  = "rain_impact";
    varlist[RAIN_IMPACT].type  = NC_FLOAT;
    varlist[RAIN_IMPACT].ndims = 2;
    varlist[RAIN_IMPACT].dims = dimensions; 
    varlist[RAIN_IMPACT].nattrs = num_standard_attrs + 2;
    ERR((varlist[RAIN_IMPACT].attrs = (typeof varlist[RAIN_IMPACT].attrs)
                malloc(varlist[RAIN_IMPACT].nattrs * 
                    (sizeof *varlist[RAIN_IMPACT].attrs))) == NULL);

    varlist[RAIN_IMPACT].attrs[FILL_VALUE].name = "FillValue";
    varlist[RAIN_IMPACT].attrs[FILL_VALUE].type = varlist[RAIN_IMPACT].type;
    varlist[RAIN_IMPACT].attrs[FILL_VALUE].value.f = 0.0f;
    varlist[RAIN_IMPACT].attrs[VALID_MIN].name = "valid_min";
    varlist[RAIN_IMPACT].attrs[VALID_MIN].type = varlist[RAIN_IMPACT].type;
    varlist[RAIN_IMPACT].attrs[VALID_MIN].value.f = 0.0f;
    varlist[RAIN_IMPACT].attrs[VALID_MAX].name = "valid_max";
    varlist[RAIN_IMPACT].attrs[VALID_MAX].type = varlist[RAIN_IMPACT].type;
    varlist[RAIN_IMPACT].attrs[VALID_MAX].value.f = HUGE_VAL;
    varlist[RAIN_IMPACT].attrs[LONG_NAME].name = "long_name";
    varlist[RAIN_IMPACT].attrs[LONG_NAME].type = NC_CHAR;
    varlist[RAIN_IMPACT].attrs[LONG_NAME].value.s = "rain_impact";
    varlist[RAIN_IMPACT].attrs[num_standard_attrs].name = "units";
    varlist[RAIN_IMPACT].attrs[num_standard_attrs].type = NC_CHAR;
    varlist[RAIN_IMPACT].attrs[num_standard_attrs].value.s = "???";
    varlist[RAIN_IMPACT].attrs[num_standard_attrs + 1].name = "scale_factor";
    varlist[RAIN_IMPACT].attrs[num_standard_attrs + 1].type = NC_FLOAT;
    varlist[RAIN_IMPACT].attrs[num_standard_attrs + 1].value.f = 1.0f;
  
    varlist[DIVERGENCE].name  = "wind_divergence";
    varlist[DIVERGENCE].type  = NC_FLOAT;
    varlist[DIVERGENCE].ndims = 2;
    varlist[DIVERGENCE].dims = dimensions; 
    varlist[DIVERGENCE].nattrs = num_standard_attrs + 2;
    ERR((varlist[DIVERGENCE].attrs = (typeof varlist[DIVERGENCE].attrs)
                malloc(varlist[DIVERGENCE].nattrs * 
                    (sizeof *varlist[DIVERGENCE].attrs))) == NULL);

    varlist[DIVERGENCE].attrs[FILL_VALUE].name = "FillValue";
    varlist[DIVERGENCE].attrs[FILL_VALUE].type = varlist[DIVERGENCE].type;
    varlist[DIVERGENCE].attrs[FILL_VALUE].value.f = 0.0f;
    varlist[DIVERGENCE].attrs[VALID_MIN].name = "valid_min";
    varlist[DIVERGENCE].attrs[VALID_MIN].type = varlist[DIVERGENCE].type;
    varlist[DIVERGENCE].attrs[VALID_MIN].value.f = 0.0f;
    varlist[DIVERGENCE].attrs[VALID_MAX].name = "valid_max";
    varlist[DIVERGENCE].attrs[VALID_MAX].type = varlist[DIVERGENCE].type;
    varlist[DIVERGENCE].attrs[VALID_MAX].value.f = HUGE_VAL;
    varlist[DIVERGENCE].attrs[LONG_NAME].name = "long_name";
    varlist[DIVERGENCE].attrs[LONG_NAME].type = NC_CHAR;
    varlist[DIVERGENCE].attrs[LONG_NAME].value.s = "wind_divergence";
    varlist[DIVERGENCE].attrs[num_standard_attrs].name = "units";
    varlist[DIVERGENCE].attrs[num_standard_attrs].type = NC_CHAR;
    varlist[DIVERGENCE].attrs[num_standard_attrs].value.s = "s-1";
    varlist[DIVERGENCE].attrs[num_standard_attrs + 1].name = "scale_factor";
    varlist[DIVERGENCE].attrs[num_standard_attrs + 1].type = NC_FLOAT;
    varlist[DIVERGENCE].attrs[num_standard_attrs + 1].value.f = 1.0f;
 
    varlist[CURL].name  = "wind_curl";
    varlist[CURL].type  = NC_FLOAT;
    varlist[CURL].ndims = 2;
    varlist[CURL].dims = dimensions; 
    varlist[CURL].nattrs = num_standard_attrs + 2;
    ERR((varlist[CURL].attrs = (typeof varlist[CURL].attrs)
                malloc(varlist[CURL].nattrs * 
                    (sizeof *varlist[CURL].attrs))) == NULL);

    varlist[CURL].attrs[FILL_VALUE].name = "FillValue";
    varlist[CURL].attrs[FILL_VALUE].type = varlist[CURL].type;
    varlist[CURL].attrs[FILL_VALUE].value.f = 0.0f;
    varlist[CURL].attrs[VALID_MIN].name = "valid_min";
    varlist[CURL].attrs[VALID_MIN].type = varlist[CURL].type;
    varlist[CURL].attrs[VALID_MIN].value.f = 0.0f;
    varlist[CURL].attrs[VALID_MAX].name = "valid_max";
    varlist[CURL].attrs[VALID_MAX].type = varlist[CURL].type;
    varlist[CURL].attrs[VALID_MAX].value.f = HUGE_VAL;
    varlist[CURL].attrs[LONG_NAME].name = "long_name";
    varlist[CURL].attrs[LONG_NAME].type = NC_CHAR;
    varlist[CURL].attrs[LONG_NAME].value.s = "wind_curl";
    varlist[CURL].attrs[num_standard_attrs].name = "units";
    varlist[CURL].attrs[num_standard_attrs].type = NC_CHAR;
    varlist[CURL].attrs[num_standard_attrs].value.s = "s-1";
    varlist[CURL].attrs[num_standard_attrs + 1].name = "scale_factor";
    varlist[CURL].attrs[num_standard_attrs + 1].type = NC_FLOAT;
    varlist[CURL].attrs[num_standard_attrs + 1].value.f = 1.0f;

    varlist[WIND_STRESS].name  = "stress";
    varlist[WIND_STRESS].type  = NC_FLOAT;
    varlist[WIND_STRESS].ndims = 2;
    varlist[WIND_STRESS].dims = dimensions; 
    varlist[WIND_STRESS].nattrs = num_standard_attrs + 2;
    ERR((varlist[WIND_STRESS].attrs = (typeof varlist[WIND_STRESS].attrs)
                malloc(varlist[WIND_STRESS].nattrs * 
                    (sizeof *varlist[WIND_STRESS].attrs))) == NULL);

    varlist[WIND_STRESS].attrs[FILL_VALUE].name = "FillValue";
    varlist[WIND_STRESS].attrs[FILL_VALUE].type = varlist[WIND_STRESS].type;
    varlist[WIND_STRESS].attrs[FILL_VALUE].value.f = 0.0f;
    varlist[WIND_STRESS].attrs[VALID_MIN].name = "valid_min";
    varlist[WIND_STRESS].attrs[VALID_MIN].type = varlist[WIND_STRESS].type;
    varlist[WIND_STRESS].attrs[VALID_MIN].value.f = 0.0f;
    varlist[WIND_STRESS].attrs[VALID_MAX].name = "valid_max";
    varlist[WIND_STRESS].attrs[VALID_MAX].type = varlist[WIND_STRESS].type;
    varlist[WIND_STRESS].attrs[VALID_MAX].value.f = HUGE_VAL;
    varlist[WIND_STRESS].attrs[LONG_NAME].name = "long_name";
    varlist[WIND_STRESS].attrs[LONG_NAME].type = NC_CHAR;
    varlist[WIND_STRESS].attrs[LONG_NAME].value.s = "wind_stress";
    varlist[WIND_STRESS].attrs[num_standard_attrs].name = "units";
    varlist[WIND_STRESS].attrs[num_standard_attrs].type = NC_CHAR;
    varlist[WIND_STRESS].attrs[num_standard_attrs].value.s = "s-1";
    varlist[WIND_STRESS].attrs[num_standard_attrs + 1].name = "scale_factor";
    varlist[WIND_STRESS].attrs[num_standard_attrs + 1].type = NC_FLOAT;
    varlist[WIND_STRESS].attrs[num_standard_attrs + 1].value.f = 1.0f;

    
    varlist[WIND_STRESS_DIVERGENCE].name  = "stress_divergence";
    varlist[WIND_STRESS_DIVERGENCE].type  = NC_FLOAT;
    varlist[WIND_STRESS_DIVERGENCE].ndims = 2;
    varlist[WIND_STRESS_DIVERGENCE].dims = dimensions; 
    varlist[WIND_STRESS_DIVERGENCE].nattrs = num_standard_attrs + 2;
    ERR((varlist[WIND_STRESS_DIVERGENCE].attrs = (typeof varlist[WIND_STRESS_DIVERGENCE].attrs)
                malloc(varlist[WIND_STRESS_DIVERGENCE].nattrs * 
                    (sizeof *varlist[WIND_STRESS_DIVERGENCE].attrs))) == NULL);

    varlist[WIND_STRESS_DIVERGENCE].attrs[FILL_VALUE].name = "FillValue";
    varlist[WIND_STRESS_DIVERGENCE].attrs[FILL_VALUE].type = varlist[WIND_STRESS_DIVERGENCE].type;
    varlist[WIND_STRESS_DIVERGENCE].attrs[FILL_VALUE].value.f = 0.0f;
    varlist[WIND_STRESS_DIVERGENCE].attrs[VALID_MIN].name = "valid_min";
    varlist[WIND_STRESS_DIVERGENCE].attrs[VALID_MIN].type = varlist[WIND_STRESS_DIVERGENCE].type;
    varlist[WIND_STRESS_DIVERGENCE].attrs[VALID_MIN].value.f = 0.0f;
    varlist[WIND_STRESS_DIVERGENCE].attrs[VALID_MAX].name = "valid_max";
    varlist[WIND_STRESS_DIVERGENCE].attrs[VALID_MAX].type = varlist[WIND_STRESS_DIVERGENCE].type;
    varlist[WIND_STRESS_DIVERGENCE].attrs[VALID_MAX].value.f = HUGE_VAL;
    varlist[WIND_STRESS_DIVERGENCE].attrs[LONG_NAME].name = "long_name";
    varlist[WIND_STRESS_DIVERGENCE].attrs[LONG_NAME].type = NC_CHAR;
    varlist[WIND_STRESS_DIVERGENCE].attrs[LONG_NAME].value.s = "stress_divergence";
    varlist[WIND_STRESS_DIVERGENCE].attrs[num_standard_attrs].name = "units";
    varlist[WIND_STRESS_DIVERGENCE].attrs[num_standard_attrs].type = NC_CHAR;
    varlist[WIND_STRESS_DIVERGENCE].attrs[num_standard_attrs].value.s = "s-1";
    varlist[WIND_STRESS_DIVERGENCE].attrs[num_standard_attrs + 1].name = "scale_factor";
    varlist[WIND_STRESS_DIVERGENCE].attrs[num_standard_attrs + 1].type = NC_FLOAT;
    varlist[WIND_STRESS_DIVERGENCE].attrs[num_standard_attrs + 1].value.f = 1.0f;

    varlist[WIND_STRESS_CURL].name  = "stress_curl";
    varlist[WIND_STRESS_CURL].type  = NC_FLOAT;
    varlist[WIND_STRESS_CURL].ndims = 2;
    varlist[WIND_STRESS_CURL].dims = dimensions; 
    varlist[WIND_STRESS_CURL].nattrs = num_standard_attrs + 2;
    ERR((varlist[WIND_STRESS_CURL].attrs = (typeof varlist[WIND_STRESS_CURL].attrs)
                malloc(varlist[WIND_STRESS_CURL].nattrs * 
                    (sizeof *varlist[WIND_STRESS_CURL].attrs))) == NULL);

    varlist[WIND_STRESS_CURL].attrs[FILL_VALUE].name = "FillValue";
    varlist[WIND_STRESS_CURL].attrs[FILL_VALUE].type = varlist[WIND_STRESS_CURL].type;
    varlist[WIND_STRESS_CURL].attrs[FILL_VALUE].value.f = 0.0f;
    varlist[WIND_STRESS_CURL].attrs[VALID_MIN].name = "valid_min";
    varlist[WIND_STRESS_CURL].attrs[VALID_MIN].type = varlist[WIND_STRESS_CURL].type;
    varlist[WIND_STRESS_CURL].attrs[VALID_MIN].value.f = 0.0f;
    varlist[WIND_STRESS_CURL].attrs[VALID_MAX].name = "valid_max";
    varlist[WIND_STRESS_CURL].attrs[VALID_MAX].type = varlist[WIND_STRESS_CURL].type;
    varlist[WIND_STRESS_CURL].attrs[VALID_MAX].value.f = HUGE_VAL;
    varlist[WIND_STRESS_CURL].attrs[LONG_NAME].name = "long_name";
    varlist[WIND_STRESS_CURL].attrs[LONG_NAME].type = NC_CHAR;
    varlist[WIND_STRESS_CURL].attrs[LONG_NAME].value.s = "stress_curl";
    varlist[WIND_STRESS_CURL].attrs[num_standard_attrs].name = "units";
    varlist[WIND_STRESS_CURL].attrs[num_standard_attrs].type = NC_CHAR;
    varlist[WIND_STRESS_CURL].attrs[num_standard_attrs].value.s = "s-1";
    varlist[WIND_STRESS_CURL].attrs[num_standard_attrs + 1].name = "scale_factor";
    varlist[WIND_STRESS_CURL].attrs[num_standard_attrs + 1].type = NC_FLOAT;
    varlist[WIND_STRESS_DIVERGENCE].attrs[num_standard_attrs + 1].value.f = 1.0f;
    
    varlist[TIME].name = "time";
    varlist[TIME].type = NC_INT;
    varlist[TIME].ndims = 1;
    varlist[TIME].dims = dimensions;
    varlist[TIME].nattrs = num_standard_attrs + 1;
    ERR((varlist[TIME].attrs = (typeof varlist[TIME].attrs)
                malloc(varlist[TIME].nattrs * 
                    (sizeof *varlist[TIME].attrs))) == NULL);

    varlist[TIME].attrs[FILL_VALUE].name = "FillValue";
    varlist[TIME].attrs[FILL_VALUE].type = varlist[TIME].type;
    varlist[TIME].attrs[FILL_VALUE].value.i = 0;
    varlist[TIME].attrs[VALID_MIN].name = "valid_min";
    varlist[TIME].attrs[VALID_MIN].type = varlist[TIME].type;
    varlist[TIME].attrs[VALID_MIN].value.i = INT_MIN;
    varlist[TIME].attrs[VALID_MAX].name = "valid_max";
    varlist[TIME].attrs[VALID_MAX].type = varlist[TIME].type;
    varlist[TIME].attrs[VALID_MAX].value.i = INT_MAX;
    varlist[TIME].attrs[LONG_NAME].name = "long_name";
    varlist[TIME].attrs[LONG_NAME].type = NC_CHAR;
    varlist[TIME].attrs[LONG_NAME].value.s = "time";
    varlist[TIME].attrs[num_standard_attrs].name = "units";
    varlist[TIME].attrs[num_standard_attrs].type = NC_CHAR;
    varlist[TIME].attrs[num_standard_attrs].value.s = "seconds since 1990-01-01 00:00:00";

    varlist[FLAGS].name  = "flags";
    varlist[FLAGS].type  = NC_BYTE;
    varlist[FLAGS].ndims = 2;
    varlist[FLAGS].dims = dimensions; 
    varlist[FLAGS].nattrs = 0;
    varlist[FLAGS].nattrs = num_standard_attrs + 6;
    ERR((varlist[FLAGS].attrs = (typeof varlist[FLAGS].attrs)
                malloc(varlist[FLAGS].nattrs * 
                    (sizeof *varlist[FLAGS].attrs))) == NULL);

    varlist[FLAGS].attrs[FILL_VALUE].name = "FillValue";
    varlist[FLAGS].attrs[FILL_VALUE].type = varlist[FLAGS].type;
    varlist[FLAGS].attrs[FILL_VALUE].value.c = 0;
    varlist[FLAGS].attrs[VALID_MIN].name = "valid_min";
    varlist[FLAGS].attrs[VALID_MIN].type = varlist[FLAGS].type;
    varlist[FLAGS].attrs[VALID_MIN].value.c = 0;
    varlist[FLAGS].attrs[VALID_MAX].name = "valid_max";
    varlist[FLAGS].attrs[VALID_MAX].type = varlist[FLAGS].type;
    varlist[FLAGS].attrs[VALID_MAX].value.c = 0x7f;
    varlist[FLAGS].attrs[LONG_NAME].name = "long_name";
    varlist[FLAGS].attrs[LONG_NAME].type = NC_CHAR;
    varlist[FLAGS].attrs[LONG_NAME].value.s = "flags";
    varlist[FLAGS].attrs[num_standard_attrs].name = "usable_mask";
    varlist[FLAGS].attrs[num_standard_attrs].type = NC_BYTE;
    varlist[FLAGS].attrs[num_standard_attrs].value.c = USABLE_MASK;
    varlist[FLAGS].attrs[num_standard_attrs + 1].name = "rain_mask";
    varlist[FLAGS].attrs[num_standard_attrs + 1].type = NC_BYTE;
    varlist[FLAGS].attrs[num_standard_attrs + 1].value.c = RAIN_MASK; 
    varlist[FLAGS].attrs[num_standard_attrs + 2].name = "beam_mask";
    varlist[FLAGS].attrs[num_standard_attrs + 2].type = NC_BYTE;
    varlist[FLAGS].attrs[num_standard_attrs + 2].value.c = BEAM_MASK; 
    varlist[FLAGS].attrs[num_standard_attrs + 3].name = "land_mask";
    varlist[FLAGS].attrs[num_standard_attrs + 3].type = NC_BYTE;
    varlist[FLAGS].attrs[num_standard_attrs + 3].value.c = LAND_MASK; 
    varlist[FLAGS].attrs[num_standard_attrs + 4].name = "ice_mask";
    varlist[FLAGS].attrs[num_standard_attrs + 4].type = NC_BYTE;
    varlist[FLAGS].attrs[num_standard_attrs + 4].value.c = ICE_MASK; 
    varlist[FLAGS].attrs[num_standard_attrs + 5].name = "point_valid_mask";
    varlist[FLAGS].attrs[num_standard_attrs + 5].type = NC_BYTE;
    varlist[FLAGS].attrs[num_standard_attrs + 5].value.c = VALID_MASK; 

    
    if (config.extended) { 
        varlist[NUDGE_SPEED].name  = "nudge_speed";
        varlist[NUDGE_SPEED].type  = NC_FLOAT;
        varlist[NUDGE_SPEED].ndims = 2;
        varlist[NUDGE_SPEED].dims = dimensions; 
        varlist[NUDGE_SPEED].nattrs = num_standard_attrs + 2;
        ERR((varlist[NUDGE_SPEED].attrs = (typeof varlist[NUDGE_SPEED].attrs)
                    malloc(varlist[NUDGE_SPEED].nattrs * 
                        (sizeof *varlist[NUDGE_SPEED].attrs))) == NULL);
    
        varlist[NUDGE_SPEED].attrs[FILL_VALUE].name = "FillValue";
        varlist[NUDGE_SPEED].attrs[FILL_VALUE].type = varlist[NUDGE_SPEED].type;
        varlist[NUDGE_SPEED].attrs[FILL_VALUE].value.f = 0.0f;
        varlist[NUDGE_SPEED].attrs[VALID_MIN].name = "valid_min";
        varlist[NUDGE_SPEED].attrs[VALID_MIN].type = varlist[NUDGE_SPEED].type;
        varlist[NUDGE_SPEED].attrs[VALID_MIN].value.f = 0.0f;
        varlist[NUDGE_SPEED].attrs[VALID_MAX].name = "valid_max";
        varlist[NUDGE_SPEED].attrs[VALID_MAX].type = varlist[NUDGE_SPEED].type;
        varlist[NUDGE_SPEED].attrs[VALID_MAX].value.f = HUGE_VAL;
        varlist[NUDGE_SPEED].attrs[LONG_NAME].name = "long_name";
        varlist[NUDGE_SPEED].attrs[LONG_NAME].type = NC_CHAR;
        varlist[NUDGE_SPEED].attrs[LONG_NAME].value.s = "nudge_speed";
        varlist[NUDGE_SPEED].attrs[num_standard_attrs].name = "units";
        varlist[NUDGE_SPEED].attrs[num_standard_attrs].type = NC_CHAR;
        varlist[NUDGE_SPEED].attrs[num_standard_attrs].value.s = "m s-1";
        varlist[NUDGE_SPEED].attrs[num_standard_attrs + 1].name = "scale_factor";
        varlist[NUDGE_SPEED].attrs[num_standard_attrs + 1].type = NC_FLOAT;
        varlist[NUDGE_SPEED].attrs[num_standard_attrs + 1].value.f = 1.0f;
                 
        varlist[NUDGE_DIRECTION].name  = "nudge_direction";
        varlist[NUDGE_DIRECTION].type  = NC_FLOAT;
        varlist[NUDGE_DIRECTION].ndims = 2;
        varlist[NUDGE_DIRECTION].dims = dimensions; 
        varlist[NUDGE_DIRECTION].nattrs = num_standard_attrs + 2;
        ERR((varlist[NUDGE_DIRECTION].attrs = (typeof varlist[NUDGE_DIRECTION].attrs)
                    malloc(varlist[NUDGE_DIRECTION].nattrs * 
                        (sizeof *varlist[NUDGE_DIRECTION].attrs))) == NULL);
    
        varlist[NUDGE_DIRECTION].attrs[FILL_VALUE].name = "FillValue";
        varlist[NUDGE_DIRECTION].attrs[FILL_VALUE].type = varlist[NUDGE_DIRECTION].type;
        varlist[NUDGE_DIRECTION].attrs[FILL_VALUE].value.f = 0.0f;
        varlist[NUDGE_DIRECTION].attrs[VALID_MIN].name = "valid_min";
        varlist[NUDGE_DIRECTION].attrs[VALID_MIN].type = varlist[NUDGE_DIRECTION].type;
        varlist[NUDGE_DIRECTION].attrs[VALID_MIN].value.f = 0.0f;
        varlist[NUDGE_DIRECTION].attrs[VALID_MAX].name = "valid_max";
        varlist[NUDGE_DIRECTION].attrs[VALID_MAX].type = varlist[NUDGE_DIRECTION].type;
        varlist[NUDGE_DIRECTION].attrs[VALID_MAX].value.f = 360.0f;
        varlist[NUDGE_DIRECTION].attrs[LONG_NAME].name = "long_name";
        varlist[NUDGE_DIRECTION].attrs[LONG_NAME].type = NC_CHAR;
        varlist[NUDGE_DIRECTION].attrs[LONG_NAME].value.s = "nudge_direction";
        varlist[NUDGE_DIRECTION].attrs[num_standard_attrs].name = "units";
        varlist[NUDGE_DIRECTION].attrs[num_standard_attrs].type = NC_CHAR;
        varlist[NUDGE_DIRECTION].attrs[num_standard_attrs].value.s = "degrees";
        varlist[NUDGE_DIRECTION].attrs[num_standard_attrs + 1].name = "scale_factor";
        varlist[NUDGE_DIRECTION].attrs[num_standard_attrs + 1].type = NC_FLOAT;
        varlist[NUDGE_DIRECTION].attrs[num_standard_attrs + 1].value.f = 1.0f;
                   
        varlist[SEL_OBJ].name  = "wind_obj";
        varlist[SEL_OBJ].type  = NC_FLOAT;
        varlist[SEL_OBJ].ndims = 2;
        varlist[SEL_OBJ].dims = dimensions; 
        varlist[SEL_OBJ].nattrs = 0;
        varlist[SEL_OBJ].nattrs = num_standard_attrs + 2;
        ERR((varlist[SEL_OBJ].attrs = (typeof varlist[SEL_OBJ].attrs)
                    malloc(varlist[SEL_OBJ].nattrs * 
                        (sizeof *varlist[SEL_OBJ].attrs))) == NULL);
    
        varlist[SEL_OBJ].attrs[FILL_VALUE].name = "FillValue";
        varlist[SEL_OBJ].attrs[FILL_VALUE].type = varlist[SEL_OBJ].type;
        varlist[SEL_OBJ].attrs[FILL_VALUE].value.f = 0.0f;
        varlist[SEL_OBJ].attrs[VALID_MIN].name = "valid_min";
        varlist[SEL_OBJ].attrs[VALID_MIN].type = varlist[SEL_OBJ].type;
        varlist[SEL_OBJ].attrs[VALID_MIN].value.f = 0.0f;
        varlist[SEL_OBJ].attrs[VALID_MAX].name = "valid_max";
        varlist[SEL_OBJ].attrs[VALID_MAX].type = varlist[SEL_OBJ].type;
        varlist[SEL_OBJ].attrs[VALID_MAX].value.f = 360.0f;
        varlist[SEL_OBJ].attrs[LONG_NAME].name = "long_name";
        varlist[SEL_OBJ].attrs[LONG_NAME].type = NC_CHAR;
        varlist[SEL_OBJ].attrs[LONG_NAME].value.s = "wind_obj";
        varlist[SEL_OBJ].attrs[num_standard_attrs].name = "units";
        varlist[SEL_OBJ].attrs[num_standard_attrs].type = NC_CHAR;
        varlist[SEL_OBJ].attrs[num_standard_attrs].value.s = "1";
        varlist[SEL_OBJ].attrs[num_standard_attrs + 1].name = "scale_factor";
        varlist[SEL_OBJ].attrs[num_standard_attrs + 1].type = NC_FLOAT;
        varlist[SEL_OBJ].attrs[num_standard_attrs + 1].value.f = 1.0f;

        varlist[AMBIG_SPEED].name  = "ambiguity_speed";
        varlist[AMBIG_SPEED].type  = NC_FLOAT;
        varlist[AMBIG_SPEED].ndims = 3;
        varlist[AMBIG_SPEED].dims = dimensions; 
        varlist[AMBIG_SPEED].nattrs = 0;
        varlist[AMBIG_SPEED].nattrs = num_standard_attrs + 2;
        ERR((varlist[AMBIG_SPEED].attrs = (typeof varlist[AMBIG_SPEED].attrs)
                    malloc(varlist[AMBIG_SPEED].nattrs * 
                        (sizeof *varlist[AMBIG_SPEED].attrs))) == NULL);
    
        varlist[AMBIG_SPEED].attrs[FILL_VALUE].name = "FillValue";
        varlist[AMBIG_SPEED].attrs[FILL_VALUE].type = varlist[AMBIG_SPEED].type;
        varlist[AMBIG_SPEED].attrs[FILL_VALUE].value.f = 0.0f;
        varlist[AMBIG_SPEED].attrs[VALID_MIN].name = "valid_min";
        varlist[AMBIG_SPEED].attrs[VALID_MIN].type = varlist[AMBIG_SPEED].type;
        varlist[AMBIG_SPEED].attrs[VALID_MIN].value.f = 0.0f;
        varlist[AMBIG_SPEED].attrs[VALID_MAX].name = "valid_max";
        varlist[AMBIG_SPEED].attrs[VALID_MAX].type = varlist[AMBIG_SPEED].type;
        varlist[AMBIG_SPEED].attrs[VALID_MAX].value.f = HUGE_VAL;
        varlist[AMBIG_SPEED].attrs[LONG_NAME].name = "long_name";
        varlist[AMBIG_SPEED].attrs[LONG_NAME].type = NC_CHAR;
        varlist[AMBIG_SPEED].attrs[LONG_NAME].value.s = "ambig_speed";
        varlist[AMBIG_SPEED].attrs[num_standard_attrs].name = "units";
        varlist[AMBIG_SPEED].attrs[num_standard_attrs].type = NC_CHAR;
        varlist[AMBIG_SPEED].attrs[num_standard_attrs].value.s = "m s-1";
        varlist[AMBIG_SPEED].attrs[num_standard_attrs + 1].name = "scale_factor";
        varlist[AMBIG_SPEED].attrs[num_standard_attrs + 1].type = NC_FLOAT;
        varlist[AMBIG_SPEED].attrs[num_standard_attrs + 1].value.f = 1.0f;

        varlist[AMBIG_DIRECTION].name  = "ambiguity_direction";
        varlist[AMBIG_DIRECTION].type  = NC_FLOAT;
        varlist[AMBIG_DIRECTION].ndims = 3;
        varlist[AMBIG_DIRECTION].dims = dimensions; 
        varlist[AMBIG_DIRECTION].nattrs = num_standard_attrs + 2;
        ERR((varlist[AMBIG_DIRECTION].attrs = (typeof varlist[AMBIG_DIRECTION].attrs)
                    malloc(varlist[AMBIG_DIRECTION].nattrs * 
                        (sizeof *varlist[AMBIG_DIRECTION].attrs))) == NULL);
    
        varlist[AMBIG_DIRECTION].attrs[FILL_VALUE].name = "FillValue";
        varlist[AMBIG_DIRECTION].attrs[FILL_VALUE].type = varlist[AMBIG_DIRECTION].type;
        varlist[AMBIG_DIRECTION].attrs[FILL_VALUE].value.f = 0.0f;
        varlist[AMBIG_DIRECTION].attrs[VALID_MIN].name = "valid_min";
        varlist[AMBIG_DIRECTION].attrs[VALID_MIN].type = varlist[AMBIG_DIRECTION].type;
        varlist[AMBIG_DIRECTION].attrs[VALID_MIN].value.f = 0.0f;
        varlist[AMBIG_DIRECTION].attrs[VALID_MAX].name = "valid_max";
        varlist[AMBIG_DIRECTION].attrs[VALID_MAX].type = varlist[AMBIG_DIRECTION].type;
        varlist[AMBIG_DIRECTION].attrs[VALID_MAX].value.f = 360.0f;
        varlist[AMBIG_DIRECTION].attrs[LONG_NAME].name = "long_name";
        varlist[AMBIG_DIRECTION].attrs[LONG_NAME].type = NC_CHAR;
        varlist[AMBIG_DIRECTION].attrs[LONG_NAME].value.s = "ambig_direction";
        varlist[AMBIG_DIRECTION].attrs[num_standard_attrs].name = "units";
        varlist[AMBIG_DIRECTION].attrs[num_standard_attrs].type = NC_CHAR;
        varlist[AMBIG_DIRECTION].attrs[num_standard_attrs].value.s = "degrees";
        varlist[AMBIG_DIRECTION].attrs[num_standard_attrs + 1].name = "scale_factor";
        varlist[AMBIG_DIRECTION].attrs[num_standard_attrs + 1].type = NC_FLOAT;
        varlist[AMBIG_DIRECTION].attrs[num_standard_attrs + 1].value.f = 1.0f;
                      
        varlist[AMBIG_OBJ].name  = "ambiguity_obj";
        varlist[AMBIG_OBJ].type  = NC_FLOAT;
        varlist[AMBIG_OBJ].ndims = 3;
        varlist[AMBIG_OBJ].dims = dimensions; 
        varlist[AMBIG_OBJ].nattrs = num_standard_attrs + 2;
        ERR((varlist[AMBIG_OBJ].attrs = (typeof varlist[AMBIG_OBJ].attrs)
                    malloc(varlist[AMBIG_OBJ].nattrs * 
                        (sizeof *varlist[AMBIG_OBJ].attrs))) == NULL);
    
        varlist[AMBIG_OBJ].attrs[FILL_VALUE].name = "FillValue";
        varlist[AMBIG_OBJ].attrs[FILL_VALUE].type = varlist[AMBIG_OBJ].type;
        varlist[AMBIG_OBJ].attrs[FILL_VALUE].value.f = 0.0f;
        varlist[AMBIG_OBJ].attrs[VALID_MIN].name = "valid_min";
        varlist[AMBIG_OBJ].attrs[VALID_MIN].type = varlist[AMBIG_OBJ].type;
        varlist[AMBIG_OBJ].attrs[VALID_MIN].value.f = 0.0f;
        varlist[AMBIG_OBJ].attrs[VALID_MAX].name = "valid_max";
        varlist[AMBIG_OBJ].attrs[VALID_MAX].type = varlist[AMBIG_OBJ].type;
        varlist[AMBIG_OBJ].attrs[VALID_MAX].value.f = 360.0f;
        varlist[AMBIG_OBJ].attrs[LONG_NAME].name = "long_name";
        varlist[AMBIG_OBJ].attrs[LONG_NAME].type = NC_CHAR;
        varlist[AMBIG_OBJ].attrs[LONG_NAME].value.s = "ambig_obj";
        varlist[AMBIG_OBJ].attrs[num_standard_attrs].name = "units";
        varlist[AMBIG_OBJ].attrs[num_standard_attrs].type = NC_CHAR;
        varlist[AMBIG_OBJ].attrs[num_standard_attrs].value.s = "1";
        varlist[AMBIG_OBJ].attrs[num_standard_attrs + 1].name = "scale_factor";
        varlist[AMBIG_OBJ].attrs[num_standard_attrs + 1].type = NC_FLOAT;
        varlist[AMBIG_OBJ].attrs[num_standard_attrs + 1].value.f = 1.0f;

        varlist[N_IN_FORE].name  = "number_in_fore";
        varlist[N_IN_FORE].type  = NC_BYTE;
        varlist[N_IN_FORE].ndims = 2;
        varlist[N_IN_FORE].dims = dimensions; 
        varlist[N_IN_FORE].nattrs = 0;
        varlist[N_IN_FORE].nattrs = num_standard_attrs + 1;
        ERR((varlist[N_IN_FORE].attrs = (typeof varlist[N_IN_FORE].attrs)
                    malloc(varlist[N_IN_FORE].nattrs * 
                        (sizeof *varlist[N_IN_FORE].attrs))) == NULL);
    
        varlist[N_IN_FORE].attrs[FILL_VALUE].name = "FillValue";
        varlist[N_IN_FORE].attrs[FILL_VALUE].type = varlist[N_IN_FORE].type;
        varlist[N_IN_FORE].attrs[FILL_VALUE].value.c = 0;
        varlist[N_IN_FORE].attrs[VALID_MIN].name = "valid_min";
        varlist[N_IN_FORE].attrs[VALID_MIN].type = varlist[N_IN_FORE].type;
        varlist[N_IN_FORE].attrs[VALID_MIN].value.c = 0;
        varlist[N_IN_FORE].attrs[VALID_MAX].name = "valid_max";
        varlist[N_IN_FORE].attrs[VALID_MAX].type = varlist[N_IN_FORE].type;
        varlist[N_IN_FORE].attrs[VALID_MAX].value.c = 0x7f;
        varlist[N_IN_FORE].attrs[LONG_NAME].name = "long_name";
        varlist[N_IN_FORE].attrs[LONG_NAME].type = NC_CHAR;
        varlist[N_IN_FORE].attrs[LONG_NAME].value.s = "number_in_fore";
        varlist[N_IN_FORE].attrs[num_standard_attrs].name = "units";
        varlist[N_IN_FORE].attrs[num_standard_attrs].type = NC_CHAR;
        varlist[N_IN_FORE].attrs[num_standard_attrs].value.s = "1";
            
        varlist[N_IN_AFT].name  = "number_in_aft";
        varlist[N_IN_AFT].type  = NC_BYTE;
        varlist[N_IN_AFT].ndims = 2;
        varlist[N_IN_AFT].dims = dimensions; 
        varlist[N_IN_AFT].nattrs = num_standard_attrs + 1;
        ERR((varlist[N_IN_AFT].attrs = (typeof varlist[N_IN_AFT].attrs)
                    malloc(varlist[N_IN_AFT].nattrs * 
                        (sizeof *varlist[N_IN_AFT].attrs))) == NULL);
    
        varlist[N_IN_AFT].attrs[FILL_VALUE].name = "FillValue";
        varlist[N_IN_AFT].attrs[FILL_VALUE].type = varlist[N_IN_AFT].type;
        varlist[N_IN_AFT].attrs[FILL_VALUE].value.c = 0;
        varlist[N_IN_AFT].attrs[VALID_MIN].name = "valid_min";
        varlist[N_IN_AFT].attrs[VALID_MIN].type = varlist[N_IN_AFT].type;
        varlist[N_IN_AFT].attrs[VALID_MIN].value.c = 0;
        varlist[N_IN_AFT].attrs[VALID_MAX].name = "valid_max";
        varlist[N_IN_AFT].attrs[VALID_MAX].type = varlist[N_IN_AFT].type;
        varlist[N_IN_AFT].attrs[VALID_MAX].value.c = 0x7f;
        varlist[N_IN_AFT].attrs[LONG_NAME].name = "long_name";
        varlist[N_IN_AFT].attrs[LONG_NAME].type = NC_CHAR;
        varlist[N_IN_AFT].attrs[LONG_NAME].value.s = "number_in_aft";
        varlist[N_IN_AFT].attrs[num_standard_attrs].name = "units";
        varlist[N_IN_AFT].attrs[num_standard_attrs].type = NC_CHAR;
        varlist[N_IN_AFT].attrs[num_standard_attrs].value.s = "1";
               
        varlist[N_OUT_FORE].name  = "number_out_fore";
        varlist[N_OUT_FORE].type  = NC_BYTE;
        varlist[N_OUT_FORE].ndims = 2;
        varlist[N_OUT_FORE].dims = dimensions; 
        varlist[N_OUT_FORE].nattrs = num_standard_attrs + 1;
        ERR((varlist[N_OUT_FORE].attrs = (typeof varlist[N_OUT_FORE].attrs)
                    malloc(varlist[N_OUT_FORE].nattrs * 
                        (sizeof *varlist[N_OUT_FORE].attrs))) == NULL);
    
        varlist[N_OUT_FORE].attrs[FILL_VALUE].name = "FillValue";
        varlist[N_OUT_FORE].attrs[FILL_VALUE].type = varlist[N_OUT_FORE].type;
        varlist[N_OUT_FORE].attrs[FILL_VALUE].value.c = 0;
        varlist[N_OUT_FORE].attrs[VALID_MIN].name = "valid_min";
        varlist[N_OUT_FORE].attrs[VALID_MIN].type = varlist[N_OUT_FORE].type;
        varlist[N_OUT_FORE].attrs[VALID_MIN].value.c = 0;
        varlist[N_OUT_FORE].attrs[VALID_MAX].name = "valid_max";
        varlist[N_OUT_FORE].attrs[VALID_MAX].type = varlist[N_OUT_FORE].type;
        varlist[N_OUT_FORE].attrs[VALID_MAX].value.c = 0x7f;
        varlist[N_OUT_FORE].attrs[LONG_NAME].name = "long_name";
        varlist[N_OUT_FORE].attrs[LONG_NAME].type = NC_CHAR;
        varlist[N_OUT_FORE].attrs[LONG_NAME].value.s = "number_out_fore";
        varlist[N_OUT_FORE].attrs[num_standard_attrs].name = "units";
        varlist[N_OUT_FORE].attrs[num_standard_attrs].type = NC_CHAR;
        varlist[N_OUT_FORE].attrs[num_standard_attrs].value.s = "1";
               
        varlist[N_OUT_AFT].name  = "number_out_aft";
        varlist[N_OUT_AFT].type  = NC_BYTE;
        varlist[N_OUT_AFT].ndims = 2;
        varlist[N_OUT_AFT].dims = dimensions; 
        varlist[N_OUT_AFT].nattrs = 0;
        ERR((varlist[N_OUT_AFT].attrs = (typeof varlist[N_OUT_AFT].attrs)
                    malloc(varlist[N_OUT_AFT].nattrs * 
                        (sizeof *varlist[N_OUT_AFT].attrs))) == NULL);
    
        varlist[N_OUT_AFT].attrs[FILL_VALUE].name = "FillValue";
        varlist[N_OUT_AFT].attrs[FILL_VALUE].type = varlist[N_OUT_AFT].type;
        varlist[N_OUT_AFT].attrs[FILL_VALUE].value.c = 0;
        varlist[N_OUT_AFT].attrs[VALID_MIN].name = "valid_min";
        varlist[N_OUT_AFT].attrs[VALID_MIN].type = varlist[N_OUT_AFT].type;
        varlist[N_OUT_AFT].attrs[VALID_MIN].value.c = 0;
        varlist[N_OUT_AFT].attrs[VALID_MAX].name = "valid_max";
        varlist[N_OUT_AFT].attrs[VALID_MAX].type = varlist[N_OUT_AFT].type;
        varlist[N_OUT_AFT].attrs[VALID_MAX].value.c = 0x7f;
        varlist[N_OUT_AFT].attrs[LONG_NAME].name = "long_name";
        varlist[N_OUT_AFT].attrs[LONG_NAME].type = NC_CHAR;
        varlist[N_OUT_AFT].attrs[LONG_NAME].value.s = "number_out_aft";
        varlist[N_OUT_AFT].attrs[num_standard_attrs].name = "units";
        varlist[N_OUT_AFT].attrs[num_standard_attrs].type = NC_CHAR;
        varlist[N_OUT_AFT].attrs[num_standard_attrs].value.s = "1";
    }



    for (int i = 0; i < (config.extended ? num_variables : 
                first_extended_variable); i++) {
        NCERR(nc_def_var(ncid, varlist[i].name, varlist[i].type,
                   varlist[i].ndims, varlist[i].dims, &varlist[i].id));

        for (int j = 0; j < varlist[i].nattrs; j++) {
            attribute *attr = &varlist[i].attrs[j];
            switch (attr->type) {
                case NC_CHAR:
                    NCERR(nc_put_att_text(ncid, varlist[i].id, attr->name,
                                strlen(attr->value.s), (attr->value.s)));
                    break;
                case NC_INT:
                    NCERR(nc_put_att_int(ncid, varlist[i].id, attr->name, 
                                attr->type, (size_t)(1), &(attr->value.i)));
                    break;
                case NC_FLOAT:
                    NCERR(nc_put_att_float(ncid, varlist[i].id, attr->name, 
                                attr->type, (size_t)(1), &(attr->value.f)));
                    break;
                case NC_BYTE:
                    NCERR(nc_put_att_ubyte(ncid, varlist[i].id, attr->name, 
                                attr->type, (size_t)(1), &(attr->value.c)));
                    break;
                default:
                    fprintf(stderr, "Incorrect attribute type: %d\n", 
                            attr->type);
                    exit(EXIT_FAILURE);
            }
        }
    }

    // Write the header attributes
    NCERR(nc_put_att_float(ncid, NC_GLOBAL, "cross_track_resolution", 
                NC_FLOAT, (size_t)(1), &l2b.header.crossTrackResolution));
    NCERR(nc_put_att_float(ncid, NC_GLOBAL, "along_track_resolution", 
                NC_FLOAT, (size_t)(1), &l2b.header.alongTrackResolution));
    NCERR(nc_put_att_int  (ncid, NC_GLOBAL, "zero_index", 
                NC_INT,   (size_t)(1), &l2b.header.zeroIndex));
    NCERR(nc_put_att_float(ncid, NC_GLOBAL, "inclination", 
                NC_FLOAT, (size_t)(1), &l2b.header.inclination));
    NCERR(nc_put_att_int  (ncid, NC_GLOBAL, "version_id_major", 
                NC_INT,   (size_t)(1), &l2b.header.version_id_major));
    NCERR(nc_put_att_int  (ncid, NC_GLOBAL, "version_id_minor", 
                NC_INT,   (size_t)(1), &l2b.header.version_id_minor));

    tmp = l2b.frame.swath.GetCrossTrackBins();
    NCERR(nc_put_att_int  (ncid, NC_GLOBAL, "cross_track_bins", 
                NC_INT,   (size_t)(1), &tmp));
    tmp = l2b.frame.swath.GetAlongTrackBins();
    NCERR(nc_put_att_int  (ncid, NC_GLOBAL, "along_track_bins", 
                NC_INT,   (size_t)(1), &tmp));

    NCERR(nc_enddef(ncid));


    /* Same sneakiness with idx as described above with dimensions */
    size_t idx[3];
    WVC *wvc;
    unsigned char flags;
    const int zero = 0;
    const float zerof = 0.0f;
    float conversion;

    for (idx[1] = 0; (int)idx[1] < l2b.frame.swath.GetCrossTrackBins(); idx[1]++) {
        for (idx[0] = 0; (int)idx[0] < l2b.frame.swath.GetAlongTrackBins(); idx[0]++) {

            wvc = l2b.frame.swath.swath[idx[1]][idx[0]];

            NCERR(nc_put_var1_float(ncid, varlist[DIVERGENCE].id, idx, 
                        &zerof));
            NCERR(nc_put_var1_float(ncid, varlist[CURL].id, idx, 
                        &zerof));
            NCERR(nc_put_var1_float(ncid, varlist[WIND_STRESS].id, idx, 
                        &zerof));
            NCERR(nc_put_var1_float(ncid, varlist[WIND_STRESS_DIVERGENCE].id, idx, 
                        &zerof));
            NCERR(nc_put_var1_float(ncid, varlist[WIND_STRESS_CURL].id, idx, 
                        &zerof));
    

            /* && wvc->selected != NULL */
            if (wvc != NULL && wvc->selected != NULL) {
                flags = VALID_MASK;
                flags |= wvc->rainFlagBits    << 0;
                flags |= wvc->landiceFlagBits << 4;


                NCERR(nc_put_var1_float(ncid, varlist[LATITUDE].id, idx, 
                            &wvc->lonLat.latitude));
                NCERR(nc_put_var1_float(ncid, varlist[LONGITUDE].id, idx, 
                            &wvc->lonLat.longitude));
                NCERR(nc_put_var1_float(ncid, varlist[SEL_SPEED].id, idx, 
                            &wvc->selected->spd));
                conversion = (wvc->selected->dir)*180.0f/(float)(M_PI);
                NCERR(nc_put_var1_float(ncid, varlist[SEL_DIRECTION].id, idx, 
                            &conversion));
                NCERR(nc_put_var1_float(ncid, varlist[RAIN_IMPACT].id, idx, 
                            &wvc->rainImpact));
                NCERR(nc_put_var1_ubyte(ncid, varlist[FLAGS].id, idx, 
                        &flags));

                /* Extended variables */
                if (config.extended) {
                    NCERR(nc_put_var1_float(ncid, varlist[SEL_OBJ].id, idx,
                                &wvc->selected->obj));
    
                    NCERR(nc_put_var1_float(ncid, varlist[NUDGE_SPEED].id, idx, 
                                &wvc->nudgeWV->spd));
                    conversion = (wvc->nudgeWV->dir)*180.0f/(float)(M_PI);
                    NCERR(nc_put_var1_float(ncid, varlist[NUDGE_DIRECTION].id, idx, 
                                &conversion));
    
                    NCERR(nc_put_var1_ubyte(ncid, varlist[N_IN_FORE].id, idx, 
                                &wvc->numInFore));
                    NCERR(nc_put_var1_ubyte(ncid, varlist[N_IN_AFT].id, idx, 
                                &wvc->numInAft));
                    NCERR(nc_put_var1_ubyte(ncid, varlist[N_OUT_FORE].id, idx, 
                                &wvc->numOutFore));
                    NCERR(nc_put_var1_ubyte(ncid, varlist[N_OUT_AFT].id, idx, 
                                &wvc->numOutAft));
        
    
                    WindVectorPlus *wv = wvc->ambiguities.GetHead();
                    int num_ambiguities = wvc->ambiguities.NodeCount();
        
                    for (idx[2] = 0; (int)idx[2] < num_ambiguities && wv != NULL; 
                            idx[2]++, wv = wvc->ambiguities.GetNext()) {
        
                        NCERR(nc_put_var1(ncid, varlist[AMBIG_SPEED].id, 
                                    idx, &wv->spd));
                        conversion = (wv->dir)*180.0f/(float)(M_PI);
                        NCERR(nc_put_var1(ncid, varlist[AMBIG_DIRECTION].id, 
                                    idx, &conversion));
                        NCERR(nc_put_var1(ncid, varlist[AMBIG_OBJ].id, 
                                    idx, &wv->obj));
                    }
                    for ( ; (int)idx[2] < max_ambiguities; idx[2]++) {
                        NCERR(nc_put_var1(ncid, varlist[AMBIG_SPEED].id, 
                                    idx, &zerof));
                        NCERR(nc_put_var1(ncid, varlist[AMBIG_DIRECTION].id, 
                                    idx, &zerof));
                        NCERR(nc_put_var1(ncid, varlist[AMBIG_OBJ].id, 
                                    idx, &zerof));
                    }
                }    
            } else {
            
                NCERR(nc_put_var1_float(ncid, varlist[LATITUDE].id, idx, 
                            &zerof));
                NCERR(nc_put_var1_float(ncid, varlist[LONGITUDE].id, idx, 
                            &zerof));
                NCERR(nc_put_var1_float(ncid, varlist[SEL_SPEED].id, idx, 
                            &zerof));
                NCERR(nc_put_var1_float(ncid, varlist[SEL_DIRECTION].id, idx, 
                            &zerof));
                NCERR(nc_put_var1_float(ncid, varlist[RAIN_IMPACT].id, idx, 
                            &zerof));
                NCERR(nc_put_var1_ubyte(ncid, varlist[FLAGS].id, idx, 
                        &flags));


                /* Extended variables */
                if (config.extended) {
                    NCERR(nc_put_var1_float(ncid, varlist[NUDGE_SPEED].id, idx, 
                                &zerof));
                    NCERR(nc_put_var1_float(ncid, varlist[NUDGE_DIRECTION].id, idx, 
                                &zerof));
                    NCERR(nc_put_var1_float(ncid, varlist[SEL_OBJ].id, idx, 
                                &zerof));
                    NCERR(nc_put_var1_ubyte(ncid, varlist[N_IN_FORE].id, idx, 
                                (unsigned char *)&zero));
                    NCERR(nc_put_var1_ubyte(ncid, varlist[N_IN_AFT].id, idx, 
                                (unsigned char *)&zero));
                    NCERR(nc_put_var1_ubyte(ncid, varlist[N_OUT_FORE].id, idx, 
                                (unsigned char *)&zero));
                    NCERR(nc_put_var1_ubyte(ncid, varlist[N_OUT_AFT].id, idx, 
                                (unsigned char *)&zero));
        
                    for (idx[2] = 0; (int)idx[2] < max_ambiguities; idx[2]++) {
        
                        NCERR(nc_put_var1(ncid, varlist[AMBIG_SPEED].id, 
                                    idx, &zero));
                        NCERR(nc_put_var1(ncid, varlist[AMBIG_DIRECTION].id, 
                                    idx, &zero));
                        NCERR(nc_put_var1(ncid, varlist[AMBIG_OBJ].id, 
                                    idx, &zero));
                    }
                }
            }
        }
    }

    NCERR(nc_close(ncid));

    //----------------------//
    // close files and exit //
    //----------------------//

    l2b.Close();
    return(0);
}


int parse_commandline(int argc, char **argv, l2b_to_netcdf_config *config) {

    const char* usage_array = "--input=<input_file>" "--output=<output_file>" "[--extended]";
    int opt;

    /* Initialize configuration structure */
    config->command = no_path(argv[0]);
    config->input_file = NULL;
    config->output_file = NULL;
    config->extended = 0;

    struct option longopts[] = 
    {
        { "input", required_argument, NULL, 'i'},
        { "output", required_argument, NULL, 'o'},
        { "extended", no_argument, NULL, 'e'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "i:o:e", longopts, NULL)) != -1) {
        switch (opt) {
            case 'i': 
                config->input_file = optarg;
                break;
            case 'o':
                config->output_file = optarg;
                break;
            case 'e':
                config->extended = 1;
                break;
        }

    }

    if (config->input_file == NULL || config->output_file == NULL) {
        fprintf(stderr, "%s: %s\n", config->command, usage_array);
        return -1;
    }

    return 0;
}

