#==============================================================#
# Copyright (C) 2013-2014, California Institute of Technology. #
# U.S. Government sponsorship acknowledged.                    #
#==============================================================#
"""
Tracker.py contains the Doppler and Range classes
for reading and converting the doppler and range tables.
"""
__version__ = '$Id$'

import os
import sys
import pdb
import struct
import StringIO
import util.time
import datetime
import numpy as np

RANGE_OPCODES = (21990,21991)
DOPPLER_OPCODES = (22002,22003)

class Tracker(object):
    """Base class for Range and Doppler tracking classes."""
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
    
    def __iter__(self):
        for ii in range(len(self.amp_terms_eu)):
            yield self.amp_terms_eu[ii], self.pha_terms_eu[ii], self.bias_terms_eu[ii]
  
    def __getitem__(self, key):
        return self.amp_terms_eu[key], self.pha_terms_eu[key], self.bias_terms_eu[key]
  
    def WriteGSASCII(self,filename):
        """Writes the GS style ASCII hex doppler / range tables."""
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
            return
        
        # use StringIO as a buffer as if it were a file
        dib_data = StringIO.StringIO()
        
        # Write OpCode depending on if this is a range/doppler table and beam
        if self.dtype.char=='B': # Range
            dib_data.write(struct.pack('>H',RANGE_OPCODES[beam_index]))
            this_size = 804
        elif self.dtype.char=='H': # Doppler
            dib_data.write(struct.pack('>H',DOPPLER_OPCODES[beam_index]))
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
        scale      = np.array(struct.unpack('@%df'%6, ifp.read(4*6))).reshape([3,2])
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
    
    def WriteSimBinary(self, filename):
        """Writes out the tracking table to the Sim / C++ binary format."""
        ofp = open(filename,"w")
        ofp.write(struct.pack('@i',256))
        ofp.write(struct.pack('@f',self.amp_scale_bias))
        ofp.write(struct.pack('@f',self.amp_scale_mag))
        ofp.write(struct.pack('@f',self.pha_scale_bias))
        ofp.write(struct.pack('@f',self.pha_scale_mag))
        ofp.write(struct.pack('@f',self.bias_scale_bias))
        ofp.write(struct.pack('@f',self.bias_scale_mag))
        terms = np.zeros([256,3])
        terms[:,0] = self.amp_terms
        terms[:,1] = self.pha_terms
        terms[:,2] = self.bias_terms
        terms.astype(self.dtype.char).tofile(ofp)
        ofp.close()

class Range(Tracker):
    """Range is a Tracker class with the byte option."""
    def __init__(self):
        super(Range,self).__init__(np.dtype('>u1'))

class Doppler(Tracker):
    """Doppler is a Tracker class with the unsigned short option."""
    def __init__(self):
        super(Doppler,self).__init__(np.dtype('>u2'))

# This coult be a classmethod, not sure that it matters??
def Merge(trackers, orbsteps):
    """
    Merges multiple trackers into one tracker object using some optimal mixing
    of the two trackers.
    
    tracker_list: list of tracker objects to merge.
    orbstep_list: fractional orbit time in [0,1) to start using each tracker.
    """
    assert len(trackers) == len(orbsteps), \
        "List of trackers and orbit steps must be same length."
    
    assert isinstance(trackers[0],Tracker)
    
    # Figure out if all trackers in tracker_list are the same type of tracker
    # and which kind of tracker they are
    merged_tracker = trackers[0].__class__()
    if not all([isinstance(item,merged_tracker.__class__) 
                for item in trackers]):
        raise TypeError
    
    # Determine dynamic range that we need to cover
    amp_lims = (min([min(item.amp_terms_eu) for item in trackers]),
                max([max(item.amp_terms_eu) for item in trackers]))
    
    pha_lims = (min([min(item.pha_terms_eu) for item in trackers]),
                max([max(item.pha_terms_eu) for item in trackers]))
    
    bias_lims = (min([min(item.bias_terms_eu) for item in trackers]),
                 max([max(item.bias_terms_eu) for item in trackers]))
    
    if merged_tracker.dtype == np.dtype('u1'):
        max_dn = 255
    else:
        max_dn = 65535
    
    # Compute bias factors as min for each
    merged_tracker.amp_scale_bias = amp_lims[0]
    merged_tracker.pha_scale_bias = pha_lims[0]
    merged_tracker.bias_scale_bias = bias_lims[0]
    
    # Scale factors are give by (max-min)/(max integer range)
    merged_tracker.amp_scale_mag = (amp_lims[1]-amp_lims[0])/(max_dn*1.0)
    merged_tracker.pha_scale_mag = (pha_lims[1]-pha_lims[0])/(max_dn*1.0)
    merged_tracker.bias_scale_mag = (bias_lims[1]-bias_lims[0])/(max_dn*1.0)
    
    merged_tracker.amp_terms = np.zeros(256)
    merged_tracker.pha_terms = np.zeros(256)
    merged_tracker.bias_terms = np.zeros(256)
    
    # Compute terms
    for iterm in range(256):
        this_orbstep = iterm/256.0
        # use most recent commanded table
        min_delta = 999.0
        for step, tracker in zip(orbsteps,trackers):
            delta = this_orbstep - step
            if delta >= 0 and delta < min_delta:
                use_tracker = tracker
                min_delta = delta
        
        merged_tracker.amp_terms[iterm] = (
            (use_tracker.amp_terms_eu[iterm]-merged_tracker.amp_scale_bias)
            / merged_tracker.amp_scale_mag )
        
        merged_tracker.pha_terms[iterm] = (
            (use_tracker.pha_terms_eu[iterm]-merged_tracker.pha_scale_bias)
            / merged_tracker.pha_scale_mag )
        
        merged_tracker.bias_terms[iterm] = (
            (use_tracker.bias_terms_eu[iterm]-merged_tracker.bias_scale_bias)
            / merged_tracker.bias_scale_mag )
    merged_tracker.ComputeTermsEu()
    return merged_tracker

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
