## \namespace pm.utils.rdf_helper RDF helpers
"""helper/ module has python rdf_helper functions"""
import os

import rdf

from pm.constants import paths

# TODO : ensure casting to str for ALL OUTPUTS. Deal w/ purge at last second?

## A special RDF class tailored for injecting environments into processors
class PMRDF_COMPLICATED_WORSE_THAN_COMPLEX(rdf.RDF):
    """A special RDF sub just for PM"""

    ## variables to purge from the environment
    purge = ('EXIT', 'LEVEL')

    ## Hardcode extension of staticmethod to build THIS class
    ## from RDF's machinery
    ## @param src RDF input file
    ## @retval pmrdf A PMRDF instance
    @staticmethod
    def fromfile(src):
        return PMRDF.fromdict(rdf.RDF.fromfile(src).todict())

    ## Extend RDF.todict method to work just for PM.
    ## @param None
    ## @retval dict_ A purged environment dictionary
    def todenv(self):
        result = self.todict(str)
        # purge unnecessary values
        for key in resul:
            try:
                result.pop(key)
            except KeyError:
                pass
            continue
        return result
    
    ## Force all gets to be a string
    def __getitem__(self, key):
        return str(super(PMRDF, self).__getitem__(key))


## Read src from paths.CONFIG or return empty RDF.
## @param src A possible src file, PATH WILL BE FORCED
## @reval result a path-fixed PMRDF instance
def forcerdf(src):
    full_src = os.path.join(paths.CONFIG, src)
    ## parse is an interactive function, so avoid that:
    if os.path.exists(full_src):  # guard
        result = PMRDF.fromfile(full_src)
    else:
        print 'stubbing: ', full_src
        result = rdf.RDF()
        pass
    return result

class PMRDF(rdf.RDF):
    pass


## Do not force file paths.
parse = PMRDF.fromfile
