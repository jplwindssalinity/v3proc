//=========================================================
// Copyright  (C)1995, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.2   20 Apr 1998 15:20:20   sally
// change List to EAList
// 
//    Rev 1.1   26 Feb 1998 10:02:46   sally
// add virtual to destructor
// 
//    Rev 1.0   04 Feb 1998 14:17:30   daffer
// Initial checking
// Revision 1.2  1998/01/30 22:28:46  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

#ifndef TODO_H
#define TODO_H

//---------------------------------------------------------------
// The ToDo object is used for automatic scheduling of commands. 
// Commands can either be executed at a given time on given days 
// of the week, or when a file appears in a given directory.     
//---------------------------------------------------------------

static const char rcs_id_todo_h[]="@(#) $Header$";

#include <string.h>

#include "CommonDefs.h"
#include "Itime.h"
#include "EAList.h"

#define SUNDAY_STRING       "Su"
#define MONDAY_STRING       "Mo"
#define TUESDAY_STRING      "Tu"
#define WEDNESDAY_STRING    "We"
#define THURSDAY_STRING     "Th"
#define FRIDAY_STRING       "Fr"
#define SATURDAY_STRING     "Sa"

#define LINE_LEN            1024

//======
// ToDo 
//======

class ToDo
{
public:
    friend operator==(const ToDo&, const ToDo&);

    enum StatusE
    {
        OK,
        WARNING_NOTHING_TO_DO,
        ERROR_OUT_OF_MEMORY,
        ERROR_NULL_COMMAND,     // should never have happened
        ERROR_BAD_DIRECTORY,        // should never have happened
        ERROR_GET_TIME
    };

    ToDo(const char* command);
    virtual ~ToDo();

    virtual int     ToDoToString(char* string) = 0;
    inline StatusE  GetStatus(void) { return (_status); };
    const char*     GetStatusString(void);

                    // return 1 if there is something to do and done
    virtual StatusE DoItIfDue(Itime& lastExecTime) = 0;

protected:
    virtual StatusE _DoIt(void);

    StatusE         _status;
    char*           _command;

    static const char* StatusStrings[];

private:
    StatusE         _SetCommand(const char* command);
};

inline int operator==(const ToDo& a, const ToDo& b)
{ return (strcmp(a._command, b._command) == 0 ? 1 : 0); }

//==========
// ToDoTime 
//==========

class ToDoTime : public ToDo
{
public:
    enum DaysE
    {
        SUNDAY =    0x01,
        MONDAY =    0x02,
        TUESDAY =   0x04,
        WEDNESDAY = 0x08,
        THURSDAY =  0x10,
        FRIDAY =    0x20,
        SATURDAY =  0x40
    };

    ToDoTime(const char* command, const unsigned char days,
                const unsigned char hours, const unsigned char minutes);
    ~ToDoTime();

    int                 ToDoToString(char* string);
    static ToDoTime*    StringToToDo(const char* string);

                        // return 1 if there is something to do and done
    virtual StatusE     DoItIfDue(Itime& lastExecTime);

private:
    StatusE             _SetTime(   const unsigned char days,
                                    const unsigned char hours,
                                    const unsigned char minutes);

    unsigned char       _days;
    unsigned char       _hours;
    unsigned char       _minutes;
};

int     DaysToString(const unsigned char days, char* string);
int     StringToDays(const char* string, unsigned char* days);

//==========
// ToDoFile 
//==========

struct DueFiles
{
    char    filename[BIG_SIZE];
    off_t   filesize;
};

inline int operator==(const DueFiles& a, const DueFiles& b)
{
    return(strcmp(a.filename, b.filename) == 0 &&
                           a.filesize == b.filesize ?  1 : 0);
}

class ToDoFile : public ToDo
{
public:
    ToDoFile(const char* command, const char* directory);
    ~ToDoFile();

    int                 ToDoToString(char* string);
    static ToDoFile*    StringToToDo(const char* string);

                        // return 1 if there is something to do and done
    virtual StatusE     DoItIfDue(Itime& lastExecTime);

protected:

    StatusE             _SetDirectory(const char* directory);
    char*               _directory;
    EAList<DueFiles>    _files;
};

//==========
// ToDoList 
//==========

class ToDoList : public EAList<ToDo>
{
public:
    enum StatusE
    {
        OK,
        ERROR_OPENING_FILE,
        ERROR_READING_TODO_LINE,
        ERROR_CREATING_TODO,
        ERROR_PARSING_TODO
    };

    ToDoList();
    ToDoList(const char* filename);
    ~ToDoList();

    inline StatusE  GetStatus() { return (_status); };

    StatusE     Write(const char* filename);
    StatusE     Read(const char* filename);
    ToDo*       RemoveToDo(char* string);

private:
    StatusE     _status;
};

#endif
