"""Grammar/Language interactions live here in the form of
the Prosodic.

The Prosodic comprises a clitic (a sentence or line) that is interpreted
in the context of host (grammar).

Their coupling occurs in the Prosodic class.
"""
## \namespace rdf.language.prosodic Prosidics define a
## <a href="http://en.wikipedia.org/wiki/Dependency_grammar">
## Dependency Grammar</a>.
import collections
try:
    import importlib
except ImportError:
    _27 = False
else:
    _27 = True

from rdf.language.grammar import punctuation


## In Dependency Grammar, the clitic (an RDF line) depends on its host
## (the current grammar)-- together they are a Prosodic-- this is
## important because and RDF line on its own has no definite meaning-
## it must be interpreted in terms of its host Grammar, this tuple exists
## because the pair is passed allover the place.
class Prosodic(object):
    """Prosodic(clitic, host)

    clitic             is a string reppin' an RDF line
    host               is a Grammar() instance


    The actions of the lexis are carried out in methods.
    """
    __slots__ = ('clitic', 'host')

    def __init__(self, clitic, host):
        ## The line
        self.clitic = clitic
        ## The rdf.language.grammar.syntax.Grammar hosting the clitic
        self.host = host

    @property
    def _prefix(self):
        return self.host.prefix

    @property
    def _suffix(self):
        return self.host.suffix

    ## Return host's operator glyph
    @property
    def comment(self):
        return self.host.comment

    ## Return host's comment glyph
    @property
    def operator(self):
        return self.host.operator

    def __iter__(self):
        return iter((getattr(self, attr) for attr in self.__slots__))

    def __int__(self):
        return int(self.host)

    ## Extract the "value" field by\n\n
    ## getting the line left of Prosodic.comment  \n
    ## getting the remains right of Prosodic.operator
    def _value(self):
        return (+self.operator)((-self.comment)(self.clitic))

    ## Stripped sentence (as in no white spaces)
    def strip(self):
        return self.clitic.strip()

    ## INCLUDE
    def include_file(self):
        from rdf.uRDF import rdf_include
        return rdf_include(self._value(), _grammar=self.host)

    ## Change: OPERATOR / COMMENT
    def change_symbol(self, attr):
        """Send request to host to change attr to _value()"""
        self.host.change_symbol(attr, self._value())

    ## Change: PREFIX / SUFFIX
    def set_affix(self, classname):
        """Send request to host to set
        host.classname[int(host)] = _value()

        (that is, set PREFIX or SUFFIX's host.depth item to the value
        given by the clitic-- that the is, in the AFFFIX = <value>
        line"""
        self.host.set_affix(classname, self._value())

    ## Add to an rdf.units.physical_quantity.Unit to the rdf.units.GLOSSARY
    def set_unit(self):
        from rdf import utils
        from rdf.units import physical_quantity
        from rdf.units.errors import RedefiningUnitWarning
        ## Convert line into arguments and/or keywords
        args, kwargs = utils.parse_tuple_input(
            self._value().lstrip("(").rstrip(")")
            )
        ## Instantiation is memoized.
        physical_quantity._UnAccepted(*args, **kwargs)

    ## Add affixes--note: Grammar just adds, the overloaded __add__ and
    ## __radd__ invoke the affix protocol.
    def make_affix(self, stem):
        return self._prefix + stem + self._suffix

    ## RDF Record Reader
    def extract_record(self):
        """if self is a record, you'll get it-- if not who knows"""
        from rdf.language.grammar import punctuation
        from rdf.data.entries import RDFField
        left, comments = self.comment(self.clitic)
        left, value = self.operator(left)
        base_key, units, dimensions, element = punctuation.parse_left(left)
        return self.make_affix(base_key) + RDFField(value.strip(),
                                                    units=units,
                                                    dimensions=dimensions,
                                                    element=element,
                                                    comments=comments)

    ## RDF Comment Reader
    def extract_comment(self):
        """convert in a comment"""
        from rdf.data.entries import RDFComment
        line = self.strip()
        return RDFComment(line) if line else None  # semi-Guard
