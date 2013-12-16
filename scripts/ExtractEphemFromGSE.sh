#!/usr/bin/env bash
#==============================================================#
# Copyright (C) 2013, California Institute of Technology.      #
# U.S. Government sponsorship acknowledged.                    #
#==============================================================#
#----------------------------------------------------------------------
# NAME
#    ExtractEphemFromGSE.sh
#
# SYNOPSIS
#    ExtractEphemFromGSE.sh -i GSE_file -o out_directory
#
# DESCRIPTION
#    Extracts the Ephemeris and quaternions from the given GSE file,
#    writes them out in the sim format in out_directory.  Will skip
#    writing if the files already exist, unless the "-c" option is
#    commanded.
#
# OPTIONS
#
# OPERANDS
#
# EXAMPLES
#
# ENVIRONMENT
#    Not environment dependent.
#
# EXIT STATUS
#    The following exit values are returned:
#       0  Successful execution
#       1  Error
#
# NOTES
#    None.
#
# AUTHOR
#    Alex Fore
#    alexander.fore@jpl.nasa.gov
#----------------------------------------------------------------------
# rcs_id = "$Id$"
QSCAT_SIM_PATH=/home/fore/qscatsim/QScatSim/

USAGE="$0 -i gse_file -o out_gse_directory [-c]"

iflag=0
oflag=0
cflag=0
while getopts 'i:o:c' OPTION
do
  case $OPTION in 
    i) iflag=1
       INFILE="$OPTARG"
       ;;
    o) oflag=1
       OUT_DIR="$OPTARG"
       ;;
    c) cflag=1
       ;;
    ?) echo "$USAGE"
       exit 1
       ;;
  esac
done

if [ "$iflag" -ne 1 -o "$oflag" -ne 1 ]; then
  echo "$USAGE"
  exit 1
fi

if [ ! -f $INFILE ]; then
  echo "$INFILE does not appear to be a valid filename"
  exit 1
fi

if [ ! -d $OUT_DIR ]; then
  echo "$OUT_DIR does not appear to be a valid directory"
  exit 1
fi

# Figure out the output ephem and quaternion filenames
tag=`basename $INFILE`
tag=${tag#RS_GSE_}

out_ephem_file=${OUT_DIR}/RS_EPHEM_${tag}
out_quats_file=${OUT_DIR}/RS_QUATS_${tag}

if [ "$cflag" -eq 1 ] || [ ! -f "$out_ephem_file" ] || [ ! -f "$out_quats_file" ]; then
  ${QSCAT_SIM_PATH}/programs/RS_GSE_to_ephem_quat -i $INFILE -e $out_ephem_file -q $out_quats_file
   exit $?
else
  echo "output files already exist, skipping $INFILE"
  exit 0
fi





