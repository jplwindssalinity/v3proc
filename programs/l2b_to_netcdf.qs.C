//==============================================================//
// Copyright (C) 1998-2010, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    l2b_to_netcdf
//
// SYNOPSIS
//    l2b_to_netcdf <l2b_file> <l2bc_file> 
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

#include <netcdf.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <mfhdf.h>
#include <time.h>
#include <unistd.h>

#include "AngleInterval.h"
#include "NetCDF.h"
#include "Misc.h"
#include "L2B.h"
#include "List.h"
#include "BufferedList.h"
#include "Tracking.h"
#include "Constants.h"
#include "Wind.h"


//--------//
// MACROS //
//--------//
#define STRINGIFY(x) #x
#define S(x) STRINGIFY(x)

#ifdef ZERO_FILL
    #define FILL(x, y) (x)
#else
    #define FILL(x, y) (y)
#endif

#define ERR(call) \
    if (call) { \
        perror("Error in " __FILE__ " @ " S(__LINE__)); \
        return EXIT_FAILURE; \
    }
    
#define NCERR(call) \
{ \
    int error; \
    if ((error = call) != NC_NOERR) { \
        fprintf(stderr, "Error %d in %s @ %d: %s\n", error, \
                __FILE__, __LINE__, nc_strerror(error)); \
        return EXIT_FAILURE; \
    } \
}

#define HDFERR(call) \
    if (call) { \
        fprintf(stderr, "Error in " __FILE__ " @ " S(__LINE__) ":\n"); \
        HEprint(stderr, 0); \
        return EXIT_FAILURE; \
    }

// Bit twiddling operations
#define SET_IF(var, mask, cond) {\
    if (cond) { \
        var |= (mask); \
    } \
}
#define UNSET_IF(var, mask, cond) {\
    if (cond) { \
        var &= ~(mask); \
    } \
}
#define SET(var, mask)   SET_IF(var, mask, 1)
#define UNSET(var, mask) UNSET_IF(var, mask, 1)
#define IS_SET(var, mask) ((var) & (mask))
#define IS_NOT_SET(var, mask) (!IS_SET(var, mask))

const size_t NUM_FLAGS = 10;
enum {
    SIGMA0_MASK               = 0x0001,
    AZIMUTH_DIV_MASK          = 0x0002,
    COASTAL_MASK              = 0x0080,
    ICE_EDGE_MASK             = 0x0100,
    WIND_RETRIEVAL_MASK       = 0x0200,
    HIGH_WIND_MASK            = 0x0400,
    LOW_WIND_MASK             = 0x0800,
    RAIN_IMPACT_UNUSABLE_MASK = 0x1000,
    RAIN_IMPACT_MASK          = 0x2000,
    AVAILABLE_DATA_MASK       = 0x4000
};

const size_t NUM_EFLAGS = 4;
enum {
    RAIN_CORR_NOT_APPL_MASK = 0x0001,
    NEG_WIND_SPEED_MASK     = 0x0002,
    ALL_AMBIG_CONTRIB_MASK  = 0x0004,
    RAIN_CORR_LARGE_MASK    = 0x0008
};

//-----------//
// TEMPLATES //
//-----------//

typedef enum {
    TIME = 0,
    LATITUDE,
    LONGITUDE,
    SEL_SPEED,
    SEL_DIRECTION,
    RAIN_IMPACT,
    WIND_DIVERGENCE,
    WIND_CURL,
    WIND_STRESS,
    STRESS_DIVERGENCE,
    STRESS_CURL,
    FLAGS,
    EFLAGS,
    NUDGE_SPEED,
    NUDGE_DIRECTION,
    WIND_SPEED_UNCORRECTED,
    NUM_AMBIG,

    first_extended_variable,

    SEL_OBJ = first_extended_variable,
    NUM_MEDFILT_AMBIG,
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
    const char *l2b_file;
    const char *l2bhdf_file;
    const char *l2bc_file;
    const char *l1bhdf_file;
    char extended;
} l2b_to_netcdf_config;

typedef struct {
    const char *name;
    nc_type type;
    size_t size;
    union {
        float f;
        float *pf;
        int32 i;
        int32 *pi;
        int16 s;
        int16 *ps;
        unsigned char b;
        unsigned char *pb;
        const char *str;
    } value;
} attribute;

typedef struct {
    const char *name;
    nc_type type;
    int ndims;
    int *dims;
    int id;
    int nattrs;
    attribute *attrs;
} netcdf_variable;

typedef struct {
    float lambda_0;
    float inclination;
    float rev_period;
    int xt_steps;
    double at_res;
    double xt_res;
} latlon_config;

int parse_commandline(int argc, char **argv, 
        l2b_to_netcdf_config *config);
int copy_l2bhdf_attributes(int ncid, int hdfid);
static void bin_to_latlon(int at_ind, int ct_ind,
        const latlon_config *config, float *lat, float *lon);
static int write_history(int ncid, int argc, const char * const * argv);


//--------------//
// MAIN PROGRAM //
//--------------//

