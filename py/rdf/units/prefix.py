"""Unit prefixes"""
## \namespace rdf.units.prefix Auto Prefix Handlers
import abc


## Abstract Base class for class decorator factory that makes an instance
## of the decorated _Unit subclass according to the prefix and the power.
class Prefix(object):
    """prefix = Prefix("symbol", exponent)

    INPUT:
           symbol   The prefix string symbol, e.g: "M" for mega
           exponent The exponent for the factor... 6   for 10**6

    OUTPUT:
            prefix   A class decorator that creates a new instance of
                     the decorated class (that must be a sub-class of _Unit)
                     See _Unit.__doc__ for why that works.

    @prefix
    class Dimension(_Unit)
          symbol = <what ever measure dimension>


    Note: Of course, you can stack them up.
    """

    ## Without a self.base, this class is no good
    __metaclass__ = abc.ABCMeta

    ## Sub's need a base to define what their prefixing means
    @abc.abstractproperty
    def base(self):
        """base"""

    @abc.abstractmethod
    def cast(self):
        """cast values to ...."""

    ## Construct with a symbol and an exponent
    ## \param symbol A string symbol that IS the abbreviation
    ## \param exponent sets the scale factor = base ** exponent
    def __init__(self, symbol, exponent):
        ## The prefix's official symbol
        self.symbol = str(symbol)
        ## \f$ f = B^x \f$
        self.factor = self.base ** exponent
        return None

    ## str(prefix) is the prefix's symbol.
    def __str__(self):
        return self.symbol

    ## Class decorator:
    ## \param cls A _Unit sub-class
    ## \par Side Effects:
    ## instantiate decorated instance and put into Glossary
    ## \retval cls Class decorators return classes.
    def __call__(self, n=1):
        """prefix(cls)-->cls'
        with SIDE EFFECTS"""

        ## Here we can look up base class to see if it
        ## a log on not?
        def decorator(cls):
            """decorator from factory"""
            ## Instantiate cls with prefixed name, and scale factor
            ## (with exponent)
            cls(str(self) + cls._symbol, self**n)
            return cls
        return decorator

    ## get factor and raise it to n
    def __pow__(self, n):
        return (self.cast()(self)) ** n


## <a href="http://en.wikipedia.org/wiki/Metric_prefix">Metric</a> Prefix.
class MetricPrefix(Prefix):
    """Prefix based on 10"""

    ## Metric is a Perfect 10
    base = 10

    ## cast() returns a function that calls this
    def __float__(self):
        return float(self.factor)

    ## float
    def cast(self):
        """cast values to float"""
        return float


## <a href="http://en.wikipedia.org/wiki/Binary_prefix">Binary</a> Prefix
## Note: limits/dIfferences of/between JEDEC and IEC
class BinaryPrefix(Prefix):
    """Prefix based on 1024"""

    ## \f$ 2^{10} \f$
    base = 1024

    ## cast() returns a function that calls this
    def __long__(self):
        return long(self.factor)

    ## long
    def cast(self):
        """cast values to long"""
        return long


