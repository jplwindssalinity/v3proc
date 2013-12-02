#!/usr/bin/env python
#==============================================================#
# Copyright (C) 2013, California Institute of Technology.      #
# U.S. Government sponsorship acknowledged.                    #
#==============================================================#
#----------------------------------------------------------------------
# NAME
#    EphemGetMinzTimes.py
#
# SYNOPSIS
#    EphemGetMinzTimes.py -f ephem_file > min_z_times.txt
#
# DESCRIPTION
#    Parses the ephemeris file, does linear interpolation to find all
#    times of minimum z positions. Print rev start and stop times to
#    stdout in both sim seconds since epoch and CODE B format.
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

from optparse import OptionParser
import datetime
import struct
import os
import pdb
import array

parser = OptionParser()
parser.add_option( "-f", "--file", 
                   action="store", type="string", dest="ephemfile")
(options, args) = parser.parse_args()


if not options.ephemfile or not os.path.isfile(options.ephemfile):
  print '%s does not exist' % options.ephemfile
  exit(1)

ephem_size = os.path.getsize( options.ephemfile )
n_ephem = ephem_size/(7.0*8.0)

if round(n_ephem) != n_ephem:
  print '%s does not have correct file size' % options.ephemfile
  exit(1) # indicate failure and quit

n_ephem = int(n_ephem)

time = array.array('d')
posx = array.array('d')
posy = array.array('d')
posz = array.array('d')
velx = array.array('d')
vely = array.array('d')
velz = array.array('d')
try:
  ifp = open(options.ephemfile,"rb")
except IOError:
   print 'Error opening file %s' % options.ephemfile
   exit(1)

for ii in range(0,n_ephem):
  time.fromfile(ifp,1)
  posx.fromfile(ifp,1)
  posy.fromfile(ifp,1)
  posz.fromfile(ifp,1)
  velx.fromfile(ifp,1)
  vely.fromfile(ifp,1)
  velz.fromfile(ifp,1)
ifp.close()

t_minz = array.array('d')
minz_count = 0
for ii in range(0,n_ephem-1):
  if velz[ii] < 0 and velz[ii+1] >= 0:
    minz_count += 1
    # linear interpolation for time of minimum z position (velz==0)
    t_minz.append(time[ii] + (time[ii+1]-time[ii])*(-velz[ii])/(velz[ii+1]-velz[ii]))

for ii in range(t_minz.buffer_info()[1]-1):
  start_time = t_minz[ii]
  delta      = t_minz[ii+1]-t_minz[ii]
  n_revs     = round( delta/5571.792 )
  end_time   = start_time + delta / n_revs
  
  dt_start = datetime.datetime(1970,1,1)+datetime.timedelta(0,start_time)
  dt_end   = datetime.datetime(1970,1,1)+datetime.timedelta(0,end_time)
  
#   print dt_start.strftime('%Y-%jT%H:%M:%S.%f'), dt_end.strftime('%Y-%jT%H:%M:%S.%f')
  
  print '%f' % start_time, '%f' % end_time, dt_start.strftime('%Y%j%H%M%S'), dt_end.strftime('%Y%j%H%M%S')

# indicate success.
exit(0)