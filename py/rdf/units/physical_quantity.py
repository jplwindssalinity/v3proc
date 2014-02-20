#pylint: disable=R0901
"""Physical Quantities are defined here. They are all sub classes of _Unit-and
that's what counts. Other private classes are only for organization: they
impart no functionality.

So, to make your own unit that is called the "mine" and measure the amount
of stuff:


@base
class Stuff(_Unit):
    _symbol = "Mine"

is all it takes. The symbol is defined by the string, and the @base
decorator puts it in the GLOSSARY. To define your tithing, you could
do:


@base
@deci
class Stuff(_Unit):
    _symbol = "Mine"


which also puts the "dMine" in the dictionary, such that 1 dMine is 10% of
your stuff. Should you need a not standard version, say your 34% tax
margin, then you go to the addendum module and add:

taxes = Stuff('tax', 0.34)

Then:

GLOASSARY['tax'](1.00) = .34
GLOASSARY['tax'].symbol = Mine

converts the value (100% of your stuff) to (34% of a Mine). (the taxes
variable is irrelevant- it's just there to remind you what it was).
"""
## \namespace rdf.units.physical_quantity Classes for Physical Quantities
import abc
import sys
from rdf.units.prefix import metric, base, jedec, iec, base2


## The _Unit class memoizes its instances
class _Unit(str):
    """_Unit(value, multiplier=1, adder=0 [, symbol=None])

    On _Units and Prefixes:

    Instances of the Prefix class decorate _Unit classes- and as such
    create instances of:

    Sym = <Prefix> + <_Unit> when the _Unit subclass is created (at import).

    That instance is, of course, also a <str> and is memoized in

    _Unit.Glossary

    dictionary as:

    {Sym: Sym}

    At fist, that looks odd. The point is to do a hash-table search (not a list
    search) in the Glossary with "Sym" as a key-- here "Sym" is the ordinary
    string supplied by the RDF file's (unit) field.

    the resulting Value converts units to <_Unit>'s symbol with <unit>.factor
    as a scaling.

    Hence, of you're talking float(x) "km", you get:

    Glossary["km"](x) --> 1000*x, "m"
    """

    ## The base class is for PRIVATE Units, hence the _Unit
    __metaclass__ = abc.ABCMeta


    ## When ever a unit is instantiated, it goes into here.
    Glossary = {}

    ## getter for target unit
    @property
    def symbol(self):
        """symbol getter"""
        return self._symbol

    ## setter for target unit
    @symbol.setter
    def symbol(self, symbol):
        """symbol setter"""
        self._symbol = symbol

    ## The conversion function defined: \n
    ## \f$ y = mx + b \f$ \n
    # \param m is the multiplier for the conversion
    # \param b is the adder (applied 1st).
    # \par Side Effects:
    #     Instance is _memoized()'d.
    # \returns A string that can be looked up with a str in a hash-table
    #  and can then do unit conversion.
    def __new__(cls, abbreviation="", multiplier=1, adder=0, symbol=None):
        """abbrev="", multiplier=1, adder=0, symbol=None): """
        self = super(_Unit, cls).__new__(cls,
                                         abbreviation or symbol or cls._symbol)
        self._multiplier = multiplier
        self._adder = adder
        ## is this legit? what to do for units..
        if symbol is not None:  # Guard on keyword option
            self._symbol = str(symbol)
        ## All new instances get memoized
        self._memoize()
        return self

    ## TODO: refactor so that init does what init does and new does
    ## what new does.
    def __init__(self, *args, **kwargs):
        super(_Unit, self).__init__()

    ## Memoize into _Unit.Glossary
    def _memoize(self, warn=True):
        """save self into Glossary, w/ overwrite warning option"""
        # check key or not?
        if warn and self in self.Glossary:  # Guard
            print >> sys.stderr, (
                'Warning: Overwriting Unit.Glossary["%s"]' % self
                )
        self.Glossary.update({self: self})

    ## The conversion function called: \n
    ## \f$ y = mx + b \f$ \n
    ## \param x is the value in non-base/SI units, and must support float()
    ## \retval y  is the value in self.symbol
    def __call__(self, x):
        # todo: case x? who has case?
        return self._multiplier * float(x) + self._adder

    ## \param index Key to delete
    ## \par Side Effects:
    ## deletes key from rdf.units.GLOSSARY for ever.
    @classmethod
    def __delitem__(cls, index):
        del cls.Glossary[index]

    ## This is a TypeError: only Prefix.init can set Unit.Glossary
    @classmethod
    def __setitem__(cls, index, value):
        raise TypeError("Only Instantiation can set items for % class" %
                        cls.__name__)


