//=========================================================//
// Copyright  (C)1995, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef LIMITCHECKER_H
#define LIMITCHECKER_H

#include <stdlib.h>

#include "Parameter.h"
#include "Itime.h"
#include "LimitState.h"

static const char rcsidLimitChecker_h[] = "@(#) $Id$";

//*********************************************************//
//  ReadFunc: read from inBuf, put it in value             //
//  ReadLimitLine: reads from line, put it in cl,ch,al,ah  //
//*********************************************************//
typedef char (*ReadFunc) (char* inBuf, void* value);
typedef char (*ReadLimitLine) (char* line,
                                void* cl, void* ch, void* al, void* ah);

//*********************************************************//
// all aEQbFunc returns (a==b) ?                           //
//*********************************************************//
typedef char (*aEQbFunc) (void* a, void* b);

//*********************************************************//
// all aGTbFunc returns (a>b) ?                            //
//*********************************************************//
typedef char (*aGTbFunc) (void* a, void* b);

//*********************************************************//
//  all Write function                                     //
//         value: is a value of type XXX                   //
//         inBuf: is a string where sprintf will write to  //
//*********************************************************//
typedef char (*WriteFunc) (char* string, void* value);

//*********************************************************//
// all assignBtoAFunc : a = b;                             //
//*********************************************************//
typedef void (*assignBtoAFunc) (void* a, void* b);

//*****************************************************************
// LimitChecker:
//        Each instance of LimitChecker represents an entry in
//        the limits file.
//*****************************************************************

class PolynomialTable;

class LimitChecker
{
public:

    LimitChecker(   Parameter*      parameter,
                    char            enable=1);

    // default constructor
    LimitChecker();

    // copy constructor
    LimitChecker(const LimitChecker& old);

    virtual             ~LimitChecker();

    IotBoolean          OpenParamDataSets(TlmHdfFile*);
    IotBoolean          CloseParamDataSets(TlmHdfFile*);

    inline void*        AllocOneValue(void) { return(malloc(_bytes)); }

    LimitStatusE        GetStatus(void) { return _status; }

    SourceIdE           GetSource(void) const
                        { return(_parameter == 0 ? SOURCE_UNKNOWN :
                                                _parameter->sourceId); };

    char                GetEnable(void) const { return _enable; };
    void                SetEnable(char enable) { _enable = enable; };

    const char*         GetParamName(void) const
                        { return(_parameter == 0 ? 0 :
                                                _parameter->paramName); };
    const char*         GetUnitName(void) const
                        { return(_parameter == 0 ? 0 :
                                                _parameter->unitName); };
    Parameter*          GetParameter(void) { return _parameter; }

    void                SetLimits(LimitStatePair*,
                                void* cl, void* ch, void* al, void* ah);

    LimitStatusE        CheckFrame(
                            PolynomialTable*  polyTable,
                            TlmHdfFile*       tlmFile,
                            int32             startIndex,
                            FILE*             fp,    // output file pointer
                            LimitStatePair*   limitState);

    virtual char        PrintText(char* destString)=0;

                        // report what the final status is
    virtual void        FinalReport(FILE* fp);

    LimitStatusE        _checkValue(LimitStatePair* state, void* value);

    void                _checkStatus(
                            TlmHdfFile*     tlmFile,
                            int32           startIndex,
                            FILE*           fp,     // output fp
                            LimitStatusE    newStatus,
                            void*           value);
    //========================================
    // these functions allow user to evaluate
    // and change limit values
    //========================================
    ReadFunc            _readFunc;
    WriteFunc           _writeFunc;
    aGTbFunc            _aGTb;
    aEQbFunc            _aEQb;
    assignBtoAFunc      _assignBtoA;
    ReadLimitLine       _readLimitLine;

    Parameter*          _parameter;
    char                _enable;

    Itime               _mevTime;
    LimitStatusE        _status;
    Parameter*          _timeParamP;

    void*               _maxExceedValue;
    void*               _limits;
    size_t              _bytes;
    LimitStatePair*     _statePair;

    virtual char        _initialize(void);


    // for alarm status reporting
    inline int          _getIndex(LimitStatusE alarmState);

}; //class LimitChecker

class HK2LimitChecker : public LimitChecker
{
public:
    HK2LimitChecker(           Parameter*      parameter,
                                char            enable=1);

    // copy constructor
    HK2LimitChecker(const HK2LimitChecker&);

    virtual ~HK2LimitChecker() {};

    virtual char        PrintText(char* destString);

protected:
    virtual char        _initialize(void);

};//HK2LimitChecker

class L1ALimitChecker : public LimitChecker
{
public:
    L1ALimitChecker(             Parameter*      parameter,
                                char            enable=1);

    // copy constructor
    L1ALimitChecker(const L1ALimitChecker&);

    virtual ~L1ALimitChecker() {};

    virtual char        PrintText(char* destString);

protected:
    virtual char        _initialize(void);

};//L1ALimitChecker

inline int operator==(const LimitChecker& a, const LimitChecker& b)
{
    Parameter* aParam = a._parameter;
    Parameter* bParam = b._parameter;
    if (aParam == 0 || bParam == 0)
        return 0;
    return (aParam->sourceId == bParam->sourceId &&
            aParam->paramId == bParam->paramId &&
            aParam->unitId == bParam->unitId &&
            a._enable == b._enable ? 1 : 0);
}

int operator!=(const LimitChecker& a, const LimitChecker& b);
int operator==(const L1ALimitChecker& a, const L1ALimitChecker& b);
int operator!=(const L1ALimitChecker& a, const L1ALimitChecker& b);
int operator==(const HK2LimitChecker& a, const HK2LimitChecker& b);
int operator!=(const HK2LimitChecker& a, const HK2LimitChecker& b);

#endif //LIMITCHECKER_H
