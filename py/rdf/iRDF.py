"""iRdf are expert-usage interactive tools:

------------------------
Analyzer
Accumulator

"""
import abc
from rdf.language.grammar.syntax import Grammar
from rdf.data import RDF, RDFField, RDFRecord
from rdf.units import SI

## \namespace rdf.iRDF __i__ nteractive RDF tools for 'perts.

## The RDF Toybox.
__all__ = ('Grammar', 'RDF', 'RDFField', 'RDFRecord', 'RDFAccumulator',
           'RDFAnalyzer', 'SI', 'rdf_list')


## A list of rdf records that can filter itself and make an RDF
class rdf_list(list):
    """see the list constructor.

    Adds method rdf() to make it into an rdf
    """
    @property
    ## Convert list to an RDF instance: filter out comments
    ## and pass to RDF constructor
    ## \return RDF instance from filter(bool, self)
    def rdf(self):
        """Convert list's contents into an RDF instance"""
        from rdf.data.files import RDF
        # filter out comments and send over to RDF.
        return RDF(*filter(bool, self))


## Base class
class _RDFAccess(object):
    """Base class for RDFAnalyzer and RDFAccumulator"""

    __metaclass__ = abc.ABCMeta

    ## New instances get a private new rdf.language.grammar.syntax.Grammar
    ## instance
    def __init__(self):
        self._grammar = Grammar()

   ## Getter for grammar
    @property
    def grammar(self):
        return self._grammar

    ## Protect the language!
    @grammar.setter
    def grammar(self):
        raise TypeError("Cannot change grammar (like this)")

    ## Just call the grammar
    ## \param line An full rdf line
    ## \returns Grammar's interpretation of line
    def __call__(self, line):
        return self.grammar(line)


## RDFAnalyzer is created with an rdf.language.syntax.Grammar object and
## then emulates a function that converts single line inputs into
## a pre-RDF output -note: it is overly complicated, and its sole purpose
## is to provide an interactive RDF reader.
class RDFAnalyzer(_RDFAccess):
    """a = RDFAnalyzer()

    creates an RDFAnalyzer with 'fresh' Grammar. __call__ then runs it:

    >>>a(line) --> RDFRecord.
    """
    ## FUNCTION EMULATION: The instance processes RDF lines sequentially
    ## \param line  A complete (or incomplete) rdf sentence.
    ## \retval rdf_list An rdf_list of rdf.data.entries objects...
    def __call__(self, line):
        """self(line) --> processes the line and updates the grammar

        wrapped lines are OK.
        """
        ## Deal with wraps:
        while line.strip().endswith(self.grammar.wrap):
            line = line.strip().rstrip(self.grammar.wrap) + raw_input('...')
        ## Process the line and unpack all the results.
        result = super(RDFAnalyzer, self).__call__(line)
        return rdf_list([item for item in result] if result else [result])


## A TBD rdf accumulator - prolly slower than RDFAnalyzer as it rebuild
## dictionary all the time-- see RDF.__iadd__
class RDFAccumulator(_RDFAccess):
    """a = RDFAccumulator()

    creates and accumulator, who's __call__ eats rdf lines and appends their
    results to 'record_list', which is an rdf_list.

    rdf() method calls rdf_list.rdf().
    """

    ## The following are equivalent:          \n\N
    ## >> >RDFAccumulator.fromfile(src).rdf()     \n
    ## >> >rdf.rdfparse(src)                   \n\n
    # \param src RDF source file
    # \retval accumulator And RDFAccumulator instance (full of src).
    @classmethod
    def fromfile(cls, src):
        """instantiate from src"""
        accumulator = RDFAccumulator()
        accumulator("INCLUDE = %s" % src)
        return accumulator

    ## There are no inputs, but some private static attributes are created
    ## \param None There are no inputs allowed.
    def __init__(self):
        # call super
        super(RDFAccumulator, self).__init__()
        ## Remember the list (as an rdf_list) -starts empty.
        self.record_list = rdf_list()
        return None

    ## Call rdf.language.grammar.syntax.Grammar() and remember
    # \param line Any type of rdf string, including continued.
    # \returns <a href=
    # "http://docs.python.org/2/library/constants.html?highlight=none#None">
    # None</a>
    def __call__(self, line):
        """rdf sentence goes in"""
        self.record_list.extend(super(RDFAccumulator, self).__call__(line))

    @property
    ## Unpack RDFRecord elements into RDF
    def rdf(self):
        """see rdf_list.rdf()"""
        return self.record_list.rdf()


def test():
    accum = RDFAccumulator()
    accum("INCLUDE = rdf.txt")
    return accum
