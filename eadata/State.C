//=========================================================//
// Copyright  (C)1995, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

static const char rcs_id[]="@(#) $Id$";

#include <stdio.h>
#include "State.h"

//======================//
// StateElement methods //
//======================//

StateElement::StateElement()
:   condition(StateElement::UNINITIALIZED)
{
    return;
}

StateElement::~StateElement()
{
    return;
}

//===============//
// State methods //
//===============//

State::State()
{
    return;
}

State::~State()
{
    return;
}

//--------------//
// StateExtract //
//--------------//

StateElement::ConditionE State::StateExtract(
    const char*     dataRec,
    ExtractFunc     function,
    char*           data,
    StateElement::ConditionE    prev_condition)
{
    if (function(dataRec, data))
        return (StateElement::CURRENT);
    else if (prev_condition == StateElement::CURRENT)
        return (StateElement::HELD);
    else
        return (prev_condition);
}
