""" Name convention module"""
## \namespace pm.names.aux GPSEPHM/ATT file protocols

import abc
import datetime
import functools
import itertools
import os

from pm.names import names
from pm import utils


def helper(cls, *args):
    year, doy, hh, mm, process_time_ = args
    return cls(utils.doy2mmdd(year, doy, hh, mm),
               process_time_)


## @param year Year
## @param doy Day of Year
## @param hh Hour
## @param mm minute
## @param process_time_ A names.ProcessTime instance.
## @retval GPSEphm a GPSEphm instance
def gpsephm(year, doy, hh, mm, process_time_):
    """gpsephm(year, doy, hh, m, process_time_)"""
    return helper(GPSephm, year, doy, hh, mm, process_time_)

def att(year, doy, hh, mm, process_time_):
    return helper(ATT, year, doy, hh, mm, process_time_)


## For understanding GPS/Ephemeris files
class ABCAux(names.ABCDateTimeAndProcess):
    """Use gpsephm helper"""


    ## Get unique identiFier
    def id(self):
        return (self.full_doy_str(self._truncate) +
                os.extsep +
                str(self.process_time))

    def __str__(self):
        return self.HEADER + self.id()


class GPSephm(ABCAux):
    ## gpd empheris header string
    HEADER = names.MISSION_ID.upper() + "_SEPHG"

class ATT(ABCAux):
    HEADER = names.MISSION_ID.upper() + "ATT"
        
def test():
    return gpsephm(2013, 12, 11, 10,
                   names.process_time(2014, 15, 16, 17))
