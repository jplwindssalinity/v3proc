//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.6   18 Aug 1998 10:56:30   sally
// make L1ADrvExtract return any number of values
// 
//    Rev 1.5   22 May 1998 16:44:38   daffer
// Added L1ToItime2 method
// 
//    Rev 1.4   01 Apr 1998 13:36:10   sally
// for L1A Derived table
// 
//    Rev 1.3   19 Mar 1998 13:37:04   sally
//  added "days", "hours", "minutes" and "seconds" units
// 
//    Rev 1.2   16 Mar 1998 10:52:30   sally
// ReadReqi() are split into two methods
// 
//    Rev 1.1   12 Mar 1998 17:15:54   sally
// adapt to the new QSCAT REQI format
// 
//    Rev 1.0   04 Feb 1998 14:15:38   daffer
// Initial checking
// Revision 1.2  1998/01/30 22:28:21  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

#ifndef ITIME_H
#define ITIME_H

#include <stdio.h>
#include <time.h>
#include <limits.h>

#include "CommonDefs.h"

static const char rcs_id_itime_h[] =
    "@(#) $Header$";

#define CODEA_DEFAULT_TIME  "1993-01-01T00:00:00.000Z"
#define CODEA_SEC_FORMAT    "%Y-%m-%dT%H:%M:%S"
#define BDATE_DEFAULT_TIME  "19930101"
#define BDATE_MOD_FORMAT    "%Y-%m-%d"
#define L1_DEFAULT_TIME     "1993-001T00:00:00.000"
#define L1_SEC_FORMAT       "%Y-%jT%H:%M:%S"

#define CODEA_TIME_LEN      25
#define BDATE_TIME_LEN      9
#define L1_TIME_LEN         22
#define HMS_TIME_LEN        9
#define ELAPSED_TIME_LEN    15

#define SEC_PER_MINUTE  60
#define SEC_PER_HOUR    3600
#define SEC_PER_DAY     86400
#define SEC_PER_WEEK    604800
#define SEC_PER_28_DAYS 2419200
#define MS_PER_SECOND   1000
#define MS_PER_MINUTE   60000
#define MS_PER_HOUR     3600000
#define MS_PER_DAY      86400000
#define MS_PER_WEEK     604800000

// TAI time base: "1993-01-01T00:00:00.000Z"
#define ITIME_DEFAULT_SEC  725846400

#define INVALID_TIME    Itime((time_t)0)
#define MIN_TIME        Itime((time_t)0)
#define MAX_TIME        Itime((time_t)LONG_MAX, USHRT_MAX)

#define INVALID_TIME_STRING     "<none>"

//--------
// macros 
//--------

#define SIGN(A)     ((A)<0?-1:1)

struct Itime
{
    enum CodeAFormatE
    {
        CODE_A_FORMAT_UNKNOWN,
        CODE_A_FORMAT_YEAR,
        CODE_A_FORMAT_MONTH,
        CODE_A_FORMAT_DAY,
        CODE_A_FORMAT_HOUR,
        CODE_A_FORMAT_MINUTE,
        CODE_A_FORMAT_SECOND,
        CODE_A_FORMAT_MILLI_SECOND
    };

    friend Itime    operator+(const Itime&, const Itime&);
    friend Itime    operator-(const Itime&, const Itime&);
    friend int      operator==(const Itime&, const Itime&);
    friend int      operator!=(const Itime&, const Itime&);
    friend int      operator>(const Itime&, const Itime&);
    friend int      operator>=(const Itime&, const Itime&);
    friend int      operator<(const Itime&, const Itime&);
    friend int      operator<=(const Itime&, const Itime&);
    friend Itime    operator/(const Itime&, const Itime&);
    friend Itime    operator*(const Itime&, int num);

    //--------------
    // constructors 
    //--------------

    Itime():sec(0), ms(0) {};
    Itime(time_t seconds):
                    sec(seconds), ms(0) {};
    Itime(time_t seconds, unsigned short milliseconds):
                    sec(seconds), ms(milliseconds) {};
    Itime(const char* codea);
    Itime(struct tm*);
    Itime(double seconds);
    // Itime(const char* yyyyddd, const char* hhmmssccc);

