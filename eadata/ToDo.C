//=========================================================
// Copyright  (C)1995, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.0   04 Feb 1998 14:17:28   daffer
// Initial checking
// Revision 1.2  1998/01/30 22:29:25  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

#ifndef TODO_C_INCLUDED
#define TODO_C_INCLUDED

static const char info_rcs_id[]="@(#) $Header$";

#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "ToDo.h"

//static
const char* ToDo::StatusStrings[] =
{
    "OK",
    "WARNING: nothing to do",
    "ERROR: out of memory",
    "ERROR: null command",
    "ERROR: bad directory",
    "ERROR: get time failed"
};

//==============
// ToDo methods 
//==============

ToDo::ToDo(
    const char*     command)
:   _status(OK), _command(0)
{
    _SetCommand(command);
}

ToDo::~ToDo()
{
    if (_command)
        delete [] _command;
    return;
}

const char*
ToDo::GetStatusString(void)
{
    static char statusString[BIG_SIZE];
    (void)sprintf(statusString, "%s: %s", _command, StatusStrings[_status]);
    return statusString;

}//ToDo::GetStatusString

//-------------
// _SetCommand 
//-------------
// Sets the _command field of the object

ToDo::StatusE
ToDo::_SetCommand(
    const char*     command)
{
    if (_command)
        delete [] _command;

    _command = new char[strlen(command) + 1];

    if (_command == 0)
        return (_status = ToDo::ERROR_OUT_OF_MEMORY);

    strcpy(_command, command);
    return (_status = ToDo::OK);
}

ToDo::StatusE
ToDo::_DoIt(void)
{
    //------------------------------
    // time to execute the command
    //------------------------------
    if (_command)
    {
        char* line = new char[strlen(_command) + 3];
        (void)sprintf(line, "%s &", _command);
        system(line);
        delete [] line;
        return(_status = OK);
    }
    else
        return(_status = ERROR_NULL_COMMAND);
}//ToDo::_DoIt

//==========
// ToDoTime 
//==========

ToDoTime::ToDoTime(
    const char*             command,
    const unsigned char     days,
    const unsigned char     hours,
    const unsigned char     minutes)
:   ToDo(command)
{
    if (_SetTime(days, hours, minutes) != ToDo::OK)
        return;

    _status = ToDo::OK;
    return;
}

ToDoTime::~ToDoTime()
{
    return;
}

//--------------
// StringToToDo 
//--------------
// constructs a ToDoTime base on string
// returns null if string is not for a ToDoTime

ToDoTime*
ToDoTime::StringToToDo(
    const char*     string)
{
    char command[256], day_string[64];
    int hours, minutes;
    unsigned char days;

    if (sscanf(string, "[%[^]]] [%s %d:%d]",
        command, day_string, &hours, &minutes) != 4)
        return (0);
    
    if (! StringToDays(day_string, &days))
        return (0);

    ToDoTime* newtodo = new ToDoTime(command, days, hours, minutes);
    return (newtodo);
}

//--------------
// ToDoToString 
//--------------

int
ToDoTime::ToDoToString(
char*   string)
{
    char    tmp_days[32];
    DaysToString(_days, tmp_days);

    sprintf(string, "[%s] [%s %02d:%02d]", _command, tmp_days,
            _hours, _minutes);
    return (1);
}

ToDo::StatusE
ToDoTime::DoItIfDue(
Itime&      lastExecTime)
{
    //------------------------------------------------------
    // get timeofday, convert it to Itime,
    // get today's weekday, check against the _days
    //------------------------------------------------------
    Itime currentItime((time_t)time(0));;
    struct tm* tm_time=0;
    if ( ! currentItime.ItimeTotm(tm_time))
        return(_status = ERROR_GET_TIME);

    //------------------------------------------------------
    // if day of the week is not in _days, return
    //------------------------------------------------------
    char rightDay=0;
    switch(tm_time->tm_wday)
    {
        case 0: // sunday
            if (_days & SUNDAY)
                rightDay = 1;
            break;
        case 1: // monday
            if (_days & MONDAY)
                rightDay = 1;
            break;
        case 2: // tuesday
            if (_days & TUESDAY)
                rightDay = 1;
            break;
        case 3: // wednesday
            if (_days & WEDNESDAY)
                rightDay = 1;
            break;
        case 4: // thursday
            if (_days & THURSDAY)
                rightDay = 1;
            break;
        case 5: // friday
            if (_days & FRIDAY)
                rightDay = 1;
            break;
        case 6: // saturday
            if (_days & SATURDAY)
                rightDay = 1;
            break;
        default:
            return(_status = ERROR_GET_TIME);
    }
    //------------------------------
    // wrong day, return
    //------------------------------
    if ( ! rightDay)
        return(_status = WARNING_NOTHING_TO_DO);

    //-----------------------------------------------------------------
    // check the hour and minute, if it is past the last exec time,
    // or has not reached the current time, return WARNING_NOTHING_TO_DO
    //-----------------------------------------------------------------
    //------------------------------
    // already executed, return
    //------------------------------
    struct tm* last_tm_time=0;
    if ( ! lastExecTime.ItimeTotm(last_tm_time))
        return(_status = ERROR_GET_TIME);
    if (last_tm_time->tm_hour > (int)_hours ||
            (last_tm_time->tm_hour == (int)_hours &&
                    last_tm_time->tm_min > (int)_minutes))
        return(_status = WARNING_NOTHING_TO_DO);

    //------------------------------
    // not time yet, return
    //------------------------------
    if (tm_time->tm_hour < (int)_hours ||
            (tm_time->tm_hour == (int)_hours &&
                    tm_time->tm_min < (int)_minutes))
        return(_status = WARNING_NOTHING_TO_DO);

    //------------------------------
    // time to execute the command
    //------------------------------
    return(_DoIt());

}//ToDoTime::DoItIfDue

