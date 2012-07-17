#!/bin/bash
######################################################################
#
# ORIGINAL AUTHOR: Thomas Werne
# COMPANY: Jet Propulsion Laboratory
# VERSION: 
#
# File Name:     oscat2-reprocess.sh
# Creation Date: 09 April 2012
#
# $Author$
# $Date$
# $Revision$
#
# Copyright 2012, by the California Institute of Technology.
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

#######################################################################
# Custom file un/locking commands.  flock apparently doesn't work 
# across NFS, so we need to homebrew.
#
# Uses mkdir for atomicity.  

#######################################################################
# Lock a lockfile.  The lockfile filename should have no spaces in it.
# Accepts a second parameter (PROC_ID), which is used as the name of a
# blank file that is written into the lockfile directory.  This
# makes it easy to tell who owns the lock.
function lock () {
    LOCKFILE="$1"
    PROC_ID="$2"

    echo "$PROC_ID trying to lock: $LOCKFILE"
    echo "$PROC_ID holds: $HELD_LOCKS"
    echo

    echo "$HELD_LOCKS" | grep "$LOCKFILE" > /dev/null 2>&1
    HAVE_LOCK=$? 

    # Check to see if we already hold the lock
    if [[ $HAVE_LOCK -eq 0 ]]; then
        return 0
    fi

    mkdir "$LOCKFILE" > /dev/null 2>&1
    while [[ $? -ne 0 ]]; do
        echo "$PROC_ID lock attempt failed.  Block."
        sleep 1
        mkdir "$LOCKFILE" > /dev/null 2>&1
    done

    # If a PROC_ID is passed, create a 
    if [[ -n "$PROC_ID" ]]; then
        touch "$LOCKFILE/$PROC_ID"
    fi

    HELD_LOCKS="$HELD_LOCKS $LOCKFILE"

    echo "$PROC_ID lock attempt successful."
    echo "$PROC_ID holds: $HELD_LOCKS"
    echo
}

#######################################################################
# Release a held lock file
function unlock () {
    LOCKFILE="$1"
    
    echo "Releasing lock: $LOCKFILE"

    echo "$HELD_LOCKS" | grep "$LOCKFILE" > /dev/null 2>&1
    HAVE_LOCK=$? 
    
    # Check to see if we already hold the lock
    if [[ $HAVE_LOCK -eq 0 ]]; then
        rm -rf "$LOCKFILE" > /dev/null 2>&1
        HELD_LOCKS="${HELD_LOCKS/ $LOCKFILE/}"
    fi

    echo "Still holding: $HELD_LOCKS"
    echo
}

#######################################################################
# Release all held locks
function unlock_all () {
    rm -rf $HELD_LOCKS
    HELD_LOCKS=""
}

#######################################################################
# Print a prompt
function os2_reproc_prompt () {
cat <<_EOF
================================================================
  OSCAT-2 Reprocessing
================================================================

 0 QUIT
      Exit reprocessing
 1 STAGE
      Stages data for a rev.
 2 GENERATE
      Generates the folder structure for the rev.
 3 L1BHDF-TO-L1B
      Converts the L1B HDF file to simulation L1B format
 4 L1B-TO-L2A
      Performs L1B to L2A processing
 5 L2A-FIX-ISRO-COMPOSITES
 6 L2A-TO-L2B
      Performs L2A to L2B processing
 7 L2B-MEDIAN-FILTER
      Applies the L2B median filtering
 8 L2B-TO-NETCDF
      Converts the L2B file to NetCDF format
 9 LINK
      Symlink files in a directory structure
10 MAKE-ARRAYS
      Extract arrays from dataset
11 FIXUP
      Apply bias correction
12 BUILD-MD5S
      Create md5 checksums
13 CLEAN
      Removed unnecessary data files
================================================================

_EOF
    echo -n "> "
}

#######################################################################
# Convert command numbers to command names
function os2_reproc_get_command() {
(
    COMMAND=$1
    
    case "$COMMAND" in
    0)  echo "QUIT"
        ;;
    1)  echo "STAGE"
        ;;
    2)  echo "GENERATE"
        ;;
    3)  echo "L1BHDF-TO-L1B"
        ;;
    4)  echo "L1B-TO-L2A"
        ;;
    5)  echo "L2A-FIX-ISRO-COMPOSITES"
        ;;
    6)  echo "L2A-TO-L2B"
        ;;
    7)  echo "L2B-MEDIAN-FILTER"
        ;;
    8)  echo "L2B-TO-NETCDF"
        ;;
    9)  echo "LINK"
        ;;
    10) echo "MAKE-ARRAYS"
        ;;
    11) echo "FIXUP"
        ;;
    12) echo "BUILD-MD5S"
        ;;
    13) echo "CLEAN"
        ;;
    *)  echo "$COMMAND"
        ;;
    
    esac
)
}

