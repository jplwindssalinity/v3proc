//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.9   02 Dec 1998 12:54:28   sally
// use timegm() for linux
// 
//    Rev 1.8   04 Nov 1998 15:06:26   sally
// change shadow var names such as time, remainder
// 
//    Rev 1.7   18 Aug 1998 10:55:48   sally
// make L1ADrvExtract return any number of values
// 
//    Rev 1.6   28 May 1998 11:23:06   daffer
// Fixed L1ToItime2
// 
//    Rev 1.5   22 May 1998 16:45:00   daffer
// Added L1ToItime2 method
// 
//    Rev 1.4   01 Apr 1998 13:36:04   sally
// for L1A Derived table
// 
//    Rev 1.3   19 Mar 1998 13:36:06   sally
// added "days", "hours", "minutes" and "seconds" units
// 
//    Rev 1.2   16 Mar 1998 10:52:26   sally
// ReadReqi() are split into two methods
// 
//    Rev 1.1   12 Mar 1998 17:15:52   sally
// adapt to the new QSCAT REQI format
// 
//    Rev 1.0   04 Feb 1998 14:15:36   daffer
// Initial checking
// Revision 1.3  1998/01/30 22:29:06  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

#include <memory.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "Itime.h"
#include "CommonDefs.h"

static const char rcs_id_Itime_C[] =
    "@(#) $Header$";

//===============
// Itime methods 
//===============

//-------------------
// CodeA constructor 
//-------------------

Itime::Itime(
    const char*     string)
{
    CodeAToItime(string);
    return;
}

//-----------------------
// struct tm constructor 
//-----------------------

Itime::Itime(
    struct tm*      tm_time)
{
    tmToItime(tm_time);
    return;
}

//--------------------
// double constructor 
//--------------------

Itime::Itime(
    double      seconds)
{
    sec = (time_t)seconds;
    ms = (unsigned short)(1000*(seconds - (double)sec));
    return;
}


//-----------
// tmToItime 
//-----------
// return 1 on successful conversion, 0 on failure

int
Itime::tmToItime(
    struct tm*  tm_time)
{
#ifdef INTEL86
    sec = timegm(tm_time);
    if (sec == (time_t)-1)
        return 0;
    else
        return 1;
#else
    // mktime() is affected by timezone
    // but we want UTC timezone
    sec = mktime(tm_time);
    if (sec == (time_t)-1)
    {
        return (0);
    }
    else
    {
        sec -= timezone;
        ms = 0;
        return (1);
    }
#endif
}

//-----------
// ItimeTotm 
//-----------
// tm is in UTC

int
Itime::ItimeTotm(
struct tm*& tm_time)
{
    tm_time = gmtime(&sec);
    return (1);     // assume success
}

//--------------
// Char6ToItime 
//--------------

int
Itime::Char6ToItime(
    const char* string)
{
    memcpy((char *)&sec, string, sizeof(time_t));
    memcpy((char *)&ms, string + sizeof(time_t),
        sizeof(unsigned short));
    return (1);     // assume success
}

//--------------
// ItimeToChar6 
//--------------

int
Itime::ItimeToChar6(
    char*       char6)
{
    memcpy(char6, (char *)&sec, sizeof(time_t));
    memcpy(char6 + sizeof(time_t), (char *)&ms, sizeof(unsigned short));
    return (1);     // assume success
}

//--------------
// CodeAToItime 
//--------------
// YYYY-MM-DDThh:mm:ss.dddZ 

int
Itime::CodeAToItime(
    const char*     string)
{
    // check for invalid time or empty string
    if (strcmp(string, INVALID_TIME_STRING) == 0 |
        *string == '\0')
    {
        *this = INVALID_TIME;
        return(1);
    }

    // check the length of the string
    int default_length = strlen(CODEA_DEFAULT_TIME);
    int length = strlen(string);
    if (length > default_length)
        return (0);

    // overlay the time string on the Code A default time string
    char time_string[CODEA_TIME_LEN];
    memcpy(time_string, CODEA_DEFAULT_TIME, default_length);
    memcpy(time_string, string, length);
    time_string[default_length] = '\0';

    // convert milliseconds
    unsigned short millisec;
    int rc = sscanf(time_string, "%*19c.%hd", &millisec);
    if (rc != 1)    // error parsing time string
        return (0);

    // put the rest in a struct tm
    time_string[19] = '\0';
    struct tm tm_time;
    if (strptime(time_string, CODEA_SEC_FORMAT, &tm_time) == NULL)
        return (0);

    // convert
    tmToItime(&tm_time);
    ms = millisec;
    return (1);
}

