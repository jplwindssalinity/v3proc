//=========================================================
// Copyright  (C)1995, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.7   17 Apr 1998 16:48:22   sally
// add L2A and L2B parameter tables
// 
//    Rev 1.6   10 Apr 1998 14:04:08   daffer
//   Changed ConfigList to EAConfigList throughout
// 
//    Rev 1.5   07 Apr 1998 08:48:36   sally
// fixed for SVT's ConfigList
// 
//    Rev 1.4   06 Apr 1998 16:26:14   sally
// merged with SVT
// 
//    Rev 1.3   24 Mar 1998 09:08:36   sally
// de-warn for GNU compiler
// 
//    Rev 1.2   26 Feb 1998 09:42:00   sally
// rearrange the order of member initialization
// 
//    Rev 1.1   17 Feb 1998 14:44:50   sally
// NOPM
// 
//    Rev 1.0   04 Feb 1998 14:14:36   daffer
// Initial checking
// Revision 1.3  1998/01/30 22:28:49  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================
 
static const char rcs_id_args_c[]="@(#) $Header$";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Args.h"
#include "ArgDefs.h"

//==============
// Args methods 
//==============

Args::Args(
    int         argc,
    char*       argv[],
    ArgInfo*    usage_array[])
:   _status(OK),
    Log( argv[0], GetLogFileName(argc, argv), argc, argv),
    _usageArray(usage_array)
{
    _programName = argv[0];
    if (_MakeCmdLineConfigList(argc, argv) != Args::OK)
        return;
    char* cmd_line_config_file = GetFromCmdLine(CONFIG_FILE_OPTION);
    if (cmd_line_config_file)
        _MakeConfigFileConfigList(cmd_line_config_file);
    return;
}

Args::Args(
    int         argc,
    char*       argv[],
    char*       config_filename,
    ArgInfo*    usage_array[])
:   _status(OK),
    Log( argv[0], GetLogFileName(argc, argv), argc, argv),
    _usageArray(usage_array)
{
    _programName = argv[0];
    if (_MakeCmdLineConfigList(argc, argv) != Args::OK)
        return;
    char* cmd_line_config_file = GetFromCmdLine(CONFIG_FILE_OPTION);
    if (cmd_line_config_file)
        _MakeConfigFileConfigList(cmd_line_config_file);
    else
        _MakeConfigFileConfigList(config_filename);
    return;
}

//----------------
// GetFromCmdLine 
//----------------

char*
Args::GetFromCmdLine(
    char*   keyword)
{
    return(_cmdLineConfigList.Get(keyword));
}

//-------------------
// GetFromConfigFile 
//-------------------

char*
Args::GetFromConfigFile(
    char*       keyword)
{
    return(_configFileConfigList.Get(keyword));
}

//-----
// Get 
//-----

char*
Args::Get(
    char*   option,
    char*   keyword)
{
    char* value;
    value = GetFromCmdLine(option);
    if (value)
        return(value);
    value = GetFromConfigFile(keyword);
    return(value);
}

//-----
// Get 
//-----
 
char*
Args::Get(
    ArgInfo arg_info)
{
    return(Get(arg_info.option, arg_info.keyword));
}

//-----------
// GetOrExit 
//-----------
 
char*
Args::GetOrExit(
    ArgInfo arg_info)
{
    char*   ptr = Get(arg_info);
    int i;
    if (! ptr)
    {
        int length = strlen(_programName);
        fprintf(stderr, "%s: need [ %s %s ] on command line\n",
            _programName, arg_info.option, arg_info.argument);
        for (i = 0; i < length + 2; i++)
            fprintf(stderr, " ");
        fprintf(stderr, "or %s in configuration file\n", arg_info.keyword);
	
        // Now write the same message to the log file

        sprintf( _UsageMsg, "%s: need [ %s %s ] on command line\n",
                      _programName, arg_info.option, arg_info.argument );
        Log.WriteMsg(_UsageMsg);
        for (i = 0; i < length + 2; i++)
            _UsageMsg[i] = ' ';
        sprintf(_UsageMsg+i, "or %s in configuration file\n", arg_info.keyword);
        Log.WriteMsg(_UsageMsg);
        Usage();
        //exit(1);
        Log.SetWriteAndExit(EALog::EA_FAILURE," --- Aborting ---\n");
    }
    else
        return(ptr);

    return 0;
}

//-------
// Usage 
//-------
 
#define USAGE "Usage: "
 
