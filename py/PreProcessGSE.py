#!/usr/bin/env python2.7
#==============================================================#
# Copyright (C) 2013, California Institute of Technology.      #
# U.S. Government sponsorship acknowledged.                    #
#==============================================================#
#----------------------------------------------------------------------
# NAME
#    PreProcessGSE.py
#
# SYNOPSIS
#    PreProcessGSE.py -c config_file
#
# DESCRIPTION
#    Runs RS_GSE_merge on all GSE files, extracts the ephem for each,
#    then coputes all minimum z position times.
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
#       1  File probably isn't a sim ephem file
#
# NOTES
#    None.
#
# AUTHOR
#    Alex Fore
#    alexander.fore@jpl.nasa.gov
#----------------------------------------------------------------------

rcs_id = "$Id$"

QSCATSIM_PY_DIR='/home/fore/qscatsim/QScatSim/py'

import sys
if not QSCATSIM_PY_DIR in sys.path:
  sys.path.append(QSCATSIM_PY_DIR)

from optparse import OptionParser
import os
import rdf
import pdb
import subprocess
import SimEphem

def PreProcessGSE( config_file ):
  if not config_file or not os.path.isfile(config_file):
    print>>sys.stderr, '%s not right' % config_file
    return 0
  try:
    rdf_data      = rdf.parse(config_file)
    gse_in_dir    = rdf_data["GSE_SOURCE_DIR"]
    gse_out_dir   = rdf_data["GSE_MERGED_DIR"]
    ephem_dir     = rdf_data["EPHEM_MERGED_DIR"]
    minz_dir      = rdf_data["MINZ_TIMES_DIR"]
    node_long_dir = rdf_data["ASC_NODE_LONGS_DIR"]
  except KeyError:
    print>>sys.stderr, 'Required keywords not found in rdf file: %s\n' % options.rdffile
    return(0)
  
  # Find all GSE files
  cmd    = 'find %s -name "RS_GSE*" -a ! -name "*.rpsm" | sort' % gse_in_dir
  output = subprocess.check_output(cmd,shell=True)
  files  = output.split('\n')
  
  for file in files:
    if not os.path.isfile(file):
      continue
    
    # Check to see if merged GSE file already exists
    merged_file = gse_out_dir + '/' + os.path.basename(file)
    if not os.path.isfile(merged_file):
      # Run RS_GSE_merge on this file
      tmpfile = subprocess.check_output('mktemp').rstrip('\n')
      f = open(tmpfile,'w')
      f.write('%s\n' % file)
      f.close()
      
      ierr = subprocess.call('RS_GSE_merge -i %s -o %s' % (tmpfile,merged_file),shell=True)
      subprocess.call('rm -f %s' % tmpfile,shell=True)
      
      if not ierr==0:
        print>>sys.stderr, 'Error merging' % file
  
  # Make all ephem files
  files = subprocess.check_output('find %s -name "*RS_GSE*"' % \
                                  gse_out_dir,shell=True).split('\n')
  for file in files:
    if not os.path.isfile(file):
      continue
    revtag = os.path.basename(file).strip('RS_GSE_')
    
    ephem_file = ephem_dir + '/RS_EPHEM_' + revtag
    quats_file = ephem_dir + '/RS_QUATS_' + revtag
    
    if not ( os.path.isfile(ephem_file) and os.path.isfile(quats_file) ):
      ierr = subprocess.call('RS_GSE_to_ephem_quat -i %s -e %s -q %s' % \
                            ( file, ephem_file, quats_file ), shell=True )
      if not ierr==0:
        print>>sys.stderr, 'Error making ephem/quats files' % file
  
  
  # Make all minz times and asc node longitudes
  files = subprocess.check_output('find %s -name "RS_EPHEM_*"' % \
                                  ephem_dir,shell=True).split('\n')
  for file in files:
    if not os.path.isfile(file):
      continue
    
    revtag = os.path.basename(file).strip('RS_EPHEM_')
    
    minz_file = minz_dir      + '/minz_times_'     + revtag
    long_file = node_long_dir + '/asc_node_longs_' + revtag
    
    if not (os.path.isfile(minz_file) and os.path.isfile(long_file)):
      # Using the SimEphem class methods
      ephem      = SimEphem.SimEphem(file)
      minz_times = ephem.GetMinZTimes()
      nodes      = ephem.GetAscendingNodes()
      
      ofp = open(minz_file,'w')
      for minz_time in minz_times:
        print>>ofp,'%f' % minz_time
      ofp.close()
      
      ofp = open(long_file,'w')
      
      for ii in range(len(nodes['times'])):
        print>>ofp,'%f,%f' % ( nodes['times'][ii], nodes['longs'][ii] )
      ofp.close()
  
  # Done preprocessing GSE data!
  return(1)

def main():
  # Parse command line
  parser = OptionParser()
  parser.add_option( "-c", "--rdffile", action="store", type="string", dest="rdffile")
  (options, args) = parser.parse_args()
  
  if not options.rdffile or not os.path.isfile(options.rdffile):
    print>>sys.stderr, 'Usage: PreProcessGSE.py -c config.rdf'
    sys.exit(1)
  
  if PreProcessGSE( options.rdffile )==0:
    print>>sys.stderr, 'Error in PreProcessGSE'
    sys.exit(1)
  sys.exit(0)

if __name__ == '__main__':
  main()

