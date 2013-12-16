"""syntax handles syntax via the Grammar class. It handles syntax
but farms out some work to cooperative classes"""
## \namespace rdf.language.grammar.syntax Syntax glues it all together
import abc
import itertools
import sys
from rdf.language import errors
from rdf.language.grammar import punctuation, morpheme
from rdf import reserved
from rdf.language.lexis import semantics, pragmatics


## decorate setters to prevent setting required commands to NULL
def null_command_watch(method):
    def setter(self, value):
        """if bool(value): setter, else raise NullCommandError"""
        if not value:  # guard
            raise errors.NullCommandError
        return method(self, value)
    return setter


## decorate incremental ops to only allow +1 change
def unit_change(method):
    def setter(self, value):
        """if value != 1: raise error"""
        if value != 1:  # guard
            raise ValueError("can only in/dec-rement by +1")
        return method(self, value)
    return setter


## Metaclass for Grammar gets defines the pragmatics and semantics at
## load-time" pragmatics.Verb instances are assigned according to the
## rdf.reserved.words.KEYWORDS, and the semantics. Noun instances are created-
## these are needed by Grammar.process
class lexicon(abc.ABCMeta):
    """lexicon meta class deal with the keywords defined in
    verbs.
    """
    ## Create class and add pragmatics and semantics
    def __new__(mcs, *args, **kwargs):
        cls = type.__new__(mcs, *args, **kwargs)
        ## Set up verbs
        cls._VERBS = tuple(map(apply, pragmatics.VERBS))
        ## Set up Noun instances by instantiating NOUNS's classes
        cls._NOUNS = tuple(map(apply, semantics.NOUNS))
        return cls


