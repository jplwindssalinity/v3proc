//=========================================================//
// Copyright  (C)1995, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#include <assert.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>

#include "LimitChecker.h"
#include "LimitState.h"

#include "L1AExtract.h"
#include "ParTab.h"
#include "Polynomial.h"
#include "PolyTable.h"

static const char rcs_id_limit_checker_c[] = "@(#) $Id$";

//*********************************************************//
// macros for calculating the offsets for caution low,
// caution high, action low and action high
//*********************************************************//
#define CL_OFFSET(x) (x)
#define CH_OFFSET(x) (x + _bytes)
#define AL_OFFSET(x) (x + 2 * _bytes)
#define AH_OFFSET(x) (x + 3 * _bytes)

#define STATIC_CL_OFFSET(obj,x) (x)
#define STATIC_CH_OFFSET(obj,x) (x + obj->_bytes)
#define STATIC_AL_OFFSET(obj,x) (x + 2 * obj->_bytes)
#define STATIC_AH_OFFSET(obj,x) (x + 3 * obj->_bytes)

//*********************************************************//
//  all read function:                                     //
//         inBuf: is a string where sscanf recognizes      //
//         value: is a user provided spaces for the value  //
//*********************************************************//

char
read_uint1(
char*   inBuf,
void*   value)
{
    unsigned int ival;
    int rc = sscanf(inBuf,"%u", &ival);
    unsigned char* val = (unsigned char*) value;
    *val = (unsigned char) ival;
#ifdef DEBUG
    fprintf(stdout, "read_uint1:(%d)\n", *((unsigned char*)value) );
    fflush(stdout);
#endif
    return(rc==1 ? 1 : 0);
}//read_uint1

char
read_uint2(
char*   inBuf,
void*   value)
{
    unsigned int ival;
    int rc = sscanf(inBuf,"%u", &ival);
    unsigned short* val = (unsigned short*) value;
    *val = (unsigned short) ival;
#ifdef DEBUG
    fprintf(stdout, "read_uint2:(%d)\n", *((unsigned short*)value) );
    fflush(stdout);
#endif
    return(rc==1 ? 1 : 0);
}//read_uint2

char
read_uint4(
char*   inBuf,
void*   value)
{
    int rc = sscanf(inBuf,"%u", (unsigned int*) value);
#ifdef DEBUG
    fprintf(stdout, "read_uint4:(%d)\n", *((unsigned int*)value) );
    fflush(stdout);
#endif
    return(rc==1 ? 1 : 0);
}//read_uint4

char
read_int1(
char*   inBuf,
void*   value)
{
    int ival;
    int rc = sscanf(inBuf,"%d", &ival);
    char* val = (char*) value;
    *val = (char) ival;
#ifdef DEBUG
    fprintf(stdout, "read_int1:(%d)\n", *((char*)value) );
    fflush(stdout);
#endif
    return(rc==1 ? 1 : 0);
}//read_int1

char
read_int2(
char*   inBuf,
void*   value)
{
    int rc = sscanf(inBuf, "%hd", (short*)value);
#ifdef DEBUG
    fprintf(stdout, "read_int2:(%d)\n", *shortP);
    fflush(stdout);
#endif
    return(rc==1 ? 1 : 0);

}//read_int2

char
read_int4(
char*   inBuf,
void*   value)
{
    int rc = sscanf(inBuf,"%d", (int*) value);
#ifdef DEBUG
    fprintf(stdout, "read_int4:(%d)\n", *((int*)value) );
    fflush(stdout);
#endif
    return(rc==1 ? 1 : 0);
}//read_int4

char
read_float(
char*   inBuf,
void*   value)
{
    int rc = sscanf(inBuf,"%f", (float*) value);
#ifdef DEBUG
    fprintf(stdout, "read_float:(%g)\n", *((float*)value) );
    fflush(stdout);
#endif
    return(rc==1 ? 1 : 0);
}//read_float

char
read_double(
char*   inBuf,
void*   value)
{
    return(sscanf(inBuf,"%lf", (double*) value) == 1 ? 1 : 0);
}//read_double

//*********************************************************//
// all ReadLimitLine()s read CL, CH, AL and AH             //
//       from a line of chars                              //
// line should read "Caution:(xx, xx) Action:(xx, xx)      //
//*********************************************************//

char
read_uint1_limits(
char*   line,
void*   aCL,
void*   aCH,
void*   aAL,
void*   aAH)
{
    unsigned int icl, ich, ial, iah;
    int rc = sscanf(line, " Caution:(%u, %u) Action:(%u, %u)",
                                &icl,&ich,&ial,&iah);
    unsigned char* cl = (unsigned char*) aCL;
    unsigned char* ch = (unsigned char*) aCH;
    unsigned char* al = (unsigned char*) aAL;
    unsigned char* ah = (unsigned char*) aAH;
    *cl = (unsigned char) icl;
    *ch = (unsigned char) ich;
    *al = (unsigned char) ial;
    *ah = (unsigned char) iah;

#ifdef DEBUG
    fprintf(stdout, "read_uint1_limits: (%d, %d, %d, %d)\n",
                        *((unsigned char*)aCL), *((unsigned char*)aCH),
                        *((unsigned char*)aAL), *((unsigned char*)aAH));
    fflush(stdout);
#endif //DEBUG

    return (rc == 4 ? 1 : 0);
}//read_uint1_limits

char
read_uint2_limits(
char*   line,
void*   aCL,
void*   aCH,
void*   aAL,
void*   aAH)
{
    unsigned int icl, ich, ial, iah;
    int rc = sscanf(line, " Caution:(%u, %u) Action:(%u, %u)",
                                &icl,&ich,&ial,&iah);
    unsigned short* cl = (unsigned short*) aCL;
    unsigned short* ch = (unsigned short*) aCH;
    unsigned short* al = (unsigned short*) aAL;
    unsigned short* ah = (unsigned short*) aAH;
    *cl = (unsigned short) icl;
    *ch = (unsigned short) ich;
    *al = (unsigned short) ial;
    *ah = (unsigned short) iah;
#ifdef DEBUG
    fprintf(stdout, "read_uint2_limits: (%d, %d, %d, %d)\n",
                        *((unsigned short*)aCL), *((unsigned short*)aCH),
                        *((unsigned short*)aAL), *((unsigned short*)aAH));
    fflush(stdout);
#endif //DEUBG
    return (rc == 4 ? 1 : 0);
}//read_uint2_limits

char
read_uint4_limits(
char*   line,
void*   aCL,
void*   aCH,
void*   aAL,
void*   aAH)
{
    int rc = sscanf(line, " Caution:(%u, %u) Action:(%u, %u)",
                        (unsigned int*)aCL, (unsigned int*)aCH,
                        (unsigned int*)aAL, (unsigned int*)aAH);
#ifdef DEBUG
    fprintf(stdout, "read_uint4_limits: (%d, %d, %d, %d)\n",
                        *((unsigned int*)aCL), *((unsigned int*)aCH),
                        *((unsigned int*)aAL), *((unsigned int*)aAH));
    fflush(stdout);
#endif //DEBUG
    return (rc == 4 ? 1 : 0);
}//read_uint4_limits

char
read_int1_limits(
char*   line,
void*   aCL,
void*   aCH,
void*   aAL,
void*   aAH)
{
    int icl, ich, ial, iah;
    int rc = sscanf(line, " Caution:(%d, %d) Action:(%d, %d)",
                                &icl,&ich,&ial,&iah);
    char* cl = (char*)aCL;
    char* ch = (char*)aCH;
    char* al = (char*)aAL;
    char* ah = (char*)aAH;
    *cl = (char) icl;
    *ch = (char) ich;
    *al = (char) ial;
    *ah = (char) iah;

#ifdef DEBUG
    fprintf(stdout, "read_int1_limits: (%d, %d, %d, %d)\n",
                        *((char*)aCL), *((char*)aCH),
                        *((char*)aAL), *((char*)aAH));
    fflush(stdout);
#endif //DEBUG
    return (rc == 4 ? 1 : 0);
}//read_int1_limits

char
read_int2_limits(
char*   line,
void*   aCL,
void*   aCH,
void*   aAL,
void*   aAH)
{
    int rc = sscanf(line, " Caution:(%hd, %hd) Action:(%hd, %hd)",
                        (short*)aCL, (short*)aCH,
                        (short*)aAL, (short*)aAH );
#ifdef DEBUG
    fprintf(stdout, "read_int2_limits: (%d, %d, %d, %d)\n",
                        *((short*)aCL), *((short*)aCH),
                        *((short*)aAL), *((short*)aAH));
    fflush(stdout);
#endif //DEBUG
    return (rc == 4 ? 1 : 0);
}//read_int2_limits

char
read_int4_limits(
char*   line,
void*   aCL,
void*   aCH,
void*   aAL,
void*   aAH)
{
    int rc = sscanf(line, " Caution:(%d, %d) Action:(%d, %d)",
                        (int*)aCL, (int*)aCH, (int*)aAL, (int*)aAH );

#ifdef DEBUG
    fprintf(stdout, "read_int4_limits: (%d, %d, %d, %d)\n",
                        *((int*)aCL), *((int*)aCH),
                        *((int*)aAL), *((int*)aAH));
    fflush(stdout);
#endif //DEBUG
    return (rc == 4 ? 1 : 0);
}//read_int4_limits

