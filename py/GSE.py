#==============================================================#
# Copyright (C) 2013-2014, California Institute of Technology. #
# U.S. Government sponsorship acknowledged.                    #
#==============================================================#
rcs_id      = '$Id$'
__version__ = '$Revision$'

class GSE:
  """ Class for GSE files"""
  
  def __init__(self,filename=None):
    self.filename = filename
    self.version  = None
    
    if filename != None:
      self.filename = filename

def read_packet_tt(gsefile):
  import os
  import struct
  import sys
  import numpy
  
  PACKET_SIZE  = 526
  HEADER_BYTES = 39
  NHEAD        = 9
  TT0_OFF      = 40
  TT1_OFF      = 53
  GPS2UTC_OFF  = 444
  
  if not gsefile or not os.path.isfile(gsefile):
    sys.stderr.write('Problem with %s\n' % gsefile)
    return None
  
  # First check that size of the file equals an integer multiple of PACKET_SIZE
  filesize = os.path.getsize( gsefile )
  n_packets = filesize/(PACKET_SIZE*1.0)
  if round(n_packets) != n_packets:
    sys.stderr.write('%s: does not have correct file size\n' % gsefile)
    return None
  
  try:
    ifp = open(gsefile,"rb")
  except IOError:
    sys.stderr.write('Error opening file %s' % gsefile)
    return None
  
  utc_tt = numpy.zeros(n_packets,dtype='f8')
  
  for ii in range(int(n_packets)):
    
    this_packet_offset = ii*PACKET_SIZE
    
    ifp.seek(this_packet_offset+TT0_OFF+NHEAD-1)
    tt0 = struct.unpack( ">i", ifp.read(4) )
    
    ifp.seek(this_packet_offset+TT1_OFF+NHEAD-1)
    tt1 = struct.unpack( ">B", ifp.read(1) )
    
    ifp.seek(this_packet_offset+GPS2UTC_OFF+NHEAD-1)
    gps2utc = struct.unpack( ">h", ifp.read(2) )
    
    utc_tt[ii] = 1.0*tt0[0] + tt1[0]/255.0 + 1.0*gps2utc[0]
    
  ifp.close()
  return(utc_tt)

# def detect_version( gsefile ):
#   
#   import struct
#   
#   return 0