//=========================================================
// Copyright  (C)1996, California Institute of Technology.
// U.S. Government sponsorship under
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.0   02 Jun 1999 16:21:32   sally
// Initial revision.
//
// $Date$
// $Revision$
// $Author$
//
//=========================================================

#ifndef LEAP_SEC_TABLE_H
#define LEAP_SEC_TABLE_H

#include "EAList.h"

class Itime;

struct LeapSecEntry
{
public:
    LeapSecEntry() : isUpdateTime(0) { };
    char        l1TimeString[L1_TIME_LEN];
    double      taiTimeSecond;
    int         isUpdateTime;
    int         adjustSecond;
};

class LeapSecTable : public EAList<LeapSecEntry>
{
public:
    enum LeapSecTableStatus
    {
        LEAP_TABLE_OK = 0,
        ERROR_OPEN_LEAP_TABLE,
        ERROR_READ_LEAP_TABLE
    };

    static const char* LeapSecTableStatusStrings[];

    LeapSecTable(const char*   leapFilename);

    virtual ~LeapSecTable() {};


    LeapSecTableStatus  GetStatus(void) { return(_status); }
    const char*         GetStatusString(void)
                        { return(LeapSecTableStatusStrings[(int)_status]); }

#if 0
             //------------------------------------------------------------
             // Itime includes leap seconds, need to subtract delta seconds
             // before converting to string
             //------------------------------------------------------------
    int      GetDeltaSeconds(double       taiLeapSeconds,      // IN
                             int&         deltaSeconds,        // OUT
                             int&         isUpdateTime,        // OUT
                             int&         updateTaiSeconds,    // OUT
                             char*        updateL1TimeString); // OUT if update
#endif

             //------------------------------------------------------------
             // Itime includes leap seconds, need to subtract delta seconds
             // before converting to string
             //------------------------------------------------------------
    int      ItimeToDeltaSeconds(const Itime& itime,               // IN
                                 int&         deltaSeconds,        // OUT
                                 int&         isUpdateTime,        // OUT
                                 char*        updateL1TimeString);//OUT if update

             //------------------------------------------------------------
             // need to add delta seconds to time string to get leap seconds
             //------------------------------------------------------------
    int      StringToDeltaSeconds(const char*  l1TimeString,      // IN
                                  int&         deltaSeconds,      // OUT
                                  int&         isUpdateTime,      // OUT
                                  double       taiTimeSecond);// OUT if update

protected:
    LeapSecTableStatus      _status;
};

inline int operator==(const LeapSecEntry& a, const LeapSecEntry& b)
{
    return(a.taiTimeSecond == b.taiTimeSecond);
}


#endif
