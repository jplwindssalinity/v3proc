#!/bin/bash
######################################################################
#
# ORIGINAL AUTHOR: Thomas Werne
# COMPANY: Jet Propulsion Laboratory
# VERSION: 
#
# File Name:     build-md5s.sh
# Creation Date: 03 May 2011
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

function store-md5 () (
{
    echo "Running md5"
    cd `dirname "$1"`
    FILE=`basename "$1"`
    md5sum "$FILE" > "$FILE.md5"
}
)

find -L -name "qs_l2[bc]*.nc" | while read file; do store-md5 "$file"; done
