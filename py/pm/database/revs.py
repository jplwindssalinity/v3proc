"""Interface to the revs data base.

DB             is the database filename
State          reps a processing state
Row            Rev's dependents.
Rev            reps a Rev (number) with a Row.
RevDataBase    is an interface into a database, with proper pythonicity

See:
http://docs.python.org/2/library/sqlite3.html
http://www.python.org/dev/peps/pep-0249/
"""
## \namespace pm.database.revs Rev Database interface
import datetime
import itertools
import sqlite3 as lite
   
__version__ = "2.0"

## Partial implementation of
## <a href="http://docs.python.org/2/library/datetime.html?highlight=strptime#datetime.datetime.strptime>
## datetime.strptime</a>## 
## @param string A datetime stirng that matches iso
## @param iso "%Y-%m-%d %H:%M:%S.%f" - prolly the only allowed format
## @throws ValueError if format doesn't work out
def str2time(string, iso="%Y-%m-%d %H:%M:%S.%f"):
    return datetime.datetime.strptime(str(string), iso)

## The eponymous database is Alex's file
FILENAME = "alex.db"

## Generic PM Database Error
class PMDatabaseError(UserWarning):
    pass
    
## UserWarning for nonsense inputs.
class UnknownStateError(PMDatabaseError):
    """Raised when setting processing state to an undefined value"""

## UserWarning for working on Revs not in the ::DB.
class NonExistentRevError(PMDatabaseError):
    """Raised when operating on a non-extent rev"""

## Usage is TBD, basically an ENUM describing processing state of a REV.
class State(object):
    """State(state)

    state is an integer key into the State.states dictionary

    This class represents all the possible states of Rev processing,
    as seen in the static states dictionary. Instances support:

    bool(state)      True is the Rev is 'in the mix'
    int(state)       The code
    str(state)       The decode
    state == int     equality testing
    state != int     inequality testing

    rich comparison is not supported, unless a definite order to processing
    states is decided.

    The purpose of this object is to avoid 2 antipatterns, which
    are codified in C as the 'ENUM' type:

    http://c2.com/cgi/wiki?ZeroMeansNull
    http://c2.com/cgi/wiki?MagicNumber
    """
    
    ## Class wraps one integer
    __slots__ = ('state',)

    ## Some guesses at what a processing state might be
    states = {0: 'Non Existent',
              1: 'Downlinked',
              2: 'Waiting for GPS/EPHM',
              3: 'Waiting for science telemetry',
              4: 'Processing at Level 1A',
              5: 'Processing at Level 1B',
              6: 'Processing at Level 2A',
              7: 'Processing at Level 2B',
              8: 'Processing at Level 1Bv3',
              9: 'Processing at Level 2Av3',
              10: 'Processing at Level 2Bv3',
              11: 'in NETCDF Conversion',
              42: 'Manual Reprocessing',
              -1: 'Processed',
              -2: 'Reprocessed',
              666: 'Abandoned',
              667: "classified s**t"}

    ## keys to states that are in use
    _in_use = xrange(2, 12)

    ## Partial format for sqlite3 definition
    format = 'state number'

    ## string value in sqlite3 interface
    def __repr__(self):
        return str(self.state)

    ## A value-checked init
    ## @param state An int (or int-like) in States.state
    ## @throws UnknownStateError self explanatory.
    ## @throws TypeError if state can't be caste to int.
    def __init__(self, state):
        if int(state) not in self.states:  # guard on bad value.
            raise UnknownStateError
        ## The state code
        self.state = int(state)

    ## int is the encoding of the ENUM \n
    ## Note that: State(int(State)) == State
    def __int__(self):
        return self.state

    ## str is the decoding of the ENUM
    def __str__(self):
        return self.states[int(self)]

    ## Comparison with int or other State instance
    def __eq__(self, other):
        return int(self) == int(other)

    ## Comparison with int or other State instance
    def __ne__(self, other):
        return int(self) != int(other)

    ## A State is True if it IN-USE
    ## @retval bool If state represents State._in_use
    def __nonzero__(self):
        return int(self) in self._in_use

    ## Row = State + (start, stop)
    ## @param tup A tuple of 2-dates
    ## @retval row A Row instance
    def __add__(self, tup):
        return Row(tup[0], tup[1], self)


