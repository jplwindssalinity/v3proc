//=========================================================
// Copyright  (C)1995, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.6   20 Apr 1998 15:20:14   sally
// change List to EAList
// 
//    Rev 1.5   14 Apr 1998 16:40:44   sally
// move back to EA's old list
// 
//    Rev 1.4   06 Apr 1998 16:29:08   sally
// merged with SVT
// 
//    Rev 1.3   03 Mar 1998 13:28:02   sally
// support generic type file list
// 
//    Rev 1.2   26 Feb 1998 10:01:34   sally
// took out obsolete constructor
// 
//    Rev 1.1   17 Feb 1998 14:46:24   sally
// add Range()
// 
//    Rev 1.0   04 Feb 1998 14:17:22   daffer
// Initial checking
// Revision 1.2  1998/01/30 22:28:44  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

#ifndef TLMFILELIST_H
#define TLMFILELIST_H

static const char rcs_id_TlmFileList_h[] = "@(#) $Header$";

#include "EAList.h"
#include "TlmHdfFile.h"
#include "Parameter.h"
#include "Itime.h"

//--------------------------------------------------------------
// TlmFileList is a pseudo-file class which manages a set of
// TlmHdfFiles.  The set of TlmHdfFiles is specified by a string of
// colon/space separated filenames and a source id
//--------------------------------------------------------------

class TlmFileList : public SortedList<TlmHdfFile>
{
public:
    enum StatusE
    {
        OK,
        INVALID_SOURCE_ID,
        ERROR_CREATING_TLM_FILE,
        NO_MORE_FILES,
        ERROR_RANGING,
        FILE_NOT_IN_LIST
    };

    TlmFileList(SourceIdE       tlm_type,
                const char*     tlm_filenames,  // ':' or ' ' separated
                StatusE&        returnStatus,              // OUT
                const Itime     start_time = INVALID_TIME, // IN
                const Itime     end_time = INVALID_TIME);  // IN

    TlmFileList();  // default constructor, don't have tlm type
                    // i.e. can have mixed types

    virtual ~TlmFileList();

    StatusE      GetStatus() const { return (_status); };
    StatusE      NextFile();
    StatusE      Range(FILE* ofp);

    StatusE      AddFileSorted(
                       const SourceIdE tlm_type,
                       const Itime     start_time,
                       const Itime     end_time,
                       const char*     filePath);

    StatusE      DeleteFile(const char*     filePath);
    void         DeleteAllFiles(void);
    IotBoolean   FileInList(const char*     filePath);

protected:
    StatusE      _AddFileIfNeeded(
                       const SourceIdE tlm_type,
                       const Itime     start_time,
                       const Itime     end_time,
                       const char*     directory,
                       const char*     filename);

    void         _InsertSortedFile(TlmHdfFile &newFile);

    StatusE      _status;
    TlmHdfFile*  _currentFile;
    SourceIdE    _tlmType;
};

#endif //TLMFILELIST_H
