//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef ETIME_H
#define ETIME_H

static const char rcs_id_etime_h[] =
    "@(#) $Id$";

#include <stdio.h>
#include <time.h>

//======================================================================
// CLASSES
//    ETime
//======================================================================

//======================================================================
// CLASS
//    ETime
//
// DESCRIPTION
//    The ETime object handles time.
//======================================================================

#define CODE_B_DEFAULT_TIME     "1993-001T00:00:00.000\0"
#define CODE_B_TIME_LENGTH      22
#define CODE_B_STRPTIME_FORMAT  "%Y-%m-%dT%H:%M:%S"
#define CODE_B_SCANF_FORMAT     "%04d-%03dT%02d:%02d:%02d.%03d"

class ETime
{
public:

    //--------------//
	// construction //
	//--------------//

	ETime();
	~ETime();

    //--------------//
    // current time //
    //--------------//

    int  CurrentTime();

    //-------------//
    // conversions //
    //-------------//

    int  FromStructTm(struct tm* tm_time);
    int  FromCodeB(const char* code_b_string);
    int  ToCodeB(char* string);
    int  FromChar6(char* string);

    //--------------//
    // input/output //
    //--------------//

    int  Write(int fd);
    int  Read(int fd);
    int  WriteAscii(FILE* ofp);
    int  WriteCurrentTimeAscii(FILE* ofp);

    //-----------//
    // operators //
    //-----------//

    friend int  operator==(const ETime& a, const ETime& b);
    friend int  operator<(const ETime& a, const ETime& b);
    friend int  operator<=(const ETime& a, const ETime& b);
    friend int  operator>(const ETime& a, const ETime& b);
    friend int  operator>=(const ETime& a, const ETime& b);

protected:

    //-----------//
    // variables //
    //-----------//

    time_t          _sec;    // seconds since epoch
    unsigned short  _ms;     // milliseconds
};

int  operator==(const ETime& a, const ETime& b);
int  operator<(const ETime& a, const ETime& b);
int  operator<=(const ETime& a, const ETime& b);
int  operator>(const ETime& a, const ETime& b);
int  operator>=(const ETime& a, const ETime& b);

#endif
