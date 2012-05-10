#!/bin/bash
######################################################################
#
# ORIGINAL AUTHOR: Thomas Werne
# COMPANY: Jet Propulsion Laboratory
# VERSION:
#
# File Name:     oscat2-fixup.sh
# Creation Date: 10 May 2012
#
# $Author$
# $Date$
# $Revision$
#
# Copyright 2009-2012, by the California Institute of Technology.
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

if [ $# -eq 1 ]; then
    BASEDIR="$1"
else
    echo -n "Base directory: "
    read BASEDIR;
fi

OS2_MATLAB_INC_DIR="/u/potr-r0/werne/QScatSim/mat"

OUTFILE=`basename "$L1B_HDF_FNAME"`
OUTFILE="${OUTFILE/h5/nc}"
OUTFILE="${OUTFILE/L1B/l2b}"
OUTFILE="${OUTFILE/S1/os2_}"

# Apply fixup
(
    echo "addpath('$OS2_MATLAB_INC_DIR');"
    echo "[~, files] = system('find $BASEDIR -name \"*.nc\"');"
    echo "cells = textscan(files, '%s');"
    echo "for k = 1:length(cells{1})"
    echo "    os2_netcdf_fixup(cells{1}{k});"
    echo "end"
) | matlab-7.11 -nodisplay