char
read_float4_limits(
char*   line,
void*   aCL,
void*   aCH,
void*   aAL,
void*   aAH)
{
    int rc = sscanf(line, " Caution:(%f, %f) Action:(%f, %f)",
                        (float*)aCL, (float*)aCH, (float*)aAL, (float*)aAH );

#ifdef DEBUG
    fprintf(stdout, "read_float4_limits: (%g, %g, %g, %g)\n",
                        *((float*)aCL), *((float*)aCH),
                        *((float*)aAL), *((float*)aAH));
    fflush(stdout);
#endif//DEBUG
    return (rc == 4 ? 1 : 0);
}//read_float4_limits

char
read_float8_limits(
char*   line,
void*   aCL,
void*   aCH,
void*   aAL,
void*   aAH)
{
    int rc = sscanf(line, " Caution:(%lf, %lf) Action:(%lf, %lf)",
                     (double*)aCL, (double*)aCH, (double*)aAL, (double*)aAH);

#ifdef DEBUG
    fprintf(stdout, "read_float8_limits: (%g, %g, %g, %g)\n",
                        *((double*)aCL), *((double*)aCH),
                        *((double*)aAL), *((double*)aAH));
    fflush(stdout);
#endif//DEBUG
    return (rc == 8 ? 1 : 0);
}//read_float8_limits

//*********************************************************//
// all aEQbFunc returns (a==b) ?                           //
//*********************************************************//

inline char aEQb_uint1(void* a, void* b)
{
    unsigned char* aa = (unsigned char*) a;
    unsigned char* bb = (unsigned char*) b;
    return (*aa == *bb ? 1 : 0);
}//aEQb_uint1

inline char aEQb_uint2(void* a, void* b)
{
    unsigned short* aa = (unsigned short*) a;
    unsigned short* bb = (unsigned short*) b;
    return (*aa == *bb ? 1 : 0);
}//aEQb_uint2

inline char aEQb_uint4(void* a, void* b)
{
    unsigned int* aa = (unsigned int*) a;
    unsigned int* bb = (unsigned int*) b;
    return (*aa == *bb ? 1 : 0);
}//aEQb_uint4

inline char aEQb_int1(void* a, void* b)
{
    char* aa = (char*) a;
    char* bb = (char*) b;
    return (*aa == *bb ? 1 : 0);
}//aEQb_int1

inline char aEQb_int2(void* a, void* b)
{
    short* aa = (short*) a;
    short* bb = (short*) b;
    return (*aa == *bb ? 1 : 0);
}//aEQb_int2

inline char aEQb_int4(void* a, void* b)
{
    int* aa = (int*) a;
    int* bb = (int*) b;
    return (*aa == *bb ? 1 : 0);
}//aEQb_int4

inline char aEQb_float(void* a, void* b)
{
    float* aa = (float*) a;
    float* bb = (float*) b;
    return (*aa == *bb ? 1 : 0);
}//aEQb_float

inline char aEQb_double(void* a, void* b)
{
    return (*((double*)a) == *((double*)b) ? 1 : 0);
}//aEQb_float

//*********************************************************//
// all aGTbFunc returns (a>b) ?                            //
//*********************************************************//

inline char aGTb_uint1(void* a, void* b)
    {   unsigned char* aVal = (unsigned char*) a;
        unsigned char* bVal = (unsigned char*) b;
        return ( *aVal > *bVal ? 1 : 0);
    }
inline char aGTb_uint2(void* a, void* b)
    {   unsigned short* aVal = (unsigned short*) a;
        unsigned short* bVal = (unsigned short*) b;
        return ( *aVal > *bVal ? 1 : 0);
    }
inline char aGTb_uint4(void* a, void* b)
    {   unsigned int* aVal = (unsigned int*) a;
        unsigned int* bVal = (unsigned int*) b;
        return ( *aVal > *bVal ? 1 : 0);
    }
inline char aGTb_int1(void* a, void* b)
    {   char* aVal = (char*) a;
        char* bVal = (char*) b;
        return ( *aVal > *bVal ? 1 : 0);
    }
inline char aGTb_int2(void* a, void* b)
    {   short* aVal = (short*) a;
        short* bVal = (short*) b;
        return ( *aVal > *bVal ? 1 : 0);
    }
inline char aGTb_int4(void* a, void* b)
    {   int* aVal = (int*) a;
        int* bVal = (int*) b;
        return ( *aVal > *bVal ? 1 : 0);
    }
inline char aGTb_float(void* a, void* b)
    {   float* aVal = (float*) a;
        float* bVal = (float*) b;
        return ( *aVal > *bVal ? 1 : 0);
    }
inline char aGTb_double(void* a, void* b)
    {
        return ( *((double*)a) > *((double*)b) ? 1 : 0);
    }


//*********************************************************//
//  all Write function                                     //
//         value: is a value of type XXX                   //
//         inBuf: is a string where sprintf will write to  //
//*********************************************************//

inline char
write_uint1(
char*   string,
void*   value)
{
    unsigned char*  val = (unsigned char*) value;
    int rc = sprintf(string,"%d", *val);
    return (rc > 0 ? 1 : 0);

}//write_uint1

inline char
write_uint2(
char*   string,
void*   value)
{
    unsigned short* val = (unsigned short*) value;
    int rc = sprintf(string,"%d", *val);
    return (rc > 0 ? 1 : 0);
}//write_uint2

inline char
write_uint4(
char*   string,
void*   value)
{
    unsigned int*   val = (unsigned int*) value;
    int rc = sprintf(string,"%d", *val);
    return (rc > 0 ? 1 : 0);
}//write_uint4

inline char
write_int1(
char*   string,
void*   value)
{
    char*   val = (char*) value;
    int rc = sprintf(string,"%d", *val);
    return (rc > 0 ? 1 : 0);
}//write_int1

inline char
write_int2(
char*   string,
void*   value)
{
    short*  val = (short*) value;
    int rc = sprintf(string,"%d", *val);
    return (rc > 0 ? 1 : 0);
}//write_int2

inline char
write_int4(
char*   string,
void*   value)
{
    int*    val = (int*) value;
    int rc = sprintf(string,"%d", *val);
    return (rc > 0 ? 1 : 0);
}//write_int4

inline char
write_float(
char*   string,
void*   value)
{
    float*  val = (float*) value;
    int rc = sprintf(string,"%g", *val);
    return (rc > 0 ? 1 : 0);
}//write_float

inline char
write_double(
char*   string,
void*   value)
{
    return (sprintf(string,"%g", *((double*)value)) > 0 ? 1 : 0);
}//write_double

//*********************************************************//
// all assignBtoAFunc : a = b;                             //
//*********************************************************//

inline void assignBtoA_uint1(void* a, void* b)
{
        unsigned char* aa = (unsigned char*) a;
        *aa = *((unsigned char*)b);
}//assignBtoA_uint1

inline void assignBtoA_uint2(void* a, void* b)
{
        unsigned short* aa = (unsigned short*) a;
        *aa = *((unsigned short*)b);
}//assignBtoA_uint2

inline void assignBtoA_uint4(void* a, void* b)
{
        unsigned int* aa = (unsigned int*) a;
        *aa = *((unsigned int*)b);
}//assignBtoA_uint4

inline void assignBtoA_int1(void* a, void* b)
{
        char* aa = (char*) a;
        *aa = *((char*)b);
}//assignBtoA_int1

inline void assignBtoA_int2(void* a, void* b)
{
        short* aa = (short*) a;
        *aa = *((short*)b);
}//assignBtoA_int2

inline void assignBtoA_int4(void* a, void* b)
{
        int* aa = (int*) a;
        *aa = *((int*)b);
}//assignBtoA_int4

inline void assignBtoA_float(void* a, void* b)
{
        float* aa = (float*) a;
        *aa = *((float*)b);
}//assignBtoA_float

inline void assignBtoA_double(void* a, void* b)
{
        *((double*)a) = *((double*)b);
}//assignBtoA_float

//********************************************************//
//      LimitChecker                                      //
//********************************************************//

const char *enable_map[] = {"Disabled", "Enabled"};

LimitChecker::LimitChecker(
Parameter*  parameter,
char        enable) // enable
:   _readFunc(0), _writeFunc(0), _aGTb(0), _aEQb(0),
    _assignBtoA(0), _readLimitLine(0),
    _parameter(parameter), _enable(enable),
    _mevTime(), _status(LIMIT_OK), _timeParamP(0),
    _maxExceedValue(0), _limits(0), _bytes(0)
{
    (void) LimitChecker::_initialize();

}//LimitChecker::LimitChecker