## SI units
class _SI(_Unit):
    """SI units"""


## Accepted non SI units
class _Accepted(_Unit):
    """Accepted non-SI Units"""


## Unaccepted units (If you must)
class UnAccepted(_Unit):
    """Unaccepted non-SI Units"""


## Base SI units
class _Base(_SI):
    """SI Base units"""


## Derived DI unts
class _Derived(_SI):
    """SI Derived units"""


## Coherent Derived SI Units
class _Coherent(_Derived):
    """SI Coherent Derived Units"""


## Special Coherent Derived SI Units
class _Special(_Coherent):
    """Special Coherent Derived Units"""


## Length conversion to meters
@metric()
class Length(_Base):
    """Meter"""
    _symbol = 'm'


## Conversion to kilograms
@metric()
class Mass(_Base):
    """Kilogram"""
    _symbol = 'kg'


## Time conversion to seconds
@metric()
class Time(_Base):
    """Second"""
    _symbol = 's'


## Andre-Marie Ampere (22 January 1775 - 10 June 1836)
@metric()
class ElectricCurrent(_Base):
    """Ampere"""
    _symbol = 'amp'


## William Thomson, 1st Baron Kelvin OM, GCVO, PC, PRS, PRSE,
## (26 June 1824 - 17 December 1907)
@metric()
class Temperature(_Base):
    """Kelvin"""
    _symbol = 'K'


## Lorenzo Romano Amedeo Carlo Avogadro di Quaregna e di Cerreto,
## Count of Quaregna and Cerreto (9 August 1776, Turin, Piedmont - 9 July 1856)
@metric()
class AmountOfSubstance(_Base):
    """Mole"""
    _symbol = "mol"


## Brightness
@metric()
class LuminousIntensity(_Base):
    """Candela"""
    _symbol = "cd"


## Angle conversion to degrees
@metric()
class PlaneAngle(_Special):
    """radian"""
    _symbol = 'rad'


## Steradians
@metric()
class SolidAngle(_Special):
    """steradian"""
    _symbol = 'sr'


## Blaise Pascal (19 June 1623 - 19 August 1662)
@metric()
class Pressure(_Special):
    """Pascal"""
    _symbol = 'Pa'


## Heinrich Rudolf Hertz (22 February 1857 - 1 January 1894)
@metric()
class Frequency(_Special):
    """Hertz"""
    _symbol = 'Hz'


## Sir Isaac Newton PRS MP (25 December 1642 - 20 March 1727)
@metric()
class Force(_Special):
    """Newton"""
    _symbol = 'N'


## James Prescott Joule FRS (24 December 1818 - 11 October 1889)
@metric()
class Energy(_Special):
    """Joule"""
    _symbol = 'J'


## James Watts, FRS, FRSE (19 January 1736 - 25 August 1819)
@metric()
class Power(_Special):
    """Watt"""
    _symbol = 'W'


## Charles-Augustin de Coulomb (14 June 1736 - 23 August 1806)
@metric()
class Charge(_Special):
    """Coloumb"""
    _symbol = 'C'


## Alessandro Giuseppe Antonio Anastasio Volta
## (18 February 1745 - 5 March 1827)
@metric()
class EMF(_Special):
    """Volt"""
    _symbol = 'V'


## Michael Faraday, FRS (22 September 1791 - 25 August 1867)
@metric()
class Capacitance(_Special):
    """Faraday"""
    _symbol = 'F'


## Georg Simon Ohm (16 March 1789 - 6 July 1854)
@metric()
class ElectricalResitance(_Special):
    """Ohm"""
    _symbol = 'ohm'


## Ernst Werner Siemens, von Siemens since 1888,
## (13 December 1816 - 6 December 1892)
@metric()
class ElectricalConductance(_Special):
    """Siemens"""
    _symbol = 'S'


## Wilhelm Eduard Weber (24 October 1804 - 23 June 1891)
@metric()
class MagneticFlux(_Special):
    """Weber"""
    _symbol = 'Wb'


## Nikola Tesla (10 July 1856 - 7 January 1943)
@metric()
class MagneticFluxDensity(_Special):
    """Tesla"""
    _symbol = 'T'


## Joseph Henry (December 17, 1797 - May 13, 1878)
@metric()
class Inductance(_Special):
    """Henery"""
    _symbol = 'H'


## Anders Celsius (27 November 1701 - 25 April 1744)
@metric()
class CelsiusTemperatrue(_Special):
    """Celcius"""
    _symbol = 'degC'


