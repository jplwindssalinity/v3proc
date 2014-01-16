""" Name convention module"""
## \namespace pm.names.base Base (input) names based on date/time.

import abc
import datetime
import functools
import operator
import os

from pm import names


## Derived file decorator: method --> cls(self) --> derived instance.
def derived(cls):
    """derived(cls) converts method to compute new instace of cls"""
    ## decorator converts method so that it returns cls(self)
    def derived_decorator(method):
        ## Derived method takes only implicit 'self' input and sends it to cls
        def derived_method(self):
            ## No need to call the method, we have 'self' in the namespace
            return cls(self)
        ## compute the docstring from the output cls name.
        derived_method.__doc__ = """get %s instance""" % cls.__name__
        return derived_method
    ## return the decorator
    return derived_decorator


## Helper function to build BaseName object from primatives.
def basename(year, month, day, hour, minute, seconds, ground_station):
    """basename(year, month, day, hour, minute, seconds, ground_station)
    --> BaseName instance"""
    return BaseName(datetime.datetime(year, month, day, hour, minute, seconds),
                    ground_station)


## Derived File ABC handles concrete footer()
class _DerivedFile(names._StrTemplate):

    __metaclass__ = abc.ABCMeta

    ## Derived files are instantiaed from a string
    def __init__(self, base_name):
        ## The BaseName 
        self.base_name = base_name
        return None

    def __str__(self):
        return str(self.base_name) + self.footer()
    
    @classmethod
    ## trailer + extension, as define in concrete sub-classes.
    def footer(cls):
        return cls.trailer + os.extsep + cls.extension


    ## Allow circular access to properties and methods....
    def __getattr__(self, key):
        try:
            ## look for key in self.__class__
            return super(self.__class__, self).__getattribute__(key)
        except AttributeError:
            ## failed? then get it from the base_name attribute
            return getattr(self.base_name, key)
        pass
    pass


## ABC for files with a __dat__ extension
class _DatFile(_DerivedFile):
    """todo fix cicular header"""
    __metaclass__ = abc.ABCMeta
    extension = 'dat'
    
    ## __q__ uick __s__ cat __t__ elemetry (or rapid scat), \n
    ## depending on ::names.MISSION_ID
    header = names.MISSION_ID.lower() + 't'
    ## The mystery seperator
    p = 'p'


    ## Make an instance from a filename (todo: isn't this __repr__?)
    @classmethod
    def fromsrc(cls, src):
        src_di = src.rstrip(cls.footer()).lstrip(cls.header)
        left, right = src_di.split(cls.p)
        gds = int(right)
        year = int(left[0:4])
        month = int(left[4:6])
        day = int(left[6:8])
        hour = int(left[8:10])
        minute = int(left[10:12])
        seconds = int(left[12:14])
        return operator.methodcaller(cls.trailer)(
            basename(year, month, day, hour, minute, seconds, gds)
            )
        
    
## Science file template
class Sci(_DatFile):
    trailer = 'sci'

    
## House Keeping 1 file template
class HK1(_DatFile):
    trailer = 'hk1'
    
    
## House Keeping 2 file template
class HK2(_DatFile):
    trailer = 'hk2'


## Base Name Template from which all real file names are derived
class BaseName(names._DateTimeFile):
    """BaseName(date_time, ground_station)

    date_time is the datetime.datetime tag
    ground_station is the 'ground_station' tag"""

    ## __q__ uick __s__ cat __t__ elemetry (or rapid scat), \n
    ## depending on ::names.MISSION_ID
    header = names.MISSION_ID.lower() + 't'

    ## The mystery seperator
    p = 'p'

    def __init__(self, date_time, ground_station):
        ## Record date time portion
        super(BaseName, self).__init__(date_time)
        ## ground_station tag
        self.ground_station = str(ground_station).zfill(2)
        return None

    def __str__(self):
        return (
            self.header +
            self.year +
            self.month +
            self.day +
            self.hour +
            self.minute +
            self.second +
            self.p +
            self.ground_station
            )
    
    ## corresponding Sci instance
    @derived(Sci)
    def sci(self):
        pass

    ## corresponding HK1 instance
    @derived(HK1)
    def hk1(self):
        pass

    ## corresponding HK2 instance
    @derived(HK2)
    def hk2(self):
        pass


def test():
    return basename(2013, 1, 31, 6, 28, 7, 62)
