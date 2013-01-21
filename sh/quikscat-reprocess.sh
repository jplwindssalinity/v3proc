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
    echo "      Converts the L1B HDF (for QuikSCAT or OSCAT2) file to "
    echo "      simulation L1B format"
    echo "4  L1B-TO-L2A"
    echo "      Performs L1B to L2A processing"
    echo "5  L2A-FIX-QS-COMPOSITES"
    echo "6  L2A-TO-L2B"
    echo "      Performs L2A to L2B processing"
    echo "7  L2B-MEDIAN-FILTER"
    echo "      Applies the L2B median filtering"
    echo "8  L2B-TO-NETCDF"
    echo "      Converts the L2B file to NetCDF format"
    echo "9  LINK"
    echo "      Symlink files in a directory structure"
    echo "10 TDV"
    echo "      Run 2D Var"
    echo "11 MAKE-ARRAYS"
    echo "      Extract arrays from dataset"
    echo "12 L2B-HDF-TO-NETCDF"
    echo "      Convert L2B HDF files to NetCDF"
    echo "13 FIXUP"
    echo "      Apply Matlab Fixups to NetCDF file"
    echo "14 L2C"
    echo "      Apply L2C conversions"
    echo "15 BUILD-MD5S"
    echo "      Create md5 checksums"
    echo "16 CLEAN"
    echo "      Removed unnecessary data files"
    echo "================================================================"
    echo ""
    echo -n "> "
}

#######################################################################
# Convert command numbers to command names
function qs_reproc_get_command() {
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
    5)  echo "L2A-FIX-QS-COMPOSITES"
        ;;
    6)  echo "L2A-TO-L2B"
        ;;
    7)  echo "L2B-MEDIAN-FILTER"
        ;;
    8)  echo "L2B-TO-NETCDF"
        ;;
    9)  echo "LINK"
        ;;
    10) echo "TDV"
        ;;
    11) echo "MAKE-ARRAYS"
        ;;
    12) echo "L2B-HDF-TO-NETCDF"
        ;;
    13) echo "FIXUP"
        ;;
    14) echo "L2C"
        ;;
    15) echo "BUILD-MD5S"
        ;;
    16) echo "CLEAN"
        ;;
    *)  echo "$COMMAND"
        ;;
    
    esac
)
}