char
LimitChecker::_initialize(void)
{
    _parameter->data = (char*)malloc(_parameter->byteSize);
    assert(_parameter->data != 0);
    switch (_parameter->dataType)
    {
        case DATA_UINT1:
        case DATA_UINT1_49:
            _maxExceedValue = (void*) new unsigned char;
            _readFunc = read_uint1;
            _writeFunc = write_uint1;
            _aGTb = aGTb_uint1;
            _aEQb = aEQb_uint1;
            _assignBtoA = assignBtoA_uint1;
            _readLimitLine = read_uint1_limits;
            _bytes = 1;
            break;
        case DATA_UINT2:
        case DATA_UINT2_12:
        case DATA_UINT2_76:
        case DATA_UINT2_100:
        case DATA_UINT2_810:
        case DATA_UINT2_3240:
        case DATA_UINT2_100_12:
        case DATA_UINT2_4:
        case DATA_UINT2_5:
        case DATA_UINT2_25:
        case DATA_UINT2_2_8:
            _maxExceedValue = (void*) new unsigned short;
            _readFunc = read_uint2;
            _writeFunc = write_uint2;
            _aGTb = aGTb_uint2;
            _aEQb = aEQb_uint2;
            _assignBtoA = assignBtoA_uint2;
            _readLimitLine = read_uint2_limits;
            _bytes = 2;
            break;
        case DATA_UINT4:
        case DATA_UINT4_4:
        case DATA_UINT4_12:
        case DATA_UINT4_25:
        case DATA_UINT4_100:
        case DATA_UINT4_100_12:
            _maxExceedValue = (void*) new unsigned int;
            _readFunc = read_uint4;
            _writeFunc = write_uint4;
            _aGTb = aGTb_uint4;
            _aEQb = aEQb_uint4;
            _assignBtoA = assignBtoA_uint4;
            _readLimitLine = read_uint4_limits;
            _bytes = 4;
            break;
        case DATA_INT1:
        case DATA_INT1_76:
        case DATA_INT1_810:
        case DATA_INT1_3240:
            _maxExceedValue = (void*) new char;
            _readFunc = read_int1;
            _writeFunc = write_int1;
            _aGTb = aGTb_int1;
            _aEQb = aEQb_int1;
            _assignBtoA = assignBtoA_int1;
            _readLimitLine = read_int1_limits;
            _bytes = 1;
            break;
        case DATA_INT2:
        case DATA_INT2_100:
            _maxExceedValue = (void*) new short;
            _readFunc = read_int2;
            _writeFunc = write_int2;
            _aGTb = aGTb_int2;
            _aEQb = aEQb_int2;
            _assignBtoA = assignBtoA_int2;
            _readLimitLine = read_int2_limits;
            _bytes = 2;
            break;
        case DATA_INT4:
            _maxExceedValue = (void*) new int;
            _readFunc = read_int4;
            _writeFunc = write_int4;
            _aGTb = aGTb_int4;
            _aEQb = aEQb_int4;
            _assignBtoA = assignBtoA_int4;
            _readLimitLine = read_int4_limits;
            _bytes = 4;
            break;
        case DATA_FLOAT4:
        case DATA_FLOAT4_12:
        case DATA_FLOAT4_25:
        case DATA_FLOAT4_76:
        case DATA_FLOAT4_76_4:
        case DATA_FLOAT4_100:
        case DATA_FLOAT4_810:
        case DATA_FLOAT4_3240:
        case DATA_FLOAT4_100_8:
            _maxExceedValue = (void*) new float;
            _readFunc = read_float;
            _writeFunc = write_float;
            _aGTb = aGTb_float;
            _aEQb = aEQb_float;
            _assignBtoA = assignBtoA_float;
            _readLimitLine = read_float4_limits;
            _bytes = 4;
            break;
        case DATA_FLOAT8:
            _maxExceedValue = (void*) new double;
            _readFunc = read_double;
            _writeFunc = write_double;
            _aGTb = aGTb_double;
            _aEQb = aEQb_double;
            _assignBtoA = assignBtoA_double;
            _readLimitLine = read_float8_limits;
            _bytes = 8;
            break;
        default:
            _status = INVALID_PARAMETER;
            return FALSE;
    }
    return TRUE;

}//LimitChecker::_initialize

// default constructor
LimitChecker::LimitChecker()
:   _readFunc(0), _writeFunc(0), _aGTb(0), _aEQb(0),
    _assignBtoA(0), _readLimitLine(0),
    _parameter(0), _enable(0),
    _mevTime(), _status(LIMIT_OK), _timeParamP(0),
    _maxExceedValue(0), _limits(0), _bytes(0)
{
    _maxExceedValue = 0;
    _status = INVALID_PARAMETER;

}//LimitChecker::LimitChecker

// copy constructor
LimitChecker::LimitChecker(
const LimitChecker& other)
:   _enable(other._enable)
{
    //****************************************
    // create its own parameter structure
    // this needs to be done before _initialize() is called
    //****************************************
    _parameter = new Parameter(*(other._parameter));
    (void) LimitChecker::_initialize();

    // now copy other members
    _timeParamP = new Parameter(*(other._timeParamP));
    _status = other._status;
    (void)memcpy(_maxExceedValue, other._maxExceedValue, _bytes);

}//LimitChecker::LimitChecker

LimitChecker::~LimitChecker()
{
    if (_parameter->data != 0)
    {
        free(_parameter->data);
        _parameter->data = 0;
    }
    delete _parameter;
    delete _maxExceedValue;
    delete [] _limits;
    if (_timeParamP) delete _timeParamP;

}//LimitChecker::~LimitChecker

IotBoolean
LimitChecker::OpenParamDataSets(
TlmHdfFile*   tlmFile)
{
    assert(tlmFile != 0);
    if (tlmFile->OpenParamDatasets(_timeParamP) != HdfFile::OK)
        return(0);
    if (tlmFile->OpenParamDatasets(_parameter) != HdfFile::OK)
        return(0);
    return 1;

} //LimitChecker::OpenParamDataSets

IotBoolean
LimitChecker::CloseParamDataSets(
TlmHdfFile*   tlmFile)
{
    assert(tlmFile != 0);
    IotBoolean closeOK = 1;
    if (tlmFile->CloseParamDatasets(_parameter) != HdfFile::OK)
        closeOK = 0;
    if (tlmFile->CloseParamDatasets(_timeParamP) != HdfFile::OK)
        closeOK = 0;

    return(closeOK);

} //LimitChecker::CloseParamDataSets

void
LimitChecker::FinalReport(
FILE*       fp)
{
    // if it was normal, report nothing
    if (_status == LIMIT_OK)
        return;

    char deviateCodeAString[STRING_LEN];
    _mevTime.ItimeToCodeA(deviateCodeAString);

    char maxDevString[STRING_LEN];
    (*_writeFunc)(maxDevString, _maxExceedValue);
    if (_status == CAUTION_LOW)
    {
        (void) fprintf(fp, "   %s %s was below Caution Low limit\n",
            deviateCodeAString, _parameter->paramName);
        // report the max deviation value
        (void) fprintf(fp, "   Lowest value was %s %s\n",
            maxDevString, _parameter->unitName);
    }
    else if (_status == CAUTION_HIGH)
    {
        (void) fprintf(fp, "   %s %s was above Caution High limit\n",
            deviateCodeAString, _parameter->paramName);
        // report the max deviation value
        (void) fprintf(fp, "   Highest value was %s %s\n",
            maxDevString, _parameter->unitName);
    }
    else if (_status == ACTION_LOW)
    {
        (void) fprintf(fp, "   %s %s was below Action Low limit\n",
            deviateCodeAString, _parameter->paramName);
        // report the max deviation value
        (void) fprintf(fp, "   Lowest value was %s %s\n",
            maxDevString, _parameter->unitName);
    }
    else if (_status == ACTION_HIGH)
    {
        (void) fprintf(fp, "   %s %s was above Action High limit\n",
            deviateCodeAString, _parameter->paramName);
        // report the max deviation value
        (void) fprintf(fp, "   Highest value was %s %s\n",
            maxDevString, _parameter->unitName);
    }
    fflush(fp);
    return;
}//LimitChecker::FinalReport