## A Row is the Rev information in the database
class Row(object):
    """Row(start, stop, state=0)

    The purpose of this class is to capture primitives that
    depend in the rev number.

    It adheres to 'composition before inheritance'.
    """

    ## Partial format for sqlite3 definition
    format = 'start text, stop text, ' + State.format

    ## string value in sqlite3 interface
    def __repr__(self):
        return "%s, %s, %s" % (str(self.start),
                               str(self.stop),
                               repr(self.state))

    ## Define what is in a row here.
    __slots__ = ('start', 'stop',) + State.__slots__

    ## A type-checked init.
    ## @param start rev start time (datetime instance)
    ## @param stop rev stop time (datetime instance)
    ## @param state =0, a state code (or State object).
    ## @throws TypeError for bad types
    ## @throws UnknownStateError for state.
    def __init__(self, start, stop, state=0):
        for attr, arg in zip(self.__slots__, (start, stop)):
            # A hideous type converter.
            if isinstance(arg, datetime.datetime):
                value = arg
            elif isinstance(arg, basestring):
                value = str2time(arg)
            else:
                msg = "Row expected datetime, got %s" % type(arg).__name__
                raise TypeError(msg)
            setattr(self, attr, value)
            continue
        self.state = State(state)

    ## A Row is True when it is in-use by pm.
    ## @retval bool If state means "in use".
    def __nonzero__(self):
        return bool(self.state)

    ## @retval str ASCII- for humans
    def __str__(self):
        # can't use iter() b/c of int cast on state, so back to basics.
        return "%s\t%s\t%s" % tuple(map(str, (getattr(self, attr) for
                                              attr in self.__slots__)))

    ## Forward to state
    def __eq__(self, state):
        int(self.state) == int(state)

    ## Forward to state
    def __ne__(self, state):
        int(self.state) != int(state)

    ## @param rev_number rev number
    ## @retval rev A full-blown Rev object.
    def __add__(self, rev_number):
        return Rev(rev_number, self)

    ## @returns a primitive generator
    ## @throws StopIteration
    def __iter__(self):
        yield self.start
        yield self.stop
        yield int(self.state)  # must cast to sqlite3 allowed type.

    ## @param implicit Row.start
    ## @param implicit Row.stop
    ## @retval interval A pm.utils.interval.TimeInterval for the Rev.
    def interval(self):
        from pm.utils.interval import TimeInterval
        return TimeInterval(self.start, self.stop)

## A Rev is a single row in the database
class Rev(object):
    """Rev(rev, row)

    The purpose of this object is to rep a row in the database
    via composition.

    
    Rev's instances support:

    bool(rev)         if the rev is 'in the mix'
    int(rev)          The rev number
    str(rev)          An ASCII table row
    repr(rev)         An ASCII TBD
    iter(rev)         Iterates over a rev + row entries

    It exists to alleviate sqlite3's built-in antipattern:

    http://c2.com/cgi/wiki?PrimitiveObsession

    which says tuples are rows.
    """

    ## Partial format for sqlite3 definition, include the __unique__ thing
    format = 'rev number unique, ' + Row.format

    ## Number of digits used to display a Rev number.
    ZFILL = 5

    ## Define what is in a row here.
    __slots__ = ('rev', 'row')

    ## Class constructor: from a tuple of primitives
    ## @param args is (rev, datetime, datetime, state)
    ## @retval instance cls, constructed.
    @classmethod
    def fromtuple(cls, args):
        return cls(args[0], Row(*args[1:]))

    ## @param rev A rev number
    ## @param row A Row instance
    def __init__(self, rev, row):
        ## int rev number
        self.rev = int(rev)
        ## Row object
        self.row = row

    @property
    def state(self):
        return self.row.state

    ## forwarding method
    @state.setter
    def state(self, state):
        self.row.state = state

    ## forwarding method
    @property
    def start(self):
        return self.row.start

    ## forwarding method
    @start.setter
    def start(self, start):
        self.row.start = start

    ## forwarding method
    @property
    def stop(self):
        return self.row.stop

    ## forwarding method
    @stop.setter
    def stop(self, stop):
        self.row.stop = stop

    ## forwarding method
    def __eq__(self, state):
        self.row == int(state)

    ## forwarding method
    def __ne__(self, state):
        self.row != int(state)

    ## forwarding method: Row.interval
    def interval(self):
        return self.row.interval()

    ## A Rev is True when it is in-use by pm.
    ## @retval bool If state means "in use".
    def __nonzero__(self):
        return bool(self.row)

    ## @retval str ASCII- for humans
    def __str__(self):
        return "%s\t%s" % (str(self.rev).zfill(self.ZFILL), str(self.row))

    ## @retval representation for db insertion (TBD)
    def __repr__(self):
        return ", ".join(map(repr, self))

    ## Iterate into format for sq3lite's INSERT operation.
    ## @returns a primitive generator
    ## @throws StopIteration
    ## @throws TypeError if Rev.row is not right (iterable)
    def __iter__(self):
        yield self.rev
        for item in self.row:
            yield item

    ## @param db insert() into db
    def __add__(self, db):
        return self.insert(db)

    ## The int() is the rev
    def __int__(self):
        return self.rev

    ## Insert Rev into database
    ## @param db =::DBinsert() into db
    def insert(self, db=None):
        db = db or DB
        return db + self

    ## @param lon longitude (defaults to antipattern: -999)
    def to_csv(self, lon=-999.):
        import revlist
        # is this the right string for revlist's format?
        d1 = str(self.start)
        d2 = str(self.stop)
        # how do you invert fromtimestamp?
        t1 = NotImplemented
        t2 = NotImplemented
        raise NotImplementedError(
            "don't know how to invert datetime.datetime.fromtimestamp"
            )
        return revlist.RevRow(self.rev, t1, t2, d1, d2, float(lone))


