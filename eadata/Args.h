//=========================================================
// Copyright  (C)1995, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.2   22 May 1998 16:24:36   daffer
// Added GetStatus method
// 
//    Rev 1.1   10 Apr 1998 14:04:08   daffer
//   Changed ConfigList to EAConfigList throughout
// 
//    Rev 1.0   04 Feb 1998 14:14:36   daffer
// Initial checking
// Revision 1.3  1998/01/30 22:28:05  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

#ifndef ARGS_H
#define ARGS_H

static const char rcs_args_h[]="@(#) $Header$";

#include <stdio.h>
#include "EAConfigList.h"
#include "EALog.h"

//=========
// ArgInfo 
//=========

class ArgInfo
{
public:
    char*   keyword;
    char*   option;
    char*   argument;
};

//======
// Args 
//======

#define CONFIG_FILE_OPTION  "-config"
#define MAX_USAGE_MSG_LEN 1024
class Args
{
public:
    enum StatusE
    {
        OK, ERROR_ALLOCATING_VALUE_STRING, ERROR_ADDING_PAIR
    };

    Args(int argc, char* argv[], ArgInfo* usage_array[]);
    Args(int argc, char* argv[], char* config_filename,
        ArgInfo* usage_array[]);
    char*       GetFromCmdLine(char* option);
    char*       GetFromConfigFile(char* keyword);
    char*       Get(char* option, char* keyword);
    char*       Get(ArgInfo arg_info);
    char*       GetOrExit(ArgInfo arg_info);
    char*       GetLogFileName(int, char**);
    void        Usage(ArgInfo* arg_info_array[]);
    void        Usage();
    StatusE     GetStatus() { return (_status);};

protected:
    int         _IsBadOption(const char* option);
    StatusE     _MakeCmdLineConfigList(int argc, char* argv[]);
    StatusE     _MakeConfigFileConfigList(char* config_filename);
    StatusE     _AddToConfigList(EAConfigList* config_list, char* keyword,
                    int value_count, char* value_array[]);

    char*       _programName;
    EAConfigList  _cmdLineConfigList;
    EAConfigList  _configFileConfigList;
    char*       _argFreeOpt;
    char        _UsageMsg[MAX_USAGE_MSG_LEN+1];
    StatusE     _status;
    
    EALog       Log;
private:
    ArgInfo**   _usageArray;
};

#endif
