//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

static const char rcs_id_morefiles_c[] =
    "@(#) $Id$";

#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include "MoreFiles.h"
#include "List.h"

//==========//
// FilePack //
//==========//

FilePack::FilePack()
{
    return;
}

FilePack::~FilePack()
{
    if (filename)
        free(filename);
    if (mode)
        free(mode);
    if (fp)
        fclose(fp);

    return;
}

//------------------------------//
// FilePack::SetFilenameAndMode //
//------------------------------//

int
FilePack::SetFilenameAndMode(
    const char*  file_name,
    const char*  file_mode)
{
    int length = strlen(file_name);
    filename = (char *)malloc(length + 1);
    if (filename == NULL)
        return(0);
    strcpy(filename, file_name);

    length = strlen(file_mode);
    mode = (char *)malloc(length + 1);
    if (mode == NULL)
        return(0);
    strcpy(mode, file_mode);

    return(1);
}

//===========//
// MoreFiles //
//===========//

MoreFiles::MoreFiles()
{
    return;
}

MoreFiles::~MoreFiles()
{
    return;
}

//-----------------//
// MoreFiles::Open //
//-----------------//
// opens a file, returns a token (NULL on error)

FileToken
MoreFiles::Open(
    const char*  filename,
    const char*  mode)
{
    //-------------------//
    // create a filepack //
    //-------------------//

    FilePack* filepack = new FilePack();
    if (filepack == NULL)
        return(NULL);

    if (! filepack->SetFilenameAndMode(filename, mode))
    {
        delete filepack;
        return(NULL);
    }

    //---------------//
    // open the file //
    //---------------//

    if (! _OpenHold(filepack))
    {
        delete filepack;
        return(NULL);
    }

    //-------------------------//
    // add it to the full list //
    //-------------------------//

    if (! _fullList.Append(filepack))
    {
        delete filepack;
        return(NULL);
    }

    //--------------//
    // return token //
    //--------------//

    return(filepack);
}

//------------------//
// MoreFiles::Close //
//------------------//

int
MoreFiles::Close(
    FileToken  token)
{
    if (token->fp == NULL)
        return(1);

    for (FilePack* filepack = _openList.GetHead(); filepack;
        filepack = _openList.GetNext())
    {
        if (filepack == token)
        {
            fclose(token->fp);
            _openList.RemoveCurrent();
            return(1);
        }
    }
    return(0);
}

//----------------------//
// MoreFiles::TokenToFp //
//----------------------//
// convert a token to a file pointer

FILE*
MoreFiles::TokenToFp(
    FileToken  token)
{
    if (token->fp == NULL)
    {
        // file is on hold, re-open it for appending
        if (! _OpenHold(token))
            return(NULL);
    }
    return(token->fp);
}

//----------------------//
// MoreFiles::_OpenHold //
//----------------------//

int
MoreFiles::_OpenHold(
    FilePack*  filepack)
{
    //---------------//
    // open the file //
    //---------------//

    filepack->fp = NULL;
    for (int i = 0; i < 2; i++)
    {
        // append forced (assuming writing)
        filepack->fp = fopen(filepack->filename, "a");
        if (filepack->fp != NULL)
        {
            if (! _openList.Append(filepack))
            {
                return(0);
            }
            return(1);
        }

        // put a file on hold and try again
        if (! Hold())
            return(NULL);
    }
    return(0);
}

//-----------------//
// MoreFiles::Hold //
//-----------------//

int
MoreFiles::Hold()
{
    // remove a file from the open list
    FilePack* filepack = _openList.GetHead();
    if (filepack == NULL)
        return(0);

    _openList.RemoveCurrent();

    // close it
    fclose(filepack->fp);
    filepack->fp = NULL;

    return(1);
}
