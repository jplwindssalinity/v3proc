#!/bin/bash
######################################################################
#
# ORIGINAL AUTHOR: Thomas Werne
# COMPANY: Jet Propulsion Laboratory
# VERSION: 
#
# File Name:     quikscat-reprocess.sh
# Creation Date: 6 Dec 2010
#
# $Author$
# $Date$
# $Revision$
#
# Copyright 2010, by the California Institute of Technology.
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

    echo
    echo "$PROC_ID trying to lock: $LOCKFILE"
    echo "$PROC_ID holds: $HELD_LOCKS"
    echo

    echo "$HELD_LOCKS" | grep "$LOCKFILE" > /dev/null 2>&1
    HAVE_LOCK=$? 

    # Check to see if we already hold the lock
    if [[ $HAVE_LOCK -eq 0 ]]; then
        return 0
    fi

    mkdir "$1" > /dev/null 2>&1
    while [[ $? -ne 0 ]]; do
        sleep 1
        mkdir "$1" > /dev/null 2>&1
    done

    # If a PROC_ID is passed, create a 
    if ! [[ -z "$PROC_ID" ]]; then
        touch "$LOCKFILE/$PROC_ID"
    fi

    HELD_LOCKS="$HELD_LOCKS $LOCKFILE"
}

#######################################################################
# Release a held lock file
function unlock () {
    LOCKFILE="$1"
    
    echo
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
function qs_reproc_prompt () {
    
    echo "================================================================"
    echo "  QuikSCAT Reprocessing"
    echo "================================================================"
    echo ""
    echo "0  QUIT"
    echo "      Exit reprocessing"
    echo "1  STAGE"
    echo "      Stages data for a rev."
    echo "2  GENERATE"
    echo "      Generates the folder structure for the rev."
    echo "3  L1BHDF-TO-L1B"
    echo "      Converts the L1B HDF file to simulation L1B format"
    echo "4  L1B-TO-L2A"
    echo "      Performs L1B to L2A processing"
    echo "5  L2A-FIX-QS-COMPOSITES"
    echo "6  L2A-TO-L2B"
    echo "      Performs L2A to L2B processing"
    echo "7  L2B-MEDIAN-FILTER"
    echo "      Applies the L2B median filtering"
    echo "8  L2B-TO-NETCDF"
    echo "      Converts the L2B file to NetCDF format"
    echo "9  CLEAN"
    echo "      Removed unnecessary data files"
    echo "================================================================"
    echo ""
    echo -n "> "
    
}

#######################################################################
# Convert command numbers to command names
function qs_reproc_get_command() {
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
    5)  echo "L2A-FIX-QS-COMPOSITES"
        ;;
    6)  echo "L2A-TO-L2B"
        ;;
    7)  echo "L2B-MEDIAN-FILTER"
        ;;
    8)  echo "L2B-TO-NETCDF"
        ;;
    9)  echo "CLEAN"
        ;;
    *)  echo "$COMMAND"
        ;;
    
    esac
}

