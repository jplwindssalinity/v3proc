//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef NEW_FILES_H
#define NEW_FILES_H

static const char rcs_id_new_files_h[] =
    "@(#) $Id$";

#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <fnmatch.h>
#include <sys/stat.h>
#include "List.h"

//======================================================================
// CLASSES
//======================================================================

//======================================================================
// CLASS
// FileInfo
//
// DESCRIPTION
//    The FileInfo object containts file information.
//======================================================================

class FileInfo
{
public:

    //--------------//
    // construction //
    //--------------//

    FileInfo();
    ~FileInfo();

    int          Set(const char* file);
    const char*  Get() { return(_file); };

private:

    //-----------//
    // variables //
    //-----------//

    char*  _file;
};

//======================================================================
// CLASS
// FileList
//
// DESCRIPTION
//    The FileList object containts a list of FileInfo.
//======================================================================

class FileList : public List<FileInfo>
{
public:

    //--------------//
    // construction //
    //--------------//

    FileList();
    ~FileList();

    int  Add(const char* file);

    //--------------//
    // input/output //
    //--------------//

    int  Read(const char* filename);
    int  Print();

    //--------------//
    // manipulation //
    //--------------//

    int  ContainsFile(const char* filename);
};

//======================================================================
// CLASS
// Pattern
//
// DESCRIPTION
//    The Pattern object containts information about where to find
//    and how to determine the maturity of files.
//======================================================================

class Pattern
{
public:

    //--------------//
    // construction //
    //--------------//

    Pattern();
    ~Pattern();

    int  Set(const char* directory, const char* pattern);

    //--------------//
    // manipulation //
    //--------------//

    int  NewStableFiles(FileList* new_file_list, FileList* done_file_list,
             int maturity);

private:

    //-----------//
    // variables //
    //-----------//

    char*  _directory;
    char*  _pattern;
};

//======================================================================
// CLASS
// PatternList
//
// DESCRIPTION
//    The PatternList object containts a list of patterns.
//======================================================================

class PatternList : public List<Pattern>
{
public:

    //--------------//
    // construction //
    //--------------//

    PatternList();
    ~PatternList();

    //--------------//
    // input/output //
    //--------------//

    int  Read(const char* pattern_file, const char* target_type);

    //--------------//
    // manipulation //
    //--------------//

    int  Add(const char* directory, const char* pattern);
    int  NewStableFiles(FileList* new_file_list, FileList* done_file_list);

private:

    //-----------//
    // variables //
    //-----------//

    int  _maturity;
};

//==========//
// FileInfo //
//==========//

FileInfo::FileInfo()
:   _file(NULL)
{
    return;
}

FileInfo::~FileInfo()
{
    free(_file);
    return;
}

//---------------//
// FileInfo::Set //
//---------------//

int
FileInfo::Set(
    const char*  file)
{
    free(_file);

    _file = strdup(file);
    if (_file == NULL)
        return(0);

    return(1);
}

//==========//
// FileList //
//==========//

FileList::FileList()
{
    return;
}

FileList::~FileList()
{
    FileInfo* fileinfo;
    GotoHead();
    while ((fileinfo = RemoveCurrent()) != NULL)
        delete fileinfo;
    return;
}

//---------------//
// FileList::Add //
//---------------//

int
FileList::Add(
    const char*  file)
{
    FileInfo* new_fileinfo = new FileInfo;
    if (new_fileinfo == NULL)
        return(0);

    if (! new_fileinfo->Set(file) ||
        ! Append(new_fileinfo))
    {
        delete new_fileinfo;
        return(0);
    }

    return(1);
}

//----------------//
// FileList::Read //
//----------------//

#define FILE_SIZE  1024

int
FileList::Read(
    const char*  filename)
{
    FILE* ifp = fopen(filename, "r");
    if (ifp == NULL)
        return(0);

    char file[FILE_SIZE];
    while (fscanf(ifp, " %s", file) == 1)
    {
        if (! Add(file))
        {
            fclose(ifp);
            return(0);
        }
    }

    if (feof(ifp))
    {
        fclose(ifp);
        return(1);
    }

    fclose(ifp);
    return(0);
}

//-----------------//
// FileList::Print //
//-----------------//

int
FileList::Print()
{
    for (FileInfo* fileinfo = GetHead(); fileinfo != NULL;
        fileinfo = GetNext())
    {
        printf("%s\n", fileinfo->Get());
    }
    return(1);
}

//------------------------//
// FileList::ContainsFile //
//------------------------//

