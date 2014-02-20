"""Non RDF specIfic python helpers"""
## \namespace rdf.utils python and rdf utilities
import functools
from rdf import reserved

from itertools import imap, ifilter


## Coroutine Decorator, returns it kick started
def coroutine(generator):
    """g = couroutine(generator)

    returns a coroutine with a kick started generator
    """
    @functools.wraps(generator)
    def starter(*args, **kwargs):
        """generic function interface"""
        # create generator
        co_routine = generator(*args, **kwargs)
        # kick-start it.
        co_routine.send(None)
        return co_routine
    return starter


## Coroutine yields None, until a full line is ingested, then it's yielded.
@coroutine
def cowrap(wrap=reserved.WRAP):
    """Yield None, or a whole, unwrapped, line"""
    line = None
    while True:
        # take in a line while yielding the last one
        line = yield line
        # True is it's continued
        while line.endswith(wrap):
            # get continued while yielding None
            dline = (yield None)
            # concatenate w/o the wrap character
            line = line.rstrip(wrap) + dline


## Return unwrapped lines from a file.
## \param src A src-file
## \param wrap = reserved.WRAP, the wrapping EOL character(s).
## \retval itertools.ifilter yields unwrapped lines.
def unwrap_file(src, wrap=reserved.WRAP):
    """unwrap_file(src, wrap=reserved.WRAP) generator"""
    with open(src, 'r') as fsrc:
        return unwrap_readlines(fsrc.readlines(), wrap=wrap)


## Return unwrapped lines from an fsrc.readlines() style list.
## \param line_list A list from file.readlines()
## \param wrap = reserved.WRAP, the wrapping EOL character(s).
## \retval itertools.ifilter yields unwrapped lines for line_list.
def unwrap_readlines(line_list, wrap=reserved.WRAP):
    """Return lines unwrapped relative to wrap='/'"""
    # strip the file's lines, unwrapped, chuck the Nones and blanks
    return ifilter(bool,
                   imap(cowrap(wrap=wrap).send,
                        imap(str.strip,
                             line_list)))


## A basic parsing of standard python 2x args and kwargs
## \param raw_line A python function def statement
## \retval tuple of (str, dict) of (signature, kwargs)
def parse_tuple_input(raw_line):
    """parse_tuple_input("1, 2, ..., n, a=1, b=2, ...., n=n")

    returns [1,2,..,n], {'a':1, 'b':2, ..., 'n':n}
    that is: args    ,   kwargs
    """
    # try to make a float (since this is for units)
    def cast(x):
        """nested"""
        try:
            z = float(x)
        except ValueError:
            z = str(x)
        return z

    # return [], kwarg
    def get_kwarg(item):
        """nested"""
        key, value = map(str.strip, item.split("="))
        return [], {key: cast(value)}

    # return [arg], {}
    def get_arg(item):
        """nested"""
        return [cast(item.strip())], {}

    # get arg or kwarg based on inp.__contains__("=")
    LOOK_UP_PARSER = {False: get_arg, True: get_kwarg}

    # split signature up
    inputs = raw_line.split(", ")
    args = []
    kwargs = {}
    for inp in inputs:
        arg, kwarg = LOOK_UP_PARSER["=" in inp](inp)
        args.extend(arg)
        kwargs.update(kwarg)

    return args, kwargs


## open a file picking GUI
## \param *args    for tkFileDialog.askopenfilename
## \param **kwargs for tkFileDialog.askopenfilename
## \retval filename from tkFileDialog.askopenfilename or raw_input
def dialog_pickfile(*args, **kwargs):
    """Use tkinter to make a dialog_pickfile GUI """
    from tkFileDialog import askopenfilename
    try:
        return askopenfilename(*args, **kwargs) or raw_input("Last Chance: ")
    except KeyboardInterrupt:
        pass
