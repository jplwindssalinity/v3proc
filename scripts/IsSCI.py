#!/usr/bin/env python
#==============================================================#
# Copyright (C) 2013, California Institute of Technology.      #
# U.S. Government sponsorship acknowledged.                    #
#==============================================================#

#----------------------------------------------------------------------
# NAME
#    IsSCI.py
#
# SYNOPSIS
#    IsSCI.py -f file_to_test
#
# DESCRIPTION
#    Check for some bytes that all SCI packets should have, returns 1 if
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
#       0  File header bytes match SCI file headers
#       1  File probably isn't a SCI file
#
# NOTES
#    None.
#
# AUTHOR
#    Alex Fore
#    alexander.fore@jpl.nasa.gov
#----------------------------------------------------------------------

rcs_id = "$Id$"

PRIMARY_HEADER_OFFSET = 6
PRIMARY_HEADER_BYTES  = 6

from optparse import OptionParser
import struct

parser = OptionParser()
parser.add_option( "-f", "--file", dest="filename", help="Test if sci_filename is SCI file",          
                   metavar="sci_filename")

num_bytes_to_read = PRIMARY_HEADER_BYTES + PRIMARY_HEADER_OFFSET

(options, args) = parser.parse_args()

sci_file = open(options.filename,"rb")
sci_headers = struct.unpack( "%dB" % num_bytes_to_read, sci_file.read(num_bytes_to_read) )
sci_file.close()

primary_header = sci_headers[PRIMARY_HEADER_OFFSET:PRIMARY_HEADER_OFFSET+PRIMARY_HEADER_BYTES]

# import pdb
# pdb.set_trace()

packet_length = primary_header[4]*256+primary_header[5]

if primary_header[0]==8 and primary_header[1]==85 and packet_length==789:
  exit(0)
else:
  exit(1)

