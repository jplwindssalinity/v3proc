"""Header readers"""
## \namesapce pm.data.header So data parseres

import datetime

TEST_FILES = ("QS_S0B20122861735.20132821036",
              "QS_SATT_20123172249.20123180617",
              "QS_SEPHG20122861735.20122870117",
              "QS_P1A20122861735.20132811504",
              "QS_S0A20130302337.20132731220")




## A header is a list of HeaderRecords
class Header(list):
    pass
    


## A Header Record is a traditional RDF
class HeaderRecords(object):
    return Header(src2rdf(src), name=src)

## Anything longer than this is not a header line
WIDTH = 80

## A pointless character that appears in the header lines
TERMINATOR = ";"

## The header assignment operator
OPERATOR = "="


def header(src):
    return Header(src2rdf(src), name=src)

## Make an RDF;
def src2rdf(src):
    import rdf
    return rdf.RDF.fromdict(src2dict(src))

## Perserve header info
def src2dict(src, ordered=True):
    from collections import OrderedDict
    dict_ = OrderedDict if ordered else dict

    result = dict_()

    ## We need to loop (explicitly) to handle duplicate keys
    for line in read(src):
        key, value = splitline(line)
        if not value:                   # guard to skip blanks
            continue                    # (e.g, spare_metadata field).
        if result.has_key(key):
            # get old value
            old_value = result[key]
            # append for the 1st time
            if isinstance(old_value, basestring):
                result[key] = [old_value, value]
            # append for the n-th time
            else:
                result[key] = old_value + [value]
                continue
            continue
        # it is a new key, so set it.
        result[key] = value
        continue
    return result


## Read non-zero header lines into a list - note:
## This preserves all non-zero header info
def read(src):
    return filter(bool, map(r_strip, pre_read(src)))


## open src and pre_readf() it in a context manager
def pre_read(src):
    with open(src, 'r') as fsrc:
        return pre_readf(fsrc)

## read ascii lines out of head of fsrc
def pre_readf(fsrc, rewind=True):
    if rewind: fsrc.seek(0)
    result = []
    while True:
        line = fsrc.readline()
        ## Bail if line is too long or a non ASICII
        if len(line) > WIDTH or not isascii(line):
            break
        result.append(line)
        continue
    return result


## IFF the line is ASCII
def isascii(line):
    try:
        str.decode(line)
    except UnicodeDecodeError:
        print "not ascii read and rejected"
        return False
    return True


## Remove trailing spaces and ::TERMINATOR
def r_strip(line):
    return line.strip().rstrip(TERMINATOR).strip()

## Make line into a (key, value) tuple
def splitline(line):
    return tuple(map(str.strip, line.split(OPERATOR)))



## Header object wraps a file's RDF values.
class Header(object):
    

    ## It's just an RDF wrapper
    def __init__(self, rdf_, name=None):
        self._rdf = rdf_
        self.name = name
        return None
    
    ## Note: Header[key] uses Header._rdf.get(key) to avoid casting\n
    ## Errors for missing/ambiguus keys are raise here.
    def __getitem__(self, key):
        try:
            result = self._rdf.get(key).value
        except AttributeError:
            raise MissingKeyError("Header has no %s value" % str(key))
        if isinstance(result, list):
            raise AmbiguousKeyError("Mutliple Values for %s" % str(key))
        return result


    ## key better return a yyyy-doy
    def _parse_date(self, key):
        year, doy = map(int, self[key].split('-'))
        return (datetime.date(year, 1, 1) +
                datetime.timedelta(doy-1))
    
    ## key better return a hh:mm:ss.sss
    def _parse_time(self, key):
        hour, minute, sec = self[key].split(":")
        hour = int(float(hour))
        minute = int(float(minute))
        sec = float(sec)
        
        i_sec = int(sec//1)
        frac_sec = sec - i_sec
        mu_sec = int(1e6 * frac_sec)

        return datetime.time(hour, minute, i_sec, mu_sec)

    ## This thing needs a date and time object from datetime
    def _add_date_time(self, date, time):
        result = datetime.timedelta(0, time.second, time.microsecond,
                                     0, time.minute, time.hour)
        result += datetime.datetime(date.year,
                                    date.month,
                                     date.day)
        return result


    def begin_date(self): return self._parse_date("RangeBeginningDate")
    def end_date(self): return self._parse_date("RangeEndingDate")
    def begin_time(self): return self._parse_time("RangeBeginningTime")
    def end_time(self): return self._parse_time("RangeEndingTime")
        
    def begin_datetime(self):
        return self._add_date_time(self.begin_date(), self.begin_time())
    
    def end_datetime(self):
        return self._add_date_time(self.end_date(), self.end_time())
    
    @property
    def begin(self):
        return self.begin_datetime()

    @property
    def end(self):
        return self.end_datetime()
    
    ## true iff self ends before other starts
    def __lt__(self, other):
        return self.end < other.begin
    
    ## true iff self begins after other ends
    def __gt__(self, other):
        return self.begin > other.end
    
    ## IDENTICAL time stamps for begin/end.
    def __eq__(self, other):
        return ( self.begin == other.begin
                 and
                 self.end == other.end)

    ## self ends during other
    def __le__(self, other):
        return other.begin < self.end  <= other.end

    ## self starts during other
    def __ge__(self, other):
        return other.begin <= self.begin < other.end

    ## There is NO overlap
    def __ne__(self, other):
        return (self < other) or (self > other)

    ## other is contained in self's time stamp.
    def __contains__(self, other):
        return self(other.begin) and self(other.end)

    ## A datetime is in range
    def __call__(self, datetime_):
        return self.begin <= datetime_ <= self.end


    ## Length in seconds
    def __abs__(self):
        delta = self.end - self.begin
        return (86400. * delta.days +
                1. * delta.seconds +
                delta.microseconds/1.e6)


    ## check if time field exists.
    def hastime(self):
        try:
            self.begin, self.end
        except MissingKeyError:
            return False
        return True

    ## Not so good
    def glob_matches(self, top_dir='./'):
        from pm import utils
        result = []
        for src in utils.glob_files(glob_str="Q*", top_dir=top_dir):
            hsrc = header(src)
            if hsrc and hsrc.hastime():
                if hsrc != self:
                    continue
                result.append(src)
                continue
            continue
        return result


    ## show times...
    def showtime(self):
        print self.begin
        print self.end
        return None


    ## Factory method to make a processor.
    def create_processor(self):
        return PROCESSOR_FACTORY[self['ShortName']]




def test0():
    from pm.utils import DoThere
    tester = lambda : map(header, TEST_FILES)
    with DoThere('/Users/belz/python/pm/data') as dothere:
        return dothere(tester)

def test(idx=-1):
    src = TEST_FILES[idx]
    a1 = read(src)
    a2 = src2rdf(src)
    a3 = header(src)
    return a1, a2, a3
