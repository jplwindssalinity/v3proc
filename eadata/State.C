//=========================================================
// Copyright  (C)1995, California Institute of Technology.
// U.S. Government sponsorship under
// NASA Contract NAS7-1260 is acknowledged
//
// CM Log
// $Log$
// 
//    Rev 1.3   23 Jul 1998 16:14:56   sally
// pass polynomial table to extractFunc()
// 
//    Rev 1.2   18 May 1998 14:48:06   sally
// added error checker for L1A
// 
//    Rev 1.1   13 May 1998 16:27:32   sally
 
// $Date$
// $Revision$
// $Author$
//
//=========================================================

static const char rcs_id[]=
    "@(#) $Header$";

#include <assert.h>
#include <stdio.h>
#include "State.h"

//======================//
// StateElement methods //
//======================//

StateElement::StateElement()
:   condition(StateElement::UNINITIALIZED),
    value(0), byteSize(0)
{
    return;
}

StateElement::StateElement(
const StateElement&     other)
:   condition(other.condition)
{
    if (byteSize != other.byteSize)
    {
        CleanUp();
        AllocValue(other.byteSize);
    }
    (void) memcpy(value, other.value, other.byteSize);

} // StateElement::StateElement

void
StateElement::CleanUp(void)
{
    if (value) free(value);
    byteSize = 0;
    return;
}

StateElement::~StateElement()
{
    CleanUp();
    return;
}

StateElement&
StateElement::operator=(
const StateElement&     other)
{
    condition = other.condition;
    if (byteSize != other.byteSize)
    {
        CleanUp();
        AllocValue(other.byteSize);
    }
    (void) memcpy(value, other.value, other.byteSize);
    return *this;
}

void
StateElement::AllocValue(
int    pbyteSize)
{
    assert(pbyteSize != 0);
    byteSize = pbyteSize;
    value = malloc(byteSize);

} //StateElement::AllocValue

//===============//
// State methods //
//===============//

State::State(
int       numStateElements)
: stateElements(0), _numStateElements(numStateElements)
{
    stateElements = new StateElement[numStateElements];
    assert(stateElements != 0);
    return;
}

State::State(
const State&     otherState)
{
    if (stateElements) delete [] stateElements;
    stateElements = 0;
    _numStateElements = otherState._numStateElements;
    stateElements = new StateElement[_numStateElements];
    assert(stateElements != 0);
    for (int i = 0; i < _numStateElements; i++)
    {
        stateElements[i] = StateElement(otherState.stateElements[i]);
    }
    return;
}

State&
State::operator=(
const State&     other)
{
    if (stateElements) delete [] stateElements;
    stateElements = 0;
    _numStateElements = other._numStateElements;
    stateElements = new StateElement[_numStateElements];
    assert(stateElements != 0);
    for (int i = 0; i < _numStateElements; i++)
    {
        stateElements[i] = other.stateElements[i];
    }
    return *this;

} // State::operator=

State::~State()
{
    if (stateElements) delete [] stateElements;
    return;
}

//--------------//
// StateExtract //
//--------------//

StateElement::ConditionE
State::StateExtract(
ExtractFunc           extractFunc,       // IN
TlmHdfFile*           tlmFile,           // IN
int32*                sdsIds,            // IN
int32                 startIndex,        // IN
const StateElement&   oldStateElement,   // IN/OUT
StateElement&         newStateElement,   // IN/OUT
VOIDP                 data)              // IN/OUT
{
    int rc = extractFunc(tlmFile, sdsIds, startIndex, 1, 1, data, 0);

    if (rc > 0)
        newStateElement.condition = StateElement::CURRENT;
    else if (rc < 0)
        newStateElement.condition = StateElement::ERROR;
    else
    {
        if (oldStateElement.condition == StateElement::CURRENT)
            // rc == 0, no value
            newStateElement.condition = StateElement::HELD;
        else
            newStateElement.condition = oldStateElement.condition;
    }

    return(newStateElement.condition);

}//State::StateExtract
