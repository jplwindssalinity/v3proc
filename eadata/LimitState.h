//=========================================================//
// Copyright  (C)1995, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef LIMIT_STATE_H
#define LIMIT_STATE_H

#include "Parameter.h"

static const char rcs_id_limit_state_h[] = "@(#) $Id$";

//----------------------------------------------------------------------
// ATTENTION: don't change order without changing limitStatusStrings[]
//----------------------------------------------------------------------
enum LimitStatusE
{
    LIMIT_OK,
    CAUTION_LOW,
    CAUTION_HIGH,
    ACTION_LOW,
    ACTION_HIGH,
    LIMIT_CHECK_OK,  // this includes above
    INVALID_PARAMETER,
    WRONG_TYPE,
    LIMIT_MISSING_SDS_NAME,
    LIMIT_APPLY_POLYNOMIAL_TO_NON_FLOAT,
    LIMIT_POLYNOMIAL_NOT_IN_TABLE,
    LIMIT_NO_POLYNOMIAL_TABLE
};

// get the corresponding string for LimitStatusE
const char* GetLimitStatusString(LimitStatusE);

//*******************************************************//
// _____LimitState represents a state for one frame      //
//*******************************************************//

//*******************************************************//
//      L1ALimitState                                     //
//*******************************************************//
struct L1ALimitState
{
    // constructor
    L1ALimitState(int amode, int atwt, int atwta, int aframe) :
            mode(amode), twt(atwt), twta(atwta), frame(aframe) {};

    // default constructor
    L1ALimitState() : mode(0), twt(0), twta(0), frame(0) {};

    // copy constructor
    L1ALimitState(const L1ALimitState& old)
            : mode(old.mode), twt(old.twt), twta(old.twta), frame(old.frame) {};

    virtual ~L1ALimitState() {};

    int     mode;
    int     twt;
    int     twta;
    int     frame;

};//L1ALimitState

//*******************************************************//
//      HK2LimitState                                   //
//*******************************************************//
struct HK2LimitState
{
    // constructor
    HK2LimitState(int amode, int atwt, int atwta) :
            mode(amode), twt(atwt), twta(atwta) {};

    // default constructor
    HK2LimitState() : mode(0), twt(0), twta(0) {};

    // copy constructor
    HK2LimitState(const HK2LimitState& old)
            : mode(old.mode), twt(old.twt), twta(old.twta) {};

    virtual ~HK2LimitState() {};

    int     mode;
    int     twt;
    int     twta;

};//HK2LimitState


//*******************************************************//
// _____LimitStatePair manages the states for two frames://
// whether the state is changed, ...                     //
//*******************************************************//

//*******************************************************//
//      LimitState is an abstract class                  //
//*******************************************************//
struct LimitStatePair
{
    // copy constructor
    LimitStatePair(const LimitStatePair& old)
                : changed(old.changed) {};

    virtual ~LimitStatePair() {};

    virtual IotBoolean      OpenParamDataSets(TlmHdfFile*)=0;
    virtual IotBoolean      CloseParamDataSets(TlmHdfFile*)=0;

    virtual int     offset(void)=0;
    virtual void    PrintNewState(FILE* fp)=0;
    virtual void    PrintChange(FILE* fp)=0;

                    // return 1 if state changed, 0 if not
    virtual char    ApplyNewFrame(TlmHdfFile* tlmFile, int32 startIndex)=0;

    char            changed;

protected:
    //constructor
    LimitStatePair(): changed(0) {};

};//LimitStatePair

//*******************************************************//
//      L1ALimitStatePair                                 //
//*******************************************************//
struct L1ALimitStatePair : public LimitStatePair
{
    // constructor
    L1ALimitStatePair(int mode, int twt, int twta, int frame);
            
    // default constructor
    L1ALimitStatePair();

    // copy constructor
    L1ALimitStatePair(const L1ALimitStatePair& old);

    virtual ~L1ALimitStatePair();

    virtual IotBoolean      OpenParamDataSets(TlmHdfFile*);
    virtual IotBoolean      CloseParamDataSets(TlmHdfFile*);

    static int      numStates(void);

    virtual int     offset(void);
    virtual void    PrintNewState(FILE* fp);
    virtual void    PrintChange(FILE* fp);
    inline void     _printState(FILE* fp, L1ALimitState*);

                        // return 1 if state changed, 0 if not
    virtual char    ApplyNewFrame(TlmHdfFile* tlmFile, int32 startIndex);

    L1ALimitState    stateA;
    L1ALimitState    stateB;
    L1ALimitState    *oldState, *newState;

    Parameter*      modeParamP;
    Parameter*      k9ParamP;
    Parameter*      k10ParamP;
    Parameter*      k11ParamP;
    Parameter*      k12ParamP;
    Parameter*      frameParamP;

};//L1ALimitStatePair

//*******************************************************//
//      HK2LimitStatePair                               //
//*******************************************************//
struct HK2LimitStatePair : public LimitStatePair
{
    // constructor
    HK2LimitStatePair(int mode, int twt, int twta);
            
    // default constructor
    HK2LimitStatePair();

    // copy constructor
    HK2LimitStatePair(const HK2LimitStatePair& old);

    virtual ~HK2LimitStatePair();

    virtual IotBoolean      OpenParamDataSets(TlmHdfFile*);
    virtual IotBoolean      CloseParamDataSets(TlmHdfFile*);

    static int      numStates(void);

    virtual int     offset(void);
    virtual void    PrintNewState(FILE* fp);
    virtual void    PrintChange(FILE* fp);
    inline void     _printState(FILE* fp, HK2LimitState*);

                    // return 1 if state changed, 0 if not
    virtual char    ApplyNewFrame(TlmHdfFile* tlmFile, int32 startIndex);

    HK2LimitState  stateA;
    HK2LimitState  stateB;
    HK2LimitState  *oldState, *newState;

    Parameter*     modeParamP;
    Parameter*     k9ParamP;
    Parameter*     k10ParamP;
    Parameter*     k11ParamP;
    Parameter*     k12ParamP;

};//HK2LimitStatePair

int operator== (const L1ALimitState& a, const L1ALimitState& b);
int operator!= (const L1ALimitState& a, const L1ALimitState& b);
int operator== (const HK2LimitState& a, const HK2LimitState& b);
int operator!= (const HK2LimitState& a, const HK2LimitState& b);

int operator== (const L1ALimitStatePair& a, const L1ALimitStatePair& b);
int operator!= (const L1ALimitStatePair& a, const L1ALimitStatePair& b);
int operator== (const HK2LimitStatePair& a, const HK2LimitStatePair& b);
int operator!= (const HK2LimitStatePair& a, const HK2LimitStatePair& b);

#endif //LIMIT_STATE_H
