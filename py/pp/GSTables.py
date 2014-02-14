#==============================================================#
# Copyright (C) 2013-2014, California Institute of Technology. #
# U.S. Government sponsorship acknowledged.                    #
#==============================================================#
import os
import rdf
import Tracker
import Factor

def validate_file(filename):
    # Supposed to check that the file actually exists before returning it.
    # Not really sure if this is what I want to do.
    if os.path.isfile(filename):
        return filename
    else:
        return

def GetS(config_file, table_ids, orbsteps, this_rev):
    """
    Returns of filename of the Sfactor table to use for processing the 
    data.
    
    table_ids: list of table ids found in SCI.dat files
    orbsteps: fractional orbit position to start using each of the tables.
    
    Will merge multiple tables together if table_ids and orbsteps are not
    scalars.
    """
    try:
        rdf_data = rdf.parse(config_file)
        gs_sfact_dir = rdf_data['GS_SFACTOR_DIR']
        gs_sfact_merged_dir = rdf_data['GS_SFACTOR_MERGED_DIR']
    except KeyError:
        print>>sys.stderr, ('Required keywords not found in rdf file: %s\n'%
                            config_file)
        return
    try:
        iter(table_ids)
        outfile=os.path.join(gs_sfact_merged_dir, 'RS_SFACT_%5.5d'%this_rev)
        Factor.S.Merge([
            Factor.S(os.path.join(gs_sfact_dir, 'RS_SFACT_%5.5d'%id)) for
            id in table_ids], orbsteps).Write(outfile)
    except TypeError:
        # Don't need to merge, return path to sfactor file to use
        outfile = os.path.join(gs_sfact_dir, 'RS_SFACT_%5.5d'%table_ids)
    return validate_file(outfile)

def GetX(config_file, table_ids, orbsteps, this_rev):
    """
    Returns of filename of the Xfactor table to use for processing the 
    data.
    
    table_ids: list of table ids found in SCI.dat files
    orbsteps: fractional orbit position to start using each of the tables.
    
    Will merge multiple tables together if table_ids and orbsteps are not
    scalars.
    """
    try:
        rdf_data = rdf.parse(config_file)
        gs_xfact_dir = rdf_data['GS_XFACTOR_DIR']
        gs_xfact_merged_dir = rdf_data['GS_XFACTOR_MERGED_DIR']
    except KeyError:
        print>>sys.stderr, ('Required keywords not found in rdf file: %s\n'%
                            config_file)
        return
    
    try:
        iter(table_ids)
        outfile=os.path.join(gs_xfact_merged_dir, 'RS_XFACT_%5.5d'%this_rev)
        Factor.X.Merge([
            Factor.X(os.path.join(gs_xfact_dir, 'RS_XFACT_%5.5d'%id)) for
            id in table_ids], orbsteps).Write(outfile)
    except TypeError:
        # Don't need to merge, return path to xfactor file to use
        outfile = os.path.join(gs_xfact_dir, 'RS_XFACT_%5.5d'%table_ids)
    return validate_file(outfile)

def GetDopl(config_file, table_ids, orbsteps, this_rev):
    """
    Returns of filename of the DOPL table to use for processing the 
    data.
    
    table_ids: list of table ids found in SCI.dat files
    orbsteps: fractional orbit position to start using each of the tables.
    
    Will merge multiple tables together if table_ids and orbsteps are not
    scalars.
    """
    try:
        rdf_data = rdf.parse(config_file)
        range_table_dir = rdf_data['RANGE_TABLE_DIR']
        doppler_table_dir = rdf_data['DOPPLER_TABLE_DIR']
        xfactor_table_dir = rdf_data['XFACTOR_TABLE_DIR']
        gs_rng_dopp_dir = rdf_data['GS_RNG_DOPP_TABLE_DIR']
        gs_xfact_dir = rdf_data['GS_XFACTOR_DIR']
    except KeyError:
        print>>sys.stderr, ('Required keywords not found in rdf file: %s\n'%
                            config_file)
        return
    # Try to make this function work with lists or scalers (when we don't
    # need to merge tables.  Is this pythonic??
    try:
        iter(table_ids)
        iter(orbsteps)
    except TypeError:
        table_ids = [table_ids]
        orbsteps = [orbsteps]
    
    doppler_trackers = [Tracker.Merge([
        Tracker.Doppler(os.path.join(doppler_table_dir,
        'DTC_%5.5d.%d'%(id, ib+1))) for id in table_ids], orbsteps) 
        for ib in range(2)]
    
    range_trackers = [Tracker.Merge([
        Tracker.Range(os.path.join(range_table_dir,
        'RGC_%5.5d.%d'%(id, ib+1))) for id in table_ids], orbsteps) 
        for ib in range(2)]
    
    outfile = os.path.join(gs_rng_dopp_dir, 'RS_DOP_%5.5d'%this_rev)
    
    # Write out the combined range / doppler table.
    ofp = open(outfile,'w')
    for idx, tracker in enumerate(doppler_trackers+range_trackers):
        ibeam = idx%2
        tracker.CreateDIBData(ibeam, this_rev)
        tracker.WriteGSASCII(ofp, os.path.basename(outfile))
    ofp.close()
    
    return validate_file(outfile)
