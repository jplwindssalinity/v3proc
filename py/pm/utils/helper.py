1## \namespace pm.utils.helper Generic and SpeciFic python helpers
"""helper/ module has python helper functions"""
import functools
import os

## A debug flag
VERBOSE = 0

## tbd cmd file extension
CMD = 'cmd'

## The <a href="http://www.perl.org/">perl</a> interpreter.
PERL = "/usr/bin/perl"


## Chain functions (from left to right)
def chain(*args):
    """chain(f1, f2, ..., fn)(x) --> f1(f2(...fn(x)))"""
    
    # compile nested function
    def chained(x):
        """function chain:"""
        for n, func in enumerate(reversed(args)):
            result = func(result if n else x)
        return result

    # make a doc string
    for func in reversed(args):
        try:
            doc = func.func_name
        except AttributeError:
            doc = type(func).__name__
            pass
        chained.__doc__ = chained.__doc__ + "\n    %s" % doc
    return chained



## @param fullpath A full path string: path/oldname
## @param basename A new basename (optionally: oldpath/basename)
## @param newpath  A new full path: path/basename
def new_basename(fullpath, basename):
    return os.path.join(
        os.path.dirname(fullpath),
        os.path.basename(basename)
        )


## strip and blanks, and anyting else
def clean_line(line, remove=()):
    line = line.strip()
    for item in remove:
        line = line.rstrip(item).strip()
        continue
    return line


## Run a perl file with arguments
def run_perl(perl_file, *args):
    import subprocess
    result = subprocess.check_call([PERL, perl_file] + list(args))
    return result


## glob file 
def glob_files(glob_str="*", top_dir=os.curdir):
    """glob files in a directory:

    glob_files(glob_str="*", top_dir=os.curdir)
    """
    import glob
    with DoThere(top_dir) as do_there:
        return do_there(glob.glob, glob_str)
    
    
## file finder
def find_files(directory, pattern):
    import os
    import fnmatch
    for root, dirs, files in os.walk(directory):
        for basename in files:
            if fnmatch.fnmatch(basename, pattern):  # guard
                filename = os.path.join(root, basename)
                yield filename
                continue
            continue
        continue
    pass


## Handle file names with knonw extensions that may or may not be there
def ensure_ext(ext, fname):
    import os
    if fname.endswidth(ext):  # guard
        return fname
    return fname + os.extsep + ext

## Ensure ".cmd" files
ensure_cmd = functools.partial(ensure_ext, CMD)


## mkdir with error catching on file exisits
def make_local_dir(dirname):
    """make_local_dir(dirname) --> os.mkdir(dirname)


    with errno.EEXIST exception protection.
    """
    try:
        os.mkdir(dirname)
    except OSError as err:
        ## If directory exists, then ignore
        import errno
        if not err.errno == errno.EEXIST:  # raise
            raise err
    return None


## A __init__ decorator factory for composite classes: \n
## Input: *types \n
## Output: A __init__ decorator that asserts *types matches *args, for each.
def type_check(*types):
    """type_check(*types)"""
    def init_decorator(init):
        def type_checked_init(self, *args, **kwargs):
            for type_, arg in zip(types, args):
                msg = ("composite class %s expected %s got %s" %
                       (self.__class__.__name__, type_.__name__,
                        arg.__class__.__name__))
                assert isinstance(arg, type_), msg
                continue
            ## Call class's __init__ .
            return init(self, *args, **kwargs)
        return type_checked_init
    return init_decorator


## A context manager for working in a diFferent directory
class DoThere(object):
    """A context manager:

    with DoThere(working_path) as do_there:
        do_there(func, *args, **kwargs)


    will go to working_path to run func(*args, **kwargs)
    and then return home.
    """
    ## Send working path to __init__
    def __init__(self, working_path):
        ## Where to execute function
        self.working_path = working_path
        ## Where we are now, and where we will finish
        self.home_path = os.getenv("PWD")

        ## A reminder to use in context
        self._usage = False
        return None

    ## Change DIR and return nself
    def __enter__(self):
        """with DoThere(<path>) as x:

        returns self bound to x,
        but with the working directory set to path
        """
        self._usage = True
        if self:  # guard on non with-as usage
            os.chdir(self.working_path)
        return self
    
    ## Return to original DIR
    def __exit__(self, arg1, arg2, arg3):
        if self:  # guard on non with-as usage
            os.chdir(self.home_path)
        return None

    ## Call func(*arg, **kwargs)
    def __call__(self, func, *args, **kwargs):
        if not self._usage:  # guard/raise
            raise SyntaxWarning(
                "DoThere must be used with the 'with...as...' syntax"
                )
        return func(*args, **kwargs)

    ## True IFF bool( DoThere.working_path )
    def __nonzero__(self):
        return bool(self.working_path)
    
    pass


## Generic list wrapper with *-magic signature
class StarListWrapper(object):

    ## A mangled decorator for wrapped methods of list
    def __doc(list_method):
        def method_decorator(method):
            def wrapped_method(self, *args, **kwargs):
                return method(self, *args, **kwargs)
            wrapped_method.__doc__ = list_method.__doc__
            return wrapped_method
        return method_decorator

    ## Build with star-magic.
    def __init__(self, *args):
        self._data = list(args)
        return None

    def __str__(self):
        return str(map(str, self))

    @property
    def data(self):
        return self._data

    @__doc(list.__getitem__)
    def __getitem__(self, index):
        return self.data[index]

    @__doc(list.__setitem__)
    def __setitem__(self, index, value):
        self._data[index] = value

    @__doc(list.__delitem__)
    def __delitem__(self, index):
        del self._data[index]

    @__doc(list.__len__)
    def __len__(self):
        return len(self.data)

    @__doc(list.__iter__)
    def __iter__(self):
        return iter(self.data)

    @__doc(list.append)
    def append(self, item):
        return self._data.append(item)

    @__doc(list.count)
    def _count(self, value):
        return self._data.count(value)

    @__doc(list.extend)
    def extend(self, iterable):
        return self._data.extend(iterable)

    @__doc(list.index)
    def index(self, value, *args):
        return self._data.index(value, *args)

    @__doc(list.insert)
    def insert(self, index, object_):
        self._data.insert(index, object_)

    @__doc(list.pop)
    def pop(self, index):
        return self._data.pop(index)
    
    @__doc(list.remove)
    def remove(self, value):
        return self._data.remove(value)

    @__doc(list.reverse)
    def reverse(self):
        return self._data.remove(value)

    @__doc(list.sort)
    def sort(self, cmp=None, key=None, reverse=False):
        return self._data.sort(cmp=cmp, key=key, reverse=reverse)


