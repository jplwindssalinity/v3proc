"""Supported morphemes are affixes. The Affix ABC (and subclass of the list
built-in) has two concrete subs:

Prefix
Suffix

They know how to apply themselves, and they know what to do to Grammar
as it traverses the IFT.
"""
## \namespace rdf.language.grammar.morpheme Key Changing Morphemes
import abc


## Abstract Base Class for Pre/Suf behavior
class Affix(list):
    """The Affix is an abstract base class.
    It implements the:

    descend/ascend methods for traversing the IFT

    It is callable: Given a key, it will do what morphemes do and make
    as new key per the RDF spec.

    Sub classes use operator overloads to do their thing
    """

    __metaclass__ = abc.ABCMeta

    ## Descend the IFT-- add a null string to the affix list
    ## \param None
    ## \par Side Effects:
    ## Append null string to Affix
    ## \returns None
    def descend(self):
        """append null string to self"""
        return self.append("")

    ## Ascend the IFT-- pop the affix off and forget it
    ## \param None
    ## \par Side Effects:
    ##  Pops last affix off of Affix
    ## \returns None
    def ascend(self):
        """pop() from self"""
        return self.pop()

    ## Call implements the construction of the affix (so IF you change the def
    ## you change this 1 line of code.
    ## \returns Sum of self- the complete affix
    def __call__(self):
        """call implements the nest affix protocol: add 'em up"""
        return "".join(self)

    ## strictly for safety
    def __add__(self, other):
        from rdf.language import errors
        raise (
            {True: errors.MorphemeExchangeError(
                    "Cannot Pre/Ap-pend a Suf/Pre-fix"),
             False: TypeError("Can only add strings to this list sub")}[
                isinstance(other, basestring)
                ]
            )

    __radd__ = __add__


## Appears Before the stem:
class Prefix(Affix):
    """prefix + stem

    is the only allowed operator overload- it, by definition, must
    be pre-pended"""
    ## prefix + stem (over-rides list concatenation)
    def __add__(self, stem):
        return self() + stem


## Appears After the stem
class Suffix(Affix):
    """stem + suffix

    is the only allowed operator overload- it, by definition, must
    be appended"""
    ## stem + prefix  (over-rides list concatenation)
    def __radd__(self, stem):
        return stem + self()
