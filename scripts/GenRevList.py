#!/usr/bin/env python
#==============================================================#
# Copyright (C) 2013, California Institute of Technology.      #
# U.S. Government sponsorship acknowledged.                    #
#==============================================================#
#----------------------------------------------------------------------
# NAME
#    GenRevList.py
#
# SYNOPSIS
#    GenRevList.py -i minztimes.txt -o revlist.csv
#
# DESCRIPTION
#    Reads in all minimum z times, writes out a revlist containing:
#    5 digit rev number, sum start time, sim end time.
#    Note sim time-tags are the seconds since Jan 1st 0 hours 1970 UTC.
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

# Functions
def date_time_from_sim( sim_tt ):
  return(datetime.datetime(1970,1,1)+datetime.timedelta(0,sim_tt))

def ToCodeB( dt ):
  string = dt.strftime('%Y-%jT%H:%M:%S') + '.%3.3d' % round(dt.microsecond/1000.)
  return(string)

# Main
from optparse import OptionParser
import datetime
import numpy
import pdb
import sys
import os

parser = OptionParser()
parser.add_option( "-i", "--infile",  action="store", type="string", dest="infile")
parser.add_option( "-o", "--outfile", action="store", type="string", dest="outfile")
(options, args) = parser.parse_args()

if not options.infile or not os.path.isfile(options.infile):
  sys.stderr.write('%s does not exist\n' % options.infile)
  exit(1)

if not options.outfile:
  sys.stderr.write( 'outfile required\n')
  exit(1)

minz_tt = numpy.loadtxt( options.infile, delimiter=' ', usecols=(0,) )

period_guess = 92.8*60.0
rev_no_ref   = 1
# tt_ref       = 1357002995.680623
tt_ref       = 1375289435.359200

ofp = open(options.outfile,'w')
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

exit(0)
