//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
// CM Log
// $Log$
// 
//    Rev 1.1   01 Sep 1998 16:38:48   sally
// change operator== to Match in order to pass outFP
// 
//    Rev 1.0   28 Aug 1998 11:21:18   sally
// Initial revision.
// 
// $Date$
// $Revision$
// $Author$
//
//=========================================================

static const char rcs_id_Reqa_C[] =
    "@(#) $Header$";

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "Reqa.h"
#include "Reqq.h"
#include "ReqaqList.h"

Reqa::Reqa(
const char*      filename,  // reqq file path + name
FILE*            outFP)
: _filename(0), _status(REQQ_OK)
{
    _reqaList = new ReqaRecordList;
    _status = _reqaList->Read(filename, outFP);
    if ((_filename = strdup(filename)) == 0)
    {
        fprintf(stderr, "Reqq: Out of memory\n");
        _status=REQQ_OUT_OF_MEMORY;
    }

} // Reqa::Reqa

Reqa::~Reqa()
{
    if (_filename) free(_filename);
    if (_reqaList) delete(_reqaList);
    return;
}

int
Reqa::Match(
const Reqq&      reqq,
FILE*            outFP)
{
    return(_reqaList->Match(*(reqq._reqqList), outFP));

} // Reqa::Match
