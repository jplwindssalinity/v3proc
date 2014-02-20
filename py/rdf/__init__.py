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
__version__ = "1.0.17"
__maintainer__ = "Eric Belz"
__email__ = "eric.belz@jpl.nasa.gov"
__status__ = "Production"
__date__ = "Jan 14, 2014"
__history__ = """
1.0.3  added dialogpickfile
1.0.4 deprecated '!' comment symbol in favor of '#'
......make a todict(dtype) keyword
1.0.5 added RDFField, RDFRecord into top namesapce
1.0.6 Pep08'd, units is very different, make RDF.data private.
1.0.7 fly-spelled comments.
1.0.8 Fixed ValueError when printing an empty RDF file
1.0.9 Ignoring (&) units for UAVSAR standard
1.0.10 added RDF.fromstr method, which required breaking unwrap_file into
       unwrap_file, unwrap_readlines functions (which it should have been
       in the 1st place).
1.0.11 moved extension base class from eRDF ot iRDF. Added a todo (casting)
1.0.12 Added UAVSAR workaround for "&" unit.
1.0.13 Added RDF.copy() method.
1.0.14 Ensure RDF.todict() makes an OrderedDict
1.0.15 staic checked.
       952 statements analysed.

+---------+-------+-----------+-----------+------------+---------+
|type     |number |old number |difference |%documented |%badname |
+=========+=======+===========+===========+============+=========+
|module   |26     |1          |+25.00     |100.00      |11.54    |
+---------+-------+-----------+-----------+------------+---------+
|class    |106    |0          |+106.00    |100.00      |2.83     |
+---------+-------+-----------+-----------+------------+---------+
|method   |152    |0          |+152.00    |100.00      |0.00     |
+---------+-------+-----------+-----------+------------+---------+
|function |34     |1          |+33.00     |100.00      |2.94     |
+---------+-------+-----------+-----------+------------+---------+
1.0.16 Addesd 'list' parsing to RDField
1.0.17 __eq__ added to RDF; use copy.copy and copy.deepcopy correctly.
"""

## \namespace rdf The rdf package
from rdf.uRDF import rdf_reader, RDF
from rdf.data.entries import RDFField, RDFRecord
from rdf.units import GLOSSARY

## Backwards compatible rdf readers.
rdfparse = rdf_reader
## less redundant parser
parse = rdf_reader

__TODO__ = """1) Make Element into a type cast, e.g: [str],
              2) handle list values proactively
              3) Make __iops__ on rdf[key] unit preserving.
              4) investigate __contains__ behavior"""


def test():
    """test() function - run from rdf/test"""
    import os
    rdf_ = rdfparse('rdf.txt')
    with open('new.rdf', 'w') as fdst:
        fdst.write(str(rdf_))
    if os.system("xdiff old.rdf new.rdf"):
        os.system("diff old.rdf new.rdf")
