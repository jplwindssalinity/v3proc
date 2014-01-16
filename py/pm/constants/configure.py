## \namespace pm.constants.configure RDF Configuration files
"""Configuration Constants"""
import os
import rdf

from pm.utils import rdf_helper

## The official name and order of levels
LEVELS = ('0', '1A', '1B', '2A', '2B') # '1Bv3')


## Level 0 preprocessor inputs file name
_L00_RDF_FILENAME = 'level_00.rdf'
## Level 1A processor inputs file name
_L1A_RDF_FILENAME = 'level_1a.rdf'
## Level 1B processor inputs file name
_L1B_RDF_FILENAME = 'level_1b.rdf'
## Level 2A processor inputs file name
_L2A_RDF_FILENAME = 'level_2a.rdf'
## Level 2B processor inputs file name
_L2B_RDF_FILENAME = 'level_2b.rdf'


## Level 0 preprocessor inputs (RDF)
L00 = rdf_helper.forcerdf(_L00_RDF_FILENAME)
## Level 1A processor inputs (RDF)
L1A = rdf_helper.forcerdf(_L1A_RDF_FILENAME)
## Level 1B processor inputs (RDF)
L1B = rdf_helper.forcerdf(_L1B_RDF_FILENAME)
## Level 2A processor inputs (RDF)
L2A = rdf_helper.forcerdf(_L2A_RDF_FILENAME)
## Level 2B processor inputs (RDF)
L2B =rdf_helper.forcerdf(_L2B_RDF_FILENAME)
