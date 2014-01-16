## \namespace pm.database.revlist Read csv rev-list / Write database.
import collections
import datetime

REVLIST = '/u/pawpaw-z0/fore/rapidscat0/gse_preprocess/revlist.csv'
REVLIST = 'revlist.csv'

## The function
## @param src = ::REVLIST
## @retval table a Table.
def reader(src=REVLIST):
    with open(src, 'r') as fsrc:
        return make_table(*fsrc.readlines())

## @param *args Lines from the file
## @retval table a Table.
def make_table(*args):
    return Table(map(parse_line, args))


## @param line from the ::REVLIST
## @retval revrow A RevRow.
def parse_line(line):
    line = line.strip()
    return revrow(*line.split(','))


## Convert string entries to a RevRow
## @param rev column 1
## @param t1 column 2
## @param t2 column 3
## @param d1 column 4
## @param d2 column 5
## @param lon column 6
## @retval revrow A RevRow.
def revrow(rev, t1, t2, d1, d2, lon):
    return RevRow(int(rev),
                  float(t1), float(t2),
                  str(d1), str(d2),
                  float(lon))


## convert float strings to datetime
## @param str floating point time string
## @retval datetime The fromtimestamp result
def t2date(t):
    return datetime.datetime.fromtimestamp(float(t))


## convert date strings to datetime TBD: FORMAT.
## @param d date string in format
## @param format '%Y-%jT%H:%M:%S.%f'
## @retval datetime strptime result
def d2date(d, format='%Y-%jT%H:%M:%S.%f'):
    return datetime.datetime.strptime(d, format)


## A Row in the Table.
class RevRow(collections.namedtuple("RevRow",
                                    "rev fstart fstop start stop lon")):
          
    def __str__(self):
        return ", ".join(map(str, self))

    def __int__(self):
        return int(self.rev)

    def __eq__(self, int_):
        return int(self) == int_

    @property
    def start_date(self):
        return t2date(self.fstart)
    
    @property
    def stop_date(self):
        return t2date(self.fstop)
    
    def interval(self):
        from pm.utils import interval
        return interval.TimeInterval(self.start_date, self.stop_date)
    
    def to_db(self):
        from pm.database import revs
        return revs.Rev.fromtuple(
            (int(self), self.start_date, self.stop_date, revs.State(-1))
            )


class Table(list):
    """Use table = reader(src) to create from a file.

    To make into a database:
    
    table >> "filename.db"
    """

    ## use rev #, not list index
    def __getitem__(self, index):
        for item in self:
            if item == index:
                return item

    ## use rev #, not list index
    def __contains__(self, index):
        for item in self:
            if item == index:
                return True
        return False

    def __setitem__(*args):
        raise TypeError

    def __delitem__(*args):
        raise TypeError
        
    def __rshift__(self, filename):
        return self.to_db(filename=filename)

    def to_db(self, filename='revs.db'):
        from pm.database import revs
        db = +revs.RevDataBase(filename=filename)
        for item in self:
            print 'adding:', str(item)
            db + item.to_db()


def test():
    return reader()
