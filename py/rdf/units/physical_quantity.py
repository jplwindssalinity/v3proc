"""Physical Quantities are defined here. They are all sub classes of _Unit-and
that's what counts. Other private classes are only for organization: they
impart no functionality.

So, to make you own unit that is called the "mine" and measure the amount
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
from rdf.units.prefix import *


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
        return self._symbol

    ## setter for target unit
    @symbol.setter
    def symbol(self, symbol):
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
        self = str.__new__(cls, abbreviation or symbol or cls._symbol)
        self._multiplier = multiplier
        self._adder = adder
        ## is this legit? what to do for units..
        if symbol is not None:  # Guard on keyword option
            self._symbol = str(symbol)

        ## All new instances get memoized
        self._memoize()

        return self

    ## Memoize into _Unit.Glossary
    def _memoize(self, warn=True):
        """save self into Glossary, w/ overwrite warning option"""
        # check key or not?
        if warn and self in self.Glossary:  # Guard
            from rdf.units.errors import RedefiningUnitWarning
#            raise RedefiningUnitWarning #you cannot raise this safely, why?
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
    pass


## Accepted non SI units
class _Accepted(_Unit):
    pass


## Unaccepted units (If you must)
class _UnAccepted(_Unit):
    pass


## Base SI units
class _Base(_SI):
    pass


## Derived DI unts
class _Derived(_SI):
    pass


## Coherent Derived SI Units
class _Coherent(_Derived):
    pass


## Special Coherent Derived SI Units
class _Special(_Coherent):
    pass


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
    _symbol = 'K'


## Lorenzo Romano Amedeo Carlo Avogadro di Quaregna e di Cerreto,
## Count of Quaregna and Cerreto (9 August 1776, Turin, Piedmont - 9 July 1856)
@metric()
class AmountOfSubstance(_Base):
    _symbol = "mol"


## Brightness
@metric()
class LuminousIntensity(_Base):
    _symbol = "cd"


## Angle conversion to degrees
@metric()
class PlaneAngle(_Special):
    _symbol = 'rad'


## Steradians
@metric()
class SolidAngle(_Special):
    _symbol = 'sr'


## Blaise Pascal (19 June 1623 - 19 August 1662)
@metric()
class Pressure(_Special):
    _symbol = 'Pa'


## Heinrich Rudolf Hertz (22 February 1857 - 1 January 1894)
@metric()
class Frequency(_Special):
    _symbol = 'Hz'


## Sir Isaac Newton PRS MP (25 December 1642 - 20 March 1727)
@metric()
class Force(_Special):
    _symbol = 'N'


## James Prescott Joule FRS (24 December 1818 - 11 October 1889)
@metric()
class Energy(_Special):
    _symbol = 'J'


## James Watts, FRS, FRSE (19 January 1736 - 25 August 1819)
@metric()
class Power(_Special):
    _symbol = 'W'


## Charles-Augustin de Coulomb (14 June 1736 - 23 August 1806)
@metric()
class Charge(_Special):
    _symbol = 'C'


## Alessandro Giuseppe Antonio Anastasio Volta
## (18 February 1745 - 5 March 1827)
@metric()
class EMF(_Special):
    _symbol = 'V'


## Michael Faraday, FRS (22 September 1791 - 25 August 1867)
@metric()
class Capacitance(_Special):
    _symbol = 'F'


## Georg Simon Ohm (16 March 1789 - 6 July 1854)
@metric()
class ElectricalResitance(_Special):
    _symbol = 'ohm'


## Ernst Werner Siemens, von Siemens since 1888,
## (13 December 1816 - 6 December 1892)
@metric()
class ElectricalConductance(_Special):
    _symbol = 'S'


## Wilhelm Eduard Weber (24 October 1804 - 23 June 1891)
@metric()
class MagneticFlux(_Special):
    _symbol = 'Wb'


## Nikola Tesla (10 July 1856 - 7 January 1943)
@metric()
class MagneticFluxDensity(_Special):
    _symbol = 'T'


## Joseph Henry (December 17, 1797 - May 13, 1878)
@metric()
class Inductance(_Special):
    _symbol = 'H'


## Anders Celsius (27 November 1701 - 25 April 1744)
@metric()
class CelsiusTemperatrue(_Special):
    _symbol = 'degC'


## lumen
@metric()
class LuminouFlux(_Special):
    _symbol = 'lm'


## lux
@metric()
class Illuminance(_Special):
    _symbol = 'lx'


## Antoine Henri Becquerel (15 December 1852 - 25 August 1908)
@metric()
class Activity(_Special):
    _symbol = 'Bq'


## Louis Harold Gray (10 November 1905 - 9 July 1965)
@metric()
class AbsorbedDose(_Special):
    _symbol = 'Gy'


## Professor Rolf Maximilian Sievert (6 May 1896 - 3 October 1966)
@metric()
class DoseEquivalent(_Special):
    _symbol = 'Sv'


## katal
@metric()
class CatalyticActivity(_Special):
    _symbol = 'kat'


## Area
@metric(2)
class Area(_Coherent):
    _symbol = 'm**2'


## Volume
@metric(3)
class Volume(_Coherent):
    _symbol = 'm**3'


## Speed
@metric()
class Velocity(_Coherent):
    _symbol = 'm/s'


## dynamic viscosity
@metric()
class DynamicViscosity(_Coherent):
    _symbol = "Pa * s"


## Torque
@metric()
class MomentOfForce(_Derived):
    _symbol = 'N*m'


## decibel Power -is not power- it's just a number:
## (Alexander Graham Bell (March 3, 1847 - August 2, 1922))
@base()
class dBPower(_Accepted):
    _symbol = 'dbW'


## Inverse meter
@metric(-1)
class WaveNumber(_Coherent):
    _symbol = 'm**-1'


## meters per seconds squared
@metric()
class Acceleration(_Coherent):
    _symbol = 'm/s**2'


## \f$ \rho \f$
@metric()
class Density(_Coherent):
    _symbol = 'kg/m**3'


## \f$ \rho^{-1} \f$
@metric()
class SpecificVolume(_Coherent):
    _symbol = 'm**3/kg'


##  Amps per square meter
@metric()
class CurrentDensity(_Coherent):
    _symbol = 'A/m**2'


## A/m
@metric()
class MagneticFieldStrength(_Coherent):
    _symbol = 'A/m'


##  cd/m**2
@metric()
class Luminance(_Coherent):
    _symbol = 'cd/m**2'


## concentration
@metric()
class Concentration(_Coherent):
    _symbol = 'mol/m**3'


## Data Volume conversion to bits
@jedec
@iec
@base2()
class Bit(_Accepted):
    _symbol = 'bits'


## Data rate conversion to bps
@jedec
@iec
@base2()
class BitPerSecond(_Accepted):
    _symbol = 'bits/s'


## Data Volume conversion to bits
@jedec
@iec
@base2()
class Byte(_Accepted):
    _symbol = 'byte'


## Data rate conversion to bytes per second
@jedec
@iec
@base2()
class BytesPerSecond(_Accepted):
    _symbol = 'byte/s'


## TBD
class Ratio(_Accepted):
    pass