//return TRUE if parameter status is not changed;
// else return FALSE, and the error msg is writen to fp (if any)
LimitStatusE
LimitChecker::CheckFrame(
PolynomialTable*  polyTable,
TlmHdfFile*       tlmFile,
int32             startIndex,
FILE*             fp,         // output file pointer
LimitStatePair*   limitState,
int               firstOnly)
{
    // if is disabled, skip this
    if (! _enable)
        return (LIMIT_OK);

    int numExtracted = _parameter->extractFunc(tlmFile, _parameter->sdsIDs,
                  startIndex, 1, 1, _parameter->data, 0);
    switch(numExtracted)
    {
        case 0:
            return(LIMIT_OK);
        case -1:
            return(EXTRACT_ERROR);
        default:
            break;
    }

    // apply polynomial if this parameter requires it
    if (_parameter->needPolynomial)
    {
        if (polyTable == 0) return (LIMIT_NO_POLYNOMIAL_TABLE);

        char tempString[BIG_SIZE];
        (void)strncpy(tempString, _parameter->sdsNames, BIG_SIZE);
        char* oneSdsName=0;
        oneSdsName = (char*)strtok(tempString, ",");
        if (oneSdsName == 0)
        {
            fprintf(stderr, "Missing SDS name\n");
            return(LIMIT_MISSING_SDS_NAME);
        }
        // is this parameter in the polynomial table?
        const Polynomial* polynomial = polyTable->SelectPolynomial(
                                 oneSdsName, _parameter->unitName);
        // it is in the polynomial table. need to apply to the data
        if (polynomial)
        {
            switch(_bytes)
            {
                case 4:
                {
                    if (_parameter->unitId == UNIT_DB ||
                        _parameter->unitId == UNIT_DBM ||
                        _parameter->unitId == UNIT_DB_DN)
                    {
                        float* numP = (float*) _parameter->data;
                        for (int k=0; k < numExtracted; k++, numP++)
                        {
                        if (*numP == 0.0)
                            *numP = EA_BAD_DB;
                        else
                            polynomial->ApplyReplaceArray(numP, 1);
                        }
                    }
                    else
                        polynomial->ApplyReplaceArray((float*)_parameter->data,
                                                   (int)numExtracted);
                    break;
                }
                case 8:
                {
                    if (_parameter->unitId == UNIT_DB ||
                        _parameter->unitId == UNIT_DBM ||
                        _parameter->unitId == UNIT_DB_DN)
                    {
                        double* numP = (double*) _parameter->data;
                        for (int k=0; k < numExtracted; k++, numP++)
                        {
                        if (*numP == 0.0)
                            *numP = EA_BAD_DB;
                        else
                            polynomial->ApplyReplaceArray(numP, 1);
                        }
                    }
                    else
                        polynomial->ApplyReplaceArray((double*)_parameter->data,
                                                   (int)numExtracted);
                    break;
                }
                default:
                    return(_status = LIMIT_APPLY_POLYNOMIAL_TO_NON_FLOAT);
            }
        }
        else return (LIMIT_POLYNOMIAL_NOT_IN_TABLE);
    }

    char* nextBuf = _parameter->data;
    LimitStatusE newStatus;
    //----------------------------------------------------
    // check the first value only if flag is specified,
    // else check all the values
    //----------------------------------------------------
    if (firstOnly)
    {
        newStatus = _checkValue(limitState, (void*)nextBuf);
        _statePair = limitState;
        _checkStatus(tlmFile, startIndex, fp, newStatus, (void*)nextBuf);
    }
    else
    {
        for (int counter=0; counter < numExtracted; counter++)
        {
            newStatus = _checkValue(limitState, (void*)nextBuf);
            _statePair = limitState;
            _checkStatus(tlmFile, startIndex, fp, newStatus, (void*)nextBuf);
            nextBuf += _bytes;
        }
    }
    return(_status = newStatus);

}//LimitChecker::CheckFrame

//*******************************************************//
//              HK2LimitChecker                         //
//*******************************************************//
HK2LimitChecker::HK2LimitChecker(
Parameter*  parameter,
char        enable) // enable
:   LimitChecker(parameter, enable)
{
    (void)HK2LimitChecker::_initialize();

}//HK2LimitChecker::HK2LimitChecker

// copy constructor
HK2LimitChecker::HK2LimitChecker(
const HK2LimitChecker&     other)
:   LimitChecker(other)
{
    (void)HK2LimitChecker::_initialize();
    (void)memcpy(_limits, other._limits,
        EXT_MODE_COUNT * HVPS_STATE_COUNT * TWTA_COUNT *
        4 * _bytes);

}//HK2LimitChecker::HK2LimitChecker

char
HK2LimitChecker::_initialize(void)
{
    int numElements = HK2LimitStatePair::numStates();
    _timeParamP = ParTabAccess::GetParameter(SOURCE_HK2, UTC_TIME, UNIT_CODE_A);
    if (_timeParamP == 0)
    {
       fprintf(stderr, "HK2LimitChecker: cannot get time parameter\n");
       return 0;
    }
    switch (_parameter->dataType)
    {
        case DATA_UINT1:
        case DATA_UINT1_49:
            _limits = (void*)new unsigned char [numElements];
            (void)memset(_limits, 0, numElements * sizeof(unsigned char));
            break;
        case DATA_UINT2:
        case DATA_UINT2_12:
        case DATA_UINT2_76:
        case DATA_UINT2_100:
        case DATA_UINT2_810:
        case DATA_UINT2_3240:
        case DATA_UINT2_100_12:
        case DATA_UINT2_4:
        case DATA_UINT2_5:
        case DATA_UINT2_25:
        case DATA_UINT2_2_8:
            _limits = (void*)new unsigned short [numElements];
            (void)memset(_limits, 0, numElements * sizeof(unsigned short));
            break;
        case DATA_UINT4:
        case DATA_UINT4_4:
        case DATA_UINT4_12:
        case DATA_UINT4_25:
        case DATA_UINT4_100:
        case DATA_UINT4_100_12:
            _limits = (void*)new unsigned int [numElements];
            (void)memset(_limits, 0, numElements * sizeof(unsigned int));
            break;
        case DATA_INT1:
        case DATA_INT1_76:
        case DATA_INT1_810:
        case DATA_INT1_3240:
            _limits = (void*)new char [numElements];
            (void)memset(_limits, 0, numElements * sizeof(char));
            break;
        case DATA_INT2:
        case DATA_INT2_100:
            _limits = (void*)new short [numElements];
            (void)memset(_limits, 0, numElements * sizeof(short));
            break;
        case DATA_INT4:
            _limits = (void*)new int [numElements];
            (void)memset(_limits, 0, numElements * sizeof(int));
            break;
        case DATA_FLOAT4:
        case DATA_FLOAT4_12:
        case DATA_FLOAT4_25:
        case DATA_FLOAT4_76:
        case DATA_FLOAT4_76_4:
        case DATA_FLOAT4_100:
        case DATA_FLOAT4_810:
        case DATA_FLOAT4_3240:
        case DATA_FLOAT4_100_8:
            _limits = (void*)new float [numElements];
            (void)memset(_limits, 0, numElements * sizeof(float));
            break;
        case DATA_FLOAT8:
            _limits = (void*)new double [numElements];
            (void)memset(_limits, 0, numElements * sizeof(double));
            break;
        default:
            _status = INVALID_PARAMETER;
            return 0;
    }
    return 1;

}//HK2LimitChecker::_initialize

// print the limits as a long string
// user needs to provide the space
char
HK2LimitChecker::PrintText(
char*       destString)
{
    if (destString == 0)
        return 0;

    char temp_string[BUFSIZ];
    int unique_count = 0;
    int mode, twt, twta, match, i;
    const int maxIndex = EXT_MODE_COUNT*HVPS_STATE_COUNT*TWTA_COUNT;

    *destString = '\0';

    /*-----------------------------------------*/
    /* create an array of unique limit 4-packs */
    /*-----------------------------------------*/
    void* unique_4pack[maxIndex][4];
    for (i=0; i < maxIndex; i++)
    {
        unique_4pack[i][0] = AllocOneValue();
        unique_4pack[i][1] = AllocOneValue();
        unique_4pack[i][2] = AllocOneValue();
        unique_4pack[i][3] = AllocOneValue();
    }

    //=============================================
    // identify all the unique limits first
    //=============================================
    for (mode = 0; mode < EXT_MODE_COUNT; mode++)
        for (twt = 0; twt < HVPS_STATE_COUNT; twt++)
            for (twta = 0; twta < TWTA_COUNT; twta++)
                {
                    match = 0;
                    const char* offset = (char*)_limits +
                                 (mode * HVPS_STATE_COUNT * TWTA_COUNT +
                                  twt * TWTA_COUNT + twta) * 4 * _bytes;
                    for (i = 0; i < unique_count; i++)
                    {
                        if ( (*_aEQb) (unique_4pack[i][0],
                                        (void*)CL_OFFSET(offset)) &&
                            (*_aEQb) (unique_4pack[i][1],
                                        (void*)CH_OFFSET(offset)) &&
                            (*_aEQb) (unique_4pack[i][2],
                                        (void*)AL_OFFSET(offset)) &&
                            (*_aEQb) (unique_4pack[i][3],
                                        (void*)AH_OFFSET(offset)) )
                        {
                            match = 1;
                            break;  /* it's been found already */
                        }
                    }
                    if (match == 0)
                    {   /* new 4-pack */
                        if (unique_count >= maxIndex)
                            return 0;

                        (void)(*_assignBtoA)(unique_4pack[unique_count][0],
                                                (void*)CL_OFFSET(offset));
                        (void)(*_assignBtoA)(unique_4pack[unique_count][1],
                                                (void*)CH_OFFSET(offset));
                        (void)(*_assignBtoA)(unique_4pack[unique_count][2],
                                                (void*)AL_OFFSET(offset));
                        (void)(*_assignBtoA)(unique_4pack[unique_count][3],
                                                (void*)AH_OFFSET(offset));
                        unique_count++;
                    }//if

                }

    /*-------------------------------------*/
    /* print out the parameter information */
    /*-------------------------------------*/

    (void) sprintf(temp_string, "[%s] [%s] [%s]",
        _parameter->paramName, _parameter->unitName, enable_map[_enable]);
    (void) strcat(destString, temp_string);

    //================================================
    // print out the unique limit values
    //================================================
    char clString[STRING_LEN];
    char chString[STRING_LEN];
    char alString[STRING_LEN];
    char ahString[STRING_LEN];
    for (i = 0; i < unique_count; i++)
    {
        (void) (*_writeFunc)(clString, unique_4pack[i][0]);
        (void) (*_writeFunc)(chString, unique_4pack[i][1]);
        (void) (*_writeFunc)(alString, unique_4pack[i][2]);
        (void) (*_writeFunc)(ahString, unique_4pack[i][3]);
        sprintf(temp_string, "\nCaution:(%s, %s)  Action:(%s, %s)",
                            clString, chString, alString, ahString);
        strcat(destString, temp_string);

        //================================================
        /* print out the matched limit conditions */
        //================================================

        for (mode = 0; mode < EXT_MODE_COUNT; mode++)
          for (twt = 0; twt < HVPS_STATE_COUNT; twt++)
              for (twta = 0; twta < TWTA_COUNT; twta++)
              {
                    const char* offset = (char*)_limits +
                          (mode * HVPS_STATE_COUNT * TWTA_COUNT +
                                twt * TWTA_COUNT + twta) * 4 * _bytes;
                    if ((*_aEQb)(unique_4pack[i][0],
                                    (void*)CL_OFFSET(offset)) &&
                        (*_aEQb)(unique_4pack[i][1],
                                    (void*)CH_OFFSET(offset)) &&
                        (*_aEQb)(unique_4pack[i][2],
                                    (void*)AL_OFFSET(offset)) &&
                        (*_aEQb)(unique_4pack[i][3],
                                    (void*)AH_OFFSET(offset)) )
                    {
                        sprintf(temp_string,
                            "\nMode:%-3s  TWT:%-3s  TWTA:%s",
                            ext_mode_map[mode], twt_map[twt], twta_map[twta]);
                        strcat(destString, temp_string);
                    }
              }
    }
    for (i=0; i < maxIndex; i++)
    {
        free(unique_4pack[i][0]);
        free(unique_4pack[i][1]);
        free(unique_4pack[i][2]);
        free(unique_4pack[i][3]);
    }
    return 1;
}//HK2LimitChecker::PrintText


