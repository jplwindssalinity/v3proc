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

#define USABLE_MASK 0x01
#define RAIN_MASK   0x02
#define BEAM_MASK   0x04
#define LAND_MASK   0x10
#define ICE_MASK    0x20
#define VALID_MASK  0x40

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
    const char *input_file;
    const char *output_file;
    char extended;
} l2b_to_netcdf_config;

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
    unsigned char chartmp;

    int max_ambiguities = l2b.frame.swath.GetMaxAmbiguityCount();
    int cross_track_dim_id, along_track_dim_id, ambiguities_dim_id;

    // Initialize the NetCDF DB
    NCERR(nc_create(config.output_file, (NC_NETCDF4 | NC_NOFILL) & 
                ~NC_CLASSIC_MODEL, &ncid));

    // Create the data dimensions
    NCERR(nc_def_dim(ncid, "cross_track", 
                (size_t)l2b.frame.swath.GetCrossTrackBins(), 
                &cross_track_dim_id));
    NCERR(nc_def_dim(ncid, "along_track", 
                (size_t)l2b.frame.swath.GetAlongTrackBins(), 
                &along_track_dim_id));
    if (config.extended) {
        NCERR(nc_def_dim(ncid, "ambiguities",
                (size_t)max_ambiguities, &ambiguities_dim_id));
    }

    /*************************************************************
     *
     * Declare the NetCDF variables
     *
     ************************************************************/

    enum variables {
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
        FLAGS,

        first_extended_variable,

        SEL_OBJ = first_extended_variable,
        NUDGE_SPEED,
        NUDGE_DIRECTION,
        AMBIG_SPEED,
        AMBIG_DIRECTION,
        AMBIG_OBJ,
        NUM_IN_FORE,
        NUM_IN_AFT,
        NUM_OUT_FORE,
        NUM_OUT_AFT,
        num_variables
    };
