//=========================================================
// Copyright  (C)1995, California Institute of Technology.
// U.S. Government sponsorship under
// NASA Contract NAS7-1260 is acknowledged
//
// CM Log
// $Log$
// 
//    Rev 1.3   19 May 1998 15:48:58   sally
// took out comparison function
// 
//    Rev 1.2   18 May 1998 14:48:08   sally
// added error checker for L1A
// 
//    Rev 1.1   13 May 1998 16:27:36   sally
 
// $Date$
// $Revision$
// $Author$
//
//=========================================================


#ifndef STATE_H
#define STATE_H

static const char rcs_id_state_h[]=
    "@(#) $Header$";

#include "Parameter.h"
#include "Itime.h"
#include "TlmHdfFile.h"

//==============//
// StateElement //
//==============//

class StateElement
{
public:
    enum ConditionE
    {
        ERROR = -1,
        UNINITIALIZED = 0,
        CURRENT,
        HELD
    };

    StateElement();
    StateElement(const StateElement&);
    StateElement&   operator=(const StateElement& other);

    virtual ~StateElement();

    void            AllocValue(int pbyteSize);

    ConditionE      condition;
    void*           value;
    int             byteSize;

protected:
    void            CleanUp(void);

};//StateElement

// return 1 if same, 0 if different.  for checking "TRANSITION"
typedef int (*StateCompareFunc) (const StateElement&    stateElementA,
                                 const StateElement&    stateElementB,
                                 size_t                 byteSize);

inline int
DefaultStateCompareFunc(
const StateElement&    stateElementA,
const StateElement&    stateElementB,
size_t                 byteSize)
{
    return(memcmp(stateElementA.value, stateElementB.value, byteSize)
                      == 0 ? 1 : 0);
}

struct StateTabEntry
{
    ParamIdE           paramId;
    UnitIdE            unitId;
};

//=======//
// State //
//=======//

class State
{
public:

    State(int numStateElements);
    State(const State& otherState);
    State&   operator=(const State& other);

    virtual ~State();

    virtual StateElement::ConditionE
            StateExtract(
                    ExtractFunc             extractFunc,     // IN
                    TlmHdfFile*             tlmFile,         // IN
                    int32*                  sdsIds,          // IN
                    int32                   startIndex,      // IN
                    const StateElement&     oldStateElement, // IN
                    StateElement&           newStateElement, // IN/OUT
                    VOIDP                   data);           // IN/OUT

    char            time_string[CODEA_TIME_LEN];

    StateElement*   stateElements;   // array of StateElement
    int             _numStateElements;

private:
    State() { printf("State default constructor\n");} // NULL default constructor

};//State

#endif // STATE_H
