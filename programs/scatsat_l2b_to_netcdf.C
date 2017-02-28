//==============================================================//
// Copyright (C) 1998-2012, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    scatsat_l2b_to_netcdf
//
// SYNOPSIS
//    scatsat_l2b_to_netcdf config_file
//
// DESCRIPTION
//    Reads frames from OS2 L2B file, incorporates metadata from associated L1B
//    HDF5 file and writes out to an NetCDF file
//
// OPTIONS
//
// AUTHOR
//    Thomas Werne
//    werne@jpl.nasa.gov
//
//    Alex Fore
//    fore@jpl
//----------------------------------------------------------------------

//-----------------------//
// Configuration Control //
//-----------------------//

static const char rcs_id[] =
    "@(#) $Id$";

//----------//
// INCLUDES //
//----------//

#include <ctime>
#include <ctype.h>
#include <libgen.h>
#include <netcdf.h>
#include <getopt.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <hdf5.h>
#include <hdf5_hl.h>
#include <sys/time.h>

#include "NetCDF_Attr.h"
#include "NetCDF_Var.h"
#include "Misc.h"
#include "L2B.h"
#include "ConfigList.h"
#include "List.h"
#include "BufferedList.h"
#include "Tracking.h"

using namespace std;

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

#define HDFERR(cond) \
    if (cond) { \
        fprintf(stderr, "Error in " __FILE__ " @ " S(__LINE__) ":\n"); \
        H5Eprint1(stderr); \
        exit(EXIT_FAILURE); \
    }

#define ARRAY_LEN(x) (sizeof(x)/sizeof(x[0]))

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

#define DATASET_TITLE "Oceansat-II Level 2B Ocean Wind Vectors in 12.5km Slice Composites"
#define BUILD_ID "v2_0_0:A"
#define VERSION_ID "2"

#define EPOCH    "1999-001T00:00:00.000 UTC"
#define EPOCH_CF "1999-1-1 0:0:0"

// Used to separate date-time strings from YYYY-DDDTHH:MM:SS.sss into
// {YYYY-DDD,HH:MM:SS.sss}
#define DT_SPLIT(x) \
do {                \
    (x)[8] = '\0';  \
} while(0)

#define DT_DATE(x) (x + 0)
#define DT_TIME(x) (x + 9)


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
    const char *nc_file;
    const char *l1bhdf_file_asc;
    const char *l1bhdf_file_dec;
    char *revtag;
    float hpol_adj;
    float vpol_adj;
} l2b_to_netcdf_config;

typedef struct {
    float lambda_0;
    float inclination;
    float rev_period;
    int xt_steps;
    double at_res;
    double xt_res;
} latlon_config;

static int parse_commandline(int argc, char **argv,
        l2b_to_netcdf_config *config);
static int set_global_attributes(int argc, char **argv,
        const l2b_to_netcdf_config *config, ConfigList* config_list,
        const L2B *l2b, int ncid);
static void read_attr_h5(hid_t obj_id, char *name, void *buffer);
static void bin_to_latlon(int at_ind, int ct_ind,
        const latlon_config *config, float *lat, float *lon);
static char * rtrim(char *str);
static struct timeval str_to_timeval(const char *end);
static double difftime(const struct timeval t1, const struct timeval t0);

static struct timeval epoch;

static int16_t flag_masks[] = {SIGMA0_MASK, AZIMUTH_DIV_MASK, COASTAL_MASK,
    ICE_EDGE_MASK, WIND_RETRIEVAL_MASK, HIGH_WIND_MASK, LOW_WIND_MASK,
    RAIN_IMPACT_UNUSABLE_MASK, RAIN_IMPACT_MASK, AVAILABLE_DATA_MASK};
static int16_t eflag_masks[] = {RAIN_CORR_NOT_APPL_MASK, NEG_WIND_SPEED_MASK,
    ALL_AMBIG_CONTRIB_MASK, RAIN_CORR_LARGE_MASK};

