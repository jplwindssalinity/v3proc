//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.22   07 Oct 1999 14:00:22   sally
// added L2Ahr file type
// 
//    Rev 1.21   27 Aug 1999 14:49:16   sally
// need to change status to OK if the tlm file list is not empty in constructor
// 
//    Rev 1.20   20 Aug 1999 14:12:18   sally
// ignore bad HDF files and continue processing
// 
//    Rev 1.19   11 Aug 1999 13:23:10   sally
// fix NO_MORE_DATA logic
// 
//    Rev 1.18   04 Aug 1999 11:07:40   sally
// need to get around HDF's maximum of 32 files
// 
//    Rev 1.17   21 Jul 1999 11:11:02   sally
// need to check error and pass it on
// 
//    Rev 1.16   25 May 1999 14:06:06   sally
// add L2Ax for Bryan Stiles
// 
//    Rev 1.15   03 Nov 1998 16:02:06   sally
// adapt to Vdata
// 
//    Rev 1.14   13 Oct 1998 15:34:34   sally
// added L1B file
// 
//    Rev 1.13   10 Jun 1998 16:25:10   sally
// look for " " not ": "
// 
//    Rev 1.12   01 May 1998 14:47:52   sally
// added HK2 file
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
#include "Hk2File.h"
#include "L1AFile.h"
#include "L1BHdfFile.h"
#include "NoTimeTlmFile.h"

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
    char* lasts = 0;
    for (char* string = safe_strtok(tlm_filenames_copy, " ", &lasts);
            string; string = safe_strtok(0, " ", &lasts))
    {
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
                // errors are reported but ignored and continue
                if (*(dirEntry->d_name) != '.')
                    (void) _AddFileIfNeeded(tlm_type, startTime, endTime,
                        string, dirEntry->d_name);
            }
            closedir(dir);
        }
        else
        {
            //------------------
            // string is a file 
            //------------------
            // errors are reported but ignored and continue
            (void)_AddFileIfNeeded(tlm_type, startTime, endTime, 0, string);
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
    returnStatus = TlmFileList::OK;
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
    case SOURCE_HK2:
        file = new HK2File(fullname, returnStatus, startTime, endTime);
        break;
    case SOURCE_L1A:
    case SOURCE_L1AP:
    case SOURCE_L1A_DERIVED:
        file = new L1AFile(fullname, returnStatus, startTime, endTime);
        break;
    case SOURCE_L1B:
        file = new L1BHdfFile(fullname, returnStatus, startTime, endTime);
        break;
    case SOURCE_L2A:
    case SOURCE_L2Ax:
    case SOURCE_L2Ahr:
    case SOURCE_L2B:
        file = new NoTimeTlmFile(fullname, tlm_type, returnStatus);
        break;
    default:
        _status = INVALID_SOURCE_ID;
        return(_status);
        break;
    }

    if (file == 0)
        return (_status = ERROR_CREATING_TLM_FILE);

    HdfFile::StatusE fileStatus  = file->GetStatus();
    if (fileStatus != HdfFile::OK)
    {
            delete file;

        // this file is outside of the range, discard but return OK
        if (fileStatus == HdfFile::NO_MORE_DATA)
            return (_status = OK);
        else
            return (_status = ERROR_CREATING_TLM_FILE);
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
    if (newStartTime == INVALID_TIME)
    {
        // this file has no "time" parameter, just append
        Append(&newFile);
    }
    else
    {
        // this file has "time" parameter, insert in "time" order
        Itime itemStartTime;
        TlmHdfFile* fileInList;
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
    }

    return;

}//TlmFileList::_InsertSortedFile
