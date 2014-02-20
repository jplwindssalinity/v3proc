#==============================================================#
# Copyright (C) 2013-2014, California Institute of Technology. #
# U.S. Government sponsorship acknowledged.                    #
#==============================================================#
"""
Helper functions for doing stuff with time-tags.
"""

import pdb
import sys
import datetime

GPS_EPOCH = datetime.datetime(1980, 1, 6)
SIM_EPOCH = datetime.datetime(1970, 1, 1)
GS_EPOCH  = datetime.datetime(1993, 1, 1)
ONE_DAY = datetime.timedelta(1)
DT_LEAP = [ONE_DAY+item for item in [datetime.datetime(1979,12,31),
                                     datetime.datetime(1981, 6,30),
                                     datetime.datetime(1982, 6,30),
                                     datetime.datetime(1983, 6,30),
                                     datetime.datetime(1985, 6,30),
                                     datetime.datetime(1987,12,31),
                                     datetime.datetime(1989,12,31),
                                     datetime.datetime(1990,12,31),
                                     datetime.datetime(1992, 6,30),
                                     datetime.datetime(1993, 6,30),
                                     datetime.datetime(1994, 6,30),
                                     datetime.datetime(1995,12,31),
                                     datetime.datetime(1997, 6,30),
                                     datetime.datetime(1998,12,31),
                                     datetime.datetime(2005,12,31),
                                     datetime.datetime(2008,12,31),
                                     datetime.datetime(2012, 6,30)]]

def datetime_interp(x0, x1, x, dt0, dt1):
    """
    Linearly interpolates two datetime objects (x==x0 at dt0; x==x1 at dt1
    to desired value (x).
    """
    if x1==x0:
        weight=0.0
    else:
        weight = (x-x0)/(x1-x0)
    time_off = datetime.timedelta(seconds=(dt1-dt0).total_seconds()*weight)
    return(dt0+time_off)

def tz_delta():
    return(datetime.datetime.utcfromtimestamp(0) -
           datetime.datetime.fromtimestamp(0))

def datetime_from_gs(gs_tt):
    """Converts a GS referenced time-tag to datetime object"""
    return(GS_EPOCH+datetime.timedelta(0,gs_tt))

def datetime_from_sim(sim_tt):
    """Converts a sim time-tag to datetime object"""
    return(SIM_EPOCH+datetime.timedelta(0,sim_tt))

def datetime_from_gps(gps_tt):
    """Converts a GPS time-tag to a datetime object"""
    return(GPS_EPOCH+datetime.timedelta(0,gps_tt))

def sim_from_datetime(dt):
    delta = dt - SIM_EPOCH
    return(delta.days*86400.0+delta.seconds+delta.microseconds/1000000.)

def gps_to_sim(gps_tt):
    delta = GPS_EPOCH - SIM_EPOCH
    return(gps_tt+delta.days*86400.0+delta.seconds)

def sim_to_gps(sim_tt):
    delta = SIM_EPOCH - GPS_EPOCH
    return(sim_tt+delta.days*86400.0+delta.seconds)

def to_code_b(dt):
    return(dt.strftime('%Y-%jT%H:%M:%S.%f')[:-3])

def from_code_b(code_b_string):
    return(datetime.datetime.strptime(code_b_string+'000','%Y-%jT%H:%M:%S.%f'))

def leap_seconds(dt):
    """
    Figures the number of leap seconds at the UTC input datetime
    From: http://www.nist.gov/pml/div688/grp50/leapsecond.cfm
    """
    valid_through = datetime.datetime(2014, 1, 1)

    if (dt-valid_through).days > 90:
        print "Check for updates: http://www.nist.gov/pml/div688/grp50/leapsecond.cfm"

    for ii in range(len(DT_LEAP))[::-1]:
        if dt >= DT_LEAP[ii]:
            return(ii)
  
    print>>sys.stderr, "Stop trying to call this function for times before %s" % \
                      to_code_b(dt_leap[0])
  
    return(0)
  