//*******************************************************//
//                 L1ALimitChecker                        //
//*******************************************************//
L1ALimitChecker::L1ALimitChecker(
Parameter*  parameter,
char        enable) // enable
:   LimitChecker(parameter, enable)
{
    (void)L1ALimitChecker::_initialize();

}//L1ALimitChecker::L1ALimitChecker

// copy constructor
L1ALimitChecker::L1ALimitChecker(
const L1ALimitChecker&       other)
:   LimitChecker(other)
{
    (void)L1ALimitChecker::_initialize();
    (void)memcpy(_limits, other._limits,
        NSCAT_MODE_COUNT * HVPS_STATE_COUNT * TWTA_COUNT * 
        FRAME_TYPE_COUNT * 4 * _bytes);

}//L1ALimitChecker::L1ALimitChecker

char
L1ALimitChecker::_initialize(void)
{
    int numElements = L1ALimitStatePair::numStates();
    _timeParamP = ParTabAccess::GetParameter(SOURCE_L1A, UTC_TIME, UNIT_CODE_A);
    if (_timeParamP == 0)
    {
       fprintf(stderr, "L1ALimitChecker: cannot get time parameter\n");
       return 0;
    }

    switch (_parameter->dataType)
    {
        case DATA_UINT1:
        case DATA_UINT1_49:
            _limits = (void*)new unsigned char [numElements];
            (void)memset(_limits, 0, numElements * sizeof(unsigned char));
            break;
        case DATA_UINT2:
        case DATA_UINT2_12:
        case DATA_UINT2_76:
        case DATA_UINT2_100:
        case DATA_UINT2_810:
        case DATA_UINT2_3240:
        case DATA_UINT2_100_12:
        case DATA_UINT2_4:
        case DATA_UINT2_5:
        case DATA_UINT2_25:
        case DATA_UINT2_2_8:
            _limits = (void*)new unsigned short [numElements];
            (void)memset(_limits, 0, numElements * sizeof(unsigned short));
            break;
        case DATA_UINT4:
        case DATA_UINT4_4:
        case DATA_UINT4_12:
        case DATA_UINT4_25:
        case DATA_UINT4_100:
        case DATA_UINT4_100_12:
            _limits = (void*)new unsigned int [numElements];
            (void)memset(_limits, 0, numElements * sizeof(unsigned int));
            break;
        case DATA_INT1:
        case DATA_INT1_76:
        case DATA_INT1_810:
        case DATA_INT1_3240:
            _limits = (void*)new char [numElements];
            (void)memset(_limits, 0, numElements * sizeof(char));
            break;
        case DATA_INT2:
        case DATA_INT2_100:
            _limits = (void*)new short [numElements];
            (void)memset(_limits, 0, numElements * sizeof(short));
            break;
        case DATA_INT4:
            _limits = (void*)new int [numElements];
            (void)memset(_limits, 0, numElements * sizeof(int));
            break;
        case DATA_FLOAT4:
        case DATA_FLOAT4_12:
        case DATA_FLOAT4_25:
        case DATA_FLOAT4_76:
        case DATA_FLOAT4_76_4:
        case DATA_FLOAT4_100:
        case DATA_FLOAT4_810:
        case DATA_FLOAT4_3240:
        case DATA_FLOAT4_100_8:
            _limits = (void*)new float [numElements];
            (void)memset(_limits, 0, numElements * sizeof(float));
            break;
        case DATA_FLOAT8:
            _limits = (void*)new double [numElements];
            (void)memset(_limits, 0, numElements * sizeof(double));
            break;
        default:
            _status = INVALID_PARAMETER;
            return 0;
    }
    return 1;

}//L1ALimitChecker::_initialize

LimitStatusE
LimitChecker::_checkValue(
LimitStatePair*     limitState,
void*               value)
{
    // calculate the bytes offset first
    const char* offset = (char*)_limits + limitState->offset() * _bytes;

    //============================================================
    // check if the value is within normal range first. (most cases)
    // if value is between caution low and high, it is normal
    //============================================================
    if ( ((*_aEQb) (value, (void*)CL_OFFSET(offset)) ||
        (*_aGTb) (value, (void*)CL_OFFSET(offset)))
        &&
        ((*_aEQb) ((void*)CH_OFFSET(offset), value) ||
        (*_aGTb) ((void*)CH_OFFSET(offset), value)) )
        return LIMIT_OK;
    else if ( (*_aGTb) ((void*)AL_OFFSET(offset), value))
        return ACTION_LOW;
    else if ( (*_aGTb) (value, (void*)AH_OFFSET(offset)))
        return ACTION_HIGH;
    else if ( (*_aGTb) ((void*)CL_OFFSET(offset), value))
        return CAUTION_LOW;
    else
        return CAUTION_HIGH;

}//LimitChecker::_checkValue

