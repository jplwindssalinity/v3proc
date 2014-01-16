## \namespace pm.utils.times Help with datetime needs
"""Fucntion and mixins for dealing with datetimes as strings, times,
and intervals."""
import collections
import datetime
import operator
import time

from pm.utils import interval

now = datetime.datetime.now

ZERO = datetime.timedelta()
DAY = datetime.timedelta(1)
SECOND = datetime.timedelta(0, 1)
HOUR = SECOND*3600

def seconds(delta):
    return (86400 * delta.days +
            delta.seconds +
            operator.truediv(delta.microseconds, 1000*2))

## Promote a date to a datetime
## @param date a date or datetime
## @param delta = ::ZERO optional delta
## @returns datetime object
def date2time(date):
    try:
        date.microsecond
    except AttributeError:
        return datetime.datetime(date.year, date.month, date.day)
    return date


## String Conversion: \n "YYYYMMDDhhmmss" --> datetime
## @param string as YYYYMMDD<hhmmss> format: MANDATORY<optional>
## @retval datetime datetime instance
def yyyymmddhhmmss2datetime(string):
    """ "YYYYMMDDhhmmss" --> datetime"""
    yyyy = string[0:4]
    month = string[4:6] 
    dd = string[6:8]
    hh = string[8:10] if len(string) >= 10 else "0"
    mm = string[10:12] if len(string) >= 12 else "0"
    ss = string[12:14] if len(string) >= 14 else "0"
    yyyy, month, dd, hh, mm, ss = map(int, (yyyy, month, dd, hh, mm, ss))
    return datetime.datetime(yyyy, month, dd, hh, mm, ss)

## Format Conversion: \n "YYYYdoy(hhmmss)" --> datetime
## @param yyyy Year (must support int() )
## @param doy __D__ay __o__f __Y__ear (must support int() )
## @param hh=0 hour
## @param mm=0 minute
## @param ss=0 second
## @retval datetime datetime instance
def doy2mmdd(yyyy, doy, hh=0, mm=0, ss=0):
    """doy2mmdd(yyyy, doy, hh=0, mm=0, ss=0) --> datetime"""
    yyyy, doy, hh, mm, ss = map(int, (yyyy, doy, hh, mm, ss))
    return (
        datetime.datetime(yyyy, 1, 1, hh, mm, ss) +
        datetime.timedelta(int(doy)-1)
        )


    
def test():
    x = date2time(datetime.date(2014,1,1))
    I = interval.TimeInterval

    i0 = I(x+0*HOUR, x+3*HOUR)               # [0  3]
    i1 = I(x+1*HOUR, x+4*HOUR)               # [1  4]
    i2 = I(x+2*HOUR, x+5*HOUR)               # [2  5]
    i3 = I(x+3*HOUR, x+6*HOUR)               # [3  6]
    i4 = I(x+4*HOUR, x+7*HOUR)               # [4  7]
    


    return i0,i1,i2,i3,i4

    

