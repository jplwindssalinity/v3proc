"""Reserved Symbols"""
## \namespace rdf.reserved Reserved Symbols
import abc


## Meta word defining operator symbol
OPERATOR = "="

## Meta word defining operator symbol
COMMENT = "#"

## SEPARATOR Symbol
SEPARATOR = ", "
## Line Wrap Symbol
WRAP = '/'
## Carriage Return
CR = '\n'

## Unit Delimiter ordered glyphs
UNITS = '()'
## Dimensions wrapper ordered glyphs
DIMENSIONS = '{}'
## Element wrapper ordered glyphs
ELEMENT = '[]'

## Singular characters, who's repetition constitutes an
## rdf.language.errors.ReservedCharacterError
SINGULAR = OPERATOR + COMMENT

## If this is True, then the UAVSAR workaround will be in effect
UAVSAR = " & "


## For classes that need to know about OPERATOR and COMMENT (statically).
class SymbolsMixIn(object):
    """Mixin makes class constants: Operator, Comment from
    module constants: OPERATOR, COMMENT."""

    __metaclass__ = abc.ABCMeta

    ## The operator symbol (default) -capitalized to avoid class with property
    Operator = OPERATOR
    ## The comment symbol (default) -capitalized to avoid class with property
    Comment = COMMENT