// print the limits as a long string
// user needs to provide the space
char
L1ALimitChecker::PrintText(
char*       destString)
{
    if (destString == 0)
        return 0;

    char temp_string[BUFSIZ];
    int unique_count = 0;
    int mode, twt, twta, frame, match, i;
    const int maxIndex = NSCAT_MODE_COUNT * HVPS_STATE_COUNT *
                                    TWTA_COUNT * FRAME_TYPE_COUNT;

    *destString = '\0';

    /*-----------------------------------------*/
    /* create an array of unique limit 4-packs */
    /*-----------------------------------------*/
    void* unique_4pack[maxIndex][4];
    for (i=0; i < maxIndex; i++)
    {
        unique_4pack[i][0] = AllocOneValue();
        unique_4pack[i][1] = AllocOneValue();
        unique_4pack[i][2] = AllocOneValue();
        unique_4pack[i][3] = AllocOneValue();
    }

    //=============================================
    // identify all the unique limits first
    //=============================================
    for (mode = 0; mode < NSCAT_MODE_COUNT; mode++)
        for (twt = 0; twt < HVPS_STATE_COUNT; twt++)
            for (twta = 0; twta < TWTA_COUNT; twta++)
                for (frame = 0; frame < FRAME_TYPE_COUNT; frame++)
                {
                    match = 0;
                    const char* offset = (char*)_limits +
                      (mode * HVPS_STATE_COUNT * TWTA_COUNT * FRAME_TYPE_COUNT+
                      twt * TWTA_COUNT * FRAME_TYPE_COUNT +
                      twta * FRAME_TYPE_COUNT + frame) * 4 * _bytes;

                    for (i = 0; i < unique_count; i++)
                    {
                        if ((*_aEQb) (unique_4pack[i][0],
                                        (void*)CL_OFFSET(offset)) &&
                            (*_aEQb) (unique_4pack[i][1],
                                        (void*)CH_OFFSET(offset)) &&
                            (*_aEQb) (unique_4pack[i][2],
                                        (void*)AL_OFFSET(offset)) &&
                            (*_aEQb) (unique_4pack[i][3],
                                        (void*)AH_OFFSET(offset)) )
                        {
                            match = 1;
                            break;  /* it's been found already */
                        }
                    }
                    if (match == 0)
                    {   /* new 4-pack */
                        if (unique_count >= maxIndex)
                            return 0;

                        (void)(*_assignBtoA)(unique_4pack[unique_count][0],
                                            (void*)CL_OFFSET(offset));
                        (void)(*_assignBtoA)(unique_4pack[unique_count][1],
                                            (void*)CH_OFFSET(offset));
                        (void)(*_assignBtoA)(unique_4pack[unique_count][2],
                                            (void*)AL_OFFSET(offset));
                        (void)(*_assignBtoA)(unique_4pack[unique_count][3],
                                            (void*)AH_OFFSET(offset));
                        unique_count++;
                    }//if

                }

    /*-------------------------------------*/
    /* print out the parameter information */
    /*-------------------------------------*/

    (void) sprintf(temp_string, "[%s] [%s] [%s]",
        _parameter->paramName, _parameter->unitName, enable_map[_enable]);
    (void) strcat(destString, temp_string);

    //================================================
    // print out the unique limit values
    //================================================
    char clString[STRING_LEN];
    char chString[STRING_LEN];
    char alString[STRING_LEN];
    char ahString[STRING_LEN];
    for (i = 0; i < unique_count; i++)
    {
        (void) (*_writeFunc)(clString, unique_4pack[i][0]);
        (void) (*_writeFunc)(chString, unique_4pack[i][1]);
        (void) (*_writeFunc)(alString, unique_4pack[i][2]);
        (void) (*_writeFunc)(ahString, unique_4pack[i][3]);
        sprintf(temp_string, "\nCaution:(%s, %s)  Action:(%s, %s)",
                            clString, chString, alString, ahString);
        strcat(destString, temp_string);

        //================================================
        /* print out the matched limit conditions */
        //================================================

        for (mode = 0; mode < NSCAT_MODE_COUNT; mode++)
          for (twt = 0; twt < HVPS_STATE_COUNT; twt++)
            for (twta = 0; twta < TWTA_COUNT; twta++)
              for (frame = 0; frame < FRAME_TYPE_COUNT; frame++)
              {
                  const char* offset = (char*)_limits +
                    (mode * HVPS_STATE_COUNT * TWTA_COUNT * FRAME_TYPE_COUNT +
                     twt * TWTA_COUNT * FRAME_TYPE_COUNT +
                     twta * FRAME_TYPE_COUNT + frame) * 4 * _bytes;
                  if ((*_aEQb)(unique_4pack[i][0],
                                            (void*)CL_OFFSET(offset)) &&
                        (*_aEQb)(unique_4pack[i][1],
                                            (void*)CH_OFFSET(offset)) &&
                        (*_aEQb)(unique_4pack[i][2],
                                            (void*)AL_OFFSET(offset)) &&
                        (*_aEQb)(unique_4pack[i][3],
                                            (void*)AH_OFFSET(offset)) )
                  {
                      sprintf(temp_string,
                            "\nMode:%-3s  TWT:%-3s  TWTA:%-3s  Frame:%-3s",
                            mode_map[mode], twt_map[twt],
                            twta_map[twta], cmf_map[frame]);
                      strcat(destString, temp_string);
                  }
              }
    }
    for (i=0; i < maxIndex; i++)
    {
        free(unique_4pack[i][0]);
        free(unique_4pack[i][1]);
        free(unique_4pack[i][2]);
        free(unique_4pack[i][3]);
    }
    return 1;
}//L1ALimitChecker::PrintText

L1ADrvLimitChecker::L1ADrvLimitChecker(
Parameter*      parameter,
char            enable)
: L1ALimitChecker(parameter, enable)
{
  // empty
} // L1ADrvLimitChecker::L1ADrvLimitChecker

// copy constructor
L1ADrvLimitChecker::L1ADrvLimitChecker(
const L1ADrvLimitChecker&       other)
:   L1ALimitChecker(other)
{
  // empty
}//L1ALimitChecker::L1ALimitChecker

//return TRUE if parameter status is not changed;
// else return FALSE, and the error msg is writen to fp (if any)
LimitStatusE
L1ADrvLimitChecker::CheckFrame(
PolynomialTable*  polyTable,
TlmHdfFile*       tlmFile,
int32             startIndex,
FILE*             fp,         // output file pointer
LimitStatePair*   limitState,
int               firstOnly)
{
    // if is disabled, skip this
    if (! _enable)
        return (LIMIT_OK);

    DerivedExtractResult extractResult;
    extractResult.dataBuf =
                 new char [MAX_NUM_DERIVED_VALUES * (_parameter->byteSize)];
    int numExtracted = 0;
    if (_parameter->paramId == UTC_TIME)
        numExtracted = _parameter->extractFunc(tlmFile, _parameter->sdsIDs,
                   startIndex, 1, 1, (void*)extractResult.dataBuf, polyTable);
    else
        numExtracted = _parameter->extractFunc(tlmFile, _parameter->sdsIDs,
                   startIndex, 1, 1, &extractResult, polyTable);
    switch(numExtracted)
    {
        case 0:
            return(LIMIT_OK);
        case -1:
            return(EXTRACT_ERROR);
        default:
            break;
    }

    // apply polynomial if this parameter requires it
    if (_parameter->needPolynomial)
    {
        if (polyTable == 0) return (LIMIT_NO_POLYNOMIAL_TABLE);

        char tempString[BIG_SIZE];
        (void)strncpy(tempString, _parameter->sdsNames, BIG_SIZE);
        char* oneSdsName=0;
        oneSdsName = (char*)strtok(tempString, ",");
        if (oneSdsName == 0)
        {
            fprintf(stderr, "Missing SDS name\n");
            return(LIMIT_MISSING_SDS_NAME);
        }
        // is this parameter in the polynomial table?
        const Polynomial* polynomial = polyTable->SelectPolynomial(
                                 oneSdsName, _parameter->unitName);
        // it is in the polynomial table. need to apply to the data
        if (polynomial)
        {
            switch(_bytes)
            {
                case 4:
                {
                    float* nextBuf=0;
                    for (int k=0; k < MAX_NUM_DERIVED_VALUES; k++)
                    {
                        if (extractResult.validDataMap[k])
                        {
                            nextBuf = (float*)extractResult.dataBuf + k*_bytes;
                            if ((_parameter->unitId == UNIT_DB ||
                                _parameter->unitId == UNIT_DBM ||
                                _parameter->unitId == UNIT_DB_DN) &&
                                                       *nextBuf == 0.0)
                                *nextBuf = EA_BAD_DB;
                            else
                                polynomial->ApplyReplaceArray(nextBuf, 1);
                        }
                    }
                    break;
                }
                case 8:
                {
                    double* nextBuf=0;
                    for (int k=0; k < MAX_NUM_DERIVED_VALUES; k++)
                    {
                        if (extractResult.validDataMap[k])
                        {
                            nextBuf = (double*)extractResult.dataBuf +
                                                k * _bytes;
                            if ((_parameter->unitId == UNIT_DB ||
                                _parameter->unitId == UNIT_DBM ||
                                _parameter->unitId == UNIT_DB_DN) &&
                                                       *nextBuf == 0.0)
                                *nextBuf = EA_BAD_DB;
                            else
                                polynomial->ApplyReplaceArray(nextBuf, 1);
                        }
                    }
                    break;
                }
                default:
                    return(_status = LIMIT_APPLY_POLYNOMIAL_TO_NON_FLOAT);
            }
        }
        else return (LIMIT_POLYNOMIAL_NOT_IN_TABLE);
    }

    char* nextBuf = 0;
    LimitStatusE newStatus;
    //----------------------------------------------------
    // check the first value only if flag is specified,
    // else check all the values
    //----------------------------------------------------
    if (firstOnly)
    {
        for (int counter=0; counter < MAX_NUM_DERIVED_VALUES; counter++)
        {
            if (extractResult.validDataMap[counter])
            {
                nextBuf = extractResult.dataBuf + counter * _bytes;
                newStatus = _checkValue(limitState, (void*)nextBuf);
                _statePair = limitState;
                _checkStatus(tlmFile, startIndex, fp,newStatus,(void*)nextBuf);
                break;
            }
        }
    }
    else
    {
        for (int counter=0; counter < MAX_NUM_DERIVED_VALUES; counter++)
        {
            if (extractResult.validDataMap[counter])
            {
                nextBuf = extractResult.dataBuf + counter * _bytes;
                newStatus = _checkValue(limitState, (void*)nextBuf);
                _statePair = limitState;
                _checkStatus(tlmFile, startIndex, fp,newStatus,(void*)nextBuf);
            }
        }
    }
    return(_status = newStatus);

}//L1ADrvLimitChecker::CheckFrame

