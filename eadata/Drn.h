//=========================================================
// Copyright  (C)1995, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.1   10 Apr 1998 14:04:12   daffer
//     Changed ConfigList to EAConfigList throughout
// 
//    Rev 1.0   04 Feb 1998 14:15:10   daffer
// Initial checking
// Revision 1.2  1998/01/30 22:28:15  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

// This file outlines the Drn object

#ifndef DRN_H
#define DRN_H

#include "EAConfigList.h"

static const char rcs_id_drn_h[] =
    "@(#) $Header$";

#define ERROR_ADDRESS_KEYWORD       "ERROR_ADDRESS"
#define RECEIVING_SITE_KEYWORD      "RECEIVING_SITE"

#define FROM_LABEL          "From:"
#define TO_LABEL            "To:"
#define SUBJECT_LABEL       "Subject:"

#define FILE_XMIT_MAIL_TYPE_ID  "F"
#define DRN_BREAKOUT            "I"
#define RCN_BREAKOUT            "R"
#define BEGINNING_OF_MESSAGE    "<BOM>"
#define END_OF_MESSAGE          "<EOM>"
#define RCN_MAIL_URGENCY_ID     "NR"

#define TAG_LINE_DELIMETER      "----------\n"

#define LINE_LENGTH     1024

//===========
// Drn class 
//===========

class Drn
{
public:
    enum StatusE
    {
        OK,
        ERROR_OPENING_LOG_FILE
    };

    Drn(const char* config_filename, const char* log_filename);
    ~Drn();

    int     ReadConfigList(const char* config_file);
    int     ReadHeader();
    int     IsValidDrn();
    int     ProcessBody(const char* config_filename);
    int     GetFileSendRcn(const char* file_name, const int file_size,
                const char* dest_computer_id, const char* config_filename);
    int     SendRcn(const char* file_name, const char* local_filename,
                const int file_size, const char* dest_computer_id,
                const char* rcn_address, const char* cc_address,
                const int status);
    void    ReportError(const char* log_file);
    StatusE GetStatus() { return(_status); };

private:
    EAConfigList*     _configList;        // must be deleted
    char*           _from;              // must be freed
    char*           _to;                // must be freed
    char*           _subject;           // must be freed

    char*           _errorAddress;
    char*           _receivingSite;

    char            _computerIdOfSender[10];
    char            _computerIdOfReceiver[10];

    StatusE         _status;
    FILE*           _logFp;
};

#endif
