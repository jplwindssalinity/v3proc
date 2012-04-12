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

#include "netcdf.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <mfhdf.h>
#include <vector>

#include "NetCDF.h"
#include "Misc.h"
#include "L2B.h"
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Tracking.h"
#include "Tracking.C"
#include "Constants.h"

using namespace std;

//--------//
// MACROS //
//--------//
#define STRINGIFY(x) #x
#define S(x) STRINGIFY(x)

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

#define BUILD_ID "v1_0_0:A"

// Used to create a structure to define NetCDF variables
#define DEF_VAR(var_, varid_, name_, type_, ndims_, dims_, nattrs_) {   \
    var_[varid_].name = name_;                                          \
    var_[varid_].type = type_;                                          \
    var_[varid_].ndims = ndims_;                                        \
    var_[varid_].dims = dims_;                                          \
    var_[varid_].nattrs = nattrs_;                                      \
    ERR((var_[varid_].attrs = (typeof var_[varid_].attrs)               \
                malloc(var_[varid_].nattrs *                            \
                    (sizeof *var_[varid_].attrs))) == NULL);            \
}
    
// Used to create an structure to define an attribute for a NetCDF variable
#define DEF_ATTR(var_, varid_, attid_, name_, size_,    \
        type_, off_, val_) {                            \
    var_[varid_].attrs[attid_].name = (name_);          \
    var_[varid_].attrs[attid_].size = (size_);          \
    var_[varid_].attrs[attid_].type = (type_);          \
    var_[varid_].attrs[attid_].value.off_ = (val_);     \
}

// If you want to kill a standard attribute, use this macro
#define SKIP_ATTR(var, varid, attr)   { \
    var[varid].attrs[attr].name = NULL; \
}

// Builds a variable and defines its standard attributes
#define DEF_VAR_STD_ATTRS(var, varid, name,         \
        type, ndims, dims, nattrs, off, fill, min,  \
        max, lname, std_name, units, scale)       { \
    DEF_VAR((var), (varid), (name), (type),         \
            (ndims), (dims), (nattrs));             \
    DEF_ATTR((var), (varid), FILL_VALUE,            \
            "_FillValue", 1, (type), off, (fill));  \
    DEF_ATTR((var), (varid), VALID_MIN,             \
            "valid_min", 1, (type), off, (min));    \
    DEF_ATTR((var), (varid), VALID_MAX,             \
            "valid_max", 1, (type), off, (max));    \
    DEF_ATTR((var), (varid), LONG_NAME,             \
            "long_name", 1, NC_CHAR, str, (lname)); \
    DEF_ATTR((var), (varid), STD_NAME,              \
            "standard_name", 1, NC_CHAR, str,       \
            (std_name));                            \
    DEF_ATTR((var), (varid), COORDINATES,           \
            "coordinates", 1, NC_CHAR, str,         \
            "lon lat");                             \
    DEF_ATTR((var), (varid), UNITS, "units", 1,     \
            NC_CHAR, str, units);                   \
    DEF_ATTR((var), (varid), SCALE, "scale_factor", \
            1, (type), off, (scale));               \
}

    
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

class AngleInterval;
template class List<AngleInterval>;
template class List<EarthPosition>;
template class List<WindVectorPlus>;
template class List<long>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;


typedef enum {
    TIME = 0,
    LATITUDE,
    LONGITUDE,
    SEL_SPEED,
    SEL_DIRECTION,
    RAIN_IMPACT,
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
    STD_NAME,
    COORDINATES,
    UNITS,
    SCALE,
    num_standard_attrs
} standard_attrs;