#######################################################################
# Download and extract existing L1B and L2B HDF files
function os2_reproc_stage () {
(
    echo -n "Rev: "
    read REV ; tty > /dev/null 2>&1; 
        [[ $? -eq 0 ]] || echo "$REV"
    echo -n "L1B HDF Directory: "
    read DIR_OS2L1B_HDF; tty > /dev/null 2>&1; 
        [[ $? -eq 0 ]] || echo "$DIR_OS2L1B_HDF"

    L1B_H5_BASE_DIR=/u/potr-r1/fore/ISRO/L1B_v1.3_all

    mkdir -p "$DIR_OS2L1B_HDF" > /dev/null 2>&1
    cd "$DIR_OS2L1B_HDF"

    FNAME=`echo $L1B_H5_BASE_DIR/S1L1B*_$REV.h5.gz`
    gunzip -c $FNAME > `basename ${FNAME/.gz/}`

    return 0
)
}

#######################################################################
# Generate the rev directory structure.
# Based on code from A. Fore
function os2_reproc_generate_directory_structure () {
(
    # Read process-specific variables
    echo -n "Rev: "
    read REV; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$REV"
    echo -n "L1B HDF Directory: "
    read DIR_OS2L1B_HDF; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$DIR_OS2L1B_HDF"
    echo -n "Output Directory:  "
    read DIR_OUT_BASE; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$DIR_OUT_BASE"
    echo -n "Template config file: "
    read TEMPLATE_FILE; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$TEMPLATE_FILE"
    
    OS2_MATLAB_INC_DIR="/u/potr-r0/fore/ISRO/mat"
    
    L1B_HDF_FILE=`ls $DIR_OS2L1B_HDF/S1L1B*_${REV}.h5 | tail -n 1`
    
    SIM_DIR="$DIR_OUT_BASE/$REV"
    
    if ! [[ -d $SIM_DIR ]]; then
        mkdir -p $SIM_DIR
    fi
    
    L1B_TIMES_FILE="$SIM_DIR/times.txt"
    
    # Extract orbit start and end-time
    (
        echo "addpath('$OS2_MATLAB_INC_DIR');"
        echo "OS2L1B_Extract_Times('$L1B_HDF_FILE', '$L1B_TIMES_FILE');"
    ) | matlab-7.11 -nodisplay -nojvm
    RETVAL=$?
    
    SEC_YEAR=`head -1 $L1B_TIMES_FILE`
    
    # Obtain the n6h ECMWF filename
    CLOSEST_NWP_STR=`cat $L1B_TIMES_FILE | tail -1`
    NCEP_FILENAME_STR='SNWP1'$CLOSEST_NWP_STR
    
    CONFIG_FILE="${SIM_DIR}/ISRO.rdf"
    
    sed -e "s:NUDGE_WINDFIELD_FILE *= DUMMY_FILENAME:NUDGE_WINDFIELD_FILE = /u/potr-r0/fore/ECMWF/nwp1/$NCEP_FILENAME_STR:" \
        -e "s:L1B_HDF_FILE *= DUMMY_FILENAME:L1B_HDF_FILE = ../../$L1B_HDF_FILE:" \
        -e "s:ATTEN_MAP_SEC_YEAR *= DUMMY:ATTEN_MAP_SEC_YEAR = $SEC_YEAR:" \
                $TEMPLATE_FILE > $CONFIG_FILE
    RETVAL=$(($RETVAL || $?))

    return $RETVAL
)
}

#######################################################################
# Convert L1BHDF files to local-format L1B files
# Based on code from A. Fore
function os2_reproc_l1bhdf_to_l1b () {
(
    echo -n "Rev: "
    read REV; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$REV"
    echo -n "Base directory: "
    read BASEDIR; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$BASEDIR"
    echo -n "Config: "
    read CONFIG_FILE; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$CONFIG_FILE"

    cd "$BASEDIR/$REV"
    
    l1b_isro_to_l1b_v1.3 -c "${CONFIG_FILE}" -hhbias 0.5 -vvbias 0.6
    RETVAL=$?

    return $RETVAL
)
}

