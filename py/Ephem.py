#==============================================================#
# Copyright (C) 2013-2014, California Institute of Technology. #
# U.S. Government sponsorship acknowledged.                    #
#==============================================================#
__version__ = '$Id$'

import os
import sys
import math
import datetime
import spice
import pysofa
import util.time
import numpy as np

FEET_TO_METERS = 0.3048006096012

def ConvertTOPO1950(infile,outfile):
    ephem = TOPO1950(infile)
    ephem.WriteSim(outfile)

def Sim(filename):
    ephem = Ephem()
    ephem.ReadSim(filename)
    return(ephem)

def TOPO1950(filename):
    ephem = Ephem()
    ephem.ReadTOPO1950(infile)
    return(ephem)

class Ephem(object):
    """
    Ephem class:
    
    time -- UTC datetime objects
    pos  -- Earth Centered Earth Fixed position vector [meters]
    vel  -- Earth Centered Earth Fixed velocity vector [meters/second]
    """
    def __init__(self):
        self.time = None
        self.pos = None
        self.vel = None

    def ReadSim(self,filename):
        ephem_size = os.path.getsize(filename)
        n_ephem = ephem_size/(7.0*8.0)
        
        tmp = np.fromfile(filename, count=-1, sep="").reshape((n_ephem, 7))
        self.time = np.asarray(
            [util.time.datetime_from_sim(item) for item in tmp[:,0]])
        self.pos = tmp[:, (1, 2, 3)]
        self.vel = tmp[:, (4, 5, 6)]

    def WriteSim(self,filename):
        data = np.zeros([len(self.time),7])
        data[:, 0] = [util.time.sim_from_datetime(item) for item in self.time]
        data[:, (1, 2, 3)] = self.pos
        data[:, (4, 5, 6)] = self.vel
        data.astype('<f8').tofile(outfile)

    def GetMinZTimes(self):
        """
        Returns the minimum Z position times of the spacecraft as a 
        list of datetime objects
        """
        if self.time == None or self.pos == None or self.vel == None:
            print>>sys.stderr, 'Ephem::GetMinZTimes: Ephem not loaded!'
            return None

        t_minz = []
        for ii in range(self.time.size-1):
            if self.vel[ii, 2] < 0 and self.vel[ii+1, 2] >= 0:
                t_minz.append(util.time.datetime_interp(
                    self.vel[ii, 2], self.vel[ii+1, 2], 0.0,
                    self.time[ii], self.time[ii+1]))
        return(t_minz)

    def GetAscendingNodes(self):
        """
        Computes the ascending node longitudes(s)
        Returns a dictionary with 'times' and 'longs' containing the node time
        and longitude.  'times' entry of dict is a list of datetime objects.
        """
        if self.time == None or self.pos == None or self.vel == None:
            print>>sys.stderr, 'Ephem::GetMinZTimes: Ephem not loaded!'
            return None

        asc_node_longs = []
        asc_node_times = []
        for ii in range(self.time.size-1):
            if self.pos[ii, 2] < 0 and self.pos[ii+1, 2] >= 0:

                dz = (0.0 - self.pos[ii, 2]) / (self.pos[ii+1, 2]
                   - self.pos[ii, 2])

                tnode = util.time.datetime_interp( 
                    self.pos[ii, 2], self.pos[ii+1, 2], 0.0,
                    self.time[ii], self.time[ii+1] )
                xnode = self.pos[ii, 0]+(self.pos[ii+1, 0]-self.pos[ii, 0])*dz
                ynode = self.pos[ii, 1]+(self.pos[ii+1, 1]-self.pos[ii, 1])*dz

                asc_node_times.append(tnode)
                asc_node_longs.append(math.atan2(ynode, xnode)*180.0/math.pi)

        out = {'times': asc_node_times, 'longs': asc_node_longs }
        return out

    def ReadTOPO1950(self,filename):
        ifp = open(filename, "r")
        headerline1 = ifp.readline()
        headerline2 = ifp.readline()
        year = int(headerline2.split()[0])

        data = np.loadtxt(ifp)
        ifp.close()
        pos = data[:, (1, 2, 3)] * FEET_TO_METERS
        vel = data[:, (4, 5, 6)] * FEET_TO_METERS

        self.time = datetime.datetime(year, 1, 1) + np.asarray(
                    [datetime.timedelta(seconds=x) for x in data[:, 0]])

        self.pos = np.zeros(pos.shape)
        self.vel = np.zeros(vel.shape)

        b1950_to_j2000 = np.asarray(spice.pxform('B1950', 'J2000',
                                                 spice.j2000()))

        for ii in range(len(self.time)):
            [jd0,jd1] = pysofa.cal2jd(self.time[ii].year, self.time[ii].month,
                                      self.time[ii].day)
            
            jd1 += (self.time[ii] - datetime.datetime(
              self.time[ii].year,
              self.time[ii].month,
              self.time[ii].day)).total_seconds()/24/60/60
            
            [rb, rp, rbp] = pysofa.bp00(jd0,jd1);
            gha = pysofa.gst00a(jd0,jd1,jd0,jd1)
            
            rot_pos = pysofa.rxp(b1950_to_j2000, pos[ii,:])
            rot_pos = pysofa.rxp(rp, rot_pos )
            
            self.pos[ii,0] = rot_pos[0,0]*np.cos(gha)+rot_pos[0,1]*np.sin(gha)
            self.pos[ii,1] =-rot_pos[0,0]*np.sin(gha)+rot_pos[0,1]*np.cos(gha)
            self.pos[ii,2] = rot_pos[0,2]

            rot_vel = pysofa.rxp(b1950_to_j2000, vel[ii, :])
            rot_vel = pysofa.rxp(rp, rot_vel )

            self.vel[ii,0] = rot_vel[0,0]*np.cos(gha)+rot_vel[0,1]*np.sin(gha)
            self.vel[ii,1] =-rot_vel[0,0]*np.sin(gha)+rot_vel[0,1]*np.cos(gha)
            self.vel[ii,2] = rot_vel[0,2]
