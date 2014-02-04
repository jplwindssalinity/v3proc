#!/usr/bin/env python2.7
#==============================================================#
# Copyright (C) 2013-2014, California Institute of Technology. #
# U.S. Government sponsorship acknowledged.                    #
#==============================================================#
#----------------------------------------------------------------------
# NAME
#    GSEGapReport.py
#
# SYNOPSIS
#    GSEGapReport.py config_file
#
# DESCRIPTION
#    Reads in all GSE times and writes out a report of all gaps
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
__version__ = '$Id$'

QSCATSIM_PY_DIR='/home/fore/qscatsim/QScatSim/py'

import sys
if not QSCATSIM_PY_DIR in sys.path:
  sys.path.append(QSCATSIM_PY_DIR)

from optparse import OptionParser
import datetime
import os
import pdb
import numpy
import subprocess
import GSE
import rdf
import util.time
import util.file

def GSEGapReport( config_file ):
  if not config_file or not os.path.isfile(config_file):
    print>>sys.stderr, '%s not right' % config_file
    return 0
  
  try:
    rdf_data  = rdf.parse(config_file)
    gse_dir   = rdf_data["GSE_MERGED_DIR"]
    outfile   = rdf_data["GAP_REPORT"]
    gsetimes  = rdf_data["GSE_TIMES"]
    gapthresh = rdf_data["GAP_THRESHOLD"]
  except KeyError:
    print>>sys.stderr, 'Required keywords not found in rdf file: %s\n' % config_file
    return 0
  
  start_times = []
  end_times   = []
  files       = []
  
  gse_times_fp = open(gsetimes,'w')
  
  for file in util.file.find(gse_dir,"RS_GSE*"):
    if not os.path.isfile(file):
      continue
    
    gse = GSE.GSE(file)
    tt = gse.ReadPacketTimes()
    if tt==None:
      print>>sys.stderr, 'Error reading time-tags from %s' % file
      continue
    
    tt = numpy.unique(numpy.sort(tt))
    
    # Write out span of each GSE file to GSE_TIMES file
    print>>gse_times_fp, '%s,%f,%f' % ( file, util.time.gps_to_sim(tt[0]), \
                                        util.time.gps_to_sim(tt[tt.size-1]) )
    
    diff_tt  = tt[1:]-tt[0:tt.size-1]
    idx      = numpy.argwhere( diff_tt>2 )
    n_gaps   = idx.shape[0]
    tt_start = tt[0]
    
    for i_gap in range(n_gaps):
      this_idx = idx[i_gap,0]
      start_times.append(tt_start)
      end_times.append(tt[this_idx])
      files.append(file)
      tt_start = tt[1+this_idx]
    
    start_times.append(tt_start)
    end_times.append(tt[tt.size-1])
    files.append(file)
  
  gse_times_fp.close()
  
  files       = numpy.array(files)
  start_times = numpy.array(start_times)
  end_times   = numpy.array(end_times)
  
  interval_length = end_times - start_times
  
  idx = numpy.argsort( interval_length )[::-1]
  
  unique_start_times = []
  unique_end_times   = []
  unique_files       = []
  
  for ii in range(idx.size):
    this_idx = idx[ii]
    add_it = True
    
    for jj in range(len(unique_start_times)):
      if start_times[this_idx] >= unique_start_times[jj] and \
         end_times[this_idx] <= unique_end_times[jj]:
        add_it = False
    
    if add_it:
      unique_start_times.append(start_times[this_idx])
      unique_end_times.append(end_times[this_idx])
      unique_files.append(files[this_idx])
  
  unique_files       = numpy.array(unique_files)
  unique_start_times = numpy.array(unique_start_times)
  unique_end_times   = numpy.array(unique_end_times)
  
  idx = numpy.argsort( unique_start_times )
  
  unique_files       = unique_files[idx]
  unique_start_times = unique_start_times[idx]
  unique_end_times   = unique_end_times[idx]
  
  ofp = open(outfile,'w')
  for ii in range(1,unique_start_times.size):
    delta_tt = unique_start_times[ii] - unique_end_times[ii-1]
    
    if delta_tt > gapthresh:
      ofp.write('%f,%f,%f,%s,%s,%s,%s\n' % ( delta_tt, \
                util.time.gps_to_sim(unique_end_times[ii-1]),\
                util.time.gps_to_sim(unique_start_times[ii]),\
                util.time.ToCodeB(util.time.datetime_from_gps(unique_end_times[ii-1])), \
                util.time.ToCodeB(util.time.datetime_from_gps(unique_start_times[ii])), \
                os.path.basename(unique_files[ii-1]),\
                os.path.basename(unique_files[ii]) ) )
  ofp.close()
    
if __name__=='__main__':
  usage_string = 'Usage: %s <config_file>' % sys.argv[0]
  config_file = sys.argv[1]
  
  if not os.path.isfile(config_file):
    print>>sys.stderr, usage_string
    sys.exit(1)
  
  if GSEGapReport(config_file)==0:
    print>>sys.stderr, 'Error in GSEGapReport'
    sys.exit(1)
  sys.exit(0)

