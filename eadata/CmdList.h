//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.8   19 Aug 1998 13:14:26   daffer
// More CMDLP/Effects work.
// 
//    Rev 1.7   22 May 1998 16:29:36   daffer
// Added/modified code for cmdlp/effect processing
// 
//    Rev 1.6   20 Apr 1998 15:18:04   sally
// change List to EAList
// 
//    Rev 1.5   14 Apr 1998 16:40:30   sally
// move back to EA's old list
// 
//    Rev 1.4   06 Apr 1998 16:26:36   sally
// merged with SVT
// 
//    Rev 1.3   17 Mar 1998 14:41:02   sally
// changed for REQQ
// 
//    Rev 1.2   09 Mar 1998 16:34:02   sally
// adapt to the new REQI format
// 
//    Rev 1.1   20 Feb 1998 10:55:54   sally
// L1 to L1A
// 
//    Rev 1.0   04 Feb 1998 14:14:52   daffer
// Initial checking
// Revision 1.3  1998/02/03 00:12:33  sally
// change NRT to L1AP
//
// Revision 1.2  1998/01/30 22:28:09  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

#ifndef CMD_LIST_H
#define CMD_LIST_H

static const char rcsid_cmdlist_h[] =
    "@(#) $Header$";

#include <stdio.h>

#include "EAList.h"
#include "Command.h"
#include "CmdEffect.h"
#include "Eqx.h"
#include "Itime.h"
#include "Parameter.h"

#define THRESHOLD_APPROX_TIME   Itime(60,0)

class CmdList : public SortedList<Command>
{
public:
    enum StatusE
    {
        OK,
        ERROR_OPENING_FILE,
        ERROR_CREATING_CMD,
        ERROR_READING_CMD,
        ERROR_APPENDING_CMD,
        ERROR_WRITING_CMD,
        ERROR_READING_DATAFILE
    };

    CmdList();
    CmdList(        const char* filename,
                    const Itime start_time = INVALID_TIME,
                    const Itime end_time = INVALID_TIME);
    virtual ~CmdList();

    StatusE         Read(   const char* filename,
                            const Itime start_time = INVALID_TIME,
                            const Itime end_time = INVALID_TIME);
    StatusE         ReadReqi(const char* reqiFilename); 
    StatusE         WriteReqi(const char* reqiFilename); 
    StatusE         Write(const char* filename);
    StatusE         WriteForHumans(FILE *ofp, Itime start_time,
                        Itime end_time);
    StatusE         ExpandBC();

    StatusE         SetCmdEffects();
    StatusE         SetCmdExpectedTimes(EqxList* eqxList);
    StatusE         AddSortedCmds(CmdList* cmds);
    Command*        FindNearestMatchable(Command* effect, int* index);
    Command*        MissingL1AVerify(int* index);
    Command*        MissingHk2Verify(int* index);
    Command*        MissingL1ApVerify(int* index);

    /*
    StatusE         Update(CmdList* detectedCmdList, EqxList* eqxList);
    StatusE         ApplyCmds(CmdList* detectedCmds);
    StatusE         ReportAnomalies(FILE* ofp, Itime start_time,
                        Itime end_time, int* anomaly_count);
    */
    StatusE         GetStatus() { return _status; };

protected:
    // Command*        _MatchingCmd(Command* cmd);

    StatusE         _status;
};

#endif //CMD_LIST_H