//--------------
// ItimeToCodeA 
//--------------
// YYYY-MM-DDThh:mm:ss.dddZ 

int
Itime::ItimeToCodeA(
    char*       string,
    char*       invalid_time_string)
const
{
    if (*this == INVALID_TIME)
    {
        sprintf(string, "%s", invalid_time_string);
    }
    else
    {
        struct tm *tm_time = gmtime(&sec);
        (void) sprintf(string, "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ",
                    tm_time->tm_year + 1900, tm_time->tm_mon + 1,
                    tm_time->tm_mday, tm_time->tm_hour, tm_time->tm_min,
                    tm_time->tm_sec, ms);
    }
    return (1);     // assume success
}

//------------------
// ItimeToCodeADate 
//------------------
// YYYY-MM-DD 

int
Itime::ItimeToCodeADate(
    char*       string)
{
    struct tm *tm_time = gmtime(&sec);
    (void) sprintf(string, "%04d-%02d-%02d", tm_time->tm_year + 1900,
        tm_time->tm_mon + 1, tm_time->tm_mday);
    return (1);     // assume success
}
//--------------
// ItimeToCodeAHour
//--------------
// YYYY-MM-DDThh

int
Itime::ItimeToCodeAHour(
char*       string)
{
    struct tm *tm_time = gmtime(&sec);
    (void) sprintf(string, "%04d-%02d-%02dT%02d",
                tm_time->tm_year + 1900, tm_time->tm_mon + 1,
                tm_time->tm_mday, tm_time->tm_hour);
    return (1);     // assume success

} // Itime::ItimeToCodeAHour

//--------------
// ItimeToCodeASecond
//--------------
// YYYY-MM-DDThh:mm:ss 

int
Itime::ItimeToCodeASecond(
char*       string)
{
    struct tm *tm_time = gmtime(&sec);
    (void) sprintf(string, "%04d-%02d-%02dT%02d:%02d:%02d",
                tm_time->tm_year + 1900, tm_time->tm_mon + 1,
                tm_time->tm_mday, tm_time->tm_hour, tm_time->tm_min,
                tm_time->tm_sec);
    return (1);     // assume success
}

//--------------
// BDateToItime 
//--------------
// YYYYMMDD 

int
Itime::BDateToItime(
    const char*     string)
{
    // check the length of the string
    int default_length = strlen(BDATE_DEFAULT_TIME);
    int length = strlen(string);
    if (length > default_length)
        return (0);

    // overlay the time string on the default BDate time string
    char bdate_string[BDATE_TIME_LEN+4];
    memcpy(bdate_string, BDATE_DEFAULT_TIME, default_length);
    memcpy(bdate_string, string, length);
    bdate_string[default_length] = '\0';

    // convert YYYYMMDD to YYYY-MM-DD so that strptime can parse it
    char year[5], month[3], day[3];
    if (sscanf(bdate_string, "%4s%2s%2s", year, month, day) != 3)
        return (0);
    sprintf(bdate_string, "%4s-%2s-%2s", year, month, day);

    // put bdate_string into struct tm
    struct tm tm_time;
    if (strptime(bdate_string, BDATE_MOD_FORMAT, &tm_time) == NULL)
        return (0);

    // convert
    tmToItime(&tm_time);
    return (1);
}

//--------------
// ItimeToBDate 
//--------------
// YYYYMMDD 

int
Itime::ItimeToBDate(
    char*   string)
{
    struct tm *tm_time = gmtime(&sec);
    (void) sprintf(string, "%04d%02d%02d", tm_time->tm_year + 1900,
        tm_time->tm_mon + 1, tm_time->tm_mday);
    return (1);     // assume success
}

//-----------
// L1ToItime 
//-----------
// YYYY-DDDThh:mm:ss.ddd (NO SHORTENING ALLOWED!!!) 

