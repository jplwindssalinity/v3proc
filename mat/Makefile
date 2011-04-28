######################################################################
#
# ORIGINAL AUTHOR: Thomas Werne
# COMPANY: Jet Propulsion Laboratory
# VERSION: 
#
# File Name:     Makefile
# Creation Date: 27 Apr 2011
#
# $Author$
# $Date$
# $Revision$
#
# Copyright 2009-2011, by the California Institute of Technology.
# ALL RIGHTS RESERVED.  United States Government Sponsorship
# acknowledged. Any commercial use must be negotiated with the Office 
# of Technology Transfer at the California Institute of Technology.
#
# This software may be subject to U.S. export control laws and
# regulations.  By accepting this document, the user agrees to comply
# with all U.S. export laws and regulations.  User has the
# responsibility to obtain export licenses, or other export authority
# as may be required before exporting such information to foreign
# countries or providing access to foreign persons.
######################################################################

# For now this build will probably only work on the p-machines.
# TODO: Auto-detect the Matlab build environment
MATLAB := matlab-7.11
MAT_PATH := /usr/local/pkgs/${MATLAB}

CXX := mex

CXXFLAGS := -cxx -I../../include \

PROGRAMS := l2b_extraction \

# Note: have to compile the libSim.a and libEAData.a libraries
# with -fPIC.  Also, we have to use Matlab's HDF shared object
# files (the ones on the p-machines are static libraries)
LDFLAGS := -L../../lib -lSim -lEAData \
        ${MAT_PATH}/bin/glnxa64/libmfhdf.so.0 \
        ${MAT_PATH}/bin/glnxa64/libdf.so.0 \

.PHONY: default
default: $(PROGRAMS)

.PHONY: clean
clean:
	rm -rf *.mexa64
