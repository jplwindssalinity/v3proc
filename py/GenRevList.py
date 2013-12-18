#!/usr/bin/env python2.7
#==============================================================#
# Copyright (C) 2013, California Institute of Technology.      #
# U.S. Government sponsorship acknowledged.                    #
#==============================================================#
#----------------------------------------------------------------------
# NAME
#    GenRevList.py
#
# SYNOPSIS
#    GenRevList.py -i minztimes.txt -o revlist.csv -c rdf_file
#
# DESCRIPTION
#    Reads in all minimum z times, writes out a revlist containing:
#    5 digit rev number, sim start time, sim end time.
# 
#    Note sim time-tags are the seconds since Jan 1st 1970 0 hours UTC
# 
#    An example RDF file contains these three keywords:
# 
#    GSE_SOURCE_DIR       = /u/rapidscat0/GSE
#    GSE_MERGED_DIR       = /u/pawpaw-z0/fore/rapidscat0/gse_preprocess/GSE_merged
#    EPHEM_MERGED_DIR     = /u/pawpaw-z0/fore/rapidscat0/gse_preprocess/ephem_merged
#    MINZ_TIMES_DIR       = /u/pawpaw-z0/fore/rapidscat0/gse_preprocess/minz_times
# 
#    REVLIST              = /u/pawpaw-z0/fore/rapidscat0/gse_preprocess/revlist.csv
#    GAP_REPORT           = /u/pawpaw-z0/fore/rapidscat0/gse_preprocess/gapreport.csv
# 
#    PERIOD_GUESS   (min) = 92.8
#    TIME_TAG_REF   (s)   = 1375289435.359200
#    REV_NO_REF           = 1
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

rcs_id = "$Id$"

from optparse import OptionParser
import rdf
import datetime
import numpy
import pdb
import sys
import os
import subprocess

# Functions
def date_time_from_sim( sim_tt ):
  return(datetime.datetime(1970,1,1)+datetime.timedelta(0,sim_tt))

def ToCodeB( dt ):
  millisecs = round(dt.microsecond/1000.0)
  dt_out = dt + datetime.timedelta(0,0,millisecs*1000-dt.microsecond)
  string = dt_out.strftime('%Y-%jT%H:%M:%S') + '.%3.3d' % millisecs
  return(string)

def GenRevList(config_file):
  if not config_file or not os.path.isfile(config_file):
    print>>sys.stderr, 'Config file, %s, does not exist' % config_file
    return 0
  
  try:
    rdf_data = rdf.parse(config_file)
    rev_no_ref   = rdf_data["REV_NO_REF"]
    tt_ref       = rdf_data["TIME_TAG_REF"]
    period_guess = rdf_data["PERIOD_GUESS"]
    revlist      = rdf_data["REVLIST"]
    minz_dir     = rdf_data["MINZ_TIMES_DIR"]
  except KeyError:
    print>>sys.stderr, 'Required keywords not found in rdf file: %s\n' % config_file
    return 0
  
  if not os.path.isdir(minz_dir):
    print>>sys.stderr, '%s is not a directory' % minz_dir
    return 0
  
  # Make temp file for all minz times
  tmpfile = subprocess.check_output('mktemp').rstrip('\n')
  
  # Cat all minz times together, sort em, remove duplicates, and write em to tempfile
  subprocess.call('cat %s/minz_times_* | sort | uniq > %s' % ( minz_dir, tmpfile ),shell=True)
  
  # Load rev min z times
  minz_tt = numpy.loadtxt( tmpfile, delimiter=' ', usecols=(0,) )
  
  # Delete tempfile
  subprocess.call('rm -f %s' % tmpfile,shell=True)
  
  ofp = open(revlist,'w')
  for irev in range(minz_tt.size):
    
    this_rev_no   = rev_no_ref + round((minz_tt[irev]-tt_ref)/period_guess)
    this_tt_start = minz_tt[irev]
    
    if irev < minz_tt.size-1 and minz_tt[irev+1]-this_tt_start < period_guess*1.1:
      this_tt_end  = minz_tt[irev+1]
    else:
      this_tt_end  = this_tt_start + period_guess
    
    rev_no_ref = this_rev_no
    tt_ref     = this_tt_start
    
    ofp.write( '%5.5d,%f,%f,%s,%s\n' % ( this_rev_no, this_tt_start, this_tt_end, 
                                         ToCodeB(date_time_from_sim(this_tt_start)),
                                         ToCodeB(date_time_from_sim(this_tt_end))))
  ofp.close()
  return 1


if __name__=='__main__':
  # Parse command line
  parser = OptionParser()
  parser.add_option( "-c", "--rdffile", action="store", type="string", dest="rdffile")
  (options, args) = parser.parse_args()
  
  if not options.rdffile or not os.path.isfile(options.rdffile):
    print>>sys.stderr, 'Usage: GenRevList.py -c config.rdf'
    sys.exit(1)
  
  if GenRevList( options.rdffile )==0:
    print>>sys.stderr, 'Error in GenRevList'
    sys.exit(1)
  sys.exit(0)