//----------
// _SetTime 
//----------
// Sets the _days, _hours, and _minutes fields of the object

ToDoTime::StatusE
ToDoTime::_SetTime(
    const unsigned char     days,
    const unsigned char     hours,
    const unsigned char     minutes)
{
    _hours = hours;
    _minutes = minutes;
    _days = days;
    return (_status = ToDo::OK);
}

//--------------
// DaysToString 
//--------------

int
DaysToString(
    const unsigned char     days,
    char*                   string)
{
    *string = '\0';
    if (days & ToDoTime::SUNDAY) strcat(string, SUNDAY_STRING);
    if (days & ToDoTime::MONDAY) strcat(string, MONDAY_STRING);
    if (days & ToDoTime::TUESDAY) strcat(string, TUESDAY_STRING);
    if (days & ToDoTime::WEDNESDAY) strcat(string, WEDNESDAY_STRING);
    if (days & ToDoTime::THURSDAY) strcat(string, THURSDAY_STRING);
    if (days & ToDoTime::FRIDAY) strcat(string, FRIDAY_STRING);
    if (days & ToDoTime::SATURDAY) strcat(string, SATURDAY_STRING);
    return (1);
}

//--------------
// StringToDays 
//--------------

int
StringToDays(
    const char*     string,
    unsigned char*  days)
{
    *days = 0;
    if (strstr(string, SUNDAY_STRING)) *days |= ToDoTime::SUNDAY;
    if (strstr(string, MONDAY_STRING)) *days |= ToDoTime::MONDAY;
    if (strstr(string, TUESDAY_STRING)) *days |= ToDoTime::TUESDAY;
    if (strstr(string, WEDNESDAY_STRING)) *days |= ToDoTime::WEDNESDAY;
    if (strstr(string, THURSDAY_STRING)) *days |= ToDoTime::THURSDAY;
    if (strstr(string, FRIDAY_STRING)) *days |= ToDoTime::FRIDAY;
    if (strstr(string, SATURDAY_STRING)) *days |= ToDoTime::SATURDAY;
    return (1);
}

//==========
// ToDoFile 
//==========

ToDoFile::ToDoFile(
    const char*     command,
    const char*     directory)
:   ToDo(command), _directory(0)
{
    if (_SetDirectory(directory) != ToDo::OK)
        return;

    _status = ToDo::OK;
    return;
}

ToDoFile::~ToDoFile()
{
    if (_directory)
        delete [] _directory;

    return;
}

//--------------
// StringToToDo 
//--------------
// constructs a ToDoFile base on string
// returns null if string is not for a ToDoFile

ToDoFile*
ToDoFile::StringToToDo(
    const char*     string)
{
    char command[256], directory[256];
    if (sscanf(string, "[%[^]]] [%[^]]]", command, directory) != 2)
        return (0);
    
    ToDoFile* newtodo = new ToDoFile(command, directory);
    return (newtodo);
}


//--------------
// ToDoToString 
//--------------

int
ToDoFile::ToDoToString(
    char*   string)
{
    sprintf(string, "[%s] [%s]", _command, _directory);
    return (1);
}

