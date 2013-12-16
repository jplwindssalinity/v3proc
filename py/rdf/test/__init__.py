#!/usr/bin/env python
"""test/__init__.py --> test.py:

The purpose of this subpackage it to provide globla testing during
development:

You devlop, and then you do:

% ./test.py
%  xxdIff new.rdf old.rdf
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


## rdf.parse(SRC) >> DST
def main():
    """RDF...(SRC) >> DST"""
    data = rdf.parse(SRC)
    dst = data >> DST
    return diff()

if __name__ == '__main__':
    main()