void
LimitChecker::SetLimits(
LimitStatePair*     limitState,
void*               cl,
void*               ch,
void*               al,
void*               ah)
{
    const char* offset = (char*)_limits + limitState->offset() * _bytes;
    (*_assignBtoA) ((void*)CL_OFFSET(offset), cl);
    (*_assignBtoA) ((void*)CH_OFFSET(offset), ch);
    (*_assignBtoA) ((void*)AL_OFFSET(offset), al);
    (*_assignBtoA) ((void*)AH_OFFSET(offset), ah);

}//LimitChecker::SetLimits

int operator!=(const LimitChecker& a, const LimitChecker& b)
{
    return(a == b ? 0 : 1);
}//operator!=(const LimitChecker& a, const LimitChecker& b)

int
operator==(const L1ALimitChecker& a, const L1ALimitChecker& b)
{
    return ((const LimitChecker&)a == (const LimitChecker&)b &&
                memcmp(a._limits, b._limits, NSCAT_MODE_COUNT *
                        HVPS_STATE_COUNT * TWTA_COUNT * FRAME_TYPE_COUNT *
                        4 * a._bytes) == 0 ? 1 : 0);

}//operator==(const L1ALimitChecker& a, const L1ALimitChecker& b)

int operator!=(const L1ALimitChecker& a, const L1ALimitChecker& b)
{
    return(a == b ? 0 : 1);
}//operator!=(const L1ALimitChecker& a, const L1ALimitChecker& b)


int
operator==(const HK2LimitChecker& a, const HK2LimitChecker& b)
{
    return ((const LimitChecker&)a == (const LimitChecker&)b &&
                memcmp(a._limits, b._limits, EXT_MODE_COUNT *
                        HVPS_STATE_COUNT * TWTA_COUNT *
                        4 * a._bytes) == 0 ? 1 : 0);

}//operator==(const HK2LimitChecker& a, const HK2LimitChecker& b)

int operator!=(const HK2LimitChecker& a, const HK2LimitChecker& b)
{
    return(a == b ? 0 : 1);
}//operator!=(const HK2LimitChecker& a, const HK2LimitChecker& b)



//*******************************************************************//
// The following are functions and tables for alarm reporting:
//*******************************************************************//

void
timeNParam(
LimitChecker*   obj,
FILE*           fp,
Itime&          itime)
{
    char timeString[STRING_LEN];
    itime.ItimeToCodeA(timeString);
    fprintf(fp, "\n%s %s\n", timeString, obj->_parameter->paramName);

}//timeNParam

void
highMEV(
LimitChecker*   obj,
FILE*       fp,
char        printOldStateMsg)   // 1 if "in old state " msg is wanted
{
    // get time and value strings for max exceed value
    char timeString[STRING_LEN];
    obj->_mevTime.ItimeToCodeA(timeString);
    char valueString[STRING_LEN];
    (void) (*(obj->_writeFunc)) (valueString, obj->_maxExceedValue);

    if (printOldStateMsg == 1)
        fprintf(fp, "   Highest value in old state was %s %s at %s\n",
                valueString, obj->_parameter->unitName, timeString);
    else
        fprintf(fp, "   Highest value was %s %s at %s\n",
                valueString, obj->_parameter->unitName, timeString);

}//highMEV

void
lowMEV(
LimitChecker*   obj,
FILE*           fp,
char            printOldStateMsg)   // 1 if "in old state " msg is wanted
{
    // get time and value strings for max exceed value
    char timeString[STRING_LEN];
    obj->_mevTime.ItimeToCodeA(timeString);
    char valueString[STRING_LEN];
    (void) (*(obj->_writeFunc)) (valueString, obj->_maxExceedValue);

    if (printOldStateMsg == 1)
        fprintf(fp, "   Lowest value in old state was %s %s at %s\n",
                valueString, obj->_parameter->unitName, timeString);
    else
        fprintf(fp, "   Lowest value was %s %s at %s\n",
                valueString, obj->_parameter->unitName, timeString);

}//lowMEV

void
updateMEV(
LimitChecker*   obj,
void*           newValue,
Itime&          itime)
{
    if (obj->_status == ACTION_LOW || obj->_status == CAUTION_LOW)
    {
        if ( (*(obj->_aGTb)) (obj->_maxExceedValue, newValue))
        {
            (void) (*(obj->_assignBtoA)) (obj->_maxExceedValue, newValue);
            obj->_mevTime = itime;
        }
    }
    else
    {
        if ( (*(obj->_aGTb)) (newValue, obj->_maxExceedValue))
        {
            (void)(*(obj->_assignBtoA)) (obj->_maxExceedValue, newValue);
            obj->_mevTime = itime;
        }
    }

#ifdef DEBUG
    char newValString[STRING_LEN];
    (void) (*(obj->_writeFunc)) (newValString, newValue);
    char mevString[STRING_LEN];
    (void) (*(obj->_writeFunc)) (mevString, obj->_maxExceedValue);
    printf("new value = %s, MEV = %s\n", newValString, mevString);
    fflush(stdout);
#endif

}//updateMEV

void
setMEV(
LimitChecker*   obj,
void*           newValue,
Itime&          itime)
{
    (*(obj->_assignBtoA)) (obj->_maxExceedValue, newValue);
    obj->_mevTime = itime;
#ifdef DEBUG
    char newValString[STRING_LEN];
    (void) (*(obj->_writeFunc)) (newValString, newValue);
    char mevString[STRING_LEN];
    (void) (*(obj->_writeFunc)) (mevString, obj->_maxExceedValue);
    printf("new value = %s, MEV = %s\n", newValString, mevString);
    fflush(stdout);
#endif

}//setMEV

void
clearMEV(
LimitChecker*   obj,
void*           ,       // dummy
Itime&          itime)  // dummy
{
    if (obj->_statePair)
    {
        const char* offset = (char*)obj->_limits +
                                    obj->_statePair->offset() * obj->_bytes;
        (void)(*(obj->_assignBtoA)) (obj->_maxExceedValue,
                                    (void*)CL_OFFSET(offset));
    }
    obj->_mevTime = itime;
}//clearMEV

void
printState(
LimitChecker*   obj,
FILE*           fp)
{
    fprintf(fp, "   ");
    obj->_statePair->PrintNewState(fp);

}//printLimitState

void
printStateChg(
LimitChecker*   obj,
FILE*           fp)
{
    fprintf(fp, "   ");
    obj->_statePair->PrintChange(fp);

}//printStateChg

const char* alarmString[] =
{
    "ACTION LOW",
    "CAUTION LOW",
    "OK",
    "CAUTION HIGH",
    "ACTION HIGH"
};

const char* minorAlarmString[] =
{
    "Action Low",
    "Caution Low",
    "OK",
    "Caution High",
    "Action High"
};

inline
int
LimitChecker::_getIndex(
LimitStatusE    alarmState)
{
    switch(alarmState)
    {
        case ACTION_LOW:
            return 0;
        case CAUTION_LOW:
            return 1;
        case LIMIT_OK:
            return 2;
        case CAUTION_HIGH:
            return 3;
        case ACTION_HIGH:
            return 4;
        default:
            return -1;
    }
}//LimitChecker::_getIndex

void
printEnterAlarm(
LimitChecker*       obj,
FILE*               fp,
void*               value,
LimitStatusE        newAlarm)
{
    char valueString[STRING_LEN];
    (void) (*(obj->_writeFunc)) (valueString, value);
    int newAlarmIndex = obj->_getIndex(newAlarm);
    fprintf(fp, "   Alarm State = %s, Parameter Value = %s %s\n",
                        alarmString[newAlarmIndex], valueString,
                        obj->_parameter->unitName);

    char thresholdString[STRING_LEN];

    const char* offset = (char*)obj->_limits +
                            obj->_statePair->offset() * obj->_bytes;
    switch (newAlarm)
    {
        case CAUTION_LOW:
            (void) (*(obj->_writeFunc)) (thresholdString,
                                    (void*)STATIC_CL_OFFSET(obj,offset));
            break;
        case CAUTION_HIGH:
            (void) (*(obj->_writeFunc)) (thresholdString,
                                    (void*)STATIC_CH_OFFSET(obj,offset));
            break;
        case ACTION_LOW:
            (void) (*(obj->_writeFunc)) (thresholdString,
                                    (void*)STATIC_AL_OFFSET(obj,offset));
            break;
        case ACTION_HIGH:
            (void) (*(obj->_writeFunc)) (thresholdString,
                                    (void*)STATIC_AH_OFFSET(obj,offset));
            break;
        default:
            break;
    }
    if (newAlarm == CAUTION_HIGH || newAlarm == ACTION_HIGH)
        fprintf(fp, "   Above the %s limit of %s %s\n",
                            minorAlarmString[newAlarmIndex],
                            thresholdString, obj->_parameter->unitName);
    else
        fprintf(fp, "   Below the %s limit of %s %s\n",
                            minorAlarmString[newAlarmIndex],
                            thresholdString, obj->_parameter->unitName);

}//printEnterAlarm

