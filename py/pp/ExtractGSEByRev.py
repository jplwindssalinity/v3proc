#!/usr/bin/env python2.7
#==============================================================#
# Copyright (C) 2013-2014, California Institute of Technology. #
# U.S. Government sponsorship acknowledged.                    #
#==============================================================#
#----------------------------------------------------------------------
# NAME
#    ExtractGSEByRev.py
#
# SYNOPSIS
#    ExtractGSEByRev.py config_file
#
# DESCRIPTION
#    Uses the revlist and gap report to figure out which revs are gap-free
#    and extracts single rev GSE files with a pad.
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

QSCATSIM_PY_DIR='/home/fore/qscatsim/QScatSim/py'

import sys
if not QSCATSIM_PY_DIR in sys.path:
  sys.path.append(QSCATSIM_PY_DIR)

from optparse import OptionParser
import os
import pdb
import rdf
import numpy
import GSE
import subprocess

def ExtractGSEByRev( config_file ):
  if not config_file or not os.path.isfile(config_file):
    print>>sys.stderr, 'Config file, %s, does not exist' % config_file
    return 0
  
  try:
    rdf_data         = rdf.parse(config_file)
    gse_in_dir       = rdf_data["GSE_MERGED_DIR"]
    gse_out_dir      = rdf_data["GSE_BYREV_DIR"]
    gse_out_dir_gaps = rdf_data["GSE_BYREV_GAPS_DIR"]
    revlist          = rdf_data["REVLIST"]
    gapreport        = rdf_data["GAP_REPORT"]
    revpad_start     = rdf_data["REV_PAD_START"]
    revpad_end       = rdf_data["REV_PAD_END"]
    revtimes         = rdf_data["GSE_TIMES"]
  except KeyError:
    print>>sys.stderr, 'Required keywords not found in rdf file: %s\n' % config_file
    return 0
  
  # load rev start/end times
  rev_no      = numpy.loadtxt( revlist, delimiter=',', usecols=(0,) )
  rev_t_start = numpy.loadtxt( revlist, delimiter=',', usecols=(1,) )
  rev_t_end   = numpy.loadtxt( revlist, delimiter=',', usecols=(2,) )
  
  # Compute file start and end times with pad
  pad_t_start = rev_t_start - revpad_start
  pad_t_end   = rev_t_end   + revpad_end
  
  # load gap start/end times
  gap_t_start = numpy.loadtxt( gapreport, delimiter=',', usecols=(1,) )
  gap_t_end   = numpy.loadtxt( gapreport, delimiter=',', usecols=(2,) )
  
  # load time span of each file
  files        = numpy.genfromtxt( revtimes, delimiter=',', usecols=(0,), dtype=None )
  file_t_start = numpy.loadtxt( revtimes, delimiter=',', usecols=(1,) )
  file_t_end   = numpy.loadtxt( revtimes, delimiter=',', usecols=(2,) )
  
  # For each rev, determine if any gaps
  for i_rev in range(rev_no.size):
    mask = numpy.logical_or( gap_t_start > pad_t_end[i_rev], gap_t_end < pad_t_start[i_rev] )
    
    # Put in gaps directory if any gaps (keep in mind gap at end)
    if numpy.all(mask) and pad_t_end[i_rev] < file_t_end.max():
      # if no gaps in this rev, write to gse_out_dir
      outgsefile = os.path.join(gse_out_dir, 'RS_GSE_%5.5d' % rev_no[i_rev])
    else:
      # if gaps in this rev, write to gse_out_dir_gaps
      outgsefile = os.path.join(gse_out_dir_gaps, 'RS_GSE_%5.5d' % rev_no[i_rev])
    
    # skip if outgsefile already exists
    if os.path.isfile(outgsefile):
      continue
      
    # determine which GSE files we need to get data from
    mask = numpy.logical_and( file_t_start <= pad_t_end[i_rev], file_t_end >= pad_t_start[i_rev] )
    
    tmpfile = subprocess.check_output('mktemp').rstrip('\n')
    list_fp = open(tmpfile,'w')
    for file in files[mask]:
      gse = GSE.GSE(file)
      print>>list_fp, file
    list_fp.close()
    
    ierr = subprocess.call('RS_GSE_merge -i %s -o %s -tlims %f %f' % \
                          ( tmpfile, outgsefile, pad_t_start[i_rev], \
                            pad_t_end[i_rev] ), shell=True)
    
    subprocess.call('rm -f %s' % tmpfile,shell=True)
    
    if not ierr==0:
      print>>sys.stderr, 'Error running RS_GSE_Merge for rev %d' % rev_no[i_rev]
  
  return 1

if __name__=='__main__':
  usage_string = 'Usage: %s <config_file>' % sys.argv[0]
  config_file = sys.argv[1]
  
  if not os.path.isfile(config_file):
    print>>sys.stderr, usage_string
    sys.exit(1)
  
  if ExtractGSEByRev(config_file)==0:
    print>>sys.stderr, 'Error in ExtractGSEByRev'
    sys.exit(1)
  sys.exit(0)