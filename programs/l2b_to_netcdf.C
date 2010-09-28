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

#define NCERR(call) \
{ \
    int error; \
    if ((error = call) != NC_NOERR) { \
        fprintf(stderr, "Error %d in %s @ %d: %s\n", error, \
                __FILE__, __LINE__, nc_strerror(error)); \
        exit(EXIT_FAILURE); \
    } \
}

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

const char* usage_array[] = { "<input_file>", "<output_file>", 0};

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
    if (argc != 3 && argc != 4)
        usage(command, usage_array, 1);

    int clidx = 1;
    const char* input_file = argv[clidx++];
    const char* output_file = argv[clidx++];
    int hdf_flag = 0;
    if (argc >= 4)
    {
        hdf_flag = atoi(argv[clidx++]);
    }

    //-------------------//
    // create L2B object //
    //-------------------//

    L2B l2b;

    //---------------------//
    // open the input file //
    //---------------------//

    if (hdf_flag)
    {
/*
        l2b.SetInputFilename(input_file);
        if (l2b.ReadHDF() == 0)
*/
        if (! l2b.ReadPureHdf(input_file))
        {
            fprintf(stderr, "%s: error reading HDF L2B file %s\n", command,
                input_file);
            exit(1);
        }
    }
    else
    {
        if (! l2b.OpenForReading(input_file))
        {
            fprintf(stderr, "%s: error opening input file %s\n", command,
                input_file);
            exit(1);
        }

        //---------------------//
        // copy desired frames //
        //---------------------//

        if (! l2b.ReadHeader())
        {
            fprintf(stderr, "%s: error reading from input file %s\n", command,
                input_file);
            exit(1);
        }
        if (! l2b.ReadDataRec())
        {
            fprintf(stderr, "%s: error reading from input file %s\n", command,
                input_file);
            exit(1);
        }
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

    int cross_track_dim_id, along_track_dim_id, ambiguities_dim_id;

    // Initialize the NetCDF DB
    NCERR(nc_create(output_file, (NC_NETCDF4 | NC_NOFILL) & 
                ~NC_CLASSIC_MODEL, &ncid));

    // Create the data dimensions
    NCERR(nc_def_dim(ncid, "Cross Track", 
                (size_t)l2b.frame.swath.GetCrossTrackBins(), 
                &cross_track_dim_id));
    NCERR(nc_def_dim(ncid, "Along Track", 
                (size_t)l2b.frame.swath.GetAlongTrackBins(), 
                &along_track_dim_id));
    NCERR(nc_def_dim(ncid, "Ambiguities",
                (size_t)l2b.frame.swath.GetMaxAmbiguityCount(), 
                &ambiguities_dim_id));

    /*************************************************************
     *
     * Declare the NetCDF variables
     *
     ************************************************************/

    enum variables {
        POINT_VALID = 0,
        LATITUDE,
        LONGITUDE,
        NUDGE_SPEED,
        NUDGE_DIRECTION,
        NUDGE_OBJ,
        SEL_SPEED,
        SEL_DIRECTION,
        SEL_OBJ,
        AMBIG_SPEED,
        AMBIG_DIRECTION,
        AMBIG_OBJ,
        RAIN_PROBABILITY,
        LAND_ICE_FLAG,
        QUALITY_FLAG,
        RAIN_FLAG,
        RAIN_IMPACT,
        NUM_IN_FORE,
        NUM_IN_AFT,
        NUM_OUT_FORE,
        NUM_OUT_AFT
    };
    const int num_variables = 1 + NUM_OUT_AFT;

    struct netcdf_variable {
        const char *name;
        int type;
        int ndims;
        int *dims;
        int (*put_var1)(int, int, const size_t *, const void *);
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
   
    varlist[POINT_VALID].name = "Valid Poin";
    varlist[POINT_VALID].type = NC_BYTE;
    varlist[POINT_VALID].ndims = 2;
    varlist[POINT_VALID].dims = dimensions;

    varlist[LATITUDE].name  = "Latitude";
    varlist[LATITUDE].type  = NC_FLOAT;
    varlist[LATITUDE].ndims = 2;
    varlist[LATITUDE].dims = dimensions; 

    varlist[LONGITUDE].name  = "Longitude";
    varlist[LONGITUDE].type  = NC_FLOAT;
    varlist[LONGITUDE].ndims = 2;
    varlist[LONGITUDE].dims = dimensions; 
        
    varlist[NUDGE_SPEED].name  = "Nudge Speed";
    varlist[NUDGE_SPEED].type  = NC_FLOAT;
    varlist[NUDGE_SPEED].ndims = 2;
    varlist[NUDGE_SPEED].dims = dimensions; 
       
    varlist[NUDGE_DIRECTION].name  = "Nudge Direction";
    varlist[NUDGE_DIRECTION].type  = NC_FLOAT;
    varlist[NUDGE_DIRECTION].ndims = 2;
    varlist[NUDGE_DIRECTION].dims = dimensions; 
      
    varlist[NUDGE_OBJ].name  = "Nudge Obj";
    varlist[NUDGE_OBJ].type  = NC_FLOAT;
    varlist[NUDGE_OBJ].ndims = 2;
    varlist[NUDGE_OBJ].dims = dimensions; 
     
    varlist[SEL_SPEED].name  = "Selected Speed";
    varlist[SEL_SPEED].type  = NC_FLOAT;
    varlist[SEL_SPEED].ndims = 2;
    varlist[SEL_SPEED].dims = dimensions; 
    
    varlist[SEL_DIRECTION].name  = "Selected Direction";
    varlist[SEL_DIRECTION].type  = NC_FLOAT;
    varlist[SEL_DIRECTION].ndims = 2;
    varlist[SEL_DIRECTION].dims = dimensions; 
   
    varlist[SEL_OBJ].name  = "Selected Obj";
    varlist[SEL_OBJ].type  = NC_FLOAT;
    varlist[SEL_OBJ].ndims = 2;
    varlist[SEL_OBJ].dims = dimensions; 
  
    varlist[AMBIG_SPEED].name  = "Ambiguity Speed";
    varlist[AMBIG_SPEED].type  = NC_FLOAT;
    varlist[AMBIG_SPEED].ndims = 3;
    varlist[AMBIG_SPEED].dims = dimensions; 
 
    varlist[AMBIG_DIRECTION].name  = "Ambiguity Direction";
    varlist[AMBIG_DIRECTION].type  = NC_FLOAT;
    varlist[AMBIG_DIRECTION].ndims = 3;
    varlist[AMBIG_DIRECTION].dims = dimensions; 

    varlist[AMBIG_OBJ].name  = "Ambiguity Obj";
    varlist[AMBIG_OBJ].type  = NC_FLOAT;
    varlist[AMBIG_OBJ].ndims = 3;
    varlist[AMBIG_OBJ].dims = dimensions; 

    varlist[RAIN_PROBABILITY].name  = "Rain Probability";
    varlist[RAIN_PROBABILITY].type  = NC_FLOAT;
    varlist[RAIN_PROBABILITY].ndims = 2;
    varlist[RAIN_PROBABILITY].dims = dimensions; 

    varlist[LAND_ICE_FLAG].name  = "Land Ice Flag";
    varlist[LAND_ICE_FLAG].type  = NC_BYTE;
    varlist[LAND_ICE_FLAG].ndims = 2;
    varlist[LAND_ICE_FLAG].dims = dimensions; 

    varlist[QUALITY_FLAG].name  = "Quality Flag";
    varlist[QUALITY_FLAG].type  = NC_UINT;
    varlist[QUALITY_FLAG].ndims = 2;
    varlist[QUALITY_FLAG].dims = dimensions; 

    varlist[RAIN_FLAG].name  = "Rain Flag";
    varlist[RAIN_FLAG].type  = NC_BYTE;
    varlist[RAIN_FLAG].ndims = 2;
    varlist[RAIN_FLAG].dims = dimensions; 

    varlist[RAIN_IMPACT].name  = "Rain Impact";
    varlist[RAIN_IMPACT].type  = NC_FLOAT;
    varlist[RAIN_IMPACT].ndims = 2;
    varlist[RAIN_IMPACT].dims = dimensions; 

    varlist[NUM_IN_FORE].name  = "Number In Fore";
    varlist[NUM_IN_FORE].type  = NC_BYTE;
    varlist[NUM_IN_FORE].ndims = 2;
    varlist[NUM_IN_FORE].dims = dimensions; 

    varlist[NUM_IN_AFT].name  = "Number In Aft";
    varlist[NUM_IN_AFT].type  = NC_BYTE;
    varlist[NUM_IN_AFT].ndims = 2;
    varlist[NUM_IN_AFT].dims = dimensions; 

    varlist[NUM_OUT_FORE].name  = "Number Out Fore";
    varlist[NUM_OUT_FORE].type  = NC_BYTE;
    varlist[NUM_OUT_FORE].ndims = 2;
    varlist[NUM_OUT_FORE].dims = dimensions; 

    varlist[NUM_OUT_AFT].name  = "Number Out Aft";
    varlist[NUM_OUT_AFT].type  = NC_BYTE;
    varlist[NUM_OUT_AFT].ndims = 2;
    varlist[NUM_OUT_AFT].dims = dimensions; 


    for (int i = 0; i < num_variables; i++) {
        NCERR(nc_def_var(ncid, varlist[i].name, varlist[i].type,
                   varlist[i].ndims, varlist[i].dims, &varlist[i].id));
    }

    // Write the header attributes
    NCERR(nc_put_att_float(ncid, NC_GLOBAL, "Cross Track Resolution", 
                NC_FLOAT, (size_t)(1), &l2b.header.crossTrackResolution));
    NCERR(nc_put_att_float(ncid, NC_GLOBAL, "Along Track Resolution", 
                NC_FLOAT, (size_t)(1), &l2b.header.alongTrackResolution));
    NCERR(nc_put_att_int  (ncid, NC_GLOBAL, "Zero Index", 
                NC_INT,   (size_t)(1), &l2b.header.zeroIndex));
    NCERR(nc_put_att_float(ncid, NC_GLOBAL, "Inclination", 
                NC_FLOAT, (size_t)(1), &l2b.header.inclination));
    NCERR(nc_put_att_int  (ncid, NC_GLOBAL, "Version ID Major", 
                NC_INT,   (size_t)(1), &l2b.header.version_id_major));
    NCERR(nc_put_att_int  (ncid, NC_GLOBAL, "Version ID Minor", 
                NC_INT,   (size_t)(1), &l2b.header.version_id_minor));

    tmp = l2b.frame.swath.GetCrossTrackBins();
    NCERR(nc_put_att_int  (ncid, NC_GLOBAL, "Cross Track Bins", 
                NC_INT,   (size_t)(1), &tmp));
    tmp = l2b.frame.swath.GetAlongTrackBins();
    NCERR(nc_put_att_int  (ncid, NC_GLOBAL, "Along Track Bins", 
                NC_INT,   (size_t)(1), &tmp));

    NCERR(nc_enddef(ncid));


    /* Same sneakiness with idx as described above with dimensions */
    size_t idx[3];
    WVC *wvc;
    unsigned char valid;

    for (idx[0] = 0; (int)idx[0] < l2b.frame.swath.GetCrossTrackBins(); idx[0]++) {
        for (idx[1] = 0; (int)idx[1] < l2b.frame.swath.GetAlongTrackBins(); idx[1]++) {
            valid = 0;

            wvc = l2b.frame.swath.swath[idx[0]][idx[1]];

            if (wvc != NULL) {
                valid = 1;

                NCERR(nc_put_var1_float(ncid, varlist[LATITUDE].id, idx, 
                            &wvc->lonLat.latitude));
                NCERR(nc_put_var1_float(ncid, varlist[LONGITUDE].id, idx, 
                            &wvc->lonLat.longitude));
                NCERR(nc_put_var1_float(ncid, varlist[NUDGE_SPEED].id, idx, 
                            &wvc->nudgeWV->spd));
                NCERR(nc_put_var1_float(ncid, varlist[NUDGE_DIRECTION].id, idx, 
                            &wvc->nudgeWV->dir));
                NCERR(nc_put_var1_float(ncid, varlist[NUDGE_OBJ].id, idx, 
                            &wvc->nudgeWV->obj));
                NCERR(nc_put_var1_float(ncid, varlist[SEL_SPEED].id, idx, 
                            &wvc->selected->spd));
                NCERR(nc_put_var1_float(ncid, varlist[SEL_DIRECTION].id, idx, 
                            &wvc->selected->dir));
                NCERR(nc_put_var1_float(ncid, varlist[SEL_OBJ].id, idx, 
                            &wvc->selected->obj));

//                NCERR(nc_put_var1_float(ncid, varlist[ANGLE_INTERVAL].id, idx, 
//                            &wvc->directionRanges));

    
                NCERR(nc_put_var1_float(ncid, varlist[RAIN_PROBABILITY].id, idx, 
                            &wvc->rainProb));
                NCERR(nc_put_var1_ubyte(ncid, varlist[LAND_ICE_FLAG].id, idx, 
                            (const unsigned char *)&wvc->landiceFlagBits));
                NCERR(nc_put_var1_uint (ncid, varlist[QUALITY_FLAG].id, idx, 
                            &wvc->qualFlag));
                NCERR(nc_put_var1_ubyte(ncid, varlist[RAIN_FLAG].id, idx, 
                            (const unsigned char *)&wvc->rainFlagBits));
                NCERR(nc_put_var1_float(ncid, varlist[RAIN_IMPACT].id, idx, 
                            &wvc->rainImpact));
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
    
                    NCERR(nc_put_var1(ncid, varlist[AMBIG_SPEED].id, idx, &wv->spd));
                    NCERR(nc_put_var1(ncid, varlist[AMBIG_DIRECTION].id, idx, &wv->spd));
                    NCERR(nc_put_var1(ncid, varlist[AMBIG_OBJ].id, idx, &wv->spd));
                }
            }
            
            NCERR(nc_put_var1_ubyte(ncid, varlist[POINT_VALID].id, idx, 
                    &valid));
        }
    }

    NCERR(nc_close(ncid));

    //----------------------//
    // close files and exit //
    //----------------------//

    l2b.Close();
    return(0);
}
