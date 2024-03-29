//==============================================================//
// Copyright (C) 1999-2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_etime_c[] =
    "@(#) $Id$";

#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include "ETime.h"

//=======//
// ETime //
//=======//

ETime::ETime()
:   _sec(0), _ms(0)
{
    return;
}

ETime::~ETime()
{
    return;
}

//--------------------//
// ETime::CurrentTime //
//--------------------//

int
ETime::CurrentTime()
{
    _sec = time(0);
    return(1);
}

//---------------------//
// ETime::FromStructTm //
//---------------------//

int
ETime::FromStructTm(
    struct tm*  tm_time)
{

# define MACHACK
// stuck in here to force the code to work in MACOSX and probably LINUX
// will fail on SUN unless the line above is commented out

#ifdef MACHACK
    _sec = timegm(tm_time);
    if (_sec == (time_t)-1)
        return 0;
    else
    {
	_ms=0;
	return 1;
    }
#else
    _sec = mktime(tm_time);
    if (_sec == (time_t)-1)
    {
        return(0);
    }
    else
    {
        // mktime() is affected by timezone
        // but we want UTC timezone
        _sec -= timezone;
        _ms = 0;
        return(1);
    }
#endif

}

//------------------//
// ETime::FromCodeA //
//------------------//

int
ETime::FromCodeA(
    const char*  code_a_string)
{
    //------------------------------------------------//
    // overlay code a string onto default code a time //
    //------------------------------------------------//

    int length = strlen(code_a_string);
    if (length >= CODE_A_TIME_LENGTH)
        return(0);    // string too big

    char time_string[CODE_A_TIME_LENGTH];
    memcpy(time_string, CODE_A_DEFAULT_TIME, CODE_A_TIME_LENGTH);
    memcpy(time_string, code_a_string, length);

    //----------------------//
    // get the milliseconds //
    //----------------------//

    unsigned short ms;
    int retval = sscanf(time_string, "%*19c.%hd", &ms);
    if (retval != 1)
        return(0);

    //-------------------------------//
    // put the rest into a struct tm //
    //-------------------------------//

    time_string[19] = '\0';
    struct tm tm_time;
    if (strptime(time_string, CODE_A_STRPTIME_FORMAT, &tm_time) == NULL)
        return(0);

    //---------//
    // convert //
    //---------//

    if (! FromStructTm(&tm_time))
        return(0);

    _ms = ms;
    return(1);
}

//----------------//
// ETime::ToCodeA //
//----------------//

int
ETime::ToCodeA(
    char*  string)
{
    struct tm* tm_time = gmtime(&_sec);
    sprintf(string, CODE_A_PRINTF_FORMAT, tm_time->tm_year + 1900,
        tm_time->tm_mon + 1, tm_time->tm_mday, tm_time->tm_hour,
        tm_time->tm_min, tm_time->tm_sec, _ms);
    return(1);
}

//------------------//
// ETime::FromCodeB //
//------------------//