struct timeval rev_end_time, rev_start_time;

//--------------//
// MAIN PROGRAM //
//--------------//

int main(int argc, char **argv) {

    char* config_file = argv[1];

    ConfigList config_list;
    l2b_to_netcdf_config run_config;
    L2B l2b;

    config_list.Read(config_file);

    // set run_config from config file
    run_config.command = no_path(argv[0]);
    run_config.l2b_file = config_list.Get("L2B_FILE");
    run_config.nc_file = config_list.Get("L2B_NETCDF_FILE");
    run_config.l1bhdf_file_asc = config_list.Get("S1L1B_ASC_FILE");
    run_config.l1bhdf_file_dec = config_list.Get("S1L1B_DEC_FILE");
    run_config.revtag = config_list.Get("REVNO");

    config_list.GetFloat("HH_BIAS_ADJ", &run_config.hpol_adj);
    config_list.GetFloat("VV_BIAS_ADJ", &run_config.vpol_adj);

    /* File IDs */
    int ncid;

    /* Net CDF dimension information */
    int cross_track_dim_id, along_track_dim_id;
    size_t cross_track_dim_sz, along_track_dim_sz;
    int dimensions[2], dimensions_sz[2];

    /* For populating Net CDF file */
    size_t idx[2];
    WVC *wvc;
    int16 flags, eflags;
    float conversion;

    float lat, lon;

    // For generating time values
    double rev_length_time;

    // Define the local timezone to UTC so that mktime() doesn't try to
    // apply DST rules
    setenv("TZ", "UTC", 1);
    tzset();

    epoch = str_to_timeval(EPOCH);

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

    // Initialize the NetCDF DB
    NCERR(nc_create(run_config.nc_file, NC_WRITE, &ncid));

    ERR(set_global_attributes(
        argc, argv, &run_config, &config_list, &l2b, ncid) != 0);

    // Create the data dimensions
    along_track_dim_sz = (size_t)l2b.frame.swath.GetAlongTrackBins();
    cross_track_dim_sz = (size_t)l2b.frame.swath.GetCrossTrackBins();

    NCERR(nc_def_dim(ncid, "along_track", along_track_dim_sz,
                &along_track_dim_id));
    NCERR(nc_def_dim(ncid, "cross_track", cross_track_dim_sz,
                &cross_track_dim_id));

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
    dimensions_sz[0] = along_track_dim_sz;
    dimensions_sz[1] = cross_track_dim_sz;

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
        float retrieved_dir_fill = -9999.0f, retrieved_dir_min = 0.0f, retrieved_dir_max = 360.0f;
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
    NetCDF_Var<double> *time_var = new NetCDF_Var<double>("time", ncid, 1, dimensions, dimensions_sz);
        time_var->AddAttribute(new NetCDF_Attr<char>("long_name", "date and time"));
        time_var->AddAttribute(new NetCDF_Attr<char>("units", "seconds since " EPOCH_CF));
    NetCDF_Var<short> *flags_var = new NetCDF_Var<short>("flags", ncid, 2, dimensions, dimensions_sz);
        short flags_fill = 0x7fff, flags_min = 0, flags_max = 32643;
        flags_var->AddAttribute(new NetCDF_Attr<short>("_FillValue", flags_fill));
        flags_var->AddAttribute(new NetCDF_Attr<short>("valid_min", flags_min));
        flags_var->AddAttribute(new NetCDF_Attr<short>("valid_max", flags_max));
        flags_var->AddAttribute(new NetCDF_Attr<char>("long_name", "wind vector cell quality flags"));
        flags_var->AddAttribute(new NetCDF_Attr<short>("flag_masks", flag_masks, sizeof(flag_masks)/sizeof(flag_masks[0])));
        flags_var->AddAttribute(new NetCDF_Attr<char>("flag_meanings", "adequate_sigma0_flag adequate_azimuth_diversity_flag coastal_flag ice_edge_flag wind_retrieval_flag high_wind_speed_flag low_wind_speed_flag rain_impact_flag_usable rain_impact_flag available_data_flag"));
        flags_var->AddAttribute(new NetCDF_Attr<char>("units", "bit"));
        flags_var->AddAttribute(new NetCDF_Attr<char>("coordinates", "lon lat"));
    NetCDF_Var<short> *eflags_var = new NetCDF_Var<short>("eflags", ncid, 2, dimensions, dimensions_sz);
        short eflags_fill = 0x7fff, eflags_min = 0, eflags_max = 15;
        eflags_var->AddAttribute(new NetCDF_Attr<short>("_FillValue", eflags_fill));
        eflags_var->AddAttribute(new NetCDF_Attr<short>("valid_min", eflags_min));
        eflags_var->AddAttribute(new NetCDF_Attr<short>("valid_max", eflags_max));
        eflags_var->AddAttribute(new NetCDF_Attr<char>("long_name", "extended wind vector cell quality flags"));
        eflags_var->AddAttribute(new NetCDF_Attr<short>("flag_masks", eflag_masks, sizeof(eflag_masks)/sizeof(eflag_masks[0])));
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
        nudge_dir_var->AddAttribute(new NetCDF_Attr<char>("units", "degrees"));
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
    NetCDF_Var<float> *cross_track_bias_var = new NetCDF_Var<float>("cross_track_wind_speed_bias", ncid, 2, dimensions, dimensions_sz);
        float cross_track_bias_fill = -9999.0f, cross_track_bias_min = -100.0f, cross_track_bias_max = 100.f;
        cross_track_bias_var->AddAttribute(new NetCDF_Attr<float>("_FillValue", cross_track_bias_fill));
        cross_track_bias_var->AddAttribute(new NetCDF_Attr<float>("valid_min", cross_track_bias_min));
        cross_track_bias_var->AddAttribute(new NetCDF_Attr<float>("valid_max", cross_track_bias_max));
        cross_track_bias_var->AddAttribute(new NetCDF_Attr<char>("long_name", "relative wind speed bias with respect to sweet spot"));
        cross_track_bias_var->AddAttribute(new NetCDF_Attr<char>("units", "m s-1"));
        cross_track_bias_var->AddAttribute(new NetCDF_Attr<float>("scale_factor", 1.0f));
        cross_track_bias_var->AddAttribute(new NetCDF_Attr<char>("coordinates", "lon lat"));
    NetCDF_Var<float> *atm_spd_bias_var = new NetCDF_Var<float>("atmospheric_speed_bias", ncid, 2, dimensions, dimensions_sz);
        float atm_spd_bias_fill = -9999.0f, atm_spd_bias_min = -100.0f, atm_spd_bias_max = 100.f;
        atm_spd_bias_var->AddAttribute(new NetCDF_Attr<float>("_FillValue", atm_spd_bias_fill));
        atm_spd_bias_var->AddAttribute(new NetCDF_Attr<float>("valid_min", atm_spd_bias_min));
        atm_spd_bias_var->AddAttribute(new NetCDF_Attr<float>("valid_max", atm_spd_bias_max));
        atm_spd_bias_var->AddAttribute(new NetCDF_Attr<char>("long_name", "atmospheric speed bias"));
        atm_spd_bias_var->AddAttribute(new NetCDF_Attr<char>("units", "m s-1"));
        atm_spd_bias_var->AddAttribute(new NetCDF_Attr<float>("scale_factor", 1.0f));
        atm_spd_bias_var->AddAttribute(new NetCDF_Attr<char>("coordinates", "lon lat"));
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
    vars.push_back(cross_track_bias_var);
    vars.push_back(atm_spd_bias_var);
    vars.push_back(num_ambig_var);

    // Push definitions into NetCDF file
    for(vector <NetCDF_Var_Base *>::iterator it = vars.begin(); it < vars.end(); it++) {
        NCERR((*it)->Define());
        NCERR((*it)->WriteAttributes());
    }

    NCERR(nc_enddef(ncid));

    latlon_config orbit_config;
    NCERR(nc_get_att_float(ncid, NC_GLOBAL, "Equator_Crossing_Longitude", &orbit_config.lambda_0));
    NCERR(nc_get_att_float(ncid, NC_GLOBAL, "orbit_inclination", &orbit_config.inclination));
    NCERR(nc_get_att_float(ncid, NC_GLOBAL, "rev_orbit_period", &orbit_config.rev_period));
    orbit_config.rev_period *= 60;  // orbit period -> seconds
    orbit_config.xt_steps = 2*l2b.header.zeroIndex;
    orbit_config.at_res = l2b.header.alongTrackResolution;
    orbit_config.xt_res = l2b.header.crossTrackResolution;

    
    rev_length_time = difftime(rev_end_time, rev_start_time)*(along_track_dim_sz + 1)/along_track_dim_sz;

    /* Same sneakiness with idx as described above with dimensions */
    for (idx[0] = 0; idx[0] < along_track_dim_sz; idx[0]++) {
        double this_time;

        this_time = (idx[0] + 0.5)/along_track_dim_sz*rev_length_time + difftime(rev_start_time, epoch);

        time_var->SetData(idx, this_time);

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
                    atm_spd_bias_var->SetData(idx, wvc->speedBias);
                }

                if (wvc->selected->spd < retrieved_speed_min) {
                    wvc->selected->spd = retrieved_speed_min;
                } else if (wvc->selected->spd >= retrieved_speed_max) {
                    wvc->selected->spd = retrieved_speed_max;
                }

                retrieved_speed_var->SetData(idx, wvc->selected->spd);

                if (IS_SET(eflags, RAIN_CORR_NOT_APPL_MASK)) {
                    wind_speed_uncorr_var->SetData(idx, wvc->selected->spd);
                    atm_spd_bias_var->SetData(idx, 0);
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

                cross_track_bias_var->SetData(idx, cross_track_bias_fill);

                num_ambig_var->SetData(idx, wvc->numAmbiguities);

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
                cross_track_bias_var->SetData(idx, cross_track_bias_fill);
                atm_spd_bias_var->SetData(idx, atm_spd_bias_fill);

                num_ambig_var->SetData(idx, num_ambig_fill);

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

    free(run_config.revtag);
    run_config.revtag = NULL;

    return(0);
}

static int set_global_attributes(int argc, char **argv,
        const l2b_to_netcdf_config *cfg, ConfigList* config_list,
        const L2B *l2b, int ncid) {

    vector <class NetCDF_Attr_Base *> global_attributes;
    const char *date_time_fmt = "%Y-%jT%H:%M:%S%z";
    const char *whoami;
    int history_len = 23;   // Max length from date_time_fmt
    time_t now;
    char *history;

    // Compute history attribute string length
    whoami = getenv("LOGNAME");
    if (whoami != NULL) {
        history_len += strlen(whoami) + 1;
    }
    for (int i = 0; i < argc; i++) {
        history_len += strlen(argv[i]) + 1;
    }
    history_len += 1;  // For final '\n'

    history = (typeof history)calloc(history_len, sizeof(*history));

    // Insert date+time
    now = time(NULL);
    strftime(history, history_len, date_time_fmt, gmtime(&now));

    // Insert user name
    if (whoami != NULL) {
        history[strlen(history)] = ' ';
        strcat(history, whoami);
    }

    // Insert program invocation
    for (int i = 0; i < argc; i++) {
        history[strlen(history)] = ' ';
        strcat(history, argv[i]);
    }
    history[strlen(history)] = '\n';

    // Open HDF file
    hid_t l1bhdf_id_asc, science_group_asc;
    hid_t l1bhdf_id_dec, science_group_dec;
    char attribute[256], rev_number[32];

    HDFERR((l1bhdf_id_asc = H5Fopen(cfg->l1bhdf_file_asc, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0);
    HDFERR((science_group_asc = H5Gopen(l1bhdf_id_asc, "science_data", H5P_DEFAULT)) < 0);
    HDFERR((l1bhdf_id_dec = H5Fopen(cfg->l1bhdf_file_asc, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0);
    HDFERR((science_group_dec = H5Gopen(l1bhdf_id_dec, "science_data", H5P_DEFAULT)) < 0);

    global_attributes.push_back(new NetCDF_Attr<char>("title", DATASET_TITLE));
    global_attributes.push_back(new NetCDF_Attr<char>("institution", "JPL"));
    global_attributes.push_back(new NetCDF_Attr<char>("source", "SCATSAT-1 Scatterometer"));
    global_attributes.push_back(new NetCDF_Attr<char>("comment", "SCATSAT-1 Level 1B Data Processed to Winds Using QuikSCAT v3 Algorithms"));
    global_attributes.push_back(new NetCDF_Attr<char>("history", history));

    // TODO
    global_attributes.push_back(new NetCDF_Attr<float>("hpol_adj", cfg->hpol_adj));
    global_attributes.push_back(new NetCDF_Attr<float>("vpol_adj", cfg->vpol_adj));

    global_attributes.push_back(new NetCDF_Attr<char>("Conventions", "CF-1.6"));
    global_attributes.push_back(new NetCDF_Attr<char>("data_format_type", "NetCDF Classic"));
    global_attributes.push_back(new NetCDF_Attr<char>("producer_agency", "NASA"));
    global_attributes.push_back(new NetCDF_Attr<char>("build_id", BUILD_ID));
    global_attributes.push_back(new NetCDF_Attr<char>("ancillary_data_descriptors", ""));
    global_attributes.push_back(new NetCDF_Attr<char>("processing_level", "L2B"));

    char timestr[9];
    strftime(timestr, sizeof(timestr), "%Y-%j", gmtime(&now));
    global_attributes.push_back(new NetCDF_Attr<char>("creation_date", timestr));
    strftime(timestr, sizeof(timestr), "%T", gmtime(&now));
    global_attributes.push_back(new NetCDF_Attr<char>("creation_time", timestr));

    global_attributes.push_back(new NetCDF_Attr<char>("LongName", DATASET_TITLE));
    global_attributes.push_back(new NetCDF_Attr<char>("ShortName", "SCATSAT_LEVEL_2B_OWV_COMP_12_V2"));
    strncpy(attribute, cfg->nc_file, ARRAY_LEN(attribute) - 1);
    
    char l1bfiles[2048];
    sprintf(l1bfiles, "%s; %s", cfg->l1bhdf_file_asc, cfg->l1bhdf_file_dec);
    global_attributes.push_back(new NetCDF_Attr<char>("GranulePointerAsc", basename(attribute)));
    strncpy(attribute, l1bfiles, ARRAY_LEN(attribute) - 1);

    global_attributes.push_back(new NetCDF_Attr<char>("InputPointer", basename(attribute)));
    global_attributes.push_back(new NetCDF_Attr<char>("l2b_algorithm_descriptor",
                "Uses QSCAT2012 GMF from Jet Propulsion Laboratory constructed using\n"
                "data from the QuikSCAT instrument repointed to ScatSat incidence\n"
                "angles.  Applies median filter technique for ambiguity removal.\n"
                "Ambiguity removal median filter is based on wind vectors over a 7 by 7\n"
                "wind vector cell window.  Applies no median filter weights. Enhances\n"
                "the direction of the selected ambiguity based on the range of\n"
                "directions which exceed a specified probability threshold.\n"
                "Applies multi-pass median filter technique to reduce the effects of\n"
                "rain flagged cells on ambiguity selection.\n"
                "Applies Neural Network Rain Correction."));
    global_attributes.push_back(new NetCDF_Attr<char>("nudging_method", "NWP Weather Map probability threshold nudging"));
    global_attributes.push_back(new NetCDF_Attr<char>("median_filter_method", "Wind vector median"));
    global_attributes.push_back(new NetCDF_Attr<char>("sigma0_granularity", "slice composites"));
    global_attributes.push_back(new NetCDF_Attr<char>("InstrumentShortName", "OSCAT2"));
    global_attributes.push_back(new NetCDF_Attr<char>("PlatformType", "spacecraft"));
    global_attributes.push_back(new NetCDF_Attr<char>("PlatformLongName", "SCATSAT-1"));
    global_attributes.push_back(new NetCDF_Attr<char>("PlatformShortName", "SCATSAT"));

//     strcpy(attribute, cfg->revtag);
//     attribute[5] = '\0';   // Separate starting & stop orbit #s
    global_attributes.push_back(new NetCDF_Attr<char>("rev_number", cfg->revtag));

    global_attributes.push_back(new NetCDF_Attr<float>("cross_track_resolution", &l2b->header.crossTrackResolution));
    global_attributes.push_back(new NetCDF_Attr<float>("along_track_resolution", &l2b->header.alongTrackResolution));
    global_attributes.push_back(new NetCDF_Attr<int>("zero_index", &l2b->header.zeroIndex));
    global_attributes.push_back(new NetCDF_Attr<char>("version_id", VERSION_ID));

    read_attr_h5(science_group_asc, "Processor Version", attribute);
    global_attributes.push_back(new NetCDF_Attr<char>("L1B_Processor_Version", rtrim(attribute)));
    read_attr_h5(science_group_asc, "Ephemeris Type", attribute);
    global_attributes.push_back(new NetCDF_Attr<char>("Ephemeris_Type", rtrim(attribute)));
    read_attr_h5(science_group_asc, "L1B Actual Scans", attribute);
    global_attributes.push_back(new NetCDF_Attr<short>("L1B_Actual_Scans_Asc", (short)atoi(attribute)));
    read_attr_h5(science_group_dec, "L1B Actual Scans", attribute);
    global_attributes.push_back(new NetCDF_Attr<short>("L1B_Actual_Scans_Dec", (short)atoi(attribute)));
    read_attr_h5(science_group_asc, "Production Date", attribute);  DT_SPLIT(attribute);
    global_attributes.push_back(new NetCDF_Attr<char>("L1BProductionDate", rtrim(DT_DATE(attribute))));
    global_attributes.push_back(new NetCDF_Attr<char>("L1BProductionTime", rtrim(DT_TIME(attribute))));

    read_attr_h5(science_group_asc, "Orbit Inclination", attribute);
    global_attributes.push_back(new NetCDF_Attr<float>("orbit_inclination", atof(attribute)));
    read_attr_h5(science_group_asc, "Orbit Semi-major Axis", attribute);
    global_attributes.push_back(new NetCDF_Attr<float>("rev_orbit_semimajor_axis", atof(attribute)));
    read_attr_h5(science_group_asc, "Orbit Eccentricity", attribute);
    global_attributes.push_back(new NetCDF_Attr<float>("rev_orbit_eccentricity", atof(attribute)));
    global_attributes.push_back(new NetCDF_Attr<char>("NetCDF_version_id", nc_inq_libvers()));
    global_attributes.push_back(new NetCDF_Attr<char>("references", ""));

    char orbit_start[32], orbit_stop[32], node_time[32];

    strcpy(orbit_start, config_list->Get("REV_START_TIME"));
    strcpy(orbit_stop, config_list->Get("REV_STOP_TIME"));
    strcpy(node_time, config_list->Get("ASC_NODE_TIME"));

    DT_SPLIT(orbit_start);
    DT_SPLIT(orbit_stop);
    DT_SPLIT(node_time);

    rev_start_time = str_to_timeval(orbit_start);
    rev_end_time   = str_to_timeval(orbit_stop);

    float node_long, rev_period;
    config_list->GetFloat("REV_PERIOD", &rev_period);
    config_list->GetFloat("ASC_NODE_LONG", &node_long);

    global_attributes.push_back(new NetCDF_Attr<char>("RangeBeginningDate", rtrim(DT_DATE(orbit_start))));
    global_attributes.push_back(new NetCDF_Attr<char>("RangeBeginningTime", rtrim(DT_TIME(orbit_start))));
    global_attributes.push_back(new NetCDF_Attr<char>("RangeEndingDate", rtrim(DT_DATE(orbit_stop))));
    global_attributes.push_back(new NetCDF_Attr<char>("RangeEndingTime", rtrim(DT_TIME(orbit_stop))));
    global_attributes.push_back(new NetCDF_Attr<char>("EquatorCrossingDate", rtrim(DT_DATE(node_time))));
    global_attributes.push_back(new NetCDF_Attr<char>("EquatorCrossingTime", rtrim(DT_TIME(node_time))));
    global_attributes.push_back(new NetCDF_Attr<float>("Equator_Crossing_Longitude", node_long));
    global_attributes.push_back(new NetCDF_Attr<float>("rev_orbit_period", rev_period/60.0));

    for (unsigned int i = 0; i < global_attributes.size(); i++) {
        NCERR(global_attributes[i]->Write(ncid, NC_GLOBAL));
    }

    for (int i = global_attributes.size() - 1; i >= 0; i--) {
        delete global_attributes[i];
    }

    FREE(history);

    H5Fclose(l1bhdf_id_asc);
    H5Fclose(l1bhdf_id_dec);

    return 0;
}

static char * rtrim(char *str) {
    for (unsigned int i = 0; i < strlen(str); i++) {
        if (isspace(str[i])) {
            str[i] = '\0';
            return str;
        }
    }
    return str;
}

// From A. Fore
static void read_attr_h5(hid_t obj_id, char *name, void *buffer) {
    hid_t attr_id;
    HDFERR((attr_id = H5Aopen(obj_id, name, H5P_DEFAULT)) < 0);
    HDFERR((H5Aread(attr_id, H5Aget_type(attr_id), buffer)) < 0);
    HDFERR((H5Aclose(attr_id)) < 0);
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

    while (lambda < 0) {
        lambda += two_pi;
    }
    while (lambda >= two_pi) {
        lambda -= two_pi;
    }

    phi = atanf((tanf(lambda_pp)*cosf(lambda_t) -
                cosf(inc)*sinf(lambda_t))/((1 - e2)*
                sinf(inc)));

    *lon = RAD_TO_DEG(lambda);
    *lat = RAD_TO_DEG(phi);
}


static struct timeval str_to_timeval(const char *end) {

    struct tm end_tm;
    struct timeval end_tv;

    end_tm.tm_year = atoi(end +  0) - 1900;
    end_tm.tm_mon  = 0;
    // TODO: Are the ISRO DOYs 1- or 0- indexed?
    end_tm.tm_mday = atoi(end +  5);
    end_tm.tm_hour = atoi(end +  9);
    end_tm.tm_min  = atoi(end + 12);
    end_tm.tm_sec  = atoi(end + 15);

    end_tv.tv_sec = mktime(&end_tm);
    end_tv.tv_usec = atoi(end + 18)*1000;

    return end_tv;
}

static double difftime(const struct timeval t1, const struct timeval t0) {
    double retval;

    retval = difftime(t1.tv_sec, t0.tv_sec);
    retval += 1.0e-6*(t1.tv_usec - t0.tv_usec);

    return retval;
}