ToDo::StatusE
ToDoFile::DoItIfDue(
Itime&      lastExecTime)
{
    //-----------------------------------------------------
    // go through the DueFiles list,
    // if the filesize is not changed: assume no one is
    // writing to the file.  Do the command and remove the
    // record form the DueFiles list
    //-----------------------------------------------------
    DueFiles* file=0;
    for (file=_files.GetHead(); file; file=_files.GetNext())
    {
        struct stat buf;
        if (stat(file->filename, &buf) == 0)
        {
            if (buf.st_size == file->filesize)
            {
                (void)_DoIt();  // shall we pass fileFullPath???
                _files.RemoveCurrent();
            }
        }
    }

    //-------------------------------------------------------
    // open the specified directory;
    // check the modification time of each file
    //-------------------------------------------------------
    if (_directory == 0)
        return(_status = ERROR_BAD_DIRECTORY);

    DIR* dirP=0;
    if ((dirP=opendir(_directory)) == NULL)
            return(_status = ERROR_BAD_DIRECTORY);

    struct dirent* direntP=0;
    while ((direntP=readdir(dirP)) != 0)
    {
        //-----------------------------------------------------
        // skip ".", only interested in files
        //-----------------------------------------------------
        if (strcmp(direntP->d_name, ".") == 0)
            continue;

        //-----------------------------------------------------
        // if modification time is > last exec time, 
        // save the filename and filesize in the DueFiles list
        //-----------------------------------------------------
        char fileFullPath[BIG_SIZE];
        (void)sprintf(fileFullPath, "%s/%s", _directory, direntP->d_name);
        struct stat buf;
        if (stat(fileFullPath, &buf) == 0)
        {
            if (buf.st_mtime > lastExecTime.sec)
            {
                char inList=0;
                for (file=_files.GetHead(); file; file=_files.GetNext())
                {
                    //--------------------------------------------------
                    // if filesize is still changing (someone is still
                    // writing to the file), update the filesize
                    // and skip to the next file
                    //--------------------------------------------------
                    if (strcmp(fileFullPath, file->filename) == 0)
                    {
                        if (file->filesize != buf.st_size)
                            file->filesize = buf.st_size;
                        inList = 1;
                        break;
                    }
                }
                if (inList)
                    continue;

                DueFiles* newfile = new DueFiles;
                (void)strcpy(newfile->filename, fileFullPath);
                newfile->filesize = buf.st_size;
                _files.Append(newfile);
            }
        }
    }
    closedir(dirP);
    return(_status = OK);

}//ToDoFile::DoItIfDue

//---------------
// _SetDirectory 
//---------------
// Sets the _directory field of the object

ToDoFile::StatusE
ToDoFile::_SetDirectory(
    const char*     directory)
{
    if (_directory)
        delete [] _directory;

    _directory = new char[strlen(directory) + 1];

    if (_directory == 0)
        return (_status = ERROR_OUT_OF_MEMORY);

    strcpy(_directory, directory);
    return (_status = ToDo::OK);
}

//==================
// ToDoList methods 
//==================

ToDoList::ToDoList()
:   _status(OK)
{
    return;
}

ToDoList::ToDoList(
    const char*     filename)
{
    Read(filename);
    return;
}

ToDoList::~ToDoList()
{
    // empty

}//ToDoList::~ToDoList

//------
// Read 
//------

ToDoList::StatusE
ToDoList::Read(
    const char*     filename)
{
    //------------------------
    // open the ToDoList file 
    //------------------------

    FILE* ifp = fopen(filename, "r");
    if (ifp == NULL)
        return (_status = ERROR_OPENING_FILE);

    //--------------------------------
    // read in the file, line by line 
    //--------------------------------

    ToDo* newtodo;
    _status = ToDoList::OK;
    char line[LINE_LEN];
    char *ptr;
    do {
        ptr = fgets(line, LINE_LEN, ifp);
        if (ptr != line)
        {
            if (feof(ifp))
                break;
            else
                return (_status = ERROR_READING_TODO_LINE);
        }

        //----------------
        // try a ToDoTime 
        //----------------

        newtodo = ToDoTime::StringToToDo(line);
        if (newtodo)
        {
            if (newtodo->GetStatus() != ToDo::OK)
                return (_status = ERROR_CREATING_TODO);
            else
            {
                Append(newtodo);
                continue;
            }
        }

        //----------------
        // try a ToDoFile 
        //----------------

        newtodo = ToDoFile::StringToToDo(line);
        if (newtodo)
        {
            if (newtodo->GetStatus() != ToDo::OK)
                return (_status = ERROR_CREATING_TODO);
            else
            {
                Append(newtodo);
                continue;
            }
        }

        //---------
        // unknown 
        //---------

        return (_status = ERROR_PARSING_TODO);

    } while (1);

    //----------------
    // close the file 
    //----------------

    fclose(ifp);
    return (_status);
}

//-------
// Write 
//-------

ToDoList::StatusE
ToDoList::Write(
    const char*     filename)
{
    //------------------------
    // open the ToDoList file 
    //------------------------

    FILE* ofp = fopen(filename, "w");
    if (ofp == NULL)
        return (_status = ERROR_OPENING_FILE);

    //-----------------
    // write each todo 
    //-----------------

    char string[LINE_LEN];
    for (ToDo* todo = GetHead(); todo; todo = GetNext())
    {
        todo->ToDoToString(string);
        fprintf(ofp, "%s\n", string);
    }

    //-----------------------
    // close the output file 
    //-----------------------

    fclose(ofp);
    return (_status);
}

//----------------------------------
// remove a ToDo from list
//----------------------------------
ToDo*
ToDoList::RemoveToDo(
char*   toDoString)
{
    for (ToDo* toDo=GetHead(); toDo != 0; toDo=GetNext())
    {
        char string[BIG_SIZE];
        if (toDo->ToDoToString(string))
        {
            if (strcmp(toDoString, string) == 0)
            return(RemoveCurrent());
        }
    }
    return 0;

}//ToDoList::RemoveToDo

#endif