#######################################################################
# Convert from local-format L1B to L2A files
# Based on code from A. Fore
function os2_reproc_l1b_to_l2a () {
(
    echo -n "Rev: "
    read REV; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$REV"
    echo -n "Base directory: "
    read BASEDIR; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$BASEDIR"
    echo -n "Config: "
    read CONFIG_FILE; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$CONFIG_FILE"

    cd "$BASEDIR/$REV"
    
    l1b_to_l2a $CONFIG_FILE
    RETVAL=$?

    return $RETVAL
)
}

#######################################################################
# Based on code from A. Fore
function os2_reproc_l2a_fix_isro_composites () {
(
    echo -n "Rev: "
    read REV; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$REV"
    echo -n "Base directory: "
    read BASEDIR; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$BASEDIR"
    echo -n "Config: "
    read CONFIG_FILE; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$CONFIG_FILE"

    cd "$BASEDIR/$REV"
    
    l2a_fix_ISRO_composites -c "$CONFIG_FILE" -o l2a_flagged.dat -kp
    RETVAL=$?

    return $RETVAL
)
}

#######################################################################
# Convert from local-format L2A to L2B files
# Based on code from A. Fore
function os2_reproc_l2a_to_l2b () {
(
    echo -n "Rev: "
    read REV; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$REV"
    echo -n "Base directory: "
    read BASEDIR; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$BASEDIR"
    echo -n "Config: "
    read CONFIG_FILE; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$CONFIG_FILE"

    cd "$BASEDIR/$REV"
    L2A_FNAME=`awk '/L2A_FILE/ { print $3 }' "$CONFIG_FILE"`
    
    #mv "$L2A_FNAME" "$L2A_FNAME.orig"
    rm -f "${L2A_FNAME}"
    mv l2a_flagged.dat "$L2A_FNAME"

    l2a_to_l2b "$CONFIG_FILE"
    RETVAL=$?

    return $RETVAL
)
}
    

#######################################################################
# Do L2B median filtering
# Based on code from A. Fore
function os2_reproc_l2b_median_filter () {
(
    echo -n "Rev: "
    read REV; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$REV"
    echo -n "Base directory: "
    read BASEDIR; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$BASEDIR"
    echo -n "Config: "
    read CONFIG_FILE; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$CONFIG_FILE"

    cd "$BASEDIR/$REV"

    OUTFILE="l2b_S3.dat"

    l2b_medianfilter -c "${CONFIG_FILE}" -o "${OUTFILE}"
    RETVAL=$?

    return $RETVAL
)
}

#######################################################################
# Convert from local-format L2B to NetCDF files
function os2_reproc_l2b_to_netcdf () {
(
    echo -n "Rev: "
    read REV; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$REV"
    echo -n "Base directory: "
    read BASEDIR; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$BASEDIR"
    echo -n "Config: "
    read CONFIG_FILE; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$CONFIG_FILE"

    cd "$BASEDIR/$REV"
    L1B_HDF_FNAME=`awk '/L1B_HDF_FILE/ {print $3}' $CONFIG_FILE`

    INFILE=l2b.dat
    # TODO Change this
    OUTFILE=`basename "$L1B_HDF_FNAME"`
    OUTFILE="${OUTFILE/h5/nc}"
    OUTFILE="${OUTFILE/L1B/l2b}"
    OUTFILE="${OUTFILE/S1/os2_}"

    TIMESFILE="/u/potr-r1/fore/ISRO/orb_ele/OS2_orb_ele_120716.dat"

    os2_l2b_to_netcdf --l1bhdf "$L1B_HDF_FNAME" --nc "$OUTFILE"  --l2b "$INFILE" --times "$TIMESFILE"
    RETVAL=$?

    return $RETVAL
)
}

