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

import datetime
import struct
import os
import pdb
import array
import sys

def EphemGetMinzTimes( ephemfile, outfile ):
  ephem_size = os.path.getsize( ephemfile )
  n_ephem = ephem_size/(7.0*8.0)
  
  if round(n_ephem) != n_ephem:
    print>>sys.stderr, '%s does not have correct file size' % ephemfile
    return 1
  
  n_ephem = int(n_ephem)
  
  time = array.array('d')
  posx = array.array('d')
  posy = array.array('d')
  posz = array.array('d')
  velx = array.array('d')
  vely = array.array('d')
  velz = array.array('d')
  
  try:
    ifp = open(ephemfile,"rb")
  except IOError:
     print>>sys.stderr, 'Error opening file %s' % ephemfile
     return 0
  
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
  
  if outfile:
    ofp = open(outfile,'w')
  else:
    ofp = sys.stdout
    
  for ii in range(t_minz.buffer_info()[1]):
    dt_start   = datetime.datetime(1970,1,1)+datetime.timedelta(0,t_minz[ii])
    print>>ofp, '%f' % t_minz[ii], dt_start.strftime('%Y%j%H%M%S')
  
  if outfile:
    ofp.close()
    
  return 1

if __name__ == '__main__':
  from optparse import OptionParser
  parser = OptionParser()
  parser.add_option( "-f", "--ephemfile", action="store", type="string", dest="ephemfile")
  parser.add_option( "-o", "--outfile",   action="store", type="string", dest="outfile")
  (options, args) = parser.parse_args()
  
  if not options.ephemfile or not os.path.isfile(options.ephemfile):
    print>>sys.stderr, '%s does not exist' % options.ephemfile
    sys.exit(1)
  
  EphemGetMinzTimes( options.ephemfile, options.outfile )
  
  sys.exit(0)



