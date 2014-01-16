## \namespace pm.processor.errors Special scripting errors
"""Error module defines UserWarnings that can be raise in python code,
caught, and then converted to system exit codes for shell script wrappers.
"""

import errno


## Sub class for processor errors
class ProcessorError(UserWarning):
    pass

## Raised when User doesn't pick a level processor
class NoLevel(ProcessorError):
    @classmethod
    def __int__(cls):
        return errno.EIO

    
## Raised when User's processor is not in pm.processor.FACTORY
class UnknownLevel(ProcessorError):
    @classmethod
    def __int__(cls):
        return errno.EIO

    
## Raised when pm.processor.Processor.processor doesn't exist
class NoProcessor(ProcessorError):
    @classmethod
    def __int__(cls):
        return errno.ENOENT

    
## Raised when pm.processor.Processor.processor can't be executed
class ProcessorCannotProcess(ProcessorError):
    @classmethod
    def __int__(cls):
        return errno.EACCES
    

## Hash table of OS Errors that can occurring when starting the processor    
## maps error number to speciFic ProcessorError:\n
## ENOENT:  The processor file does not exist   \n
## EACCES:  The processor file could not be executed (permission).
OSERRORS = {errno.ENOENT: NoProcessor,
            errno.EACCES: ProcessorCannotProcess}
