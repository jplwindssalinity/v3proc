//=========================================================
// Copyright  (C)1996, California Institute of Technology.
// U.S. Government sponsorship under
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.0   02 Jun 1999 16:21:28   sally
// Initial revision.
//
// $Date$
// $Revision$
// $Author$
//
//=========================================================

#include <assert.h>
#include <string.h>

#include "CommonDefs.h"
#include "Itime.h"
#include "LeapSecTable.h"

#define  EA_LEAP_SECOND_UPDATE_TIME      -999

const char* LeapSecTable::LeapSecTableStatusStrings[] =
{
    "Leap Second Table OK",
    "Openning Leap Second Table failed",
    "Reading Leap Second Table failed"
};


LeapSecTable::LeapSecTable(
const char*     leapFilename)
: _status(LEAP_TABLE_OK)
{
    FILE* fp = fopen(leapFilename, "r");
    if (fp == NULL)
    {
        _status = ERROR_OPEN_LEAP_TABLE;
        return;
    }

    char line[BIG_SIZE];
    char timeString[STRING_LEN];
    while (fgets(line, BIG_SIZE, fp) != NULL)
    {
        // skip comments lines (#|*) and blank lines
        if(line[0] == '#' || line[0] == '*' || line[0] == '\n')
            continue;

        LeapSecEntry* leapSecEntry = new LeapSecEntry();
        assert(leapSecEntry != 0);

        // read in the entries
        if (sscanf(line, "%s %lf %d", timeString,
                &(leapSecEntry->taiTimeSecond),
                &(leapSecEntry->adjustSecond)) != 3)
        {
            _status = ERROR_READ_LEAP_TABLE;
            return;
        }

        strncpy(leapSecEntry->l1TimeString, timeString, L1_TIME_LEN - 1);
        leapSecEntry->l1TimeString[L1_TIME_LEN - 1] = '\0';

        if (leapSecEntry->adjustSecond == EA_LEAP_SECOND_UPDATE_TIME)
            leapSecEntry->isUpdateTime = 1;

        Append(leapSecEntry);

    }
} // LeapSecTable::LeapSecTable

#if 0
int
GetDeltaSeconds(
double       taiLeapSeconds,      // IN
int&         deltaSeconds,        // OUT
int&         isUpdateTime,        // OUT
int&         updateTaiSeconds,    // OUT
char*        updateL1TimeString); // OUT if update
{
    for (LeapSecEntry* leapSecEntry = GetHead();
                           leapSecEntry; leapSecEntry = GetNext())
    {
        if (taiLeapSeconds >= leapSecEntry->taiTimeSecond)
        {
            deltaSeconds = leapSecEntry->adjustSecond;
            if (deltaSeconds == EA_LEAP_SECOND_UPDATE_TIME)
            {
                isUpdateTime = 1;
                updateTimeString = leapSecEntry->l1TimeString;
                updateTaiSeconds = leapSecEntry->taiTimeSecond;
            }
            else isUpdateTime = 0;
            return(1);
        }
    }

    return(0);

} // LeapSecTable::GetDeltaSeconds
#endif

int
LeapSecTable::ItimeToDeltaSeconds(
const Itime& itime,             // IN
int&         deltaSeconds,      // OUT
int&         isUpdateTime,      // OUT
char*        updateL1TimeString)// OUT if update
{
    double taiTime = itime.sec - ITIME_DEFAULT_SEC;
    int lastDeltaSeconds = 0;
    for (LeapSecEntry* leapSecEntry = GetHead();
                           leapSecEntry; leapSecEntry = GetNext())
    {
        if (taiTime >= leapSecEntry->taiTimeSecond)
        {
            if (deltaSeconds == EA_LEAP_SECOND_UPDATE_TIME)
            {
                deltaSeconds = lastDeltaSeconds;
                isUpdateTime = 1;
                updateL1TimeString = leapSecEntry->l1TimeString;
            }
            else
            {
                deltaSeconds = leapSecEntry->adjustSecond;
                isUpdateTime = 0;
            }
            return(1);
        }
        lastDeltaSeconds = leapSecEntry->adjustSecond;
    }

    return(0);

} // LeapSecTable::ItimeToDeltaSeconds

int
LeapSecTable::StringToDeltaSeconds(
const char*  l1TimeString,      // IN
int&         deltaSeconds,      // OUT
int&         isUpdateTime,      // OUT
double       taiTimeSecond)     // OUT if update
{
    for (LeapSecEntry* leapSecEntry = GetHead();
                           leapSecEntry; leapSecEntry = GetNext())
    {
        if (strcmp(l1TimeString, leapSecEntry->l1TimeString) >= 0)
        {
            deltaSeconds = leapSecEntry->adjustSecond;
            if (deltaSeconds == EA_LEAP_SECOND_UPDATE_TIME)
            {
                isUpdateTime = 1;
                taiTimeSecond = leapSecEntry->taiTimeSecond;
            }
            else isUpdateTime = 0;
            return(1);
        }
    }

    return(0);

} // LeapSecTable::StringToDeltaSeconds
