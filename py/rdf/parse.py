#!/usr/bin/env python
"""Usage:

[python] ./parse.py src [dst]
"""

## \namespace rdf.parse RDF Parsing script
import sys
from rdf import rdfparse

# RUN AS AS SCRIPT
if __name__ == "__main__":

    # IF usage error, prepare error message and pipe->stderr,
    #                set exit=INVALID INPUT
    if len(sys.argv) == 1:  # guard
        import errno
        pipe = sys.stderr
        message = getattr(sys.modules[__name__], '__doc__')
        EXIT = errno.EINVAL
    # ELSE: Usage OK- the message is the result, and the pipe us stdout
    #                 set exit=0.
    else:
        if sys.argv[0].startswith('python'):
            argv = sys.argv[1:]
        else:
            argv = sys.argv[:]
        src = argv[-1]
        pipe = sys.stdout
        message = str(rdfparse(src))
        EXIT = 0

    # Send message to pipe.
    print >> pipe, message
    # exit script
    sys.exit(EXIT)
# ELSE: I You cannot import this module b/c I say so.
else:
    raise ImportError("This is a script, and only a script")
