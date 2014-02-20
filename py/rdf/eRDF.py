"""eRdf Experimental RDF stuff- no warranty"""
## \namespace rdf.eRDF __e__ xperimental RDF objects


## Use to find problem lines when developing
def debug(src='rdf.txt', sleep=0.1):
    """debug(src='rdf.txt', sleep=0.1)

    will step through your souce file and crash on the offending
    line, with a real error message."""
    import time
    from rdf import utils
    from rdf import iRDF
    lines = utils.unwrap_file(src)

    A = iRDF.RDFAnalyzer()
    B = iRDF.RDFAccumulator()

    for line in lines:
        print "INPUT: ", line
        print "OUTPUT: ", A(line)
        print B(line)
        time.sleep(sleep)
    return B


## Experimental function to factor keys and rdf in least form
def factor(rdf_):
    """Try to factor an rdf_ dict into AFFIX/ed stuff. This problem
    is np-hard, and this function doesn't work."""
    import numpy as np
    _k = rdf_.keys()
    _k.sort()
    k = _k[:]
    longest = max(map(len, k))
    m = np.zeros((len(k), 27), dtype=int)
    for jdx, key in enumerate(k):
        for idx, cc in enumerate(key):
            m[jdx, idx] = ord(cc)
    base = [2**__ for __ in map(long, range(len(m[0])))]
    return base, longest