## Interface to Database -pretty much a Singleton, but not:
## it only talks to 1 kind of db, and that db should always be ::DB.
class RevDataBase(object):
    """db = RevDataBase(filename=FILENAME)

    The purpose of this object is provide a super simple interface
    to an sqlite3 database, without have to know anything about
    sqlite3, databases, connections, cursors, etc...

    The instance does not stay connected to the database; rather for
    each operation requiring access to the database, a new connection
    is created in a context manager, a all the business happens in
    the context.

    +db              Calls db.create().
    db.create()      Creates the database and defines the TABLE base on what
                     a Rev is.

    The money methods for nominal usage are:

    Container Behavior:
    db[rev]          get the info for a rev
    db[rev] = stat   set the info for  rev
    del db[rev]      remove a rev from the table

    Iteration:
    iter(db)         yields rev-ordered Rev object

    Other:
    db + rev         add a Rev to the database
    db.add(*args)    adds a Rev created from primitives.
    """
    ## TABLE name
    table = 'revs'
    ## The REV Number is the unique key
    key = 'rev'
    ## sqlite3 format for a row
    format = '%s, %s' % (key, Row.format)

    ## A header for str().
    header = 'Rev #   Rev Begin Time                  Rev End Time                    Status\n'

    ## @param filename = ::FILENAME, the database's filename
    def __init__(self, filename=FILENAME):
        self.filename = str(filename)

    ## +self --> create() \n
    ## __Side__ __Effects__: Creates the database
    ## @retval self
    ## @throws sqlite3.OperationalError (may be caught).
    def __pos__(self):
        self.create()
        return self

    ## DESTROY SELF
    def __neg__(self):
        import os
        os.remove(self.filename)


    ## @retval str ASCII version of the database, with a RevDatabase.header
    def __str__(self):
        return self.header + (
            '\n'.join(map(str, [self[n] for n in [rev.rev for rev in self]]))
            )

    ## @retval str junk
    def __repr__(self):
        return "\n".join(map(repr, self))

    ## Iterate over Rev items.
    ## @param None
    ## @retval generator yields Rev objects
    def __iter__(self):
        with lite.connect(self.filename) as conn:
            c = conn.cursor()
            for rev in c.execute('SELECT * FROM %s ORDER BY %s' %
                                 (self.table, self.key)):
                yield Rev.fromtuple(rev)

    ## DESIGN PROBLEM: see setitem, to fix that, them iter() and
    ## getitem return DIFFERENT objects.....
    ## @param rev A rev number
    ## @retval rev A Rev instance
    def __getitem__(self, rev):
        """Get Rev object for rev"""
        with lite.connect(self.filename) as conn:
            c = conn.cursor()
            # todo: review this command for ? alternative
            c.execute('SELECT * FROM revs WHERE rev=%s' % str(int(rev)))
            row = c.fetchone()
            result = Rev.fromtuple(row) if row else None
            return result

    ## DESIGN PROBLEM: DB[4] = DB[4] raises and error for values of 4!
    ## @param rev A rev number
    ## @param state A Row instance
    ## @returns None
    ## @throws UnknownStateError if state makes no sense
    def __setitem__(self, rev, row):
        """db[rev] = row  # NOT Rev,....?,, TBD
        db[rev] = State + (datetime, datetime)"""
        # todo: replace with: self + Rev(rev, *state)
        self + Rev(rev, row)

    ## delete a rev from the table
    def __delitem__(self, rev):
        raise NotImplementedError

    ## Rev is in the Table
    def __contains__(self, rev):
        return self[int(rev)] is not None

    ## @param rev_ A Rev object:
    ## @retval error_code An old-school errno.
    ## @throws UnknownStateError if state makes no sense
    ## @throws sqlite3.OperationalError if database is locked
    ## @throws sqlite3.InterfaceError probably unsupported type
    ## @throws sqlite3.ProgrammingError for wrong number of arguments in rev_
    def __add__(self, rev_):
        with lite.connect(self.filename) as conn:
            c = conn.cursor()
            try:
                c.execute("INSERT OR REPLACE INTO revs VALUES (?, ?, ?, ?)",
                          tuple((__ for __ in iter(rev_))))
            except lite.IntegrityError as err:
                print err
                print 'with rev=', rev_
                error_code = -1  # a TBD errno or sqlite3 error code.
            else:
                conn.commit()
                error_code = 0
            return error_code

    ## Create the TABLE that defines stuff (call this ONCE).
    ## @param None
    ## @retval None
    def create(self):
        msg = 'CREATE TABLE %s (%s)' % (self.table, Rev.format)
        with lite.connect(self.filename) as conn:
            c = conn.cursor()
        try:
            c.execute(msg)
        except lite.OperationalError as err:
            print 'An Operational Error Occurred: ', str(err)
        else:
            print 'Created Table'

    ## Add a NEW rev to the table from primitives
    ## @param *args Whatever is required to define a row.
    ## @retval error_code from __add__().
    ## @throws UnknownStateError if state makes no sense
    def add(self, *args):
        """add(rev, rev, start, stop, [state])

        int(rev) is the new (unique) rev number
        int(state) is the TBD processing state
        """
        error_code = self + Rev.fromtuple(args)
        return error_code

    ## Update the state field
    ## @param rev A rev number
    ## @param state A State (or int) to update rev.
    ## @returns Nothing, it's a procedure.
    ## @throws NonExistentRevError
    def change_state(self, rev, state):
        try:
            rev_ = self[rev]
            row = rev_.row
            # todo: refactor this Law of Demeter violation:
            row.state = State(state)
            self[rev] = row
        except TypeError:
            raise NonExistentRevError
        else:
            pass  # todo: refactor try/except to isolate error.

    def change_endtime(self, rev, dt):
        return NotImplemented


## "THE" Database interface, for all to use simultaneously.
DB = RevDataBase(FILENAME)

# development stuff:
now = datetime.datetime.now

def test(rev=0, state=0):
    # step 1: create the sqlite3 database:
    +DB
    # get some time stubs.
    hour = datetime.timedelta(0, 3600)
    now_ = now()
    # put some Revs in to the data base
    for n in range(12):
        state = State(n)  # stub the 'state' field

        # add a date tuple to state to make a Row, then insert it DB rev=n
        DB[n] = state + (now_ + n*hour, now_ + (n+1)*hour)
        continue
    # Abandon Rev 4
    DB.change_state(4, 666)
    #
    # lets looks at the database:
    #
    for n in range(100,10100):
        print n
        DB[n] = State(42) + (now(), now())

    print
    print str(DB)
    print
    #
    #
    # Who is 'in' PM:
    print
    for rev in filter(bool, DB):
        print rev
    print
    #
    # What is Rev 4 doing?
    #
    print DB[4]