## lumen
@metric()
class LuminouFlux(_Special):
    """Lumen"""
    _symbol = 'lm'


## lux
@metric()
class Illuminance(_Special):
    """Lux"""
    _symbol = 'lx'


## Antoine Henri Becquerel (15 December 1852 - 25 August 1908)
@metric()
class Activity(_Special):
    """Becquerel"""
    _symbol = 'Bq'


## Louis Harold Gray (10 November 1905 - 9 July 1965)
@metric()
class AbsorbedDose(_Special):
    """50 Shades of Gray"""
    _symbol = 'Gy'


## Professor Rolf Maximilian Sievert (6 May 1896 - 3 October 1966)
@metric()
class DoseEquivalent(_Special):
    """Sievert"""
    _symbol = 'Sv'


## <a href="http://en.wikipedia.org/wiki/Katal">Katal</a>
@metric()
class CatalyticActivity(_Special):
    """Katal"""
    _symbol = 'kat'


## \f$ m^2 \f$ \n
## <a href="http://en.wikipedia.org/wiki/Area">Area</a>
@metric(2)
class Area(_Coherent):
    """Square Meter"""
    _symbol = 'm**2'


## \f$ m^3 \f$\n
## <a href="http://en.wikipedia.org/wiki/Volume">Volume</a>
@metric(3)
class Volume(_Coherent):
    """Cubic Meter"""
    _symbol = 'm**3'


## \f$ f\frac{m}{s} \f$
@metric()
class Velocity(_Coherent):
    """meters-per-second"""
    _symbol = 'm/s'


## \f$ 1\ Pl = 1\ \frac{N\, s}{m^2} \f$
@metric()
class DynamicViscosity(_Coherent):
    """Poiseuille (is not Pl)"""
    _symbol = "Pa * s"


## \f$ 1\ N\cdot m = 1\  \frac{kg\,m^2}{s^2} \f$
@metric()
class MomentOfForce(_Derived):
    """Torque"""
    _symbol = 'N*m'


## decibel Power -is not power- it's just a number:
## (Alexander Graham Bell (March 3, 1847 - August 2, 1922))
@base()
class dBPower(_Accepted):
    """decibel Power"""
    _symbol = 'dbW'


## \f$ s^{-1} \f$
@metric(-1)
class WaveNumber(_Coherent):
    """per meter"""
    _symbol = 'm**-1'


## \f$ \frac{m}{s^2} \f$
@metric()
class Acceleration(_Coherent):
    """acceleration"""
    _symbol = 'm/s**2'


## \f$ \rho = \frac{m}{V}\f$ \n
## <a href="http://en.wikipedia.org/wiki/Density">Density<a/>
@metric()
class Density(_Coherent):
    """Mass Density"""
    _symbol = 'kg/m**3'


## \f$ \rho^{-1} \f$ \n
## <a href="http://en.wikipedia.org/wiki/Specific_volume">Specific Volume</a>
@metric()
class SpecificVolume(_Coherent):
    """Inverse Mass Density"""
    _symbol = 'm**3/kg'


##  \f$ \frac{A}{m^2} \f$ \n
## <a href="http://en.wikipedia.org/wiki/Current_density">Current Density</a>
@metric()
class CurrentDensity(_Coherent):
    """Current Density"""
    _symbol = 'A/m**2'


## \f$ \frac{A}{m} \f$
@metric()
class MagneticFieldStrength(_Coherent):
    """Magnetic Field Strenth"""
    _symbol = 'A/m'


##  \f$ \frac{cd}{m^2} \f$
@metric()
class Luminance(_Coherent):
    """Luminance"""
    _symbol = 'cd/m**2'


## \f$ \frac{mol}{m^3} \f$
@metric()
class Concentration(_Coherent):
    """Concentration"""
    _symbol = 'mol/m**3'


## Data Volume conversion to bits
@jedec
@iec
@base2()
class Bit(_Accepted):
    """Bit"""
    _symbol = 'bits'


## Data rate conversion to bps
@jedec
@iec
@base2()
class BitPerSecond(_Accepted):
    """Bite Rate"""
    _symbol = 'bits/s'


## Data Volume conversion to bits
@jedec
@iec
@base2()
class Byte(_Accepted):
    """Byte"""
    _symbol = 'byte'


## Data rate conversion to bytes per second
@jedec
@iec
@base2()
class BytesPerSecond(_Accepted):
    """Bytes per second"""
    _symbol = 'byte/s'


## TBD
class Ratio(_Accepted):
    """TBD"""