int
Itime::L1ToItime(
    const char*     string)
{
    // check the length of the string
/*
    int default_length = strlen(L1_DEFAULT_TIME);
    int length = strlen(string);
    if (length > default_length)
        return (0);

    // overlay the time string on the L1 default time string
    char time_string[L1_TIME_LEN];
    memcpy(time_string, L1_DEFAULT_TIME, default_length);
    memcpy(time_string, string, length);
    time_string[default_length] = '\0';
*/
    char time_string[L1_TIME_LEN];
    strncpy(time_string, string, L1_TIME_LEN-1);
    time_string[L1_TIME_LEN-1] = '\0';

    // convert milliseconds
    unsigned short millisec;
    int rc = sscanf(time_string, "%*17c.%hd", &millisec);
    if (rc != 1)    // error parsing time string
        return (0);

    // put the rest in a struct tm
    time_string[17] = '\0';
    struct tm tm_time;
    if (strptime(time_string, L1_SEC_FORMAT, &tm_time) == NULL)
        return (0);

    // convert
    tmToItime(&tm_time);
    ms = millisec;
    return (1);
}

//-----------------------
//
// L1ToItime2, Two arguments, 
// I would overload L1ToItime, but I had problems 
// when I tried to do this with a constructor. 
// The first argument has the form 'YYYY-DDD' 
// and the second of the form 'HH:MM:SS.CCC'
//
//------------------------
int 
Itime::L1ToItime2( const char *yyyyddd,
                   const char *hhmmssccc )
{

    char time_string[L1_TIME_LEN];
    for (int i=0;i<L1_TIME_LEN;i++) time_string[i]='\0';
    int l=strlen(yyyyddd);
    strncpy(time_string, yyyyddd, l);
    strncat( time_string, "T",1);
    l=strlen( hhmmssccc );
    strncat( time_string, hhmmssccc,l );
    time_string[L1_TIME_LEN-1] = '\0';
    L1ToItime( time_string );
    return (1);
}



//-----------
// ItimeToL1 
//-----------

int
Itime::ItimeToL1(
    char*   string)
{
    struct tm *tm_time = gmtime(&sec);
    (void) sprintf(string, "%04d-%03dT%02d:%02d:%02d.%03d",
                tm_time->tm_year + 1900, tm_time->tm_yday + 1,
                tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec, ms);
    return (1);     // assume success
}

//------------
// BCDToItime 
//------------
// BCD format 

int
Itime::BCDToItime(
    const char*     string)
{
    struct tm tm_time;

    tm_time.tm_year = GetBits(*string, 7, 4) * 10 + GetBits(*string, 3, 4);
    // add 100 years if it is 21st century
    if (tm_time.tm_year < 94)
        tm_time.tm_year += 100;

    tm_time.tm_yday = GetBits(*(string + 1), 7, 4) * 100 +
                GetBits(*(string + 1), 3, 4) * 10 +
                GetBits(*(string + 2), 7, 4);

    // original values of tm_wday and tm_yday are ignored.
    // So, convert tm_yday to tm_mon and tm_mday
    ydayToMonMday (tm_time.tm_year + 1900, tm_time.tm_yday, &tm_time);

    tm_time.tm_yday = -1;
    tm_time.tm_hour = GetBits(*(string + 2), 3, 4) * 10 +
                GetBits(*(string + 3), 7, 4);
    tm_time.tm_min =    GetBits(*(string + 3), 3, 4) * 10 +
                GetBits(*(string + 4), 7, 4);
    tm_time.tm_sec =    GetBits(*(string + 4), 3, 4) * 10 +
                GetBits(*(string + 5), 7, 4);
    tm_time.tm_wday = -1;
    tm_time.tm_isdst = -1;

    // convert to seconds since epoch
    tmToItime(&tm_time);
    ms = GetBits(*(string + 5), 3, 4) * 100 +
            GetBits(*(string + 6), 7, 4) * 10;
    return (1);     // assume success
}

//------------
// ItimeToHMS 
//------------
// hh:mm:ss 

int
Itime::ItimeToHMS(
    char*       string)
