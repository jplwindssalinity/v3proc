"""interval: module has one class: Interval(start, stop), who's
single responsibility is to represent a closed interval on an ordered
field.

Since we don't know what that field is apriori (though int, float, and
datetime are front runners), some arithmetic operations are decorated
thereby allowing subclasses to override some arithmetic operations).

Say what?

For example:

-[datetime, datetime'] --> [-datetime, -datetime'],

and that's a TypeError. But if the sub-class does:


@unary
def __neg__(self):
     return neg_datetime

where you define neg_datetime to work, BOOM: the operation works.
(neg_datetime, could for instance reflect w.r.t some fixed data).

The other decorators are:

@binary
@dilator
for binary operations on two Intervals and an Interval and scaling factor,
respectively.


Rich comparison for 2 intervals A, B is defined as follows:

A < B          A is totally earlier/less than B
A <= B         A is less/earlier than, but overlaps B from the left
A == B         total equality
A !=B          not A == B
A >=B          A is larger/later than B, but overlaps it form the right
A > B          A is totally larger/later than B

For field elements, x, y:
A < x
A <= x
A > x
A >= x

are cmp(y, x) for all y in A.

See operator docstring for definition of arithmetic operations:

+A
-A
-A
B in A, x in A
A & B
A | B
A + B
A - B
A >> x
A << x
A*x, x*A
A/x

Standard Functions:

int(A)
float(A)
abs(A)
bool(A)

Iterator/Container behavior:

iter(A)
A[index]

"""
import operator
import functools


## Unary Operator (for default operation on end points)
def unary(method):
    """Unary operation on Interval"""
    @functools.wraps(method)
    def unmethod(self):
        return type(self)(*map(method(self), self))
    return unmethod


## Binary Operator (for default operation on end points)
def binary(method):
    """Binary operation on 2 Intervals"""
    @functools.wraps(method)
    def bimethod(self, other):
        return type(self)(*[method(self)(left, right) for left, right in
                            zip(self, other)])
    return bimethod


## Dilation operation (for default operation on end points)
def dilator(method):
    """Binary operation on Intervals and a number (dare I say, scalar?)"""
    @functools.wraps(method)
    def dimethod(self, alpha):
        return type(self)(*[method(self)(item, alpha) for item in self])
    return dimethod


