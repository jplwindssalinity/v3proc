#==============================================================#
# Copyright (C) 2014, California Institute of Technology.      #
# U.S. Government sponsorship acknowledged.                    #
#==============================================================#
"""
Factor.py contains classes for manipulating xfactor/sfactor 
tables for the F90 SDS.  It also supports merging of multiple
X or S factor tables into one table for processing of revs with
table updates.
"""
__version__ = '$Id$'

import struct
import numpy as np

class Factor(object):
    """Does some common stuff for the X and S classes."""
    def __init__(self):
        """Sets some common attributes for both the X and S classes."""
        self.dtype = np.dtype('<f')
        self.NumBeams = 2
        self.NumAziBins = 36
        self.NumOrbBins = 32
        self.NumModes = 8
        self.NumSciSlices = 10

    def _set_endian(self, filename):
        """Checks fortran header bytes to determine byte order of file."""
        f = open(filename, 'r')
        header = f.read(4)
        f.close()
        if struct.unpack('>i', header)[0] == self._expected_size:
            self.dtype = np.dtype('>f')

class X(Factor):
    """Class for the Xfactor files."""
    def __init__(self, filename=None):
        super(X, self).__init__()
        self.NumSciSlices = 10
        self.NumGuardSlicesPerSide = 1
        self.NumTerms = 6

        # Set shape tuples for all arrays in file.
        self.slice_shift_shape = (self.NumModes, self.NumBeams, 2)

        # Note: table has has 10 slices then the egg value,
        # hence the array size NumSciSlices+1 for one of the dimensions.
        self.x_nominal_shape = (
            self.NumModes, self.NumOrbBins, self.NumAziBins,
            self.NumBeams, self.NumSciSlices+1)

        self.x_coeff_shape = self.x_nominal_shape + (self.NumTerms,)

        self.g_factor_shape = (
            self.NumModes, self.NumOrbBins, self.NumAziBins,
            self.NumBeams, self.NumSciSlices)

        # Expected size of fortran data record in bytes.
        self._expected_size = self.dtype.itemsize * (
            np.prod(self.slice_shift_shape) + np.prod(self.x_nominal_shape) +
            np.prod(self.x_coeff_shape) + np.prod(self.g_factor_shape) )

        if filename != None:
            self._set_endian(filename)
            self.Read(filename)

    def Read(self, filename):
        ifp = open(filename, 'r')
        hdr = ifp.read(4)

        self.slice_shift_threshold = np.fromfile(
            ifp, dtype=self.dtype, count=np.prod(self.slice_shift_shape),
            sep='').reshape(self.slice_shift_shape)

        self.x_nominal=np.fromfile(
            ifp, dtype=self.dtype, count=np.prod(self.x_nominal_shape),
            sep='').reshape(self.x_nominal_shape)

        self.x_coeff=np.fromfile(
            ifp, dtype=self.dtype, count=np.prod(self.x_coeff_shape),
            sep='').reshape(self.x_coeff_shape)

        self.g_factor=np.fromfile(
            ifp, dtype=self.dtype, count=np.prod(self.g_factor_shape),
            sep='').reshape(self.g_factor_shape)
        ifp.close()

    def Write(self, filename):
        ofp = open(filename, 'w')
        ofp.write(struct.pack('@i', self._expected_size))
        self.slice_shift_threshold.astype('float32').tofile(ofp)
        self.x_nominal.astype('float32').tofile(ofp)
        self.x_coeff.astype('float32').tofile(ofp)
        self.g_factor.astype('float32').tofile(ofp)
        ofp.write(struct.pack('@i', self._expected_size))
        ofp.close()

    @classmethod
    def Merge(cls, factors, orbsteps):
        """
        Merges multiple X tables into one table

        factors: list of factor objects to merge
        orbsteps: list of fractional orbit times to start using each factor.
        """

        merged_factor = factors[0].__class__()
        if not all([isinstance(item,merged_factor.__class__) 
                    for item in factors]):
            raise TypeError

        merged_factor.slice_shift_threshold = \
            factors[0].slice_shift_threshold.copy()

        merged_factor.x_nominal = np.zeros(
            merged_factor.x_nominal_shape).astype(merged_factor.dtype)

        merged_factor.x_coeff = np.zeros(
            merged_factor.x_coeff_shape).astype(merged_factor.dtype)

        merged_factor.g_factor = np.zeros(
            merged_factor.g_factor_shape).astype(merged_factor.dtype)

        for iorb in range(merged_factor.NumOrbBins):
            this_orbstep = iorb/(merged_factor.NumOrbBins*1.0)
            min_delta = 999.0
            for i_factor in range(len(orbsteps)):
                delta = this_orbstep-orbsteps[i_factor]
                if delta >= 0 and delta < min_delta:
                    factor = factors[i_factor]
                    min_delta = delta

            merged_factor.x_nominal[:,iorb,:,:,:] = factor.x_nominal[:,iorb,:,:,:]
            merged_factor.x_coeff[:,iorb,:,:,:,:] = factor.x_coeff[:,iorb,:,:,:,:]
            merged_factor.g_factor[:,iorb,:,:,:] = factor.g_factor[:,iorb,:,:,:]
        return merged_factor

class S(Factor):
    """Class for the SFactor files."""
    def __init__(self, filename=None):
        super(S,self).__init__()
        self.shape = (self.NumModes, self.NumOrbBins, self.NumAziBins, 
                      self.NumBeams)
        self._expected_size = np.prod(self.shape)*self.dtype.itemsize
        if filename != None:
            self._set_endian(filename)
            self.Read(filename)

    def Read(self, filename):
        ifp = open(filename,'r')
        hdr = ifp.read(4)
        self.table = np.fromfile(
            ifp, dtype=self.dtype, count=np.prod(self.shape), 
            sep='').reshape(self.shape)
        ifp.close()

    def Write(self, filename):
        ofp = open(filename,'w')
        ofp.write(struct.pack('@i', self._expected_size))
        self.table.astype('float32').tofile(ofp)
        ofp.write(struct.pack('@i', self._expected_size))
        ofp.close()

    @classmethod
    def Merge(cls, factors, orbsteps):
        """
        Merges multiple S tables into one table
        
        factors: list of factor objects to merge
        orbsteps: list of fractional orbit times to start using each factor.
        """
        merged_factor = factors[0].__class__()
        if not all([isinstance(item,merged_factor.__class__) 
                    for item in factors]):
            raise TypeError

        merged_factor.table = np.zeros(
            merged_factor.shape).astype(merged_factor.dtype)

        for iorb in range(merged_factor.NumOrbBins):
            this_orbstep = iorb/(merged_factor.NumOrbBins*1.0)
            min_delta = 999.0
            for i_factor in range(len(orbsteps)):
                delta = this_orbstep-orbsteps[i_factor]
                if delta >= 0 and delta < min_delta:
                    factor = factors[i_factor]
                    min_delta = delta

            merged_factor.table[:,iorb,:,:] = factor.table[:,iorb,:,:]
        return merged_factor
