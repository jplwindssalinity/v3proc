//=========================================================//
// Copyright  (C)1996, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef TLMFILE_H
#define TLMFILE_H

static const char rcs_id_TlmFile_h[] =
    "@(#) $Id$";

#include "Parameter.h"
#include "Itime.h"

//-------------------------------------------------------------------
// TlmFile:
//  This is an abstract class used as a generic Telemetry File.
//-------------------------------------------------------------------

class TlmFile
{
public:
    enum { SEARCH_START, SEARCH_END };
    enum StatusE
    {
        //---------//
        // General //
        //---------//

        OK,
        ERROR_ALLOCATING_FILENAME,
        ERROR_SEEKING_TIMESEARCH,
        ERROR_ALLOCATING_TIMESEARCH,
        ERROR_READING_TIMESEARCH,
        ERROR_EXTRACTING_TIMESEARCH,
        UNKNOWN_CONDITION,
        NO_MORE_DATA,

        //-----------//
        // L1 errors //
        //-----------//

        ERROR_L1_OPENING_FILE,
        ERROR_L1_CLOSING_FILE,
        ERROR_L1_SEEKING_DATA_HDRS,
        ERROR_L1_SEEKING_DATA_REC,
        ERROR_L1_READING_DATA_HDRS,
        ERROR_L1_READING_DATA_REC,
        ERROR_L1_DATA_HDRS_OVERFLOW,
        ERROR_L1_EXTRACTING_DATA_REC_TIME,
        ERROR_L1_EXTRACTING_DATA_HDRS_TYPE,

        //-------------//
        // HKDT errors //
        //-------------//

        ERROR_HKDT_OPENING_FILE,
        ERROR_HKDT_CLOSING_FILE,
        ERROR_HKDT_EXTRACTING_FH_REC_AMT,
        ERROR_HKDT_EXTRACTING_FH_REC_LEN,
        ERROR_HKDT_FH_DATA_BLK_HDR_SIZE,
        ERROR_HKDT_ALLOCATING_DATA_BLKS,
        ERROR_HKDT_DBH_EXTRACTING_REC_AMT,
        ERROR_HKDT_DBH_EXTRACTING_REC_LEN,
        ERROR_HKDT_DBH_EXTRACTING_DWELL_FLAG,
        ERROR_HKDT_DBH_NORMAL_DATA_REC_SIZE,
        ERROR_HKDT_DBH_DWELL_DATA_REC_SIZE,
        ERROR_HKDT_DBH_DWELL_FLAG,
        ERROR_HKDT_DBH_EXTRACTING_START_TIME,
        ERROR_HKDT_DBH_CONVERTING_START_TIME,
        ERROR_HKDT_SEEKING_FILE_HDR,
        ERROR_HKDT_READING_FILE_HDR,
        ERROR_HKDT_SEEKING_DATA_BLK_HDR,
        ERROR_HKDT_SEEKING_DATA_BLK,
        ERROR_HKDT_READING_DATA_BLK_HDR,
        ERROR_HKDT_READING_DATA_REC
    };

    enum ContentsE
    {
        NOTHING,
        L1_DATA_HDRS, L1_DATA_REC,
        HKDT_FILE_HDR, HKDT_DATA_BLK_HDR, HKDT_DATA_REC
    };

    TlmFile(const char* tlm_filename);
    ~TlmFile();

    virtual ContentsE   GetDataRec(char* data_rec) = 0;
    virtual ContentsE   GetDataHdrs(char* data_hdrs);
    virtual ContentsE   GetSomething(char* something) = 0;
    virtual int         IsAnyData() = 0;
    virtual int         GetDataRecLen(void) = 0;
    virtual int         GetMaxDataRecLen(void) = 0;
    virtual StatusE     Range(FILE* ofp)= 0;
    inline Itime        GetFileTime() { return(_fileTime); };
    inline StatusE      GetStatus() { return(_status); };
    inline void         ClearStatus() { _status = TlmFile::OK; };

protected:
    StatusE     _DupFilename(const char* filename);
    StatusE     _SetOffsets(const int data_rec_size, const off_t first_offset,
                    const off_t last_offset, ExtractFunc time_func,
                    const Itime start_time, const Itime end_time,
                    off_t* first_used_offset, off_t* last_used_offset,
                    Itime* first_used_time, Itime* last_used_time);

    StatusE     _GetTime(off_t offset, int size, char* data_rec_buffer,
                    ExtractFunc time_func, Itime* time);
    virtual StatusE     _SetFileTime() = 0;

    int         _fd;
    StatusE     _status;
    char*       _filename;
    Itime       _fileTime;

private:
    off_t       _BinarySearch(const Itime target_time, int size,
                    char* data_rec, off_t first_offset, off_t last_offset,
                    ExtractFunc time_func, int target_type, Itime* found_time);
};

#endif
