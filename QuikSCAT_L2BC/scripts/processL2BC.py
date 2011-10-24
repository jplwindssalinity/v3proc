#! /usr/bin/env python

"""
This script will run all the programs required to generate L2BC data starting from
a NetCDF file that has winds and model data inputs.

In order for the program to run, the environment variable L2BC
must be set in the environment or the program run from L2BC/bin.
"""

import os, os.path, sys
from subprocess import call

try:
    L2BC = os.environ['L2BC']
    binDir = os.path.join(os.path.abspath(L2BC),'bin')
except:
    binDir = os.path.abspath('./')

# these are the executable programs and the order of execution

initL2BCnetcdf = os.path.join(binDir,'initL2BCnetcdf')

# This is the processing flow

program_flow = [initL2BCnetcdf,]

# make sure all the programs are in the executable path

for program in program_flow:
    if not os.path.exists(program):
        print "Program %s not found in binary directory"%program
        sys.exit(1)

# This is the template rdf file

rdf_template = """
INPUT/OUTPUT FILES

l2bc file                                       =  ! netcdf file

"""

def usage(program):
    """Usage message."""
    
    print "Usage: %s rdf_input_file"%program
    print "\nA sample RDF input file is given below\n"
    print rdf_template

def process_data(rdf_input_file):
    """Execute all the programs in order, given the input RDF file."""
    
    for program in program_flow:
        print "Executing: %s %s"%(program,rdf_input_file)
        status = call([program,rdf_input_file])
        if status:
            print "Program %s exited with status %s. Aborting..."%(program,status)
            sys.exit(1)

if __name__ == '__main__':

    if len(sys.argv[1:]) < 1:
        usage(sys.argv[0])
    else:
        process_data(sys.argv[1])



