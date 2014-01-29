#!/usr/bin/env python2.7
#==============================================================#
# Copyright (C) 2013-2014, California Institute of Technology. #
# U.S. Government sponsorship acknowledged.                    #
#==============================================================#
#----------------------------------------------------------------------
# NAME
#    ExtractEphemByRev.py
#
# SYNOPSIS
#    ExtractEphemByRev.py config_file
#
# DESCRIPTION
#    Extracts the ephem and quaternion files from all rev GSE files.
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
from pm.utils.helper import find_files

def ExtractEphemByRev( config_file ):
  if not config_file or not os.path.isfile(config_file):
    print>>sys.stderr, 'Config file, %s, does not exist' % config_file
    return 0
  
  try:
    rdf_data       = rdf.parse(config_file)
    gse_dir        = rdf_data["GSE_BYREV_DIR"]
    gse_dir_gaps   = rdf_data["GSE_BYREV_GAPS_DIR"]
    ephem_dir      = rdf_data["EPHEM_BYREV_DIR"]
    ephem_dir_gaps = rdf_data["EPHEM_BYREV_GAPS_DIR"]
    
  except KeyError:
    print>>sys.stderr, 'Required keywords not found in rdf file: %s\n' % config_file
    return 0

  for gse_file in find_files(gse_dir,"RS_GSE_*"):
    if not os.path.isfile(gse_file):
      continue
    
    revtag = os.path.basename(gse_file).strip('RS_GSE_')
    
    ephem_file = os.path.join(ephem_dir, 'RS_EPHEM_' + revtag)
    quats_file = os.path.join(ephem_dir, 'RS_QUATS_' + revtag)
    
    if not ( os.path.isfile(ephem_file) and os.path.isfile(quats_file) ):
      ierr = subprocess.call('RS_GSE_to_ephem_quat -i %s -e %s -q %s' % \
                            ( gse_file, ephem_file, quats_file ), shell=True )
      
      if not ierr==0:
        print>>sys.stderr, 'Error making ephem/quats files for %s' % gse_file
  
  for gse_file in find_files(gse_dir_gaps,"RS_GSE_*"):
    if not os.path.isfile(gse_file):
      continue
    
    revtag = os.path.basename(gse_file).strip('RS_GSE_')
    
    ephem_file = os.path.join(ephem_dir_gaps, 'RS_EPHEM_' + revtag)
    quats_file = os.path.join(ephem_dir_gaps, 'RS_QUATS_' + revtag)
    
    if not ( os.path.isfile(ephem_file) and os.path.isfile(quats_file) ):
      ierr = subprocess.call('RS_GSE_to_ephem_quat -i %s -e %s -q %s' % \
                            ( gse_file, ephem_file, quats_file ), shell=True )
      
      if not ierr==0:
        print>>sys.stderr, 'Error making gap ephem/quats files for %s' % gse_file
  
  
  return 1

if __name__=='__main__':
  usage_string = 'Usage: %s <config_file>' % sys.argv[0]
  config_file = sys.argv[1]
  
  if not os.path.isfile(config_file):
    print>>sys.stderr, usage_string
    sys.exit(1)
  
  if ExtractEphemByRev(config_file)==0:
    print>>sys.stderr, 'Error in ExtractEphemByRev'
    sys.exit(1)
  sys.exit(0)