"""The unit module.

The rdf.data.entries.RDFField.__new__ only needs access to the
SI function-- which identIfies units and converts them to nominal
inputs.

See SI.__doc__ on how Units are used.

"""
## \namespace rdf.units Units according to
## http://physics.nist.gov/cuu/pdf/sp811.pdf
from rdf.units.physical_quantity import _Unit
from rdf.units import addendum
from rdf.units import errors


## The global unit glossary dictionary:[symbol]->converter function
GLOSSARY = _Unit.Glossary


## Convert (value, units) to SI pair - this is the interface to RDField
## Search various places for units...(TBD).
## \param value A float in units
## \param units a string describing the units
## \retval (converter(value),converter.symbol) The new value in the right
## units
def SI(value, units):
    """
    Using Units:
    Unit instance are instance of <str>-- hence you can compare them or
    use them as keys in a dictionary. Hence:

    >>>km = physical_quantity.Length(1000, 'km')

    is a string == 'km', and it is a function that multiplies by 1000.

    Thus: SI just looks in a dictionary of UNITS, c.f:

    {km : km}['km']

    which returns km, such that:

    >>>print km(1)
    1000.

    Sweet.

    See physical_quanity on how to make your own units and how to put
    them in the GLOASSRY.
    """
    try:
        converter = GLOSSARY[units]
    except KeyError:
        raise errors.UnrecognizedUnitWarning(units)
    return converter(value), converter.symbol