## Grammar tracks the state of grammar, and uses it to dispatch
## work when it is called.
class Grammar(reserved.SymbolsMixIn):
    """Grammar() is the state of the grammar. See __init__ for why
    it supports only nullary instantiation.

    ALL_CAP class attributes a Pragmatic (i.e. meta) words.
    _lower_case private instance attributes are punctuation Glyphs
    lower_case  mutator methods ensure glyphs setting is kosher.
    Capitalized class attributes are default values for the lower_case
    version.

    Overloads:
    ---------
    Function Emulation

    line --> __call__(line)  ---> RDFRecord #That is, grammar is a (semi-pure)
                                             function that makes lines into
                                             RDFRecords.

    (meta)line-> __call__(line)---> None    # Pragmatic (KeyWord) lines
                                              return None (they aren't rdf
                                              records) but they change the
                                              internal state of 'self'. Hence
                                              grammar is an impure function.


    other -->   __call__(line)---> None    # Comments do nothing, Errors are
                                             identIfied, reported to stderr
                                             and forgotten.

    Integer:
    int(grammar) returns the depth-- which is a non-negative integer telling
                         how deep the processor is in the include file tree
                         (IFT) Should not pass sys.getrecursionlimit().

    grammar += 1  There are called when the depth_processor goes up or
    grammar -= 1  down the IFT. The change int(grammar) and manage the
                  affixes.
    """
    ## Add lexicon at class creation
    __metaclass__ = lexicon

    ## wrap tell read how to unwrap lines-- it's just a str
    wrap = reserved.WRAP

    ## sep is not used -yet, it would appear in RDF._eval at some point.
    sep = reserved.SEPARATOR

    ## Static default prefix
    Prefix = [""]

    ## Static default suffix
    Suffix = [""]

    ## dict to prevent ill use of set_affix
    _appendables = {"prefix": "prefix", "suffix": "suffix"}

    ## dict to prevent ill use of change_symbol
    _mutables = {'operator': 'operator', 'comment': 'comment'}

    ## Grammar always boots into the default state, and can only
    ## be changed by processing RDF lines.
    def __init__(self):
        """Nullary instantiation: you cannot inject dependencies (DI)
        in the constructor. You always start with the default grammar-
        which is defined in static class attributes.

        Only rdf Pragmatics (i.e commands or key words) can change the
        grammar -- in fact, the attributes encapsulated in mutators.
        """
        ## The recursion depth from which the rdf lines are coming.
        self.depth = 0
        ## copy rdf.reserved.SymbolsMixIn.Operator via Grammar.operator
        self.operator = self.Operator
        ## copy rdf.reserved.SymbolsMixIn.Commment via Grammar.operator
        self.comment = self.Comment
        ## Dynamic prefix is a copy of a list -and depends on depth
        self.prefix = self.Prefix[:]
        ## Dynamic suffix is a copy of a list -and depends on depth
        self.suffix = self.Suffix[:]

    ## Getter
    @property
    def operator(self):
        return self._operator

    ## Setter has mutators to ensure it is an
    ## rdf.language.grammar.punctuation.Glyph object
    @operator.setter
    @null_command_watch
    def operator(self, value):
        self._operator = punctuation.Glyph(value)

    ## Getter
    @property
    def comment(self):
        return self._comment

    ## Setter has mutators to ensure it is a
    ## rdf.language.grammar.punctuation.Glyph object
    @comment.setter
    @null_command_watch
    def comment(self, value):
        self._comment = punctuation.Glyph(value)

    ## Getter
    @property
    def prefix(self):
        return self._prefix

    ## Ensure Grammar._prefix is an rdf.language.grammar.morpheme.Prefix
    @prefix.setter
    def prefix(self, value):
        self._prefix = morpheme.Prefix(value)

    ## Getter
    @property
    def suffix(self):
        return self._suffix

    ## Ensure Grammar._suffix is an rdf.language.grammar.morpheme.Suffix
    @suffix.setter
    def suffix(self, value):
        self._suffix = morpheme.Suffix(value)

    ## str reflects the current grammar state
    def __str__(self):
        return (str(self.depth) + " " +
                self.operator + " " +
                self.comment + " " + str(self.prefix) + str(self.suffix))

    ## int() --> Grammar.depth
    def __int__(self):
        return self.depth

    ## += --> change depth and append affixes w/ morpheme.Affix.descend \n
    ## (which knows how to do it)
    ## \param n +1 or ValueError
    ## \par Side Effects:
    ## Affix.descend()
    ## \retval self self, changed
    @unit_change
    def __iadd__(self, n):
        self.depth += int(n)
        self.prefix.descend()
        self.suffix.descend()
        return self

    ## += --> change depth and truncate affixes w/ morpheme.Affix.ascend
    ## (b/c grammar just implements it)
    ## \param n +1 or ValueError
    ## \par Side Effects:
    ##  Affix.ascend()
    ## \retval self self, changed
    @unit_change
    def __isub__(self, n):
        self.depth -= int(n)
        self.prefix.ascend()
        self.suffix.ascend()
        return self

    ## Grammar(line) --> rdf.data.entries.RDFRecord \n
    ## It's the money method-- not it's not a pure function- it can
    ## change the state of grammar.
    def __call__(self, line):
        """grammar(line) --> grammar.process(line) (with error catching)"""
        from rdf.units.errors import UnrecognizedUnitWarning
        try:
            result = self._process(line)
        except UnrecognizedUnitWarning as err:
            print >> sys.stderr, "WARNING: " + repr(err) + ": : " + line
            result = []
        except errors.RDFWarning as err:
            print >> sys.stderr, "WARNING: " + repr(err) + ": : " + line
            result = []
        return result

    ## Grammar + <str> = rdf.language.Prosodic
    def __add__(self, line):
        from rdf.language import Prosodic
        return Prosodic(line, self)

    ## Process the line a Verb or a Line
    ## \param line rdf sentence (a clitic)
    ## \par Side Effects:
    ## word might change self
    ## \retval word(prosodic) The processed line
    def _process(self, line):
        import operator
        # combine line and grammar into 1 thing
        prosodic = self + line
        # Short Circuit Loop: Iterator ends when word is found,
        # then break out of loop with an return result, this is "IFless":
        for word in itertools.dropwhile(operator.methodcaller("is_not",
                                                              prosodic),
                                        itertools.chain(self._VERBS,
                                                        self._NOUNS)):
            break
        return word(prosodic)

    ## Setitem in an AFFIX in Grammar._appendables
    def set_affix(self, classname, value):
        """set_affix(self, classname, value)
        classname in (prefix, suffix)
        value = affix at depth int(self)
        """
        try:
            getattr(self, self._appendables[classname])[int(self)] = value
        except KeyError:
            raise ValueError("Can't set %s with affix protocol" %
                             repr(classname))

    ## Change a symbol in Grammar._mutables
    def change_symbol(self, attr, value):
        """change_symbol(self, attr, value)
        atr in (operator, comment)
        value = , # ! ... whatevs."""
        try:
            setattr(self, self._mutables[attr], value)
        except KeyError:
            raise ValueError("Can't change %s with change_symbol" %
                             repr(attr))