#######################################################################
# Apply NetCDF fixup
function os2_fixup () {
(
    echo -n "Rev: "
    read REV; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$REV"
    echo -n "Base directory: "
    read BASEDIR; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$BASEDIR"
    echo -n "Config: "
    read CONFIG_FILE; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$CONFIG_FILE"


    OS2_MATLAB_INC_DIR="/u/potr-r0/werne/QScatSim/mat"

    cd "$BASEDIR/$REV"
    L1B_HDF_FNAME=`awk '/L1B_HDF_FILE/ {print $3}' $CONFIG_FILE`

    OUTFILE=`basename "$L1B_HDF_FNAME"`
    OUTFILE="${OUTFILE/h5/nc}"
    OUTFILE="${OUTFILE/L1B/l2b}"
    OUTFILE="${OUTFILE/S1/os2_}"

    # Apply fixup
    (
        echo "addpath('$OS2_MATLAB_INC_DIR');"
        echo "os2_netcdf_fixup('$OUTFILE');"
    ) | matlab-7.11 -nodisplay
    RETVAL=$?
    
    return $RETVAL
)
}

#######################################################################
# Link a files from a directory structure
function os2_reproc_link() {
(
    echo -n "Rev: "
    read REV; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$REV"
    echo -n "Destination Base Directory: "
    read DIR_OUT_BASE; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$DIR_OUT_BASE"
    echo -n "Source Base Directory: "
    read SRC_DIR; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$SRC_DIR"
    echo -n "File name: "
    read FNAME; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$FNAME"

    rm -f "$DIR_OUT_BASE/$REV/$FNAME"
    RETVAL=$?
    
    mkdir -p "$DIR_OUT_BASE/$REV/"
    ln "$SRC_DIR/$REV/$FNAME" "$DIR_OUT_BASE/$REV/$FNAME"
    RETVAL=$(($RETVAL || $?))
    
    return $RETVAL
)
}

#######################################################################
# Extract data into arrays
function os2_reproc_make_arrays() {
(
    echo -n "Rev: "
    read REV; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$REV"
    echo -n "Base directory: "
    read BASEDIR; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$BASEDIR"
    echo -n "Config: "
    read CONFIG_FILE; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$CONFIG_FILE"
    echo -n "Type (SEL|NCEP): "
    read TYPE; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$TYPE"
    
    cd "$BASEDIR/$REV"

    case "${TYPE}" in
    SEL) 
        l2b_to_arrays l2b.dat sel.arr 0
        RETVAL=$?
        ;;
    NCEP)
        l2b_to_arrays l2b.dat ncep.arr 5
        RETVAL=$?
        ;;
    *)  echo "Unknown command"
        RETVAL=$?
        ;;
    esac

    return $RETVAL
)
}

#######################################################################
# Create md5 checksums
function os2_build_md5s () {
(
    echo -n "Rev: "
    read REV; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$REV"
    read BASEDIR; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$BASEDIR"

    cd "$BASEDIR/$REV"

    RETVAL=0
    for FILE in `ls os2_l2b.nc`; do
        md5sum "$FILE" > "$FILE.md5"
        RETVAL=$(($RETVAL || $?))
    done

    return $RETVAL
)
}

#######################################################################
# Clean the directories
function os2_reproc_clean () {
(
    echo -n "Rev: "
    read REV; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$REV"
    echo -n "L1B HDF Directory: "
    read DIR_OS2L1B_HDF; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$DIR_OS2L1B_HDF"
    echo -n "Output Directory:  "
    read DIR_OUT_BASE; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$DIR_OUT_BASE"
    echo -n "Config: "
    read CONFIG_FILE; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$CONFIG_FILE"
    echo -n "Clean level: "
    read LEVEL; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$LEVEL"

    RETVAL=0

    if [[ "x$LEVEL" == "x" ]]; then
        LEVEL=3
    fi

    cd "$DIR_OUT_BASE/$REV"
    L1B_FNAME=`awk '/L1B_FILE/ { print $3 }' "$CONFIG_FILE"`
    L2A_FNAME=`awk '/L2A_FILE/ { print $3 }' "$CONFIG_FILE"`
    L2B_FNAME=`awk '/L2B_FILE/ { print $3 }' "$CONFIG_FILE"`
    L1B_HDF_FNAME=`awk '/L1B_HDF_FILE/ { print $3 }' "$CONFIG_FILE"`

    # Spoof switch with fallthrough
    if [[ $LEVEL -le 3 ]]; then
        rm -f \
            "nn_train.dat" \
            "$L1B_FNAME" \
            ephem.dat \
            "$L2A_FNAME"
    fi
#    if [[ $LEVEL -le 2 ]]; then
#        rm -f \
#
#    fi
    if [[ $LEVEL -le 1 ]]; then
        rm -f \
            "$L2B_FNAME"
    fi
    if [[ $LEVEL -le 0 ]]; then
        rm -f \
            "$L2A_FNAME"
    fi

    return $RETVAL
)
}

