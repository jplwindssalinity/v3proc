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

def to_index(values, steps):
    """Converts values to array indicies"""
    values = np.asarray(values)
    return np.round(values/values.max()*(steps-1)).astype('i')

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
            np.prod(self.x_coeff_shape) + np.prod(self.g_factor_shape))

        # If filename commanded read from file, otherwise allocate arrays to
        # zeros.
        if filename != None:
            self._set_endian(filename)
            self.Read(filename)
        else:
            self.slice_shift_threshold = np.zeros(
                self.slice_shift_shape).astype(self.dtype)
            self.x_nominal = np.zeros(self.x_nominal_shape).astype(self.dtype)
            self.x_coeff = np.zeros(self.x_coeff_shape).astype(self.dtype)
            self.g_factor = np.zeros(self.g_factor_shape).astype(self.dtype)

    def Read(self, filename):
        ifp = open(filename, 'r')
        hdr = ifp.read(4)

        self.slice_shift_threshold = np.fromfile(
            ifp, dtype=self.dtype, count=np.prod(self.slice_shift_shape),
            sep='').reshape(self.slice_shift_shape)

        self.x_nominal = np.fromfile(
            ifp, dtype=self.dtype, count=np.prod(self.x_nominal_shape),
            sep='').reshape(self.x_nominal_shape)

        self.x_coeff = np.fromfile(
            ifp, dtype=self.dtype, count=np.prod(self.x_coeff_shape),
            sep='').reshape(self.x_coeff_shape)

        self.g_factor = np.fromfile(
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

    def PopulateFromXPert(self, filename, mode, beam):
        """Populates the Factor.X object using the BYU Xpert ASCII files.
        Validated to give the same exact table as the "writeToJPL.m" matlab
        script provided by N. Madsen. (same md5sum)
        """
        assert mode < self.NumModes
        assert beam < self.NumBeams
        data = np.loadtxt(filename)
        iorb = to_index(data[:, 0], self.NumOrbBins)
        iazi = to_index(data[:, 1], self.NumAziBins)
        imode = mode * np.ones(iorb.shape).astype('i')
        ibeam = beam * np.ones(iorb.shape).astype('i')

        # Not sure how to remove the magic number antipattern from this.
        x_nom_cols = range(12, 112, 10) + [122]
        g_fact_cols = range(13, 113, 10)

        self.x_nominal[imode, iorb, iazi, ibeam] = data[:, x_nom_cols]
        self.g_factor[imode, iorb, iazi, ibeam] = data[:, g_fact_cols]

        for term in range(self.NumTerms):
            iterm = term*np.ones(iorb.shape).astype('i')
            term_cols = range(14+term, 114+term, 10) + [123+term]
            self.x_coeff[imode, iorb, iazi, ibeam, :, iterm] = \
                data[:, term_cols]

        # the BYU converter sets these explicitly to zero.
        self.x_coeff[:, :, :, :, -1, 0] = 0
        self.x_coeff[:, :, :, :, :, 4] = 0
        self.x_coeff[:, :, :, :, :, 5] = 0

    @classmethod
    def Merge(cls, factors, orbsteps):
        """
        Merges multiple X tables into one table

        factors: list of factor objects to merge
        orbsteps: list of fractional orbit times to start using each factor.

        Note one of the items in orbsteps must equal zero or it don't work.
        """
        merged_factor = factors[0].__class__()
        if not all([isinstance(item, merged_factor.__class__)
                    for item in factors]):
            raise TypeError

        merged_factor.slice_shift_threshold = \
            factors[0].slice_shift_threshold.copy()

        for iorb in range(merged_factor.NumOrbBins):
            this_orbstep = iorb/(merged_factor.NumOrbBins*1.0)
            min_delta = 999.0
            for step, factor in zip(orbsteps, factors):
                delta = this_orbstep - step
                if delta >= 0 and delta < min_delta:
                    use_factor = factor
                    min_delta = delta

            merged_factor.x_nominal[:, iorb] = use_factor.x_nominal[:, iorb]
            merged_factor.x_coeff[:, iorb] = use_factor.x_coeff[:, iorb]
            merged_factor.g_factor[:, iorb] = use_factor.g_factor[:, iorb]
        return merged_factor

class S(Factor):
    """Class for the SFactor files."""
    def __init__(self, filename=None):
        super(S, self).__init__()
        self.shape = (self.NumModes, self.NumOrbBins, self.NumAziBins,
                      self.NumBeams)
        self._expected_size = np.prod(self.shape)*self.dtype.itemsize

        # If filename commanded read from file, otherwise allocate arrays to
        # zeros.
        if filename != None:
            self._set_endian(filename)
            self.Read(filename)
        else:
            self.table = np.zeros(self.shape).astype(self.dtype)

    def Read(self, filename):
        ifp = open(filename, 'r')
        hdr = ifp.read(4)
        self.table = np.fromfile(
            ifp, dtype=self.dtype, count=np.prod(self.shape),
            sep='').reshape(self.shape)
        ifp.close()

    def Write(self, filename):
        ofp = open(filename, 'w')
        ofp.write(struct.pack('@i', self._expected_size))
        self.table.astype('float32').tofile(ofp)
        ofp.write(struct.pack('@i', self._expected_size))
        ofp.close()

    def PopulateFromXPert(self, filename, mode, beam):
        assert mode < self.NumModes
        assert beam < self.NumBeams
        data = np.loadtxt(filename)
        iorb = to_index(data[:, 0], self.NumOrbBins)
        iazi = to_index(data[:, 1], self.NumAziBins)
        imode = mode * np.ones(iorb.shape).astype('i')
        ibeam = beam * np.ones(iorb.shape).astype('i')
        self.table[imode, iorb, iazi, ibeam] = data[:, -1]

    @classmethod
    def Merge(cls, factors, orbsteps):
        """
        Merges multiple S tables into one table

        factors: list of factor objects to merge
        orbsteps: list of fractional orbit times to start using each factor.

        Note one of the items in orbsteps must equal zero or it don't work.
        """
        merged_factor = factors[0].__class__()
        if not all([isinstance(item, merged_factor.__class__)
                    for item in factors]):
            raise TypeError

        for iorb in range(merged_factor.NumOrbBins):
            this_orbstep = iorb/(merged_factor.NumOrbBins*1.0)
            min_delta = 999.0
            for step, factor in zip(orbsteps, factors):
                delta = this_orbstep - step
                if delta >= 0 and delta < min_delta:
                    use_factor = factor
                    min_delta = delta

            merged_factor.table[:, iorb] = use_factor.table[:, iorb]
        return merged_factor