typedef struct {
    const char *command;
    const char *l2b_file;
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

int parse_commandline(int argc, char **argv, 
        l2b_to_netcdf_config *config);
int set_global_attributes(int ncid);

//--------------//
// MAIN PROGRAM //
//--------------//

int main(int argc, char **argv) {

    void *tmp;
    l2b_to_netcdf_config run_config;
    L2B l2b;

    /* File IDs */
    int ncid;

    /* Net CDF dimension information */
    int max_ambiguities;
    int cross_track_dim_id, along_track_dim_id, ambiguities_dim_id, time_strlen_dim_id;
    int dimensions[3];
    int time_dimensions[2];
    size_t time_vara_length[2] = {1, 0};

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
    NCERR(nc_create(run_config.l2bc_file, NC_WRITE, &ncid));

    ERR(set_global_attributes(ncid) != 0);

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
   
    DEF_VAR_STD_ATTRS(varlist, LATITUDE, "lat", NC_FLOAT, 2, dimensions,
            num_standard_attrs, f, 0.0f, -90.0f, 90.0f, "latitude",
            "latitude", "degrees_north", 1.0f);
    SKIP_ATTR(varlist, LATITUDE, FILL_VALUE);
    SKIP_ATTR(varlist, LATITUDE, COORDINATES);
    SKIP_ATTR(varlist, LATITUDE, SCALE);

    DEF_VAR_STD_ATTRS(varlist, LONGITUDE, "lon", NC_FLOAT, 2, dimensions,
            num_standard_attrs, f, 0.0f, 0.0f, 360.0f, "longitude",
            "longitude", "degrees_east", 1.0f);
    SKIP_ATTR(varlist, LONGITUDE, FILL_VALUE);
    SKIP_ATTR(varlist, LONGITUDE, COORDINATES);
    SKIP_ATTR(varlist, LONGITUDE, SCALE);

    DEF_VAR_STD_ATTRS(varlist, SEL_SPEED, "retrieved_wind_speed", NC_FLOAT, 2,
            dimensions, num_standard_attrs, f, -9999.0f, 0.0f, 100.0f,
            "equivalent neutral wind speed at 10 m", "wind_speed", "m s-1", 1.0f);

    DEF_VAR_STD_ATTRS(varlist, SEL_DIRECTION, "retrieved_wind_direction",
            NC_FLOAT, 2, dimensions, num_standard_attrs, f, -9999.0f, 0.0f,
            360.0f, "equivalent neutral wind direction at 10 m",
            "wind_to_direction", "degrees", 1.0f);
   
    DEF_VAR_STD_ATTRS(varlist, RAIN_IMPACT, "rain_impact", NC_FLOAT, 2,
            dimensions, num_standard_attrs, f, -9999.0f, 0.0f, 100.0f,
            "impact of rain upon wind vector retrieval", NULL, "1", 1.0f);
    SKIP_ATTR(varlist, RAIN_IMPACT, STD_NAME);

    DEF_VAR_STD_ATTRS(varlist, TIME, "time", NC_CHAR, 2, time_dimensions,
            num_standard_attrs, str, NULL, NULL, NULL, "date and time",
            NULL, time_format, NULL);
    SKIP_ATTR(varlist, TIME, FILL_VALUE);
    SKIP_ATTR(varlist, TIME, VALID_MIN);
    SKIP_ATTR(varlist, TIME, VALID_MAX);
    SKIP_ATTR(varlist, TIME, STD_NAME);
    SKIP_ATTR(varlist, TIME, COORDINATES);
    SKIP_ATTR(varlist, TIME, SCALE);

    ERR((tmp = (int16 *) malloc(NUM_FLAGS * sizeof(int16))) == NULL);
    {
        int16 init_list[NUM_FLAGS] = {SIGMA0_MASK, AZIMUTH_DIV_MASK, COASTAL_MASK, 
            ICE_EDGE_MASK, WIND_RETRIEVAL_MASK, HIGH_WIND_MASK, LOW_WIND_MASK, 
            RAIN_IMPACT_UNUSABLE_MASK, RAIN_IMPACT_MASK, AVAILABLE_DATA_MASK};

        ERR(memcpy(tmp, init_list, NUM_FLAGS* sizeof init_list[0]) == NULL);
    }
    DEF_VAR_STD_ATTRS(varlist, FLAGS, "flags", NC_SHORT, 2, dimensions,
            num_standard_attrs + 2, s, 0xFfff, 0, 32643,
            "wind vector cell quality flags", NULL, "bit", 1);
    SKIP_ATTR(varlist, FLAGS, STD_NAME);
    SKIP_ATTR(varlist, FLAGS, SCALE);
    DEF_ATTR(varlist, FLAGS, num_standard_attrs + 0, "flag_masks", NUM_FLAGS,
            varlist[FLAGS].type, ps, (int16 *)tmp);
    DEF_ATTR(varlist, FLAGS, num_standard_attrs + 1, "flag_meanings", NUM_FLAGS,
            NC_CHAR, str, "adequate_sigma0_flag "
        "adequate_azimuth_diversity_flag coastal_flag ice_edge_flag "
        "wind_retrieval_flag high_wind_speed_flag low_wind_speed_flag "
        "rain_impact_flag_usable rain_impact_flag available_data_flag"
    );

    ERR((tmp = (int16 *) malloc(NUM_EFLAGS * sizeof(int16))) == NULL);
    {
        int16 init_list[NUM_EFLAGS] = {RAIN_CORR_NOT_APPL_MASK, NEG_WIND_SPEED_MASK, 
            ALL_AMBIG_CONTRIB_MASK, RAIN_CORR_LARGE_MASK};

        ERR(memcpy(tmp, init_list, NUM_EFLAGS* sizeof init_list[0]) == NULL);
    }
    DEF_VAR_STD_ATTRS(varlist, EFLAGS, "eflags", NC_SHORT, 2, dimensions,
            num_standard_attrs + 2, s, 0xFfff, 0, 15,
            "extended wind vector cell quality flags", NULL, "bit", 1);
    SKIP_ATTR(varlist, EFLAGS, STD_NAME);
    SKIP_ATTR(varlist, EFLAGS, SCALE);
    DEF_ATTR(varlist, EFLAGS, num_standard_attrs + 0, "flag_masks", NUM_EFLAGS,
            varlist[EFLAGS].type, ps, (int16 *)tmp);
    DEF_ATTR(varlist, EFLAGS, num_standard_attrs + 1, "flag_meanings", NUM_EFLAGS,
            NC_CHAR, str, "rain_correction_not_applied_flag "
        "correction_produced_negative_spd_flag all_ambiguities_contribute_to_nudging_flag "
        "large_rain_correction_flag"
    );

    DEF_VAR_STD_ATTRS(varlist, NUDGE_SPEED, "nudge_wind_speed", NC_FLOAT, 2, dimensions,
            num_standard_attrs, f, -9999.0f, 0.0f, 100.0f, "model wind speed",
            "wind_speed", "m s-1", 1.0f);
             
    DEF_VAR_STD_ATTRS(varlist, NUDGE_DIRECTION, "nudge_wind_direction", NC_FLOAT, 2,
            dimensions, num_standard_attrs, f, -9999.0f, 0.0f, 360.0f,
            "model wind direction", "wind_to_direction", "degrees", 1.0f);

    DEF_VAR_STD_ATTRS(varlist, WIND_SPEED_UNCORRECTED, "wind_speed_uncorrected",
            NC_FLOAT, 2, dimensions, num_standard_attrs, f, -9999.0f, 0.0f,
            100.0f, "wind speed without rain correction", "wind_speed", "m s-1", 1.0f);

    DEF_VAR_STD_ATTRS(varlist, NUM_AMBIG, "num_ambiguities", NC_BYTE, 2, dimensions,
            num_standard_attrs, b, '\0', '\1', '\4', "number of ambiguous wind "
            "directions found in point-wise wind retrieval prior to spatial filtering",
            NULL, "1", 1);
    SKIP_ATTR(varlist, NUM_AMBIG, STD_NAME);
    SKIP_ATTR(varlist, NUM_AMBIG, SCALE);

    // Cross track wind speed bias
    // Atmospheric speed bias

#if 0
    if (run_config.extended) { 
        varlist[SEL_OBJ].name  = "wind_obj";
        varlist[SEL_OBJ].type  = NC_FLOAT;
        varlist[SEL_OBJ].ndims = 2;
        varlist[SEL_OBJ].dims = dimensions; 
        varlist[SEL_OBJ].nattrs = num_standard_attrs + 2;
        ERR((varlist[SEL_OBJ].attrs = (typeof varlist[SEL_OBJ].attrs)
                    malloc(varlist[SEL_OBJ].nattrs * 
                        (sizeof *varlist[SEL_OBJ].attrs))) == NULL);
    
        varlist[SEL_OBJ].attrs[FILL_VALUE].name = "_FillValue";
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
    
        varlist[NUM_MEDFILT_AMBIG].attrs[FILL_VALUE].name = "_FillValue";
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
    
        varlist[AMBIG_SPEED].attrs[FILL_VALUE].name = "_FillValue";
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
    
        varlist[AMBIG_DIRECTION].attrs[FILL_VALUE].name = "_FillValue";
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
    
        varlist[AMBIG_OBJ].attrs[FILL_VALUE].name = "_FillValue";
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
    
        varlist[N_IN_FORE].attrs[FILL_VALUE].name = "_FillValue";
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
    
        varlist[N_IN_AFT].attrs[FILL_VALUE].name = "_FillValue";
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
    
        varlist[N_OUT_FORE].attrs[FILL_VALUE].name = "_FillValue";
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
    
        varlist[N_OUT_AFT].attrs[FILL_VALUE].name = "_FillValue";
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
#endif

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

    NCERR(nc_enddef(ncid));

    latlon_config orbit_config;
    NCERR(nc_get_att_float(ncid, NC_GLOBAL, "EquatorCrossingLongitude", &orbit_config.lambda_0));
    NCERR(nc_get_att_float(ncid, NC_GLOBAL, "orbit_inclination", &orbit_config.inclination));
    NCERR(nc_get_att_float(ncid, NC_GLOBAL, "rev_orbit_period", &orbit_config.rev_period));
    orbit_config.xt_steps = 2*l2b.header.zeroIndex;
    orbit_config.at_res = l2b.header.alongTrackResolution;
    orbit_config.xt_res = l2b.header.crossTrackResolution;

    time_vara_length[1] = strlen(time_format);

    int edges[2] = {0, 0};
    edges[0] = l2b.frame.swath.GetAlongTrackBins(); 
    edges[1] = l2b.frame.swath.GetCrossTrackBins();

    /* Same sneakiness with idx as described above with dimensions */
    for (idx[0] = 0; (int)idx[0] < l2b.frame.swath.GetAlongTrackBins(); idx[0]++) {

        time_idx[0] = idx[0];

        for (idx[1] = 0; (int)idx[1] < l2b.frame.swath.GetCrossTrackBins(); idx[1]++) {

            wvc = l2b.frame.swath.swath[idx[1]][idx[0]];

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

    NCERR(nc_close(ncid));

    //----------------------//
    // close files and exit //
    //----------------------//

    l2b.Close();
    return(0);
}


int parse_commandline(int argc, char **argv, l2b_to_netcdf_config *config) {

    const char* usage_array = "--l2b=<l2b file> --l2bc=<l2bc file> --l1bhdf=<l1b hdf source file> [--extended]";
    int opt;

    /* Initialize configuration structure */
    config->command = no_path(argv[0]);
    config->l2b_file = NULL;
    config->l2bc_file = NULL;
    config->l1bhdf_file = NULL;
    config->extended = 0;

    struct option longopts[] = 
    {
        { "l2b",      required_argument, NULL, 'i'},
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

    // For now, no extended possible
    config->extended = 0;

    if (config->l2b_file == NULL || config->l2bc_file == NULL 
            || config->l1bhdf_file == NULL) {

        fprintf(stderr, "%s: %s\n", config->command, usage_array);
        return -1;
    }

    return 0;
}

#define WAVE "°º¤ø,¸¸,ø¤º°`°º¤ø,¸,ø¤°º¤ø,¸¸,ø¤º°`°º¤ø,¸"

int set_global_attributes(int ncid) {

    vector <class NC_Attribute *> local_attributes;


    local_attributes.push_back(new String_NC_Attribute("Conventions", "CF-1.5"));
    local_attributes.push_back(new String_NC_Attribute("title", WAVE));
    local_attributes.push_back(new String_NC_Attribute("institution", "JPL"));
    local_attributes.push_back(new String_NC_Attribute("source", "ISRO Oceansat-2 SCAT"));
    local_attributes.push_back(new String_NC_Attribute("history", WAVE));
    local_attributes.push_back(new String_NC_Attribute("references", WAVE));
    local_attributes.push_back(new String_NC_Attribute("comment", WAVE));

    local_attributes.push_back(new String_NC_Attribute("data_format_type", "NetCDF Classic"));
    local_attributes.push_back(new String_NC_Attribute("producer_agency", "NASA"));
    local_attributes.push_back(new String_NC_Attribute("build_id", BUILD_ID));
    local_attributes.push_back(new String_NC_Attribute("ProductionDateTime", WAVE));
    local_attributes.push_back(new String_NC_Attribute("StartOrbitNumber", WAVE));
    local_attributes.push_back(new String_NC_Attribute("StopOrbitNumber", WAVE));
    local_attributes.push_back(new String_NC_Attribute("rev_number", WAVE));
    local_attributes.push_back(new String_NC_Attribute("ancillary_data_descriptors", WAVE));
    local_attributes.push_back(new Float_NC_Attribute("EquatorCrossingLongitude", -9999.0f));
    local_attributes.push_back(new Float_NC_Attribute("orbit_inclination", -9999.0f));
    local_attributes.push_back(new Float_NC_Attribute("rev_orbit_period", -9999.0f));

    for (unsigned int i = 0; i < local_attributes.size(); i++) {
        NCERR(local_attributes[i]->Write(ncid, NC_GLOBAL));
    }

    return 0;
}