void
Args::Usage(
    ArgInfo*    usage_array[])
{
    char *p = _UsageMsg;
    int nchar;
    _UsageMsg[MAX_USAGE_MSG_LEN]= (char) NULL;
    nchar = sprintf(p, "%s%s", USAGE, _programName);
    int skip = strlen(USAGE) + 4;
    int position = strlen(USAGE) + strlen(_programName);
    p += nchar ;
    for (int i = 0; usage_array[i]; i++)
    {
        char optstring[80];
        if (usage_array[i]->argument) 
            nchar = sprintf(optstring, " [ %s %s ]", 
                             usage_array[i]->option, usage_array[i]->argument);
	    else 
            nchar= sprintf(optstring, " [ %s ]", usage_array[i]->option);
        int ll=strlen(_UsageMsg) + nchar;
        if ( ll > MAX_USAGE_MSG_LEN) {
            fprintf(stderr, "Line truncated to <1024 chars\n");
            break;
        }
        else
        {
            int length = nchar;
            position += length;
            ll=strlen(_UsageMsg) + length;
            if (position > 78 )
            {
                nchar = sprintf(p, "\n%*s", skip, " ");
                position = skip + length;
                p += nchar;
                nchar = sprintf(p, "%s", optstring);
                p += nchar;
            }
            else
            {
                strcat( p, optstring );
                p += nchar;
            }
        }
    }

    strcat( p, "\n");
    fprintf(stderr,"%s",_UsageMsg);
    return;
}
 
//-------
// Usage 
//-------
 
void
Args::Usage()
{
    Usage(_usageArray);
    return;
}

//--------------
// _IsBadOption 
//--------------
// returns 1 if a command line option is not valid
 
int
Args::_IsBadOption(
    const char* option)
{
    for (int i = 0; _usageArray[i]; i++)
    {
        if (strcmp(option, _usageArray[i]->option) == 0)
            return(0);  // valid option
    }
    return(1);          // invalid option
}

//------------------------
// _MakeCmdLineConfigList 
//------------------------

Args::StatusE
Args::_MakeCmdLineConfigList(
    int     argc,
    char*   argv[])
{
    int option_index = 0;
    for (int i = 1; i < argc; i++)
    {
        if (*argv[i] == '-')
        {
            // new option
            if (option_index)
            {
                // "close out" previous option
                if (_AddToConfigList(&_cmdLineConfigList, argv[option_index],
                    i - option_index - 1, argv + option_index + 1) != OK)
                {
                    return(_status);
                }
            }
            option_index = i;
        }
    }
    if (option_index)
    {
        // "close out" final option
        if (_AddToConfigList(&_cmdLineConfigList, argv[option_index],
            argc - option_index - 1, argv + option_index + 1) != OK)
        {
            return(_status);
        }
    }
    return(_status);
}

//---------------------------
// _MakeConfigFileConfigList 
//---------------------------

Args::StatusE
Args::_MakeConfigFileConfigList(
    char*   config_filename)
{
    _configFileConfigList.Read(config_filename);
    return(_status);
}

//------------------
// _AddToConfigList 
//------------------

Args::StatusE
Args::_AddToConfigList(
    EAConfigList*  config_list,
    char*           keyword,
    int             value_count,
    char*           value_array[])
{
    // determine the size of the value string
    int value_string_size;
    if (value_count < 1)
    {
        value_string_size = 1;              // for the terminator
    }
    else
    {
        value_string_size = 0;
        for (int i = 0; i < value_count; i++)
            value_string_size += strlen(value_array[i]);
        value_string_size += value_count;   // for the spaces/terminator
    }

    // allocate the value string
    char* value_string = new char[value_string_size];
    if (! value_string)
        return(_status = ERROR_ALLOCATING_VALUE_STRING);

    // concatinate the strings
    if (value_count < 1)
    {
        *value_string = '\0';
    }
    else
    {
        strcpy(value_string, value_array[0]);
        for (int i = 1; i < value_count; i++)
        {
            strcat(value_string, " ");
            strcat(value_string, value_array[i]);
        }
    }

    // add the pair to the config list
    if (! config_list->Set(keyword, value_string))
        _status = ERROR_ADDING_PAIR;

    delete value_string;

    return(_status);
}
char * 
Args::GetLogFileName(int argc, char **argv)
{
  ArgInfo logfile = LOG_FILE_ARG;
  char *config_filename = getenv(ENV_CONFIG_FILENAME);
  if (_MakeCmdLineConfigList(argc, argv) != Args::OK) {
    Log.SetStatus(EALog::EA_FAILURE);
    return Get(logfile);
  }
  char* cmd_line_config_file = GetFromCmdLine(CONFIG_FILE_OPTION);
  if (cmd_line_config_file)
    _MakeConfigFileConfigList(cmd_line_config_file);
  else
    _MakeConfigFileConfigList(config_filename);
  return Get(logfile);

} // GetLogFileName
