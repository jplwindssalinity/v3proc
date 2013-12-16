## \namespace rdf.language.lexis The Lexis comprises the words in the language.
import abc


## The Pragmatic's are RDF lines meaning.
class Word(str):
    """Word is an ABC that subclasses str. It has a call
    that dyamically dispatches args = (line, grammar) to
    the sub classes' sin qua non method-- which is the
    method that allows them to do their business.
    """

    __metaclass__ = abc.ABCMeta

    ## Negative form for running itertools.dropwhile TODO:
    ## undetstand neg v. pos
    def is_not(self, prosodic):
        return self.is_(prosodic)

    # Call the Pragmatic's 'sin_qua_non' method -which is TBD \n
    # (To be Dynamically Dispatched ;-)
    def __call__(self, prosodic):
        return self.sin_qua_non(prosodic)

    @abc.abstractmethod
    def sin_qua_non(self, prosodic):
        pass
