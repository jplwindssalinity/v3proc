#==============================================================#
# Copyright (C) 2013-2014, California Institute of Technology. #
# U.S. Government sponsorship acknowledged.                    #
#==============================================================#
"""
Tracking.py contains the DopplerTracker and RangeTracker classes
for reading and converting the doppler and range tables
"""

rcs_id      = '$Id$'
__version__ = '$Revision$'

import os
import sys
import pdb
import struct
import StringIO
import util.time
import datetime
import numpy as np

range_opcodes   = (21990,21991)
doppler_opcodes = (22002,22003)

def ComputeCheckSum(data):
  """ 
  Computes checksum of a bunch of uint16 data values.  Add all values 
  together starting with a seed of 21930.  Every time this addition
  causes the sum to exceed 65536, subtract 65536 and add 1
  aka the "end-around-carry".
  """
  
  sum = 21930
  for ii in range(len(data)):
    sum += data[ii]
    if sum >= 65536:
      sum = sum-65536+1;
  return(sum)


class Tracker(object):
  """
  Base class for RangeTracker and DopplerTracker. 
  """
  def __init__(self,dtype):
    self.dtype           = dtype
    self.amp_scale_mag   = None
    self.amp_scale_bias  = None
    self.pha_scale_mag   = None
    self.pha_scale_bias  = None
    self.bias_scale_mag  = None
    self.bias_scale_bias = None
    self.amp_terms       = None
    self.pha_terms       = None
    self.bias_terms      = None
    self.amp_terms_eu    = None
    self.pha_terms_eu    = None
    self.bias_terms_eu   = None
    self.dib_data        = None
  
  def WriteGSASCII(self,filename):
    """
    Writes the GS style ASCII hex doppler / range tables
    """
    if self.dib_data==None:
      print sys.stderr,"Call CreateDibData first"
      return
    
    # create header line consisting of 127 chars and a newline char
    basename_file = os.path.basename(filename)
    header_string = "%s RAPIDSCAT JPL %s" % ( 
                    basename_file, 
                    util.time.ToCodeB(datetime.datetime.utcnow()))
    
    if len(header_string) > 127:
      print sys.stderr,"Use a shorter filename -- You made the header too big yo!"
      return
    
    header_string = "%-127.127s\n" % header_string
    
    f = open(filename,"w")
    f.write(header_string)
    
    for ii in range(len(self.dib_data)):
      f.write("%4.4X" % self.dib_data[ii])
      
      if (ii+1) % 16==0:
        f.write("\n")
      else:
        f.write(" ")
    
    # print 59 spaces and a newline to fill out last line of file
    f.write("                                                           \n")
    f.close()
  
  def WriteDIBBinary(self,filename):
    """
    Writes the DIB format doppler / range tables
    """
    if self.dib_data==None:
      print sys.stderr,"Call CreateDibData first"
      return
    
    # replace 1st short (op code) with zeros, as demanded by ?? for ISS.
    outdata = (0,) + self.dib_data[1::]
    
    # Write it out!
    f = open(filename,'w')
    f.write(struct.pack('>%dH' % len(outdata), *outdata ))
    f.close()
  
  def CreateDIBData(self,beam_index,table_id):
    """
    Creates the binary DIB format data.
    """
    if beam_index<0 or beam_index>1:
      print>>sys.stderr,"Tracker::CreateDIBData: Beam index out-of-range: %d" % beam_index
    
    # use StringIO as a buffer as if it were a file
    dib_data = StringIO.StringIO()
    
    # Write OpCode depending on if this is a range/doppler table and beam
    if self.dtype.char=='B': # Range
      dib_data.write(struct.pack('>H',range_opcodes[beam_index]))
      this_size = 804
    elif self.dtype.char=='H': # Doppler
      dib_data.write(struct.pack('>H',doppler_opcodes[beam_index]))
      this_size = 1572
    
    # Write Table ID
    dib_data.write(struct.pack('>H',table_id))
    
    # Write size of table file
    dib_data.write(struct.pack('>H',this_size))
    
    # write spare bytes
    dib_data.write(struct.pack('>2H',0,0))
    
    # write dither bytes
    dib_data.write(struct.pack('>2H',0,0))
    
    # write amplitude scale magnitude and bias
    dib_data.write(struct.pack('>2f',self.amp_scale_mag,self.amp_scale_bias))
    
    # write phase scale magnitude and bias
    dib_data.write(struct.pack('>2f',self.pha_scale_mag,self.pha_scale_bias))
    
    # write bias scale magnitude and bias
    dib_data.write(struct.pack('>2f',self.bias_scale_mag,self.bias_scale_bias))
    
    # write amplitude terms
    dib_data.write(struct.pack('>256%c'%self.dtype.char,*self.amp_terms))
    
    # write phase terms
    dib_data.write(struct.pack('>256%c'%self.dtype.char,*self.pha_terms))
    
    # write bias terms
    dib_data.write(struct.pack('>256%c'%self.dtype.char,*self.bias_terms))
    
    num_bytes = dib_data.tell()
    dib_data.seek(0)
    num_items = num_bytes/2
    values = struct.unpack('>%dH'%num_items,dib_data.read(num_bytes))
    dib_data.seek(num_bytes)
    
    # write Checksums
    dib_data.write(struct.pack('>H',ComputeCheckSum(values)))
    fpos = dib_data.tell()
    
    num_bytes = dib_data.tell()
    dib_data.seek(0)
    num_items = num_bytes/2
    self.dib_data = struct.unpack('>%dH'%num_items,dib_data.read(num_bytes))
  
  def ComputeTermsEu(self):
    self.amp_terms_eu  = self.amp_scale_bias  + self.amp_scale_mag *self.amp_terms
    self.pha_terms_eu  = self.pha_scale_bias  + self.pha_scale_mag *self.pha_terms
    self.bias_terms_eu = self.bias_scale_bias + self.bias_scale_mag*self.bias_terms
  
  def ReadSimBinary(self,filename):
    if not os.path.isfile(filename):
      print>>sys.stderr,'Tracker::ReadSimBinary: %s does not exist' % filename
      return
    ifp        = open(filename,'r')
    nsteps     = struct.unpack('@i',ifp.read(4))[0]
    scale      = np.array(struct.unpack('@%df' % 6,ifp.read(4*6))).reshape([3,2])
    num_items  = nsteps*3
    num_bytes  = num_items * self.dtype.itemsize
    struct_str = '@%d%c' % (num_items, self.dtype.char)
    terms      = np.array(struct.unpack(struct_str,ifp.read(num_bytes))).reshape([nsteps,3])
    ifp.close()
    
    self.amp_scale_bias  = scale[0,0]
    self.amp_scale_mag   = scale[0,1]
    self.pha_scale_bias  = scale[1,0]
    self.pha_scale_mag   = scale[1,1]
    self.bias_scale_bias = scale[2,0]
    self.bias_scale_mag  = scale[2,1]
    
    self.amp_terms  = terms[:,0]
    self.pha_terms  = terms[:,1]
    self.bias_terms = terms[:,2]
    
    self.ComputeTermsEu()

class RangeTracker(Tracker):
  """
  RangeTracker is a Tracker class with the byte option.
  """
  def __init__(self):
    super(RangeTracker,self).__init__(np.dtype('>u1'))

class DopplerTracker(Tracker):
  """
  RangeTracker is a Tracker class with the unsigned short option.
  """
  def __init__(self):
    super(DopplerTracker,self).__init__(np.dtype('>u2'))












