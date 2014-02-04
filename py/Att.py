#==============================================================#
# Copyright (C) 2013-2014, California Institute of Technology. #
# U.S. Government sponsorship acknowledged.                    #
#==============================================================#
"""
Contains Att and Quat attitude classes for doing stuff with
quaternions and attitude records / files.
"""
__version__ = '$Id$'

import os
import sys
import util.time
import util.rot
import numpy as np

RAD_TO_DEG = 180.0/np.pi
DEG_TO_RAD = np.pi/180.0

def SimQuats(filename):
    quats = Quats()
    quats.ReadSim(filename)
    return quats

def AttFromSimQuats(filename):
    att = Att()
    quats = Quats()
    quats.ReadSim(filename)
    att.FromQuats(quats)
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

    def __iter__(self):
        for ii in range(len(self.time)):
            yield self.time[ii], self.yaw[ii], self.pitch[ii], self.roll[ii]

    def __getitem__(self, key):
        return self.time[key], self.yaw[key], self.pitch[key], self.roll[key]
    
    def FromQuats(self,quats):
        import copy
        self.time = copy.copy(quats.time)
        self.roll = np.zeros(self.time.shape)
        self.yaw = np.zeros(self.time.shape)
        self.pitch = np.zeros(self.time.shape)
        
        for ii in range(len(self.time)):
            quat_time, quat = quats[ii]
            self.pitch[ii], self.roll[ii], self.yaw[ii] = util.rot.q2att(quat)
        
        self.pitch *= RAD_TO_DEG
        self.roll *= RAD_TO_DEG
        self.yaw *= RAD_TO_DEG

class Quats:
    """
    Quaternion class:
    
    time  -- UTC datetime objects
    quats -- Quaternions in (w,x,y,z) order
    """
    def __init__(self):
        self.time = None
        self.quats = None
    
    def __iter__(self):
        for ii in range(len(self.time)):
            yield self.time[ii], self.quats[ii,:]
    
    def __getitem__(self, key):
        return self.time[key], self.quats[key,:]
    
    def ReadSim(self, filename):
        quat_size = os.path.getsize(filename)
        n_quats = quat_size/(5.0*8.0)
        n_quats = int(n_quats)
        tmp = np.fromfile(filename, count=-1, sep="").reshape((n_quats,5))
        self.time = np.asarray(
            [util.time.datetime_from_sim(item) for item in tmp[:,0]])
        self.quats = tmp[:,(1,2,3,4)]
    
    def WriteSim(self, filename):
        data = np.zeros([len(self.time),5])
        data[:, 0] = [util.time.sim_from_datetime(item) for item in self.time]
        data[:, 1:] = self.quats
        data.astype('<f8').tofile(filename)
    
    def FromAtt(self, att):
        import copy
        self.time = copy.copy(att.time)
        self.quats = np.zeros([len(att.time),4])
        
        for ii in range(len(self.time)):
            att_time, yaw, pitch, roll = att[ii]
            angles = (pitch*DEG_TO_RAD, roll*DEG_TO_RAD, yaw*DEG_TO_RAD)
            self.quats[ii,:] = util.rot.att2q(angles)
