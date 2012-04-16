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
#include <getopt.h>
#include <mfhdf.h>

#include "NetCDF_Attr.h"
#include "NetCDF_Var.h"
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


typedef struct {
    const char *command;
    const char *l2b_file;
    const char *l2bc_file;
    const char *l1bhdf_file;
    char extended;
} l2b_to_netcdf_config;

int parse_commandline(int argc, char **argv, 
        l2b_to_netcdf_config *config);
int set_global_attributes(int ncid);

static int16_t flag_masks[] = {SIGMA0_MASK, AZIMUTH_DIV_MASK, COASTAL_MASK, ICE_EDGE_MASK, WIND_RETRIEVAL_MASK, HIGH_WIND_MASK, LOW_WIND_MASK, RAIN_IMPACT_UNUSABLE_MASK, RAIN_IMPACT_MASK, AVAILABLE_DATA_MASK};
static int16_t eflag_masks[] = {RAIN_CORR_NOT_APPL_MASK, NEG_WIND_SPEED_MASK, ALL_AMBIG_CONTRIB_MASK, RAIN_CORR_LARGE_MASK};

//--------------//
// MAIN PROGRAM //
//--------------//

int main(int argc, char **argv) {

    l2b_to_netcdf_config run_config;
    L2B l2b;

    /* File IDs */
    int ncid;

    /* Net CDF dimension information */
    int max_ambiguities;
    int cross_track_dim_id, along_track_dim_id, ambiguities_dim_id, time_strlen_dim_id;
    size_t cross_track_dim_sz, along_track_dim_sz, time_strlen_dim_sz;
    int dimensions[3], dimensions_sz[3];
    int time_dimensions[2], time_dimensions_sz[2];
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
    along_track_dim_sz = (size_t)l2b.frame.swath.GetAlongTrackBins();
    cross_track_dim_sz = (size_t)l2b.frame.swath.GetCrossTrackBins();
    time_strlen_dim_sz = (size_t)strlen(time_format);

    NCERR(nc_def_dim(ncid, "along_track", along_track_dim_sz,
                &along_track_dim_id));
    NCERR(nc_def_dim(ncid, "cross_track", cross_track_dim_sz,
                &cross_track_dim_id));
    NCERR(nc_def_dim(ncid, "time_strlength", time_strlen_dim_sz,
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
    dimensions_sz[0] = along_track_dim_sz;
    dimensions_sz[1] = cross_track_dim_sz;
    dimensions_sz[2] = max_ambiguities;

    time_dimensions[0] = along_track_dim_id;
    time_dimensions[1] = time_strlen_dim_id;
    time_dimensions_sz[0] = along_track_dim_sz;
    time_dimensions_sz[1] = time_strlen_dim_sz;

    vector <NetCDF_Var_Base *> vars;

    // Define variables
    NetCDF_Var<float> *lat_var = new NetCDF_Var<float>("lat", ncid, 2, dimensions, dimensions_sz);
        float lat_min = -90.0f, lat_max = 90.0f;
        lat_var->AddAttribute(new NetCDF_Attr<float>("valid_min", lat_min));
        lat_var->AddAttribute(new NetCDF_Attr<float>("valid_max", lat_max));
        lat_var->AddAttribute(new NetCDF_Attr<char>("long_name", "latitude"));
        lat_var->AddAttribute(new NetCDF_Attr<char>("standard_name", "latitude"));
        lat_var->AddAttribute(new NetCDF_Attr<char>("units", "degrees_north"));
    NetCDF_Var<float> *lon_var = new NetCDF_Var<float>("lon", ncid, 2, dimensions, dimensions_sz);
        float lon_min = 0.0f, lon_max = 360.0f;
        lon_var->AddAttribute(new NetCDF_Attr<float>("valid_min", lon_min));
        lon_var->AddAttribute(new NetCDF_Attr<float>("valid_max", lon_max));
        lon_var->AddAttribute(new NetCDF_Attr<char>("long_name", "longitude"));
        lon_var->AddAttribute(new NetCDF_Attr<char>("standard_name", "longitude"));
        lon_var->AddAttribute(new NetCDF_Attr<char>("units", "degrees_east"));
    NetCDF_Var<float> *retrieved_speed_var = new NetCDF_Var<float>("retrieved_wind_speed", ncid, 2, dimensions, dimensions_sz);
        float retrieved_speed_fill = -9999.0f, retrieved_speed_min = 0.0f, retrieved_speed_max = 100.0f;
        retrieved_speed_var->AddAttribute(new NetCDF_Attr<float>("_FillValue", retrieved_speed_fill));
        retrieved_speed_var->AddAttribute(new NetCDF_Attr<float>("valid_min", retrieved_speed_min));
        retrieved_speed_var->AddAttribute(new NetCDF_Attr<float>("valid_max", retrieved_speed_max));
        retrieved_speed_var->AddAttribute(new NetCDF_Attr<char>("long_name", "equivalent neutral wind speed at 10 m"));
        retrieved_speed_var->AddAttribute(new NetCDF_Attr<char>("standard_name", "wind_speed"));
        retrieved_speed_var->AddAttribute(new NetCDF_Attr<char>("units", "m s-1"));
        retrieved_speed_var->AddAttribute(new NetCDF_Attr<float>("scale_factor", 1.0f));
        retrieved_speed_var->AddAttribute(new NetCDF_Attr<char>("coordinates", "lon lat"));
    NetCDF_Var<float> *retrieved_dir_var = new NetCDF_Var<float>("retrieved_wind_direction", ncid, 2, dimensions, dimensions_sz);
        float retrieved_dir_fill = -9999.0f, retrieved_dir_min = 0.0f, retrieved_dir_max = 100.0f;
        retrieved_dir_var->AddAttribute(new NetCDF_Attr<float>("_FillValue", retrieved_dir_fill));
        retrieved_dir_var->AddAttribute(new NetCDF_Attr<float>("valid_min", retrieved_dir_min));
        retrieved_dir_var->AddAttribute(new NetCDF_Attr<float>("valid_max", retrieved_dir_max));
        retrieved_dir_var->AddAttribute(new NetCDF_Attr<char>("long_name", "equivalent neutral wind direction at 10 m"));
        retrieved_dir_var->AddAttribute(new NetCDF_Attr<char>("standard_name", "wind_to_direction"));
        retrieved_dir_var->AddAttribute(new NetCDF_Attr<char>("units", "degrees"));
        retrieved_dir_var->AddAttribute(new NetCDF_Attr<float>("scale_factor", 1.0f));
        retrieved_dir_var->AddAttribute(new NetCDF_Attr<char>("coordinates", "lon lat"));
    NetCDF_Var<float> *rain_impact_var = new NetCDF_Var<float>("rain_impact", ncid, 2, dimensions, dimensions_sz);
        float rain_impact_fill = -9999.0f, rain_impact_min = 0.0f, rain_impact_max = 100.0f;
        rain_impact_var->AddAttribute(new NetCDF_Attr<float>("_FillValue", rain_impact_fill));
        rain_impact_var->AddAttribute(new NetCDF_Attr<float>("valid_min", rain_impact_min));
        rain_impact_var->AddAttribute(new NetCDF_Attr<float>("valid_max", rain_impact_max));
        rain_impact_var->AddAttribute(new NetCDF_Attr<char>("long_name", "impact of rain upon wind vector retrieval"));
        rain_impact_var->AddAttribute(new NetCDF_Attr<char>("units", "1"));
        rain_impact_var->AddAttribute(new NetCDF_Attr<float>("scale_factor", 1.0f));
        rain_impact_var->AddAttribute(new NetCDF_Attr<char>("coordinates", "lon lat"));
    NetCDF_Var<char> *time_var = new NetCDF_Var<char>("time", ncid, 2, time_dimensions, time_dimensions_sz);
        time_var->AddAttribute(new NetCDF_Attr<char>("long_name", "date and time"));
        time_var->AddAttribute(new NetCDF_Attr<char>("units", time_format));
    NetCDF_Var<short> *flags_var = new NetCDF_Var<short>("flags", ncid, 2, dimensions, dimensions_sz);
        short flags_fill = 0xFfff, flags_min = 0, flags_max = 32643;
        flags_var->AddAttribute(new NetCDF_Attr<short>("_FillValue", flags_fill));
        flags_var->AddAttribute(new NetCDF_Attr<short>("valid_min", flags_min));
        flags_var->AddAttribute(new NetCDF_Attr<short>("valid_max", flags_max));
        flags_var->AddAttribute(new NetCDF_Attr<char>("long_name", "wind vector cell quality flags"));
        flags_var->AddAttribute(new NetCDF_Attr<short>("flag_masks", flag_masks, sizeof(flag_masks)/sizeof(flag_masks[0])));
        flags_var->AddAttribute(new NetCDF_Attr<char>("flag_meanings", "adequate_sigma0_flag adequate_azimuth_diversity_flag coastal_flag ice_edge_flag wind_retrieval_flag high_wind_speed_flag low_wind_speed_flag rain_impact_flag_usable rain_impact_flag available_data_flag"));
        flags_var->AddAttribute(new NetCDF_Attr<char>("units", "bit"));
        flags_var->AddAttribute(new NetCDF_Attr<char>("coordinates", "lon lat"));
    NetCDF_Var<short> *eflags_var = new NetCDF_Var<short>("eflags", ncid, 2, dimensions, dimensions_sz);
        short eflags_fill = 0xFfff, eflags_min = 0, eflags_max = 15;
        eflags_var->AddAttribute(new NetCDF_Attr<short>("_FillValue", eflags_fill));
        eflags_var->AddAttribute(new NetCDF_Attr<short>("valid_min", eflags_min));
        eflags_var->AddAttribute(new NetCDF_Attr<short>("valid_max", eflags_max));
        eflags_var->AddAttribute(new NetCDF_Attr<char>("long_name", "extended wind vector cell quality flags"));
        eflags_var->AddAttribute(new NetCDF_Attr<short>("eflag_masks", eflag_masks, sizeof(eflag_masks)/sizeof(eflag_masks[0])));
        eflags_var->AddAttribute(new NetCDF_Attr<char>("flag_meanings", "rain_correction_not_applied_flag correction_produced_negative_spd_flag all_ambiguities_contribute_to_nudging_flag large_rain_correction_flag"));
        eflags_var->AddAttribute(new NetCDF_Attr<char>("units", "bit"));
        eflags_var->AddAttribute(new NetCDF_Attr<char>("coordinates", "lon lat"));
    NetCDF_Var<float> *nudge_speed_var = new NetCDF_Var<float>("nudge_wind_speed", ncid, 2, dimensions, dimensions_sz);
        float nudge_speed_fill = -9999.0f, nudge_speed_min = 0.0f, nudge_speed_max = 100.0f;
        nudge_speed_var->AddAttribute(new NetCDF_Attr<float>("_FillValue", nudge_speed_fill));
        nudge_speed_var->AddAttribute(new NetCDF_Attr<float>("valid_min", nudge_speed_min));
        nudge_speed_var->AddAttribute(new NetCDF_Attr<float>("valid_max", nudge_speed_max));
        nudge_speed_var->AddAttribute(new NetCDF_Attr<char>("long_name", "model wind speed"));
        nudge_speed_var->AddAttribute(new NetCDF_Attr<char>("standard_name", "wind_speed"));
        nudge_speed_var->AddAttribute(new NetCDF_Attr<char>("units", "m s-1"));
        nudge_speed_var->AddAttribute(new NetCDF_Attr<float>("scale_factor", 1.0f));
        nudge_speed_var->AddAttribute(new NetCDF_Attr<char>("coordinates", "lon lat"));
    NetCDF_Var<float> *nudge_dir_var = new NetCDF_Var<float>("nudge_wind_direction", ncid, 2, dimensions, dimensions_sz);
        float nudge_dir_fill = -9999.0f, nudge_dir_min = 0.0f, nudge_dir_max = 360.0f;
        nudge_dir_var->AddAttribute(new NetCDF_Attr<float>("_FillValue", nudge_dir_fill));
        nudge_dir_var->AddAttribute(new NetCDF_Attr<float>("valid_min", nudge_dir_min));
        nudge_dir_var->AddAttribute(new NetCDF_Attr<float>("valid_max", nudge_dir_max));
        nudge_dir_var->AddAttribute(new NetCDF_Attr<char>("long_name", "model wind direction"));
        nudge_dir_var->AddAttribute(new NetCDF_Attr<char>("standard_name", "wind_to_direction"));
        nudge_dir_var->AddAttribute(new NetCDF_Attr<char>("units", "m s-1"));
        nudge_dir_var->AddAttribute(new NetCDF_Attr<float>("scale_factor", 1.0f));
        nudge_dir_var->AddAttribute(new NetCDF_Attr<char>("coordinates", "lon lat"));
    NetCDF_Var<float> *wind_speed_uncorr_var = new NetCDF_Var<float>("wind_speed_uncorrected", ncid, 2, dimensions, dimensions_sz);
        float wind_speed_uncorr_fill = -9999.0f, wind_speed_uncorr_min = 0.0f, wind_speed_uncorr_max = 100.f;
        wind_speed_uncorr_var->AddAttribute(new NetCDF_Attr<float>("_FillValue", wind_speed_uncorr_fill));
        wind_speed_uncorr_var->AddAttribute(new NetCDF_Attr<float>("valid_min", wind_speed_uncorr_min));
        wind_speed_uncorr_var->AddAttribute(new NetCDF_Attr<float>("valid_max", wind_speed_uncorr_max));
        wind_speed_uncorr_var->AddAttribute(new NetCDF_Attr<char>("long_name", "wind speed without rain correction"));
        wind_speed_uncorr_var->AddAttribute(new NetCDF_Attr<char>("standard_name", "wind_speed"));
        wind_speed_uncorr_var->AddAttribute(new NetCDF_Attr<char>("units", "m s-1"));
        wind_speed_uncorr_var->AddAttribute(new NetCDF_Attr<float>("scale_factor", 1.0f));
        wind_speed_uncorr_var->AddAttribute(new NetCDF_Attr<char>("coordinates", "lon lat"));
    NetCDF_Var<unsigned char> *num_ambig_var = new NetCDF_Var<unsigned char>("num_ambiguities", ncid, 2, dimensions, dimensions_sz);
        unsigned char num_ambig_fill = 0, num_ambig_min = 1, num_ambig_max = 4;
        num_ambig_var->AddAttribute(new NetCDF_Attr<unsigned char>("_FillValue", num_ambig_fill));
        num_ambig_var->AddAttribute(new NetCDF_Attr<unsigned char>("valid_min", num_ambig_min));
        num_ambig_var->AddAttribute(new NetCDF_Attr<unsigned char>("valid_max", num_ambig_max));
        num_ambig_var->AddAttribute(new NetCDF_Attr<char>("long_name", "number of ambiguous wind " "directions found in point-wise wind retrieval prior to spatial filtering"));
        num_ambig_var->AddAttribute(new NetCDF_Attr<char>("units", "1"));
        num_ambig_var->AddAttribute(new NetCDF_Attr<char>("coordinates", "lon lat"));


    // Cross track wind speed bias
    // Atmospheric speed bias

    vars.push_back(time_var);
    vars.push_back(lat_var);
    vars.push_back(lon_var);
    vars.push_back(retrieved_speed_var);
    vars.push_back(retrieved_dir_var);
    vars.push_back(rain_impact_var);
    vars.push_back(flags_var);
    vars.push_back(eflags_var);
    vars.push_back(nudge_speed_var);
    vars.push_back(nudge_dir_var);
    vars.push_back(wind_speed_uncorr_var);
    vars.push_back(num_ambig_var);

    if (run_config.extended) { 
        // TODO: capture the new values, fill, max, min,  and iterate deletes @ exit
        vars.push_back(new NetCDF_Var<float>("wind_obj", ncid, 2, dimensions, dimensions_sz));
            vars.back()->AddAttribute(new NetCDF_Attr<float>("_FillValue", -9999.0f));
            vars.back()->AddAttribute(new NetCDF_Attr<float>("valid_min", 0.0f));
            vars.back()->AddAttribute(new NetCDF_Attr<float>("valid_max", 360.0f));
            vars.back()->AddAttribute(new NetCDF_Attr<char>("long_name", "selected wind objective function value"));
            vars.back()->AddAttribute(new NetCDF_Attr<char>("units", "1"));
            vars.back()->AddAttribute(new NetCDF_Attr<float>("scale_factor", 1.0f));
            vars.back()->AddAttribute(new NetCDF_Attr<char>("coordinates", "lon lat"));
        vars.push_back(new NetCDF_Var<unsigned char>("num_medfilt_ambiguities", ncid, 2, dimensions, dimensions_sz));
            vars.back()->AddAttribute(new NetCDF_Attr<unsigned char>("_FillValue", uint8_t(0xFf)));
            vars.back()->AddAttribute(new NetCDF_Attr<unsigned char>("valid_min",  uint8_t(0)));
            vars.back()->AddAttribute(new NetCDF_Attr<unsigned char>("valid_max",  uint8_t(4)));
            vars.back()->AddAttribute(new NetCDF_Attr<char>("long_name", "number of ambiguities from wind retrieval after median filtering"));
            vars.back()->AddAttribute(new NetCDF_Attr<char>("units", "1"));
            vars.back()->AddAttribute(new NetCDF_Attr<char>("coordinates", "lon lat"));
        vars.push_back(new NetCDF_Var<float>("ambiguity_speed", ncid, 3, dimensions, dimensions_sz));
            vars.back()->AddAttribute(new NetCDF_Attr<float>("_FillValue", -9999.0f));
            vars.back()->AddAttribute(new NetCDF_Attr<float>("valid_min", 0.0f));
            vars.back()->AddAttribute(new NetCDF_Attr<float>("valid_max", 100.0f));
            vars.back()->AddAttribute(new NetCDF_Attr<char>("long_name", "wind speed ambiguity"));
            vars.back()->AddAttribute(new NetCDF_Attr<char>("units", "m s-1"));
            vars.back()->AddAttribute(new NetCDF_Attr<float>("scale_factor", 1.0f));
            vars.back()->AddAttribute(new NetCDF_Attr<char>("coordinates", "lon lat"));
        vars.push_back(new NetCDF_Var<float>("ambiguity_to_direction", ncid, 3, dimensions, dimensions_sz));
            vars.back()->AddAttribute(new NetCDF_Attr<float>("_FillValue", -9999.0f));
            vars.back()->AddAttribute(new NetCDF_Attr<float>("valid_min", 0.0f));
            vars.back()->AddAttribute(new NetCDF_Attr<float>("valid_max", 360.0f));
            vars.back()->AddAttribute(new NetCDF_Attr<char>("long_name", "wind direction ambiguity"));
            vars.back()->AddAttribute(new NetCDF_Attr<char>("units", "degrees"));
            vars.back()->AddAttribute(new NetCDF_Attr<float>("scale_factor", 1.0f));
            vars.back()->AddAttribute(new NetCDF_Attr<char>("coordinates", "lon lat"));
        vars.push_back(new NetCDF_Var<float>("ambiguity_obj", ncid, 3, dimensions, dimensions_sz));
            vars.back()->AddAttribute(new NetCDF_Attr<float>("_FillValue", -9999.0f));
            vars.back()->AddAttribute(new NetCDF_Attr<float>("valid_min", 0.0f));
            vars.back()->AddAttribute(new NetCDF_Attr<float>("valid_max", 360.0f));
            vars.back()->AddAttribute(new NetCDF_Attr<char>("long_name", "wind ambiguity objective function value"));
            vars.back()->AddAttribute(new NetCDF_Attr<char>("units", "1"));
            vars.back()->AddAttribute(new NetCDF_Attr<float>("scale_factor", 1.0f));
            vars.back()->AddAttribute(new NetCDF_Attr<char>("coordinates", "lon lat"));
        vars.push_back(new NetCDF_Var<unsigned char>("number_in_fore", ncid, 3, dimensions, dimensions_sz));
            vars.back()->AddAttribute(new NetCDF_Attr<unsigned char>("_FillValue", uint8_t(0xFf)));
            vars.back()->AddAttribute(new NetCDF_Attr<unsigned char>("valid_min",  uint8_t(0)));
            vars.back()->AddAttribute(new NetCDF_Attr<unsigned char>("valid_max",  uint8_t(0xFe)));
            vars.back()->AddAttribute(new NetCDF_Attr<char>("long_name", "number of inner forward looks in wind vector cell"));
            vars.back()->AddAttribute(new NetCDF_Attr<char>("units", "1"));
            vars.back()->AddAttribute(new NetCDF_Attr<char>("coordinates", "lon lat"));
        vars.push_back(new NetCDF_Var<unsigned char>("number_in_aft", ncid, 3, dimensions, dimensions_sz));
            vars.back()->AddAttribute(new NetCDF_Attr<unsigned char>("_FillValue", uint8_t(0xFf)));
            vars.back()->AddAttribute(new NetCDF_Attr<unsigned char>("valid_min",  uint8_t(0)));
            vars.back()->AddAttribute(new NetCDF_Attr<unsigned char>("valid_max",  uint8_t(0xFe)));
            vars.back()->AddAttribute(new NetCDF_Attr<char>("long_name", "number of inner aft looks in wind vector cell"));
            vars.back()->AddAttribute(new NetCDF_Attr<char>("units", "1"));
            vars.back()->AddAttribute(new NetCDF_Attr<char>("coordinates", "lon lat"));
        vars.push_back(new NetCDF_Var<unsigned char>("number_out_fore", ncid, 3, dimensions, dimensions_sz));
            vars.back()->AddAttribute(new NetCDF_Attr<unsigned char>("_FillValue", uint8_t(0xFf)));
            vars.back()->AddAttribute(new NetCDF_Attr<unsigned char>("valid_min",  uint8_t(0)));
            vars.back()->AddAttribute(new NetCDF_Attr<unsigned char>("valid_max",  uint8_t(0xFe)));
            vars.back()->AddAttribute(new NetCDF_Attr<char>("long_name", "number of outer forward looks in wind vector cell"));
            vars.back()->AddAttribute(new NetCDF_Attr<char>("units", "1"));
            vars.back()->AddAttribute(new NetCDF_Attr<char>("coordinates", "lon lat"));
        vars.push_back(new NetCDF_Var<unsigned char>("number_out_aft", ncid, 3, dimensions, dimensions_sz));
            vars.back()->AddAttribute(new NetCDF_Attr<unsigned char>("_FillValue", uint8_t(0xFf)));
            vars.back()->AddAttribute(new NetCDF_Attr<unsigned char>("valid_min",  uint8_t(0)));
            vars.back()->AddAttribute(new NetCDF_Attr<unsigned char>("valid_max",  uint8_t(0xFe)));
            vars.back()->AddAttribute(new NetCDF_Attr<char>("long_name", "number of outer aft looks in wind vector cell"));
            vars.back()->AddAttribute(new NetCDF_Attr<char>("units", "1"));
            vars.back()->AddAttribute(new NetCDF_Attr<char>("coordinates", "lon lat"));
    }

    // Push definitions into NetCDF file
    for(vector <NetCDF_Var_Base *>::iterator it = vars.begin(); it < vars.end(); it++) {
        NCERR((*it)->Define());
        NCERR((*it)->WriteAttributes());
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
    edges[0] = along_track_dim_sz;
    edges[1] = cross_track_dim_sz;

    /* Same sneakiness with idx as described above with dimensions */
    for (idx[0] = 0; idx[0] < along_track_dim_sz; idx[0]++) {

        time_idx[0] = idx[0];

        for (idx[1] = 0; idx[1] < cross_track_dim_sz; idx[1]++) {

            wvc = l2b.frame.swath.swath[idx[1]][idx[0]];

            flags  = flags_fill;
            eflags = eflags_fill;

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

                lat_var->SetData(idx, lat);
                lon_var->SetData(idx, lon);

                if ((wvc->rainCorrectedSpeed == -1) && (wvc->rainImpact == 0)) {
                    rain_impact_var->SetData(idx, rain_impact_fill);
                } else {
                    rain_impact_var->SetData(idx, wvc->rainImpact);
                }

                if (IS_NOT_SET(eflags, RAIN_CORR_NOT_APPL_MASK)) {
                    /* Rain correction applied */
                    float uncorr_speed;

                    uncorr_speed = wvc->selected->spd + wvc->speedBias;

                    if ((uncorr_speed < 0) && (uncorr_speed > -1e-6)) {
                        uncorr_speed = 0.0f;
                    }

                    wind_speed_uncorr_var->SetData(idx, uncorr_speed);
                } 

                if (wvc->selected->spd < retrieved_speed_min) {
                    wvc->selected->spd = retrieved_speed_min;
                } else if (wvc->selected->spd >= retrieved_speed_max) {
                    wvc->selected->spd = retrieved_speed_max;
                }

                retrieved_speed_var->SetData(idx, wvc->selected->spd);

                if (IS_SET(eflags, RAIN_CORR_NOT_APPL_MASK)) {
                    wind_speed_uncorr_var->SetData(idx, wvc->selected->spd);
                }

                conversion = 450.0f - (wvc->selected->dir)*180.0f/(float)(M_PI);
                conversion = conversion - 360.0f*((int)(conversion/360.0f));
                retrieved_dir_var->SetData(idx, conversion);

                flags_var->SetData(idx, flags);
                eflags_var->SetData(idx, eflags);

                if (wvc->nudgeWV->spd <= nudge_speed_max) {
                    nudge_speed_var->SetData(idx, wvc->nudgeWV->spd);
                } else {
                    nudge_speed_var->SetData(idx, nudge_speed_max);
                }

                conversion = 450.0f - (wvc->nudgeWV->dir)*180.0f/(float)(M_PI);
                conversion = conversion - 360.0f*((int)(conversion/360.0f));
                nudge_dir_var->SetData(idx, conversion);

                num_ambig_var->SetData(idx, wvc->numAmbiguities);

#if 0
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
#endif
            } else {
            
                /* Assumes these flags are initially set... */
                bin_to_latlon(idx[0], idx[1], &orbit_config, &lat, &lon);

                lat_var->SetData(idx, lat);
                lon_var->SetData(idx, lon);

                retrieved_speed_var->SetData(idx, retrieved_speed_fill);
                retrieved_dir_var->SetData(idx, retrieved_dir_fill);
                rain_impact_var->SetData(idx, rain_impact_fill);
                flags_var->SetData(idx, flags);
                eflags_var->SetData(idx, flags);

                nudge_speed_var->SetData(idx, nudge_speed_fill);
                nudge_dir_var->SetData(idx, nudge_dir_fill);

                wind_speed_uncorr_var->SetData(idx, wind_speed_uncorr_fill);
                num_ambig_var->SetData(idx, num_ambig_fill);


#if 0
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
#endif
            }
        }
    }

    for(vector <NetCDF_Var_Base *>::iterator it = vars.begin(); it < vars.end(); it++) {
        NCERR((*it)->Write());
    }

    NCERR(nc_close(ncid));

    for (int i = vars.size() - 1; i >= 0; i--) {
        delete vars[i];
    }

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

    vector <class NetCDF_Attr_Base *> local_attributes;

    local_attributes.push_back(new NetCDF_Attr<char>("title", WAVE));
    local_attributes.push_back(new NetCDF_Attr<char>("institution", "JPL"));
    local_attributes.push_back(new NetCDF_Attr<char>("source", "ISRO Oceansat-2 SCAT"));
    local_attributes.push_back(new NetCDF_Attr<char>("comment", WAVE));
    local_attributes.push_back(new NetCDF_Attr<char>("history", WAVE));

    local_attributes.push_back(new NetCDF_Attr<char>("Conventions", "CF-1.5"));
    local_attributes.push_back(new NetCDF_Attr<char>("data_format_type", "NetCDF Classic"));
    local_attributes.push_back(new NetCDF_Attr<char>("producer_agency", "NASA"));
    local_attributes.push_back(new NetCDF_Attr<char>("build_id", BUILD_ID));
    local_attributes.push_back(new NetCDF_Attr<char>("ProductionDateTime", WAVE));
    local_attributes.push_back(new NetCDF_Attr<char>("StartOrbitNumber", WAVE));
    local_attributes.push_back(new NetCDF_Attr<char>("StopOrbitNumber", WAVE));
    local_attributes.push_back(new NetCDF_Attr<char>("rev_number", WAVE));
    local_attributes.push_back(new NetCDF_Attr<char>("ancillary_data_descriptors", WAVE));
    local_attributes.push_back(new NetCDF_Attr<float>("EquatorCrossingLongitude", -9999.0f));
    local_attributes.push_back(new NetCDF_Attr<float>("orbit_inclination", 98.28f));
    local_attributes.push_back(new NetCDF_Attr<float>("rev_orbit_period", 5940.31f));
    local_attributes.push_back(new NetCDF_Attr<char>("NetCDF_version_id", nc_inq_libvers()));
    local_attributes.push_back(new NetCDF_Attr<char>("references", WAVE));

    for (unsigned int i = 0; i < local_attributes.size(); i++) {
        NCERR(local_attributes[i]->Write(ncid, NC_GLOBAL));
    }

    for (int i = local_attributes.size() - 1; i >= 0; i--) {
        delete local_attributes[i];
    }

    return 0;
}