    int             tmToItime(struct tm* tm_time);
                    // user need not allocate space for tm_time
    int             ItimeTotm(struct tm*& tm_time);
    int             Char6ToItime(const char* string);
    int             ItimeToChar6(char* string);
    int             CodeAToItime(const char* string);
    int             ItimeToCodeA(char* string,
                        char* invalid_time_string = INVALID_TIME_STRING ) const;
    int             ItimeToCodeADate(char* string);
    int             ItimeToCodeAHour(char* string);
    int             ItimeToCodeASecond(char* string);
    int             BDateToItime(const char* string);
    int             ItimeToBDate(char* string);
    int             L1ToItime(const char* string);
    int             L1ToItime2(const char* yyyyddd, const char* hhmmssccc);
    int             ItimeToL1(char* string);
    int             BCDToItime(const char* string);
    int             ItimeToHMS(char* string) const;

                    // "dddddThh:mm:ss"
    int             ItimeToElapsedTime(char* string);

    Itime           AbsDif(Itime itime);

    Itime           StartOfMinute() const;
    Itime           StartOfHour() const;
    Itime           StartOfDay() const;
    Itime           StartOfMonth() const;
    Itime           StartOfYear() const;

    static const char*      CurrentCodeA(void);

    static CodeAFormatE     CheckCodeAFormat(const char* codeAString);

    static void     ydayToMonMday(int year, int yday, struct tm* tm_time);

    inline double   Days(void)
                    { return (double)sec/SEC_PER_DAY + (double)ms/MS_PER_DAY;} 
    inline double   Hours(void)
                    { return (double)sec/SEC_PER_HOUR + (double)ms/MS_PER_HOUR;}
    inline double   Minutes(void)
                    { return (double)sec/SEC_PER_MINUTE + (double)ms/MS_PER_MINUTE;}
    inline double   Seconds(void)
                    { return (double)sec + (double)ms / MS_PER_SECOND; }

    //-----------
    // operators 
    //-----------

    Itime& operator=(const Itime& time)
                    { sec = time.sec; ms = time.ms; return *this; }
    Itime& operator+=(const Itime& time)
                {
                    *this = *this + time;
                    return (*this);
                };
    Itime& operator-=(const Itime& time)
                {
                    *this = *this - time;
                    return (*this);
                };

    //-----------
    // variables 
    //-----------

    time_t          sec;    //seconds since epoch
    unsigned short  ms;     //milli seconds

}; // struct Itime

extern Itime operator+(const Itime& time1, const Itime& time2);
extern Itime operator-(const Itime& time1, const Itime& time2);
extern Itime operator/(const Itime& time1, int i);
extern Itime operator*(const Itime& time1, int i);

inline int      operator==(const Itime& a, const Itime& b)
                {
                    return (a.sec == b.sec &&  a.ms == b.ms);
                }
inline int      operator!=(const Itime& a, const Itime& b)
                {
                    return (a.sec != b.sec ||  a.ms != b.ms);
                }
inline int      operator>(const Itime& a, const Itime& b)
                {
                    return (a.sec > b.sec || (a.sec == b.sec && a.ms > b.ms));
                }
inline int      operator>=(const Itime& a, const Itime& b)
                {
                    return (a > b || a == b);
                }
inline int      operator<(const Itime& a, const Itime& b)
                {
                    return (a.sec < b.sec || (a.sec == b.sec && a.ms < b.ms));
                }
inline int      operator<=(const Itime& a, const Itime& b)
                {
                    return (a < b || a == b);
                }

//--------------------
// printing functions 
//--------------------

void pr_itime_codea(FILE* ofp, char* dataP);
void pr_itime_L1(FILE* ofp, char* dataP);
void pr_itime_d(FILE* ofp, char* dataP);
void pr_itime_h(FILE* ofp, char* dataP);
void pr_itime_m(FILE* ofp, char* dataP);
void pr_itime_s(FILE* ofp, char* dataP);

#endif //ITIME_H