//    const int first_extended_variable = SEL_OBJ;

    struct netcdf_variable {
        const char *name;
        int type;
        int ndims;
        int *dims;
        int id;
    };


    int dimensions[3];

    /* We're being sneaky here.  We can pass dimensions to both two-
     * and three-dimensional variables, since they all use the same
     * first two dimensions.
     */
    dimensions[0] = cross_track_dim_id;
    dimensions[1] = along_track_dim_id;
    dimensions[2] = ambiguities_dim_id;

    netcdf_variable varlist[num_variables];
   
    varlist[LATITUDE].name  = "latitude";
    varlist[LATITUDE].type  = NC_FLOAT;
    varlist[LATITUDE].ndims = 2;
    varlist[LATITUDE].dims = dimensions; 

    varlist[LONGITUDE].name  = "longitude";
    varlist[LONGITUDE].type  = NC_FLOAT;
    varlist[LONGITUDE].ndims = 2;
    varlist[LONGITUDE].dims = dimensions; 
        
    if (config.extended) {
        varlist[NUDGE_SPEED].name  = "nudge_speed";
        varlist[NUDGE_SPEED].type  = NC_FLOAT;
        varlist[NUDGE_SPEED].ndims = 2;
        varlist[NUDGE_SPEED].dims = dimensions; 
           
        varlist[NUDGE_DIRECTION].name  = "nudge_direction";
        varlist[NUDGE_DIRECTION].type  = NC_FLOAT;
        varlist[NUDGE_DIRECTION].ndims = 2;
        varlist[NUDGE_DIRECTION].dims = dimensions; 
    }
     
    varlist[SEL_SPEED].name  = "final_speed";
    varlist[SEL_SPEED].type  = NC_FLOAT;
    varlist[SEL_SPEED].ndims = 2;
    varlist[SEL_SPEED].dims = dimensions; 
    
    varlist[SEL_DIRECTION].name  = "final_direction";
    varlist[SEL_DIRECTION].type  = NC_FLOAT;
    varlist[SEL_DIRECTION].ndims = 2;
    varlist[SEL_DIRECTION].dims = dimensions; 
   
    if (config.extended) { 
        varlist[SEL_OBJ].name  = "final_obj";
        varlist[SEL_OBJ].type  = NC_FLOAT;
        varlist[SEL_OBJ].ndims = 2;
        varlist[SEL_OBJ].dims = dimensions; 
      
        varlist[AMBIG_SPEED].name  = "ambiguity_speed";
        varlist[AMBIG_SPEED].type  = NC_FLOAT;
        varlist[AMBIG_SPEED].ndims = 3;
        varlist[AMBIG_SPEED].dims = dimensions; 
     
        varlist[AMBIG_DIRECTION].name  = "ambiguity_direction";
        varlist[AMBIG_DIRECTION].type  = NC_FLOAT;
        varlist[AMBIG_DIRECTION].ndims = 3;
        varlist[AMBIG_DIRECTION].dims = dimensions; 
    
        varlist[AMBIG_OBJ].name  = "ambiguity_obj";
        varlist[AMBIG_OBJ].type  = NC_FLOAT;
        varlist[AMBIG_OBJ].ndims = 3;
        varlist[AMBIG_OBJ].dims = dimensions; 
    }

    varlist[RAIN_IMPACT].name  = "rain_impact";
    varlist[RAIN_IMPACT].type  = NC_FLOAT;
    varlist[RAIN_IMPACT].ndims = 2;
    varlist[RAIN_IMPACT].dims = dimensions; 

    /* not in final product */
    if (config.extended) {
        varlist[NUM_IN_FORE].name  = "number_in_fore";
        varlist[NUM_IN_FORE].type  = NC_BYTE;
        varlist[NUM_IN_FORE].ndims = 2;
        varlist[NUM_IN_FORE].dims = dimensions; 
    
        varlist[NUM_IN_AFT].name  = "number_in_aft";
        varlist[NUM_IN_AFT].type  = NC_BYTE;
        varlist[NUM_IN_AFT].ndims = 2;
        varlist[NUM_IN_AFT].dims = dimensions; 
    
        varlist[NUM_OUT_FORE].name  = "number_out_fore";
        varlist[NUM_OUT_FORE].type  = NC_BYTE;
        varlist[NUM_OUT_FORE].ndims = 2;
        varlist[NUM_OUT_FORE].dims = dimensions; 
    
        varlist[NUM_OUT_AFT].name  = "number_out_aft";
        varlist[NUM_OUT_AFT].type  = NC_BYTE;
        varlist[NUM_OUT_AFT].ndims = 2;
        varlist[NUM_OUT_AFT].dims = dimensions; 
    }

    varlist[DIVERGENCE].name  = "divergence";
    varlist[DIVERGENCE].type  = NC_FLOAT;
    varlist[DIVERGENCE].ndims = 2;
    varlist[DIVERGENCE].dims = dimensions; 

    varlist[CURL].name  = "curl";
    varlist[CURL].type  = NC_FLOAT;
    varlist[CURL].ndims = 2;
    varlist[CURL].dims = dimensions; 

    varlist[WIND_STRESS].name  = "wind_stress";
    varlist[WIND_STRESS].type  = NC_FLOAT;
    varlist[WIND_STRESS].ndims = 2;
    varlist[WIND_STRESS].dims = dimensions; 

    varlist[WIND_STRESS_DIVERGENCE].name  = "wind_stress_divergence";
    varlist[WIND_STRESS_DIVERGENCE].type  = NC_FLOAT;
    varlist[WIND_STRESS_DIVERGENCE].ndims = 2;
    varlist[WIND_STRESS_DIVERGENCE].dims = dimensions; 

    varlist[WIND_STRESS_CURL].name  = "wind_stress_curl";
    varlist[WIND_STRESS_CURL].type  = NC_FLOAT;
    varlist[WIND_STRESS_CURL].ndims = 2;
    varlist[WIND_STRESS_CURL].dims = dimensions; 

    varlist[FLAGS].name  = "flags";
    varlist[FLAGS].type  = NC_BYTE;
    varlist[FLAGS].ndims = 2;
    varlist[FLAGS].dims = dimensions; 

    for (int i = 0; i < (config.extended ? num_variables : 
                first_extended_variable); i++) {
        NCERR(nc_def_var(ncid, varlist[i].name, varlist[i].type,
                   varlist[i].ndims, varlist[i].dims, &varlist[i].id));
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

    chartmp = USABLE_MASK;
    NCERR(nc_put_att_ubyte(ncid, varlist[FLAGS].id, "usable_mask", 
                NC_BYTE,  (size_t)(1), &chartmp));
    chartmp = RAIN_MASK;
    NCERR(nc_put_att_ubyte(ncid, varlist[FLAGS].id, "rain_mask",
                NC_BYTE,  (size_t)(1), &chartmp));
    chartmp = BEAM_MASK;
    NCERR(nc_put_att_ubyte(ncid, varlist[FLAGS].id, "beam_mask",
                NC_BYTE,  (size_t)(1), &chartmp));
    chartmp = LAND_MASK;
    NCERR(nc_put_att_ubyte(ncid, varlist[FLAGS].id, "land_mask", 
                NC_BYTE,  (size_t)(1), &chartmp));
    chartmp = ICE_MASK;
    NCERR(nc_put_att_ubyte(ncid, varlist[FLAGS].id, "ice_mask",
                NC_BYTE,  (size_t)(1), &chartmp));
    chartmp = VALID_MASK;
    NCERR(nc_put_att_ubyte(ncid, varlist[FLAGS].id, "point_valid_mask", 
                NC_BYTE,  (size_t)(1), &chartmp));

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
    int zero = 0;
    float zerof = 0.0f;

    for (idx[0] = 0; (int)idx[0] < l2b.frame.swath.GetCrossTrackBins(); idx[0]++) {
        for (idx[1] = 0; (int)idx[1] < l2b.frame.swath.GetAlongTrackBins(); idx[1]++) {

            wvc = l2b.frame.swath.swath[idx[0]][idx[1]];

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
                NCERR(nc_put_var1_float(ncid, varlist[SEL_DIRECTION].id, idx, 
                            &wvc->selected->dir));
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
                    NCERR(nc_put_var1_float(ncid, varlist[NUDGE_DIRECTION].id, idx, 
                                &wvc->nudgeWV->dir));
    
                    NCERR(nc_put_var1_ubyte(ncid, varlist[NUM_IN_FORE].id, idx, 
                                &wvc->numInFore));
                    NCERR(nc_put_var1_ubyte(ncid, varlist[NUM_IN_AFT].id, idx, 
                                &wvc->numInAft));
                    NCERR(nc_put_var1_ubyte(ncid, varlist[NUM_OUT_FORE].id, idx, 
                                &wvc->numOutFore));
                    NCERR(nc_put_var1_ubyte(ncid, varlist[NUM_OUT_AFT].id, idx, 
                                &wvc->numOutAft));
        
    
                    WindVectorPlus *wv = wvc->ambiguities.GetHead();
                    int num_ambiguities = wvc->ambiguities.NodeCount();
        
                    for (idx[2] = 0; (int)idx[2] < num_ambiguities && wv != NULL; 
                            idx[2]++, wv = wvc->ambiguities.GetNext()) {
        
                        NCERR(nc_put_var1(ncid, varlist[AMBIG_SPEED].id, 
                                    idx, &wv->spd));
                        NCERR(nc_put_var1(ncid, varlist[AMBIG_DIRECTION].id, 
                                    idx, &wv->spd));
                        NCERR(nc_put_var1(ncid, varlist[AMBIG_OBJ].id, 
                                    idx, &wv->spd));
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
                    NCERR(nc_put_var1_ubyte(ncid, varlist[NUM_IN_FORE].id, idx, 
                                (unsigned char *)&zero));
                    NCERR(nc_put_var1_ubyte(ncid, varlist[NUM_IN_AFT].id, idx, 
                                (unsigned char *)&zero));
                    NCERR(nc_put_var1_ubyte(ncid, varlist[NUM_OUT_FORE].id, idx, 
                                (unsigned char *)&zero));
                    NCERR(nc_put_var1_ubyte(ncid, varlist[NUM_OUT_AFT].id, idx, 
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

