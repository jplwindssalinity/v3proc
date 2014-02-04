#==============================================================#
# Copyright (C) 2013-2014, California Institute of Technology. #
# U.S. Government sponsorship acknowledged.                    #
#==============================================================#
"""
Classes and functions for GSE data files / packets
"""
__version__ = '$Id$'

import os
import sys
import struct
import numpy as np
import pdb

PACKET_SIZE = 526, 869
HEADER_BYTES = 39
NHEAD = 9
TT0_OFF = 40
TT1_OFF = 53
GPS2UTC_OFF = 444

def ReadPacketTimes(gse_packet):
    tt0_pos = TT0_OFF+NHEAD-1
    tt1_pos = TT1_OFF+NHEAD-1
    gpc2utc_pos = GPS2UTC_OFF+NHEAD-1
    tt0 = struct.unpack(">i", gse_packet[tt0_pos:tt0_pos+4])[0]
    tt1 = struct.unpack(">B", gse_packet[tt1_pos])[0]
    gps2utc = struct.unpack(">h", gse_packet[gpc2utc_pos:gpc2utc_pos+2])[0]
    return 1.0*tt0+tt1/255.0+gps2utc*1.0

class GSE:
    """Class for GSE files"""
    def __init__(self, filename):
        self.filename = filename
        self.version = None
        self._DetectVersion()
        assert self.version != None, "%s is not a GSE file" % self.filename
        self.n_packets = os.path.getsize(self.filename)/PACKET_SIZE[self.version]

    def _DetectVersion(self):
        ifp = open(self.filename, 'r')
        header = ifp.read(HEADER_BYTES)
        ifp.close()
        packet_length = struct.unpack('>H', header[36:38:])[0]+HEADER_BYTES
        for version in range(len(PACKET_SIZE)):
            if packet_length == PACKET_SIZE[version]:
                self.version = version

    def __getitem__(self, key):
        if self.version == None:
            self._DetectVersion
        
        # Support the negative index notation
        if key<0:
            key = self.n_packets + key
        
        # Bound check requested GSE packet
        if key<0 or key >= self.n_packets:
            raise IndexError
        
        ifp = open(self.filename, 'r')
        ifp.seek(PACKET_SIZE[self.version]*key)
        data = ifp.read(PACKET_SIZE[self.version])
        ifp.close()
        return data
    
    def ReadPacketTimes(self):
        ifp = open(self.filename, 'r')
        utc_tt = np.zeros(self.n_packets,dtype='f8')
        for ii in range(int(self.n_packets)):
            this_packet_offset = ii*PACKET_SIZE[self.version]

            ifp.seek(this_packet_offset+TT0_OFF+NHEAD-1)
            tt0 = struct.unpack( ">i", ifp.read(4) )

            ifp.seek(this_packet_offset+TT1_OFF+NHEAD-1)
            tt1 = struct.unpack( ">B", ifp.read(1) )

            ifp.seek(this_packet_offset+GPS2UTC_OFF+NHEAD-1)
            gps2utc = struct.unpack( ">h", ifp.read(2) )

            utc_tt[ii] = 1.0*tt0[0] + tt1[0]/255.0 + 1.0*gps2utc[0]
        ifp.close()
        return(utc_tt)

