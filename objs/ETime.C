//==========================================================//
// Copyright (C) 1999, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

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
    int retval = sscanf(time_string, "%*19c.%hd", &ms);
    if (retval != 1)
        return(0);

    //-------------------------------//
    // put the rest into a struct tm //
    //-------------------------------//

    time_string[17] = '\0';
    struct tm tm_time;
    if (strptime(time_string, CODE_B_STRPTIME_FORMAT, &tm_time) == NULL)
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
// ETime::ToCodeB //
//----------------//

int
ETime::ToCodeB(
    char*  string)
{
    struct tm* tm_time = gmtime(&_sec);
    sprintf(string, CODE_B_SCANF_FORMAT, tm_time->tm_year + 1900,
        tm_time->tm_yday + 1, tm_time->tm_hour, tm_time->tm_min,
        tm_time->tm_sec, _ms);
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

//------------------------------//
// ETime::WriteCurrentTimeAscii //
//------------------------------//

int
ETime::WriteCurrentTimeAscii(
    FILE*  ofp)
{
    CurrentTime();
    return(WriteAscii(ofp));
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