## <a href="http://en.wikipedia.org/wiki/Yotta-">\f$10^{24}\f$</a>
yotta = MetricPrefix('Y', 24)
## <a href="http://en.wikipedia.org/wiki/Zetta-">\f$10^{21}\f$</a>
zetta = MetricPrefix('Z', 21)
## <a href="http://en.wikipedia.org/wiki/Exa-">\f$10^{18}\f$</a>
exa = MetricPrefix('E', 18)
## <a href="http://en.wikipedia.org/wiki/Peta-">\f$10^{15}\f$</a>
peta = MetricPrefix('P', 15)
## <a href="http://en.wikipedia.org/wiki/Tera-">\f$10^{12}\f$</a>
tera = MetricPrefix('T', 12)
## <a href="http://en.wikipedia.org/wiki/Giga-">\f$10^9\f$</a>
giga = MetricPrefix('G', 9)
## <a href="http://en.wikipedia.org/wiki/Mega-">\f$10^6\f$</a>
mega = MetricPrefix('M', 6)
## <a href="http://en.wikipedia.org/wiki/Kilo-">\f$10^3\f$</a>
kilo = MetricPrefix('k', 3)
## <a href="http://en.wikipedia.org/wiki/Hecto-">\f$10^2\f$</a>
hecto = MetricPrefix('h', 2)
## <a href="http://en.wikipedia.org/wiki/Deca-">\f$10^1\f$</a>
deca = MetricPrefix('da', 1)
## Trival (but it does create an instance and put it in _Unit.Glossary)
base = MetricPrefix('', 0)
## <a href="http://en.wikipedia.org/wiki/Deci-">\f$10^{-1}\f$</a>
deci = MetricPrefix('d', -1)
## <a href="http://en.wikipedia.org/wiki/Centi-">\f$10^{-2}\f$</a>
centi = MetricPrefix('c', -2)
## <a href="http://en.wikipedia.org/wiki/Milli-">\f$10^{-3}\f$</a>
milli = MetricPrefix('m', -3)
## <a href="http://en.wikipedia.org/wiki/Micro-">\f$10^{-6}\f$</a>\n
## (NB: \f$"u"\f$ is used instead of \f$"\mu"\f$ for typographical reasons)
micro = MetricPrefix('u', -6)
## <a href="http://en.wikipedia.org/wiki/Nano-">\f$10^{-9}\f$</a>
nano = MetricPrefix('n', -9)
## <a href="http://en.wikipedia.org/wiki/Pico-">\f$10^{-12}\f$</a>
pico = MetricPrefix('p', -12)
## <a href="http://en.wikipedia.org/wiki/Femto-">\f$10^{-15}\f$</a>
femto = MetricPrefix('f', -15)
## <a href="http://en.wikipedia.org/wiki/Atto-">\f$10^{-18}\f$</a>
atto = MetricPrefix('a', -18)
## <a href="http://en.wikipedia.org/wiki/Zepto-">\f$10^{-21}\f$</a>
zepto = MetricPrefix('z', -21)
## <a href="http://en.wikipedia.org/wiki/Yocto-">\f$10^{-24}\f$</a>
yocto = MetricPrefix('y', -24)


## Trival (integer measurement)
base2 = BinaryPrefix('', 0)
## \f$ 2^{10} \f$, JEDEC
kilo2 = BinaryPrefix('k', 1)
## \f$ (2^{10})^2 \f$, JEDEC
mega2 = BinaryPrefix('M', 2)
## \f$ (2^{10})^3 \f$, JEDEC
giga2 = BinaryPrefix('G', 3)

## \f$ 2^{10} \f$, IEC
kibi = BinaryPrefix('Ki', 1)
## \f$ (2^{10})^2 \f$, IEC
mebi = BinaryPrefix('Mi', 2)
## \f$ (2^{10})^3 \f$, IEC
gibi = BinaryPrefix('Gi', 3)
## \f$ (2^{10})^4 \f$, IEC
tebi = BinaryPrefix('Ti', 4)
## \f$ (2^{10})^5 \f$, IEC
pebi = BinaryPrefix('Pi', 5)
## \f$ (2^{10})^6 \f$, IEC
exbi = BinaryPrefix('Ei', 6)
## \f$ (2^{10})^7 \f$, IEC
zebi = BinaryPrefix('Zi', 7)
## \f$ (2^{10})^8 \f$, IEC
yebi = BinaryPrefix('Yi', 8)
## TBD: pick one.
yobi = yebi


## Decorate All Metric Prefixes
def metric(n=1):
    """metric(n=1) decorates all metric prefixes, with dimensionality = n"""
    def pmetric(cls):
        """nested: apply all metric prefixes with "n" """
        @exa(n)
        @peta(n)
        @tera(n)
        @giga(n)
        @mega(n)
        @kilo(n)
        @base(n)
        @centi(n)
        @milli(n)
        @micro(n)
        @nano(n)
        @pico(n)
        @femto(n)
        @atto(n)
        class _Dummy(cls):
            """_Dummy cls memoized unit"""
        return cls
    return pmetric


## Decorate JEDEC prefixes
def jedec(cls):
    """Apply all JEDEC decorators to cls"""
    return kilo2()(mega2()(giga2()(cls)))


## Decorate IEC prefixes
def iec(cls):
    """Apply all IEC decorators to cls"""
    return kibi()(mebi()(gibi()(tebi()(pebi()(exbi()(zebi()(yobi()(cls))))))))
