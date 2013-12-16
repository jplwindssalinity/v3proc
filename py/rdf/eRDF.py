"""eRdf Experimental RDF stuff- no warranty"""
## \namespace rdf.eRDF __e__ xperimental RDF objects


## A generic base class for RDF wrapped data structures -clients should
## use this when they have an object with RDF dependency injection and then
## further behavior as defined by the sub-classes methods.
class RDFWrapper(object):
    """RDFWrapper(rdf instance):

    is a base class for classes that wrap rdf instances.
    """
    ## Initialized with an RDF instance
    ## \param rdf_ a bona fide rdf.data.files.RDF object
    def __init__(self, rdf_):
        ## The wrapped rdf
        self._rdf = rdf_
        return None

    ## self.rdf == self.rdf() == self._rdf
    @property
    def rdf(self):
        return self._rdf

    ## Access rdf dictionary
    def __getitem__(self, key):
        return self._rdf.__getitem__(self, key)

    ## Access rdf dictionary
    def __setitem__(self, key, field):
        return self._rdf.__setitem__(self, key, field)

    ## Access rdf dictionary
    def __delitem__(self, key):
        return self._rdf.__delitem__(self, key)

    ## Access rdf dictionary
    def __len__(self, key):
        return len(self._rdf)


## Use to find problem lines when developing
def debug(src='rdf.txt', sleep=0.1):
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
    return NotImplemented
