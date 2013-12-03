#!/usr/bin/env bash
#==============================================================#
# Copyright (C) 2013, California Institute of Technology.      #
# U.S. Government sponsorship acknowledged.                    #
#==============================================================#
#----------------------------------------------------------------------
# NAME
#    PreProcessGSE.sh
#
# SYNOPSIS
#    PreProcessGSE.sh -i GSE_file -o out_directory
#
# DESCRIPTION
#    Parses, sorts and removes duplicate and junk data from GSE_file;
#    writes a new GSE file in out_directory with a useful filename.
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

USAGE="$0 -i gse_file -o out_gse_directory"

iflag=0
oflag=0
while getopts 'i:o:' OPTION
do
  case $OPTION in 
    i) iflag=1
       INFILE="$OPTARG"
       ;;
    o) oflag=1
       OUT_DIR="$OPTARG"
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

TEMPFILE=`mktemp`
echo $INFILE > $TEMPFILE
RS_GSE_merge -i $TEMPFILE -od $OUT_DIR
retval=$?

rm -f $TEMPFILE
if [ $retval -ne 0 ]; then
  echo "Some error running RS_GSE_merge"
  exit 1
fi

exit 0