#######################################################################
# Download and extract existing L1B and L2B HDF files
function qs_reproc_stage () {
    echo -n "Rev: "
    read REV
    echo -n "L1B HDF Directory: "
    read DIR_QSL1B_HDF
    echo -n "L2B HDF Directory: "
    read DIR_QSL2B_HDF

    RETVAL=0
    L1BURL="ftp://qL1B:data4me@podaac/data/L1B"
    L2BURL="ftp://podaac/pub/ocean_wind/quikscat/L2B12/data"

    ID=`awk "/^$REV/" "$REVLOG"`

    # Some REVs cross day boundaries.  I'm not sure which day the rev
    # file will land in, so just check in both days if this one does.

    DATE1=`echo $ID | sed -e 's/  */ /g' | cut -f 2 -d \  `
    DATE2=`echo $ID | sed -e 's/  */ /g' | cut -f 4 -d \  `

    YEAR1=`echo $DATE1 | sed -e 's/^\(.*\)-.*/\1/'`
    DAY1=`echo $DATE1 | sed -e 's/.*-\(.*\)T.*/\1/'`
    YEAR2=`echo $DATE2 | sed -e 's/^\(.*\)-.*/\1/'`
    DAY2=`echo $DATE2 | sed -e 's/.*-\(.*\)T.*/\1/'`

    # See if there are any existing L1B HDF files associated to this rev
    ls "$DIR_QSL1B_HDF"/*"$REV"* > /dev/null 2>&1

    if [[ $? -ne 0 ]]; then
        # Gotta download the data files
  
        # L1B HDF File
        wget -nH -N --cut-dirs=4 -P "$DIR_QSL1B_HDF" \
            "$L1BURL/$YEAR1/$DAY1/*$REV*"
        RETVAL1=$?
        if [[ ($YEAR1 -ne $YEAR2) || ($DAY1 -ne $DAY2) ]]; then
            # Cross a day bdry? Try the next day too.
            wget -nH -N --cut-dirs=4 -P "$DIR_QSL1B_HDF" \
                "$L1BURL/$YEAR2/$DAY2/*$REV*"
            [[ ($RETVAL1 -eq 0) || ($? -eq 0) ]]; RETVAL1=$?
        fi
        [[ ($RETVAL -eq 0) && ($RETVAL1 -eq 0) ]]; RETVAL=$?

    fi
    gunzip -f "$DIR_QSL1B_HDF"/*"$REV"*.gz > /dev/null 2>&1

    # See if there are any existing L2B HDF files associated to this rev
    ls "$DIR_QSL2B_HDF"/*"$REV"* > /dev/null 2>&1
    if [[ $? -ne 0 ]]; then
        # Gotta download the data files
  
        # L2B HDF File
        wget -nH -N --cut-dirs=4 -P "$DIR_QSL2B_HDF" \
            "$L2BURL/$YEAR1/$DAY1/*$REV*"
        RETVAL1=$?
        if [[ ($YEAR1 -ne $YEAR2) || ($DAY1 -ne $YEAR2) ]]; then
            # Cross a day bdry? Try the next day too.
            wget -nH -N --cut-dirs=4 -P "$DIR_QSL2B_HDF" \
                "$L2BURL/$YEAR2/$DAY2/*$REV*"
            [[ ($RETVAL1 -eq 0) || ($? -eq 0) ]]; RETVAL1=$?
        fi
        [[ ($RETVAL -eq 0) && ($RETVAL1 -eq 0) ]]; RETVAL=$?

    fi
    gunzip -f "$DIR_QSL2B_HDF"/*"$REV"*.gz > /dev/null 2>&1

    return $RETVAL
}

#######################################################################
# Generate the rev directory structure.
# Based on code from A. Fore
function qs_reproc_generate_directory_structure () {
    
    # Read process-specific variables
    echo -n "Rev: "
    read REV
    echo -n "L1B HDF Directory: "
    read DIR_QSL1B_HDF
    echo -n "L2B HDF Directory: "
    read DIR_QSL2B_HDF
    echo -n "Output Directory:  "
    read DIR_OUT_BASE
    echo -n "Template config file: "
    read TEMPLATE_FILE
    
    QS_MATLAB_INC_DIR="./mat"
    
    L1B_HDF_FILE=`ls $DIR_QSL1B_HDF/*$REV*`
    L2B_HDF_FILE=`ls $DIR_QSL2B_HDF/*$REV*.CP12`
    
    SIM_DIR="$DIR_OUT_BASE/$REV"
    
    if ! [[ -d $SIM_DIR ]]; then
        mkdir -p $SIM_DIR
    fi
    
    L1B_TIMES_FILE="$SIM_DIR/times.txt"
    
    # Extract orbit start and end-time
    (
        echo "addpath('$QS_MATLAB_INC_DIR');"
        echo "QSL1B_Extract_Times('$L1B_HDF_FILE', '$L1B_TIMES_FILE');"
    ) | matlab -nodisplay -nojvm
    RETVAL=$?
    
    SEC_YEAR=`head -1 $L1B_TIMES_FILE`
    
    # Obtain the n6h ECMWF filename
    CLOSEST_NWP_STR=`cat $L1B_TIMES_FILE | tail -1`
    NCEP_FILENAME_STR='SNWP3'$CLOSEST_NWP_STR
    
    ICE_FILENAME_STR='NRT_ICEM'${CLOSEST_NWP_STR:0:7}
    
    CONFIG_FILE=$SIM_DIR/'QS.rdf'
    
    sed -e "s:NUDGE_WINDFIELD_FILE        = DUMMY_FILENAME:NUDGE_WINDFIELD_FILE        = ../../ECMWF/$NCEP_FILENAME_STR:" \
        -e "s:L1B_HDF_FILE                = DUMMY_FILENAME:L1B_HDF_FILE                = ../../$L1B_HDF_FILE:" \
        -e "s:L2B_HDF_FILE                = DUMMY_FILENAME:L2B_HDF_FILE                = ../../$L2B_HDF_FILE:" \
        -e "s:QS_ICEMAP_FILE              = DUMMY_FILENAME:QS_ICEMAP_FILE              = ../../dat/ice/$ICE_FILENAME_STR:" \
        -e "s:ATTEN_MAP_SEC_YEAR          = DUMMY:ATTEN_MAP_SEC_YEAR          = $SEC_YEAR:" \
                $TEMPLATE_FILE > $CONFIG_FILE

    return $RETVAL
}

#######################################################################
# Convert L1BHDF files to local-format L1B files
# Based on code from A. Fore
function qs_reproc_l1bhdf_to_l1b () {

    echo -n "Rev: "
    read REV
    echo -n "Base directory: "
    read BASEDIR
    echo -n "Config: "
    read CONFIG_FILE

    (
        cd "$BASEDIR/$REV"
        
        l1b_hdf_to_l1b_fast $CONFIG_FILE
        RETVAL=$?

        return $RETVAL
    )
    RETVAL=$?

    return $RETVAL
}

#######################################################################
# Convert from local-format L1B to L2A files
# Based on code from A. Fore
function qs_reproc_l1b_to_l2a () {

    echo -n "Rev: "
    read REV
    echo -n "Base directory: "
    read BASEDIR
    echo -n "Config: "
    read CONFIG_FILE

    (
        cd "$BASEDIR/$REV"
        
        l1b_to_l2a $CONFIG_FILE
        RETVAL=$?

        return $RETVAL
    )
    RETVAL=$?

    return $RETVAL
}

#######################################################################
# Based on code from A. Fore
function qs_reproc_l2a_fix_qs_composites () {

    echo -n "Rev: "
    read REV
    echo -n "Base directory: "
    read BASEDIR
    echo -n "Config: "
    read CONFIG_FILE

    (
        cd "$BASEDIR/$REV"
        L2A_FNAME=`awk '/L2A_FILE/ { print $3 }' "$CONFIG_FILE"`
        
        l2a_fix_QS_composites -c "$CONFIG_FILE" -o l2a_flagged.dat -kp
        RETVAL=$?
        mv "$L2A_FNAME" "$L2A_FNAME.orig"
        mv l2a_flagged.dat "$L2A_FNAME"

        return $RETVAL
    )
    RETVAL=$?

    return $RETVAL
}

#######################################################################
# Convert from local-format L2A to L2B files
# Based on code from A. Fore
function qs_reproc_l2a_to_l2b () {

    echo -n "Rev: "
    read REV
    echo -n "Base directory: "
    read BASEDIR
    echo -n "Config: "
    read CONFIG_FILE

    (
        cd "$BASEDIR/$REV"
        
        l2a_to_l2b -t nn_train.dat "$CONFIG_FILE"
        RETVAL=$?

        return $RETVAL
    )
    RETVAL=$?

    return $RETVAL
}
    

#######################################################################
# Do L2B median filtering
# Based on code from A. Fore
function qs_reproc_l2b_median_filter () {

    echo -n "Rev: "
    read REV
    echo -n "Base directory: "
    read BASEDIR
    echo -n "Config: "
    read CONFIG_FILE
    echo -n "Type (GS|S3): "
    read TYPE

    (
        cd "$BASEDIR/$REV"
        L2B_HDF_FNAME=`awk '/L2B_HDF_FILE/ {print $3}' $CONFIG_FILE`

        case "$TYPE" in
        S3) 
            MEDFILT_CONFIG=tmp1.rdf
            OUTFILE=l2b_flagged_S3.dat
            sed -e 's:MEDIAN_FILTER_MAX_PASSES    = 0:MEDIAN_FILTER_MAX_PASSES    = 200:' \
                $CONFIG_FILE > "$MEDFILT_CONFIG"
            ;; 
        GS) 
            MEDFILT_CONFIG=tmp2.rdf
            OUTFILE=l2b_flagged_GS.dat
            sed -e 's:MEDIAN_FILTER_MAX_PASSES    = 0:MEDIAN_FILTER_MAX_PASSES    = 200:' \
                -e 's:WIND_RETRIEVAL_METHOD       = S3:WIND_RETRIEVAL_METHOD       = GS:' \
                    $CONFIG_FILE > "$MEDFILT_CONFIG"
            ;; 
        *)
            echo "Unknown TYPE"
            ;;
        esac
        
        l2b_medianfilter -c "$MEDFILT_CONFIG" -o "$OUTFILE" -nudgeHDF "$L2B_HDF_FNAME"
        RETVAL=$?

        return $RETVAL
    )
    RETVAL=$?

    return $RETVAL
}

#######################################################################
# Convert from local-format L2B to NetCDF files
function qs_reproc_l2b_to_netcdf () {

    echo -n "Rev: "
    read REV
    echo -n "Base directory: "
    read BASEDIR
    echo -n "Config: "
    read CONFIG_FILE
    echo -n "Type (GS|S3): "
    read TYPE

    (
        cd "$BASEDIR/$REV"
        L1B_HDF_FNAME=`awk '/L1B_HDF_FILE/ {print $3}' $CONFIG_FILE`
        L2B_HDF_FNAME=`awk '/L2B_HDF_FILE/ {print $3}' $CONFIG_FILE`

        case $TYPE in
        S3)
            INFILE=l2b_flagged_S3.dat
            OUTFILE=l2bc_flagged_S3.nc
            ;;
        GS)
            INFILE=l2b_flagged_GS.dat
            OUTFILE=l2bc_flagged_GS.nc
            ;;
        *)
            echo "Unknown TYPE"
            ;;
        esac

        l2b_to_netcdf --l2bhdf "$L2B_HDF_FNAME" --l1bhdf "$L1B_HDF_FNAME" --l2bc "$OUTFILE"  --l2b "$INFILE"
        RETVAL=$?

        return $RETVAL
    )
    RETVAL=$?

    return $RETVAL
}

#######################################################################
# Clean the directories
function qs_reproc_clean () {
    echo -n "Rev: "
    read REV
    echo -n "L1B HDF Directory: "
    read DIR_QSL1B_HDF
    echo -n "L2B HDF Directory: "
    read DIR_QSL2B_HDF
    echo -n "Output Directory:  "
    read DIR_OUT_BASE
    echo -n "Config: "
    read CONFIG_FILE
    echo -n "Clean level: "
    read LEVEL

    RETVAL=0

    if [[ "x$LEVEL" == "x" ]]; then
        LEVEL=3
    fi

    (
        cd "$DIR_OUT_BASE/$REV"
        L1B_FNAME=`awk '/L1B_FILE/ { print $3 }' $CONFIG_FILE`
        L2B_FNAME=`awk '/L2B_FILE/ { print $3 }' $CONFIG_FILE`
        L1B_HDF_FNAME=`awk '/L1B_HDF_FILE/ { print $3 }' $CONFIG_FILE`
        L2B_HDF_FNAME=`awk '/L2B_HDF_FILE/ { print $3 }' $CONFIG_FILE`

        # Spoof switch with fallthrough
        if [[ $LEVEL -le 2 ]]; then
            rm -f \
                "$L1B_HDF_FNAME" \
                "$L2B_HDF_FNAME" \
                "$L1B_FNAME" \
                "$L2A_FNAME.orig" \
                l1bhdf_to_l1b_tmpfile \
                ephem.dat \
                "tmp1.rdf" \
                "tmp2.rdf" \
                "l2b_flagged_S3.dat" \
                "l2b_flagged_GS.dat" 
        fi
        if [[ $LEVEL -le 1 ]]; then
            rm -f \
                "$L2B_FNAME"
        fi
        if [[ $LEVEL -le 0 ]]; then
            rm -f \
                "$L2A_FNAME"
        fi
    )

    return $RETVAL
}

#######################################################################
# Given a command, execute it.  Wrap the command with START and 
# FINISH tags.
function qs_reproc_process_command () {
    COMMAND=$1
    
    echo "STARTING: $COMMAND"
    
    case "$COMMAND" in
    QUIT)
        exit 0
        ;;
    STAGE)
        qs_reproc_stage
        RETVAL=$?
        ;;
    GENERATE) 
        qs_reproc_generate_directory_structure 
        RETVAL=$?
        ;;
    L1BHDF-TO-L1B)
        qs_reproc_l1bhdf_to_l1b
        RETVAL=$?
        ;;
    L1B-TO-L2A)
        qs_reproc_l1b_to_l2a
        RETVAL=$?
        ;;
    L2A-FIX-QS-COMPOSITES)
        qs_reproc_l2a_fix_qs_composites
        RETVAL=$?
        ;;
    L2A-TO-L2B)
        qs_reproc_l2a_to_l2b
        RETVAL=$?
        ;;
    L2B-MEDIAN-FILTER)
        qs_reproc_l2b_median_filter
        RETVAL=$?
        ;;
    L2B-TO-NETCDF)
        qs_reproc_l2b_to_netcdf
        RETVAL=$?
        ;;
    CLEAN)
        qs_reproc_clean
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
function qs_reproc_execute_automated_cmd () {
    REV="$1"
    CMD=`echo "$2" | cut -f 1 -d \ `
    ARGS=`echo "$2" | cut -f 2- -d \ `
    RETVAL=1

    case "$CMD" in 
    STAGE)
        INPUT="$REV\n$L1B_HDF_DIR\n$L2B_HDF_DIR\n"
        ;;
    GENERATE)
        INPUT="$REV\n$L1B_HDF_DIR\n$L2B_HDF_DIR\n$OUTPUT_DIR\n$GENERIC_CFG\n"
        ;;
    L1BHDF-TO-L1B)
        INPUT="$REV\n$OUTPUT_DIR\n$CFG\n"
        ;;
    L1B-TO-L2A)
        INPUT="$REV\n$OUTPUT_DIR\n$CFG\n"
        ;;
    L2A-FIX-QS-COMPOSITES)
        INPUT="$REV\n$OUTPUT_DIR\n$CFG\n"
        ;;
    L2A-TO-L2B)
        INPUT="$REV\n$OUTPUT_DIR\n$CFG\n"
        ;;
    L2B-MEDIAN-FILTER)
        INPUT="$REV\n$OUTPUT_DIR\n$CFG\n$ARGS\n"
        ;;
    L2B-TO-NETCDF)
        INPUT="$REV\n$OUTPUT_DIR\n$CFG\n$ARGS\n"
        ;;
    CLEAN)
        INPUT="$REV\n$L1B_HDF_DIR\n$L2B_HDF_DIR\n$OUTPUT_DIR\n$CFG\n$ARGS\n"
        ;;
    esac

    echo
    echo "Sending: '$INPUT' to '$CMD'"
    echo

    echo -e "$INPUT" | qs_reproc_process_command "$CMD"
    RETVAL=$?

    return $RETVAL
}

#######################################################################
# Given a CFG file and REV, process it completely.
function qs_reproc_by_rev () {
    CFG_FILE="$1"
    REV="$2"

    source "$CFG_FILE"
    if [[ -z "$LOG_DIR" ]]; then
        LOG_DIR=./
    fi

    qs_reproc_execute_automated_cmd "$REV" "STAGE"; EXIT=$?
    if [[ $EXIT -ne 0 ]]; then
        return $EXIT
    fi
    qs_reproc_execute_automated_cmd "$REV" "GENERATE"; EXIT=$?
    if [[ $EXIT -ne 0 ]]; then
        return $EXIT
    fi
    qs_reproc_execute_automated_cmd "$REV" "L1BHDF-TO-L1B"; EXIT=$?
    if [[ $EXIT -ne 0 ]]; then
        return $EXIT
    fi
    qs_reproc_execute_automated_cmd "$REV" "L1B-TO-L2A"; EXIT=$?
    if [[ $EXIT -ne 0 ]]; then
        return $EXIT
    fi
    qs_reproc_execute_automated_cmd "$REV" "L2A-FIX-QS-COMPOSITES"; EXIT=$?
    if [[ $EXIT -ne 0 ]]; then
        return $EXIT
    fi
    qs_reproc_execute_automated_cmd "$REV" "L2A-TO-L2B"; EXIT=$?
    if [[ $EXIT -ne 0 ]]; then
        return $EXIT
    fi
    qs_reproc_execute_automated_cmd "$REV" "L2B-MEDIAN-FILTER GS"; EXIT=$?
    if [[ $EXIT -ne 0 ]]; then
        return $EXIT
    fi
    qs_reproc_execute_automated_cmd "$REV" "L2B-MEDIAN-FILTER S3"; EXIT=$?
    if [[ $EXIT -ne 0 ]]; then
        return $EXIT
    fi
    qs_reproc_execute_automated_cmd "$REV" "L2B-TO-NETCDF GS"; EXIT=$?
    if [[ $EXIT -ne 0 ]]; then
        return $EXIT
    fi
    qs_reproc_execute_automated_cmd "$REV" "L2B-TO-NETCDF S3"; EXIT=$?
    if [[ $EXIT -ne 0 ]]; then
        return $EXIT
    fi
}

#######################################################################
# Scheduler to chose which command to execute next
# Requires that CMD_FILE be locked before being called
function qs_reproc_schedule () {
    CMDS=`cut -f 1-2 "$CMD_FILE"`
    
    # You can duplicate this idiom to define lower priority commands

    # TOP PRIORITY: Look for L2A-TO-L2B
    PRIORITY_CMD=`echo "$CMDS" | grep "L2A-TO-L2B" -`
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
    CMD=`echo "$CMDS" | grep -v "L2A-TO-L2B" - | head -n 1`
    echo "$CMD"
}

#######################################################################
# Given a CFG file, execute the commands in the command file.
function qs_reproc_by_file () {
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

    if ! [[ -d "$LOG_DIR" ]]; then
        mkdir -p "$LOG_DIR" > /dev/null 2>&1
    fi
    if ! [[ -d "$LOCKDIR" ]]; then
        mkdir -p "$LOCKDIR" > /dev/null 2>&1
    fi

    lock "$CMD_LOCK" "$UNIQ"
    # Let ALL be a synonym for all actual processing
    sed -i -e 's/ALL/STAGE	GENERATE	L1BHDF-TO-L1B	L1B-TO-L2A	L2A-FIX-QS-COMPOSITES	L2A-TO-L2B	L2B-MEDIAN-FILTER GS	L2B-MEDIAN-FILTER S3	L2B-TO-NETCDF GS	L2B-TO-NETCDF S3/' "$CMD_FILE"
    unlock "$CMD_LOCK"

    while [[ -s "$CMD_FILE" ]]; do
    (
        # Yes, we really do want to spawn a new subshell for each processing run.
        # This allows the logfiles to be overwritten after each command.

        # Since this is the ONLY place we nest mutexes, there's no
        # threat of deadlock
        lock "$CMD_LOCK" "$UNIQ"
        lock "$PROC_LOCK" "$UNIQ"

        # Give CMD_FILE to the scheduler.  It will chose the next command.
        # CMD is: "Revision\tCommand"
        CMD=`qs_reproc_schedule "$CMD_FILE"`
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
        CMD_LINE=`grep "^$CMD" "$CMD_FILE" | head -n 1`
        REV=`echo "$CMD_LINE" | cut -f 1`
        CMD=`echo "$CMD_LINE" | cut -f 2`
        NEXT_CMD_LINE=`echo "$CMD_LINE" | cut -f 1,3-`

        START=`date`

        # Log to the PROC_FILE
        echo "$REV	$CMD	$UNIQ	$START" >> "$PROC_FILE"
        unlock "$PROC_LOCK"

        # Delete the comamnd from the file so no one else tries 
        # to execute it
        sed -i "/$CMD_LINE/d" "$CMD_FILE"       

        unlock "$CMD_LOCK"

        # Execute the command
        echo
        echo "Executing '$CMD' for '$REV'."
        echo "The following command is '$NEXT_CMD_LINE'."
        echo

        qs_reproc_execute_automated_cmd "$REV" "$CMD"
        RETVAL=$?
        END=`date`

        # Calculate the runtime using `date`
        RUNTIME=$((`date --date="$END" +%s` - `date --date="$START" +%s`))

        if [[ $RETVAL -eq 0 ]]; then
            # If the command returns successfully, then log it to the 
            # SUCCESS_FILE and write the subsequent command (if available
            # to CMD_FILE
  
            lock "$SUCCESS_LOCK" "$UNIQ"
            echo "$REV	$CMD	$UNIQ	$START	$END	$RUNTIME" >> "$SUCCESS_FILE"
            unlock "$SUCCESS_LOCK"
    
            lock "$CMD_LOCK" "$UNIQ"
            if [[ "$NEXT_CMD_LINE" != "$REV" ]]; then
                # If there are more commands for this REV, we need to 
                # reinsert them into the command file
                if [[ -s "$CMD_FILE" ]]; then
                    # Use sed to prepend the command
                    sed -i -e "1i$NEXT_CMD_LINE" "$CMD_FILE"
                else
                    # sed can't prepend to an empty file
                    echo "$NEXT_CMD_LINE" >> "$CMD_FILE"
                fi
            fi
            unlock "$CMD_LOCK"
        else
            # If the command returns unsuccessfully, log it to the
            # ERR_FILE (along with any subsequent commands) and
            # copy the process log file to a persistent file.

            NEXT_CMD_LINE=`echo $NEXT_CMD_LINE | cut -f 2-`

            lock "$ERR_LOCK" "$UNIQ"
            echo "$REV	$CMD	$NEXT_CMD_LINE	$UNIQ	$START	$END	$RUNTIME" >> "$ERR_FILE"
            cp "$LOG_FILE" "$LOG_DIR/$REV-$CMD-$UNIQ-error.log"
            unlock "$ERR_LOCK"
        fi       

        # Remove the line from the processing file
        lock "$PROC_LOCK" "$UNIQ"
        sed -i "/$REV	$CMD.*/d" "$PROC_FILE"
        unlock "$PROC_LOCK"

    ) 2>&1 | sed -e "s/^/`date +"%F %T"` /" | tee "$LOG_FILE" | sed -e "s/^/[$UNIQ] /"
    #       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    # Write the date/time of each line into the files.
    # Timestamps are useful.
    #
    #                                         ^^^^^^^^^^^^^^
    # Write the results of this run into a per-process logfile
    # in addition to dumping to the command line.  This log
    # is rewritten on each iteration of the loop.
    #
    #                                                          ^^^^^^^^^^^^^^^^^^^^^ 
    # Prepend an ID to the output of the processes.
    # This makes reading the outputs and comparing the results
    # in the files MUCH easier.
    done 
}

#######################################################################
# Print an interactive prompt and do the user's processing
function qs_reproc_interactive () {

    while true; do
        qs_reproc_prompt 
        read COMMAND
        echo
        COMMAND=`qs_reproc_get_command "$COMMAND"`
        qs_reproc_process_command "$COMMAND"
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
        qs_reproc_by_file "$CFG_FILE"
    else
        # If there are two, the first is a CFG and the second is a REV.  
        # Use the variables specified in the CFG and reproc that REV.
        REV="$2"
        qs_reproc_by_rev "$CFG_FILE" "$REV"
    fi
else
    # If there are no parmeters, run interactively
    qs_reproc_interactive
fi
