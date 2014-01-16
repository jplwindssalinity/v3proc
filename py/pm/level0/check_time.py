"""Usage:

is_go(rdf)

for a level0 rdf, which has the filespecs for sci.dat and time conversion
"""

## \namespace pm.level0.check_time Check if Sci-file is in time conversion.

import functools
import os

from pm.constants import inputs
from pm.utils import helper

from pm.level0 import INPUT

## RDF key to time conversion file
KEY = inputs.TIME_CONVERSION

## Default time conversion file (local for debug)
TIME_CONVERSION_FILE = "QS_SCLK_UTC"

## remove ';' in pm.utils.helper.clean_line
clean_line = functools.partial(helper.clean_line, remove=(';',))


## Strip header lines from a readlines() list
## @param lines a ::clean_line treated file.readlines() list.
## @retval list list of actual lines.
def strip_header(lines):
    header_size = get_header_size(lines[0])
    return lines[header_size:]


## @param line A header line
## @param  key The key to the value
## @retval str  The string value
## @throws ValueError If key not in line
def get_header_value(line, key):
    if key not in line:
        raise ValueError("Lines=[%s] does not have key=[%]" %
                         (line, key))
    return line.split("=")[-1]


## @param line The 1st line in header
## @retval int The number of header lines
def get_header_size(line):
    return int(get_header_value(line, 'num_header_records'))


## @param line A normal ::clean_line from the ::TIME_CONVERSION_FILE
## @retval str The last word (str) in the line.
def extract_filename(line):
    return line.split()[-1]


## Read/Parse ::TIME_CONVERSION_FILE
## @param src The Source File
## @retval list of file names in the file
def tc_reader(src=TIME_CONVERSION_FILE):
    with open(src, 'r') as fsrc:
        lines = map(clean_line, fsrc.readlines())
        pass

    N = get_header_size(lines[0])
    return map(extract_filename, lines[N:])


## @param scidat The [path/]name of scidat file
## @param src The time conversion file spec.
## @retval bool scidat is in src
def scidat_in_time_conversion(scidat, src):
    print 'checking vs:', src
    return os.path.basename(scidat) in tc_reader(src=src)


## THE function, takes the level0 RDF input
def is_go(rdf):
    return scidat_in_time_conversion(rdf[INPUT], rdf[KEY])


def test():
    import injector
    rdf =  injector.test()
    return is_go(rdf)

## This is the ONLY non-private
__all__ = ('is_go',)
