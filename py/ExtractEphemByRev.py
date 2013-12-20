#!/usr/bin/env python2.7
#==============================================================#
# Copyright (C) 2013, California Institute of Technology.      #
# U.S. Government sponsorship acknowledged.                    #
#==============================================================#
#----------------------------------------------------------------------
# NAME
#    ExtractEphemByRev.py
#
# SYNOPSIS
#    ExtractEphemByRev.py -c config_file
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

  gse_files = subprocess.check_output('find %s -name "RS_GSE_*" | sort ' % gse_dir,\
                                      shell=True).split('\n')
  
  for gse_file in gse_files:
    if not os.path.isfile(gse_file):
      continue
    
    ephem_file = ephem_dir + '/RS_EPHEM_' + os.path.basename(gse_file).strip('RS_GSE_')
    quats_file = ephem_dir + '/RS_QUATS_' + os.path.basename(gse_file).strip('RS_GSE_')
    
    if not ( os.path.isfile(ephem_file) and os.path.isfile(quats_file) ):
      ierr = subprocess.call('RS_GSE_to_ephem_quat -i %s -e %s -q %s' % \
                            ( gse_file, ephem_file, quats_file ), shell=True )
      
      if not ierr==0:
        print>>sys.stderr, 'Error making ephem/quats files for %s' % gse_file
  
  gse_files = subprocess.check_output('find %s -name "RS_GSE_*" | sort ' % gse_dir_gaps,\
                                      shell=True).split('\n')
  
  for gse_file in gse_files:
    if not os.path.isfile(gse_file):
      continue
    
    ephem_file = ephem_dir_gaps + '/RS_EPHEM_' + os.path.basename(gse_file).strip('RS_GSE_')
    quats_file = ephem_dir_gaps + '/RS_QUATS_' + os.path.basename(gse_file).strip('RS_GSE_')
    
    if not ( os.path.isfile(ephem_file) and os.path.isfile(quats_file) ):
      ierr = subprocess.call('RS_GSE_to_ephem_quat -i %s -e %s -q %s' % \
                            ( gse_file, ephem_file, quats_file ), shell=True )
      
      if not ierr==0:
        print>>sys.stderr, 'Error making gap ephem/quats files for %s' % gse_file
  
  
  return 1

if __name__=='__main__':
  # Parse command line
  parser = OptionParser()
  parser.add_option( "-c", "--rdffile", action="store", type="string", dest="rdffile")
  (options, args) = parser.parse_args()
  
  if not options.rdffile or not os.path.isfile(options.rdffile):
    print>>sys.stderr, 'Usage: ExtractEphemByRev.py -c config.rdf'
    sys.exit(1)
  
  if ExtractEphemByRev( options.rdffile )==0:
    print>>sys.stderr, 'Error in ExtractEphemByRev'
    sys.exit(1)
  sys.exit(0)