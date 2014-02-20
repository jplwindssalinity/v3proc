## \namespace rdf.units.addendum Non metric and user units.
"""his modules instantiates units that do not fit the:

<prefix><metric>

format. Units are collected in to tuples of like dimension, however, that
is utterly unnecessary, as the mere act of instantiation memoizes them
in the GLOSSARY
"""
import operator
import math
from rdf.units.physical_quantity import (
    Length, Mass, Area, Time, Velocity, dBPower, Energy
    )

## Supported _Length conversions
LENGTHS = (Length('in', 0.0254),
           Length('ft', 0.3048),
           Length('mi', 1.609344e3))

## The Gram.
MASSES = (Mass('g', 0.001),)


## Supported _Area conversions
AREAS = (Area('ha', 10000),
         Area('in**2', 6.4516e-4),
         Area('ft**2', 9.290304e-2),
         Area('mi**2', 2.58995511e6)
         )

## Supported _Time conversions
TIMES = (Time('min', 60),
         Time('hour', 3600),
         Time('day', 86400)
         )


## Supported _Velocity conversions
VELOCITES = (Velocity('km/hr', operator.truediv(5, 18)),
             Velocity('ft/s', 0.3048),
             Velocity('mi/h', 0.44704),
             Velocity('kn', operator.truediv(1852, 3600)),
             Velocity('c', 299792458)
             )


POWERS = ()

## Suppoerted dB Power
DBPOWERS = (dBPower('dBm', adder=-30),)

ENERGIES = (Energy('BTU', 1055.056),)


## Supported Frequency conversions
FREQUENCIES = ()

## Supported Angle conversions
#ANGLES = (Angle('rad', operator.truediv(180, math.pi)),
#          Angle('arc', operator.truediv(1, 3600)),
#          Angle('"', operator.truediv(1, 3600)),
#      Angle("'", operator.truediv(1, 60)))


## \f$ ^{\circ}F, ^{\circ}C, \rm{eV} \f$
#TEMPERATURES = (Temperature('degC', 1.0, 273.15),
#                Temperature('degF',
#                            operator.truediv(5, 9),
#                            operator.truediv(5, 9) * 459.67),
#                Temperature('eV', 1.602176565e-19/1.3806488e-23))