int
FileList::ContainsFile(
    const char*  file)
{
    for (FileInfo* fileinfo = GetHead(); fileinfo != NULL;
        fileinfo = GetNext())
    {
        if (strcmp(fileinfo->Get(), file) == 0)
            return(1);
    }
    return(0);
}

//=========//
// Pattern //
//=========//

Pattern::Pattern()
:   _directory(NULL), _pattern(NULL)
{
    return;
}

Pattern::~Pattern()
{
    free(_directory);
    free(_pattern);
    return;
}

//--------------//
// Pattern::Set //
//--------------//

int
Pattern::Set(
    const char*  directory,
    const char*  pattern)
{
    free(_directory);
    free(_pattern);

    _directory = strdup(directory);
    if (_directory == NULL)
        return(0);

    _pattern = strdup(pattern);
    if (_pattern == NULL)
        return(0);

    return(1);
}

//-------------------------//
// Pattern::NewStableFiles //
//-------------------------//

int
Pattern::NewStableFiles(
    FileList*  new_file_list,
    FileList*  done_file_list,
    int        maturity)
{
    //----------------//
    // open directory //
    //----------------//

    DIR* dir = opendir(_directory);
    if (dir == NULL)
        return(0);    // not a directory or some kind of error

    struct dirent* dir_entry;
    while ((dir_entry = readdir(dir)) != NULL)
    {
        // check against pattern
        if (fnmatch(_pattern, dir_entry->d_name, FNM_PATHNAME) != 0)
            continue;

        // check if done
        char fullpath[1024];
        sprintf(fullpath, "%s/%s", _directory, dir_entry->d_name);
        if (done_file_list->ContainsFile(fullpath))
            continue;

        // get file info
        struct stat statbuf;
        if (! stat(fullpath, &statbuf) != 0)
            continue;    // failure to stat

        // check maturity
        time_t last_mod_time = statbuf.st_mtime;
        time_t current_time = time(0);
        time_t age = current_time - last_mod_time;
        if (age < maturity)
            continue;

        // add to list
        if (! new_file_list->Add(fullpath))
        {
            closedir(dir);
            return(0);
        }
    }
    closedir(dir);
    return(1);
}

//=============//
// PatternList //
//=============//

PatternList::PatternList()
:   _maturity(0)
{
    return;
}

PatternList::~PatternList()
{
    return;
}

//-------------------//
// PatternList::Read //
//-------------------//

#define LINE_SIZE     2048
#define TYPE_SIZE     16
#define DIR_SIZE      1024
#define PATTERN_SIZE  512

int
PatternList::Read(
    const char*  pattern_file,
    const char*  target_type)
{
    //-----------------------//
    // open the pattern file //
    //-----------------------//

    FILE* ifp = fopen(pattern_file, "r");
    if (ifp == NULL)
        return(0);

    //--------------------------//
    // read and parse each line //
    //--------------------------//

    char line[LINE_SIZE];
    while (fgets(line, LINE_SIZE, ifp) == line)
    {
        char type[TYPE_SIZE];
        char directory[DIR_SIZE];
        char pattern[PATTERN_SIZE];
        int age;
        if (sscanf(line, " %s %s %s", type, directory, pattern) == 3)
        {
            // check the type
            if (strcmp(type, target_type) != 0)
                continue;

            if (! Add(directory, pattern))
            {
                fclose(ifp);
                return(0);
            }
        }
        else if (sscanf(line, "%s %d\n", type, &age) == 2)
        {
            // check the type
            if (strcmp(type, target_type) != 0)
                continue;

            _maturity = age;
        }
    }

    //-----------------------------------//
    // check for EOF, and wrap things up //
    //-----------------------------------//

    if (feof(ifp))
    {
        fclose(ifp);
        return(1);
    }

    fclose(ifp);
    return(0);
}

//------------------//
// PatternList::Add //
//------------------//

int
PatternList::Add(
    const char*  directory,
    const char*  pattern)
{
    Pattern* new_pattern = new Pattern;
    if (new_pattern == NULL)
        return(0);

    if (! new_pattern->Set(directory, pattern) ||
        ! Append(new_pattern))
    {
        delete new_pattern;
        return(0);
    }

    return(1);
}

//-----------------------------//
// PatternList::NewStableFiles //
//-----------------------------//

int
PatternList::NewStableFiles(
    FileList*  new_file_list,
    FileList*  done_file_list)
{
    for (Pattern* pattern = GetHead(); pattern != NULL; pattern = GetNext())
    {
        if (! pattern->NewStableFiles(new_file_list, done_file_list,
            _maturity))
        {
            return(0);
        }
    }
    return(1);
}

#endif
