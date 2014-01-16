#!/usr/bin/env python2.7
#==============================================================#
# Copyright (C) 2013-2014, California Institute of Technology. #
# U.S. Government sponsorship acknowledged.                    #
#==============================================================#
#----------------------------------------------------------------------
# NAME
#    UpdateRevsDB.py
#
# SYNOPSIS
#    UpdateRevsDB.py -c config_file
#
# DESCRIPTION
#    Updates the revs DB to be in sync with the revlist CSV file
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
import time_funcs
import pm.database.revs
import pm.database.revlist

def UpdateRevsDB(config_file):
  if not config_file or not os.path.isfile(config_file):
    print>>sys.stderr, 'Config file, %s, does not exist' % config_file
    return 0
  try:
    rdf_data = rdf.parse(config_file)
    revlist        = rdf_data["REVLIST"]
    revlist_db     = rdf_data["REVLIST_DB"]
  except KeyError:
    print>>sys.stderr, 'Required keywords not found in rdf file: %s\n' % config_file
    return 0
  
  if not os.path.isfile(revlist):
    print>>sys.stderr,"%s does not exist" % revlist
    return 0
  
  if not os.path.isfile(revlist_db):
    # Make a new old
    pm.database.revlist.reader(revlist) >> revlist_db
  else:
    # Try to update it
    rev_db   = pm.database.revs.RevDataBase(revlist_db)
    rev_rows = pm.database.revlist.reader(revlist)
    
    for rev_row in rev_rows:
      if not rev_row in rev_db:
        rev_db.add(rev_row.rev, 
                   time_funcs.date_time_from_sim(rev_row.fstart),
                   time_funcs.date_time_from_sim(rev_row.fstop),
                   -1)
  return 1


if __name__=='__main__':
  # Parse command line
  parser = OptionParser()
  parser.add_option( "-c", "--rdffile", action="store", type="string", dest="rdffile")
  (options, args) = parser.parse_args()
  
  if not options.rdffile or not os.path.isfile(options.rdffile):
    print>>sys.stderr, 'Usage: UpdateRevsDB.py -c config.rdf'
    sys.exit(1)
  
  if UpdateRevsDB( options.rdffile )==0:
    print>>sys.stderr, 'Error in UpdateRevsDB'
    sys.exit(1)
  sys.exit(0)

