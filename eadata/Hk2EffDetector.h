//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship acknowledged.               
//
// CM Log
//
// $Log$
// 
//    Rev 1.0   25 Mar 1999 13:57:56   daffer
// Initial Revision
// $Date$
// $Revision$
// $Author$
//
//=========================================================

#ifndef HK2EFFDETECTOR_H
#define HK2EFFDETECTOR_H

static const char rcs_id_Hk2_cmd_detector_h[] =
    "@(#) $Id$";

#include <assert.h>
#include "EffDetector.h"

//================//
// Hk2EffDetector //
//================//

class Hk2EffDetector : public EffDetector
{
public:
    Hk2EffDetector( );
    ~Hk2EffDetector();

    int                    AddEffect(const Itime time, 
                                     const EffectE effect_id);
    int                    AddEffect(const Itime time, 
                                     const EffectE effect_id, 
                                     const unsigned short effect_value );

    int                    AddEffect( Command *cmd);

    Command *              CreateEffectEntry( const Itime time, 
                                              const EffectE effect_id);
    EffDetectorStatusE    DetectEffects(TlmFileList* );

    int numTableElements;
};
#endif
