#!/usr/bin/env python2.7
#==============================================================#
# Copyright (C) 2013-2014, California Institute of Technology. #
# U.S. Government sponsorship acknowledged.                    #
#==============================================================#
#----------------------------------------------------------------------
# NAME
#    MakeRangeDopplerTables.py
#
# SYNOPSIS
#    MakeRangeDopplerTables.py -c config_file
#
# DESCRIPTION
#    Make the range and doppler tables for a given rev
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
import Tracking
import SimQuats
import SimEphem
from pm.utils.helper import find_files

def UseTable(ephemfile, quatsfile):
  
  yaw_std_threshold   = 0.5
  pitch_std_threshold = 0.5
  roll_std_threshold  = 1.0
  
  quats = SimQuats.SimQuats(quatsfile)
  
  if quats.ypr[:,0].std() > yaw_std_threshold or \
     quats.ypr[:,1].std() > pitch_std_threshold or \
     quats.ypr[:,2].std() > roll_std_threshold:
    return(0)
  return(1)

def MakeRangeDopplerTables(config_file):
  if not config_file or not os.path.isfile(config_file):
    print>>sys.stderr, 'Config file, %s, does not exist' % config_file
    return 0
  
  try:
    rdf_data                 = rdf.parse(config_file)
    sim_config_file_template = rdf_data["SIM_CONFIG_FILE"]
    ephem_dir                = rdf_data["EPHEM_BYREV_DIR"]
    range_table_dir          = rdf_data["RANGE_TABLE_DIR"]
    doppler_table_dir        = rdf_data["DOPPLER_TABLE_DIR"]
    table_log                = rdf_data["TABLE_LOG"]
  except KeyError:
    print>>sys.stderr, 'Required keywords not found in rdf file: %s\n' % config_file
    return 0
  
  try:
    log_revtags = numpy.genfromtxt( table_log, delimiter=',', usecols=(0,), dtype='|S8' )
    use_table   = numpy.genfromtxt( table_log, delimiter=',', usecols=(1,) )
  except IOError:
    log_revtags = numpy.array([])
    use_table   = numpy.array([])
  
  # Get list of ephem file
  for ephem_file in find_files(ephem_dir,"RS_EPHEM_*"):
    revtag = os.path.basename(ephem_file).strip('RS_EPHEM_')
    
    if log_revtags.size>0 and any(log_revtags==revtag):
      continue
    
    quats_file = ephem_file.replace('RS_EPHEM','RS_QUATS')
    
    if not (os.path.isfile(ephem_file) and os.path.isfile(quats_file)):
      continue
    
    tempfile=subprocess.check_output('mktemp -t RS.rdf.XXXXXX',shell=True).rstrip('\n')
    
    rgcbase = os.path.join(range_table_dir,   'RGC_' + revtag)
    dtcbase = os.path.join(doppler_table_dir, 'DTC_' + revtag)
    
    rgcfile1 = rgcbase + '.1'
    rgcfile2 = rgcbase + '.2'
    dtcfile1 = dtcbase + '.1'
    dtcfile2 = dtcbase + '.2'
    
    rttfile = os.path.join(range_table_dir,   'RTT_' + revtag)
    dtsfile = os.path.join(doppler_table_dir, 'DTS_' + revtag)
    
    rgc1_rep_key = 'BEAM_1_RGC_FILE'
    rgc2_rep_key = 'BEAM_2_RGC_FILE'
    dtc1_rep_key = 'BEAM_1_DTC_FILE'
    dtc2_rep_key = 'BEAM_2_DTC_FILE'
    
    rgc1_rep_line = '%s = %s' % ( rgc1_rep_key, rgcfile1 )
    rgc2_rep_line = '%s = %s' % ( rgc2_rep_key, rgcfile2 )
    dtc1_rep_line = '%s = %s' % ( dtc1_rep_key, dtcfile1 )
    dtc2_rep_line = '%s = %s' % ( dtc2_rep_key, dtcfile2 )
    
    sedcmdstr = 'sed -e "s:%s.*:%s:g" -e "s:%s.*:%s:g" -e "s:%s.*:%s:g" -e "s:%s.*:%s:g" %s > %s' % \
           ( rgc1_rep_key, rgc1_rep_line, rgc2_rep_key, rgc2_rep_line, 
             dtc1_rep_key, dtc1_rep_line, dtc2_rep_key, dtc2_rep_line, 
             sim_config_file_template, tempfile )
    
    subprocess.call(sedcmdstr,shell=True)
    
    if not (os.path.isfile(rgcfile1) and os.path.isfile(rgcfile2)):
      subprocess.call('generate_rgc_from_ephem_quat_files -c %s -out_base %s -out_table %s -e %s -q %s -f -l 4.75 -nadir_clip' % \
                     ( tempfile, rgcbase, rttfile, ephem_file, quats_file ), shell=True )

    if not (os.path.isfile(dtcfile1) and os.path.isfile(dtcfile2)):
      subprocess.call('generate_dtc_from_ephem_quat_files -c %s -out_base %s -out_table %s -e %s -q %s' % ( tempfile, dtcbase, dtsfile, ephem_file, \
                      quats_file ), shell=True )
    
    subprocess.call('rm -f %s' % tempfile,shell=True)
    
    use_this_table = UseTable(ephem_file,quats_file)
    
    log_revtags = numpy.append(log_revtags, revtag)
    use_table   = numpy.append(use_table, use_this_table)
  
  idx = numpy.argsort(log_revtags)
  log_revtags=log_revtags[idx]
  use_table=use_table[idx]
  
  ofp = open(table_log,'w')
  for ii in range(use_table.size):
    print>>ofp, '%s,%d' % (log_revtags[ii], use_table[ii])
  ofp.close()
  return 1

def main():
  # Parse command line
  parser = OptionParser()
  parser.add_option( "-c", "--rdffile", action="store", type="string", dest="rdffile")
  (options, args) = parser.parse_args()
  
  if not options.rdffile or not os.path.isfile(options.rdffile):
    print>>sys.stderr, 'Usage: MakeRangeDopplerTables.py -c config.rdf'
    sys.exit(1)
  
  if MakeRangeDopplerTables( options.rdffile )==0:
    print>>sys.stderr, 'Error in MakeRangeDopplerTables'
    sys.exit(1)
  sys.exit(0)

if __name__=='__main__':
  main()


