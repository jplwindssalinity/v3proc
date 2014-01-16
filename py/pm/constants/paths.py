## \namespace pm.constants.paths Constant Paths
"""Define full paths to:

BIN:      Processor Binaries
CONFIG:   Default RDF Configuration files
"""
import os
import runtime

## partial path to configuration rdf files
_CONFIG = 'python/pm/constants/inputs'

## partial path to binaries:
_BIN = 'new_pmachines/bin'


## full path to rdf configuration
CONFIG = os.path.join(runtime.HOME, _CONFIG)

## full path to processor binaries
BIN = os.path.join(runtime.HOME, _BIN)
