#==============================================================#
# Copyright (C) 2013, California Institute of Technology.      #
# U.S. Government sponsorship acknowledged.                    #
#==============================================================#
rcs_id = "$Id$"
import math
import os
import sys
import numpy as np

class SimEphem:
  """ Class for QScatSim Ephemeris Files
  
  """
  def __init__(self,filename=None):
    self.time     = None
    self.pos      = None
    self.vel      = None
    self.filename = filename
    
    if filename != None:
      self.Read()
  
  def Read(self):
    """Reads in the ephemeris files."""
    if not os.path.isfile(self.filename):
      print>>sys.stderr, 'SimEphem::Read: %s does not exist' % self.filename
      return
    
    ephem_size = os.path.getsize( self.filename )
    n_ephem = ephem_size/(7.0*8.0)
    if round(n_ephem) != n_ephem:
      print>>sys.stderr, 'SimEphem::Read: %s does not have correct file size' % self.filename
      return
    
    n_ephem = int(n_ephem)
    tmp = np.fromfile( self.filename, count=-1, sep="" ).reshape((n_ephem,7))
    
    self.time = tmp[:,0]
    self.pos  = tmp[:,(1,2,3)]
    self.vel  = tmp[:,(4,5,6)]
  
  def GetMinZTimes(self):
    """Computes the minimum Z position times of the spacecraft"""
    
    if self.time==None or self.pos==None or self.vel==None:
      print>>sys.stderr, 'SimEphem::GetMinZTimes: Ephem not loaded!'
      return None
    
    t_minz = []
    for ii in range(self.time.size-1):
      if self.vel[ii,2] < 0 and self.vel[ii+1,2]>=0:
        t_minz.append(self.time[ii] + (self.time[ii+1]-self.time[ii]) * \
                     (-self.vel[ii,2])/(self.vel[ii+1,2]-self.vel[ii,2]))
    return(t_minz)
  
  def GetAscendingNodes(self):
    """Computes the ascending node longitudes(s)
    Returns a dictionary with 'times' and 'longs' containing the node time
    and longitude
    """
    
    if self.time==None or self.pos==None or self.vel==None:
      print>>sys.stderr, 'SimEphem::GetMinZTimes: Ephem not loaded!'
      return None
    
    asc_node_longs = []
    asc_node_times = []
    for ii in range(self.time.size-1):
      if self.pos[ii,2]<0 and self.pos[ii+1,2]>=0:
        
        dz = ( 0.0 - self.pos[ii,2] ) / ( self.pos[ii+1,2] - self.pos[ii,2] )
        
        tnode = self.time[ii]  + ( self.time[ii]    - self.time[ii]  ) * dz
        xnode = self.pos[ii,0] + ( self.pos[ii+1,0] - self.pos[ii,0] ) * dz
        ynode = self.pos[ii,1] + ( self.pos[ii+1,1] - self.pos[ii,1] ) * dz
        
        asc_node_times.append( tnode )
        asc_node_longs.append( math.atan2( ynode, xnode ) * 180.0 / math.pi )
    
    out = {'times': asc_node_times, 'longs': asc_node_longs }
    return(out)
    