const
{
    struct tm *tm_time = gmtime(&sec);
    (void) sprintf(string, "%02d:%02d:%02d",
                tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec);
    return (1);     // assume success
}

//--------
// AbsDif 
//--------

Itime
Itime::AbsDif(
    Itime   itime)
{
    if (*this > itime)
        return (*this - itime);
    else
        return (itime - *this);
}

//---------------
// ydayToMonMday 
//---------------

void
Itime::ydayToMonMday(
int     year,
int     yday,
struct tm*  tm_time)
{
    const int monDays[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
    char isLeap = FALSE;
    if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)
        isLeap = TRUE;
    int i=0;
    for (i=0; i < 12; i++)
    {
        if (yday > monDays[i])
        {
            yday -= monDays[i];
            if (i == 1 && isLeap)
                yday --;
        }
        else
            break;
    }
    tm_time->tm_mon = i;
    tm_time->tm_mday = yday;
    return;
}

//-----------
// operators 
//-----------

// assumes time1 and time2 are positive !!!
Itime
operator+(
    const Itime&    time1,
    const Itime&    time2)
{
    time_t total_sec = time1.sec + time2.sec;
    unsigned short total_ms = time1.ms + time2.ms;
    if (total_ms > 999)
    {
        total_ms -= 1000;
        total_sec++;
    }
    return (Itime(total_sec, total_ms));
}

// assumes time1 and time2 are positive and time1 > time2
Itime
operator-(
    const Itime&    time1,
    const Itime&    time2)
{
    time_t total_sec = time1.sec - time2.sec;
    short total_ms = (short)time1.ms - (short)time2.ms;
    if (total_ms < 0)
    {
        total_ms += 1000;
        total_sec--;
    }
    return (Itime(total_sec, (unsigned short)total_ms));
}

Itime
operator/(
const Itime&    time1,
int             i)
{
    time_t newSec = time1.sec / i;
    time_t remainderSec = time1.sec - (newSec * i);
    unsigned short newMs = (remainderSec * 1000 + time1.ms) / i;

    return (Itime(newSec, newMs));
}

Itime
operator*(
const Itime&    time1,
int             i)
{
    unsigned long newMs = time1.ms * i;
    time_t regroup_sec = (time_t)(newMs / 1000);
    time_t newSec = time1.sec + regroup_sec;
    newMs -= regroup_sec * 1000;

    return (Itime(newSec, (unsigned short)newMs));
}

Itime
Itime::StartOfMinute()
const
{
    struct tm *tm_time;

    tm_time = gmtime(&sec);     // convert to tm structure
    tm_time->tm_sec = 0;        // zero proper units
    Itime newtime;
    newtime.tmToItime(tm_time);
    return (newtime);
}

Itime
Itime::StartOfHour()
const
{
    struct tm *tm_time;

    tm_time = gmtime(&sec);     // convert to tm structure
    tm_time->tm_sec = 0;        // zero proper units
    tm_time->tm_min = 0;
    Itime newtime;
    newtime.tmToItime(tm_time);
    return (newtime);
}

Itime
Itime::StartOfDay()
const
{
    struct tm *tm_time;

    tm_time = gmtime(&sec);     // convert to tm structure
    tm_time->tm_sec = 0;        // zero proper units
    tm_time->tm_min = 0;
    tm_time->tm_hour = 0;
    Itime newtime;
    newtime.tmToItime(tm_time);
    return (newtime);
}

Itime
Itime::StartOfMonth()
const
{
    struct tm *tm_time;

    tm_time = gmtime(&sec);     // convert to tm structure
    tm_time->tm_sec = 0;        // zero proper units
    tm_time->tm_min = 0;
    tm_time->tm_hour = 0;
    tm_time->tm_mday = 0;
    Itime newtime;
    newtime.tmToItime(tm_time);
    return (newtime);
}

Itime
Itime::StartOfYear()
const
{
    struct tm *tm_time;

    tm_time = gmtime(&sec);     // convert to tm structure
    tm_time->tm_sec = 0;        // zero proper units
    tm_time->tm_min = 0;
    tm_time->tm_hour = 0;
    tm_time->tm_mday = 0;
    tm_time->tm_mon = 0;
    Itime newtime;
    newtime.tmToItime(tm_time);
    return (newtime);
}

