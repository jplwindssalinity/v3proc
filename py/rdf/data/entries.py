"""Define the RDF Entries as:
RDFRecord = (key, RDFField)"""
## \namespace rdf.data.entries Usable data objects for lines (records).
import collections
import sys
#from functools import partial
#from operator import methodcaller
from rdf import reserved
from rdf.language.grammar import punctuation

# A space character
SPACE = " "


## Decorator to cast values
## \param magicmethodbinding A function that binds to a magic method and
## casts instances (for example: float)
## \retval cast_method An instance method that cast ala magic-method binding
def _cast(magicmethodbinding):
    """decorator for magic method/function casting"""
    def cast_method(self):
        """__int__ --> int(self.value) --for example"""
        return magicmethodbinding(self.value)
    return cast_method

## Base RDF Field named tuple -it's all in here -note, it's assigned (public)
## name dIffers from its variable (private) name, so that users never need to
## know about this  private assignment
_RDFField = collections.namedtuple('RDFField',
                                   'value units dimensions element comments')


## Add methods and constants to _RDFField so that it lives up to its name.
class RDFField(reserved.SymbolsMixIn, _RDFField):
    """
    RDFField(value, units=None, dimensions=None, element=None, comments=None)

    represents a fully interpreted logical entry in an RDF file (sans key)
    """

    ## (-) appears as default
    _default_units = "-"
    ## non-private version: it is used in units.py
    default_units = _default_units
    ## does not appear b/c it's False
    _default_comments = ""
    ## _ditto_
    _default_dimensions = ""
    ## _ditto_
    _default_element = ""

    ## Do a namedtuple with defaults as follows...
    ## \param  [cls] class is implicitly passed...
    ## \param  value Is the value of the rdf field
    ## \param [units] defaults to RDFField._default_units
    ## \param [dimensions] defaults to RDFField._default_dimensions
    ## \param [element] defaults to RDFField._default_element
    ## \param [comments] defaults to RDFField._default_comments
    ## \returns _new__ returns new instances, or value If value
    ## already is an RDFField instance (which makes RDF.__setitem__
    ## easy to use)
    def __new__(cls, value, units=None, dimensions=None, element=None,
                comments=None):
        ## Bail on redundant instantiation
        if isinstance(value, cls):  # guard
            return value
        ## Unit conversion
        value, units = cls._handle_units(value, units)
        return _RDFField.__new__(cls,
                                 value,
                                 str(units or cls._default_units),
                                 str(dimensions or cls._default_dimensions),
                                 str(element or cls._default_element),
                                 str(comments or cls._default_comments))

    ## Do the unit conversion
    @classmethod
    def _handle_units(cls, value, units):
        from rdf.units import SI, errors
        # convert units, If they're neither None nor "-".
        if units and units != cls._default_units:
            try:
                value, units = SI(value, units)
            except errors.UnrecognizedUnitWarning:
                pass
        return value, units

    ## eval(self.value) -with some protection/massage
    ## safe for list, tuples, nd.arrays, set, dict,
    ## anything that can survive repr - this is really a work in progress,
    ## since there is a lot of python subtly involved.
    ## \returns evaluated version of RDFField.value
    def eval(self):
        """eval() uses eval built-in to interpret value"""
        try:
            result = eval(str(self.value))
        except (TypeError, NameError, AttributeError, SyntaxError):
            try:
                result = eval(repr(self.value))
            except (TypeError, NameError, AttributeError, SyntaxError):
                result = self.value
        return result

    ## Construct string on the right side of OPERATOR (w/o an IF)
    def right_field(self):
        """Parse right of operator"""
        return (str(self.value) +
                (SPACE + self.Comment) * bool(self.comments) +
                (self.comments or ""))

    ## Construct string on the left side of OPERATOR
    def left_field(self, index=0):
        """Parse left of OPERATOR
        place OPERATOR at index or don't
        """
        result = punctuation.write_brackets(self.units,
                                            self.dimensions,
                                            self.element)
        return result + (SPACE * max(0, index-len(result)))

    ## FORMAT CONTROL TBD
    def __str__(self, index=0):
        """place OPERATOR at index or don't"""
        return (self.left_field(index=index) +
                self.Operator + SPACE +
                self.right_field())

    ## Call returns value
    ## \param [func] = \f$ f(x):x \rightarrow x\f$ A callable (like float).
    ## \returns \f$ f(x) \f$ with x from eval() method.
    def __call__(self, caster=lambda __: __):
        """You can cast with call via, say:
        field(float)"""
        return caster(self.eval())

    __index__ = _cast(bin)
    __hex__ = _cast(hex)
    __oct__ = _cast(oct)
    __int__ = _cast(int)
    __long__ = _cast(long)
    __float__ = _cast(float)
    __complex__ = _cast(complex)

    ## key + field --> _RDFPreRecord, the whole thing is private.
    def __radd__(self, key):
        return RDFPreRecord(key, self)


## This assignment is a bit deeper: Just a key and a field
_RDFRecord = collections.namedtuple("RDFRecord", "key field")


## Base for iterable (key, field) pair
class _RDFRecord(object):
    """See __slots__"""

    __slots__ = ('key', 'field')

    ## (key, field)
    def __init__(self, key, field):
        assert(isinstance(field, RDFField))
        ## The RDF key (with affixes)
        self.key = key
        ## The RDFField
        self.field = field


## The pre Record is built from data and is a len=1 iterator: iterating builds
## the final product: RDFRecord-- thus line reads or include file reads yield
## the same (polymorphic) result: iterators that yield Records.
class RDFPreRecord(_RDFRecord):
    """Users should not see this class"""

    ## Iteration does not traverse (key, field), it:
    ## \returns A generator that yields RDFRecord iter(RDFPreRecord)
    def __iter__(self):
        yield RDFRecord(*(getattr(self, attr) for attr in self.__slots__))


## This is a fully parsed RDF record, and is an _RDFRecord with a format-able
## string.
class RDFRecord(_RDFRecord):
    """RDFRecord(key, field)

    is the parsed RDF file line. Key is a string (or else), and
    field is an RDFField.
    """
    ## (key, field)
    def __iter__(self):
        return iter((getattr(self, attr) for attr in self.__slots__))

    ## int is the position of the rdf.reserved.OPERATOR (for formatting)
    def __int__(self):
        return str(self).index(reserved.OPERATOR)

    ## FORMAT CONTROL TBD
    def __str__(self, index=0):
        """place OPERATOR at index or don't"""
        key = str(self.key)
        field = self.field.__str__(max(0, index-len(key)))
        return key + field


## The RDF Comment is a comment string, endowed with False RDF-ness
## and null response to iteration.
class RDFComment(str):
    """This is string that always evaluates to False.

    Why?

    False gets thrown out before being sent to the RDF constructor

    But!

    It is not None, so you can keep it in your RDFAccumulator
    """
    ## RDF comments are False in an RDF sense--regardless of their content
    # \retval < <a href=
    # "http://docs.python.org/2/library/constants.html?highlight=false#False">
    # False</a> Always returns False-- ALWAYS
    def __nonzero__(self):
        return False

    ## Iter iterates over nothing-NOT the contents of the string.
    ## \retval iter(())  An empty iterator that passes a for-loop silently
    def __iter__(self):
        return iter(())