int main(int argc, char **argv) {

    l2b_to_netcdf_config run_config;
    L2B l2b;

    /* File IDs */
    int l2bhdf_fid, l2bhdf_sds_fid, ncid;

    /* Net CDF dimension information */
    int max_ambiguities;
    int cross_track_dim_id, along_track_dim_id, ambiguities_dim_id, time_strlen_dim_id;
    int dimensions[3];
    int time_dimensions[2];
    size_t time_vara_length[2] = {1, 0};

    /* For extracting times from L2B HDF file */
    const char time_vdata_name[]  = "wvc_row_time";
    const char time_vdata_fname[] = "wvc_row_time";
    int time_vdata_ref, time_vdata_id, time_vdata_fsize;
    char *buffer;


    /* For extracting flags data from L2B HDF file */
    const char flags_sds_name[]  = "wvc_quality_flag";
    int flags_sds_idx, flags_sds_id;
    unsigned short *hdf_flags;

    /* For populating Net CDF file */
    size_t idx[3], time_idx[2] = {0, 0};
    WVC *wvc;
    int16 flags, eflags;
    float conversion;
    char time_format[] = "YYYY-DDDTHH:MM:SS.SSS";

    float lat, lon;

    // parse the command line
    if (parse_commandline(argc, argv, &run_config) != 0) {
        return -1;
    }

    // open the input files
    ERR(l2b.OpenForReading(run_config.l2b_file) == 0);
    HDFERR((l2bhdf_fid = Hopen(run_config.l2bhdf_file, 
                    DFACC_READ, 0)) == FAIL);

    ERR(l2b.ReadHeader() == 0); 
    ERR(l2b.ReadDataRec() == 0);

    l2b.header.version_id_major++;

    /********************************************
     * Build NetCDF DB                          *
     *******************************************/

    /* Here we're going to store a set of attributes, and a swath of
     * wind vector cell ambiguities.  The format of the swath is:
     * a 2-dimensional matrix (cross-track x along_track); each 
     * point is a variable length array (ambiguities); each element
     * of the array is a (u,v) two-float compounddata type.
     */

    max_ambiguities = l2b.frame.swath.GetMaxAmbiguityCount();

    // Initialize the NetCDF DB
    NCERR(nc_create(run_config.l2bc_file, 0, &ncid));

    ERR(copy_l2bhdf_attributes(ncid, l2bhdf_fid) != 0);

    // Create the data dimensions
    NCERR(nc_def_dim(ncid, "along_track", 
                (size_t)l2b.frame.swath.GetAlongTrackBins(), 
                &along_track_dim_id));
    NCERR(nc_def_dim(ncid, "cross_track", 
                (size_t)l2b.frame.swath.GetCrossTrackBins(), 
                &cross_track_dim_id));
    NCERR(nc_def_dim(ncid, "time_strlength", 
                (size_t)strlen(time_format), 
                &time_strlen_dim_id));
    if (run_config.extended) {
        NCERR(nc_def_dim(ncid, "ambiguities",
                (size_t)max_ambiguities, &ambiguities_dim_id));
    }

    /*************************************************************
     *
     * Declare the NetCDF variables
     *
     ************************************************************/

    /* We're being sneaky here.  We can pass dimensions to both two-
     * and three-dimensional variables, since they all use the same
     * first two dimensions.
     */
    dimensions[0] = along_track_dim_id;
    dimensions[1] = cross_track_dim_id;
    dimensions[2] = ambiguities_dim_id;

    time_dimensions[0] = along_track_dim_id;
    time_dimensions[1] = time_strlen_dim_id;

    netcdf_variable varlist[num_variables];
   
    varlist[LATITUDE].name  = "lat";
    varlist[LATITUDE].type  = NC_FLOAT;
    varlist[LATITUDE].ndims = 2;
    varlist[LATITUDE].dims = dimensions; 
    varlist[LATITUDE].nattrs = num_standard_attrs + 2;
    ERR((varlist[LATITUDE].attrs = (typeof varlist[LATITUDE].attrs)
                malloc(varlist[LATITUDE].nattrs * 
                    (sizeof *varlist[LATITUDE].attrs))) == NULL);

    varlist[LATITUDE].attrs[FILL_VALUE].name = "FillValue";
    varlist[LATITUDE].attrs[FILL_VALUE].size = 1;
    varlist[LATITUDE].attrs[FILL_VALUE].type = varlist[LATITUDE].type;
    varlist[LATITUDE].attrs[FILL_VALUE].value.f = FILL(0.0f, -9999.0);
    varlist[LATITUDE].attrs[VALID_MIN].name = "valid_min";
    varlist[LATITUDE].attrs[VALID_MIN].size = 1;
    varlist[LATITUDE].attrs[VALID_MIN].type = varlist[LATITUDE].type;
    varlist[LATITUDE].attrs[VALID_MIN].value.f = -90.0f;
    varlist[LATITUDE].attrs[VALID_MAX].name = "valid_max";
    varlist[LATITUDE].attrs[VALID_MAX].size = 1;
    varlist[LATITUDE].attrs[VALID_MAX].type = varlist[LATITUDE].type;
    varlist[LATITUDE].attrs[VALID_MAX].value.f =  90.0f;
    varlist[LATITUDE].attrs[LONG_NAME].name = "long_name";
    varlist[LATITUDE].attrs[LONG_NAME].size = 1;
    varlist[LATITUDE].attrs[LONG_NAME].type = NC_CHAR;
    varlist[LATITUDE].attrs[LONG_NAME].value.str = "latitude";
    varlist[LATITUDE].attrs[num_standard_attrs].name = "units";
    varlist[LATITUDE].attrs[num_standard_attrs].size = 1;
    varlist[LATITUDE].attrs[num_standard_attrs].type = NC_CHAR;
    varlist[LATITUDE].attrs[num_standard_attrs].value.str = "degrees_north";
    varlist[LATITUDE].attrs[num_standard_attrs + 1].name = "scale_factor";
    varlist[LATITUDE].attrs[num_standard_attrs + 1].size = 1;
    varlist[LATITUDE].attrs[num_standard_attrs + 1].type = NC_FLOAT;
    varlist[LATITUDE].attrs[num_standard_attrs + 1].value.f = 1.0f;

    varlist[LONGITUDE].name  = "lon";
    varlist[LONGITUDE].type  = NC_FLOAT;
    varlist[LONGITUDE].ndims = 2;
    varlist[LONGITUDE].dims = dimensions; 
    varlist[LONGITUDE].nattrs = num_standard_attrs + 2;
    ERR((varlist[LONGITUDE].attrs = (typeof varlist[LONGITUDE].attrs)
                malloc(varlist[LONGITUDE].nattrs * 
                    (sizeof *varlist[LONGITUDE].attrs))) == NULL);

    varlist[LONGITUDE].attrs[FILL_VALUE].name = "FillValue";
    varlist[LONGITUDE].attrs[FILL_VALUE].size = 1;
    varlist[LONGITUDE].attrs[FILL_VALUE].type = varlist[LONGITUDE].type;
    varlist[LONGITUDE].attrs[FILL_VALUE].value.f = FILL(0.0f, -9999.0);
    varlist[LONGITUDE].attrs[VALID_MIN].name = "valid_min";
    varlist[LONGITUDE].attrs[VALID_MIN].size = 1;
    varlist[LONGITUDE].attrs[VALID_MIN].type = varlist[LONGITUDE].type;
    varlist[LONGITUDE].attrs[VALID_MIN].value.f = 0.0f;
    varlist[LONGITUDE].attrs[VALID_MAX].name = "valid_max";
    varlist[LONGITUDE].attrs[VALID_MAX].size = 1;
    varlist[LONGITUDE].attrs[VALID_MAX].type = varlist[LONGITUDE].type;
    varlist[LONGITUDE].attrs[VALID_MAX].value.f = 360.0f;
    varlist[LONGITUDE].attrs[LONG_NAME].name = "long_name";
    varlist[LONGITUDE].attrs[LONG_NAME].size = 1;
    varlist[LONGITUDE].attrs[LONG_NAME].type = NC_CHAR;
    varlist[LONGITUDE].attrs[LONG_NAME].value.str = "longitude";
    varlist[LONGITUDE].attrs[num_standard_attrs].name = "units";
    varlist[LONGITUDE].attrs[num_standard_attrs].size = 1;
    varlist[LONGITUDE].attrs[num_standard_attrs].type = NC_CHAR;
    varlist[LONGITUDE].attrs[num_standard_attrs].value.str = "degrees_east";
    varlist[LONGITUDE].attrs[num_standard_attrs + 1].name = "scale_factor";
    varlist[LONGITUDE].attrs[num_standard_attrs + 1].size = 1;
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
    varlist[SEL_SPEED].attrs[FILL_VALUE].size = 1;
    varlist[SEL_SPEED].attrs[FILL_VALUE].type = varlist[SEL_SPEED].type;
    varlist[SEL_SPEED].attrs[FILL_VALUE].value.f = FILL(0.0f, -9999.0);
    varlist[SEL_SPEED].attrs[VALID_MIN].name = "valid_min";
    varlist[SEL_SPEED].attrs[VALID_MIN].size = 1;
    varlist[SEL_SPEED].attrs[VALID_MIN].type = varlist[SEL_SPEED].type;
    varlist[SEL_SPEED].attrs[VALID_MIN].value.f = 0.0f;
    varlist[SEL_SPEED].attrs[VALID_MAX].name = "valid_max";
    varlist[SEL_SPEED].attrs[VALID_MAX].size = 1;
    varlist[SEL_SPEED].attrs[VALID_MAX].type = varlist[SEL_SPEED].type;
    varlist[SEL_SPEED].attrs[VALID_MAX].value.f = 100.0f;
    varlist[SEL_SPEED].attrs[LONG_NAME].name = "long_name";
    varlist[SEL_SPEED].attrs[LONG_NAME].size = 1;
    varlist[SEL_SPEED].attrs[LONG_NAME].type = NC_CHAR;
    varlist[SEL_SPEED].attrs[LONG_NAME].value.str = "equivalent neutral wind speed at 10 m";
    varlist[SEL_SPEED].attrs[num_standard_attrs].name = "units";
    varlist[SEL_SPEED].attrs[num_standard_attrs].size = 1;
    varlist[SEL_SPEED].attrs[num_standard_attrs].type = NC_CHAR;
    varlist[SEL_SPEED].attrs[num_standard_attrs].value.str = "m s-1";
    varlist[SEL_SPEED].attrs[num_standard_attrs + 1].name = "scale_factor";
    varlist[SEL_SPEED].attrs[num_standard_attrs + 1].size = 1;
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
    varlist[SEL_DIRECTION].attrs[FILL_VALUE].size = 1;
    varlist[SEL_DIRECTION].attrs[FILL_VALUE].type = varlist[SEL_DIRECTION].type;
    varlist[SEL_DIRECTION].attrs[FILL_VALUE].value.f = FILL(0.0f, -9999.0);
    varlist[SEL_DIRECTION].attrs[VALID_MIN].name = "valid_min";
    varlist[SEL_DIRECTION].attrs[VALID_MIN].size = 1;
    varlist[SEL_DIRECTION].attrs[VALID_MIN].type = varlist[SEL_DIRECTION].type;
    varlist[SEL_DIRECTION].attrs[VALID_MIN].value.f = 0.0f;
    varlist[SEL_DIRECTION].attrs[VALID_MAX].name = "valid_max";
    varlist[SEL_DIRECTION].attrs[VALID_MAX].size = 1;
    varlist[SEL_DIRECTION].attrs[VALID_MAX].type = varlist[SEL_DIRECTION].type;
    varlist[SEL_DIRECTION].attrs[VALID_MAX].value.f = 360.0f;
    varlist[SEL_DIRECTION].attrs[LONG_NAME].name = "long_name";
    varlist[SEL_DIRECTION].attrs[LONG_NAME].size = 1;
    varlist[SEL_DIRECTION].attrs[LONG_NAME].type = NC_CHAR;
    varlist[SEL_DIRECTION].attrs[LONG_NAME].value.str = "equivalent neutral wind direction at 10 m";
    varlist[SEL_DIRECTION].attrs[num_standard_attrs].name = "units";
    varlist[SEL_DIRECTION].attrs[num_standard_attrs].size = 1;
    varlist[SEL_DIRECTION].attrs[num_standard_attrs].type = NC_CHAR;
    varlist[SEL_DIRECTION].attrs[num_standard_attrs].value.str = "degrees";
    varlist[SEL_DIRECTION].attrs[num_standard_attrs + 1].name = "scale_factor";
    varlist[SEL_DIRECTION].attrs[num_standard_attrs + 1].size = 1;
    varlist[SEL_DIRECTION].attrs[num_standard_attrs + 1].type = NC_FLOAT;
    varlist[SEL_DIRECTION].attrs[num_standard_attrs + 1].value.f = 1.0f;
   
    varlist[RAIN_IMPACT].name  = "rain_impact";
    varlist[RAIN_IMPACT].type  = NC_FLOAT;
    varlist[RAIN_IMPACT].ndims = 2;
    varlist[RAIN_IMPACT].dims = dimensions; 
    varlist[RAIN_IMPACT].nattrs = num_standard_attrs + 1;
    ERR((varlist[RAIN_IMPACT].attrs = (typeof varlist[RAIN_IMPACT].attrs)
                malloc(varlist[RAIN_IMPACT].nattrs * 
                    (sizeof *varlist[RAIN_IMPACT].attrs))) == NULL);

    varlist[RAIN_IMPACT].attrs[FILL_VALUE].name = "FillValue";
    varlist[RAIN_IMPACT].attrs[FILL_VALUE].size = 1;
    varlist[RAIN_IMPACT].attrs[FILL_VALUE].type = varlist[RAIN_IMPACT].type;
    varlist[RAIN_IMPACT].attrs[FILL_VALUE].value.f = FILL(0.0f, -9999.0);
    varlist[RAIN_IMPACT].attrs[VALID_MIN].name = "valid_min";
    varlist[RAIN_IMPACT].attrs[VALID_MIN].size = 1;
    varlist[RAIN_IMPACT].attrs[VALID_MIN].type = varlist[RAIN_IMPACT].type;
    varlist[RAIN_IMPACT].attrs[VALID_MIN].value.f = 0.0f;
    varlist[RAIN_IMPACT].attrs[VALID_MAX].name = "valid_max";
    varlist[RAIN_IMPACT].attrs[VALID_MAX].size = 1;
    varlist[RAIN_IMPACT].attrs[VALID_MAX].type = varlist[RAIN_IMPACT].type;
    varlist[RAIN_IMPACT].attrs[VALID_MAX].value.f = 100.0;
    varlist[RAIN_IMPACT].attrs[LONG_NAME].name = "long_name";
    varlist[RAIN_IMPACT].attrs[LONG_NAME].size = 1;
    varlist[RAIN_IMPACT].attrs[LONG_NAME].type = NC_CHAR;
    varlist[RAIN_IMPACT].attrs[LONG_NAME].value.str = "impact of rain upon wind vector retrieval";
    varlist[RAIN_IMPACT].attrs[num_standard_attrs].name = "units";
    varlist[RAIN_IMPACT].attrs[num_standard_attrs].size = 1;
    varlist[RAIN_IMPACT].attrs[num_standard_attrs].type = NC_CHAR;
    varlist[RAIN_IMPACT].attrs[num_standard_attrs].value.str = "1";
  
    varlist[WIND_DIVERGENCE].name  = "wind_divergence";
    varlist[WIND_DIVERGENCE].type  = NC_FLOAT;
    varlist[WIND_DIVERGENCE].ndims = 2;
    varlist[WIND_DIVERGENCE].dims = dimensions; 
    varlist[WIND_DIVERGENCE].nattrs = num_standard_attrs + 2;
    ERR((varlist[WIND_DIVERGENCE].attrs = (typeof varlist[WIND_DIVERGENCE].attrs)
                malloc(varlist[WIND_DIVERGENCE].nattrs * 
                    (sizeof *varlist[WIND_DIVERGENCE].attrs))) == NULL);

    varlist[WIND_DIVERGENCE].attrs[FILL_VALUE].name = "FillValue";
    varlist[WIND_DIVERGENCE].attrs[FILL_VALUE].size = 1;
    varlist[WIND_DIVERGENCE].attrs[FILL_VALUE].type = varlist[WIND_DIVERGENCE].type;
    varlist[WIND_DIVERGENCE].attrs[FILL_VALUE].value.f = FILL(0.0f, -9999.0);
    varlist[WIND_DIVERGENCE].attrs[VALID_MIN].name = "valid_min";
    varlist[WIND_DIVERGENCE].attrs[VALID_MIN].size = 1;
    varlist[WIND_DIVERGENCE].attrs[VALID_MIN].type = varlist[WIND_DIVERGENCE].type;
    varlist[WIND_DIVERGENCE].attrs[VALID_MIN].value.f = -1.0f;
    varlist[WIND_DIVERGENCE].attrs[VALID_MAX].name = "valid_max";
    varlist[WIND_DIVERGENCE].attrs[VALID_MAX].size = 1;
    varlist[WIND_DIVERGENCE].attrs[VALID_MAX].type = varlist[WIND_DIVERGENCE].type;
    varlist[WIND_DIVERGENCE].attrs[VALID_MAX].value.f = 1.0f;
    varlist[WIND_DIVERGENCE].attrs[LONG_NAME].name = "long_name";
    varlist[WIND_DIVERGENCE].attrs[LONG_NAME].size = 1;
    varlist[WIND_DIVERGENCE].attrs[LONG_NAME].type = NC_CHAR;
    varlist[WIND_DIVERGENCE].attrs[LONG_NAME].value.str = "equivalent neutral wind-divergence at 10 m";
    varlist[WIND_DIVERGENCE].attrs[num_standard_attrs].name = "units";
    varlist[WIND_DIVERGENCE].attrs[num_standard_attrs].size = 1;
    varlist[WIND_DIVERGENCE].attrs[num_standard_attrs].type = NC_CHAR;
    varlist[WIND_DIVERGENCE].attrs[num_standard_attrs].value.str = "s-1";
    varlist[WIND_DIVERGENCE].attrs[num_standard_attrs + 1].name = "scale_factor";
    varlist[WIND_DIVERGENCE].attrs[num_standard_attrs + 1].size = 1;
    varlist[WIND_DIVERGENCE].attrs[num_standard_attrs + 1].type = NC_FLOAT;
    varlist[WIND_DIVERGENCE].attrs[num_standard_attrs + 1].value.f = 1.0f;
 
    varlist[WIND_CURL].name  = "wind_curl";
    varlist[WIND_CURL].type  = NC_FLOAT;
    varlist[WIND_CURL].ndims = 2;
    varlist[WIND_CURL].dims = dimensions; 
    varlist[WIND_CURL].nattrs = num_standard_attrs + 2;
    ERR((varlist[WIND_CURL].attrs = (typeof varlist[WIND_CURL].attrs)
                malloc(varlist[WIND_CURL].nattrs * 
                    (sizeof *varlist[WIND_CURL].attrs))) == NULL);

    varlist[WIND_CURL].attrs[FILL_VALUE].name = "FillValue";
    varlist[WIND_CURL].attrs[FILL_VALUE].size = 1;
    varlist[WIND_CURL].attrs[FILL_VALUE].type = varlist[WIND_CURL].type;
    varlist[WIND_CURL].attrs[FILL_VALUE].value.f = FILL(0.0f, -9999.0);
    varlist[WIND_CURL].attrs[VALID_MIN].name = "valid_min";
    varlist[WIND_CURL].attrs[VALID_MIN].size = 1;
    varlist[WIND_CURL].attrs[VALID_MIN].type = varlist[WIND_CURL].type;
    varlist[WIND_CURL].attrs[VALID_MIN].value.f = -1.0f;
    varlist[WIND_CURL].attrs[VALID_MAX].name = "valid_max";
    varlist[WIND_CURL].attrs[VALID_MAX].size = 1;
    varlist[WIND_CURL].attrs[VALID_MAX].type = varlist[WIND_CURL].type;
    varlist[WIND_CURL].attrs[VALID_MAX].value.f = 1.0f;
    varlist[WIND_CURL].attrs[LONG_NAME].name = "long_name";
    varlist[WIND_CURL].attrs[LONG_NAME].size = 1;
    varlist[WIND_CURL].attrs[LONG_NAME].type = NC_CHAR;
    varlist[WIND_CURL].attrs[LONG_NAME].value.str = "equivalent neutral wind-curl at 10 m";
    varlist[WIND_CURL].attrs[num_standard_attrs].name = "units";
    varlist[WIND_CURL].attrs[num_standard_attrs].size = 1;
    varlist[WIND_CURL].attrs[num_standard_attrs].type = NC_CHAR;
    varlist[WIND_CURL].attrs[num_standard_attrs].value.str = "s-1";
    varlist[WIND_CURL].attrs[num_standard_attrs + 1].name = "scale_factor";
    varlist[WIND_CURL].attrs[num_standard_attrs + 1].size = 1;
    varlist[WIND_CURL].attrs[num_standard_attrs + 1].type = NC_FLOAT;
    varlist[WIND_CURL].attrs[num_standard_attrs + 1].value.f = 1.0f;

    varlist[WIND_STRESS].name  = "stress";
    varlist[WIND_STRESS].type  = NC_FLOAT;
    varlist[WIND_STRESS].ndims = 2;
    varlist[WIND_STRESS].dims = dimensions; 
    varlist[WIND_STRESS].nattrs = num_standard_attrs + 2;
    ERR((varlist[WIND_STRESS].attrs = (typeof varlist[WIND_STRESS].attrs)
                malloc(varlist[WIND_STRESS].nattrs * 
                    (sizeof *varlist[WIND_STRESS].attrs))) == NULL);

    varlist[WIND_STRESS].attrs[FILL_VALUE].name = "FillValue";
    varlist[WIND_STRESS].attrs[FILL_VALUE].size = 1;
    varlist[WIND_STRESS].attrs[FILL_VALUE].type = varlist[WIND_STRESS].type;
    varlist[WIND_STRESS].attrs[FILL_VALUE].value.f = FILL(0.0f, -9999.0);
    varlist[WIND_STRESS].attrs[VALID_MIN].name = "valid_min";
    varlist[WIND_STRESS].attrs[VALID_MIN].size = 1;
    varlist[WIND_STRESS].attrs[VALID_MIN].type = varlist[WIND_STRESS].type;
    varlist[WIND_STRESS].attrs[VALID_MIN].value.f = -9999.0;
    varlist[WIND_STRESS].attrs[VALID_MAX].name = "valid_max";
    varlist[WIND_STRESS].attrs[VALID_MAX].size = 1;
    varlist[WIND_STRESS].attrs[VALID_MAX].type = varlist[WIND_STRESS].type;
    varlist[WIND_STRESS].attrs[VALID_MAX].value.f = 9999.0;
    varlist[WIND_STRESS].attrs[LONG_NAME].name = "long_name";
    varlist[WIND_STRESS].attrs[LONG_NAME].size = 1;
    varlist[WIND_STRESS].attrs[LONG_NAME].type = NC_CHAR;
    varlist[WIND_STRESS].attrs[LONG_NAME].value.str = "equivalent neutral wind-stress";
    varlist[WIND_STRESS].attrs[num_standard_attrs].name = "units";
    varlist[WIND_STRESS].attrs[num_standard_attrs].size = 1;
    varlist[WIND_STRESS].attrs[num_standard_attrs].type = NC_CHAR;
    varlist[WIND_STRESS].attrs[num_standard_attrs].value.str = "s-1";
    varlist[WIND_STRESS].attrs[num_standard_attrs + 1].name = "scale_factor";
    varlist[WIND_STRESS].attrs[num_standard_attrs + 1].size = 1;
    varlist[WIND_STRESS].attrs[num_standard_attrs + 1].type = NC_FLOAT;
    varlist[WIND_STRESS].attrs[num_standard_attrs + 1].value.f = 1.0f;

    
    varlist[STRESS_DIVERGENCE].name  = "stress_divergence";
    varlist[STRESS_DIVERGENCE].type  = NC_FLOAT;
    varlist[STRESS_DIVERGENCE].ndims = 2;
    varlist[STRESS_DIVERGENCE].dims = dimensions; 
    varlist[STRESS_DIVERGENCE].nattrs = num_standard_attrs + 2;
    ERR((varlist[STRESS_DIVERGENCE].attrs = (typeof varlist[STRESS_DIVERGENCE].attrs)
                malloc(varlist[STRESS_DIVERGENCE].nattrs * 
                    (sizeof *varlist[STRESS_DIVERGENCE].attrs))) == NULL);

    varlist[STRESS_DIVERGENCE].attrs[FILL_VALUE].name = "FillValue";
    varlist[STRESS_DIVERGENCE].attrs[FILL_VALUE].size = 1;
    varlist[STRESS_DIVERGENCE].attrs[FILL_VALUE].type = varlist[STRESS_DIVERGENCE].type;
    varlist[STRESS_DIVERGENCE].attrs[FILL_VALUE].value.f = FILL(0.0f, -9999.0);
    varlist[STRESS_DIVERGENCE].attrs[VALID_MIN].name = "valid_min";
    varlist[STRESS_DIVERGENCE].attrs[VALID_MIN].size = 1;
    varlist[STRESS_DIVERGENCE].attrs[VALID_MIN].type = varlist[STRESS_DIVERGENCE].type;
    varlist[STRESS_DIVERGENCE].attrs[VALID_MIN].value.f = -9999.0;
    varlist[STRESS_DIVERGENCE].attrs[VALID_MAX].name = "valid_max";
    varlist[STRESS_DIVERGENCE].attrs[VALID_MAX].size = 1;
    varlist[STRESS_DIVERGENCE].attrs[VALID_MAX].type = varlist[STRESS_DIVERGENCE].type;
    varlist[STRESS_DIVERGENCE].attrs[VALID_MAX].value.f = 9999.0;
    varlist[STRESS_DIVERGENCE].attrs[LONG_NAME].name = "long_name";
    varlist[STRESS_DIVERGENCE].attrs[LONG_NAME].size = 1;
    varlist[STRESS_DIVERGENCE].attrs[LONG_NAME].type = NC_CHAR;
    varlist[STRESS_DIVERGENCE].attrs[LONG_NAME].value.str = "equivalent neutral wind-stress-divergence";
    varlist[STRESS_DIVERGENCE].attrs[num_standard_attrs].name = "units";
    varlist[STRESS_DIVERGENCE].attrs[num_standard_attrs].size = 1;
    varlist[STRESS_DIVERGENCE].attrs[num_standard_attrs].type = NC_CHAR;
    varlist[STRESS_DIVERGENCE].attrs[num_standard_attrs].value.str = "s-1";
    varlist[STRESS_DIVERGENCE].attrs[num_standard_attrs + 1].name = "scale_factor";
    varlist[STRESS_DIVERGENCE].attrs[num_standard_attrs + 1].size = 1;
    varlist[STRESS_DIVERGENCE].attrs[num_standard_attrs + 1].type = NC_FLOAT;
    varlist[STRESS_DIVERGENCE].attrs[num_standard_attrs + 1].value.f = 1.0f;

    varlist[STRESS_CURL].name  = "stress_curl";
    varlist[STRESS_CURL].type  = NC_FLOAT;
    varlist[STRESS_CURL].ndims = 2;
    varlist[STRESS_CURL].dims = dimensions; 
    varlist[STRESS_CURL].nattrs = num_standard_attrs + 2;
    ERR((varlist[STRESS_CURL].attrs = (typeof varlist[STRESS_CURL].attrs)
                malloc(varlist[STRESS_CURL].nattrs * 
                    (sizeof *varlist[STRESS_CURL].attrs))) == NULL);

    varlist[STRESS_CURL].attrs[FILL_VALUE].name = "FillValue";
    varlist[STRESS_CURL].attrs[FILL_VALUE].size = 1;
    varlist[STRESS_CURL].attrs[FILL_VALUE].type = varlist[STRESS_CURL].type;
    varlist[STRESS_CURL].attrs[FILL_VALUE].value.f = FILL(0.0f, -9999.0);
    varlist[STRESS_CURL].attrs[VALID_MIN].name = "valid_min";
    varlist[STRESS_CURL].attrs[VALID_MIN].size = 1;
    varlist[STRESS_CURL].attrs[VALID_MIN].type = varlist[STRESS_CURL].type;
    varlist[STRESS_CURL].attrs[VALID_MIN].value.f = -9999.0;
    varlist[STRESS_CURL].attrs[VALID_MAX].name = "valid_max";
    varlist[STRESS_CURL].attrs[VALID_MAX].size = 1;
    varlist[STRESS_CURL].attrs[VALID_MAX].type = varlist[STRESS_CURL].type;
    varlist[STRESS_CURL].attrs[VALID_MAX].value.f = 9999.0;
    varlist[STRESS_CURL].attrs[LONG_NAME].name = "long_name";
    varlist[STRESS_CURL].attrs[LONG_NAME].size = 1;
    varlist[STRESS_CURL].attrs[LONG_NAME].type = NC_CHAR;
    varlist[STRESS_CURL].attrs[LONG_NAME].value.str = "equivalent neutral wind-stress-curl";
    varlist[STRESS_CURL].attrs[num_standard_attrs].name = "units";
    varlist[STRESS_CURL].attrs[num_standard_attrs].size = 1;
    varlist[STRESS_CURL].attrs[num_standard_attrs].type = NC_CHAR;
    varlist[STRESS_CURL].attrs[num_standard_attrs].value.str = "s-1";
    varlist[STRESS_CURL].attrs[num_standard_attrs + 1].name = "scale_factor";
    varlist[STRESS_CURL].attrs[num_standard_attrs + 1].size = 1;
    varlist[STRESS_CURL].attrs[num_standard_attrs + 1].type = NC_FLOAT;
    varlist[STRESS_CURL].attrs[num_standard_attrs + 1].value.f = 1.0f;
    
    varlist[TIME].name = "time";
    varlist[TIME].type = NC_CHAR;
    varlist[TIME].ndims = 2;
    varlist[TIME].dims = time_dimensions;
    varlist[TIME].nattrs = num_standard_attrs + 1;
    ERR((varlist[TIME].attrs = (typeof varlist[TIME].attrs)
                malloc(varlist[TIME].nattrs * 
                    (sizeof *varlist[TIME].attrs))) == NULL);

    varlist[TIME].attrs[FILL_VALUE].name = NULL;
    varlist[TIME].attrs[FILL_VALUE].size = 1;
    varlist[TIME].attrs[FILL_VALUE].type = varlist[TIME].type;
    varlist[TIME].attrs[FILL_VALUE].value.str = NULL;
    varlist[TIME].attrs[VALID_MIN].name = NULL;
    varlist[TIME].attrs[VALID_MIN].size = 1;
    varlist[TIME].attrs[VALID_MIN].type = varlist[TIME].type;
    varlist[TIME].attrs[VALID_MIN].value.str = NULL;
    varlist[TIME].attrs[VALID_MAX].name = NULL;
    varlist[TIME].attrs[VALID_MAX].size = 1;
    varlist[TIME].attrs[VALID_MAX].type = varlist[TIME].type;
    varlist[TIME].attrs[VALID_MAX].value.str = NULL;
    varlist[TIME].attrs[LONG_NAME].name = "long_name";
    varlist[TIME].attrs[LONG_NAME].size = 1;
    varlist[TIME].attrs[LONG_NAME].type = varlist[TIME].type;
    varlist[TIME].attrs[LONG_NAME].value.str = "date and time";
    varlist[TIME].attrs[num_standard_attrs].name = "units";
    varlist[TIME].attrs[num_standard_attrs].size = 1;
    varlist[TIME].attrs[num_standard_attrs].type = varlist[TIME].type;
    varlist[TIME].attrs[num_standard_attrs].value.str = time_format;

    varlist[FLAGS].name  = "flags";
    varlist[FLAGS].type  = NC_SHORT;
    varlist[FLAGS].ndims = 2;
    varlist[FLAGS].dims = dimensions; 
    varlist[FLAGS].nattrs = num_standard_attrs + 2;
    ERR((varlist[FLAGS].attrs = (typeof varlist[FLAGS].attrs)
                malloc(varlist[FLAGS].nattrs * 
                    (sizeof *varlist[FLAGS].attrs))) == NULL);

    varlist[FLAGS].attrs[FILL_VALUE].name = "FillValue";
    varlist[FLAGS].attrs[FILL_VALUE].size = 1;
    varlist[FLAGS].attrs[FILL_VALUE].type = varlist[FLAGS].type;
    varlist[FLAGS].attrs[FILL_VALUE].value.s = 32767;
    varlist[FLAGS].attrs[VALID_MIN].name = "valid_min";
    varlist[FLAGS].attrs[VALID_MIN].size = 1;
    varlist[FLAGS].attrs[VALID_MIN].type = varlist[FLAGS].type;
    varlist[FLAGS].attrs[VALID_MIN].value.s = 0;
    varlist[FLAGS].attrs[VALID_MAX].name = "valid_max";
    varlist[FLAGS].attrs[VALID_MAX].size = 1;
    varlist[FLAGS].attrs[VALID_MAX].type = varlist[FLAGS].type;
    varlist[FLAGS].attrs[VALID_MAX].value.s = 
        varlist[FLAGS].attrs[FILL_VALUE].value.s;
    varlist[FLAGS].attrs[LONG_NAME].name = "long_name";
    varlist[FLAGS].attrs[LONG_NAME].size = 1;
    varlist[FLAGS].attrs[LONG_NAME].type = NC_CHAR;
    varlist[FLAGS].attrs[LONG_NAME].value.str = "wind vector cell quality flags";

    varlist[FLAGS].attrs[num_standard_attrs + 0].name = "flag_masks";
    varlist[FLAGS].attrs[num_standard_attrs + 0].size = NUM_FLAGS;
    varlist[FLAGS].attrs[num_standard_attrs + 0].type = varlist[FLAGS].type;
    ERR((varlist[FLAGS].attrs[num_standard_attrs + 0].value.ps = (int16 *)
                malloc(varlist[FLAGS].attrs[num_standard_attrs + 0].size *
                    sizeof(*varlist[FLAGS].attrs[num_standard_attrs + 0].value.ps)))
            == NULL);
    {
        int16 init_list[NUM_FLAGS] = {SIGMA0_MASK, AZIMUTH_DIV_MASK, COASTAL_MASK, 
            ICE_EDGE_MASK, WIND_RETRIEVAL_MASK, HIGH_WIND_MASK, LOW_WIND_MASK, 
            RAIN_IMPACT_UNUSABLE_MASK, RAIN_IMPACT_MASK, AVAILABLE_DATA_MASK};

        ERR(memcpy(varlist[FLAGS].attrs[num_standard_attrs + 0].value.ps, init_list, 
                    NUM_FLAGS* sizeof init_list[0]) == NULL);
    }

    varlist[FLAGS].attrs[num_standard_attrs + 1].name = "flag_meanings";
    varlist[FLAGS].attrs[num_standard_attrs + 1].size = 1;
    varlist[FLAGS].attrs[num_standard_attrs + 1].type = NC_CHAR;
    varlist[FLAGS].attrs[num_standard_attrs + 1].value.str = "adequate_sigma0_flag "
        "adequate_azimuth_diversity_flag coastal_flag ice_edge_flag wind_retrieval_flag "
        "high_wind_speed_flag low_wind_speed_flag rain_impact_flag_usable rain_impact_flag "
        "available_data_flag";


    varlist[EFLAGS].name  = "eflags";
    varlist[EFLAGS].type  = NC_SHORT;
    varlist[EFLAGS].ndims = 2;
    varlist[EFLAGS].dims = dimensions; 
    varlist[EFLAGS].nattrs = num_standard_attrs + 2;
    ERR((varlist[EFLAGS].attrs = (typeof varlist[EFLAGS].attrs)
                malloc(varlist[EFLAGS].nattrs * 
                    (sizeof *varlist[EFLAGS].attrs))) == NULL);

    varlist[EFLAGS].attrs[FILL_VALUE].name = "FillValue";
    varlist[EFLAGS].attrs[FILL_VALUE].size = 1;
    varlist[EFLAGS].attrs[FILL_VALUE].type = varlist[EFLAGS].type;
    varlist[EFLAGS].attrs[FILL_VALUE].value.s = 32767;
    varlist[EFLAGS].attrs[VALID_MIN].name = "valid_min";
    varlist[EFLAGS].attrs[VALID_MIN].size = 1;
    varlist[EFLAGS].attrs[VALID_MIN].type = varlist[EFLAGS].type;
    varlist[EFLAGS].attrs[VALID_MIN].value.s = 0;
    varlist[EFLAGS].attrs[VALID_MAX].name = "valid_max";
    varlist[EFLAGS].attrs[VALID_MAX].size = 1;
    varlist[EFLAGS].attrs[VALID_MAX].type = varlist[EFLAGS].type;
    varlist[EFLAGS].attrs[VALID_MAX].value.s = 
        varlist[EFLAGS].attrs[FILL_VALUE].value.s;
    varlist[EFLAGS].attrs[LONG_NAME].name = "long_name";
    varlist[EFLAGS].attrs[LONG_NAME].size = 1;
    varlist[EFLAGS].attrs[LONG_NAME].type = NC_CHAR;
    varlist[EFLAGS].attrs[LONG_NAME].value.str = "extended wind vector cell quality flags";

    varlist[EFLAGS].attrs[num_standard_attrs + 0].name = "flag_masks";
    varlist[EFLAGS].attrs[num_standard_attrs + 0].size = NUM_EFLAGS;
    varlist[EFLAGS].attrs[num_standard_attrs + 0].type = varlist[EFLAGS].type;
    ERR((varlist[EFLAGS].attrs[num_standard_attrs + 0].value.ps = (int16 *)
                malloc(varlist[EFLAGS].attrs[num_standard_attrs + 0].size *
                    sizeof(*varlist[EFLAGS].attrs[num_standard_attrs + 0].value.ps)))
            == NULL);
    {
        int16 init_list[NUM_EFLAGS] = {RAIN_CORR_NOT_APPL_MASK, NEG_WIND_SPEED_MASK, 
            ALL_AMBIG_CONTRIB_MASK, RAIN_CORR_LARGE_MASK};

        ERR(memcpy(varlist[EFLAGS].attrs[num_standard_attrs + 0].value.ps, init_list, 
                    NUM_EFLAGS* sizeof init_list[0]) == NULL);
    }

    varlist[EFLAGS].attrs[num_standard_attrs + 1].name = "flag_meanings";
    varlist[EFLAGS].attrs[num_standard_attrs + 1].size = 1;
    varlist[EFLAGS].attrs[num_standard_attrs + 1].type = NC_CHAR;
    varlist[EFLAGS].attrs[num_standard_attrs + 1].value.str = "rain_correction_not_applied_flag "
        "correction_produced_negative_spd_flag all_ambiguities_contribute_to_nudging_flag "
        "large_rain_correction_flag";

    varlist[NUDGE_SPEED].name  = "model_wind_speed";
    varlist[NUDGE_SPEED].type  = NC_FLOAT;
    varlist[NUDGE_SPEED].ndims = 2;
    varlist[NUDGE_SPEED].dims = dimensions; 
    varlist[NUDGE_SPEED].nattrs = num_standard_attrs + 2;
    ERR((varlist[NUDGE_SPEED].attrs = (typeof varlist[NUDGE_SPEED].attrs)
                malloc(varlist[NUDGE_SPEED].nattrs * 
                    (sizeof *varlist[NUDGE_SPEED].attrs))) == NULL);

    varlist[NUDGE_SPEED].attrs[FILL_VALUE].name = "FillValue";
    varlist[NUDGE_SPEED].attrs[FILL_VALUE].size = 1;
    varlist[NUDGE_SPEED].attrs[FILL_VALUE].type = varlist[NUDGE_SPEED].type;
    varlist[NUDGE_SPEED].attrs[FILL_VALUE].value.f = FILL(0.0f, -9999.0);
    varlist[NUDGE_SPEED].attrs[VALID_MIN].name = "valid_min";
    varlist[NUDGE_SPEED].attrs[VALID_MIN].size = 1;
    varlist[NUDGE_SPEED].attrs[VALID_MIN].type = varlist[NUDGE_SPEED].type;
    varlist[NUDGE_SPEED].attrs[VALID_MIN].value.f = 0.0f;
    varlist[NUDGE_SPEED].attrs[VALID_MAX].name = "valid_max";
    varlist[NUDGE_SPEED].attrs[VALID_MAX].size = 1;
    varlist[NUDGE_SPEED].attrs[VALID_MAX].type = varlist[NUDGE_SPEED].type;
    varlist[NUDGE_SPEED].attrs[VALID_MAX].value.f = 100.0f;
    varlist[NUDGE_SPEED].attrs[LONG_NAME].name = "long_name";
    varlist[NUDGE_SPEED].attrs[LONG_NAME].size = 1;
    varlist[NUDGE_SPEED].attrs[LONG_NAME].type = NC_CHAR;
    varlist[NUDGE_SPEED].attrs[LONG_NAME].value.str = "model wind speed";
    varlist[NUDGE_SPEED].attrs[num_standard_attrs].name = "units";
    varlist[NUDGE_SPEED].attrs[num_standard_attrs].size = 1;
    varlist[NUDGE_SPEED].attrs[num_standard_attrs].type = NC_CHAR;
    varlist[NUDGE_SPEED].attrs[num_standard_attrs].value.str = "m s-1";
    varlist[NUDGE_SPEED].attrs[num_standard_attrs + 1].name = "scale_factor";
    varlist[NUDGE_SPEED].attrs[num_standard_attrs + 1].size = 1;
    varlist[NUDGE_SPEED].attrs[num_standard_attrs + 1].type = NC_FLOAT;
    varlist[NUDGE_SPEED].attrs[num_standard_attrs + 1].value.f = 1.0f;
             
    varlist[NUDGE_DIRECTION].name  = "model_wind_direction";
    varlist[NUDGE_DIRECTION].type  = NC_FLOAT;
    varlist[NUDGE_DIRECTION].ndims = 2;
    varlist[NUDGE_DIRECTION].dims = dimensions; 
    varlist[NUDGE_DIRECTION].nattrs = num_standard_attrs + 2;
    ERR((varlist[NUDGE_DIRECTION].attrs = (typeof varlist[NUDGE_DIRECTION].attrs)
                malloc(varlist[NUDGE_DIRECTION].nattrs * 
                    (sizeof *varlist[NUDGE_DIRECTION].attrs))) == NULL);

    varlist[NUDGE_DIRECTION].attrs[FILL_VALUE].name = "FillValue";
    varlist[NUDGE_DIRECTION].attrs[FILL_VALUE].size = 1;
    varlist[NUDGE_DIRECTION].attrs[FILL_VALUE].type = varlist[NUDGE_DIRECTION].type;
    varlist[NUDGE_DIRECTION].attrs[FILL_VALUE].value.f = FILL(0.0f, -9999.0);
    varlist[NUDGE_DIRECTION].attrs[VALID_MIN].name = "valid_min";
    varlist[NUDGE_DIRECTION].attrs[VALID_MIN].size = 1;
    varlist[NUDGE_DIRECTION].attrs[VALID_MIN].type = varlist[NUDGE_DIRECTION].type;
    varlist[NUDGE_DIRECTION].attrs[VALID_MIN].value.f = 0.0f;
    varlist[NUDGE_DIRECTION].attrs[VALID_MAX].name = "valid_max";
    varlist[NUDGE_DIRECTION].attrs[VALID_MAX].size = 1;
    varlist[NUDGE_DIRECTION].attrs[VALID_MAX].type = varlist[NUDGE_DIRECTION].type;
    varlist[NUDGE_DIRECTION].attrs[VALID_MAX].value.f = 360.0f;
    varlist[NUDGE_DIRECTION].attrs[LONG_NAME].name = "long_name";
    varlist[NUDGE_DIRECTION].attrs[LONG_NAME].size = 1;
    varlist[NUDGE_DIRECTION].attrs[LONG_NAME].type = NC_CHAR;
    varlist[NUDGE_DIRECTION].attrs[LONG_NAME].value.str = "model wind direction";
    varlist[NUDGE_DIRECTION].attrs[num_standard_attrs].name = "units";
    varlist[NUDGE_DIRECTION].attrs[num_standard_attrs].size = 1;
    varlist[NUDGE_DIRECTION].attrs[num_standard_attrs].type = NC_CHAR;
    varlist[NUDGE_DIRECTION].attrs[num_standard_attrs].value.str = "degrees";
    varlist[NUDGE_DIRECTION].attrs[num_standard_attrs + 1].name = "scale_factor";
    varlist[NUDGE_DIRECTION].attrs[num_standard_attrs + 1].size = 1;
    varlist[NUDGE_DIRECTION].attrs[num_standard_attrs + 1].type = NC_FLOAT;
    varlist[NUDGE_DIRECTION].attrs[num_standard_attrs + 1].value.f = 1.0f;

    varlist[WIND_SPEED_UNCORRECTED].name  = "wind_speed_uncorrected";
    varlist[WIND_SPEED_UNCORRECTED].type  = NC_FLOAT;
    varlist[WIND_SPEED_UNCORRECTED].ndims = 2;
    varlist[WIND_SPEED_UNCORRECTED].dims = dimensions; 
    varlist[WIND_SPEED_UNCORRECTED].nattrs = num_standard_attrs + 2;
    ERR((varlist[WIND_SPEED_UNCORRECTED].attrs = (typeof varlist[WIND_SPEED_UNCORRECTED].attrs)
                malloc(varlist[WIND_SPEED_UNCORRECTED].nattrs * 
                    (sizeof *varlist[WIND_SPEED_UNCORRECTED].attrs))) == NULL);

    varlist[WIND_SPEED_UNCORRECTED].attrs[FILL_VALUE].name = "FillValue";
    varlist[WIND_SPEED_UNCORRECTED].attrs[FILL_VALUE].size = 1;
    varlist[WIND_SPEED_UNCORRECTED].attrs[FILL_VALUE].type = varlist[WIND_SPEED_UNCORRECTED].type;
    varlist[WIND_SPEED_UNCORRECTED].attrs[FILL_VALUE].value.f = FILL(0.0f, -9999.0);
    varlist[WIND_SPEED_UNCORRECTED].attrs[VALID_MIN].name = "valid_min";
    varlist[WIND_SPEED_UNCORRECTED].attrs[VALID_MIN].size = 1;
    varlist[WIND_SPEED_UNCORRECTED].attrs[VALID_MIN].type = varlist[WIND_SPEED_UNCORRECTED].type;
    varlist[WIND_SPEED_UNCORRECTED].attrs[VALID_MIN].value.f = 0.0f;
    varlist[WIND_SPEED_UNCORRECTED].attrs[VALID_MAX].name = "valid_max";
    varlist[WIND_SPEED_UNCORRECTED].attrs[VALID_MAX].size = 1;
    varlist[WIND_SPEED_UNCORRECTED].attrs[VALID_MAX].type = varlist[WIND_SPEED_UNCORRECTED].type;
    varlist[WIND_SPEED_UNCORRECTED].attrs[VALID_MAX].value.f = 100.0f;
    varlist[WIND_SPEED_UNCORRECTED].attrs[LONG_NAME].name = "long_name";
    varlist[WIND_SPEED_UNCORRECTED].attrs[LONG_NAME].size = 1;
    varlist[WIND_SPEED_UNCORRECTED].attrs[LONG_NAME].type = NC_CHAR;
    varlist[WIND_SPEED_UNCORRECTED].attrs[LONG_NAME].value.str = "wind speed without rain correction";
    varlist[WIND_SPEED_UNCORRECTED].attrs[num_standard_attrs].name = "units";
    varlist[WIND_SPEED_UNCORRECTED].attrs[num_standard_attrs].size = 1;
    varlist[WIND_SPEED_UNCORRECTED].attrs[num_standard_attrs].type = NC_CHAR;
    varlist[WIND_SPEED_UNCORRECTED].attrs[num_standard_attrs].value.str = "m s-1";
    varlist[WIND_SPEED_UNCORRECTED].attrs[num_standard_attrs + 1].name = "scale_factor";
    varlist[WIND_SPEED_UNCORRECTED].attrs[num_standard_attrs + 1].size = 1;
    varlist[WIND_SPEED_UNCORRECTED].attrs[num_standard_attrs + 1].type = NC_FLOAT;
    varlist[WIND_SPEED_UNCORRECTED].attrs[num_standard_attrs + 1].value.f = 1.0f;

    varlist[NUM_AMBIG].name  = "num_ambiguities";
    varlist[NUM_AMBIG].type  = NC_BYTE;
    varlist[NUM_AMBIG].ndims = 2;
    varlist[NUM_AMBIG].dims = dimensions; 
    varlist[NUM_AMBIG].nattrs = num_standard_attrs + 1;
    ERR((varlist[NUM_AMBIG].attrs = (typeof varlist[NUM_AMBIG].attrs)
                malloc(varlist[NUM_AMBIG].nattrs * 
                    (sizeof *varlist[NUM_AMBIG].attrs))) == NULL);

    varlist[NUM_AMBIG].attrs[FILL_VALUE].name = "FillValue";
    varlist[NUM_AMBIG].attrs[FILL_VALUE].size = 1;
    varlist[NUM_AMBIG].attrs[FILL_VALUE].type = varlist[NUM_AMBIG].type;
    varlist[NUM_AMBIG].attrs[FILL_VALUE].value.b = 0;
    varlist[NUM_AMBIG].attrs[VALID_MIN].name = "valid_min";
    varlist[NUM_AMBIG].attrs[VALID_MIN].size = 1;
    varlist[NUM_AMBIG].attrs[VALID_MIN].type = varlist[NUM_AMBIG].type;
    varlist[NUM_AMBIG].attrs[VALID_MIN].value.b = 1;
    varlist[NUM_AMBIG].attrs[VALID_MAX].name = "valid_max";
    varlist[NUM_AMBIG].attrs[VALID_MAX].size = 1;
    varlist[NUM_AMBIG].attrs[VALID_MAX].type = varlist[NUM_AMBIG].type;
    varlist[NUM_AMBIG].attrs[VALID_MAX].value.b = 0x04;
    varlist[NUM_AMBIG].attrs[LONG_NAME].name = "long_name";
    varlist[NUM_AMBIG].attrs[LONG_NAME].size = 1;
    varlist[NUM_AMBIG].attrs[LONG_NAME].type = NC_CHAR;
    varlist[NUM_AMBIG].attrs[LONG_NAME].value.str = "number of ambiguous wind directions found in point-wise wind retrieval prior to spatial filtering";
    varlist[NUM_AMBIG].attrs[num_standard_attrs].name = "units";
    varlist[NUM_AMBIG].attrs[num_standard_attrs].size = 1;
    varlist[NUM_AMBIG].attrs[num_standard_attrs].type = NC_CHAR;
    varlist[NUM_AMBIG].attrs[num_standard_attrs].value.str = "1";

    if (run_config.extended) { 
        varlist[SEL_OBJ].name  = "wind_obj";
        varlist[SEL_OBJ].type  = NC_FLOAT;
        varlist[SEL_OBJ].ndims = 2;
        varlist[SEL_OBJ].dims = dimensions; 
        varlist[SEL_OBJ].nattrs = num_standard_attrs + 2;
        ERR((varlist[SEL_OBJ].attrs = (typeof varlist[SEL_OBJ].attrs)
                    malloc(varlist[SEL_OBJ].nattrs * 
                        (sizeof *varlist[SEL_OBJ].attrs))) == NULL);
    
        varlist[SEL_OBJ].attrs[FILL_VALUE].name = "FillValue";
        varlist[SEL_OBJ].attrs[FILL_VALUE].size = 1;
        varlist[SEL_OBJ].attrs[FILL_VALUE].type = varlist[SEL_OBJ].type;
        varlist[SEL_OBJ].attrs[FILL_VALUE].value.f = FILL(0.0f, -9999.0);
        varlist[SEL_OBJ].attrs[VALID_MIN].name = "valid_min";
        varlist[SEL_OBJ].attrs[VALID_MIN].size = 1;
        varlist[SEL_OBJ].attrs[VALID_MIN].type = varlist[SEL_OBJ].type;
        varlist[SEL_OBJ].attrs[VALID_MIN].value.f = 0.0f;
        varlist[SEL_OBJ].attrs[VALID_MAX].name = "valid_max";
        varlist[SEL_OBJ].attrs[VALID_MAX].size = 1;
        varlist[SEL_OBJ].attrs[VALID_MAX].type = varlist[SEL_OBJ].type;
        varlist[SEL_OBJ].attrs[VALID_MAX].value.f = 360.0f;
        varlist[SEL_OBJ].attrs[LONG_NAME].name = "long_name";
        varlist[SEL_OBJ].attrs[LONG_NAME].size = 1;
        varlist[SEL_OBJ].attrs[LONG_NAME].type = NC_CHAR;
        varlist[SEL_OBJ].attrs[LONG_NAME].value.str = "selected wind objective function value";
        varlist[SEL_OBJ].attrs[num_standard_attrs].name = "units";
        varlist[SEL_OBJ].attrs[num_standard_attrs].size = 1;
        varlist[SEL_OBJ].attrs[num_standard_attrs].type = NC_CHAR;
        varlist[SEL_OBJ].attrs[num_standard_attrs].value.str = "1";
        varlist[SEL_OBJ].attrs[num_standard_attrs + 1].name = "scale_factor";
        varlist[SEL_OBJ].attrs[num_standard_attrs + 1].size = 1;
        varlist[SEL_OBJ].attrs[num_standard_attrs + 1].type = NC_FLOAT;
        varlist[SEL_OBJ].attrs[num_standard_attrs + 1].value.f = 1.0f;

        varlist[NUM_MEDFILT_AMBIG].name  = "num_ambiguities";
        varlist[NUM_MEDFILT_AMBIG].type  = NC_BYTE;
        varlist[NUM_MEDFILT_AMBIG].ndims = 2;
        varlist[NUM_MEDFILT_AMBIG].dims = dimensions; 
        varlist[NUM_MEDFILT_AMBIG].nattrs = num_standard_attrs + 1;
        ERR((varlist[NUM_MEDFILT_AMBIG].attrs = (typeof varlist[NUM_MEDFILT_AMBIG].attrs)
                    malloc(varlist[NUM_MEDFILT_AMBIG].nattrs * 
                        (sizeof *varlist[NUM_MEDFILT_AMBIG].attrs))) == NULL);
    
        varlist[NUM_MEDFILT_AMBIG].attrs[FILL_VALUE].name = "FillValue";
        varlist[NUM_MEDFILT_AMBIG].attrs[FILL_VALUE].size = 1;
        varlist[NUM_MEDFILT_AMBIG].attrs[FILL_VALUE].type = varlist[NUM_MEDFILT_AMBIG].type;
        varlist[NUM_MEDFILT_AMBIG].attrs[FILL_VALUE].value.b = 0;
        varlist[NUM_MEDFILT_AMBIG].attrs[VALID_MIN].name = "valid_min";
        varlist[NUM_MEDFILT_AMBIG].attrs[VALID_MIN].size = 1;
        varlist[NUM_MEDFILT_AMBIG].attrs[VALID_MIN].type = varlist[NUM_MEDFILT_AMBIG].type;
        varlist[NUM_MEDFILT_AMBIG].attrs[VALID_MIN].value.b = 1;
        varlist[NUM_MEDFILT_AMBIG].attrs[VALID_MAX].name = "valid_max";
        varlist[NUM_MEDFILT_AMBIG].attrs[VALID_MAX].size = 1;
        varlist[NUM_MEDFILT_AMBIG].attrs[VALID_MAX].type = varlist[NUM_MEDFILT_AMBIG].type;
        varlist[NUM_MEDFILT_AMBIG].attrs[VALID_MAX].value.b = 0x04;
        varlist[NUM_MEDFILT_AMBIG].attrs[LONG_NAME].name = "long_name";
        varlist[NUM_MEDFILT_AMBIG].attrs[LONG_NAME].size = 1;
        varlist[NUM_MEDFILT_AMBIG].attrs[LONG_NAME].type = NC_CHAR;
        varlist[NUM_MEDFILT_AMBIG].attrs[LONG_NAME].value.str = "number of ambiguities from wind retrieval after median filtering";
        varlist[NUM_MEDFILT_AMBIG].attrs[num_standard_attrs].name = "units";
        varlist[NUM_MEDFILT_AMBIG].attrs[num_standard_attrs].size = 1;
        varlist[NUM_MEDFILT_AMBIG].attrs[num_standard_attrs].type = NC_CHAR;
        varlist[NUM_MEDFILT_AMBIG].attrs[num_standard_attrs].value.str = "1";
 
        varlist[AMBIG_SPEED].name  = "ambiguity_speed";
        varlist[AMBIG_SPEED].type  = NC_FLOAT;
        varlist[AMBIG_SPEED].ndims = 3;
        varlist[AMBIG_SPEED].dims = dimensions; 
        varlist[AMBIG_SPEED].nattrs = num_standard_attrs + 2;
        ERR((varlist[AMBIG_SPEED].attrs = (typeof varlist[AMBIG_SPEED].attrs)
                    malloc(varlist[AMBIG_SPEED].nattrs * 
                        (sizeof *varlist[AMBIG_SPEED].attrs))) == NULL);
    
        varlist[AMBIG_SPEED].attrs[FILL_VALUE].name = "FillValue";
        varlist[AMBIG_SPEED].attrs[FILL_VALUE].size = 1;
        varlist[AMBIG_SPEED].attrs[FILL_VALUE].type = varlist[AMBIG_SPEED].type;
        varlist[AMBIG_SPEED].attrs[FILL_VALUE].value.f = FILL(0.0f, -9999.0);
        varlist[AMBIG_SPEED].attrs[VALID_MIN].name = "valid_min";
        varlist[AMBIG_SPEED].attrs[VALID_MIN].size = 1;
        varlist[AMBIG_SPEED].attrs[VALID_MIN].type = varlist[AMBIG_SPEED].type;
        varlist[AMBIG_SPEED].attrs[VALID_MIN].value.f = 0.0f;
        varlist[AMBIG_SPEED].attrs[VALID_MAX].name = "valid_max";
        varlist[AMBIG_SPEED].attrs[VALID_MAX].size = 1;
        varlist[AMBIG_SPEED].attrs[VALID_MAX].type = varlist[AMBIG_SPEED].type;
        varlist[AMBIG_SPEED].attrs[VALID_MAX].value.f = 100.0f;
        varlist[AMBIG_SPEED].attrs[LONG_NAME].name = "long_name";
        varlist[AMBIG_SPEED].attrs[LONG_NAME].size = 1;
        varlist[AMBIG_SPEED].attrs[LONG_NAME].type = NC_CHAR;
        varlist[AMBIG_SPEED].attrs[LONG_NAME].value.str = "wind speed ambiguity";
        varlist[AMBIG_SPEED].attrs[num_standard_attrs].name = "units";
        varlist[AMBIG_SPEED].attrs[num_standard_attrs].size = 1;
        varlist[AMBIG_SPEED].attrs[num_standard_attrs].type = NC_CHAR;
        varlist[AMBIG_SPEED].attrs[num_standard_attrs].value.str = "m s-1";
        varlist[AMBIG_SPEED].attrs[num_standard_attrs + 1].name = "scale_factor";
        varlist[AMBIG_SPEED].attrs[num_standard_attrs + 1].size = 1;
        varlist[AMBIG_SPEED].attrs[num_standard_attrs + 1].type = NC_FLOAT;
        varlist[AMBIG_SPEED].attrs[num_standard_attrs + 1].value.f = 1.0f;

        varlist[AMBIG_DIRECTION].name  = "ambiguity_to_direction";
        varlist[AMBIG_DIRECTION].type  = NC_FLOAT;
        varlist[AMBIG_DIRECTION].ndims = 3;
        varlist[AMBIG_DIRECTION].dims = dimensions; 
        varlist[AMBIG_DIRECTION].nattrs = num_standard_attrs + 2;
        ERR((varlist[AMBIG_DIRECTION].attrs = (typeof varlist[AMBIG_DIRECTION].attrs)
                    malloc(varlist[AMBIG_DIRECTION].nattrs * 
                        (sizeof *varlist[AMBIG_DIRECTION].attrs))) == NULL);
    
        varlist[AMBIG_DIRECTION].attrs[FILL_VALUE].name = "FillValue";
        varlist[AMBIG_DIRECTION].attrs[FILL_VALUE].size = 1;
        varlist[AMBIG_DIRECTION].attrs[FILL_VALUE].type = varlist[AMBIG_DIRECTION].type;
        varlist[AMBIG_DIRECTION].attrs[FILL_VALUE].value.f = FILL(0.0f, -9999.0);
        varlist[AMBIG_DIRECTION].attrs[VALID_MIN].name = "valid_min";
        varlist[AMBIG_DIRECTION].attrs[VALID_MIN].size = 1;
        varlist[AMBIG_DIRECTION].attrs[VALID_MIN].type = varlist[AMBIG_DIRECTION].type;
        varlist[AMBIG_DIRECTION].attrs[VALID_MIN].value.f = 0.0f;
        varlist[AMBIG_DIRECTION].attrs[VALID_MAX].name = "valid_max";
        varlist[AMBIG_DIRECTION].attrs[VALID_MAX].size = 1;
        varlist[AMBIG_DIRECTION].attrs[VALID_MAX].type = varlist[AMBIG_DIRECTION].type;
        varlist[AMBIG_DIRECTION].attrs[VALID_MAX].value.f = 360.0f;
        varlist[AMBIG_DIRECTION].attrs[LONG_NAME].name = "long_name";
        varlist[AMBIG_DIRECTION].attrs[LONG_NAME].size = 1;
        varlist[AMBIG_DIRECTION].attrs[LONG_NAME].type = NC_CHAR;
        varlist[AMBIG_DIRECTION].attrs[LONG_NAME].value.str = "wind direction ambiguity";
        varlist[AMBIG_DIRECTION].attrs[num_standard_attrs].name = "units";
        varlist[AMBIG_DIRECTION].attrs[num_standard_attrs].size = 1;
        varlist[AMBIG_DIRECTION].attrs[num_standard_attrs].type = NC_CHAR;
        varlist[AMBIG_DIRECTION].attrs[num_standard_attrs].value.str = "degrees";
        varlist[AMBIG_DIRECTION].attrs[num_standard_attrs + 1].name = "scale_factor";
        varlist[AMBIG_DIRECTION].attrs[num_standard_attrs + 1].size = 1;
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
        varlist[AMBIG_OBJ].attrs[FILL_VALUE].size = 1;
        varlist[AMBIG_OBJ].attrs[FILL_VALUE].type = varlist[AMBIG_OBJ].type;
        varlist[AMBIG_OBJ].attrs[FILL_VALUE].value.f = FILL(0.0f, -9999.0);
        varlist[AMBIG_OBJ].attrs[VALID_MIN].name = "valid_min";
        varlist[AMBIG_OBJ].attrs[VALID_MIN].size = 1;
        varlist[AMBIG_OBJ].attrs[VALID_MIN].type = varlist[AMBIG_OBJ].type;
        varlist[AMBIG_OBJ].attrs[VALID_MIN].value.f = 0.0f;
        varlist[AMBIG_OBJ].attrs[VALID_MAX].name = "valid_max";
        varlist[AMBIG_OBJ].attrs[VALID_MAX].size = 1;
        varlist[AMBIG_OBJ].attrs[VALID_MAX].type = varlist[AMBIG_OBJ].type;
        varlist[AMBIG_OBJ].attrs[VALID_MAX].value.f = 360.0f;
        varlist[AMBIG_OBJ].attrs[LONG_NAME].name = "long_name";
        varlist[AMBIG_OBJ].attrs[LONG_NAME].size = 1;
        varlist[AMBIG_OBJ].attrs[LONG_NAME].type = NC_CHAR;
        varlist[AMBIG_OBJ].attrs[LONG_NAME].value.str = "wind ambiguity objective function value";
        varlist[AMBIG_OBJ].attrs[num_standard_attrs].name = "units";
        varlist[AMBIG_OBJ].attrs[num_standard_attrs].size = 1;
        varlist[AMBIG_OBJ].attrs[num_standard_attrs].type = NC_CHAR;
        varlist[AMBIG_OBJ].attrs[num_standard_attrs].value.str = "1";
        varlist[AMBIG_OBJ].attrs[num_standard_attrs + 1].name = "scale_factor";
        varlist[AMBIG_OBJ].attrs[num_standard_attrs + 1].size = 1;
        varlist[AMBIG_OBJ].attrs[num_standard_attrs + 1].type = NC_FLOAT;
        varlist[AMBIG_OBJ].attrs[num_standard_attrs + 1].value.f = 1.0f;

        varlist[N_IN_FORE].name  = "number_in_fore";
        varlist[N_IN_FORE].type  = NC_BYTE;
        varlist[N_IN_FORE].ndims = 2;
        varlist[N_IN_FORE].dims = dimensions; 
        varlist[N_IN_FORE].nattrs = num_standard_attrs + 1;
        ERR((varlist[N_IN_FORE].attrs = (typeof varlist[N_IN_FORE].attrs)
                    malloc(varlist[N_IN_FORE].nattrs * 
                        (sizeof *varlist[N_IN_FORE].attrs))) == NULL);
    
        varlist[N_IN_FORE].attrs[FILL_VALUE].name = "FillValue";
        varlist[N_IN_FORE].attrs[FILL_VALUE].size = 1;
        varlist[N_IN_FORE].attrs[FILL_VALUE].type = varlist[N_IN_FORE].type;
        varlist[N_IN_FORE].attrs[FILL_VALUE].value.b = 0;
        varlist[N_IN_FORE].attrs[VALID_MIN].name = "valid_min";
        varlist[N_IN_FORE].attrs[VALID_MIN].size = 1;
        varlist[N_IN_FORE].attrs[VALID_MIN].type = varlist[N_IN_FORE].type;
        varlist[N_IN_FORE].attrs[VALID_MIN].value.b = 0;
        varlist[N_IN_FORE].attrs[VALID_MAX].name = "valid_max";
        varlist[N_IN_FORE].attrs[VALID_MAX].size = 1;
        varlist[N_IN_FORE].attrs[VALID_MAX].type = varlist[N_IN_FORE].type;
        varlist[N_IN_FORE].attrs[VALID_MAX].value.b = 0xFf;
        varlist[N_IN_FORE].attrs[LONG_NAME].name = "long_name";
        varlist[N_IN_FORE].attrs[LONG_NAME].size = 1;
        varlist[N_IN_FORE].attrs[LONG_NAME].type = NC_CHAR;
        varlist[N_IN_FORE].attrs[LONG_NAME].value.str = "number of inner forward looks in wind vector cell";
        varlist[N_IN_FORE].attrs[num_standard_attrs].name = "units";
        varlist[N_IN_FORE].attrs[num_standard_attrs].size = 1;
        varlist[N_IN_FORE].attrs[num_standard_attrs].type = NC_CHAR;
        varlist[N_IN_FORE].attrs[num_standard_attrs].value.str = "1";
            
        varlist[N_IN_AFT].name  = "number_in_aft";
        varlist[N_IN_AFT].type  = NC_BYTE;
        varlist[N_IN_AFT].ndims = 2;
        varlist[N_IN_AFT].dims = dimensions; 
        varlist[N_IN_AFT].nattrs = num_standard_attrs + 1;
        ERR((varlist[N_IN_AFT].attrs = (typeof varlist[N_IN_AFT].attrs)
                    malloc(varlist[N_IN_AFT].nattrs * 
                        (sizeof *varlist[N_IN_AFT].attrs))) == NULL);
    
        varlist[N_IN_AFT].attrs[FILL_VALUE].name = "FillValue";
        varlist[N_IN_AFT].attrs[FILL_VALUE].size = 1;
        varlist[N_IN_AFT].attrs[FILL_VALUE].type = varlist[N_IN_AFT].type;
        varlist[N_IN_AFT].attrs[FILL_VALUE].value.b = 0;
        varlist[N_IN_AFT].attrs[VALID_MIN].name = "valid_min";
        varlist[N_IN_AFT].attrs[VALID_MIN].size = 1;
        varlist[N_IN_AFT].attrs[VALID_MIN].type = varlist[N_IN_AFT].type;
        varlist[N_IN_AFT].attrs[VALID_MIN].value.b = 0;
        varlist[N_IN_AFT].attrs[VALID_MAX].name = "valid_max";
        varlist[N_IN_AFT].attrs[VALID_MAX].size = 1;
        varlist[N_IN_AFT].attrs[VALID_MAX].type = varlist[N_IN_AFT].type;
        varlist[N_IN_AFT].attrs[VALID_MAX].value.b = 0xFf;
        varlist[N_IN_AFT].attrs[LONG_NAME].name = "long_name";
        varlist[N_IN_AFT].attrs[LONG_NAME].size = 1;
        varlist[N_IN_AFT].attrs[LONG_NAME].type = NC_CHAR;
        varlist[N_IN_AFT].attrs[LONG_NAME].value.str = "number of inner aft looks in wind vector cell";
        varlist[N_IN_AFT].attrs[num_standard_attrs].name = "units";
        varlist[N_IN_AFT].attrs[num_standard_attrs].size = 1;
        varlist[N_IN_AFT].attrs[num_standard_attrs].type = NC_CHAR;
        varlist[N_IN_AFT].attrs[num_standard_attrs].value.str = "1";
               
        varlist[N_OUT_FORE].name  = "number_out_fore";
        varlist[N_OUT_FORE].type  = NC_BYTE;
        varlist[N_OUT_FORE].ndims = 2;
        varlist[N_OUT_FORE].dims = dimensions; 
        varlist[N_OUT_FORE].nattrs = num_standard_attrs + 1;
        ERR((varlist[N_OUT_FORE].attrs = (typeof varlist[N_OUT_FORE].attrs)
                    malloc(varlist[N_OUT_FORE].nattrs * 
                        (sizeof *varlist[N_OUT_FORE].attrs))) == NULL);
    
        varlist[N_OUT_FORE].attrs[FILL_VALUE].name = "FillValue";
        varlist[N_OUT_FORE].attrs[FILL_VALUE].size = 1;
        varlist[N_OUT_FORE].attrs[FILL_VALUE].type = varlist[N_OUT_FORE].type;
        varlist[N_OUT_FORE].attrs[FILL_VALUE].value.b = 0;
        varlist[N_OUT_FORE].attrs[VALID_MIN].name = "valid_min";
        varlist[N_OUT_FORE].attrs[VALID_MIN].size = 1;
        varlist[N_OUT_FORE].attrs[VALID_MIN].type = varlist[N_OUT_FORE].type;
        varlist[N_OUT_FORE].attrs[VALID_MIN].value.b = 0;
        varlist[N_OUT_FORE].attrs[VALID_MAX].name = "valid_max";
        varlist[N_OUT_FORE].attrs[VALID_MAX].size = 1;
        varlist[N_OUT_FORE].attrs[VALID_MAX].type = varlist[N_OUT_FORE].type;
        varlist[N_OUT_FORE].attrs[VALID_MAX].value.b = 0xFf;
        varlist[N_OUT_FORE].attrs[LONG_NAME].name = "long_name";
        varlist[N_OUT_FORE].attrs[LONG_NAME].size = 1;
        varlist[N_OUT_FORE].attrs[LONG_NAME].type = NC_CHAR;
        varlist[N_OUT_FORE].attrs[LONG_NAME].value.str = "number of outer forward looks in wind vector cell";
        varlist[N_OUT_FORE].attrs[num_standard_attrs].name = "units";
        varlist[N_OUT_FORE].attrs[num_standard_attrs].size = 1;
        varlist[N_OUT_FORE].attrs[num_standard_attrs].type = NC_CHAR;
        varlist[N_OUT_FORE].attrs[num_standard_attrs].value.str = "1";
               
        varlist[N_OUT_AFT].name  = "number_out_aft";
        varlist[N_OUT_AFT].type  = NC_BYTE;
        varlist[N_OUT_AFT].ndims = 2;
        varlist[N_OUT_AFT].dims = dimensions; 
        varlist[N_OUT_AFT].nattrs = num_standard_attrs + 1;
        ERR((varlist[N_OUT_AFT].attrs = (typeof varlist[N_OUT_AFT].attrs)
                    malloc(varlist[N_OUT_AFT].nattrs * 
                        (sizeof *varlist[N_OUT_AFT].attrs))) == NULL);
    
        varlist[N_OUT_AFT].attrs[FILL_VALUE].name = "FillValue";
        varlist[N_OUT_AFT].attrs[FILL_VALUE].size = 1;
        varlist[N_OUT_AFT].attrs[FILL_VALUE].type = varlist[N_OUT_AFT].type;
        varlist[N_OUT_AFT].attrs[FILL_VALUE].value.b = 0;
        varlist[N_OUT_AFT].attrs[VALID_MIN].name = "valid_min";
        varlist[N_OUT_AFT].attrs[VALID_MIN].size = 1;
        varlist[N_OUT_AFT].attrs[VALID_MIN].type = varlist[N_OUT_AFT].type;
        varlist[N_OUT_AFT].attrs[VALID_MIN].value.b = 0;
        varlist[N_OUT_AFT].attrs[VALID_MAX].name = "valid_max";
        varlist[N_OUT_AFT].attrs[VALID_MAX].size = 1;
        varlist[N_OUT_AFT].attrs[VALID_MAX].type = varlist[N_OUT_AFT].type;
        varlist[N_OUT_AFT].attrs[VALID_MAX].value.b = 0xFf;
        varlist[N_OUT_AFT].attrs[LONG_NAME].name = "long_name";
        varlist[N_OUT_AFT].attrs[LONG_NAME].size = 1;
        varlist[N_OUT_AFT].attrs[LONG_NAME].type = NC_CHAR;
        varlist[N_OUT_AFT].attrs[LONG_NAME].value.str = "number of outer aft looks in wind vector cell";
        varlist[N_OUT_AFT].attrs[num_standard_attrs].name = "units";
        varlist[N_OUT_AFT].attrs[num_standard_attrs].size = 1;
        varlist[N_OUT_AFT].attrs[num_standard_attrs].type = NC_CHAR;
        varlist[N_OUT_AFT].attrs[num_standard_attrs].value.str = "1";
    }

    for (int i = 0; i < (run_config.extended ? num_variables : 
                first_extended_variable); i++) {
        NCERR(nc_def_var(ncid, varlist[i].name, varlist[i].type,
                   varlist[i].ndims, varlist[i].dims, &varlist[i].id));

        for (int j = 0; j < varlist[i].nattrs; j++) {
            attribute *attr = &varlist[i].attrs[j];
            if (attr->name != NULL) {
                switch (attr->type) {
                    case NC_CHAR:
                        NCERR(nc_put_att_text(ncid, varlist[i].id, attr->name,
                                    strlen(attr->value.str), (attr->value.str)));
                        break;
                    case NC_SHORT:
                        if (attr->size == 1) {
                            NCERR(nc_put_att_short(ncid, varlist[i].id, attr->name, 
                                        attr->type, (size_t)(1), &(attr->value.s)));
                        } else {
                            NCERR(nc_put_att_short(ncid, varlist[i].id, attr->name, 
                                        attr->type, attr->size, attr->value.ps));
                        }
                        break;
                    case NC_INT:
                        if (attr->size == 1) {
                            NCERR(nc_put_att_int(ncid, varlist[i].id, attr->name, 
                                        attr->type, (size_t)(1), &(attr->value.i)));
                        } else {
                            NCERR(nc_put_att_int(ncid, varlist[i].id, attr->name, 
                                        attr->type, attr->size, attr->value.pi));
                        }
                        break;
                    case NC_FLOAT:
                        if (attr->size == 1) {
                            NCERR(nc_put_att_float(ncid, varlist[i].id, attr->name, 
                                        attr->type, (size_t)(1), &(attr->value.f)));
                        } else {
                            NCERR(nc_put_att_float(ncid, varlist[i].id, attr->name, 
                                        attr->type, attr->size, attr->value.pf));
                        }
                        break;
                    case NC_BYTE:
                        if (attr->size == 1) {
                            NCERR(nc_put_att_uchar(ncid, varlist[i].id, attr->name, 
                                        attr->type, (size_t)(1), &(attr->value.b)));
                        } else {
                            NCERR(nc_put_att_uchar(ncid, varlist[i].id, attr->name, 
                                        attr->type, attr->size, attr->value.pb));
                        }
                        break;
                    default:
                        fprintf(stderr, "Incorrect attribute type: %d\n", 
                                attr->type);
                        exit(EXIT_FAILURE);
                }
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
    NCERR(nc_put_att_int  (ncid, NC_GLOBAL, "version_id_major", 
                NC_INT,   (size_t)(1), &l2b.header.version_id_major));
    NCERR(nc_put_att_int  (ncid, NC_GLOBAL, "version_id_minor", 
                NC_INT,   (size_t)(1), &l2b.header.version_id_minor));
    NCERR(nc_put_att_text (ncid, NC_GLOBAL, "source_file",
                strlen(run_config.l1bhdf_file), run_config.l1bhdf_file));
    write_history(ncid, argc, argv);

    NCERR(nc_enddef(ncid));

    latlon_config orbit_config;
    NCERR(nc_get_att_float(ncid, NC_GLOBAL, "EquatorCrossingLongitude", &orbit_config.lambda_0));
    NCERR(nc_get_att_float(ncid, NC_GLOBAL, "orbit_inclination", &orbit_config.inclination));
    NCERR(nc_get_att_float(ncid, NC_GLOBAL, "rev_orbit_period", &orbit_config.rev_period));
    orbit_config.xt_steps = 2*l2b.header.zeroIndex;
    orbit_config.at_res = l2b.header.alongTrackResolution;
    orbit_config.xt_res = l2b.header.crossTrackResolution;

    /* Set up for grabbing timestamp data later */
    HDFERR(Vstart(l2bhdf_fid) == FAIL);

    HDFERR((time_vdata_ref = VSfind(l2bhdf_fid, time_vdata_name)) == 0);
    HDFERR((time_vdata_id  = VSattach(l2bhdf_fid, time_vdata_ref, "r")) == FAIL);
    HDFERR(VSsetfields(time_vdata_id, time_vdata_fname) == FAIL);
    
    HDFERR((time_vdata_fsize = VSsizeof(time_vdata_id, 
                    (char *)time_vdata_fname)) == FAIL);
    ERR((buffer = (char *)calloc(time_vdata_fsize + 1, 1)) == NULL);

    /* Set up for pulling land and ice flags from L2B HDF file */
    HDFERR((l2bhdf_sds_fid = SDstart(run_config.l2bhdf_file, DFACC_READ)) == FAIL);
    HDFERR((flags_sds_idx  = SDnametoindex(l2bhdf_sds_fid, flags_sds_name)) == FAIL);
    HDFERR((flags_sds_id   = SDselect(l2bhdf_sds_fid, flags_sds_idx)) == FAIL);
    ERR((hdf_flags = (unsigned short *)calloc(
            l2b.frame.swath.GetCrossTrackBins()*l2b.frame.swath.GetAlongTrackBins(), 
            sizeof *hdf_flags)) == NULL);

    time_vara_length[1] = strlen(time_format);

    int start[2] = {0, 0}; 
    int edges[2] = {0, 0};
    edges[0] = l2b.frame.swath.GetAlongTrackBins(); 
    edges[1] = l2b.frame.swath.GetCrossTrackBins();

    HDFERR(SDreaddata(flags_sds_id, start, NULL, edges, hdf_flags) == FAIL);

    /* Same sneakiness with idx as described above with dimensions */
    for (idx[0] = 0; (int)idx[0] < l2b.frame.swath.GetAlongTrackBins(); idx[0]++) {

        time_idx[0] = idx[0];

        HDFERR(VSread(time_vdata_id, (uint8 *)buffer, 1, FULL_INTERLACE) == FAIL);
        NCERR(nc_put_vara_text(ncid, varlist[TIME].id, time_idx, time_vara_length, buffer));

        for (idx[1] = 0; (int)idx[1] < l2b.frame.swath.GetCrossTrackBins(); idx[1]++) {

            wvc = l2b.frame.swath.swath[idx[1]][idx[0]];

            NCERR(nc_put_var1_float(ncid, varlist[WIND_DIVERGENCE].id, idx, 
                        &varlist[WIND_DIVERGENCE].attrs[FILL_VALUE].value.f));
            NCERR(nc_put_var1_float(ncid, varlist[WIND_CURL].id, idx, 
                        &varlist[WIND_CURL].attrs[FILL_VALUE].value.f));
            NCERR(nc_put_var1_float(ncid, varlist[WIND_STRESS].id, idx, 
                        &varlist[WIND_STRESS].attrs[FILL_VALUE].value.f));
            NCERR(nc_put_var1_float(ncid, varlist[STRESS_DIVERGENCE].id, idx, 
                        &varlist[STRESS_DIVERGENCE].attrs[FILL_VALUE].value.f));
            NCERR(nc_put_var1_float(ncid, varlist[STRESS_CURL].id, idx, 
                        &varlist[STRESS_CURL].attrs[FILL_VALUE].value.f));

            flags  = varlist[ FLAGS].attrs[FILL_VALUE].value.s;
            eflags = varlist[EFLAGS].attrs[FILL_VALUE].value.s;


            if (wvc != NULL && wvc->selected != NULL) {

                /* FLAGS */
                UNSET(flags, WIND_RETRIEVAL_MASK);
                
                UNSET_IF(flags, SIGMA0_MASK, IS_NOT_SET(wvc->qualFlag, L2B_QUAL_FLAG_ADQ_S0));
                UNSET_IF(flags, AZIMUTH_DIV_MASK, IS_NOT_SET(wvc->qualFlag, L2B_QUAL_FLAG_ADQ_AZI_DIV));
                UNSET_IF(flags, AVAILABLE_DATA_MASK, IS_NOT_SET(wvc->qualFlag, L2B_QUAL_FLAG_FOUR_FLAVOR));

                UNSET_IF(flags, COASTAL_MASK, IS_NOT_SET(wvc->landiceFlagBits, LAND_ICE_FLAG_COAST));
                UNSET_IF(flags, ICE_EDGE_MASK, IS_NOT_SET(wvc->landiceFlagBits, LAND_ICE_FLAG_ICE));

                UNSET_IF(flags, RAIN_IMPACT_UNUSABLE_MASK, IS_NOT_SET(wvc->rainFlagBits, RAIN_FLAG_UNUSABLE));
                UNSET_IF(flags, RAIN_IMPACT_MASK, IS_NOT_SET(wvc->rainFlagBits, RAIN_FLAG_RAIN));
                
                UNSET_IF(flags, HIGH_WIND_MASK, wvc->selected->spd <= 30);
                UNSET_IF(flags, LOW_WIND_MASK, wvc->selected->spd >= 3);

                /* EFLAGS */
                UNSET_IF(eflags, RAIN_CORR_NOT_APPL_MASK, IS_SET(wvc->qualFlag, L2B_QUAL_FLAG_RAIN_CORR_APPL));

                if (IS_NOT_SET(eflags, RAIN_CORR_NOT_APPL_MASK)) {
                    UNSET_IF(eflags, NEG_WIND_SPEED_MASK, wvc->selected->spd >= 0);
                } else {
                    UNSET(eflags, NEG_WIND_SPEED_MASK);
                }

                UNSET_IF(eflags, ALL_AMBIG_CONTRIB_MASK, IS_NOT_SET(wvc->qualFlag, L2B_QUAL_FLAG_ALL_AMBIG));
                UNSET_IF(eflags, RAIN_CORR_LARGE_MASK, fabs(wvc->speedBias) <= 1);

                lat = RAD_TO_DEG(wvc->lonLat.latitude);
                lon = RAD_TO_DEG(wvc->lonLat.longitude);

                NCERR(nc_put_var1_float(ncid, varlist[LATITUDE].id, idx, 
                            &lat));
                NCERR(nc_put_var1_float(ncid, varlist[LONGITUDE].id, idx, 
                            &lon));

                if ((wvc->rainCorrectedSpeed == -1) && (wvc->rainImpact == 0)) {
                    NCERR(nc_put_var1_float(ncid, varlist[RAIN_IMPACT].id, idx, 
                            &varlist[RAIN_IMPACT].attrs[FILL_VALUE].value.f));
                } else {
                    NCERR(nc_put_var1_float(ncid, varlist[RAIN_IMPACT].id, idx, 
                            &wvc->rainImpact));
                }

                if (IS_NOT_SET(eflags, RAIN_CORR_NOT_APPL_MASK)) {
                    /* Rain correction applied */
                    float uncorr_speed;

                    uncorr_speed = wvc->selected->spd + wvc->speedBias;

                    if ((uncorr_speed < 0) && (uncorr_speed > -1e-6)) {
                        uncorr_speed = 0.0f;
                    }

                    NCERR(nc_put_var1_float(ncid, 
                        varlist[WIND_SPEED_UNCORRECTED].id, idx, 
                        &uncorr_speed));
                } 

                if (wvc->selected->spd < 0.0f) {
                    wvc->selected->spd = 0.0f;
                } else if (wvc->selected->spd >= varlist[SEL_SPEED].attrs[VALID_MAX].value.f) {
                    wvc->selected->spd = varlist[SEL_SPEED].attrs[VALID_MAX].value.f;
                }

                NCERR(nc_put_var1_float(ncid, varlist[SEL_SPEED].id, idx, 
                        &wvc->selected->spd));

                if (IS_SET(eflags, RAIN_CORR_NOT_APPL_MASK)) {
                    NCERR(nc_put_var1_float(ncid, 
                        varlist[WIND_SPEED_UNCORRECTED].id, idx, 
                        &wvc->selected->spd));
                }

                conversion = 450.0f - (wvc->selected->dir)*180.0f/(float)(M_PI);
                conversion = conversion - 360.0f*((int)(conversion/360.0f));
                NCERR(nc_put_var1_float(ncid, varlist[SEL_DIRECTION].id, idx, 
                            &conversion));
                NCERR(nc_put_var1_short(ncid, varlist[FLAGS].id, idx, &flags));
                NCERR(nc_put_var1_short(ncid, varlist[EFLAGS].id, idx, &eflags));

                if (wvc->nudgeWV->spd <= varlist[NUDGE_SPEED].attrs[VALID_MAX].value.f) {
                    NCERR(nc_put_var1_float(ncid, varlist[NUDGE_SPEED].id, idx, 
                            &wvc->nudgeWV->spd));
                } else {
                    NCERR(nc_put_var1_float(ncid, varlist[NUDGE_SPEED].id, idx, 
                            &varlist[NUDGE_SPEED].attrs[VALID_MAX].value.f));
                }

                conversion = 450.0f - (wvc->nudgeWV->dir)*180.0f/(float)(M_PI);
                conversion = conversion - 360.0f*((int)(conversion/360.0f));
                NCERR(nc_put_var1_float(ncid, varlist[NUDGE_DIRECTION].id, idx, 
                            &conversion));

                NCERR(nc_put_var1(ncid, varlist[NUM_AMBIG].id, 
                        idx, &wvc->numAmbiguities));

                /* Extended variables */
                if (run_config.extended) {

                    NCERR(nc_put_var1_float(ncid, varlist[SEL_OBJ].id, idx, 
                                &wvc->selected->obj));
    
                    NCERR(nc_put_var1_uchar(ncid, varlist[N_IN_FORE].id, idx, 
                                &wvc->numInFore));
                    NCERR(nc_put_var1_uchar(ncid, varlist[N_IN_AFT].id, idx, 
                                &wvc->numInAft));
                    NCERR(nc_put_var1_uchar(ncid, varlist[N_OUT_FORE].id, idx, 
                                &wvc->numOutFore));
                    NCERR(nc_put_var1_uchar(ncid, varlist[N_OUT_AFT].id, idx, 
                                &wvc->numOutAft));
        
    
                    WindVectorPlus *wv = wvc->ambiguities.GetHead();
                    char num_ambiguities = wvc->ambiguities.NodeCount();
        
                    NCERR(nc_put_var1(ncid, varlist[NUM_MEDFILT_AMBIG].id, 
                            idx, &num_ambiguities));
                    for (idx[2] = 0; (int)idx[2] < num_ambiguities && wv != NULL; 
                            idx[2]++, wv = wvc->ambiguities.GetNext()) {
        
                        if (wv->spd <= varlist[AMBIG_SPEED].attrs[VALID_MAX].value.f) {
                            NCERR(nc_put_var1(ncid, varlist[AMBIG_SPEED].id, 
                                    idx, &wv->spd));
                        } else {
                            NCERR(nc_put_var1(ncid, varlist[AMBIG_SPEED].id, 
                                    idx, &varlist[AMBIG_SPEED].attrs[VALID_MAX].value.f));
                        }

                        conversion = 450.0f - (wv->dir)*180.0f/(float)(M_PI);
                        conversion = conversion - 360.0f*((int)(conversion/360.0f));
                        NCERR(nc_put_var1(ncid, varlist[AMBIG_DIRECTION].id, 
                                    idx, &conversion));
                        NCERR(nc_put_var1(ncid, varlist[AMBIG_OBJ].id, 
                                    idx, &wv->obj));

                    }
                    for ( ; (int)idx[2] < max_ambiguities; idx[2]++) {
                        NCERR(nc_put_var1_float(ncid, varlist[AMBIG_SPEED].id, idx,
                                    &varlist[AMBIG_SPEED].attrs[FILL_VALUE].value.f));
                        NCERR(nc_put_var1_float(ncid, varlist[AMBIG_DIRECTION].id, idx,
                                    &varlist[AMBIG_DIRECTION].attrs[FILL_VALUE].value.f));
                        NCERR(nc_put_var1_float(ncid, varlist[AMBIG_OBJ].id, idx,
                                    &varlist[AMBIG_OBJ].attrs[FILL_VALUE].value.f));
                    }
                }    
            } else {
            
                /* Assumes these flags are initially set... */
                UNSET_IF(flags, COASTAL_MASK,  IS_NOT_SET(hdf_flags[idx[0]*l2b.frame.swath.GetCrossTrackBins() + idx[1]], COASTAL_MASK));
                UNSET_IF(flags, ICE_EDGE_MASK, IS_NOT_SET(hdf_flags[idx[0]*l2b.frame.swath.GetCrossTrackBins() + idx[1]], ICE_EDGE_MASK));
                UNSET_IF(flags, RAIN_IMPACT_UNUSABLE_MASK, IS_NOT_SET(hdf_flags[idx[0]*l2b.frame.swath.GetCrossTrackBins() + idx[1]], RAIN_IMPACT_UNUSABLE_MASK));
                UNSET_IF(flags, RAIN_IMPACT_MASK, IS_NOT_SET(hdf_flags[idx[0]*l2b.frame.swath.GetCrossTrackBins() + idx[1]], RAIN_IMPACT_MASK));

                bin_to_latlon(idx[0], idx[1], &orbit_config, &lat, &lon);

                NCERR(nc_put_var1_float(ncid, varlist[LATITUDE].id, idx, &lat));
                NCERR(nc_put_var1_float(ncid, varlist[LONGITUDE].id, idx, &lon));

                NCERR(nc_put_var1_float(ncid, varlist[SEL_SPEED].id, idx, 
                            &varlist[SEL_SPEED].attrs[FILL_VALUE].value.f));
                NCERR(nc_put_var1_float(ncid, varlist[SEL_DIRECTION].id, idx, 
                            &varlist[SEL_DIRECTION].attrs[FILL_VALUE].value.f));
                NCERR(nc_put_var1_float(ncid, varlist[RAIN_IMPACT].id, idx, 
                            &varlist[RAIN_IMPACT].attrs[FILL_VALUE].value.f));
                NCERR(nc_put_var1_short(ncid, varlist[FLAGS].id, idx, &flags));
                NCERR(nc_put_var1_short(ncid, varlist[EFLAGS].id, idx, &eflags));

                NCERR(nc_put_var1_float(ncid, varlist[NUDGE_SPEED].id, idx, 
                            &varlist[NUDGE_SPEED].attrs[FILL_VALUE].value.f));
                NCERR(nc_put_var1_float(ncid, varlist[NUDGE_DIRECTION].id, idx, 
                            &varlist[NUDGE_DIRECTION].attrs[FILL_VALUE].value.f));

                NCERR(nc_put_var1_float(ncid, varlist[WIND_SPEED_UNCORRECTED].id, idx, 
                            &varlist[WIND_SPEED_UNCORRECTED].attrs[FILL_VALUE].value.f));

                NCERR(nc_put_var1(ncid, varlist[NUM_AMBIG].id, idx,
                            &varlist[NUM_AMBIG].attrs[FILL_VALUE].value.b));

                /* Extended variables */
                if (run_config.extended) {
                    NCERR(nc_put_var1_float(ncid, varlist[SEL_OBJ].id, idx, 
                                &varlist[SEL_OBJ].attrs[FILL_VALUE].value.f));
                    NCERR(nc_put_var1_uchar(ncid, varlist[N_IN_FORE].id, idx, 
                                &varlist[N_IN_FORE].attrs[FILL_VALUE].value.b));
                    NCERR(nc_put_var1_uchar(ncid, varlist[N_IN_AFT].id, idx, 
                                &varlist[N_IN_AFT].attrs[FILL_VALUE].value.b));
                    NCERR(nc_put_var1_uchar(ncid, varlist[N_OUT_FORE].id, idx, 
                                &varlist[N_OUT_FORE].attrs[FILL_VALUE].value.b));
                    NCERR(nc_put_var1_uchar(ncid, varlist[N_OUT_AFT].id, idx, 
                                &varlist[N_OUT_AFT].attrs[FILL_VALUE].value.b));
        
                    NCERR(nc_put_var1(ncid, varlist[NUM_MEDFILT_AMBIG].id, idx,
                                &varlist[NUM_MEDFILT_AMBIG].attrs[FILL_VALUE].value.b));

                    for (idx[2] = 0; (int)idx[2] < max_ambiguities; idx[2]++) {
        
                        NCERR(nc_put_var1_float(ncid, varlist[AMBIG_SPEED].id, idx,
                                    &varlist[AMBIG_SPEED].attrs[FILL_VALUE].value.f));
                        NCERR(nc_put_var1_float(ncid, varlist[AMBIG_DIRECTION].id, idx,
                                    &varlist[AMBIG_DIRECTION].attrs[FILL_VALUE].value.f));
                        NCERR(nc_put_var1_float(ncid, varlist[AMBIG_OBJ].id, idx,
                                    &varlist[AMBIG_OBJ].attrs[FILL_VALUE].value.f));
                    }
                }
            }
        }
    }

    free(buffer);

    HDFERR(SDend(l2bhdf_sds_fid) == FAIL);

    HDFERR(VSdetach(time_vdata_id) == FAIL);
    HDFERR(Vend(l2bhdf_fid) == FAIL);
    HDFERR(Hclose(l2bhdf_fid) == FAIL);

    NCERR(nc_close(ncid));

    //----------------------//
    // close files and exit //
    //----------------------//

    l2b.Close();
    return(0);
}


int parse_commandline(int argc, char **argv, l2b_to_netcdf_config *config) {

    const char* usage_array = "--l2b=<l2b file> --l2bhdf=<l2b hdf file> --l2bc=<l2bc file> --l1bhdf=<l1b hdf source file> [--extended]";
    int opt;

    /* Initialize configuration structure */
    config->command = no_path(argv[0]);
    config->l2b_file = NULL;
    config->l2bhdf_file = NULL;
    config->l2bc_file = NULL;
    config->l1bhdf_file = NULL;
    config->extended = 0;

    struct option longopts[] = 
    {
        { "l2b",      required_argument, NULL, 'i'},
        { "l2bhdf",   required_argument, NULL, 'h'},
        { "l2bc",     required_argument, NULL, 'o'},
        { "l1bhdf",   required_argument, NULL, 's'},
        { "extended", no_argument,       NULL, 'e'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "i:h:o:s:e", longopts, NULL)) != -1) {
        switch (opt) {
            case 'i': 
                config->l2b_file = optarg;
                break;
            case 'h': 
                config->l2bhdf_file = optarg;
                break;
            case 'o':
                config->l2bc_file = optarg;
                break;
            case 's':
                config->l1bhdf_file = optarg;
                break;
            case 'e':
                config->extended = 1;
                break;
        }

    }

    if (config->l2b_file == NULL || config->l2bc_file == NULL 
            || config->l1bhdf_file == NULL || config->l2bhdf_file == NULL) {

        fprintf(stderr, "%s: %s\n", config->command, usage_array);
        return -1;
    }

    return 0;
}

int copy_l2bhdf_attributes(int ncid, int hdfid) {

    unsigned int i;
    int vdata_ref, vdata_id;
    int attr_size;
    const char attr_fname[] = "VALUES";
    char *attr_value, *token;

    int ival;
    float fval;
    
    const char *hdf_attributes[] = {
        "producer_agency", "producer_institution",
        "PlatformType", "InstrumentShortName", "PlatformLongName",
        "PlatformShortName", "project_id",
        "QAPercentOutOfBoundsData", "QAPercentMissingData", "build_id",
        "ProductionDateTime", "sis_id", "OrbitParametersPointer",
        "HDF_version_id", "OperationMode", "StartOrbitNumber",
        "StopOrbitNumber", "EquatorCrossingLongitude", "EquatorCrossingTime",
        "EquatorCrossingDate", "rev_orbit_period", "orbit_inclination",
        "orbit_semi_major_axis", "orbit_eccentricity", "rev_number",
        "RangeBeginningDate", "RangeEndingDate", "RangeBeginningTime",
        "RangeEndingTime", "ephemeris_type", "sigma0_granularity",
        "median_filter_method", "sigma0_attenuation_method", "nudging_method",
        "ParameterName", "l2b_algorithm_descriptor", "InputPointer",
        "ancillary_data_descriptors", "QAGranulePointer", "GranulePointer"
    };

    const char *local_attributes[][2] = {
        {"LongName", "QuikSCAT Level 2B Ocean Wind and Stress Vectors in 12.5km Slice Composites"},
        {"ShortName", "QSCAT_LEVEL_2B_OWSV_COMP_12"},
        {"data_format_type", "NetCDF Classic"},
        {"Conventions", "CF 1.4"}
    };

    for (i = 0; i < (sizeof local_attributes)/(sizeof *local_attributes); i++) {
            NCERR(nc_put_att_text(ncid, NC_GLOBAL, 
                        local_attributes[i][0], 
                        strlen(local_attributes[i][1]),
                        local_attributes[i][1]));
    }

    HDFERR(Vstart(hdfid) == FAIL);

    for (i = 0; i < (sizeof hdf_attributes)/(sizeof *hdf_attributes); i++) {
        HDFERR((vdata_ref = VSfind(hdfid, hdf_attributes[i])) == 0);
        HDFERR((vdata_id  = VSattach(hdfid, vdata_ref, "r")) == FAIL);
        HDFERR((attr_size = VSsizeof(vdata_id, (char *)attr_fname)) == FAIL);

        ERR((attr_value = 
                    (typeof attr_value)calloc(attr_size + 1, 1)) == NULL);

        HDFERR(VSsetfields(vdata_id, attr_fname) == FAIL);
        HDFERR(VSread(vdata_id, (uint8 *)attr_value, 1, FULL_INTERLACE) == FAIL);

        ERR((token = strtok(attr_value, "\n")) == NULL);

        if (strcmp(token, "char") == 0) {
            ERR((token = strtok(NULL, "\n")) == NULL);
            ERR((token = strtok(NULL, "")) == NULL);

            // Wipe final newline
            if (token[strlen(token) - 1] == '\n') {
                token[strlen(token) - 1] = '\0';
            }

            NCERR(nc_put_att_text(ncid, NC_GLOBAL, hdf_attributes[i],
                    strlen(token), token));
        } else if (strcmp(token, "int") == 0) {
            ERR((token = strtok(NULL, "\n")) == NULL);
            ERR((token = strtok(NULL, "\n")) == NULL);

            ival = atoi(token);
            NCERR(nc_put_att_int(ncid, NC_GLOBAL, hdf_attributes[i],
                    NC_INT, (size_t)(1), &ival));
            
        } else if (strcmp(token, "float") == 0) {
            ERR((token = strtok(NULL, "\n")) == NULL);
            ERR((token = strtok(NULL, "\n")) == NULL);

            fval = strtof(token, NULL);
            NCERR(nc_put_att_float(ncid, NC_GLOBAL, hdf_attributes[i],
                    NC_FLOAT, (size_t)(1), &fval));
        } else {
            fprintf(stderr, "Unknown attribute type [%s] = %s\n", 
                    hdf_attributes[i], token);
            return -1;
        }

        free(attr_value);
        HDFERR(VSdetach(vdata_id) == FAIL);
    }

    HDFERR(Vend(hdfid) == FAIL);

    return 0;
}

static void bin_to_latlon(int at_ind, int ct_ind,
        const latlon_config *config, float *lat, float *lon) {

    /* Utilizes e2, r1_earth from Constants.h */
    const static double P1 = 60*1440.0f;

    const static double P2 = config->rev_period;
    const double inc = DEG_TO_RAD(config->inclination);
    const int    r_n_xt_bins = config->xt_steps;
    const double at_res = config->at_res;
    const double xt_res = config->xt_res;

    const double lambda_0 = DEG_TO_RAD(config->lambda_0);

    const double r_n_at_bins = 1624.0 * 25.0 / at_res;
    const double atrack_bin_const = two_pi/r_n_at_bins;
    const double xtrack_bin_const = xt_res/r1_earth;

    double lambda, lambda_t, lambda_pp;
    double phi, phi_pp;
    double Q, U, V, V1, V2;

    double sin_phi_pp, sin_lambda_pp;
    double sin_phi_pp2, sin_lambda_pp2;
    double sini, cosi;

    sini = sinf(inc);
    cosi = cosf(inc);

    lambda_pp = (at_ind + 0.5)*atrack_bin_const - pi_over_two;
    phi_pp = -(ct_ind - (r_n_xt_bins/2 - 0.5))*xtrack_bin_const;

    sin_phi_pp = sinf(phi_pp);
    sin_phi_pp2 = sin_phi_pp*sin_phi_pp;
    sin_lambda_pp = sinf(lambda_pp);
    sin_lambda_pp2 = sin_lambda_pp*sin_lambda_pp;
    
    Q = e2*sini*sini/(1 - e2);
    U = e2*cosi*cosi/(1 - e2);

    V1 = (1 - sin_phi_pp2/(1 - e2))*cosi*sin_lambda_pp;
    V2 = (sini*sin_phi_pp*sqrtf((1 + Q*sin_lambda_pp2)*(1 - 
                    sin_phi_pp2) - U*sin_phi_pp2));

    V = (V1 - V2)/(1 - sin_phi_pp2*(1 + U));

    lambda_t = atan2f(V, cosf(lambda_pp));
    lambda = lambda_t - (P2/P1)*lambda_pp + lambda_0;

    lambda += (lambda < 0)       ?  two_pi : 
              (lambda >= two_pi) ? -two_pi :
                                    0.0f;
    phi = atanf((tanf(lambda_pp)*cosf(lambda_t) - 
                cosf(inc)*sinf(lambda_t))/((1 - e2)*
                sinf(inc)));

    *lon = RAD_TO_DEG(lambda);
    *lat = RAD_TO_DEG(phi);
}

static int write_history(int ncid, int argc, const char * const * argv) {
    /* Build and insert a history attribute per the NUG:
     *
     * history
     *     A global attribute for an audit trail. This is a character array with
     *     a line for each invocation of a program that has modified the
     *     dataset. Well-behaved generic netCDF applications should append a
     *     line containing: date, time of day, user name, program name and
     *     command arguments.
     *
     */
    char *history;
    int i;
    int len = 0;
    time_t now;
    char *cnow;
    char *login;

    now = time(NULL);
    cnow = strdup(ctime(&now));
    login = strdup(getlogin());

    len += strlen(cnow) + 1;
    len += strlen(login) + 1;

    for (i = 0; i < argc; i++) {
        len += strlen(argv[i]) + 1;
    }

    history = (typeof history)calloc(len, sizeof(*history));

    strcat(history, cnow);
    history[strlen(history) - 1] = ' ';
    strcat(history, login);

    for (i = 0; i < argc; i++) {
        strcat(history, " ");
        strcat(history, argv[i]);
    }
    strcat(history, "\n");

    NCERR(nc_put_att_text (ncid, NC_GLOBAL, "history",
                strlen(history), history));

    free(history);
    free(login);
    free(cnow);

    return 0;
}
