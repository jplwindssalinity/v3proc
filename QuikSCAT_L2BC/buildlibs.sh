#!/bin/bash
######################################################################
#
# ORIGINAL AUTHOR: Thomas Werne
# COMPANY: Jet Propulsion Laboratory
# VERSION: 
#
# File Name:     buildlibs.sh
# Creation Date: 20 Oct 2011
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

function make-blitz () {

BASEDIR="$1"

cd packages
tar zxvf blitz-0.9.tar.gz
cd blitz-0.9

./configure --prefix="$BASEDIR"
make install

cd ../
cd ../

}

function make-scons () {

BASEDIR="$1"

cd packages
tar zxvf scons-2.0.0.final.0.tar.gz
cd scons-2.0.0.final.0

python setup.py install --prefix="$BASEDIR"

cd ../
cd ../

}

function make-netcdf () {

BASEDIR="$1"

cd packages
tar zxvf netcdf-4-1.1.1.tar.gz
cd netcdf-4.1.1

./configure --prefix="$BASEDIR" --disable-netcdf-4 --disable-dap --disable-fortran --disable-f90
make check install

cd ../
cd ../

}

function make-zlib () {

BASEDIR="$1"

cd packages
tar zxvf zlib-1.2.5.tar.gz
cd zlib-1.2.5

./configure --prefix="$BASEDIR"
make install

cd ../
cd ../

}

function make-jpeg () {

BASEDIR="$1"

cd packages
tar zxvf jpegsrc.v6b.tar.gz
cd jpeg-6b

./configure --prefix="$BASEDIR"
make install
make install-lib

cd ../
cd ../

}

function make-hdf () {

BASEDIR="$1"

make-zlib "$BASEDIR"
make-jpeg "$BASEDIR"

cd packages
tar zxvf hdf-4.2.5.tar.gz
cd hdf-4.2.5

./configure --prefix="$BASEDIR" --with-zlib="$BASEDIR" --with-jpeg="$BASEDIR" --enable-fortran=no --enable-netcdf=no
make install

cd ../
cd ../

}

function make-gsl () {

BASEDIR="$1"

cd packages
tar zxvf gsl-1.14.tar.gz
cd gsl-1.14

./configure --prefix="$BASEDIR"
make
make install

cd ../
cd ../

}

# Main
L2BCDIR="$PWD"

make-blitz  "$L2BCDIR"
make-scons  "$L2BCDIR"
make-netcdf "$L2BCDIR"
make-hdf    "$L2BCDIR"
make-gsl    "$L2BCDIR"

