//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.11   20 Apr 1998 15:19:52   sally
// change List to EAList
// 
//    Rev 1.10   20 Apr 1998 10:22:58   sally
// change for WindSwatch
// 
//    Rev 1.9   17 Apr 1998 16:51:20   sally
// add L2A and L2B file formats
// 
//    Rev 1.8   14 Apr 1998 16:40:50   sally
// move back to EA's old list
// 
//    Rev 1.7   06 Apr 1998 16:29:06   sally
// merged with SVT
// 
//    Rev 1.6   27 Mar 1998 10:00:50   sally
// added L1A Derived data
// 
//    Rev 1.5   04 Mar 1998 14:32:04   sally
// change range dialog
// 
//    Rev 1.4   03 Mar 1998 13:28:30   sally
// support generic type file list
// 
//    Rev 1.3   26 Feb 1998 10:02:12   sally
// took out obsolete constructor
// 
//    Rev 1.2   20 Feb 1998 10:59:20   sally
// L1 to L1A
// 
//    Rev 1.1   17 Feb 1998 14:46:16   sally
// add Range()
// 
//    Rev 1.0   04 Feb 1998 14:17:20   daffer
// Initial checking
// Revision 1.3  1998/02/03 00:12:33  sally
// change NRT to L1AP
//
// Revision 1.2  1998/01/30 22:29:24  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "TlmFileList.h"
#include "L1AFile.h"
#include "NoTimeTlmFile.h"

#if 0
#include "HkdtFile.h"
#endif

static const char TlmFileList_c_rcs_id[] =
    "@(#) $Header$";

//=====================
// TlmFileList methods 
//=====================

TlmFileList::TlmFileList(
SourceIdE       tlm_type,
const char*     tlm_filenames,  // ':' or ' ' separated
StatusE&        returnStatus,   // OUT
const Itime     startTime,      // IN
const Itime     endTime)        // IN
:   _status(OK), _currentFile(0), _tlmType(tlm_type)
{
    //----------------------------------------------------
    // step through each file in tlm_filenames and add it 
    //----------------------------------------------------

    char* tlm_filenames_copy = strdup(tlm_filenames);
    char* ptr = tlm_filenames_copy;
    char* string;
    while ((string = strtok(ptr, ": ")))
    {
        ptr = NULL;     // for repeat calls to strtok

        DIR* dir = opendir(string);
        if (dir)
        {
            //-----------------------
            // string is a directory 
            //-----------------------

            struct dirent *dirEntry;
            while ((dirEntry = readdir(dir)))
            {
                // add each file in the directory
                if (*(dirEntry->d_name) != '.')
                    _AddFileIfNeeded(tlm_type, startTime, endTime,
                        string, dirEntry->d_name);
            }
            closedir(dir);
        }
        else
        {
            //------------------
            // string is a file 
            //------------------

            _AddFileIfNeeded(tlm_type, startTime, endTime, 0, string);
        }
    }
    free(tlm_filenames_copy);

    // go to the first file
    _currentFile = GetHead();
    if (_currentFile == NULL)
    {
        returnStatus = _status = TlmFileList::NO_MORE_FILES;
        return;
    }
    returnStatus = _status;
    return;
} // TlmFileList::TlmFileList

//--------------------------------------------------------
// mix and match sources are allowed here
//--------------------------------------------------------
TlmFileList::TlmFileList()
:   SortedList<TlmHdfFile>(),
   _status(OK), _currentFile(0), _tlmType(SOURCE_UNKNOWN)
{
} // TlmFileList::TlmFileList

TlmFileList::~TlmFileList()
{
    return;
}

//----------
// NextFile 
//----------

TlmFileList::StatusE
TlmFileList::NextFile()
{
    _currentFile = GetNext();
    if (! _currentFile)
        return(_status = TlmFileList::NO_MORE_FILES);
    return(_status);
}

//-------//
// Range //
//-------//

TlmFileList::StatusE
TlmFileList::Range(
FILE*   ofp)
{
    TlmHdfFile* fileInList;
    for (fileInList = GetHead(); fileInList != NULL; fileInList = GetNext())
    {
        if (fileInList->Range(ofp) != HdfFile::OK)
            return(_status = TlmFileList::ERROR_RANGING);
        fprintf(ofp, "\n");
    }
    return(_status = OK);

}//TlmFileList::Range