#######################################################################
# Given a command, execute it.  Wrap the command with START and 
# FINISH tags.
function os2_reproc_process_command () {
    COMMAND=$1
    
    echo "STARTING: $COMMAND"
    
    case "$COMMAND" in
    QUIT)
        exit 0
        ;;
    STAGE)
        os2_reproc_stage
        RETVAL=$?
        ;;
    GENERATE) 
        os2_reproc_generate_directory_structure 
        RETVAL=$?
        ;;
    L1BHDF-TO-L1B)
        os2_reproc_l1bhdf_to_l1b
        RETVAL=$?
        ;;
    L1B-TO-L2A)
        os2_reproc_l1b_to_l2a
        RETVAL=$?
        ;;
    L2A-FIX-ISRO-COMPOSITES)
        os2_reproc_l2a_fix_isro_composites
        RETVAL=$?
        ;;
    L2A-TO-L2B)
        os2_reproc_l2a_to_l2b
        RETVAL=$?
        ;;
    L2B-MEDIAN-FILTER)
        os2_reproc_l2b_median_filter
        RETVAL=$?
        ;;
    L2B-TO-NETCDF)
        os2_reproc_l2b_to_netcdf
        RETVAL=$?
        ;;
    FIXUP)
        os2_fixup
        RETVAL=$?
        ;;
    LINK)
        os2_reproc_link
        RETVAL=$?
        ;;
    MAKE-ARRAYS)
        os2_reproc_make_arrays
        RETVAL=$?
        ;;
    BUILD-MD5S)
        os2_build_md5s
        RETVAL=$?
        ;;
    CLEAN)
        os2_reproc_clean
        RETVAL=$?
        ;;
    *)
        echo "Unknown command"
        RETVAL=$?
        ;;
    esac
    
    echo
    echo "FINISHED: $COMMAND"
    echo

    return $RETVAL
}

#######################################################################
# Execute commands, echoing values into the STDIN of those processes
function os2_reproc_execute_automated_cmd () {
(
    REV="$1"
    CMD=`echo "$2" | cut -f 1 -d \ `
    ARGS=`echo "$2" | cut -f 2- -d \ `
    RETVAL=1

    case "$CMD" in 
    STAGE)
        INPUT="$REV\n$L1B_HDF_DIR\n"
        ;;
    GENERATE)
        INPUT="$REV\n$L1B_HDF_DIR\n$OUTPUT_DIR\n$GENERIC_CFG\n"
        ;;
    L1BHDF-TO-L1B)
        INPUT="$REV\n$OUTPUT_DIR\n$CFG\n"
        ;;
    L1B-TO-L2A)
        INPUT="$REV\n$OUTPUT_DIR\n$CFG\n"
        ;;
    L2A-FIX-ISRO-COMPOSITES)
        INPUT="$REV\n$OUTPUT_DIR\n$CFG\n"
        ;;
    L2A-TO-L2B)
        INPUT="$REV\n$OUTPUT_DIR\n$CFG\n"
        ;;
    L2B-MEDIAN-FILTER)
        ARGS=`echo "$ARGS" | tr ' ' '\n'`
        INPUT="$REV\n$OUTPUT_DIR\n$CFG\n$ARGS\n"
        ;;
    L2B-TO-NETCDF)
        INPUT="$REV\n$OUTPUT_DIR\n$CFG\n"
        ;;
    FIXUP)
        INPUT="$REV\n$OUTPUT_DIR\n$CFG\n"
        ;;
    LINK)
        ARGS=`echo "$ARGS" | tr ' ' '\n'`
        INPUT="$REV\n$OUTPUT_DIR\n$ARGS\n"
        ;;
    TDV)
        ARGS="l2b_flagged_GS.dat"
        INPUT="$REV\n$OUTPUT_DIR\n$ARGS\n$CFG\n"
        ;;
    MAKE-ARRAYS)
        INPUT="$REV\n$OUTPUT_DIR\n$CFG\n$ARGS\n"
        ;;
    L2C)
        INPUT="$REV\n$OUTPUT_DIR\n"
        ;;
    BUILD-MD5S)
        INPUT="$REV\n$OUTPUT_DIR\n"
        ;;
    CLEAN)
        INPUT="$REV\n$L1B_HDF_DIR\n$OUTPUT_DIR\n$CFG\n$ARGS\n"
        ;;
    esac

    echo
    echo "Sending: '$INPUT' to '$CMD'"
    echo

    echo -e "$INPUT" | os2_reproc_process_command "$CMD"
    RETVAL=$?

    return $RETVAL
)
}

