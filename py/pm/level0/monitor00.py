#!/usr/bin/env python
"""Usage:

./run.py {2014-01-14T19:32}

Starts an infilte loop that looks in watcher.HOT_DIR for new science
telemetry occurring after the optional date (defaults to
datetime.datetime.now()

"""
# python imports
import os
import datetime
import time

# package imports 
from pm.utils import time_funcs
from pm.processor import processor

# sub-package imports
import watcher
import check_time
import injector


WAIT = 10.

## @param scidat A science telemetry filename
## @param wait ::WAIT -delay between input checks.
## @retval rdf_data A DI'd rdf object for the processor
def verify_inputs(scidat, wait=WAIT):
    # create level 0 rdf environemnt
    rdf_data = injector.inject(scidat)  # get's real L00.
    # wait for time conversion to exist
    while not check_time.is_go(rdf_data):
        print 'Waiting for TIME CONVERSION file update'
        try:
            time.sleep(wait)
        except KeyboardInterrupt:  # break during debugging
            break
        continue
    return rdf_data

## @param scidat A science telemetry filename
## @retval error_code The processoring error_code
def spawn(scidat):
    print 'spawing:', scidat
    # verify and create inputs rdf data
    rdf_data = verify_inputs(scidat)
    # create the level processor
    level_processor = processor.fromrdf(rdf_data)
    # spawn the level proceesor
    error_code = level_processor()  # run the level processor
    # return the error code?
    return error_code

## Implement with pm.observer.generic.Observer
def refactor(scidat):
    rdf_data = injector.inject(rdf_data)
    from pm.observer import generic
    sub = generic.Subject(check_time.is_go, rdf_data)
    act = generic.Action(processor.fromrdf(rdf_data))
    obs = gerneric.Observer(sub, act, wait=wait, time_out=3600)
    obs()
    

## Procedure: \n
## Start the watcher.Watcher loop \n
## and spawn() all files found
## @param since A STIRNG since.
def main(since):
    print 'Starting Watcher with cut-off:', since
    print '               Loop Delay (s):', watcher.WAIT
    watch = watcher.Watcher(since=since)
    for item in watch:
        print 'found:', item
        spawn(item)
        

if __name__ == '__main__':
    import sys
    # parse input string
    value = sys.argv[-1]
    if '.py' in value:
        since = datetime.datetime.now()
    else:
        raise NotImplementedError('datetime arg is not implemented')
    
    since = datetime.datetime.now()
    main(since)
        
        
