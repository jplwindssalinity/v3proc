"""Lexis is the collection of Words and their meaning"""
## \namespace rdf.language.lexis The Lexis comprises the words in the language.
import abc


## The Pragmatic's are RDF lines meaning.
class Word(str):
    """Word is an ABC that subclasses str. It has a call
    that dynamically dispatches args = (line, grammar) to
    the sub classes' sin qua non method-- which is the
    method that allows them to do their business.
    """

    __metaclass__ = abc.ABCMeta


    ## Transform class name into it's string value.
    @abc.abstractproperty
    def _namer(self):
        """_namer must transform class name into a string value that is
        and RDF keyword or concept:

        For example:

        pragmatic._Verb: Include  class is instantiated with "INCLUDE"
                         Operator class is instantiated with "OPERATOR"
                         Prefix   class is instantiated with "PREFIX"
                         Suffix   class is instantiated with "SUFFIX"
                         Unit     class is instantiated with "UNIT"
                         (this makes KEYWORDS self instantiating)


        semantics._Noun: Record class is instantiated with "Record"
                         Comment class is instantiated with "Comment"
        """

    ## A Verb need a name, and it *will* be VERB.
    ## @param mcs (implicit)
    ## @retval cls instantiated with str.upper(mcs.__name__)
    def __new__(cls):
        return super(Word, cls).__new__(cls, cls._namer(cls.__name__))

    ## Negative form for running itertools.dropwhile TODO:
    ## undetstand neg v. pos
    def is_not(self, prosodic):
        """is_not is is_"""
        return self.is_(prosodic)

    # Call the Pragmatic's 'sin_qua_non' method -which is TBD \n
    # (To be Dynamically Dispatched ;-)
    def __call__(self, prosodic):
        return self.sin_qua_non(prosodic)

    @abc.abstractmethod
    def sin_qua_non(self, prosodic):
        """Word subs ne a sin_qua_non"""
