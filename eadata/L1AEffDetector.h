//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship acknowledged.               
//
// CM Log
//
// $Log$
// 
//    Rev 1.8   25 Mar 1999 14:11:16   daffer
// Uploadable tables!
// 
//    Rev 1.7   01 Sep 1998 11:15:26   daffer
// Changed 3 arg AddEffect to accomodate PBI commanding
// 
//    Rev 1.6   19 Aug 1998 14:38:34   daffer
// Add whole bunch more effects.
// 
//    Rev 1.5   22 Jun 1998 09:25:08   sally
// took out some compile errors and warnings for GNU GCC
// 
//    Rev 1.4   28 May 1998 13:19:26   daffer
// fixed bugs in DetectEffects
// 
//    Rev 1.3   22 May 1998 17:18:22   daffer
// Added/modified code for qscat effect detection
// 
//    Rev 1.2   01 May 1998 15:24:24   daffer
// Added pvcs header keywords
//
// $Date$
// $Revision$
// $Author$
//
//=========================================================

#ifndef L1AEFFDETECTOR_H
#define L1AEFFDETECTOR_H

static const char rcs_id_l1a_cmd_detector_h[] =
    "@(#) $Id$";

#include <assert.h>
#include "EffDetector.h"
#include "EALog.h"
#include "Parameter.h"
#include "ParTab.h"
#include "State.h"
#include "TlmHdfFile.h"
#include  "UpldTbl.h"
#include  "UpldTblList.h"
//================//
// L1AEffDetector //
//================//

class L1AEffDetector : public EffDetector
{
public:
    L1AEffDetector( const char * qpa_directory, 
                    const char * qpf_directory,
                    const char * uqpx_directory,
                    Itime start_time, 
                    Itime end_time);
    virtual ~L1AEffDetector();

    int                    AddEffect(const Itime time, 
                                     const EffectE effect_id);
    int                    AddEffect(const Itime time, 
                                     const EffectE effect_id, 
                                     const unsigned short effect_value );

    int                    AddEffect( Command *cmd);

    Command *              CreateEffectEntry( const Itime time, 
                                              const EffectE effect_id);
    EffDetectorStatusE    DetectEffects(TlmFileList* );
    EAUpldTbl_Table_StatusE  FindNearestQpx( UpldTbl *, UpldTbl *& );
    char *        GetQpfDirectory(){ return (_qpf_directory);};
    char *        GetQpaDirectory(){ return (_qpa_directory);};
    char *        GetUqpxDirectory(){ return (_uqpx_directory);};

 private: 

    int                   numTableElements;
    char                * _qpf_directory;
    char                * _qpa_directory;
    char                * _uqpx_directory;
    UpldTblList         * _qpx_list;

};

//================//
// NrtEffDetector //
//================//

class NrtEffDetector : public L1AEffDetector
{
public:
    NrtEffDetector();
    ~NrtEffDetector();

};

#endif