void
printReturnAlarm(
LimitChecker*       obj,
FILE*               fp,
void*               value,
LimitStatusE        newAlarm)
{
    char valueString[STRING_LEN];
    (void) (*(obj->_writeFunc)) (valueString, value);
    int newAlarmIndex = obj->_getIndex(newAlarm);
    fprintf(fp, "   Alarm State = %s, Parameter Value = %s %s\n",
                alarmString[newAlarmIndex], valueString,
                obj->_parameter->unitName);

    char thresholdString[STRING_LEN];
    int oldAlarmIndex= obj->_getIndex(obj->_status);
    const char* offset = (char*)obj->_limits +
                             obj->_statePair->offset() * obj->_bytes;
    switch (obj->_status)
    {
        case CAUTION_LOW:
            (void) (*(obj->_writeFunc)) (thresholdString,
                                        (void*)STATIC_CL_OFFSET(obj,offset));
            break;
        case CAUTION_HIGH:
            (void) (*(obj->_writeFunc)) (thresholdString,
                                        (void*)STATIC_CH_OFFSET(obj,offset));
            break;
        case ACTION_LOW:
            (void) (*(obj->_writeFunc)) (thresholdString,
                                        (void*)STATIC_AL_OFFSET(obj,offset));
            break;
        case ACTION_HIGH:
            (void) (*(obj->_writeFunc)) (thresholdString,
                                        (void*)STATIC_AH_OFFSET(obj,offset));
            break;
        default:
            break;
    }
    if (obj->_status == CAUTION_LOW || obj->_status == ACTION_LOW)
        fprintf(fp, "   Returned above the %s limit of %s %s\n",
                            minorAlarmString[oldAlarmIndex],
                            thresholdString, obj->_parameter->unitName);
    else
        fprintf(fp, "   Returned below the %s limit of %s %s\n",
                            minorAlarmString[oldAlarmIndex],
                            thresholdString, obj->_parameter->unitName);

}//printReturnAlarm

void
printOK(
LimitChecker*       obj,
FILE*               fp,
void*               value,
LimitStatusE)
{
    char valueString[STRING_LEN];
    (void) (*(obj->_writeFunc)) (valueString, value);
    fprintf(fp, "   Alarm State = OK, Parameter Value = %s %s\n",
                            valueString, obj->_parameter->unitName);
    fprintf(fp, "   The value in the new state is OK\n");

}//printOK

typedef void (*TimeNParam) (LimitChecker*, FILE*, Itime& itime);
typedef void (*PrintMEV) (LimitChecker*, FILE*, char printOldStateMsg);
typedef void (*UpdateMEV) (LimitChecker*, void* value, Itime& itime);
typedef void (*PrintState) (LimitChecker*, FILE*);
typedef void (*PrintAlarmStatus) (LimitChecker*, FILE*, void* value, LimitStatusE newAlarm);

struct AlarmPrintEntry
{
    TimeNParam          timeNParam;
    PrintMEV            printMEV;
    UpdateMEV           updateMEV;
    PrintState          printState;
    PrintAlarmStatus    printAlarmStatus;
};

const AlarmPrintEntry noChangeOpsTable[5][5] =
{
    {
        { 0,            0,      updateMEV,  0,          0 },
        { timeNParam,   lowMEV, 0,          printState, printReturnAlarm },
        { timeNParam,   lowMEV, clearMEV,   printState, printReturnAlarm },
        { timeNParam,   lowMEV, setMEV,     printState, printEnterAlarm },
        { timeNParam,   lowMEV, setMEV,     printState, printEnterAlarm }
    },
    {
        { timeNParam,   0,      setMEV,     printState, printEnterAlarm },
        { 0,            0,      updateMEV,  0,          0 },
        { timeNParam,   lowMEV, clearMEV,   printState, printReturnAlarm },
        { timeNParam,   lowMEV, setMEV,     printState, printEnterAlarm },
        { timeNParam,   lowMEV, setMEV,     printState, printEnterAlarm }
    },
    {
        { timeNParam,   0,      setMEV,     printState, printEnterAlarm },
        { timeNParam,   0,      setMEV,     printState, printEnterAlarm },
        { 0,            0,      0,          0,          0 },
        { timeNParam,   0,      setMEV,     printState, printEnterAlarm },
        { timeNParam,   0,      setMEV,     printState, printEnterAlarm }
    },
    {
        { timeNParam,   highMEV,setMEV,     printState, printEnterAlarm },
        { timeNParam,   highMEV,setMEV,     printState, printEnterAlarm },
        { timeNParam,   highMEV,clearMEV,   printState, printReturnAlarm },
        { 0,            0,      updateMEV,  0,          0 },
        { timeNParam,   0,      setMEV,     printState, printEnterAlarm }
    },
    {
        { timeNParam,   highMEV,setMEV,     printState, printEnterAlarm },
        { timeNParam,   highMEV,setMEV,     printState, printEnterAlarm },
        { timeNParam,   highMEV,clearMEV,   printState, printReturnAlarm },
        { timeNParam,   highMEV,setMEV,     printState, printReturnAlarm },
        { 0,            0,      updateMEV,  0,          0 }
    }
};
const AlarmPrintEntry changeOpsTable[5][5] =
{
    {
        { timeNParam,   lowMEV, setMEV,     printStateChg,  printEnterAlarm },
        { timeNParam,   lowMEV, setMEV,     printStateChg,  printEnterAlarm },
        { timeNParam,   lowMEV, clearMEV,   printStateChg,  printOK },
        { timeNParam,   lowMEV, setMEV,     printStateChg,  printEnterAlarm },
        { timeNParam,   lowMEV, setMEV,     printStateChg,  printEnterAlarm }
    },
    {
        { timeNParam,   lowMEV, setMEV,     printStateChg,  printEnterAlarm },
        { timeNParam,   lowMEV, setMEV,     printStateChg,  printEnterAlarm },
        { timeNParam,   lowMEV, clearMEV,   printStateChg,  printOK },
        { timeNParam,   lowMEV, setMEV,     printStateChg,  printEnterAlarm },
        { timeNParam,   lowMEV, setMEV,     printStateChg,  printEnterAlarm }
    },
    {
        { timeNParam,   0,      setMEV,     printStateChg,  printEnterAlarm },
        { timeNParam,   0,      setMEV,     printStateChg,  printEnterAlarm },
        { 0,            0,      0,          0,              0 },
        { timeNParam,   0,      setMEV,     printStateChg,  printEnterAlarm },
        { timeNParam,   0,      setMEV,     printStateChg,  printEnterAlarm }
    },
    {
        { timeNParam,   highMEV,setMEV,     printStateChg,  printEnterAlarm },
        { timeNParam,   highMEV,setMEV,     printStateChg,  printEnterAlarm },
        { timeNParam,   highMEV,clearMEV,   printStateChg,  printOK },
        { timeNParam,   highMEV,setMEV,     printStateChg,  printEnterAlarm },
        { timeNParam,   highMEV,setMEV,     printStateChg,  printEnterAlarm }
    },
    {
        { timeNParam,   highMEV,setMEV,     printStateChg,  printEnterAlarm },
        { timeNParam,   highMEV,setMEV,     printStateChg,  printEnterAlarm },
        { timeNParam,   highMEV,clearMEV,   printStateChg,  printOK },
        { timeNParam,   highMEV,setMEV,     printStateChg,  printEnterAlarm },
        { timeNParam,   highMEV,setMEV,     printStateChg,  printEnterAlarm }
    }
};

void
LimitChecker::_checkStatus(
TlmHdfFile*         tlmFile,        // TLM file
int32               startIndex,     // index in TLM file
FILE*               fp,             // output file pointer
LimitStatusE        newStatus,      // new status
void*               value)
{
    // if old and new statuses are both OK, return
    if (_status == LIMIT_OK && newStatus == LIMIT_OK)
        return;

    // get the correct table entry, and call its report functions
    int oldIndex = _getIndex(_status);
    int newIndex = _getIndex(newStatus);
    if (oldIndex == -1 || newIndex == -1)
    {
        printf("Invalid Parameter\n");
        return;
    }
#ifdef DEBUG
    fprintf(fp,"old = %d, new = %d\n", oldIndex, newIndex);
#endif
    const AlarmPrintEntry* entry;
    if (_statePair->changed)
        entry = &(changeOpsTable[oldIndex][newIndex]);
    else
        entry = &(noChangeOpsTable[oldIndex][newIndex]);

    BYTE6 buffer;
    if (_timeParamP->extractFunc(tlmFile, _timeParamP->sdsIDs,
                  startIndex, 1, 1, &buffer, 0) == FALSE)
    {
        printf("Cannot extract time\n");
        return;
    }

    Itime itime;
    itime.Char6ToItime(buffer);
    if (entry->timeNParam)
        (*(entry->timeNParam)) (this, fp, itime);
    if (entry->printMEV)
        (*(entry->printMEV)) (this, fp, _statePair->changed);
    if (entry->updateMEV)
        (*(entry->updateMEV)) (this, value, itime);
    if (entry->printState)
        (*(entry->printState)) (this, fp);
    if (entry->printAlarmStatus)
        (*(entry->printAlarmStatus)) (this, fp, value, newStatus);

}//LimitChecker::_checkStatus