int
Itime::ItimeToElapsedTime(
char*   string)
{
    int days, hours, minutes, seconds=sec;

    days = (int)((float)seconds / SEC_PER_DAY);
    seconds -= SEC_PER_DAY * days;

    hours = (int)((float)seconds / SEC_PER_HOUR);
    seconds -= SEC_PER_HOUR * hours;

    minutes = (int)((float)seconds / SEC_PER_MINUTE);
    seconds -= SEC_PER_MINUTE * minutes;

    // "dddddThh:mm:ss"
    if (sprintf(string, "%5dT%02d:%02d:%02d",
                    days, hours, minutes, seconds) > 0)
        return(1);
    else
        return(0);

}//Itime::ItimeToElapsedTime

//static
const char*
Itime::CurrentCodeA()
{
    static char time_string[CODEA_TIME_LEN];
    Itime current_time((time_t)time(0));
    current_time.ItimeToCodeA(time_string);
    return(time_string);

}//Itime::CurrentCodeA

//--------------------
// printing functions 
//--------------------

void pr_itime_codea(FILE *ofp, char *dataP)
{
    Itime it;
    it.Char6ToItime(dataP);
    char output_string[CODEA_TIME_LEN];
    it.ItimeToCodeA(output_string);
    fprintf(ofp, "%s", output_string);
    return;
}
 
void pr_itime_L1(FILE *ofp, char *dataP)
{
    Itime it;
    it.Char6ToItime(dataP);
    char output_string[L1_TIME_LEN];
    it.ItimeToL1(output_string);
    fprintf(ofp, "%s", output_string);
    return;
}
 
void pr_itime_d(FILE *ofp, char *dataP)
{
    // this is "time since...", so don't adjust it with ITIME_DEFAULT_SEC
    Itime it;
    it.Char6ToItime(dataP);
    fprintf(ofp, "%.7f", it.Days());
    return;
}
 
void pr_itime_h(FILE *ofp, char *dataP)
{
    // this is "time since...", so don't adjust it with ITIME_DEFAULT_SEC
    Itime it;
    it.Char6ToItime(dataP);
    fprintf(ofp, "%.5f", it.Hours());
    return;
}
 
void pr_itime_m(FILE *ofp, char *dataP)
{
    // this is "time since...", so don't adjust it with ITIME_DEFAULT_SEC
    Itime it;
    it.Char6ToItime(dataP);
    fprintf(ofp, "%.4f", it.Minutes());
    return;
}
 
void pr_itime_s(FILE *ofp, char *dataP)
{
    // this is "time since...", so don't adjust it with ITIME_DEFAULT_SEC
    Itime it;
    it.Char6ToItime(dataP);
    fprintf(ofp, "%.3f", it.Seconds());
    return;
}

Itime::CodeAFormatE
Itime::CheckCodeAFormat(
const char*     codeAString)
{
    char SecString[CODEA_TIME_LEN - 5];
    unsigned short millisec;
    int wholeStringRC = sscanf(codeAString, "%s.%hdZ", SecString, &millisec);
    // no match, not an Code A
    if (wholeStringRC == 0 || wholeStringRC == EOF)
        return (CODE_A_FORMAT_UNKNOWN);

    int year=0, month=0, day=0, hour=0, minute=0, second=0;
    int timeStringRC = sscanf(SecString, "%4d-%2d-%2dT%2d:%2d:%2d",
                        &year, &month, &day, &hour, &minute, &second);
    switch (timeStringRC)
    {
        case 1:
            return CODE_A_FORMAT_YEAR;
            break;
        case 2:
            return CODE_A_FORMAT_MONTH;
            break;
        case 3:
            return CODE_A_FORMAT_DAY;
            break;
        case 4:
            return CODE_A_FORMAT_HOUR;
            break;
        case 5:
            return CODE_A_FORMAT_MINUTE;
            break;
        case 6:
            if (wholeStringRC == 1)
                return CODE_A_FORMAT_SECOND;
            else
                return CODE_A_FORMAT_MILLI_SECOND;
            break;
        default:
            break;
    }

    return CODE_A_FORMAT_UNKNOWN;

} // Itime::CheckCodeAFormat
