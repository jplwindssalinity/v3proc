//=========================================================
// Copyright  (C)1995, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.3   21 Apr 1998 16:39:30   sally
// for L2B
// 
//    Rev 1.2   20 Apr 1998 15:18:08   sally
// change List to EAList
// 
//    Rev 1.1   10 Apr 1998 14:04:12   daffer
//   Copied version 1.1 of ConfigList to EAConfiglist (undoing the svt merge sally had done)
//   Changed ConfigList to EAConfigList throughout
// 
//    Rev 1.0   08 Apr 1998 10:12:10   daffer
// Initial Check-in, copy of v1.1 of ConfigList
// 
//    Rev 1.1   26 Feb 1998 09:54:58   sally
// to pacify GNU C++ compiler
// 
//    Rev 1.0   04 Feb 1998 14:15:04   daffer
// Initial checking
// Revision 1.2  1998/01/30 22:28:59  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

#ifndef CONIGLIST_C_INCLUDED
#define CONIGLIST_C_INCLUDED

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "CommonDefs.h"
#include "EAConfigList.h"

static const char ConfigList_c_rcsid[] = "@(#) $Header$";

EAConfigList::EAConfigList()
:   _status(OK)
{
    _badLine[0] = '\0';
    return;
}//EAConfigList::EAConfigList

EAConfigList::EAConfigList(
const char*     filename)
:   _status(OK)
{
    (void) Read(filename);
    return;
}//EAConfigList::EAConfigList

EAConfigList::~EAConfigList()
{
    //=====================================================
    // delete the list of EAStringPair here, the destructor  
    // of list<EAStringPair> is not enough                   
    //=====================================================
    (void) GetHead();
    EAStringPair* pair=0;
    while ((pair = RemoveCurrent()))
    {
        delete [] pair->keyword;
        delete [] pair->value;
        delete pair;
    }
    return;
}//EAConfigList::~EAConfigList

EAConfigList::StatusE
EAConfigList::Read(
const char*     filename)
{
    //===========================================
    // open a file for reading
    //===========================================
    FILE* ifp = fopen(filename, "r");
    if (ifp == NULL)
    {
        return (_status = ERROR_OPENING_FILE);
    }

    //===========================================
    // read the keyword-value pair in,
    // save the original string in the list
    // RER Added isalnum check so comments are
    //     delimited by any non alpha-numeric char
    //     at the start of a line.  
    // JNH Reads line by line to avoid reading
    //     across line boundaries
    //===========================================
    char line[BIG_SIZE];
    char keyword[BIG_SIZE], value[BIG_SIZE];
    int numRead=0;
    for (;;)
    {
        char* ptr = fgets(line, BIG_SIZE, ifp);
        if (ptr != line)
        {
            if (feof(ifp))  // EOF
                break;
            else            // error
                return (_status = ERROR_READING_CONFIG_ENTRY);
        }
        numRead = sscanf(line, " %s %s", keyword, value);
        switch (numRead)
        {
        case 1: 
            if (isalnum(keyword[0]))
            {   // looks like a valid keyword, but there is no value
                strncpy(_badLine, line, strlen(line) - 1);
                return (_status = ERROR_READING_CONFIG_ENTRY);
            }
            break;
        case 2:
            if (isalnum(keyword[0])) {
                EAStringPair* pair = new EAStringPair();
                pair->keyword = new char[strlen(keyword) + 1];
                (void) strcpy(pair->keyword, keyword);
                pair->value = new char[strlen(value) + 1];
                (void) strcpy(pair->value, value);
                Append(pair);
            }
        }
    }

    // don't need the file anymore
    fclose(ifp);

    return (_status = OK);
}//EAConfigList::Read

EAConfigList::StatusE
EAConfigList::Write(
const char*     filename)
{
    //===========================================
    // open the file for writing, write the whole
    // list into the file.                       
    //===========================================
    FILE* ofp = fopen(filename, "w");
    if (ofp == NULL)
    {
        return (_status = ERROR_OPENING_FILE);
    }
    int numChars = 0;
    for (EAStringPair* pair = GetHead(); pair != NULL; pair = GetNext())
    {
        if ((numChars = fprintf(ofp, "%s %s\n",
                                pair->keyword, pair->value)) < 0)
            return (_status = ERROR_WRITING_CONFIG_FILE);
    }
    (void) fclose(ofp);

    return (_status);

}//EAConfigList::Write

char*
EAConfigList::Get(
const char*     pkeyword)
{
    for (EAStringPair* pair = GetHead(); pair != NULL; pair = GetNext())
    {
        if (strcmp(pair->keyword, pkeyword) == 0)
            return (pair->value);
    }
    return 0;
}//EAConfigList::Get

//--------
// GetInt 
//--------
// sets the integer based on the converted value
// returns OK, ERROR_FINDING_KEYWORD, or ERROR_CONVERTING_VALUE

EAConfigList::StatusE
EAConfigList::GetInt(
    const char*     pkeyword,
    int*            pvalue)
{
    char* string = Get(pkeyword);
    if (string == 0)
        _status = ERROR_FINDING_KEYWORD;
    else if (sscanf(string, "%d", pvalue) != 1)
        _status = ERROR_CONVERTING_VALUE;
    else
        _status = OK;

    return (_status);
}

//--------
// SetInt 
//--------
// enters a keyword value pair
// returns 1 on success, 0 on failure

char
EAConfigList::SetInt(
    const char*     pkeyword,
    const int       value)
{
    char string[32];
    sprintf(string, "%d", value);
    return (Set(pkeyword, string));
}

char
EAConfigList::Set(
const char*     pkeyword,
const char*     pvalue)
{
    //==========================================================
    // remove the old entry first (if set), then add the new one
    //==========================================================
    if (pkeyword == 0 || pvalue == 0)
        return 0;
    Remove(pkeyword);

    EAStringPair* pair = new EAStringPair();
    pair->keyword = new char[strlen(pkeyword) + 1];
    (void) strcpy(pair->keyword, pkeyword);
    pair->value = new char[strlen(pvalue) + 1];
    (void) strcpy(pair->value, pvalue);
    Append(pair);

    return 1;
}//EAConfigList::Set

void
EAConfigList::Remove(
const char*     pkeyword)
{
    for (EAStringPair* pair = GetHead(); pair != NULL; pair = GetNext())
    {
        if (strcmp(pair->keyword, pkeyword) == 0)
        {
            pair = RemoveCurrent();
            delete [] pair->keyword;
            delete [] pair->value;
            delete pair;
            return;
        }
    }

}//EAConfigList::Remove

#endif
