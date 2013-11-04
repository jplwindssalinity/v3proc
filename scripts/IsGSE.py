#!/usr/bin/env python
#==============================================================#
# Copyright (C) 2013, California Institute of Technology.      #
# U.S. Government sponsorship acknowledged.                    #
#==============================================================#

#----------------------------------------------------------------------
# NAME
#    IsGSE.py
#
# SYNOPSIS
#    IsGSE.py -f file_to_test
#
# DESCRIPTION
#    Check for some bytes that all GSE packets should have, returns 1 if
#    file_to_test has these bytes, 0 otherwise. 
#
# OPTIONS
#
# OPERANDS
#
# EXAMPLES
#
# ENVIRONMENT
#    Not environment dependent.
#
# EXIT STATUS
#    The following exit values are returned:
#       0  File header bytes match GSE file headers
#       1  File probably isn't a GSE file
#
# NOTES
#    None.
#
# AUTHOR
#    Alex Fore
#    alexander.fore@jpl.nasa.gov
#----------------------------------------------------------------------

rcs_id = "$Id$"

from optparse import OptionParser
import struct

PACKET_SIZE  = 526
HEADER_BYTES = 39
NHEAD        = 9
TT0_OFF      = 40
GPS2UTC_OFF  = 444

parser = OptionParser()
parser.add_option( "-f", "--file", dest="filename", help="Test if gse_filename is GSE file",          
                   metavar="gse_filename")

(options, args) = parser.parse_args()

gse_file = open(options.filename,"rb")
gse_headers = struct.unpack( "%dB" % HEADER_BYTES, gse_file.read(HEADER_BYTES) );
gse_file.close()

packet_length = gse_headers[36]*256 + gse_headers[37]

#  Testing for four bytes so far, maybe can improve this in the future
if packet_length==PACKET_SIZE-HEADER_BYTES and gse_headers[0]==35 and gse_headers[3]==6:
  exit(0)
else:
  exit(1)

