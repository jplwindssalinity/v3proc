## \namespace pm.level0.watcher Level 0 pm process watchers
"""The level-0 hot dir watcher"""

# python imports
import logging
import time
import datetime
import glob
import fnmatch
import os
import sys

# package imports
from pm.utils import helper


## Todo: refactor to configure.py
HOT_DIR = '/home/belz/stage/L00/HOT_DIR'

WAIT = 10

## File pattern to match: TODO: refactor from constants/names.
GLOB = "*sci.dat"

## Logging level
LEVEL = logging.DEBUG


# Import Time Error Checks w/ warning to stderr.
if not os.path.exists(HOT_DIR):
    print >> sys.stderr, "HOT_DIR=%s  Does NOT exist." % HOT_DIR
else:
    if not os.path.isdir(HOT_DIR):
        print >> sys.stderr, "HOT_DIR=%s  is NOT a directory." % HOT_DIR


## Function to get a file's ctime.
## @param x A file spec.
## @retval datetime The datetime of src's ctime
ctime = helper.chain(datetime.datetime.fromtimestamp, os.path.getctime)
        
#def ctime(src):
#    return datetime.datetime.fromtimestamp(os.getctime(src))


## Non-business mixin for logging functions.
class _LogMixin(object):
        
    logger = None

    def _create_log(self):
        if LEVEL:
            self.logger = logging.basicConfig(filename='obs00.log',
                                              level=LEVEL)    
            
    def _log(self, func, msg):
        if LEVEL:
            func(str(datetime.datetime.now()) + " Level0 watch loop: "+ msg)

    ## todo- refactor all __iter__ logging into this (but how?)
    def _log_iter(self):
        self._create_log()
        self._log(logging.info, 'entered:')


## Watch ::HOT_DIR for files matching ::GLOB, and yield them while
## updating watch list unti it's over
class Watcher(_LogMixin):
    """watcher = Watcher(since=datetime.datetime.now(), *grandfather)
    
    for src in watcher:
    ...process(src)

    """
    
    ## @param since datetime object that cut's search
    def __init__(self, since=datetime.datetime.now(), *grandfather):
        self.since = since
        self.subjects = list(grandfather)
        pass
        
    ## @retval generator Yields files, for ever.
    ## @throws StopIteration Only on KeyboardInterrupt.
    def __iter__(self):
        super(Watcher, self)._log_iter()
        while True:
            print 'in loop'
            result = next(self)
            print 'next=', result
            if result is None:
                try:
                    self._log(logging.warning, 'wating:')
                    time.sleep(WAIT)
                except KeyboardInterrupt:
                    break
                continue
            else:
                self._log(logging.info,
                          'yielding %s :' % result)
                yield result
                continue
            continue
        self._log(logging.warning, 'Manual exit:')
        raise StopIteration
                
    def __len__(self):
        return len(self.subjects)

    ## Reboot subjects list
    def __pos__(self):
        # extend with any new files
        self.subjects.extend(self.get_subjects())
        # sort by date
        self.subjects.sort(key=ctime)
        return self


    ## @returns item filename
    ## @throws OSError for badly grndfathered files
    ## Side Effects: Watcher.subjects is popped, Watcher.since is modified.
    def __neg__(self):
        # pop earliest item of the list
        item = self.subjects.pop(0)
        # update since to item's time.
        self << item
        # return item
        return item

    ## Procedure: update Watcher.since to ctime() of item.
    ## @param item A file name
    ## @retval None Side Effects: changes Watcher.since 
    def __lshift__(self, item):
        self.since = ctime(item)

    ## @return item A file, or None
    def next(self):
        # update the list of files
        +self 
        if not len(self):  # guard on no file found
            return None
        # otherwise, pop() and return
        item = -self
        return item

    # ::GLOB the ::HOT_DIR and find matching files\n
    def get_subjects(self):
        result = []
        for item in glob.glob(os.path.join(HOT_DIR, GLOB)):
            if ctime(item) > self.since and item not in self.subjects:
                result.append(item)
                pass
            continue
        return result
    

def test():
    return Watcher(datetime.datetime(2014, 1, 1))
    
