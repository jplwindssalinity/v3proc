"""data-->RDF is THE RDF OBJECT"""
##\namespace rdf.data.files Usable data object for files.
import collections
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
    @functools.wraps(method)
    def stripped_method(self, key, *args):
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
    ## Make an instance from DICT argument
    ## \param dict_ is an rdf-enough dictionary
    ## \return RDF instance
    @classmethod
    def fromdict(cls, dict_):
        """instantiate from a DICT"""
        result = cls()
        for key, value in dict_.items():
            result[key] = value
        return result

    ## Make it from keyword arguments
    ## \param *args is an rdf-enough arguments (like dict)
    ## \param **dict_ is an rdf-enough dictionary
    ## \return RDF instance
    @classmethod
    def fromstar(cls, *args, **dict_):
        """instantiate from *args **dict_)"""
        # todo: (dict(*args) + dict_)
        rdf_ = cls()
        for pair in args:
            key, value = pair
            rdf_[str(key)] = value
        return rdf_ + cls.fromdict(dict_)

    ## Instaniate from a file
    ## \param src RDF file name
    ## \retval RDF an rdf instance
    @staticmethod
    def fromfile(src):
        """src -> RDF"""
        from rdf import rdfparse
        return rdfparse(src)

    ## Don't use init-- it's internal.
    ## \param *args Internal constructor takes *args
    def __init__(self, *args):
        """RDF(*args) matched dict built-in"""
        ## The data are an associative array-or the rdf spec is worthless.
        self._data = DICT(args)

    ## Get attr from ::DICT If needed
    ## \param attr Attritubute
    ## \retval self.attr OR
    ## \retval self._data.attr If needed
    def __getattr__(self, attr):
        try:
            result = object.__getattribute__(self, attr)
        except AttributeError:
            result = getattr(self._data, attr)
        return result

    ## rdf is rdf
    @property
    def rdf(self):
        return self

    ## Return value, vs "get" returns RDFField
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
        from rdf.data.entries import RDFField
        self._data[key] = RDFField(value)

    ## Delete key, field
    ## \param key an RDF key
    ## \par Side Effects:
    ## deletes key from RDF._data
    ## \returns None
    @stripper
    def __delitem__(self, key):
        self._data.__delitem__(key)

    ## iteritems()
    def __iter__(self):
        return iter(self._data.iteritems())

    ## Access as method or property (this is really for clients)
    def __call__(self):
        """self()->self so that x.rdf()()-->x.rdf()->x.rdf"""
        return self

    ## key --> rdf.data.entries.RDFRecord
    def record(self, key):
        """convery self[key] to an RDFRecord"""
        from rdf.data.entries import RDFRecord
        return RDFRecord(key, self.get(key))

    ## get record() for all keys.
    def records(self):
        """Get all records from record()"""
        return map(self.record, self.keys())

    ## Get maximum index (column) of OPERATOR in file's strings
    def _max_index(self):
        return max(map(int, self.records()))

    ## A nice formatted string
    def __str__(self):
        from rdf.data.entries import RDFRecord
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
        return self.tofile(dst)

    ## \verb{ rdf << src } Sucks up an new rdf file (see fromfile())
    ## \param src RDF file name
    ## \returns src + RDF
    def __lshift__(self, src):
        return self + type(self).fromfile(src)

    ## rdf << rdf' # lets rdf suck up an new rdf file.
    def __ilshift__(self, src):
        return self << src

    ## Add is concatenation, and is not communative
    ## \param other An RDF instance
    ## \retval result Is another RDF instance
    def __add__(self, other):
        result = self
        for key, field in other.items():
            result[key] = field
        return result

    ## See _-add__
    def __iadd__(self, other):
        self = self+other
        return self

    ## Write to file, with some formatting
    ## \param dst file name (writable)
    ## \par Side Effects:
    ##  writes dst
    ## \returns
    def tofile(self, dst):
        """write data to a file"""
        with open(dst, 'w') as fdst:
            fdst.write(str(self))
        ## return dst to make idempotent
        return dst

    ## Convert to a standard (key, value) ::DICT
    def todict(self, caster=lambda __: __):
        """Convert to a normal dict()

        caster key words casts the values

        """
        result = {}
        for key, field in self.iteritems():
            result.update({key: caster(field.value)})
        return result