#######################################################################
# Given a CFG file and REV, process it completely.
function os2_reproc_by_rev () {
(
    CFG_FILE="$1"
    REV="$2"

    source "$CFG_FILE"
    if [[ -z "$LOG_DIR" ]]; then
        LOG_DIR=./
    fi

    os2_reproc_execute_automated_cmd "$REV" "GENERATE"; EXIT=$?
    if [[ $EXIT -ne 0 ]]; then
        return $EXIT
    fi
    os2_reproc_execute_automated_cmd "$REV" "L1BHDF-TO-L1B"; EXIT=$?
    if [[ $EXIT -ne 0 ]]; then
        return $EXIT
    fi
    os2_reproc_execute_automated_cmd "$REV" "L1B-TO-L2A"; EXIT=$?
    if [[ $EXIT -ne 0 ]]; then
        return $EXIT
    fi
    os2_reproc_execute_automated_cmd "$REV" "L2A-FIX-ISRO-COMPOSITES"; EXIT=$?
    if [[ $EXIT -ne 0 ]]; then
        return $EXIT
    fi
    os2_reproc_execute_automated_cmd "$REV" "L2A-TO-L2B"; EXIT=$?
    if [[ $EXIT -ne 0 ]]; then
        return $EXIT
    fi
    os2_reproc_execute_automated_cmd "$REV" "L2B-MEDIAN-FILTER S3"; EXIT=$?
    if [[ $EXIT -ne 0 ]]; then
        return $EXIT
    fi
    os2_reproc_execute_automated_cmd "$REV" "L2B-TO-NETCDF S3"; EXIT=$?
    if [[ $EXIT -ne 0 ]]; then
        return $EXIT
    fi
    os2_reproc_execute_automated_cmd "$REV" "BUILD-MD5S"; EXIT=$?
    if [[ $EXIT -ne 0 ]]; then
        return $EXIT
    fi

)
}

#######################################################################
# Scheduler to chose which command to execute next
# Requires that CMD_FILE be locked before being called
function os2_reproc_schedule () {
(
    CMDS=`cut -f 1-2 "$CMD_FILE"`
    
    # You can duplicate this idiom to define lower priority commands

    # TOP PRIORITY: Look for L2A-TO-L2B
    PRIORITY_CMD=`echo "$CMDS" | grep "L2A-TO-L2B"`
    if [[ $? -eq 0 ]]; then
        # Found an L2A-TO-L2B as a valid next command
        PRIORITY_CMD=`echo "$PRIORITY_CMD" | head -n 1`
        NUM_RUNS=`grep "L2A-TO-L2B" "$PROC_FILE" | wc -l`
        echo "$NUM_RUNS/$MAX_L2A_TO_L2B runs executing."
        if [[ $NUM_RUNS -lt $MAX_L2A_TO_L2B ]]; then
            # There are fewer running L2A-TO-L2B processes than allowed.
            # Do this one.
            echo "$PRIORITY_CMD"
            return 0
        fi
    fi

    # Either there are no L2A-TO-L2B processes to run, or there are too
    # many already running.  Just grab the next legitimate command
    CMD=`echo "$CMDS" | grep -m 1 -v "L2A-TO-L2B"`
    echo "$CMD"
)
}