//--------------------------------------------------------
// Add File in a sorted list
// tlm_type must match _tlmType, unless _tlmType is unknown
//--------------------------------------------------------
TlmFileList::StatusE
TlmFileList::AddFileSorted(
const SourceIdE tlm_type,
const Itime     start_time,
const Itime     end_time,
const char*     filePath)
{
    if (_tlmType != SOURCE_UNKNOWN  && tlm_type != _tlmType)
        return(INVALID_SOURCE_ID);

    return(_AddFileIfNeeded(tlm_type, start_time, end_time, 0, filePath));

} // TlmFileList::AddFileSorted

//--------------------------------------------------------
// Delete a file from list
//--------------------------------------------------------
TlmFileList::StatusE
TlmFileList::DeleteFile(
const char*     filePath)
{
    TlmHdfFile* fileInList;
    for (fileInList = GetHead(); fileInList != NULL; fileInList = GetNext())
    {
        if (strcmp(fileInList->GetFileName(), filePath) == 0)
        {
            delete (RemoveCurrent());
            return(OK);
        }
    }
    return(FILE_NOT_IN_LIST);

} //TlmFileList::DeleteFile

void
TlmFileList::DeleteAllFiles(void)
{
    GetHead();
    TlmHdfFile* fileInList;
    while ((fileInList=RemoveCurrent()) != NULL)
        delete fileInList;

} // TlmFileList::DeleteAllFiles

//--------------------------------------------------------
// Delete a file from list
//--------------------------------------------------------
IotBoolean
TlmFileList::FileInList(
const char*     filePath)
{
    TlmHdfFile* fileInList;
    for (fileInList = GetHead(); fileInList != NULL; fileInList = GetNext())
    {
        if (strcmp(fileInList->GetFileName(), filePath) == 0)
            return 1;
    }
    return 0;

} // TlmFileList::FileInList

//------------------
// _AddFileIfNeeded 
//------------------

TlmFileList::StatusE
TlmFileList::_AddFileIfNeeded(
const SourceIdE tlm_type,
const Itime     startTime,
const Itime     endTime,
const char*     directory,
const char*     filename)
{
    char fullname[1024];
    if (directory)
        sprintf(fullname, "%s/%s", directory, filename);
    else
        sprintf(fullname, "%s", filename);
    TlmHdfFile* file = 0;
    TlmHdfFile::StatusE     returnStatus;
    switch(tlm_type)
    {
#if 0
    case SOURCE_HK2:
        file = new HkdtFile(fullname, startTime, endTime, 1);
        break;
#endif
    case SOURCE_L1A:
    case SOURCE_L1AP:
    case SOURCE_L1A_DERIVED:
        file = new L1AFile(fullname, returnStatus, startTime, endTime);
        break;
    case SOURCE_L2A:
    case SOURCE_L2B:
        file = new NoTimeTlmFile(fullname, returnStatus);
        break;
    default:
        _status = INVALID_SOURCE_ID;
        return(_status);
        break;
    }

    if (file == 0)
        return (_status = ERROR_CREATING_TLM_FILE);

    if (file->GetStatus() != HdfFile::OK)
    {
        delete file;
        return (_status);
    }

    _InsertSortedFile(*file);
    return (_status);

}//TlmFileList::_AddFileIfNeeded

//-------------------
// _InsertSortedFile 
//-------------------

void
TlmFileList::_InsertSortedFile(
TlmHdfFile &newFile)
{
    Itime newStartTime = newFile.GetFirstDataTime();
    TlmHdfFile* fileInList;
    Itime itemStartTime;
    for (fileInList = GetHead(); fileInList != NULL; fileInList = GetNext())
    {
        // if the file in the file is older than the new one,
        // insert the new file in front of the old file
        itemStartTime = fileInList->GetFirstDataTime();
        if (newStartTime < itemStartTime)
        {
            InsertBefore(&newFile);
            return;
        }
    }

    // every file is older than the new one, just append at the end
    Append(&newFile);

    return;
}//TlmFileList::_InsertSortedFile