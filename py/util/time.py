#==============================================================#
# Copyright (C) 2013-2014, California Institute of Technology. #
# U.S. Government sponsorship acknowledged.                    #
#==============================================================#
__version__ = '$Revision$'

import pdb
import sys
import datetime
 
gps_epoch = datetime.datetime(1980,1,6)
sim_epoch = datetime.datetime(1970,1,1)
gs_epoch  = datetime.datetime(1993,1,1)
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


def tz_delta():
    return(datetime.datetime.utcfromtimestamp(0) - 
           datetime.datetime.fromtimestamp(0))

def date_time_from_gs( tt ):
    return(gs_epoch+datetime.timedelta(0,tt))

def date_time_from_sim( tt ):
    return(sim_epoch+datetime.timedelta(0,tt))

def date_time_from_gps( tt ):
    return(gps_epoch+datetime.timedelta(0,tt))

def sim_from_date_time(dt):
    delta = dt - sim_epoch
    return(delta.days*86400.0+delta.seconds+delta.microseconds/1000000.)

def gps_to_sim( gps_tt ):
    delta = gps_epoch - sim_epoch
    return(gps_tt+delta.days*86400.0+delta.seconds)

def sim_to_gps( sim_tt ):
    delta = sim_epoch - gps_epoch
    return(sim_tt+delta.days*86400.0+delta.seconds)

def ToCodeB( dt ):
    return(dt.strftime('%Y-%jT%H:%M:%S.%f')[:-3])

def FromCodeB(code_b_string):
    return(datetime.datetime.strptime(code_b_string+'000','%Y-%jT%H:%M:%S.%f'))

def leap_seconds( dt ):
    """From: http://www.nist.gov/pml/div688/grp50/leapsecond.cfm"""
    valid_through = datetime.datetime(2014,1,1)

    if (dt-valid_through).days > 90:
        print "Check for updates: http://www.nist.gov/pml/div688/grp50/leapsecond.cfm"

    for ii in range(len(DT_LEAP))[::-1]:
        if dt >= DT_LEAP[ii]:
            return(ii)
  
    print>>sys.stderr, "Stop trying to call this function for times before %s" % \
                      ToCodeB( dt_leap[0] )
  
    return(0)
  
