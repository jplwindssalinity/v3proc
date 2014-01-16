"""The Processor"""
## \namespace pm.observer.generic Generic Observer
import time

from pm import utils

DEBUG = True

class ObserverTimeOut(StopIteration):
    pass


## Function wrapper (a function with state)
class _FWrap(object):
    
    ## @param func A callable object
    ## @param *args  Arguments to func
    ## @param **kwargs Keywords for func
    def __init__(self, func, *args, **kwargs):
        self.func = func
        self.args = args
        self.kwargs = kwargs
        return None

## Observation Condition
class Subject(_FWrap):    
    """Subject(func, *args, **kwargs)"""

    ## True If func(*args, **kwargs)
    ## @retval bolo _FWwrap.func called with _FWrap.args _Fwrap.kwargs
    def __nonzero__(self):
        return bool(self.func(*self.args, **self.kwargs))

## Post Observation Action
class Action(_FWrap):
    """Action(func, *args, **kwargs)"""

    ## @retval unknown  _FWwrap.func called with _FWrap.args _Fwrap.kwargs
    def __call__(self):
        return self.func(*self.args, **self.kwargs)
    


## Watch for Subject and the do Action
class Observer(object):
    """Observer(subject, action, wait=1., time_out=20)

    observer check subject every wait (up to time_out).
    If subject: Action
    """

    ## @param subject a Subject
    ## @param action an Action
    ## @param wait waiting time between queries
    ## @param time_out maximum time on station
    @utils.type_check(Subject, Action)
    def __init__(self, subject, action, wait=1., time_out=20):
        self.subject = subject
        self.action = action
        self.wait = wait
        self.time_out = time_out
        return None
    
    ## Watch Observer.subject until True, or time out.
    ## @param None
    ## @retval bool is bool(Observer.subject)
    def watch(self):
        for n in xrange(int(self.time_out//self.wait)):
            print 'observing:', n 
            ###################
            if DEBUG and n==10:
                with open('tmp.dat', 'w') as fdst:
                    fdst.write('tmp.dat exisits\n')
                    pass
                pass
            ####################
            if self.subject:
                break  # sucess, break-out
            time.sleep(self.wait) # wait
            continue
        else:
            # loop finished, subject never went True
            return False
        # subject went True
        return True

    ## @param None
    ## @retval See watch()
    ## Side Effects: This may call Oberserver.action
    def __call__(self):
        result =  self.watch()
        if result:
            print 'Found'
            result = self.action()
        else:
            print 'Time Out'
            pass
        return result






## REdo as an Itertor
class IObserver(object):
    """IObserver(subject, action, wait=1., time_out=20)

    """

    count = 0
    
    ## @param subject a Subject
    ## @param action an Action
    ## @param wait waiting time between queries
    ## @param time_out maximum time on station
    @utils.type_check(Subject, Action)
    def __init__(self, subject, action, wait=1., time_out=20):
        self.subject = subject
        self.action = action
        self.wait = wait
        self.time_out = time_out
        self.max_count = max(1, int(time_out//self.wait))
        return None
    
    def __len__(self):
        return self.max_count

    def __int__(self):
        return self.count

    def __iadd__(self, n):
        self.count += 1
        return self

    def __nonzero__(self):
        return bool(self.subject)

    ## NO - reurn iterator 
    def __iter__(self):
        while int(self) < len(self):
            if bool(self):
                break
            self.sleep()
            yield None
        else:
            raise ObserverTimeOut("Time Out")
        yield self()

    def __call__(self):
        print 'taking:', self.action
        return self.action()


    def sleep(self):
        print 'waiting....', len(self) - int(self)
        time.sleep(self.wait)
        self += 1

import os

sub = Subject(os.path.exists, 'tmp.dat')
act = Action(os.system, 'more tmp.dat')

def test():
#    o = Observer(sub, act)
    o = IObserver(sub, act)
    return o

def test2():
    sub = Subject(check_time.is_go, rdf_data)
    act = Action(processor.fromrdf(rdf_data))
    return Observer(sub, act)
                 
