"""RDF Language Exceptions"""
## \namespace rdf.language.errors RDF Language Exceptions


## Fatal attempt to CODE badly
class RDFError(Exception):
    """Base RDF Error for BAD RDF coding (Fatal)"""


## RDF Warning of INPUT problems
class RDFWarning(UserWarning):
    """Base RDF Warning for bad RDF input grammar"""


## <a href="http://en.wikipedia.org/wiki/Speech_error">Morpheme Exchange
## Currents</a>?
class MorphemeExchangeError(RDFError):
    """fix-pre and/or suffix would cast list v. str type errors on "+"
    anyway, so this is a TypeError"""


## Error for using a character in RESERVED
class ReservedCharacterError(RDFWarning):
    """Error for using a RESERVED character badly"""


## Unmatched or un parsable pairs
class UnmatchedBracketsError(ReservedCharacterError):
    """1/2 a delimiter was used"""


## Unmatched or un parsable pairs
class RunOnSentenceError(ReservedCharacterError):
    """Too many punctuation marks"""


## Unmatched or un-parsable pairs
class BackwardBracketsError(ReservedCharacterError):
    """Inverted Punctuation"""


## Should be thrown in constructor?
class NullCommandError(RDFWarning):
    """Setting a required command to nothing"""
