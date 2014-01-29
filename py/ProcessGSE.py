#!/usr/bin/env python2.7
#==============================================================#
# Copyright (C) 2013-2014, California Institute of Technology. #
# U.S. Government sponsorship acknowledged.                    #
#==============================================================#
#----------------------------------------------------------------------
# NAME
#    ProcessGSE.py
#
# SYNOPSIS
#    ProcessGSE.py config_file
#
# DESCRIPTION
#    Makes the revlist, gap report, and gse times files.  Then it
#    extracts the GSE data by rev, ephem by rev, and fills gaps
#    in the ephem files.
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
rcs_id      = '$Id$'
__version__ = '$Revision$'

QSCATSIM_PY_DIR='/home/fore/qscatsim/QScatSim/py'

import sys
if not QSCATSIM_PY_DIR in sys.path:
  sys.path.append(QSCATSIM_PY_DIR)

from optparse import OptionParser
import os
import rdf
import pdb
import subprocess
import GenRevList
import UpdateRevsDB
import GSEGapReport
import ExtractGSEByRev
import ExtractEphemByRev
import EphemQuatFillGaps
import MakeRangeDopplerTables
import ConvertToGS

def ProcessGSE( config_file ):
  # Makes the revlist file
  GenRevList.GenRevList(config_file)
  
  # Updates the revlist database to be in sync with revlist file
  UpdateRevsDB.UpdateRevsDB(config_file)
  
  # Makes the Gap report file
  GSEGapReport.GSEGapReport(config_file)
  
  # Extracts the GSE data into rev chunks
  ExtractGSEByRev.ExtractGSEByRev(config_file)
  
  # Converts GSE data to sim ephem and quats files for each GSE rev file
  ExtractEphemByRev.ExtractEphemByRev(config_file)
  
  # Fills gaps in the gap-ridden ephem files
  EphemQuatFillGaps.EphemQuatFillGaps(config_file)
  
  # Makes the range and doppler talbes
  MakeRangeDopplerTables.MakeRangeDopplerTables(config_file)
  
  # Convert Stuff to GS Format
  ConvertToGS.ConvertToGS(config_file)
  
  # Done processing the GSE data!
  return(1)

def main():
  usage_string = 'Usage: %s <config_file>' % sys.argv[0]
  config_file = sys.argv[1]
  
  if not os.path.isfile(config_file):
    print>>sys.stderr, usage_string
    sys.exit(1)
  
  if ProcessGSE(config_file)==0:
    print>>sys.stderr, 'Error in ProcessGSE'
    sys.exit(1)
  sys.exit(0)

if __name__ == '__main__':
    main()


