#!/usr/bin/env python
#pylint: disable-all
"""test/__init__.py --> test.py:

The purpose of this subpackage it to provide globla testing during
development:

You devlop, and then you do:

% ./test.py
%  xxdIff new.rdf old.rdf (this is now automatic)


% ./test --debug

if you want to see each step
"""
## \namespace rdf.test A brief test suite
import sys
import rdf

## Top of the nested desting chain
SRC = "rdf.txt"   # and 'uni.txt'
## File made by script
DST = "new.rdf"
## Result of a sucessfile run
STANDARD = "old.rdf"


## Run diff on tested files.
def diff():
    import subprocess
    try:
        result = subprocess.check_call(["diff", DST, STANDARD])
    except subprocess.CalledProcessError:
        import errno
        result = errno.EPERM
    return result


## non debug version.
def main_rdf():
    """RDF...(SRC) >> DST"""
    data = rdf.parse(SRC)
    dst = data >> DST
    return diff()


def main_debug():
    """RDF...(SRC) >> DST"""
    from rdf import eRDF
    data = eRDF.debug(SRC, sleep=0.1)
    dst = data.rdf() >> DST
    return diff()

if __name__ == '__main__':
    {False: main_rdf, True: main_debug}['d' in sys.argv[-1]]()
