"""Verbs may appear to be an anti-pattern: the methods go mutate another
objects attributes (Grammar). But that is how RDF works: meta-words change
the grammar.
"""
## \namespace rdf.language.lexis.pragmatics Words with (reflexive) meaning
## (Verb)
import abc
from rdf.language import lexis


## Verbs are rdf.language.lexis.Word objects that "act" as their sin_qua_non
class _Verb(lexis.Word):
    """_Pragmatic is an self identIfying string"""

    ## _Verb is to general to instaniated, it delegates sin_qua_non to
    ## act and decides is_not()
    __metaclass__ = abc.ABCMeta

    ## A Verb need a name, and it *will* be VERB.
    def __new__(cls):
        return str.__new__(cls, cls.__name__.upper())

    ## Allow class to identIfiy itself in the context of an
    ## rdf.language.prosodic.
    def is_(self, prosodic):
        """is_not(prosodic) IFF line is pragmaatic"""
        line = prosodic.strip()
        ## TODO: refactor this pos:
        return not (
            (line.startswith(self) and
             (line.lstrip(self).strip().startswith(prosodic.operator)))
            )

    ## Act is not action--> act tells this object to go do it's thing \n
    ## which is act on the grammar according to line.
    @abc.abstractmethod
    def act(self, prosodic):
        """act(prosodic) --> do what the line says and return result"""

    ## Verbs must act -- or return an empty iterable.
    def sin_qua_non(self, prosodic):
        """see act"""
        return self.act(prosodic) or ()


## Open an Include File
class Include(_Verb):
    """Verb can identIfy the INCLUDE lines"""
    ## Include via rdf.language.prosodic.include_file
    def act(self, prosodic):
        """return contents of file as an iterable full of RDFRecords"""
        return prosodic.include_file()


## ABC sends sub's class.__name__.lower() to
## rdf.language.prosodic.change_symbol
class _SymbolChanger(_Verb):
    """action is change symbol"""

    __metaclass__ = abc.ABCMeta

    ## A concrete method for an abstract class-- this changes grammar
    def act(self, prosodic):
        """change the symbol"""
        prosodic.change_symbol(type(self).__name__.lower())


## Change rdf.language.grammar.syntax.Grammar.operator via
## _SymbolChangrr.act
class Operator(_SymbolChanger):
    """Change grammar's operator"""


## Change rdf.language.grammar.syntax.Grammar.comment via
## _SymbolChangrr.act
class Comment(_SymbolChanger):
    """Change grammar's comment attribute"""


## ABC sends sub's class name to rdf.language.prosodic.set_affix
class _AffixAdder(_Verb):
    """_AffixAdder is an ABC"""

    __metaclass__ = abc.ABCMeta

    ## Act means set the affix, according to the sub's __name__.
    def act(self, prosodic):
        """add the affix"""
        prosodic.set_affix(type(self).__name__.lower())


## Add to rdf.language.grammar.syntax.Grammar.prefix via _AffixAdder.act
class Prefix(_AffixAdder):
    """Prefix is an _AffixAdder that cooperates with Grammar.prefix"""


## Add to rdf.language.grammar.syntax.Grammar.suffix via _AffixAdder.act
class Suffix(_AffixAdder):
    """Suffix is an _AffixAdder that cooperates with Grammar.suffix"""


## Add to an rdf.units.physical_quantity.Unit to the rdf.units.GLOSSARY
## via rdf.prosodic.Prosodic.set_unit
class Unit(_Verb):
    """The idea:

    UNIT = ('km', 1000, symbol='m')

    will load another unit in the GLOSSARY
    """
    def act(self, prosodic):
        """create a unit"""
        return prosodic.set_unit()


## Verb classes are auto instantiated by
## rdf.language.grammar.syntax.lexicon metaclass.
VERBS = (Include, Operator, Comment, Prefix, Suffix, Unit)