int
ETime::FromCodeB(
    const char*  code_b_string)
{
    //------------------------------------------------//
    // overlay code b string onto default code b time //
    //------------------------------------------------//

    int length = strlen(code_b_string);
    if (length >= CODE_B_TIME_LENGTH)
        return(0);    // string too big

    char time_string[CODE_B_TIME_LENGTH];
    memcpy(time_string, CODE_B_DEFAULT_TIME, CODE_B_TIME_LENGTH);
    memcpy(time_string, code_b_string, length);

    //----------------------//
    // get the milliseconds //
    //----------------------//

    unsigned short ms;
    int retval = sscanf(time_string, "%*17c.%hd", &ms);
    if (retval != 1)
        return(0);

    //-------------------------------//
    // put the rest into a struct tm //
    //-------------------------------//

    time_string[17] = '\0';
    struct tm tm_time;

//    if (strptime(time_string, CODE_B_STRPTIME_FORMAT, &tm_time) == NULL)
//    	return(0);
    	
    // Required to handle at least one case of HH:MM:SS == 24:00:00 in OSCAT-2
    // data
    // taw - 02 May 2012
    if (sscanf(time_string, "%d-%dT%d:%d:%d", &tm_time.tm_year, &tm_time.tm_yday,
                &tm_time.tm_hour, &tm_time.tm_min, &tm_time.tm_sec) != 5) {
        return(0);
    }

    // tm_year is # of years since 1900
    tm_time.tm_year -= 1900;
    // tm_yday (\in [0, 365]) is # days since Jan 1, but the parsed value is the day
    // number (\in [1, 366])
    tm_time.tm_yday--;

    tm_time.tm_yday += tm_time.tm_hour/24;
    tm_time.tm_hour -= 24*(tm_time.tm_hour/24);

   	// strptime sets year day, but is too stupid
   	// to realize that corresponds precisely to a month and month day, so do that
   	// conversion here b/c those fields are needed later.
   	// number of days in each month
   	int d_in_m[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	// tm_yday is complete elapsed days, but we need the day that we are currently in
	// so add 1 to make that adjustment
   	int mday = tm_time.tm_yday + 1;
   	
   	int cal_year = tm_time.tm_year+1900;
   	if( cal_year%400 == 0 || (cal_year%100 != 0 && cal_year%4 == 0 ) ) // leap year
   	  d_in_m[1]++;
   	
	int m_i;
	for (m_i = 0; m_i < 12 && mday > d_in_m[m_i]; m_i++)
		mday -= d_in_m[m_i];
		
	tm_time.tm_mday = mday;
	
	// Changed m_i + 1 to m_i since tm class wants the integer part of the
	// number of months since January, not the current month number. AGF 10-20-2010
	tm_time.tm_mon = m_i;

//	printf("ts %s; %s\n", time_string, CODE_B_STRPTIME_FORMAT);
//	printf("year: %d, month: %d, day: %d, hour: %d, min: %d, sec: %d\n",
//		tm_time.tm_year, tm_time.tm_mon, tm_time.tm_mday, tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);

    //---------//
    // convert //
    //---------//

    if (! FromStructTm(&tm_time))
        return(0);

    _ms = ms;
    return(1);
}

//----------------//
// ETime::ToCodeB //
//----------------//

int
ETime::ToCodeB(
    char*  string)
{
    struct tm* tm_time = gmtime(&_sec);
    sprintf(string, CODE_B_PRINTF_FORMAT, tm_time->tm_year + 1900,
        tm_time->tm_yday + 1, tm_time->tm_hour, tm_time->tm_min,
        tm_time->tm_sec, _ms);
    return(1);
}

//-----------------//
// ETime::ToBlockB //
//-----------------//

int
ETime::ToBlockB(
    char*  string)
{
    struct tm* tm_time = gmtime(&_sec);
    sprintf(string, BLOCK_B_PRINTF_FORMAT, tm_time->tm_year + 1900,
        tm_time->tm_yday + 1, tm_time->tm_hour, tm_time->tm_min);
    return(1);
}

//------------------//
// ETime::FromChar6 //
//------------------//

int
ETime::FromChar6(
    char*  string)
{
    memcpy(&_sec, string, 4);
    memcpy(&_ms, string + 4, 2);
    return(1);
}

//--------------------//
// ETime::ToBlockDate //
//--------------------//

int
ETime::ToBlockDate(
    char*  string)
{
    struct tm* tm_time = gmtime(&_sec);
    sprintf(string, BLOCKDATE_PRINTF_FORMAT, tm_time->tm_year + 1900,
        tm_time->tm_mon + 1, tm_time->tm_mday);
    return(1);
}

//-------------------//
// ETime::ToJustTime //
//-------------------//

int
ETime::ToJustTime(
    char*  string)
{
    struct tm* tm_time = gmtime(&_sec);
    sprintf(string, JUSTTIME_PRINTF_FORMAT, tm_time->tm_hour, tm_time->tm_min,
        tm_time->tm_sec);
    return(1);
}

//--------------//
// ETime::Write //
//--------------//

int
ETime::Write(
    int  fd)
{
    if (write(fd, (void *)&_sec, sizeof(time_t)) != sizeof(time_t) ||
        write(fd, (void *)&_ms, sizeof(unsigned short)) !=
          sizeof(unsigned short))
    {
        return(0);
    }
    return(1);
}

//-------------//
// ETime::Read //
//-------------//

int
ETime::Read(
    int  fd)
{
    if (read(fd, (void *)&_sec, sizeof(time_t)) != sizeof(time_t) ||
        read(fd, (void *)&_ms, sizeof(unsigned short)) !=
            sizeof(unsigned short))
    {
        return(0);
    }
    return(1);
}

//-------------------//
// ETime::WriteAscii //
//-------------------//

int
ETime::WriteAscii(
    FILE*  ofp)
{
    char string[CODE_B_TIME_LENGTH];
    if (! ToCodeB(string))
        return(0);
    fprintf(ofp, "%s", string);
    return(1);
}

//----------------//
// ETime::SetTime //
//----------------//

void
ETime::SetTime(
    double  seconds)
{
    _sec = (time_t)seconds;
    _ms = (unsigned short)(1E3 * (seconds - (double)_sec));
    return;
}

//----------------//
// ETime::GetTime //
//----------------//

double
ETime::GetTime()
{
    double seconds = (double)_sec + (double)_ms / 1000.0;
    return seconds;
}

//------------//
// operator== //
//------------//

int
operator==(
    const ETime&  a,
    const ETime&  b)
{
    return(a._sec == b._sec && a._ms == b._ms);
}

//-----------//
// operator< //
//-----------//

int
operator<(
    const ETime&  a,
    const ETime&  b)
{
    return(a._sec < b._sec || (a._sec == b._sec && a._ms < b._ms));
}

//------------//
// operator<= //
//------------//

int
operator<=(
    const ETime&  a,
    const ETime&  b)
{
    return(a < b || a == b);
}

//-----------//
// operator> //
//-----------//

int
operator>(
    const ETime&  a,
    const ETime&  b)
{
    return(a._sec > b._sec || (a._sec == b._sec && a._ms > b._ms));
}

//------------//
// operator>= //
//------------//

int
operator>=(
    const ETime&  a,
    const ETime&  b)
{
    return(a > b || a == b);
}

//-------------------//
// CurrentTimeString //
//-------------------//

const char*
CurrentTimeString()
{
    static char string[CODE_A_TIME_LENGTH];

    ETime current_time;
    current_time.CurrentTime();
    if (! current_time.ToCodeA(string))
        return(NULL);

    return(string);
}
