//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
// CM Log
// $Log$
// 
//    Rev 1.4   21 Sep 1998 15:06:14   sally
// added Qpa
// 
//    Rev 1.3   01 Sep 1998 16:39:12   sally
// change operator== to Match in order to pass outFP
// 
//    Rev 1.2   28 Aug 1998 16:30:48   sally
// use REQA's file number to match REQQ file 
// 
//    Rev 1.1   28 Aug 1998 14:53:42   sally
// make the ReqqRecordList a Sorted List
// 
//    Rev 1.0   28 Aug 1998 11:21:24   sally
// Initial revision.
// 
// $Date$
// $Revision$
// $Author$
//
//=========================================================

static const char rcs_id_Reqq_C[] =
    "@(#) $Header$";

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "ReqaqList.h"

#define REQQ_NUM_MIN_TIME_CHAR     11
#define REQQ_NUM_MAX_TIME_CHAR     19

ReqqRecord::ReqqRecord()
{
}

ReqqRecord::~ReqqRecord()
{
}

int
ReqqRecord::Match(
const ReqqRecord&     other,
FILE*                 outFP)
{
    outFP = (outFP == 0 ? stderr : outFP);
    //--------------------------------------------------------
    // assumption: one of them is REQQ and the other is REQA
    // find out which one is which
    // REQQ: either path and gamma are filled with "*", or
    //       time string is of format "YYYY-MM-DDT" (11 chars)
    //--------------------------------------------------------
    int firstIsREQQ = 0;
    if (strlen(_timeString) == REQQ_NUM_MIN_TIME_CHAR)
        firstIsREQQ = 1;
    else if (strcmp(_pathString, REQQ_NULL_PATH_STRING) == 0 &&
        strcmp(_gammaString, REQQ_NULL_GAMMA_STRING) == 0)
        firstIsREQQ = 1;

    int numCompTimeString;
    // first record (a) is a REQQ
    if (firstIsREQQ)
    {
        numCompTimeString = strlen(_timeString);
        if (numCompTimeString < REQQ_NUM_MIN_TIME_CHAR)
        {
            fprintf(outFP, "REQQ Command(%d): Time String is too short\n",
                                      _commandNumber);
            return 0;
        }
        if (strncmp(_timeString, other._timeString,
                           REQQ_NUM_MIN_TIME_CHAR) != 0)
        {
            fprintf(outFP,
                  "Command(%d): Time Strings are different: %s and %s\n",
                          _commandNumber, _timeString, other._timeString);
            return 0;
        }
        // if path (and gamma) is specified in REQQ, then compare it with REQA
        if (strcmp(_pathString, REQQ_NULL_PATH_STRING) != 0)
        {
            if (strcmp(_pathString, other._pathString) != 0)
            {
                fprintf(outFP,
                    "Command(%d): Path Strings are different: %s and %s\n",
                            _commandNumber, _pathString, other._pathString);
                return 0;
            }
            if (strcmp(_gammaString, other._gammaString) != 0)
            {
                fprintf(outFP,
                    "Command(%d): Gamma Strings are different: %s and %s\n",
                            _commandNumber, _gammaString, other._gammaString);
                return 0;
            }
        }
    }
    else
    {
    // second record (b) is a REQQ
        numCompTimeString = strlen(other._timeString);
        if (numCompTimeString < REQQ_NUM_MIN_TIME_CHAR)
        {
            fprintf(outFP, "REQQ Command(%d): Time String is too short\n",
                                      other._commandNumber);
            return 0;
        }
        if (strncmp(_timeString, other._timeString, 
                              REQQ_NUM_MIN_TIME_CHAR) != 0)
        {
            fprintf(outFP,
                  "Command(%d): Time Strings are different: %s and %s\n",
                          _commandNumber, _timeString, other._timeString);
            return 0;
        }
        if (strcmp(other._pathString, REQQ_NULL_PATH_STRING) != 0)
        {
            if (strcmp(_pathString, other._pathString) != 0)
            {
                fprintf(outFP,
                    "Command(%d): Path Strings are different: %s and %s\n",
                            _commandNumber, _pathString, other._pathString);
                return 0;
            }
            if (strcmp(_gammaString, other._gammaString) != 0)
            {
                fprintf(outFP,
                    "Command(%d): Gamma Strings are different: %s and %s\n",
                            _commandNumber, _gammaString, other._gammaString);
                return 0;
            }
        }
    }

    if (_commandNumber != other._commandNumber)
    {
        fprintf(outFP, "Command Numbers are different: %d and %d\n",
                      _commandNumber, other._commandNumber);
        return 0;
    }
    if (strcmp(_sensorName, other._sensorName) != 0)
    {
        fprintf(outFP, "Command(%d): Sensor Names are different: %s and %s\n",
                      _commandNumber, _sensorName, other._sensorName);
        return 0;
    }
    if (strcmp(_mnemonic, other._mnemonic) != 0)
    {
        fprintf(outFP, "Command(%d): Mnemonics are different: %s and %s\n",
                      _commandNumber, _mnemonic, other._mnemonic);
        return 0;
    }
    if (strcmp(_param1String, other._param1String) != 0)
    {
        fprintf(outFP,
            "Command(%d): First Parameter Words are different: %s and %s\n",
                      _commandNumber, _param1String, other._param1String);
        return 0;
    }
    if (strcmp(_param2String, other._param2String) != 0)
    {
        fprintf(outFP,
            "Command(%d): Second Parameter Words are different: %s and %s\n",
                      _commandNumber, _param2String, other._param2String);
        return 0;
    }
    if (strcmp(_checksumString, other._checksumString) != 0)
    {
        fprintf(outFP, "Command(%d): Checksums are different: %s and %s\n",
                      _commandNumber, _checksumString, other._checksumString);
        return 0;
    }
    return(1);
}



