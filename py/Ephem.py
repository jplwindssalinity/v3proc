#==============================================================#
# Copyright (C) 2013-2014, California Institute of Technology. #
# U.S. Government sponsorship acknowledged.                    #
#==============================================================#

import pdb
import os
import sys
import math
import datetime
import spice
import pysofa
import util.time
import numpy as np

FEET_TO_METERS = 0.3048006096012

def ConvertTOPOB1950(infile, outfile):
    ephem = TOPOB1950(infile)
    ephem.WriteSim(outfile)

def ConvertTOPOJ2000(infile, outfile):
    ephem = TOPOJ2000(infile)
    ephem.WriteSim(outfile)

def Sim(filename):
    ephem = Ephem()
    ephem.ReadSim(filename)
    return ephem

def TOPOB1950(filename):
    ephem = Ephem()
    ephem.ReadTOPOB1950(filename)
    return ephem

def TOPOJ2000(filename):
    ephem = Ephem()
    ephem.ReadTOPOJ2000(filename)
    return ephem

def datetime_to_JD(dt):
    jd0, jd1 = pysofa.cal2jd(dt.year, dt.month, dt.day)
    jd1 += (dt - datetime.datetime(dt.year, dt.month, dt.day)
           ).total_seconds()/24/60/60
    return jd0, jd1

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

    def __iter__(self):
        for ii in range(len(self.time)):
            yield self.time[ii], self.pos[ii, :], self.vel[ii, :]

    def __getitem__(self, key):
        return self.time[key], self.pos[key, :], self.vel[key, :]

    def ReadSim(self, filename):
        ephem_size = os.path.getsize(filename)
        n_ephem = ephem_size/(7.0*8.0)

        tmp = np.fromfile(filename, count=-1, sep="").reshape((n_ephem, 7))
        self.time = np.asarray(
            [util.time.datetime_from_sim(item) for item in tmp[:, 0]])
        self.pos = tmp[:, (1, 2, 3)]
        self.vel = tmp[:, (4, 5, 6)]

    def WriteSim(self, filename):
        data = np.zeros([len(self.time), 7])
        data[:, 0] = [util.time.sim_from_datetime(item) for item in self.time]
        data[:, (1, 2, 3)] = self.pos
        data[:, (4, 5, 6)] = self.vel
        data.astype('<f8').tofile(filename)

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
        return t_minz

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
                    self.time[ii], self.time[ii+1])
                xnode = self.pos[ii, 0]+(self.pos[ii+1, 0]-self.pos[ii, 0])*dz
                ynode = self.pos[ii, 1]+(self.pos[ii+1, 1]-self.pos[ii, 1])*dz

                asc_node_times.append(tnode)
                asc_node_longs.append(math.atan2(ynode, xnode)*180.0/math.pi)

        out = {'times': asc_node_times, 'longs': asc_node_longs}
        return out

    def ReadTOPOB1950(self, filename):
        """For reading in the B1950 TOPO ISS predicted ephmeris."""
        with open(filename, 'r') as ifp:
            headerline1 = ifp.readline()
            headerline2 = ifp.readline()
            data = np.loadtxt(ifp)

        year = int(headerline2.split()[0])

        pos = data[:, (1, 2, 3)] * FEET_TO_METERS
        vel = data[:, (4, 5, 6)] * FEET_TO_METERS

        self.time = datetime.datetime(year, 1, 1) + np.asarray(
                    [datetime.timedelta(seconds=x) for x in data[:, 0]])

        self.pos = np.zeros(pos.shape)
        self.vel = np.zeros(vel.shape)

        b1950_to_j2000 = np.mat(spice.pxform('B1950', 'J2000', spice.j2000()))

        for ii in range(len(self.time)):
            jd0, jd1 = datetime_to_JD(self.time[ii])
            rb, rp, rbp = pysofa.bp00(jd0, jd1);
            gha = pysofa.gst00a(jd0, jd1, jd0, jd1)

            rot_pos = rp * b1950_to_j2000 * np.mat(pos[ii, :]).T

            self.pos[ii, 0] = rot_pos[0]*np.cos(gha)+rot_pos[1]*np.sin(gha)
            self.pos[ii, 1] =-rot_pos[0]*np.sin(gha)+rot_pos[1]*np.cos(gha)
            self.pos[ii, 2] = rot_pos[2]

            rot_vel = rp * b1950_to_j2000 * np.mat(vel[ii, :]).T

            self.vel[ii, 0] = rot_vel[0]*np.cos(gha)+rot_vel[1]*np.sin(gha)
            self.vel[ii, 1] =-rot_vel[0]*np.sin(gha)+rot_vel[1]*np.cos(gha)
            self.vel[ii, 2] = rot_vel[2]

    def ReadTOPOJ2000(self, filename):
        """For reading in the J2000 TOPO ISS predicted ephmeris."""
        time_strings = np.genfromtxt(filename, dtype=None, skip_header=3,
                                     usecols=(0,))

        pos = np.genfromtxt(filename, skip_header=3, usecols=(5, 6, 7))
        vel = np.genfromtxt(filename, skip_header=3, usecols=(8, 9, 10))

        self.time = np.asarray(
            [datetime.datetime.strptime(item+'000', '%Y:%m:%d:%H:%M:%S.%f')
            for item in time_strings])

        self.pos = np.zeros(pos.shape)
        self.vel = np.zeros(vel.shape)

        for ii in range(len(self.time)):
            jd0, jd1 = datetime_to_JD(self.time[ii])
            rb, rp, rbp = pysofa.bp00(jd0, jd1);
            gha = pysofa.gst00a(jd0, jd1, jd0, jd1)

            rot_pos = rp * np.mat(pos[ii, :]).T
            self.pos[ii, 0] = rot_pos[0]*np.cos(gha)+rot_pos[1]*np.sin(gha)
            self.pos[ii, 1] =-rot_pos[0]*np.sin(gha)+rot_pos[1]*np.cos(gha)
            self.pos[ii, 2] = rot_pos[2]

            rot_vel = rp * np.mat(vel[ii, :]).T
            self.vel[ii, 0] = rot_vel[0]*np.cos(gha)+rot_vel[1]*np.sin(gha)
            self.vel[ii, 1] =-rot_vel[0]*np.sin(gha)+rot_vel[1]*np.cos(gha)
            self.vel[ii, 2] = rot_vel[2]
