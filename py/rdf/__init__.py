#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#
#
# Author: Eric Belz
# Copyright 2013, by the California Institute of Technology.
# ALL RIGHTS RESERVED. United States Government Sponsorship acknowledged.
# Any commercial use must be negotiated with the Office of Technology Transfer
# at the California Institute of Technology.
#
# This software may be subject to U.S. export control laws. By accepting this
# software, the user agrees to comply with all applicable U.S. export laws and
# regulations. User has the responsibility to obtain export licenses, or other
# export authority as may be required before exporting such information to
# foreign countries or providing access to foreign persons.
#
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
"""Usage:

Interactive:

>>>import rdf
>>>rdf_mapping = rdf.rdfparse("<src>")

Shell Script:

%python rdf/parse.py <src> > <dst>
"""
__author__ = "Eric Belz"
__copyright__ = "Copyright 2013,  by the California Institute of Technology."
__credits__ = ["Eric Belz", "Scott Shaffer"]
__license__ = NotImplemented
__version__ = "1.0.7"
__maintainer__ = "Eric Belz"
__email__ = "eric.belz@jpl.nasa.gov"
__status__ = "Production"
__date__ = "Mon Nov 25 13: 32: 17 PST 2013"
__history__ = """
1.0.3  added dialogpickfile
1.0.4 deprecated '!' comment symbol in favor of '#'
......make a todict(dtype) keyword
1.0.5 added RDFField, RDFRecord into top namesapce
1.0.6 Pep08'd, units is very different, make RDF.data private.
1.0.7 fly-spelled comments.
"""

## \namespace rdf The rdf package
from rdf.uRDF import rdf_reader, RDF
from rdf.data.entries import RDFField, RDFRecord
from rdf.units import GLOSSARY

## Backwards compatible rdf readers.
rdfparse = rdf_reader
## less redundant parser
parse = rdf_reader


def test():
    """test() function - run from rdf/test"""
    import os
    rdf_ = rdfparse('rdf.txt')
    with open('new.rdf', 'w') as fdst:
        fdst.write(str(rdf_))
    if os.system("xdiff old.rdf new.rdf"):
        os.system("diff old.rdf new.rdf")
