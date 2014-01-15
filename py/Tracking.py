#==============================================================#
# Copyright (C) 2013-2014, California Institute of Technology. #
# U.S. Government sponsorship acknowledged.                    #
#==============================================================#
rcs_id      = '$Id$'
__version__ = '$Revision$'


import os
import sys
import pdb
import struct
import numpy as np

def ReadBinaryRGC(filename):
  try:
    ifp        = open(filename,'r')
    nsteps     = struct.unpack('@i',ifp.read(4))[0]
    terms_size = nsteps*3
    scale      = np.array(struct.unpack('@%df' % 6,ifp.read(4*6))).reshape([3,2])
    terms      = np.array(struct.unpack('@%dB' % terms_size,ifp.read(terms_size))).reshape([nsteps,3])
    ifp.close()
  except IOError:
    return None
  fterms = np.zeros(terms.shape)
  fterms[:,0] = scale[0,0] + terms[:,0] * scale[0,1]
  fterms[:,1] = scale[1,0] + terms[:,1] * scale[1,1]
  fterms[:,2] = scale[2,0] + terms[:,2] * scale[2,1]
  return(fterms)

def ReadBinaryDTC(filename):
  try:
    ifp        = open(filename,'r')
    nsteps     = struct.unpack('@i',ifp.read(4))[0]
    terms_size = nsteps*3
    scale      = np.array(struct.unpack('@%df' % 6,ifp.read(4*6))).reshape([3,2])
    terms      = np.array(struct.unpack('@%dH' % terms_size, ifp.read(2*terms_size))).reshape([nsteps,3])
    ifp.close()
  except IOError:
    return None
  fterms = np.zeros(terms.shape)
  fterms[:,0] = scale[0,0] + terms[:,0] * scale[0,1]
  fterms[:,1] = scale[1,0] + terms[:,1] * scale[1,1]
  fterms[:,2] = scale[2,0] + terms[:,2] * scale[2,1]
  return(fterms)


