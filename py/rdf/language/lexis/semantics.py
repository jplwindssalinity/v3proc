"""The nouns.NOUNS classes process Record or Comment lines. The class structure
may seem odd, and on it's own it is. It's structure is polymorphic to
the more complex Verb.VERB classes, which do much more.
"""
## \namespace rdf.language.lexis.semantics References to Things (Noun)
import abc
import operator

from rdf.language import lexis


## Nouns are rdf.language.lexis.Word objects that "concrete" as their
## sin_qua_non
class _Noun(lexis.Word):
    """The Noun IS something"""

    __metaclass__ = abc.ABCMeta

    ## a _Noun is Capitalized
    _namer = operator.methodcaller("capitalize")

    ## ID Noun type-- race hazard: don't send verbs to this...
    def is_(self, prosodic):
        """is_ is is_not..."""
        line, grammar = prosodic
        return not (
            (grammar.operator in grammar.comment(line)[0]) ==
            self._operator_in_line
            )

    ## Calling a Verb lats the agent act on the patient.
    ## \param line A complete RDF sentence (str)
    ## \param grammar An rdf.language.grammar.syntax.Grammar instance
    ## \return Whatever the noun's concrete method returns
    def __call__(self, prosodic):
        return self.concrete(prosodic)

    ## Calling a noun makes it's concrete person place or thing from \n
    ## line, according to grammar
    ## \param line A complete RDF sentence (str)
    ## \param grammar An rdf.language.grammar.syntax.Grammar instance
    ## \return N/A: this is an
    ## <a href=
    ##"http://docs.python.org/2/library/abc.html?highlight=abstractmethod#abc.abstractmethod">
    ## abstractmethod</a>
    @abc.abstractmethod
    def concrete(self, prosodic):
        """Abstract method must be over-ridden in concrete subclasses"""

    ## W/o a concrete rep, you're not a noun.
    sin_qua_non = concrete


## The Record Noun processes the basic input: An RDF line.
class Record(_Noun):
    """Records have '=' an concretely extract_record"""

    _operator_in_line = True

    ## act uses RDFField.__radd__ to build some form of an _RDFRecord \n
    ## (we don't know what that is here, no should we).
    @staticmethod
    def concrete(prosodic):
        """return a iterable with 1 RDFRecords"""
        return prosodic.extract_record()


## The Comment Noun remembers passive comment lines.
class Comment(_Noun):
    """Comments don't have '=' an concretely extract_comment"""
    _operator_in_line = False

    @staticmethod
    def concrete(prosodic):
        """Return an RDFComment"""
        return prosodic.extract_comment()


## Nouns
NOUNS = (Record, Comment)
