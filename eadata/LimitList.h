#ifndef LIMITLIST_H
#define LIMITLIST_H

#include <stdio.h>
#include "Parameter.h"
#include "EAList.h"
#include "LimitChecker.h"

static const char rcsidLimitList_h[]="(@)# $Id$";

class TlmHdfFile;

//*******************************************************************
// LimitList:
//          This class reads in a limits file.
//          It creates an array of LimitChecker class pointer to hold the
//          entries.
//
//          The format of the limits file:
//          "ParamName" "UnitName" CautionLow CautionHigh AlarmLow
//                  (same line) AlarmHigh Enable/Disable mode [mode...]
//*******************************************************************
class LimitList : public EAList<LimitChecker>
{
public:
    enum StatusE
    {
        ERROR_OPENING_FOR_READING,
        ERROR_OPENING_FOR_WRITING,
        ERROR_OPEN_DATASETS,
        ERROR_CLOSE_DATASETS,
        BAD_PARAM_IN_LIMIT_CHECKER,
        END_OF_LIMIT_FILE,
        TIME_ERROR,
        READ_ERROR,
        CHECK_FRAME_FAILED,
        WRITE_TO_LIMIT_FILE_FAILED,
        WRITE_TO_LOG_FILE_FAILED,
        GOOD
    };

    enum WorstE
    {
        OK,
        CAUTION,
        ACTION
    };

    LimitList(  SourceIdE       source,         // HK, L1A, ...
                const char*     limitFilename,  // limit file name
                const FILE*     logFP,          // error log file pointer
                const int       keepDisabled = 1);  // 0 removes disabled
    virtual ~LimitList();

    StatusE             OpenParamDataSets(TlmHdfFile*);
    StatusE             CloseParamDataSets(TlmHdfFile*);

                        // check file status
    StatusE             GetStatus(void) { return _status; }
    const char*         GetFilename(void) { return _limitFilename; }
    WorstE              GetWorst(void) { return _worst; }
    StatusE             CheckFrame(TlmHdfFile* tlmFile, int32 startIndex);

    StatusE             WriteLimitText(void);

    LimitChecker*       GetLimitChecker(SourceIdE source,
                                        char* param, char* unit);

    LimitChecker*       DeleteLimitChecker(LimitChecker*);

protected:
    static LimitChecker*        L1ATxtToLim(LimitList&, FILE* limitFP);
#if 0
    static LimitChecker*        HkTxtToLim(LimitList&, FILE* limitFP);
#endif

    SourceIdE                   _source;
    char                        _limitFilename[BUFSIZ];
    FILE*                       _limitFP;
    FILE*                       _logFP;
    StatusE                     _status;
    WorstE                      _worst;
    LimitStatePair*             _limitState;

    LimitChecker*               (*_readOneParamLimit) (LimitList&, FILE*);

};//LimitList

#endif //LIMITLIST_H