ReqqRecordList::ReqqRecordList()
:   SortedList<ReqqRecord>(),
    _fileNumber(0), _fileVersion(' ')
{
}

ReqqRecordList::~ReqqRecordList()
{
}

//----------------------------------------------------------
// read from a REQQ file, and create a list of REQQ records
//----------------------------------------------------------
ReqqStatusE 
ReqqRecordList::Read(
const char*     reqqFile,
FILE*           outFP)
{
    outFP = (outFP == 0 ? stderr : outFP);
    // open the REQQ file
    FILE* reqqFP = fopen(reqqFile, "r");
    if (reqqFP == NULL)
    {
        fprintf(outFP, "Failed to open REQQ file: %s\n", reqqFile);
        return(REQQ_OPEN_FAILURE);
    }

    // read the header line
    ReqqStatusE rc = _ReadHeader(reqqFP, outFP);
    if (rc != REQQ_OK) return rc;

    // keep reading in the the records, until EOF or error
    while((rc = _ReadRecord(reqqFP, outFP)) == REQQ_OK);
    if (rc != REQQ_EOF) return rc;
    return(REQQ_OK);

} // ReqqRecordList::Read

//----------------------------------------------------------
// read a header line from REQQ file pointer
//----------------------------------------------------------
ReqqStatusE 
ReqqRecordList::_ReadHeader(
FILE*     fp,
FILE*     )
{
    rewind(fp);   // rewind to make sure it is a the beginning
 
    char headString[BIG_SIZE];
    char* ptr = fgets(headString, BIG_SIZE - 1, fp);
    if (ptr != headString) return(REQQ_ERROR);

    // read the file name, number and version character
    int rc = sscanf(headString, " QS_REQQ%06d%c", &_fileNumber, &_fileVersion);
    if (rc == EOF)
        return REQQ_EMPTY_FILE;
    else if (rc < 1)
        return(REQQ_ERROR);
    else
    return(REQQ_OK);

} // ReqqRecordList::_ReadHeader

//----------------------------------------------------------
// read a record line from REQQ file pointer
//----------------------------------------------------------
ReqqStatusE 
ReqqRecordList::_ReadRecord(
FILE*     fp,
FILE*     )
{
    char recordString[BIG_SIZE];
    char* ptr = fgets(recordString, BIG_SIZE - 1, fp);
    if (ptr != recordString)
    {
        if (feof(fp))
            return(REQQ_EOF);
        else
            return(REQQ_ERROR);
    }

    ReqqRecord* newRecord = new ReqqRecord;
    int rc = sscanf(recordString, " JPL%d %s %s %s %s %s %s %s %s",
                             &(newRecord->_commandNumber),
                             newRecord->_sensorName, 
                             newRecord->_mnemonic,
                             newRecord->_timeString, 
                             newRecord->_pathString, 
                             newRecord->_gammaString,
                             newRecord->_param1String, 
                             newRecord->_param2String, 
                             newRecord->_checksumString);
    if (rc != 9)
        return REQQ_ERROR;

    AddSorted(newRecord);
    return REQQ_OK;

} // ReqqRecordList::_ReadRecord

ReqaRecordList::ReqaRecordList()
: ReqqRecordList()
{
} // ReqaRecordList::ReqaRecordList

ReqaRecordList::~ReqaRecordList()
{
} // ReqaRecordList::~ReqaRecordList

