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
#    ConvertToGS.py -c config_file
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
rcs_id      = '$Id$'
__version__ = '$Revision$'

QSCATSIM_PY_DIR='/home/fore/qscatsim/QScatSim/py'

import sys
if not QSCATSIM_PY_DIR in sys.path:
  sys.path.append(QSCATSIM_PY_DIR)

from optparse import OptionParser
import os
import pdb
import rdf
import numpy
import subprocess
import time_funcs
from pm.utils.helper import find_files

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
    revlist           = rdf_data["REVLIST"]
  except KeyError:
    print>>sys.stderr, 'Required keywords not found in rdf file: %s\n' % \
           options.rdffile
    return(0)
  
  rev_no      = numpy.loadtxt( revlist, delimiter=',', usecols=(0,) )
  rev_t_start = numpy.loadtxt( revlist, delimiter=',', usecols=(1,) )
  
  # Convert the Gap-free ephemeris/quaternion files
  for ephem_file in find_files(ephem_dir,"RS_EPHEM_*"):
    revtag         = os.path.basename(ephem_file).strip('RS_EPHEM_')
    quats_file     = ephem_file.replace('RS_EPHEM','RS_QUATS')
    out_ephem_file = os.path.join(ephem_gs_dir, 'RS_SEPHG_' + revtag)
    out_att_file   = os.path.join(att_gs_dir,   'RS_SATTG_' + revtag)
    
    if os.path.isfile(ephem_file) and os.path.isfile(quats_file) and \
       not ( os.path.isfile(out_ephem_file) and os.path.isfile(out_att_file)):
      
      idx = rev_no==int(revtag)
      leap_secs = time_funcs.leap_seconds( 
        time_funcs.date_time_from_sim( rev_t_start[idx][0] ) )
      
      subprocess.call('ephem_to_gs -o %s -l %d %s' % \
                     (out_ephem_file, leap_secs, ephem_file), shell=True )
      
      subprocess.call('quat_to_gs_att -o %s -l %d %s' % \
                     (out_att_file, leap_secs, quats_file), shell=True )
      
  # Convert the Gap-filled ephemeris/quaternion files
  for ephem_file in find_files(ephem_gaps_dir,"RS_EPHEM_*"):
    revtag         = os.path.basename(ephem_file).strip('RS_EPHEM_')
    quats_file     = ephem_file.replace('RS_EPHEM','RS_QUATS')
    out_ephem_file = os.path.join(ephem_gs_gaps_dir, 'RS_SEPHG_' + revtag)
    out_att_file   = os.path.join(att_gs_gaps_dir,   'RS_SATTG_' + revtag)
    
    if( os.path.isfile(ephem_file) and os.path.isfile(quats_file) and 
        not (os.path.isfile(out_ephem_file) and os.path.isfile(out_att_file))):
      
      idx = rev_no==int(revtag)
      leap_secs = time_funcs.leap_seconds( 
        time_funcs.date_time_from_sim( rev_t_start[idx][0] ) )
      
      subprocess.call('ephem_to_gs -o %s -l %d %s' % \
                     (out_ephem_file, leap_secs, ephem_file), shell=True )
      
      subprocess.call('quat_to_gs_att -o %s -l %d %s' % \
                     (out_att_file, leap_secs, quats_file), shell=True )
  
  return(1)

def ConvertRangeDopplerToGS(config_file):
  if not config_file or not os.path.isfile(config_file):
    print>>sys.stderr, '%s not right' % config_file
    return 0
  try:
    rdf_data = rdf.parse(config_file)
    rng_in_dir = rdf_data['RANGE_TABLE_DIR']
    dop_in_dir = rdf_data['DOPPLER_TABLE_DIR']
    rng_gs_dir = rdf_data['GS_RANGE_TABLE_DIR']
    dop_gs_dir = rdf_data['GS_DOPPLER_TABLE_DIR']
    revlist    = rdf_data["REVLIST"]
  except KeyError:
    print>>sys.stderr, 'Required keywords not found in rdf file: %s\n' % \
           options.rdffile
    return(0)
  
  rev_nos = numpy.loadtxt(revlist, delimiter=',', usecols=(0,))
  
  for rev_no in rev_nos:
    rng_in_table1 = '%s/RGC_%5.5d.1' % (rng_in_dir, rev_no)
    rng_in_table2 = '%s/RGC_%5.5d.2' % (rng_in_dir, rev_no)
    rng_gs_table  = '%s/RGC_%5.5d'   % (rng_gs_dir, rev_no)
    
    if os.path.isfile(rng_in_table1) and os.path.isfile(rng_in_table2) and \
       not os.path.isfile(rng_gs_table):
      subprocess.call('rgc_format -b 1 -f b -i %s -b 2 -f b -i %s -f g -o %s' % \
                     (rng_in_table1,rng_in_table2,rng_gs_table),shell=True)
    
    dop_in_table1 = '%s/DTC_%5.5d.1' % (dop_in_dir, rev_no)
    dop_in_table2 = '%s/DTC_%5.5d.2' % (dop_in_dir, rev_no)
    dop_gs_table  = '%s/DTC_%5.5d'   % (dop_gs_dir, rev_no)
    
    if os.path.isfile(dop_in_table1) and os.path.isfile(dop_in_table2) and \
       not os.path.isfile(dop_gs_table):
      subprocess.call('dtc_format -b 1 -f b -i %s -b 2 -f b -i %s -f g -o %s' % \
                     (dop_in_table1,dop_in_table2,dop_gs_table),shell=True)

def ConvertToGS( config_file ):
  if not config_file or not os.path.isfile(config_file):
    print>>sys.stderr, '%s not right' % config_file
    return 0
  ConvertEphemToGS(config_file)
  ConvertRangeDopplerToGS(config_file)
  return(1)

def main():
  # Parse command line
  parser = OptionParser()
  parser.add_option( "-c", "--rdffile", action="store", type="string", dest="rdffile")
  (options, args) = parser.parse_args()
  
  if not options.rdffile or not os.path.isfile(options.rdffile):
    print>>sys.stderr, 'Usage: ConvertToGS.py -c config.rdf'
    sys.exit(1)
  
  if ConvertToGS( options.rdffile )==0:
    print>>sys.stderr, 'Error in ConvertToGS'
    sys.exit(1)
  sys.exit(0)

if __name__=='__main__':
  main()