#######################################################################
# Download and extract existing L1B and L2B HDF files
function qs_reproc_stage () {
(
    echo -n "Rev: "
    read REV ; tty > /dev/null 2>&1; 
        [[ $? -eq 0 ]] || echo "$REV"
    echo -n "Rev log: "
    read REVLOG ; tty > /dev/null 2>&1; 
        [[ $? -eq 0 ]] || echo "$REVLOG"
    echo -n "L1B HDF Directory: "
    read DIR_QSL1B_HDF; tty > /dev/null 2>&1; 
        [[ $? -eq 0 ]] || echo "$DIR_QSL1B_HDF"
    echo -n "L2B HDF Directory: "
    read DIR_QSL2B_HDF; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$DIR_QSL2B_HDF"

    # We need a temporary directory to use as a target for wget.
    # This keeps .listing files from getting smashed
    TMP="./tmp"
    mkdir "$TMP" > /dev/null 2>&1
    TDIR=`mktemp -d "$TMP/XXXXXXXXXX"`

    L1BURL="ftp://qL1B:data4me@podaac-old/data/L1B"
    #L2BURL="ftp://podaac-ftp.jpl.nasa.gov/allData/quikscat/L2B/" # 25km L2B
    L2BURL="ftp://podaac-ftp.jpl.nasa.gov/allData/quikscat/L2B12/" # 12.5km L2B

    ID=`awk "/^$REV/" "$REVLOG"`

    # Some REVs cross day boundaries.  I'm not sure which day the rev
    # file will land in, so just check in both days if this one does.

    DATE1=`echo $ID | sed -e 's/  */ /g' | cut -f 2 -d \  `

    YEAR1=`echo $DATE1 | sed -e 's/^\(.*\)-.*/\1/'`
    DAY1=`echo $DATE1 | sed -e 's/.*-0*\(.*\)T.*/\1/'`

    # Play some games with `date` to get a "properly formatted" 
    # version of DATE1, then add one day to it to find the following
    # day
    DATE1=$(date --date="$YEAR1-01-01 +$(($DAY1 - 1))days")
    DATE2=$(date --date="$DATE1 +1days")
    
    YEAR2=$(date --date="$DATE2" +%Y)
    DAY2=$(date --date="$DATE2" +%j)

    # Prepend zeros as necessary
    DAY1=$(date --date="$DATE1" +%j)

    # See if there are any existing L1B HDF files associated to this rev
    ls "$DIR_QSL1B_HDF"/*"S1B$REV"* > /dev/null 2>&1

    if [[ $? -ne 0 ]]; then
        # Gotta download the data files
  
        # L1B HDF File
        wget -nH -N --cut-dirs=4 -P "$TDIR" \
            "$L1BURL/$YEAR1/$DAY1/*S1B$REV*"
        ls "$TDIR"/*"S1B$REV"* > /dev/null 2>&1

        if [[ $? -ne 0 ]]; then
            # Couldn't find the file in DATE1, try DATE2
            wget -nH -N --cut-dirs=4 -P "$TDIR" \
                "$L1BURL/$YEAR2/$DAY2/*S1B$REV*"
        fi

        ls "$TDIR"/*"S1B$REV"* > /dev/null 2>&1
        if [[ $? -ne 0 ]]; then
            echo "ERROR: REV L1B HDF file not found"
            return 1
        fi

        gunzip -f "$TDIR"/*"S1B$REV"*.gz
        mkdir "$DIR_QSL1B_HDF" > /dev/null 2>&1
        mv "$TDIR"/*"S1B$REV"* "$DIR_QSL1B_HDF/"
    fi


    # See if there are any existing L2B HDF files associated to this rev
    ls "$DIR_QSL2B_HDF"/*"S2B$REV"* > /dev/null 2>&1
    if [[ $? -ne 0 ]]; then
        # Gotta download the data files
  
        # L2B HDF File
        wget -nH -N --cut-dirs=4 -P "$TDIR" \
            "$L2BURL/$YEAR1/$DAY1/*S2B$REV*"

        ls "$TDIR"/*"S2B$REV"* > /dev/null 2>&1
        if [[ $? -ne 0 ]]; then
            # Couldn't find the file in DATE1, try DATE2
            wget -nH -N --cut-dirs=4 -P "$TDIR" \
                "$L2BURL/$YEAR2/$DAY2/*S2B$REV*"
        fi

        ls "$TDIR"/*"S2B$REV"* > /dev/null 2>&1
        if [[ $? -ne 0 ]]; then
            echo "ERROR: REV L2B HDF file not found"
            return 1
        fi

        gunzip -f "$TDIR"/*"S2B$REV"*.gz
        mkdir "$DIR_QSL2B_HDF" > /dev/null 2>&1
        mv "$TDIR"/*"S2B$REV"* "$DIR_QSL2B_HDF/"
    fi

    rm -rf "$TDIR"
    return 0
)
}

#######################################################################
# Generate the rev directory structure.
# Based on code from A. Fore
function qs_reproc_generate_directory_structure () {
(
    # Read process-specific variables
    echo -n "Rev: "
    read REV; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$REV"
    echo -n "L1B HDF Directory: "
    read DIR_QSL1B_HDF; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$DIR_QSL1B_HDF"
    echo -n "L2B HDF Directory: "
    read DIR_QSL2B_HDF; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$DIR_QSL2B_HDF"
    echo -n "Output Directory:  "
    read DIR_OUT_BASE; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$DIR_OUT_BASE"
    echo -n "Template config file: "
    read TEMPLATE_FILE; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$TEMPLATE_FILE"
    
    QS_MATLAB_INC_DIR="./mat"
    
    L1B_HDF_FILE=`ls $DIR_QSL1B_HDF/QS_S1B$REV* | tail -n 1`
    L2B_HDF_FILE=`ls $DIR_QSL2B_HDF/QS_S2B$REV*.CP12 | tail -n 1`
    
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
    NCEP_FILENAME_STR='SNWP1'$CLOSEST_NWP_STR
#    NCEP_FILENAME_STR='SNWP3'$CLOSEST_NWP_STR
    
    ICE_FILENAME_STR='NRT_ICEM'${CLOSEST_NWP_STR:0:7}
    
    CONFIG_FILE=$SIM_DIR/'QS.rdf'
    
    sed -e "s:NUDGE_WINDFIELD_FILE        = DUMMY_FILENAME:NUDGE_WINDFIELD_FILE        = ../../ECMWF/$NCEP_FILENAME_STR:" \
        -e "s:L1B_HDF_FILE                = DUMMY_FILENAME:L1B_HDF_FILE                = ../../$L1B_HDF_FILE:" \
        -e "s:L2B_HDF_FILE                = DUMMY_FILENAME:L2B_HDF_FILE                = ../../$L2B_HDF_FILE:" \
        -e "s:QS_ICEMAP_FILE              = DUMMY_FILENAME:QS_ICEMAP_FILE              = ../../dat/ice/$ICE_FILENAME_STR:" \
        -e "s:ATTEN_MAP_SEC_YEAR          = DUMMY:ATTEN_MAP_SEC_YEAR          = $SEC_YEAR:" \
                $TEMPLATE_FILE > $CONFIG_FILE
    RETVAL=$(($RETVAL || $?))

    return $RETVAL
)
}

#######################################################################
# Convert L1BHDF files to local-format L1B files
# Based on code from A. Fore
function qs_reproc_l1bhdf_to_l1b () {
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
    echo -n "Type (QSCAT|OSCAT2): "
    read TYPE; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$TYPE"

    cd "$BASEDIR/$REV"
    
    DIR_QSL1B_HDF=`grep L1B_HDF_FILE "$CONFIG_FILE" | cut -d \/ -f 3`
    case "$TYPE" in
    QSCAT) 
        l1b_hdf_to_l1b_fast $CONFIG_FILE
        RETVAL=$?
        ;; 
    OSCAT2) 
        L1B_ISRO_FILE=`ls ../../$DIR_QSL1B_HDF/S1L1B*$REV*_*`
        sed -i -e "s:\(L1B_HDF_FILE *=.*\)[^\\]*:\1$L1B_ISRO_HDF_FILE:" \
            $CONFIG_FILE
        l1b_isro_to_l1b $CONFIG_FILE
        RETVAL=$?
        ;; 
    *)
        echo "Unknown TYPE"
        RETVAL=1
        ;;
    esac
    


    return $RETVAL
)
}

#######################################################################
# Convert from local-format L1B to L2A files
# Based on code from A. Fore
function qs_reproc_l1b_to_l2a () {
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
function qs_reproc_l2a_fix_qs_composites () {
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
    
    l2a_fix_QS_composites -c "$CONFIG_FILE" -o l2a_flagged.dat -kp
    RETVAL=$?
    mv "$L2A_FNAME" "$L2A_FNAME.orig"
    mv l2a_flagged.dat "$L2A_FNAME"

    return $RETVAL
)
}

#######################################################################
# Convert from local-format L2A to L2B files
# Based on code from A. Fore
function qs_reproc_l2a_to_l2b () {
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
    
    l2a_to_l2b -t nn_train.dat "$CONFIG_FILE"
    RETVAL=$?

    return $RETVAL
)
}
    

#######################################################################
# Do L2B median filtering
# Based on code from A. Fore
function qs_reproc_l2b_median_filter () {
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
    echo -n "Type (GS|S3|TDV): "
    read TYPE; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$TYPE"
    echo -n "Nudge Windfield (NCEP|ECMWF): "
    read WINDFIELD; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$WINDFIELD"

    cd "$BASEDIR/$REV"
    L2B_HDF_FNAME=`awk '/L2B_HDF_FILE/ {print $3}' $CONFIG_FILE`

    case "$TYPE" in
    S3) 
        MEDFILT_CONFIG=tmp1.rdf
        OUTFILE=l2b_flagged_S3.dat
        #OTHER="-nudgeHDF $L2B_HDF_FNAME"
        OTHER="-flagsHDF $L2B_HDF_FNAME"
        sed -e 's:MEDIAN_FILTER_MAX_PASSES    = 0:MEDIAN_FILTER_MAX_PASSES    = 200:' \
            $CONFIG_FILE > "$MEDFILT_CONFIG"
        RETVAL=$?
        ;; 
    GS) 
        MEDFILT_CONFIG=tmp2.rdf
        OUTFILE=l2b_flagged_GS.dat
        #OTHER="-nudgeHDF $L2B_HDF_FNAME"
        OTHER="-flagsHDF $L2B_HDF_FNAME"
        sed -e 's:MEDIAN_FILTER_MAX_PASSES    = 0:MEDIAN_FILTER_MAX_PASSES    = 200:' \
            -e 's:WIND_RETRIEVAL_METHOD       = S3:WIND_RETRIEVAL_METHOD       = GS:' \
                $CONFIG_FILE > "$MEDFILT_CONFIG"
        RETVAL=$?
        ;; 
    TDV)
        MEDFILT_CONFIG=tmp3.rdf
        OUTFILE=l2b_flagged_TDV.dat
        OTHER="-flagsHDF $L2B_HDF_FNAME"
        sed -e 's:MEDIAN_FILTER_MAX_PASSES    = 0:MEDIAN_FILTER_MAX_PASSES    = 200:' \
            -e 's:\(NUDGE_WINDFIELD_TYPE *= *\).*:\1NCEP\nQSCP12_ECMWF_ARRAY_NUDGING  = 1:' \
            -e 's:\(NUDGE_WINDFIELD_FILE *= *\).*:\1./nudge_tdv.dat:' \
                $CONFIG_FILE > "$MEDFILT_CONFIG"
        RETVAL=$?
        ;; 
    *)
        echo "Unknown TYPE"
        RETVAL=1
        ;;
    esac
    if [[ "$WINDFIELD" = "ECMWF" ]]; then
        E2BFILE="../../E2B12/E2B_$REV.cp12.dat"
        sed -i -e "s:^NUDGE_WINDFIELD_FILE.*:NUDGE_WINDFIELD_FILE      = $E2BFILE\nQSCP12_ECMWF_ARRAY_NUDGING  = 1:" \
            "$MEDFILT_CONFIG"
        RETVAL=$(($RETVAL || $?))
        OTHER=`echo $OTHER | sed -e 's/nudgeHDF/flagsHDF/'`
    fi
    if [[ $RETVAL -ne 0 ]]; then
        return $RETVAL
    fi
    
    echo l2b_medianfilter -c "$MEDFILT_CONFIG" -o "$OUTFILE" $OTHER
    l2b_medianfilter -c "$MEDFILT_CONFIG" -o "$OUTFILE" $OTHER
    RETVAL=$?

    return $RETVAL
)
}

#######################################################################
# Convert from local-format L2B to NetCDF files
function qs_reproc_l2b_to_netcdf () {
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
    echo -n "Type (GS|S3|TDV): "
    read TYPE; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$TYPE"

    cd "$BASEDIR/$REV"
    L1B_HDF_FNAME=`awk '/L1B_HDF_FILE/ {print $3}' $CONFIG_FILE`
    L2B_HDF_FNAME=`awk '/L2B_HDF_FILE/ {print $3}' $CONFIG_FILE`

    case $TYPE in
    S3)
        INFILE=l2b_flagged_S3.dat
    	OUTFILE=`basename ${L2B_HDF_FNAME/%\.CP12/_S3.L2BC.nc}`
        ;;
    GS)
        INFILE=l2b_flagged_GS.dat
    	OUTFILE=`basename ${L2B_HDF_FNAME/%\.CP12/_GS.L2BC.nc}`
        ;;
    TDV)
        INFILE=l2b_flagged_TDV.dat
    	OUTFILE=`basename ${L2B_HDF_FNAME/%\.CP12/_TDV.L2BC.nc}`
        ;;
    *)
        echo "Unknown TYPE"
        ;;
    esac

    l2b_to_netcdf --l2bhdf "$L2B_HDF_FNAME" --l1bhdf "$L1B_HDF_FNAME" --l2bc "$OUTFILE"  --l2b "$INFILE"
    RETVAL=$?

    return $RETVAL
)
}

#######################################################################
# Link a files from a directory structure
function qs_reproc_link() {
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
# Perform 2D Var
function qs_reproc_tdv() {
(
    echo -n "Rev: "
    read REV; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$REV"
    echo -n "Base directory: "
    read BASEDIR; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$BASEDIR"
    echo -n "Input File: "
    read INFILE; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$INFILE"
    echo -n "Config: "
    read CONFIG_FILE; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$CONFIG_FILE"

    QS_MATLAB_INC_DIR="./mat"
    NCFILE="l2b_flagged_GS_ext.nc"
    DATFILE="nudge_tdv.dat"

    cd "$BASEDIR/$REV"
    L1B_HDF_FNAME=`awk '/L1B_HDF_FILE/ {print $3}' $CONFIG_FILE`
    L2B_HDF_FNAME=`awk '/L2B_HDF_FILE/ {print $3}' $CONFIG_FILE`

    echo l2b_to_netcdf --l2bhdf "$L2B_HDF_FNAME" --l1bhdf "$L1B_HDF_FNAME" \
        --l2bc "$NCFILE"  --l2b "$INFILE" --extended
    l2b_to_netcdf --l2bhdf "$L2B_HDF_FNAME" --l1bhdf "$L1B_HDF_FNAME" \
        --l2bc "$NCFILE"  --l2b "$INFILE" --extended
    RETVAL=$?

    cd "../../"
    # Extract orbit start and end-time
    (
        echo "addpath('$QS_MATLAB_INC_DIR');"
        echo "addpath('$QS_MATLAB_INC_DIR/minFunc');"
        echo "run_tdv_qs_l2bnc('$BASEDIR/$REV/$NCFILE', '$BASEDIR/$REV/$DATFILE');"
    ) | matlab -nodisplay -nojvm
    RETVAL=$(($RETVAL || $?))

    return $RETVAL
)
}

#######################################################################
# Extract data into arrays
function qs_reproc_make_arrays() {
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
    echo -n "Type (L2B): "
    read TYPE; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$TYPE"
    

    QS_MATLAB_INC_DIR="./mat"
    INFILE="l2b.dat"
    NCFILE="l2b.nc"
    DATFILE="l2b-tdv-nudge.dat"

    cd "$BASEDIR/$REV"
    L1B_HDF_FNAME=`awk '/L1B_HDF_FILE/ {print $3}' $CONFIG_FILE`
    L2B_HDF_FNAME=`awk '/L2B_HDF_FILE/ {print $3}' $CONFIG_FILE`

    l2b_to_netcdf --l2bhdf "$L2B_HDF_FNAME" --l1bhdf "$L1B_HDF_FNAME" \
        --l2bc "$NCFILE"  --l2b "$INFILE" --extended
    RETVAL=$?

    cd "../../"
    # Extract orbit start and end-time
    (
        echo "addpath('$QS_MATLAB_INC_DIR');"
        echo "addpath('$QS_MATLAB_INC_DIR/minFunc');"
        echo "run_tdv_qs_l2bnc('$BASEDIR/$REV/$NCFILE', '$BASEDIR/$REV/$DATFILE');"
    ) | matlab -nodisplay -nojvm
    RETVAL=$(($RETVAL || $?))

    return $RETVAL
)
}

#######################################################################
# Convert from L2B HDF files to NetCDF
function qs_l2b_hdf_to_netcdf () {
(
    echo -n "Rev: "
    read REV; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$REV"
    echo -n "L1B HDF Directory: "
    read DIR_QSL1B_HDF; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$DIR_QSL1B_HDF"
    echo -n "L2B HDF Directory: "
    read DIR_QSL2B_HDF; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$DIR_QSL2B_HDF"
    echo -n "Output Directory:  "
    read DIR_OUT_BASE; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$DIR_OUT_BASE"

    L1B=`ls "$DIR_QSL1B_HDF/"*"$REV"*`
    L2B=`ls "$DIR_QSL2B_HDF/"*"$REV"*`
    L2BC="$DIR_OUT_BASE/$REV/`basename \"$L2B\"`.nc"
    
    mkdir -p `dirname "$L2BC"` > /dev/null 2>&1

    l2b_hdf_to_netcdf --l2bhdf="$L2B" --l1bhdf="$L1B" --l2bc="$L2BC"
    RETVAL=$?
    return $RETVAL
)
}

#######################################################################
# Apply NetCDF fixup
function qs_fixup () {
(
    echo -n "Rev: "
    read REV; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$REV"
    read E2B12_DIR; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$E2B12_DIR"
    read BASEDIR; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$BASEDIR"

    QS_MATLAB_INC_DIR="$PWD/../QScatSim/mat"

    NC_SRC=`ls $BASEDIR/$REV/*.L2BC.nc`

    cd "$BASEDIR/$REV"
    # Extract orbit start and end-time
    (
        echo "addpath('$QS_MATLAB_INC_DIR');"
        echo "qs_convert_netcdf('$NC_SRC', '$E2B12_DIR');"
    ) | matlab -nodisplay -nojvm

    RETVAL=$?
    return $RETVAL
)
}

#######################################################################
# Do L2C processing
function qs_l2c () {
(
    echo -n "Rev: "
    read REV; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$REV"
    read BASEDIR; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$BASEDIR"

    cd "$BASEDIR/$REV"

    CURL_DIV_CFG="makeCurlDivergence.rdf"
    FILTERED_WINDS_CFG="makeFilteredWinds.rdf"

    L2BC_FILE="`ls *.L2BC.nc`"
    L2C_FILE="`ls qs_l2c*.nc`"

cat <<EOF > "$CURL_DIV_CFG"
Usage: makeCurlDivergence commandFile
filtered l2bc file                              = $L2C_FILE ! input/output netcdf file
estimation window size                          = 3 ! size of the estimation window for curl divergence (odd > 1)
minimum number of good points                   = 6 ! minimum number of good points needed for fitting
vector field type                               = wind ! wind or stress
output flag bit position                        = 14 ! position of the bad filtering flag bit 
number of flag fields                           = 1 ! number of flags to be checked for good data
flag 1 bit position                             = 15 ! flag 1 bit index to check starting from zero
flag 1 bit value                                = 0 ! flag 1 bit value for good data
EOF

cat <<EOF > "$FILTERED_WINDS_CFG"
unfiltered l2bc file                            = $L2BC_FILE ! input netcdf file
filtered l2bc file                              = $L2C_FILE ! output netcdf file
smoothing window size                           = 3 ! size of the smoothing window
minimum number of good points                   = 6 ! minimum number of good points needed for smoothing
output flag bit position                        = 15 ! position of the bad filtering flag bit 
number of flag fields                           = 2 ! number of flags to be checked for good data
flag 1 bit position                             = 9 ! flag 1 bit index to check starting from zero
flag 1 bit value                                = 0 ! flag 1 bit value for good data
flag 2 bit position                             = 13 ! flag 2 bit index to check starting from zero
flag 2 bit value                                = 0 ! flag 2 bit value for good data
EOF

    makeFilteredWinds "$FILTERED_WINDS_CFG"
    RETVAL=$?

    makeCurlDivergence "$CURL_DIV_CFG"
    RETVAL=$(($RETVAL || $?))

    return $RETVAL
)
}

#######################################################################
# Create md5 checksums
function qs_build_md5s () {
(
    echo -n "Rev: "
    read REV; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$REV"
    read BASEDIR; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$BASEDIR"

    cd "$BASEDIR/$REV"

    RETVAL=0
    for FILE in `ls qs_l2[bc]*.nc`; do
        md5sum "$FILE" > "$FILE.md5"
        RETVAL=$(($RETVAL || $?))
    done

    return $RETVAL
)
}

#######################################################################
# Clean the directories
function qs_reproc_clean () {
(
    echo -n "Rev: "
    read REV; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$REV"
    echo -n "L1B HDF Directory: "
    read DIR_QSL1B_HDF; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$DIR_QSL1B_HDF"
    echo -n "L2B HDF Directory: "
    read DIR_QSL2B_HDF; tty > /dev/null 2>&1;
        [[ $? -eq 0 ]] || echo "$DIR_QSL2B_HDF"
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
    L2B_HDF_FNAME=`awk '/L2B_HDF_FILE/ { print $3 }' "$CONFIG_FILE"`

    # Spoof switch with fallthrough
    if [[ $LEVEL -le 3 ]]; then
        rm -f \
            "nn_train.dat" \
            "$L1B_HDF_FNAME" \
            "$L2B_HDF_FNAME" \
            "$L1B_FNAME" \
            "$L2A_FNAME.orig" \
            l1bhdf_to_l1b_tmpfile \
            ephem.dat \
            "l2b_flagged_S3.dat" \
            "l2b_flagged_GS.dat" \
            "l2b_flagged_TDV.dat" \
            "l2b_flagged_GS_ext.nc" \
            "$L2A_FNAME"
    fi
    if [[ $LEVEL -le 2 ]]; then
        rm -f \
            "tmp1.rdf" \
            "tmp2.rdf" \
            "tmp3.rdf" \
            "nudge_tdv.dat" \
            "*_GS.L2BC.nc" \
            "*_TDV.L2BC.nc"
    fi
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
    LINK)
        qs_reproc_link
        RETVAL=$?
        ;;
    TDV)
        qs_reproc_tdv
        RETVAL=$?
        ;;
#    MAKE-ARRAYS)
#        qs_reproc_make_arrays
#        RETVAL=$?
#        ;;
    L2B-HDF-TO-NETCDF)
        qs_l2b_hdf_to_netcdf 
        RETVAL=$?
        ;;
    FIXUP)
        qs_fixup
        RETVAL=$?
        ;;
    L2C)
        qs_l2c
        RETVAL=$?
        ;;
    BUILD-MD5S)
        qs_build_md5s
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
(
    REV="$1"
    CMD=`echo "$2" | cut -f 1 -d \ `
    ARGS=`echo "$2" | cut -f 2- -d \ `
    RETVAL=1

    case "$CMD" in 
    STAGE)
        INPUT="$REV\n$REVLOG\n$L1B_HDF_DIR\n$L2B_HDF_DIR\n"
        ;;
    GENERATE)
        INPUT="$REV\n$L1B_HDF_DIR\n$L2B_HDF_DIR\n$OUTPUT_DIR\n$GENERIC_CFG\n"
        ;;
    L1BHDF-TO-L1B)
        INPUT="$REV\n$OUTPUT_DIR\n$CFG\n$ARGS\n"
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
        ARGS=`echo "$ARGS" | tr ' ' '\n'`
        INPUT="$REV\n$OUTPUT_DIR\n$CFG\n$ARGS\n"
        ;;
    L2B-TO-NETCDF)
        INPUT="$REV\n$OUTPUT_DIR\n$CFG\n$ARGS\n"
        ;;
    LINK)
        ARGS=`echo "$ARGS" | tr ' ' '\n'`
        INPUT="$REV\n$OUTPUT_DIR\n$ARGS\n"
        ;;
    TDV)
        ARGS="l2b_flagged_GS.dat"
        INPUT="$REV\n$OUTPUT_DIR\n$ARGS\n$CFG\n"
        ;;
    L2B-HDF-TO-NETCDF)
        INPUT="$REV\n$L1B_HDF_DIR\n$L2B_HDF_DIR\n$OUTPUT_DIR\n"
        ;;
    MAKE-ARRAYS)
        INPUT="$REV\n$OUTPUT_DIR\n$CFG\n$ARGS\n"
        ;;
    FIXUP)
        INPUT="$REV\n$E2B12_DIR\n$OUTPUT_DIR\n"
        ;;
    L2C)
        INPUT="$REV\n$OUTPUT_DIR\n"
        ;;
    BUILD-MD5S)
        INPUT="$REV\n$OUTPUT_DIR\n"
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
)
}

#######################################################################
# Given a CFG file and REV, process it completely.
function qs_reproc_by_rev () {
(
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
    qs_reproc_execute_automated_cmd "$REV" "L2B-MEDIAN-FILTER S3"; EXIT=$?
    if [[ $EXIT -ne 0 ]]; then
        return $EXIT
    fi
    qs_reproc_execute_automated_cmd "$REV" "L2B-TO-NETCDF S3"; EXIT=$?
    if [[ $EXIT -ne 0 ]]; then
        return $EXIT
    fi
    qs_reproc_execute_automated_cmd "$REV" "FIXUP"; EXIT=$?
    if [[ $EXIT -ne 0 ]]; then
        return $EXIT
    fi
    qs_reproc_execute_automated_cmd "$REV" "L2C"; EXIT=$?
    if [[ $EXIT -ne 0 ]]; then
        return $EXIT
    fi
    qs_reproc_execute_automated_cmd "$REV" "BUILD-MD5S"; EXIT=$?
    if [[ $EXIT -ne 0 ]]; then
        return $EXIT
    fi

)
}

#######################################################################
# Scheduler to chose which command to execute next
# Requires that CMD_FILE be locked before being called
function qs_reproc_schedule () {
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
    sed -i -e 's/ALL/STAGE	GENERATE	L1BHDF-TO-L1B QSCAT	L1B-TO-L2A L2A-FIX-QS-COMPOSITES	L2A-TO-L2B	L2B-MEDIAN-FILTER S3	L2B-TO-NETCDF S3 FIXUP	L2C	BUILD-MD5S/' "$CMD_FILE"
    unlock "$CMD_LOCK"

    while [[ -s "$CMD_FILE" ]]; do
    (
        # Yes, we really do want to spawn a new subshell for each processing run.
        # This allows the logfiles to be overwritten after each command.

        # Try to deal with funny NFS no-mount bug...
        # /u/potr-r0 is the NFS mount point for the 10 TB drive.
        # Look for it, and if `ls` fails, then the NFS mount must be bad
        ls '/u/potr-r0' > /dev/null 2>&1
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
        CMD_LINE=`grep -m 1 "^$CMD" "$CMD_FILE"`
        REV=`echo "$CMD_LINE" | cut -f 1`
        CMD=`echo "$CMD_LINE" | cut -f 2`
        NEXT_CMD_LINE=`echo "$CMD_LINE" | cut -f 3-`

        START=`date`

        # Log to the PROC_FILE
        echo "$REV	$CMD	$UNIQ	$START" >> "$PROC_FILE"
        unlock "$PROC_LOCK"

        # Delete the comamnd from the file so no one else tries 
        # to execute it
        sed -i "/$CMD_LINE/d" "$CMD_FILE"       

        unlock "$CMD_LOCK"

        # Execute the command
        echo "Executing '$CMD' for '$REV'."
        echo "The following commands are '$NEXT_CMD_LINE'."
        echo

        qs_reproc_execute_automated_cmd "$REV" "$CMD"
        RETVAL=$?
        END=`date`

        # Calculate the runtime using `date`
        RUNTIME=$((`date --date="$END" +%s` - `date --date="$START" +%s`))

        if [[ $RETVAL -eq 0 ]]; then
            # If the command returns successfully, then log it to the 
            # SUCCESS_FILE and write the subsequent command (if available
            # to CMD_FILE)
  
            lock "$SUCCESS_LOCK" "$UNIQ"
            echo "$REV	$CMD	$UNIQ	$START	$END	$RUNTIME" >> \
                "$SUCCESS_FILE"
            unlock "$SUCCESS_LOCK"
    
            lock "$CMD_LOCK" "$UNIQ"
            if [[ -n "$NEXT_CMD_LINE" ]]; then
                # If there are more commands for this REV, we need to 
                # reinsert them into the command file
                if [[ -s "$CMD_FILE" ]]; then
                    # Use sed to prepend the command
                    sed -i -e "1i$REV	$NEXT_CMD_LINE" "$CMD_FILE"
                else
                    # sed can't prepend to an empty file
                    echo "$REV	$NEXT_CMD_LINE" >> "$CMD_FILE"
                fi
            fi
            unlock "$CMD_LOCK"
        else
            # If the command returns unsuccessfully, log it to the
            # ERR_FILE (along with any subsequent commands) and
            # copy the process log file to a persistent file.

            lock "$ERR_LOCK" "$UNIQ"
            echo "$REV	$CMD	$NEXT_CMD_LINE	$UNIQ	$START	$END	$RUNTIME" >> "$ERR_FILE"
            unlock "$ERR_LOCK"
        fi       

        # Remove the line from the processing file
        lock "$PROC_LOCK" "$UNIQ"
        sed -i "/$REV	$CMD.*/d" "$PROC_FILE"
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
