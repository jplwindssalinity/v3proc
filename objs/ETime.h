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

#define CODE_A_DEFAULT_TIME     "1993-01-01T00:00:00.000\0"
#define CODE_A_TIME_LENGTH      24
#define CODE_A_STRPTIME_FORMAT  "%Y-%m-%dT%H:%M:%S"
#define CODE_A_PRINTF_FORMAT    "%04d-%02d-%02dT%02d:%02d:%02d.%03d"

#define CODE_B_DEFAULT_TIME     "1993-001T00:00:00.000\0"
#define CODE_B_TIME_LENGTH      22
#define CODE_B_STRPTIME_FORMAT  "%Y-%jT%H:%M:%S"
#define CODE_B_PRINTF_FORMAT    "%04d-%03dT%02d:%02d:%02d.%03d"

#define BLOCKDATE_DEFAULT_TIME     "19930101\0"
#define BLOCKDATE_LENGTH           9
#define BLOCKDATE_STRPTIME_FORMAT  "%Y%m%d"
#define BLOCKDATE_PRINTF_FORMAT    "%04d%02d%02d"

#define JUSTTIME_DEFAULT_TIME     "00:00:00\0"
#define JUSTTIME_LENGTH           9
#define JUSTTIME_STRPTIME_FORMAT  "%H:%M:%S"
#define JUSTTIME_PRINTF_FORMAT    "%02d:%02d:%02d"

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
    void  Zero()    { _sec = 0; _ms = 0; return; };

    //-------------//
    // conversions //
    //-------------//

    int  FromStructTm(struct tm* tm_time);
    int  FromCodeA(const char* code_a_string);
    int  ToCodeA(char* string);
    int  FromCodeB(const char* code_b_string);
    int  ToCodeB(char* string);
    int  FromChar6(char* string);
    int  ToBlockDate(char* string);
    int  ToJustTime(char* string);

    //--------------//
    // input/output //
    //--------------//

    int  Write(int fd);
    int  Read(int fd);
    int  WriteAscii(FILE* ofp);

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

//------------------//
// helper functions //
//------------------//

const char* CurrentTimeString();

#endif
