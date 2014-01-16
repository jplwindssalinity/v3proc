"""Header readers"""
## \namesapce pm.data.header So data parseres

import datetime
import rdf
from rdf import iRDF

TEST_FILES = ("QS_S0B20122861735.20132821036",
              "QS_SATT_20123172249.20123180617",
              "QS_SEPHG20122861735.20122870117",
              "QS_P1A20122861735.20132811504",
              "QS_S0A20130302337.20132731220")




## Anything longer than this is not a header line
WIDTH = 80

## A pointless character that appears in the header lines
TERMINATOR = ";"

## The header assignment operator
OPERATOR = "="


## A Header Record is a traditional RDF
class Header(list):
    pass
    

def header(src):
    return Header(read2rdfs(src))


def read2rdfs(src):
    result = []
    for list_ in read2lists(src):
        A = iRDF.RDFAccumulator()
        map(A, list_)
        result.append(A.rdf())
        pass
    return result

## Read non-zero header lines into a list - note:
## This preserves all non-zero header info
def read2lists(src):
    import rdf
    result = []
    input_list = parse(src)
    
    print 'not fully parsing due to bad SIS'
    return [input_list]
    
    if input_list:
        A = iRDF.RDFAnalyzer()
        num_header_records = A.value(input_list[0])
        m = []
        for i in xrange(num_header_records):
            if i:
                m.append(A.value(input_list[sum(m)]))
                result.append(input_list[sum(m[:-1]):sum(m)])
            else:
                num_el_rec = A(input_list[1])[0]
                ## Abort on 1 header record with ill name key words.
                if not num_el_rec.key == "num_header_elements":
                    result =  [input_list]
                    break
                m.append(int(num_el_rec.field.value))
                result.append(input_list[:m[0]])
                pass
            pass
        pass
    return result

## Put non-zero heads into a list
def parse(src):
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
class HeaderRecord(iRDF.RDFWrapper):

    ## It's just an RDF wrapper
    def __init__(self, rdf_, name=""):
        super(HeaderRecord, self).__init__(rdf_)
        self.name = name
    
    ## Note: Header[key] uses Header._rdf.get(key) to avoid casting\n
    ## Errors for missing/ambiguus keys are raise here.
    def __getitem__(self, key):
        try:
            result = super(HeaderRecord, self).__getitem__(key)
        except AttributeError:
            raise MissingKeyError("Header has no %s value" % str(key))
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
    def begin(self): return self.begin_datetime()
    
    @property
    def end(self): return self.end_datetime()
    
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

def test(i):
    return map(header, TEST_FILES)

