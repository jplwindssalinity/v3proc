#==============================================================#
# Copyright (C) 2013-2014, California Institute of Technology. #
# U.S. Government sponsorship acknowledged.                    #
#==============================================================#
__version__ = '$Id$'

import os
import sys
import util.time
import numpy as np

def SimQuats(filename):
    quats = Quats()
    quats.ReadSim(filename)
    return quats

def AttFromSimQuats(filename):
    att = Att()
    quats = Quats()
    quats.ReadSim(filename)
    att.FromQuat(quats)
    return att

class Att:
    """
    Attitude class:
    
    time           -- UTC datetime objects
    yaw,pitch,roll -- degrees
    """
    def __init__(self):
        self.time = None
        self.yaw = None
        self.pitch = None
        self.roll = None

    def __getitem__(self, key):
        return self.time[key], self.yaw[key], self.pitch[key], self.roll[key]

    def FromQuat(self,quat):
        """Note this makes an assumption about order of rotations!"""
        import copy
        self.time = copy.copy(quat.time)

        w = quat.quats[:,0]
        x = quat.quats[:,1]
        y = quat.quats[:,2]
        z = quat.quats[:,3]

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

        self.roll  = np.arctan2(sin_roll, cos_roll) * 180.0 / np.pi;
        self.yaw   = np.zeros(self.roll.shape)
        self.pitch = np.zeros(self.roll.shape)

        mask = np.logical_or(cos_roll==0, np.abs(sin_roll)==1)

        idx = np.argwhere( mask )
        self.yaw[idx]   = 0.0
        self.pitch[idx] = np.arctan2(x12[idx]/sin_roll[idx], x11[idx])*180.0/np.pi

        idx = np.argwhere(~mask)
        self.yaw[idx]   = np.arctan2(x21[idx], x22[idx]) * 180.0/np.pi
        self.pitch[idx] = np.arctan2(x13[idx], x33[idx]) * 180.0/np.pi

class Quats:
    """
    Quaternion class:
    
    time  -- UTC datetime objects
    quats -- Quaternions in (w,x,y,z) order
    """
    def __init__(self):
        self.time = None
        self.quats = None
    
    def __getitem__(self, key):
        return self.time[key], self.quats[key,:]
    
    def ReadSim(self,filename):
        quat_size = os.path.getsize(filename)
        n_quats = quat_size/(5.0*8.0)
        n_quats = int(n_quats)
        tmp = np.fromfile(filename, count=-1, sep="").reshape((n_quats,5))
        self.time = np.asarray(
            [util.time.datetime_from_sim(item) for item in tmp[:,0]])
        self.quats = tmp[:,(1,2,3,4)]
