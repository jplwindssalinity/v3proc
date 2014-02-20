"""data-->RDF is THE RDF OBJECT"""
##\namespace rdf.data.files Usable data object for files.
import collections
import copy
import functools
import sys



#pylint:disable=E1101
try:
    DICT = collections.OrderedDict
except AttributeError:
    print >> sys.stderr, "Not 2.7: using (UnOrdered) dict for rdf mapping"
    DICT = dict


WARN = False


## Decorator to cast and strip arguments
def stripper(method):
    """stripper(method) strips blanks from method's I/O"""
    @functools.wraps(method)
    def stripped_method(self, key, *args):
        """method(key, *args)--- **kwargs is not supported"""
        return method(self, str(key).strip(), *args)
    return stripped_method


## An RDF Mothership: A fully interpresed RDF File.
class RDF(object):
    """RDF object is made from the read_rdf helper function:

    >>>data = rdf_reader('rdf.txt')

    It is an associate array, so like a dict:

    >>>data[key]

    returns a value- as a float or string-or whatever "eval" returns.

    All the standard OrderDict methods can be used, and will return the
    full RDFField object that represent the value, units, dimension....
    comments.


    You may __setitem__:
    >>>rdf[key] = value #equivalent to
    >>>rdf[key] = RDFField(value)

    That is, it transforms assignee into an RDFField for you.
    """
    from . entries import RDFRecord
    
    ## Build an RDf from a string repping an entire file as follows:\n
    ## split string on carriage returns \n
    ## Send it through rdf.utils.unwrap_readlines() \n
    ## Make an rdf.iRDF.RDFAccumulator instance and map the
    ## list of unwrapped rdf lines to its __call__ method
    ## \param str_ A string representing a valid rdf line
    ## \retval accumulator.rdf an RDF instance
    @classmethod
    def fromstr(cls, str_):
        """RDF.fromstr(str_)

        str_ is a single string representing an RDF file.

        The point is, if you have an object who's string method turns it
        into an RDF file, there is no need to write the file in order to
        turn it into an RDF object.
        """
        from rdf import iRDF
        from rdf import utils
        accumulator = iRDF.RDFAccumulator()
        filter(accumulator, utils.unwrap_readlines(str_.split('\n')))
        return accumulator.rdf

    ## Make an instance from DICT argument
    ## \param dict_ is an rdf-enough dictionary
    ## \retval result an RDF instance
    @classmethod
    def fromdict(cls, dict_):
        """instantiate from a DICT:


        RDF.fromdict(dict(<whatever>))
        """
        result = cls()
        for key, value in dict_.items():
            result[key] = value
        return result

    ## Make it from keyword arguments
    ## \param *args is an rdf-enough arguments (like dict)
    ## \param **dict_ is an rdf-enough dictionary
    ## \retval result an RDF instance
    @classmethod
    def fromstar(cls, *args, **dict_):
        """instantiate from *args **dict_:


        RDF.fromstar(*args, **dict_)

        where args must by of the form such that:
        dict(args) works.
        """
        # todo: (dict(*args) + dict_)
        rdf_ = cls()
        for pair in args:
            key, value = pair
            rdf_[str(key)] = value
        return rdf_ + cls.fromdict(dict_)

    ## Instantiate from a file
    ## \param src RDF file name
    ## \retval rdfparse(src) an RDF instance
    @staticmethod
    def fromfile(src):
        """src -> RDF

        src is a file (path/)name.
        """
        from rdf import rdfparse
        return rdfparse(src)

    ## Yet another synonym
    parse = fromfile

    ## Don't use init-- it's internal.
    ## \param *args Internal constructor takes *args
    def __init__(self, *args, **kwargs):
        """RDF(*args, **kwargs) matched dict built-in

        Use caution with **kwargs, and if you have trouble, trt:
        RDF.fromstar(*args, **kwargs)
        """
        ## The data are an associative array-or the rdf spec is worthless.
        self._data = DICT(args, **kwargs)

    ## Get attr from ::DICT If needed
    ## \param attr Attribute
    ## \retval self.attr OR
    ## \retval self._data.attr If needed
    def __getattr__(self, attr):
        try:
            result = object.__getattribute__(self, attr)
        except AttributeError:
            result = getattr(self._data, attr)
        return result


    def __eq__(self, other):
        return self._data == other.todict()


    ## rdf is rdf
    @property
    def rdf(self):
        """rdf is self is rdf is self..."""
        return self

    # \param func function
    # \retval rdf RDF w/ false func(value) items removed.
    def filter(self, func=lambda item: item):
        """filter rdf results on func"""
        try:
            return type(self).fromdict(
                DICT(
                    {key: value for key, value in
                     self.iteritems() if func(value.value)}  # filter
                    )
                )
        except TypeError as err:
            if callable(func):
                raise err
            raise TypeError("func argument is not callable")

    ## Return value, vs "get" returns RDFField
    ## \param key an RDF key
    ## \retval value The record's value.
    @stripper
    def __getitem__(self, key):
        return self._data[key]()    # .value ?

    ## Set the item to an RDFField
    ## \param key an RDF key
    ## \param value any kind of value
    ## \par Side Effects:
    ##  assigns key in RDF._data with RDFField value
    ## \returns None
    @stripper
    def __setitem__(self, key, value):
        self._data[key] = self.RDFRecord.RDFField(value)  #  LoD is TBD

    ## Delete key, field
    ## \param key an RDF key
    ## \par Side Effects:
    ## deletes key from RDF._data
    ## \returns None
    @stripper
    def __delitem__(self, key):
        self._data.__delitem__(key)

    ## \retval generator over data's items.
    def __iter__(self):
        return iter(self._data.iteritems())

    ## Access as method or property (this is really for clients)
    def __call__(self):
        """self()->self so that x.rdf()()-->x.rdf()->x.rdf"""
        return self

    ## A nice formatted string
    def __str__(self):
        max_index = self._max_index()
        ## now insert space...
        final_result = []
        for record in self.records():
            line = record.__str__(index=max_index)
            final_result.append(line + '\n')
        return "".join(final_result)

    ## rep the data attribute
    def __repr__(self):
        return repr(self._data)

    ## len()
    def __len__(self):
        return len(self._data)

    ## rdf >> dst \\n
    ## see tofile().
    def __rshift__(self, dst):
        """Usage:
        >>>rdf >> 'dst.rdf'
        """
        return self.tofile(dst)

    ## \verb{ rdf << src } Sucks up an new rdf file (see fromfile())
    ## \param src RDF file name
    ## \returns src + RDF
    def __lshift__(self, src):
        """Usage:
        >>>rdf.RDF() << 'src.rdf'
        """
        return self.fromfile(src)

    ## rdf << rdf' # lets rdf suck up an new rdf file.
    def __ilshift__(self, src):
        """Usage:
        >>>rdf <<= 'src.rdf'
        """
        self = self + (type(self)() << src)
        return self

    ## Add is concatenation, and is not commutative
    ## \param other An RDF instance
    ## \retval result Is another RDF instance
    def __add__(self, other):
        """rdf3 = rdf1 + rdf2

        add's rdf2's dictionary to rdf1's. Note:
        duplicte keys in rdf2 will overwrite rdf1's value, while
        maintining rdf1's order.
        """
        # shallow copy
        result = copy.copy(self)
        for key, field in other.items():
            result[key] = field
        return result

    ## See _-add__
    def __iadd__(self, other):
        for key, field in other.items():
            self[key] = field
        return self

    ## key --> rdf.data.entries.RDFRecord
    ## \param key a dictionary key (will be cast to a str)
    ## \retval record an RDFRecord of self[key].
    def record(self, key):
        """convert self[key] to an RDFRecord"""
        return self.RDFRecord(str(key), self.get(str(key)))

    ## get record() for all keys.
    def records(self):
        """Get all records from record()"""
        return map(self.record, self.keys())

    ## Get maximum index (column) of OPERATOR in file's strings
    def _max_index(self):
        """private formatter figures out max depth of '=' sign"""
        try:
            result = max(map(int, self.records()))
        except ValueError:
            result = 0
        return result

    ## Write to file, with some formatting
    ## \param dst file name (writable)
    ## \par Side Effects:
    ##  writes dst
    ## \returns
    def tofile(self, dst):
        """write data to a file"""
        with open(dst, 'w') as fdst:
            fdst.write(str(self))
        # return dst to make idempotent
        return dst

    ## Convert to a standard (key, value) ::DICT
    ## \param caster = lambda __: __ can caste fields to types.
    ## \return DICT
    def todict(self, caster=lambda __: __):
        """Convert to a normal dict()

        caster=callable key words casts the values to whatever caster
        decides. The default is the identity function. A typical usage
        is:

        RDF.todict(caster=str)

        which forces all the values to be string (e.g., if you are setting
        an environment variables: NUMBER_OF_THREADS = 32
        that 32 has to be a string '32', not an int 32.
        """
        # preserve dict type as OrderedDict if its enabled.
        result = DICT()
        for key, field in self.iteritems():
            result.update(DICT({key: caster(field.value)}))
        return result

    ## Make a copy of self.
    def copy(self):
        """make a copy if internal dictionary"""
        return copy.copy(self)

    def deepcopy(self):
        return copy.deepcopy(self)
    