//----------------------------------------------------------
// read a header line from REQA file pointer
//----------------------------------------------------------
ReqqStatusE
ReqaRecordList::_ReadHeader(
FILE*     fp,
FILE*     )
{
    rewind(fp);   // rewind to make sure it is a the beginning
 
    char headString[BIG_SIZE];
    char* ptr = fgets(headString, BIG_SIZE - 1, fp);
    if (ptr != headString) return(REQQ_ERROR);

    // read the file name, number and version character
    int rc = sscanf(headString, " QS_REQA%06d%c", &_fileNumber, &_fileVersion);
    if (rc == EOF)
        return REQQ_EMPTY_FILE;
    else if (rc < 1)
        return(REQQ_ERROR);
    else
    return(REQQ_OK);
 
} // ReqaRecordList::_ReadHeader

//----------------------------------------------------------
// read a record line from REQQ file pointer
//----------------------------------------------------------
ReqqStatusE 
ReqaRecordList::_ReadRecord(
FILE*     fp,
FILE*     outFP)
{
    char recordString[BIG_SIZE];
    char* ptr = fgets(recordString, BIG_SIZE - 1, fp);
    if (ptr != recordString)
    {
        if (feof(fp))
            return(REQQ_EOF);
        else
            return(REQQ_ERROR);
    }

    ReqqRecord* newRecord = new ReqqRecord;
    int rc = sscanf(recordString, " JPL%d %s %s %s %s %s %s %s %s",
                             &(newRecord->_commandNumber),
                             newRecord->_sensorName, 
                             newRecord->_mnemonic,
                             newRecord->_timeString, 
                             newRecord->_pathString, 
                             newRecord->_gammaString,
                             newRecord->_param1String, 
                             newRecord->_param2String, 
                             newRecord->_checksumString);
    if (rc != 9)
    {
        delete newRecord;
        return REQQ_ERROR;
    }

    if (strlen(newRecord->_timeString) !=  REQQ_NUM_MAX_TIME_CHAR)
    {
        fprintf(outFP, "REQA: Command (%d): Time String is too short\n",
                                      newRecord->_commandNumber);
        delete newRecord;
        return REQQ_ERROR;
    }
    int path;
    if (sscanf(newRecord->_pathString, "%d", &path) != 1)
    {
        fprintf(outFP, "REQA: Command (%d): Bad Path\n",
                                      newRecord->_commandNumber);
        delete newRecord;
        return REQQ_ERROR;
    }
    float gamma;
    if (sscanf(newRecord->_gammaString, "%f", &gamma) != 1)
    {
        fprintf(outFP, "REQA: Command (%d): Bad Gamma\n",
                                      newRecord->_commandNumber);
        delete newRecord;
        return REQQ_ERROR;
    }

    AddSorted(newRecord);
    return REQQ_OK;

} // ReqaRecordList::_ReadRecord


int
ReqaRecordList::Match(
const ReqqRecordList&    reqq,
FILE*                    outFP)
{
    outFP = (outFP == 0 ? stderr : outFP);

    if (_fileNumber != reqq._fileNumber)
    {
        fprintf(outFP, "File numbers are different: %d and %d\n",
                          _fileNumber, reqq._fileNumber);
        return 0;
    }
    if (_fileVersion != reqq._fileVersion)
    {
        fprintf(outFP, "File versions are different: %d and %d\n",
                          _fileVersion, reqq._fileVersion);
        return 0;
    }

    // go through all the records in the files
    int isEqual = 1;
    ReqqRecordList* aPtr = (ReqqRecordList*)this;
    ReqqRecordList* bPtr = (ReqqRecordList*)&reqq;
    ReqqRecord*  aRecord = aPtr->GetHead();
    ReqqRecord*  bRecord = bPtr->GetHead();
    while (aRecord != 0 && bRecord != 0)
    {
        if (*aRecord > *bRecord)
        {
            fprintf(outFP, "Nothing matches Command %d\n",
                                        bRecord->_commandNumber);
            bRecord = bPtr->GetNext();
            continue;
        }
        else if (*aRecord < *bRecord)
        {
            fprintf(outFP, "Nothing matches Command %d\n",
                                        aRecord->_commandNumber);
            aRecord = aPtr->GetNext();
            continue;
        }
        if ( ! aRecord->Match(*bRecord, outFP)) isEqual = 0;
        aRecord = aPtr->GetNext();
        bRecord = bPtr->GetNext();
    }

    // REQA and REQQ must have same number of record entries
    if (aRecord != 0 || bRecord != 0) isEqual = 0;
    while (aRecord != 0)
    {
        fprintf(outFP, "Nothing matches Command %d\n",
                                        aRecord->_commandNumber);
        aRecord = aPtr->GetNext();
    }
    while (bRecord != 0)
    {
        fprintf(outFP, "Nothing matches Command %d\n",
                                        bRecord->_commandNumber);
        bRecord = bPtr->GetNext();
    }

    return isEqual;

} // ReqqRecordList::Match