#######################################################################
# Given a CFG file, execute the commands in the command file.
function os2_reproc_by_file () {
    GFG_FILE="$1"

    source "$CFG_FILE"

    # Multiple concurrent processes.  We need a unique ID to use in 
    # logging.
    UNIQ="`hostname -s`:$$"

    # Define per-process logfile
    LOG_FILE="$LOG_DIR/$UNIQ.log"

    # Define lockfiles
    CMD_LOCK="$LOCKDIR/$CMD_FILE"
    PROC_LOCK="$LOCKDIR/$PROC_FILE"
    ERR_LOCK="$LOCKDIR/$ERR_FILE"
    SUCCESS_LOCK="$LOCKDIR/$SUCCESS_FILE"

    ERR_DIR="$LOG_DIR/error"
    SUC_DIR="$LOG_DIR/success"

    if ! [[ -d "$LOG_DIR" ]]; then
        mkdir -p "$LOG_DIR" > /dev/null 2>&1
        mkdir -p "$ERR_DIR" > /dev/null 2>&1
        mkdir -p "$SUC_DIR" > /dev/null 2>&1
    fi
    if ! [[ -d "$LOCKDIR" ]]; then
        mkdir -p "$LOCKDIR" > /dev/null 2>&1
    fi

    # We want to work on a COPY of the command file.  If that copy
    # doesn't already exist, create it.
    NEW_CMD_FILE="$CMD_FILE.working"
    lock "$CMD_LOCK" "$UNIQ"
    if ! [[ -e "$NEW_CMD_FILE" ]]; then 
        cp -f "$CMD_FILE" "$NEW_CMD_FILE"
    fi
    unlock "$CMD_LOCK"

    CMD_FILE="$NEW_CMD_FILE"
    CMD_LOCK="$LOCKDIR/$CMD_FILE"

    lock "$CMD_LOCK" "$UNIQ"
    # Let ALL be a synonym for all actual processing
    sed -i -e 's/ALL/STAGE,GENERATE,L1BHDF-TO-L1B,L1B-TO-L2A,L2A-FIX-ISRO-COMPOSITES,L2A-TO-L2B,L2B-TO-NETCDF,MAKE-ARRAYS SEL,MAKE-ARARYS NCEP,FIXUP/' "$CMD_FILE"
    unlock "$CMD_LOCK"

    while [[ -s "$CMD_FILE" ]]; do
    (
        # Yes, we really do want to spawn a new subshell for each processing run.
        # This allows the logfiles to be overwritten after each command.

        # Try to deal with funny NFS no-mount bug...
        # /u/potr-r1 is the NFS mount point for the 21 TB drive.
        # Look for it, and if `ls` fails, then the NFS mount must be bad
        ls '/u/potr-r1' > /dev/null 2>&1
        if [ $? -ne 0 ]; then
            echo "Possible NFS no-mount bug... sleeping"
            sleep 30
            continue
        fi


        # Since this is the ONLY place we nest mutexes, there's no
        # threat of deadlock
        lock "$CMD_LOCK" "$UNIQ"
        lock "$PROC_LOCK" "$UNIQ"

        # Give CMD_FILE to the scheduler.  It will chose the next command.
        # CMD is: "Revision\tCommand"
        CMD=`os2_reproc_schedule "$CMD_FILE"`
        if [ -z "$CMD" ]; then
            # If a NULL command is returned, exit gracefully
            # Make sure we unlock first
            unlock "$PROC_LOCK"
            unlock "$CMD_LOCK"

            sleep 30
            return 0
        fi

        # Parse the command line
        # Get the corresponding line in the file
        # Pick off the revision, command, and subsequent commands
        CMD_LINE=`grep -m 1 "^$CMD" "$CMD_FILE"`
        REV=`echo "$CMD_LINE" | cut -f 1 -d,`
        CMD=`echo "$CMD_LINE" | cut -f 2 -d,`
        NEXT_CMD_LINE=`echo "$CMD_LINE" | cut -f 3- -d,`

        START=`date`

        # Log to the PROC_FILE
        echo "$REV,$CMD,$UNIQ,$START" >> "$PROC_FILE"
        unlock "$PROC_LOCK"

        # Delete the comamnd from the file so no one else tries 
        # to execute it
        sed -i "/$CMD_LINE/d" "$CMD_FILE"       

        unlock "$CMD_LOCK"

        # Execute the command
        echo "Executing '$CMD' for '$REV'."
        echo "The following commands are '$NEXT_CMD_LINE'."
        echo

        os2_reproc_execute_automated_cmd "$REV" "$CMD"
        RETVAL=$?
        END=`date`

        # Calculate the runtime using `date`
        RUNTIME=$((`date --date="$END" +%s` - `date --date="$START" +%s`))

        if [[ $RETVAL -eq 0 ]]; then
            # If the command returns successfully, then log it to the 
            # SUCCESS_FILE and write the subsequent command (if available
            # to CMD_FILE)
  
            lock "$SUCCESS_LOCK" "$UNIQ"
            echo "$REV,$CMD,$UNIQ,$START,$END,$RUNTIME" >> \
                "$SUCCESS_FILE"
            unlock "$SUCCESS_LOCK"
    
            lock "$CMD_LOCK" "$UNIQ"
            if [[ -n "$NEXT_CMD_LINE" ]]; then
                # If there are more commands for this REV, we need to 
                # reinsert them into the command file
                if [[ -s "$CMD_FILE" ]]; then
                    # Use sed to prepend the command
                    sed -i -e "1i$REV,$NEXT_CMD_LINE" "$CMD_FILE"
                else
                    # sed can't prepend to an empty file
                    echo "$REV,$NEXT_CMD_LINE" >> "$CMD_FILE"
                fi
            fi
            unlock "$CMD_LOCK"
        else
            # If the command returns unsuccessfully, log it to the
            # ERR_FILE (along with any subsequent commands) and
            # copy the process log file to a persistent file.

            lock "$ERR_LOCK" "$UNIQ"
            echo "$REV,$CMD,$NEXT_CMD_LINE,$UNIQ,$START,$END,$RUNTIME" >> "$ERR_FILE"
            unlock "$ERR_LOCK"
        fi       

        # Remove the line from the processing file
        lock "$PROC_LOCK" "$UNIQ"
        sed -i "/$REV,$CMD.*/d" "$PROC_FILE"
        unlock "$PROC_LOCK"

        # The last thing to do is handle the logfile.
        # This is a little tricky since the logfile is populated by
        # redirecting stdout & stderr from this subshell, and the
        # associated file descriptors may not flush until the
        # subshell exits (i.e. AFTER a file copy).
        
        # First, manually close stdout and sterr, then copy the log files
        exec 1>&-
        exec 2>&-
        if [[ $RETVAL -eq 0 ]]; then
            # The command was sucessful, see if we need to save the
            # log file
            if [[ "$(echo "$KEEP_SUCCESS_LOGS" | \
                    tr "[:lower:]" "[:upper:]")" == "YES" ]]; then
                cp "$LOG_FILE" "$SUC_DIR/$REV-$CMD-$UNIQ.log"
            fi
        else
            # The command ended in an error.  Log it.
            cp "$LOG_FILE" "$ERR_DIR/$REV-$CMD-$UNIQ.log"
        fi

    ) 2>&1 | sed -e "s/^/(`date +"%F %T"`) /" | tee "$LOG_FILE" | sed -e "s/^/[$UNIQ] /"
    #       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    # Write the date/time of each line into the files.
    # Timestamps are useful.
    #
    #                                           ^^^^^^^^^^^^^^
    # Write the results of this run into a per-process logfile
    # in addition to dumping to the command line.  This log
    # is rewritten on each iteration of the loop.
    #
    #                                                             ^^^^^^^^^^^^^^^^^^^^^ 
    # Prepend an ID to the output of the processes.
    # This makes reading the outputs and comparing the results
    # in the files MUCH easier.
    done 

    # If CMD is empty, delete it.  Don't worry if there are other 
    # processes still running: they APPEND subsequent commands to CMD.
    # So, if we delete it now and someone else needs it, it will get
    # created later.
    lock "$CMD_LOCK" "$UNIQ"
    rm -f "$CMD_FILE"
    unlock "$CMD_LOCK"
}

#######################################################################
# Print an interactive prompt and do the user's processing
function os2_reproc_interactive () {
    while true; do
        os2_reproc_prompt 
        read COMMAND
        echo
        COMMAND=`os2_reproc_get_command "$COMMAND"`
        os2_reproc_process_command "$COMMAND"
    done
}

#######################################################################
# Main

# We fire off a bunch of children.
# Trap SIGINT and exit gracefully so that the children are killed as well.
trap "exit" SIGINT

# Release all file locks on exit
trap "unlock_all" EXIT

if [[ $# -ge 1 ]]; then
    CFG_FILE="$1"
    if [[ $# -eq 1 ]]; then
        # If there is one parameter, it's a CFG file.  Execute the commands 
        # in the command file specified in it.
        os2_reproc_by_file "$CFG_FILE"
    else
        # If there are two, the first is a CFG and the second is a REV.  
        # Use the variables specified in the CFG and reproc that REV.
        REV="$2"
        os2_reproc_by_rev "$CFG_FILE" "$REV"
    fi
else
    # If there are no parmeters, run interactively
    os2_reproc_interactive
fi
