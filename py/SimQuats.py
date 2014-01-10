#==============================================================#
# Copyright (C) 2013, California Institute of Technology.      #
# U.S. Government sponsorship acknowledged.                    #
#==============================================================#
rcs_id      = '$Id$'
__version__ = '$Revision$'

import math
import os
import sys
import numpy as np

class SimQuats:
  """ Class for QScatSim Quaternion Files
  
  """
  def __init__(self,filename=None):
    self.time     = None
    self.quats    = None
    self.ypr      = None
    self.filename = filename
    
    if filename != None:
      self.Read()
      self.ComputeYPR()
  
  def Read(self):
    """Reads in the quaternion files."""
    if not os.path.isfile(self.filename):
      print>>sys.stderr, 'SimEphem::Read: %s does not exist' % self.filename
      return
    
    quat_size = os.path.getsize( self.filename )
    n_quats = quat_size/(5.0*8.0)
    if round(n_quats) != n_quats:
      print>>sys.stderr, 'SimQuats::Read: %s does not have correct file size' % self.filename
      return
    
    n_quats = int(n_quats)
    tmp = np.fromfile( self.filename, count=-1, sep="" ).reshape((n_quats,5))
    
    self.time  = tmp[:,0]
    self.quats = tmp[:,(1,2,3,4)]
  
  def ComputeYPR(self):
    w = self.quats[:,0]
    x = self.quats[:,1]
    y = self.quats[:,2]
    z = self.quats[:,3]
    
    x11 = 1.0 - 2.0 * ( y*y + z*z )
    x12 =       2.0 * ( x*y - w*z )
    x13 =       2.0 * ( x*z + w*y )
    
    x21 =       2.0 * ( x*y + w*z )
    x22 = 1.0 - 2.0 * ( x*x + z*z )
    x23 =       2.0 * ( y*z - w*x )
    
    x31 =       2.0 * ( x*z - w*y )
    x32 =       2.0 * ( y*z + w*x )
    x33 = 1.0 - 2.0 * ( x*x + y*y )
    
    cos_roll = np.sqrt( x21*x21 + x22*x22 )
    sin_roll = -x23
    
    roll  = np.arctan2( sin_roll, cos_roll ) * 180.0 / np.pi;
    yaw   = np.zeros(roll.shape)
    pitch = np.zeros(roll.shape)
    
    mask = np.logical_or( cos_roll==0, np.abs(sin_roll)==1 )
    
    idx = np.argwhere( mask )
    yaw[idx]   = 0.0
    pitch[idx] = np.arctan2( x12[idx] / sin_roll[idx], x11[idx] ) * 180.0/np.pi
    
    idx = np.argwhere(~mask)
    yaw[idx]   = np.arctan2( x21[idx], x22[idx] ) * 180.0/np.pi
    pitch[idx] = np.arctan2( x13[idx], x33[idx] ) * 180.0/np.pi
    
    self.ypr = np.zeros([roll.size,3])
    self.ypr[:,0] = yaw
    self.ypr[:,1] = pitch
    self.ypr[:,2] = roll
    
    
    