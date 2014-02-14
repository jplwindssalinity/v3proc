#!/usr/bin/env python2.7
#==============================================================#
# Copyright (C) 2013-2014, California Institute of Technology. #
# U.S. Government sponsorship acknowledged.                    #
#==============================================================#
#----------------------------------------------------------------------
# NAME
#    EphemQuatFillGaps.py
#
# SYNOPSIS
#    EphemQuatFillGaps.py config_file
#
# DESCRIPTION
#    Fills gaps in ephem and quat files
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
import subprocess
import util.file

def EphemQuatFillGaps( config_file ):
  if not config_file or not os.path.isfile(config_file):
    print>>sys.stderr, 'Config file, %s, does not exist' % config_file
    return 0
  
  try:
    rdf_data       = rdf.parse(config_file)
    ephem_in_dir   = rdf_data["EPHEM_BYREV_GAPS_DIR"]
    ephem_out_dir  = rdf_data["EPHEM_BYREV_GAPFILLED"]
    revlist        = rdf_data["REVLIST"]
    revpad_start   = rdf_data["REV_PAD_START"]
    revpad_end     = rdf_data["REV_PAD_END"]

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
  
  for ephem_file in util.file.find(ephem_in_dir,"RS_EPHEM*"):
    quats_file = ephem_file.replace('RS_EPHEM','RS_QUATS')
    
    if not os.path.isfile(ephem_file) or not os.path.isfile(quats_file):
      continue
    
    this_revno = int(os.path.basename(ephem_file)[9:])
    
    out_ephem_file = os.path.join(ephem_out_dir,os.path.basename(ephem_file))
    out_quats_file = os.path.join(ephem_out_dir,os.path.basename(quats_file))
    
    if not ( os.path.isfile(out_ephem_file) and os.path.isfile(out_quats_file) ):
      
      idx = numpy.argwhere(rev_no==this_revno)
      
      this_pad_start = pad_t_start[idx[0,0]]
      this_pad_end   = pad_t_end[idx[0,0]]
      this_period    = rev_t_end[idx[0,0]] - rev_t_start[idx[0,0]]
      
      ierr = subprocess.call('ephem_quat_fill_gaps -ie %s -iq %s -oe %s -oq %s -p %f -tlims %f %f' \
                          % ( ephem_file, quats_file, out_ephem_file, out_quats_file, \
                              this_period, this_pad_start, this_pad_end ),shell=True)
      if not ierr==0:
        print>>sys.stderr, 'Error filling gaps in ephem/ quat files: %s' % ephem_file
  
  return 1

def main():
  usage_string = 'Usage: %s <config_file>' % sys.argv[0]
  config_file = sys.argv[1]
  
  if not os.path.isfile(config_file):
    print>>sys.stderr, usage_string
    sys.exit(1)

  if EphemQuatFillGaps(config_file) == 0:
    print>>sys.stderr, 'Error in EphemQuatFillGaps'
    sys.exit(1)
  sys.exit(0)
  
if __name__ == '__main__':
    main()
