//=========================================================//
// Copyright  (C)1996, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef L1AEFFDETECTOR_H
#define L1AEFFDETECTOR_H

static const char rcs_id_l1a_cmd_detector_h[] =
    "@(#) $Id$";

#include "EffDetector.h"
#include "State.h"

//===============//
// L1AEffDetector //
//===============//

class L1AEffDetector : public EffDetector
{
public:
    L1AEffDetector();
    ~L1AEffDetector();

    int     AddEffect(const Itime time, const EffectE effect_id);
    int     DetectEffects(const char* dataRec);
};

//================//
// NrtEffDetector //
//================//

class NrtEffDetector : public L1AEffDetector
{
public:
    NrtEffDetector();
    ~NrtEffDetector();

    int     AddEffect(const Itime time, const EffectE effect_id);
};

#endif
