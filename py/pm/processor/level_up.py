#!/usr/bin/env python
"""The RapidScat level processor:
Usage:

./level_up.py <src>

exit code is: level_up.level_up(src)"""
## \namespace pm.processor.level_up The Script

# python imports
import errno
import sys

# package imports
from pm.processor import processor
from pm.processor import errors


## create a pm.processor.Processor and run it, catching all UserWarning
## and passing them off as non-zero exit codes.
## @param src Input RDF file
## @retval status Shell script status
def level_up(src):
    """level_up(src) will execute

    status = pm.processor.processor(src).processor
    
    in the environment defined in src--that's how the processor get
    it input.

    Globals Variables are a sin,
    Enviroment Variables are the Ultimate Global,
    Therefore...."""
    ## try to set up AND run processor, catch ProcessorErrors
    try:
        # create processing instance
        processor_ = processor.processor(src)
        # run the procesor
        status = processor_()
    except errors.ProcessorError as err:
        # print User error message
        print >> sys.stderr, err
        # get errorcode
        status = int(err)
    else:
        # print error code If needed
        if status:  # raise or warn
            print "%s terminated: %d " % (str(processor_), status)
    return status


# Python script template
if __name__ == "__main__":
    # If no argument: print a usage message and exit
    if len(sys.argv) < 2:  # raise usage
        # print the usage string
        print sys.modules[__name__].__doc__
        status = errno.EIO
    else:
        # get the input rdf file name
        src = sys.argv[-1]
        # run the processor
        status = level_up(src)
    # exit shell script with status
    sys.exit(status)
