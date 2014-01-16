"""name conventions"""
## \namespace pm.names.names File name conventions
import abc
import itertools

from pm import utils

## The default mission file tag ID.
MISSION_ID = "QS"

## @param year Year
## @param doy Day of Year
## @param hh Hour
## @param mm minute
## @retval process_time A ProcessTime instance
def process_time(year, doy, hh, mm):
    date_time = utils.doy2mmdd(year, doy, hh, mm)
    return ProcessTime(date_time)


## Instances emulate functions that return their string
class _StrTemplate(object):

    __metaclass__ = abc.ABCMeta

    ## template() --> str(template)
    def __call__(self):
        return str(self)

    pass


## For any file class that has a date_time dependency.
class ABCDateTimeFile(_StrTemplate):

    __metaclass__ = abc.ABCMeta

    ## Character to strip from full_doy_str() call.
    _truncate = -2

    ## @param date_time date_time instance (No Seconds).
    def __init__(self, date_time):
        ## datetime.datetime tag
        self.date_time = date_time
        return None

    ## Iterate over defining attributes (NO SEONCDS).
    def __iter__(self):
        yield self.year
        yield self.month
        yield self.doy
        yield self.hour
        yield self.minute


    ## Year String
    @property
    def year(self):
        return str(self.date_time.year).zfill(4)
    
    ## Month String
    @property
    def month(self):
        return str(self.date_time.month).zfill(2)
    
    ## Day String
    @property
    def day(self):
        return str(self.date_time.day).zfill(2)
    
    ## Equivalence to match datetime.datetime convention
    mday = day

    ## Hour String
    @property
    def hour(self):
        return str(self.date_time.hour).zfill(2)
    
    ## Minute String
    @property
    def minute(self):
        return str(self.date_time.minute).zfill(2)
    
    ## Second String
    @property
    def second(self):
        return str(self.date_time.second).zfill(2)
    
    ## Get the DoY string (yday)
    @property
    def doy(self):
        return str(self.date_time.timetuple().tm_yday).zfill(3)

    ## Previous day of year, for which Level0A (B) file may be named.
    def yesterday_of_year(self):
        return str(int(self.doy)-1).zfill(3)

    ## Equivalence to match datetime.datetime convention
    yday = doy

    def full_doy_str(self, truncate=None):
        """full_doy_str(truncate=None)

        Convert to yyyyDoYhhmmss[:truncate]

        string (e.g., truncate=-2 clips the seconds field)
        """
        result = self.year + self.doy + self.hour + self.minute + self.second
        return result[slice(None, truncate, None)]

    ## Get <a href="http://en.wikipedia.org/wiki/ISO_8601">
    ## ISO-8601 time stamp</a>
    def iso8601(self):
        return str(self.date_time)

    pass


class ABCDateTimeAndProcess(ABCDateTimeFile):
    """cls(data_time, process_time_)"""
    ## @param date_time 
    def __init__(self, date_time, process_time_):
        super(ABCDateTimeAndProcess, self).__init__(date_time)
        self.process_time = process_time_
        return None

        
## A class to handle procesing time tags on file.
class ProcessTime(ABCDateTimeFile):
#    def __init__(self, date_time):
#        super(ProcessTime, self).__init__(date_time)
#        return None

    def __str__(self):
        return self.full_doy_str(self._truncate)

#    def __iter__(self):
#        return itertools.chain(
#            super(self).__iter__(), (self.second,)
#            )


## It is the method that returns the file name
def test():
    import base
    return base.test().sci()
