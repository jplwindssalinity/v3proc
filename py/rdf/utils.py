"""Non RDF specIfic python helpers"""
## \namespace rdf.utils Non-RDF specIfic utilities
import functools
from rdf import reserved


## Coroutine Decorator, returns it kick started
def coroutine(generator):
    @functools.wraps(generator)
    def starter(*args, **kwargs):
        co_routine = generator(*args, **kwargs)
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
def unwrap_file(src, wrap="/"):
    """Return lines unwrapped relative to wrap='/'"""
    with open(src, 'r') as fsrc:
        # strip the file's lines, unwrapped, chuck the Nones and blanks
        return filter(bool,
                      map(cowrap(wrap=wrap).send,
                          map(str.strip,
                              fsrc.readlines())))


## A basic parsing of standard python 2x args and kwargs
def parse_tuple_input(raw_line):
    """parse_tuple_input("1, 2, ..., n, a=1, b=2, ...., n=n")

    returns [1,2,..,n], {'a':1, 'b':2, ..., 'n':n}
    that is: args    ,   kwargs
    """
    # try to make a float (since this is for units)
    def cast(x):
        try:
            z = float(x)
        except ValueError:
            z = str(x)
        return z

    # return [], kwarg
    def get_kwarg(item):
        key, value = map(str.strip, item.split("="))
        return [], {key: cast(value)}

    # return [arg], {}
    def get_arg(item):
        return [cast(item.strip())], {}

    # get arg or kwarg based on inp.__contains__("=")
    LOOK_UP_PARSER = {False: get_arg, True: get_kwarg}

    # split signature up
    inputs = raw_line.split(",")
    args = []
    kwargs = {}
    for inp in inputs:
        arg, kwarg = LOOK_UP_PARSER["=" in inp](inp)
        args.extend(arg)
        kwargs.update(kwarg)

    return args, kwargs


## open a file picking GUI
def dialog_pickfile(*args, **kwargs):
    """Use tkinter to make a dialog_pickfile GUI """
#    import tkMessageBox
#    from tkColorChooser import askcolor
    from tkFileDialog import askopenfilename
    return askopenfilename(*args, **kwargs) or raw_input("Last Chance: ")
