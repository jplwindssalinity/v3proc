"""Level Processor Classes and helper functions.

Level0
Level1A
Level1B
Level2A
Level2B
Level1Bv3 TBD
Level2Av3 TBD
Level2Bv3 TBD...

processor  <-- process from a src file
process_rdf <--- process from a PMRDF instance.
"""
## \namespace pm.processor.processor Processor Classes

# python imports
import abc
import os
import subprocess

# semi-3rd party imports
from pm.utils import rdf_helper

# package import
from pm.constants import configure
from pm.constants import paths

# sub-package  imports
from pm.processor import errors


## Class decorator to resolve processor's absolute path
## @param cls A concrete LevelProcessor subclass.
## @retval cls with a fully concrete LevelProcessor.processor path
def resolve(cls):
    """resolve(cls)
    
    adds the full path to cls's cls.processor attribute
    """
    # update the processor attribute with runtime path to binaries
    cls.processor = os.path.join(paths.BIN, cls.processor)
    return cls


## ABC for Level Processors
class LevelProcessor(object):
    """The purpose of this class is: run the right processor with the
    correct environment."""
    
    __metaclass__ = abc.ABCMeta

    ## Absolute Path to the Executable Level Processor
    @abc.abstractproperty
    def processor(self):
        """Concrete processors need an processor"""

    ## Instantiate with a processing environment
    ## @param env The environment for the processing
    def __init__(self, env=None):
        """Use processor helper function to instantiate"""
        ## Environment dictionary
        self.env = env or {}

    ## Spawn the executable with the environment string
    ## @param None
    ## @retval status The subprocess's error code.
    ## @throws errors.OSERRORS A Dx'd error
    ## @throws OSError and un-Dx'd error w.r.t IO.
    def __call__(self):
        try:
            print "calling: %s, with: " % str(self)
            # print the environment to the screen
            for item in self.env.iteritems():
                print "%s = %s" % item
            status = subprocess.check_call(str(self), env=self.env, shell=True)
        except OSError as err:
            try:
                # make the message based on the OSError's errno:
                msg = ("processor %s error" %
                       {2: "existence", 13: "permission"}[err.errno])
                raise errors.OSERRORS[err.errno](msg)
            except KeyError:
                raise err("Undiagonsed OS error")
        except subprocess.CalledProcessError as err:
            status = err.returncode
        return status

    ## @retval processor The executable string
    def __str__(self):
        return str(self.processor)
    
    
## Level 0 Preprocessor
@resolve
class Level0(LevelProcessor):
    processor = "qs_pp_L0"


## Level 1A Processor
@resolve
class Level1A(LevelProcessor):
    processor = "qs_lp_L1A"


## Level 1B Processor
@resolve
class Level1B(LevelProcessor):
    processor = "qs_lp_L1B"


## Level 2A Processor
@resolve
class Level2A(LevelProcessor):
    processor = "qs_lp_L2A"


## Level 2B Processor
@resolve
class Level2B(LevelProcessor):
    processor = "qs_lp_L2B"


    
## A factory hash-table to choose a LevelProcessor based on level keyword.
FACTORY = {level: cls for level, cls in zip(configure.LEVELS,
                                            (Level0,
                                             Level1A, Level1B,
                                             Level2A, Level2B))}


## Parse source file and make a LevelProcessor instance
## @param src  Input RDF file
## @retval result A fully instaniated Processor
def processor(src):
    """processor = processor(src)

    src        An rdf file with processing information
    processor  The appropriate LevelProcessor instance for the job
    """
    # read rdf data from src file
    rdf_data = rdf_helper.parse(src)
    result = fromrdf(rdf_data)
    return result


## @param rdf_data pm.utils.rdf_helper.PMRDF data.
## @retval result A fully instaniated Processor
## @throws errors.NoLevel error
## @throws errors.UnknownLevel for a bad rdf level.
def fromrdf(rdf_data):
    # look for level keyword
    try:
        # cast level to a string.
        level = str(rdf_data["LEVEL"])
    except KeyError:
        raise errors.NoLevel("LEVEL keyword is not set in rdf source file")
    # find matching processor class
    try:
        cls = FACTORY[level]
    except KeyError:
        raise errors.UnknownLevel("LEVEL = %s is not recognized" % level)
    # return a processing instance
    return cls(rdf_data.todict())
