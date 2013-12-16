"""RDF Unit Exceptions"""
## \namespace rdf.units.errors RDF Unit Errors
import sys
from rdf.language.errors import RDFError, RDFWarning


## Fatal error for unknown units (not really an acceptable idea)
class FatalUnitError(RDFError):
    """raise for unregocnized units (fatally)"""


## RDF Error for a unit problem (not sure what kind of error this is)
class UnitsError(RDFWarning):
    """Raised for a non-existent unit"""


## Error for input w/ unknown units.
class UnrecognizedUnitWarning(RDFWarning):
    """Unrecognized unit (ignored)"""

    ## Memoize warnings
    warnings = []

    ## Memoize in init
    def __init__(self, *args):
        """UnrecognizedUnitWarning(units or None)"""
        units = str(args[0]) if len(args) else None
        ## If it has not been memoized, then warn and memoize/
        if units and units not in type(self).warnings:
            print >> sys.stderr, "UnrecognizedUnitWarning: %s" % units
            self.__class__.warnings.append(units)
        super(UnrecognizedUnitWarning, self).__init__(*args)


## Warning when you overwrite a unit
class RedefiningUnitWarning(UnitsError):
    """Redefining Units"""
