//=========================================================//
// Copyright  (C)1995, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef LIMIT_STATE_H
#define LIMIT_STATE_H

#include "Parameter.h"

static const char rcs_id_limit_state_h[] = "@(#) $Id$";

enum LimitStatusE
{
    LIMIT_OK,
    CAUTION_LOW,
    CAUTION_HIGH,
    ACTION_LOW,
    ACTION_HIGH,
    INVALID_PARAMETER,
    WRONG_TYPE
};

//*******************************************************//
// _____LimitState represents a state for one frame      //
//*******************************************************//

//*******************************************************//
//      L1ALimitState                                     //
//*******************************************************//
struct L1ALimitState
{
    // constructor
    L1ALimitState(int amode, int ahvps, int aframe) :
            mode(amode), hvps(ahvps), frame(aframe) {};

    // default constructor
    L1ALimitState() : mode(0), hvps(0), frame(0) {};

    // copy constructor
    L1ALimitState(const L1ALimitState& old)
            : mode(old.mode), hvps(old.hvps), frame(old.frame) {};

    virtual ~L1ALimitState() {};

    int     mode;
    int     hvps;
    int     frame;

};//L1ALimitState

#if 0
//*******************************************************//
//      HkdtLimitState                                   //
//*******************************************************//
struct HkdtLimitState
{
    // constructor
    HkdtLimitState(int amode, int ahvps, int adss, int atwta) :
            mode(amode), hvps(ahvps), dss(adss), twta(atwta) {};

    // default constructor
    HkdtLimitState() : mode(0), hvps(0), dss(0), twta(0) {};

    // copy constructor
    HkdtLimitState(const HkdtLimitState& old)
            : mode(old.mode), hvps(old.hvps),
            dss(old.dss), twta(old.twta) {};

    virtual ~HkdtLimitState() {};

    int     mode;
    int     hvps;
    int     dss;
    int     twta;

};//HkdtLimitState

#endif

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
    L1ALimitStatePair(int mode, int hvps, int frame);
            
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
    Parameter*      hvpsParamP;
    Parameter*      frameParamP;

};//L1ALimitStatePair

#if 0
//*******************************************************//
//      HkdtLimitStatePair                               //
//*******************************************************//
struct HkdtLimitStatePair : public LimitStatePair
{
    // constructor
    HkdtLimitStatePair(int mode, int hvps, int dss, int twta) :
            LimitStatePair(),
            stateA(mode, hvps, dss, twta), stateB(mode, hvps, dss, twta),
            oldState(&stateA), newState(&stateB) {};
            
    // default constructor
    HkdtLimitStatePair() : LimitStatePair(),
                            oldState(&stateA), newState(&stateB) {};

    // copy constructor
    HkdtLimitStatePair(const HkdtLimitStatePair& old) :
            LimitStatePair(old), stateA(old.stateA), stateB(old.stateB),
            oldState(old.oldState), newState(old.newState) {};

    virtual ~HkdtLimitStatePair() {};

    virtual IotBoolean      OpenParamDataSets(TlmHdfFile*);
    virtual IotBoolean      CloseParamDataSets(TlmHdfFile*);

    static int      numStates(void);

    virtual int     offset(void);
    virtual void    PrintNewState(FILE* fp);
    virtual void    PrintChange(FILE* fp);
    inline void     _printState(FILE* fp, HkdtLimitState*);

                    // return 1 if state changed, 0 if not
    virtual char    ApplyNewFrame(TlmHdfFile* tlmFile, int32 startIndex);

    HkdtLimitState  stateA;
    HkdtLimitState  stateB;
    HkdtLimitState  *oldState, *newState;

};//HkdtLimitStatePair
#endif

int operator== (const L1ALimitState& a, const L1ALimitState& b);
int operator!= (const L1ALimitState& a, const L1ALimitState& b);
#if 0
int operator== (const HkdtLimitState& a, const HkdtLimitState& b);
int operator!= (const HkdtLimitState& a, const HkdtLimitState& b);
#endif

int operator== (const L1ALimitStatePair& a, const L1ALimitStatePair& b);
int operator!= (const L1ALimitStatePair& a, const L1ALimitStatePair& b);
#if 0
int operator== (const HkdtLimitStatePair& a, const HkdtLimitStatePair& b);
int operator!= (const HkdtLimitStatePair& a, const HkdtLimitStatePair& b);
#endif

#endif //LIMIT_STATE_H