## A finite interval over an orderable field
class Interval(object):
    """interval = Interval(start, stop)

    start, stop:  almost anything that can be ordered, specially if
    start-stop    makes sense, and for sure if
    stop+stop     makes sense too, and an almost certainty if
    alpha*start   \
    start*alpha    \ some or all of these work.
    stop/beta      /
    beta/stop     /
    """
    __slots__ = ('start', 'stop')

    ## @param start starting point
    ## @param stop ending point 
    def __init__(self, start, stop):
        self.start = start
        self.stop = stop

    ## Closed interval string
    def __str__(self):
        return "["+"-".join(map(str, self))+"]"

    __repr__ = __str__

    ## @returns generator yields self[0], self[1]
    ## @throws StopIteration
    def __iter__(self):
        """iterate endpoints in order"""
        yield self[0]
        yield self[1]

    ## Container of endpoints
    def __getitem__(self, index):
        """[start, stop]"""
        return getattr(self, self.__slots__[index])

    ## Set-like "in"
    def __contains__(self, other):
        """a in [x, y] --> x <= a <= y \n
        [x', y'] in [x, y] --> x' in [x, y] and y' in [x, y]"""
        if isinstance(other, type(self)):
            return (other[1] in self) and (other[0] in self)
        return self[0] <= other <= self[1]

    ## Identity (if defined)
    @unary
    def __pos__(self):
        """+[x, y] --> [+x, +y]"""
        return operator.pos

    ## Reflection about 0
    @unary
    def __neg__(self):
        """-[x, y] --> [-x, -y]"""
        return operator.neg

    ## Transposition
    def __invert__(self):
        """~[x, y] --> [y, x]"""
        return type(self)(self[1], self[0])

    def __nonzero__(self):
        """non zero support"""
        return bool(float(self))

    ## [x, y] == [x', y'] --> x == x' and y == y'
    def __eq__(self, other):
        """ """
        return self[0] == other[0] and self[1] == other[1]

    ## [x, y] != [x', y'] --> not [x, y] == [x', y']
    def __ne__(self, other):
        """ """
        return self[0] != other[0] or self[1] != other[1]

    ## Less Than
    def __lt__(self, other):
        """[x, y] >= [x', y'] --> y < x'
        [x, y] >= a --> x > a and y > a"""
        if isinstance(other, type(self)):
            return self[1] < other[0]
        return max(self) < value

    ## Overlap Left
    def __le__(self, other):
        """[x, y] <= [x', y'] --> y in [x', y'] ' and x < x'
        [x, y] >= a --> x > a and y > a"""
        if isinstance(other, type(self)):
            return self[0] < other[0] and self[1] < other[1]
        return max(self) <= other

    ## Greater Than
    def __gt__(self, other):
        """[x, y] > [x', y'] --> x > y'
        [x, y] > a --> x > a and y > a"""
        if isinstance(other, type(self)):
            return self[0] > other[1]
        return min(self) > other

    ## Overlaps Right
    def __ge__(self, other):
        """[x, y] >= [x', y'] --> x in [x', y'] ' and y > y'
        [x, y] >= a --> x > a and y > a"""
        if isinstance(other, type(self)):
            return self[0] in other and self[1] > other[1]
        return min(self) > other

    ## Minimal Interval containing both
    def __or__(self, other):
        """[x, y] | [x', y'] --> [min(x, x'), max(y, y')]"""
        return type(self)(min(self[0], other[0]),
                          max(self[1], other[1]))

    ## Maximal Interval contained by both
    def __and__(self, other):
        """[x, y] & [x', y'] --> [max(x, x'), min(y, y')]"""
        return type(self)(max(self[0], other[0]),
                          min(self[1], other[1]))

    ## Add endpoints, pairwise
    @binary
    def __add__(self):
        """[x, y] + [x', y'] --> [x+x', y+y']"""
        return operator.add

    ## Subtract Endpoints, pairwise
    @binary
    def __sub__(self):
        """[x, y] - [x', y'] --> [x-x', y-y']"""
        return operator.sub

    ## Scale Endpoints
    @dilator
    def __mul__(self):
        """ [x, y]*a -> [x*a, y*a]"""
        return operator.mul

    ## Reflected multiplication for endpoint scaling
    @dilator
    def __rmul__(self):
        """a*[x, y] -> [a*x, a*y]"""
        return lambda x, y: y*x

    ## (inverse) Scale Endpoints
    @dilator
    def __div__(self):
        """[x, y]/a --> [x/a, y/a]"""
        return operator.div

    ## Increment right endpoint
    def __rshift__(self, delta):
        """[x, y] >> a --> [x, y+a]"""
        return type(self)(self[0], self[1]+delta)

    ## Decrement left endpoint
    def __lshift__(self, delta):
        """[x, y] << a --> [x-a, y]"""
        return type(self)(self[0]-delta, self[1])

    ## sgn(interval)
    def __int__(self):
        """-1  Reversed ordered
        0   Zero support
        1  Normal ordered"""
        import math
        return int(math.copysign(1, float(self))) if self else 0

    ## float of support() or _to_float()
    def __float__(self):
        """Try to convert support to a float"""
        try:
            return float(self.support())
        except TypeError:
            return float(self._to_float())

    ## | float |
    def __abs__(self):
        """abs(float(self))"""
        return abs(float(self))

    ## TBD float (if, e.g., support is a datetime.timedetla)
    def _to_float(self):
        raise NotImplementedError

    ## Size of Interval's support
    def support(self):
        """Difference of endpoints"""
        return self[1]-self[0]

    ## Normal order, inplace
    def sort(self):
        """INPLACE normal ordering of endpoints"""
        if self[0] > self[1]:
            self = ~self

    ## Apply func to endpoints
    def apply(self, func):
        """[x, y] --> [func(x), func(y)]"""
        return type(self)(*map(func, self))


## A class just from datetime intervals
class TimeInterval(Interval):
    """TimeInterval(datetime.datetime, datetime.datetime')

    See TimeInterval.__base__.__doc__ for more.
    """

    ## Convert support() to seconds
    def _to_float(self):
        return self.support().total_seconds()
