#==============================================================#
# Copyright (C) 2013-2014, California Institute of Technology. #
# U.S. Government sponsorship acknowledged.                    #
#==============================================================#
__version__ = '$Id$'

"""
Utility stuff for files
"""

import os
import fnmatch

# From Belz and http://www.dabeaz.com/generators/Generators.pdf 
def find(directory, pattern, antipattern=''):
    """file finder"""
    for root, dirs, files in os.walk(directory):
        for basename in files:
            if fnmatch.fnmatch(basename, pattern) and not \
               fnmatch.fnmatch(basename, antipattern):  # guard
                filename = os.path.join(root, basename)
                yield filename
                continue
            continue
        continue
    pass
