//=========================================================//
// Copyright  (C)1996, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//
 
static const char rcs_id[] =
	"@(#) $Id$";

#include "EffDetector.h"
#include "L1AExtract.h"
#include "HkdtExtract.h"

#define RELAY_SET 0

//=====================//
// EffDetector methods //
//=====================//

EffDetector::EffDetector()
: _status(EFFDETECTOR_OK)
{
	_list = new CmdList();
	return;
}

EffDetector::~EffDetector()
{
	delete _list;
	return;
}

int 
EffDetector::GetParamIndex( Parameter **p, ParamIdE paramid )
{
    int i;
    for (i=0; p[i] != NULL; i++) {
        if (p[i]->paramId==paramid) 
            break;
    }

    if (p[i] == NULL ) 
        return (-1);
    else
        return(i);

}; // GetParamIndex


Parameter *
EffDetector::GetParam( Parameter **params, ParamIdE parId, 
                       TlmHdfFile *tlmFile, int32 nextIndex ) 
{

    int i;
    Parameter *p=0;
    // Get the parameter index using parameter Id
    i=GetParamIndex( params, parId );
    if (i<0) {
        (void) sprintf(_last_msg, "Can't get index using paramId %d\n",
                       parId );
        fprintf(stderr,"%s",_last_msg);
        _status=EFFDETECTOR_BAD_PARID;
        return (0);
    }
    
    // Extract the actual data
    p=params[i];
    if (! p->extractFunc(tlmFile, p->sdsIDs, 
                             nextIndex, 1, 1, p->data, 0)) {
        (void) sprintf(_last_msg,
                         "Can't extract parameter %s-- Aborting -- \n",
                         p->paramName );
        fprintf(stderr,"%s",_last_msg);
        _status=EFFDETECTOR_ERROR_EXTRACT_DATA;
        return(0);
        
    }

    return (p);
}; // GetParam
