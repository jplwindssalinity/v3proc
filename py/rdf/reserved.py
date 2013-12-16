"""Reserved Symbols"""
## \namespace rdf.reserved Reserved Symbols

## Meta word defining operator symbol
OPERATOR = "="
## Meta word defining comment symbol (deprecated !)
COMMENT = "#"

## SEPARATOR Symbol
SEPARATOR = ","
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


## For classes that need to know about OPERATOR and COMMENT (statically).
class SymbolsMixIn(object):
    ## The operator symbol (default) -capitalized to avoid class with property
    Operator = OPERATOR
    ## The comment symbol (default) -capitalized to avoid class with property
    Comment = COMMENT
