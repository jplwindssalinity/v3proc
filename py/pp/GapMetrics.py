#!/usr/bin/env python2.7
#==============================================================#
# Copyright (C) 2013-2014, California Institute of Technology. #
# U.S. Government sponsorship acknowledged.                    #
#==============================================================#
#----------------------------------------------------------------------
# NAME
#    GapMetrics.py
#
# SYNOPSIS
#    GapMetrics.py config_file
#
# DESCRIPTION
#    Load the gap report and plots some metrics.
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
#       1  ???
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
import os
import pdb
import rdf
import numpy
import subprocess
import datetime
import util.time
import math
import matplotlib.pyplot as plt
import matplotlib

def GapMetrics(config_file):
  if not config_file or not os.path.isfile(config_file):
    print>>sys.stderr, 'Config file, %s, does not exist' % config_file
    return 0
  try:
    rdf_data  = rdf.parse(config_file)
    gapreport = rdf_data["GAP_REPORT"]
  except KeyError:
    print>>sys.stderr, 'Required keywords not found in rdf file: %s\n' % config_file
    return 0
  
  # load gap start/end times
  gap_size    = numpy.loadtxt( gapreport, delimiter=',', usecols=(0,) )
  gap_t_start = numpy.loadtxt( gapreport, delimiter=',', usecols=(1,) )
  gap_t_end   = numpy.loadtxt( gapreport, delimiter=',', usecols=(2,) )
  
  # Compute gap start time and gap ratio metric
  gap_start_time = ()
  days_since_gap = ()
  gap_ratio      = ()
  for ii in range(2,gap_t_start.size):
    gap_start_time += (util.time.date_time_from_sim(gap_t_start[ii]),)
    days_since_gap += ((gap_t_start[ii]-gap_t_end[ii-1])/86400.0,)
    gap_ratio      += ((gap_size[ii])/(gap_t_start[ii]-gap_t_end[ii-1]),)
  
  # plot it up
  dates = matplotlib.dates.date2num(gap_start_time)
  plt.plot_date(dates,gap_ratio)
  plt.grid(True)
  plt.ylabel('Gap Duration / Time Since Last Gap')
  plt.xlabel('Gap Start Time')
  plt.title('GSE Gap Ratio vs Time; %d Gaps Total' % gap_size.size)
  plt.yscale('log')
  plt.axis( ( plt.axis()[0], plt.axis()[1], 0, 1 ) )
  plt.show()
  
# Make a monthly average of the gap ratio
#   days_per_bin = 7
#   dt_ref       = datetime.datetime(2013,8,1)
#   num_bins     = math.ceil((datetime.datetime.utcnow()-dt_ref).days / days_per_bin)
#   
#   for ii in range(num_bins):
#     sim_tt_low  = util.time.sim_from_date_time(dt_ref)+ii*days_per_bin*86400.0)
#     sim_tt_high = sim_tt_low + days_per_bin*86400.0
#     
#     mask = numpy.logical_and(gap_t_start>=sim_tt_low,gap_t_start<sim_tt_high)
#     idx  = nympy.argwhere(mask)
#     
#     
#     
# #   for ii in range(len(bin_start)):
# #     mask = numpy.logical_and( gap_t_start>util.time.sim_from_date_time(gap_start_time[0])
# #   
#   
#   pdb.set_trace()
# 
  
  
  return 1
  
if __name__=='__main__':
  usage_string = 'Usage: %s <config_file>' % sys.argv[0]
  config_file = sys.argv[1]
  
  if not os.path.isfile(config_file):
    print>>sys.stderr, usage_string
    sys.exit(1)
  
  if GapMetrics( config_file )==0:
    print>>sys.stderr, 'Error in GapMetrics'
    sys.exit(1)
  sys.exit(0)