//=========================================================
// Copyright  (C)1998, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.3   21 Apr 1998 16:39:32   sally
// for L2B
// 
//    Rev 1.2   20 Apr 1998 15:18:12   sally
// change List to EAList
// 
//    Rev 1.1   10 Apr 1998 14:04:14   daffer
//   Copied version 1.1 of ConfigList to EAConfiglist (undoing the svt merge sally had done)
//   Changed ConfigList to EAConfigList throughout
// 
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

//=========================================================
// Config List is a list of keyword-and-value pair of      
// strings.  e.g.:                                         
//    HK2_LIMIT_FILE      /home1/omicron/files/...        
//                                                         
// Config List is designed to replace the need for getenv()
//=========================================================

#ifndef EACONFIG_LIST_H
#define EACONFIG_LIST_H

static const char rcsidEAConfigList_h[] = "@(#) $Header$";

#include <stdio.h>
#include <string.h>

#include "EAList.h"

#define LINE_SIZE   1024

struct EAStringPair
{
    EAStringPair() : keyword(0), value(0) {};
    char*   keyword;
    char*   value;
};

inline int operator== (const EAStringPair& a, const EAStringPair& b)
{
    return(strcmp(a.keyword, b.keyword) == 0 &&
             strcmp(a.value, b.value) == 0 ? 1 : 0);
}

class EAConfigList : public EAList<EAStringPair>
{
public:
    enum StatusE
    {
        OK,
        ERROR_OPENING_FILE,
        ERROR_READING_CONFIG_ENTRY,
        ERROR_APPENDING_CONFIG_ENTRY,
        ERROR_WRITING_CONFIG_FILE,
        ERROR_FINDING_KEYWORD,
        ERROR_CONVERTING_VALUE
    };

    EAConfigList();
    EAConfigList(const char* filename);
    virtual ~EAConfigList();

    StatusE         GetStatus() { return (_status); };
    char*           GetBadLine() { return (_badLine); };

    StatusE         Read(const char* filename);
    StatusE         Write(const char* filename);

    char*           Get(const char* keyword);

    // GetInt puts the integer corresponding to keyword into
    // the space pointed to by value.  It returns OK, 
    // ERROR_FINDING_KEYWORD, or ERROR_CONVERTING_VALUE
    StatusE         GetInt(const char* keyword, int* value);

                    //===================================================
                    // if keyword previously set, change to new value,
                    // else append.
                    //===================================================
    char            Set(const char* keyword, const char* value);
    char            SetInt(const char* keyword, const int value);

                    //===================================================
                    // returned char* is a local static string that is
                    // reused upon each call; user should copy it.
                    //===================================================
    void            Remove(const char* keyword);

protected:
    StatusE         _status;
    char            _badLine[LINE_SIZE];

};//EAConfigList

#endif //EACONFIG_LIST_H
