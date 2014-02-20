#!/usr/bin/env python2.7
#==============================================================#
# Copyright (C) 2013-2014, California Institute of Technology. #
# U.S. Government sponsorship acknowledged.                    #
#==============================================================#
#----------------------------------------------------------------------
# NAME
#    ConvertToGS.py
#
# SYNOPSIS
#    ConvertToGS.py config_file
#
# DESCRIPTION
#    Converts stuff to GS format
#
# OPTIONS
#
# OPERANDS
#
# EXAMPLES
#
# ENVIRONMENT
#    Not environment dependent.
#
# EXIT STATUS
#    The following exit values are returned:
#       0  Successful execution
#       1  Errors
#
# NOTES
#    None.
#
# AUTHOR
#    Alex Fore
#    alexander.fore@jpl.nasa.gov
#----------------------------------------------------------------------
import sys
import os
import rdf
import numpy
import subprocess
import util.time
import Tracker
import pm.database.revs
import util.file

def ConvertEphemToGS( config_file ):
    """ Converts the QScatSim Ephemeris and Quats files to the GS format"""

    try:
        rdf_data = rdf.parse(config_file)
        ephem_dir         = rdf_data["EPHEM_BYREV_DIR"]
        ephem_gaps_dir    = rdf_data["EPHEM_BYREV_GAPFILLED"]
        ephem_gs_dir      = rdf_data["GS_EPHEM_DIR"]
        ephem_gs_gaps_dir = rdf_data["GS_EPHEM_GAPFILLED"]
        att_gs_dir        = rdf_data["GS_ATT_DIR"]
        att_gs_gaps_dir   = rdf_data["GS_ATT_GAPFILLED"]    
        revlist_db        = rdf_data["REVLIST_DB"]
    except KeyError:
        print>>sys.stderr, ('Required keywords not found in rdf file: %s\n'%
                            config_file)
        return(0)

    rev_db = pm.database.revs.RevDataBase(revlist_db)

    # Convert the Gap-free ephemeris/quaternion files
    for ephem_file in util.file.find(ephem_dir,"RS_EPHEM_*"):
        revtag         = os.path.basename(ephem_file).strip('RS_EPHEM_')
        quats_file     = ephem_file.replace('RS_EPHEM','RS_QUATS')
        out_ephem_file = os.path.join(ephem_gs_dir, 'RS_SEPHG_' + revtag)
        out_att_file   = os.path.join(att_gs_dir,   'RS_SATTG_' + revtag)

        if os.path.isfile(ephem_file) and os.path.isfile(quats_file) and \
           not ( os.path.isfile(out_ephem_file) and os.path.isfile(out_att_file)):

            try:
                leap_secs = util.time.leap_seconds(rev_db[int(revtag)].start)
            except TypeError:
                print>>sys.stderr,'Rev: %s is not in database!'%revtag
                return(0)

            subprocess.call('ephem_to_gs -o %s -l %d %s' % \
                           (out_ephem_file, leap_secs, ephem_file), shell=True)

            subprocess.call('quat_to_gs_att -o %s -l %d %s' % \
                           (out_att_file, leap_secs, quats_file), shell=True)

    # Convert the Gap-filled ephemeris/quaternion files
    for ephem_file in util.file.find(ephem_gaps_dir,"RS_EPHEM_*"):
        revtag         = os.path.basename(ephem_file).strip('RS_EPHEM_')
        quats_file     = ephem_file.replace('RS_EPHEM','RS_QUATS')
        out_ephem_file = os.path.join(ephem_gs_gaps_dir, 'RS_SEPHG_' + revtag)
        out_att_file   = os.path.join(att_gs_gaps_dir,   'RS_SATTG_' + revtag)

        if( os.path.isfile(ephem_file) and os.path.isfile(quats_file) and 
            not (os.path.isfile(out_ephem_file) and os.path.isfile(out_att_file))):

            try:
                leap_secs = util.time.leap_seconds(rev_db[int(revtag)].start)
            except TypeError:
                print>>sys.stderr,'Rev: %s is not in database!'%revtag
                return(0)

            subprocess.call('ephem_to_gs -o %s -l %d %s' % \
                           (out_ephem_file, leap_secs, ephem_file), shell=True)
            subprocess.call('quat_to_gs_att -o %s -l %d %s' % \
                           (out_att_file, leap_secs, quats_file), shell=True)
    return(1)

def ConvertRangeDopplerToDIB(config_file):
    if not config_file or not os.path.isfile(config_file):
        print>>sys.stderr, '%s not right' % config_file
        return 0
    try:
        rdf_data = rdf.parse(config_file)
        rng_in_dir  = rdf_data['RANGE_TABLE_DIR']
        dop_in_dir  = rdf_data['DOPPLER_TABLE_DIR']
        rng_dib_dir = rdf_data['DIB_RANGE_TABLE_DIR']
        dop_dib_dir = rdf_data['DIB_DOPPLER_TABLE_DIR']
        revlist_db  = rdf_data["REVLIST_DB"]
    except KeyError:
        print>>sys.stderr, 'Required keywords not found in rdf file: %s\n' % \
               config_file
        return(0)

    rev_db = pm.database.revs.RevDataBase(revlist_db)
    for rev in rev_db:
        rev_no = rev.rev

        if rev_no < 0:
            continue

        for ib in range(2):
            # Convert range tables
            rng_in_table  = os.path.join(rng_in_dir,"RGC_%5.5d.%d"%(rev_no,ib+1))
            rng_dib_table = os.path.join(rng_dib_dir,"RGC_%5.5d.%d"%(rev_no,ib+1))

            if os.path.isfile(rng_in_table) and not os.path.isfile(rng_dib_table):
                rng_track = Tracker.Range()
                rng_track.ReadSimBinary(rng_in_table)
                rng_track.CreateDIBData(ib,rev_no)
                rng_track.WriteDIBBinary(rng_dib_table)

            # Convert doppler tables
            dop_in_table  = os.path.join(dop_in_dir,"DTC_%5.5d.%d"%(rev_no,ib+1))
            dop_dib_table = os.path.join(dop_dib_dir,"DTC_%5.5d.%d"%(rev_no,ib+1))

            if os.path.isfile(dop_in_table) and not os.path.isfile(dop_dib_table):
                dop_track = Tracker.Doppler()
                dop_track.ReadSimBinary(dop_in_table)
                dop_track.CreateDIBData(ib,rev_no)
                dop_track.WriteDIBBinary(dop_dib_table)

def ConvertToGS( config_file ):
    if not config_file or not os.path.isfile(config_file):
        print>>sys.stderr, '%s not right' % config_file
        return 0
    ConvertEphemToGS(config_file)
    ConvertRangeDopplerToDIB(config_file)
    return(1)

def main():
    usage_string = 'Usage: %s <config_file>' % sys.argv[0]
    config_file = sys.argv[1]

    if not os.path.isfile(config_file):
        print>>sys.stderr, usage_string
        sys.exit(1)

    if ConvertToGS(config_file) == 0:
        print>>sys.stderr, 'Error in ConvertToGS'
        sys.exit(1)
    sys.exit(0)

if __name__ == '__main__':
    main()

