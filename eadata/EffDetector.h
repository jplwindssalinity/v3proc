//=========================================================//
// Copyright  (C)1996, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef EFFDETECTOR_H
#define EFFDETECTOR_H

static const char rcs_id_eff_detector_h[] =
	"@(#) $Id$";

#include "CmdList.h"
#include "TlmFileList.h"
#include "Parameter.h"
#include "State.h"

//=============//
// EffDetector //
//=============//

class EffDetector
{
public:
    enum EffDetectorStatusE { EFFDETECTOR_OK,
                              EFFDETECTOR_ERROR,
                              EFFDETECTOR_BAD_PARID,
                              EFFDETECTOR_ERROR_EXTRACT_DATA,
                              EFFDETECTOR_STATUS_LAST};

    EffDetector();
    virtual ~EffDetector();
    CmdList*		         GetCmdList() {return (_list); };
    EffDetectorStatusE           GetStatus() {return (_status); };
    virtual EffDetectorStatusE   DetectEffects(TlmFileList *)=0;
    char *                       GetLastMsg() {return (_last_msg);};

protected:
    int                GetParamIndex(Parameter **p, ParamIdE par);
    Parameter          *GetParam(Parameter **p, ParamIdE par, 
                                 TlmHdfFile *tlmFile, int32 nextIndex);
    CmdList	       *_list;
    EffDetectorStatusE _status;
    char               _last_msg[256];

};

#endif
