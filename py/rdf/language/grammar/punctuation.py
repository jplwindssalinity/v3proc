"""Brackets: This is where glyphs take on meaning-- the Bracket class


defines them, while the punctuation module serves as a static-class
like object with the 3 RDF brackets defined:

(UNITS)
{DIMENSIONS}
[ELEMENT]

Along with reading and writing functions:

parse_left            Parse left side of an rdf line key, units, ..., element
read_rdfkey           Parse the key the left side of an rdf line
read_brackets         Get the 3 bracketed values
write_brackets        Write a nice formatted string with units, ..., element


Note: Function accept left side of an rdf line to preclude awkward parsing of
potentially silly comment fields.
"""
## \namespace rdf.language.grammar.punctuation Language's Punctuation Marks.
from rdf import reserved


## A <a href="http://en.wikipedia.org/wiki/Glyph">character<\a> that knows
## how to find itself in strings
class Glyph(str):
    """A Glyph is a str sub-class that can be called to figure itself out.
    For example:

    >>> line = "unit = equal"
    >>> g = Glyph("=")

    You get:

      OP                VALUE                     Comments
    -------------------------------------------------------------
    g(line)         ['unit', 'equal']           split string on glyph
    (+g)(line)          'equal'                   right side string
    (-g)(line)           'unit'                    left side string
    """
    ## split line on self
    ## \param line A line
    ## \returns (left, right) side of line (with possible null str on right)
    def __call__(self, line):
        try:
            # This works if the glyph is in line,
            index = line.index(self)
        except ValueError:
            # otherwise, there is nothing to split, so left=line and right="".
            left, right = line, ""
        else:
            # Since it did work, take the 1-char Glyph out of the results.
            left = line[:index]
            right = line[index+1:]
        # now return the results with leading/trailing junk stripped.
        return map(str.strip, (left, right))

    ## Get line left of self
    ## \param line A line with or without self
    ## \retval left line left of self
    def _left(self, line):
        """left symbol"""
        # return the left side of __call__.
        return self(line)[0]

    ## Get line right of self
    ## \param line A line with or without self
    ## \retval right line right of self
    def _right(self, line):
        """right symbol"""
        # return the right side of __call__.
        return self(line)[-1]

    ## (+glyph)(line) --> glyph.right(line)
    def __pos__(self):
        """+glyph --> glyph.right"""
        # NB: this returns the METHOD, not the value
        return self._right

    ## (-glyph)(line) --> glyph.left(line)
    def __neg__(self):
        """-glyph --> glyph.left"""
        # NB: this returns the METHOD, not the value
        return self._left


## <a href="http://en.wikipedia.org/wiki/Bracket">Brackets</a> that
## know thyselves.
class Brackets(str):
    """_Delimeter('LR')

    get it? Knows how to find itself in a line, por exemplio:

    >>>line = "a line <with> a bracket"
    >>>b = Brackets("<>")

    You get:

      OP                VALUE                     Comments
    -------------------------------------------------------------
     +b                  '>'               + is the right side
     -b                  '<'               - is the left side

     b >> line          'with'           / Extract bracket's contents for
     line << b          'with'           \ a line with the bracket

     'with' >> b       ' <with> '        / Insert a value in the brackets
     b << 'with'       ' <with> '        \ and make a usable string

     b - line                                TypeError
     line - b       'a line  a bracket'    get the bracket out of the line

     b in line           False           /    normal
     b in ' <> '         True            \   str.__contains___

     line in b           True               True If line uses bracket
     (line - b) in b     False              False If it does not.
     """
    ## L, R --> -, + \n + is right
    def __pos__(self):
        return self[-len(self)/2:]

    ## L, R --> -, + \n - is left
    def __neg__(self):
        return self[:len(self)/2]

    ## extract enclosed:  line << pair
    # \param line An RDF sentence
    # \par Side Effects:
    #  raises RDFWarning on bad grammar
    # \retval contents The string inside the Bracket
    def __rlshift__(self, line):
        """INPUTS: pair, line

        pair is 2 characters LR, this extracts part of line
        between L    and   R. Throws errors IF need be.
        """
        # 5 IF's are for error checking, not processing
        from rdf.language import errors
        ## Count start and stops
        count = map(line.count, self)
        ## Guard: early return.
        if min(count) is 0:   # Guard
            ## Check IF there is an oper/close error
            if max(count):   # Guard
                raise errors.UnmatchedBracketsError(self)
            return None
        ## Ensure 1 pair
        if max(count) > 1:   # Guard
            raise errors.RunOnSentenceError(self)
        i_start = line.index(-self) + 1
        i_stop = line.index(+self)
        ## ensure order:
        if i_stop <= i_start:    # Guard
            raise errors.BackwardBracketsError(self)
        contents = line[i_start: i_stop]
        # finally check for nonesense
        for single_char in contents:
            if single_char in reserved.SINGULAR:   # Guard
                raise errors.ReservedCharacterError(self)
        return contents

    ## Insert: line >> pair or go blank
    def __rrshift__(self, line):
        """Insert non-zero line in string, or nothing"""
        return " %s%s%s " % (-self, str(line), +self) if line else ""

    __lshift__ = __rrshift__
    __rshift__ = __rlshift__

    ## (line in delimiter) IF the line has token in it legally
    # \param line an RDF sentence
    # \retval <bool> IF Bracket is in the line
    def __contains__(self, line):
        return ((-self in line) and
                (+self in line) and
                line.index(-self) < line.index(+self))

    ## line - delimiter removes delimiter from line, with no IF
    def __rsub__(self, line):
        """Get value surrounded"""
        return self._rsub_choser[line in self](self, line)

    ## Extract part of line outside the glyph (stripped)
    def _get_outer(self, line):
        """_get_outer('a line <with> a glyph') --> 'a line  a glyph'
        so it's every thing but the glyph and its contents"""
        return (line[:line.index(-self)] +
                line[1+line.index(+self):]).strip()

    ## Method "could be a function" -except it is called as an unbound
    ## method  by __rsub__ and the signature must match _get_outer()
    ## which could not be a function-- so-to-speak.
    #pylint: disable=R0201
    def _noop_on_inner(self, line):
        """If glyph is not in line, then do nothing"""
        return line

    ## dict allows __rsub__ to chose correct method to process a line with
    ## neither If/then no try/except.
    _rsub_choser = {True: _get_outer, False: _noop_on_inner}


## (units) Brackets
UNITS = Brackets(reserved.UNITS)


## {dim} Brackets
DIMENSIONS = Brackets(reserved.DIMENSIONS)


## [elements] Brackets
ELEMENT = Brackets(reserved.ELEMENT)


## and RDF line (that is, left of OPERATOR)
def parse_left(leftline):
    """'key (unit) {dim} [el]' --> ['key', 'unit', 'dim', 'el']"""
    return [read_rdfkey(leftline)] + read_brackets(leftline)


## Remove brackets from left line
def read_rdfkey(leftline):
    """'key (unit) {dim} [el]' --> 'key'"""
    return (leftline -
            UNITS -
            DIMENSIONS -
            ELEMENT).strip()


## Read list of brackets from left line
def read_brackets(leftline):
    """'key (unit) {dim} [el]' --> ['unit', 'dim', 'el']"""
    return [leftline << item for item in (UNITS, DIMENSIONS, ELEMENT)]


## Pack bracket values into a string
def write_brackets(units, dimensions, element):
    """'unit', 'dim', 'el' --> '(unit) {dim} [el]'"""
    return ((units >> UNITS) +
            (dimensions >> DIMENSIONS) +
            (element >> ELEMENT))
