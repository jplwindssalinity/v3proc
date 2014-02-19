#==============================================================#
# Copyright (C) 2013-2014, California Institute of Technology. #
# U.S. Government sponsorship acknowledged.                    #
#==============================================================#
"""
Contains Att and Quat attitude classes for doing stuff with
quaternions and attitude records / files.
"""

import os
import sys
import datetime
import util.time
import util.rot
import util.ang
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
    
    def Get(self,time,method='linear'):
        """Returns the yaw, pitch, roll linearly interpolated to time."""
        
        dt = np.asarray([item.total_seconds() for item in self.time - time])
        
        # Find bracketing records
        idx_minus = np.argwhere(dt<=0)
        idx_plus = np.argwhere(dt>0)
        try:
            idx_0 = idx_minus[dt[idx_minus].argmax()][0]
            idx_1 = idx_plus[dt[idx_plus].argmin()][0]
        except ValueError:
            return
        
        t0 = self.time[idx_0]
        t1 = self.time[idx_1]
        
        try:
            weight = (time-t0).total_seconds() / (t1-t0).total_seconds()
        except ZeroDivisionError:
            weight = 0.0
        
        if method=='linear':
            yaw = self.yaw[idx_0] + weight*(
                self.yaw[idx_1]-self.yaw[idx_0])
            pitch = self.pitch[idx_0] + weight*(
                self.pitch[idx_1]-self.pitch[idx_0])
            roll = self.roll[idx_0] + weight*(
                self.roll[idx_1]-self.roll[idx_0])
        elif method=='last':
            yaw = self.yaw[idx_0]
            pitch = self.pitch[idx_0]
            roll = self.roll[idx_0]
        
        return yaw, pitch, roll
    
    def FromQuats(self, quats):
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
    
    def ReadADCO(self, filename):
        """Absurd file format that does not include the year."""
        
        print "Get better ADCO data!"
        
        time_string = np.genfromtxt(filename, dtype=None, skip_header=1, 
                                    delimiter=',', usecols=(0,))
        
        angles = np.genfromtxt(filename, skip_header=1, delimiter=',', 
                               usecols=(1,2,3))
        
        # Do some stuff to make the ADCO data more useable (assume constant
        # attitude until next record, instead of default interpolation between.
        adco_dt = [datetime.datetime.strptime('2013:'+item,'%Y:%j/%H:%M') 
                   for item in time_string]  
        adco_yaw = np.asarray(map(util.ang.wrap, angles[:,0]))
        adco_pitch = np.asarray(map(util.ang.wrap, angles[:,1]))
        adco_roll = np.asarray(map(util.ang.wrap, angles[:,2]))
        
        times = []
        yaws = []
        pitches = []
        rolls = []
        
        for ii in range(len(adco_dt)):
            this_time = adco_dt[ii]
            try:
                next_time = adco_dt[ii+1]+datetime.timedelta(seconds=-1)
            except IndexError:
                next_time = adco_dt[ii]+datetime.timedelta(days=7)
            
            # put two records for every rec, so interpolators will return
            # contant values between the two times (requires linear interp
            # being used, we use slerp for quat interpolation in C++ code)
            times.append(this_time)
            yaws.append(adco_yaw[ii])
            pitches.append(adco_pitch[ii])
            rolls.append(adco_roll[ii])
            
            times.append(next_time)
            yaws.append(adco_yaw[ii])
            pitches.append(adco_pitch[ii])
            rolls.append(adco_roll[ii])
            
        self.time = np.asarray(times)
        self.yaw = np.asarray(yaws)
        self.pitch = np.asarray(pitches)
        self.roll = np.asarray(rolls)

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
