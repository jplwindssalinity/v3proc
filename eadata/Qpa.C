//=========================================================
// Copyright  (C)1996, California Institute of Technology.
// U.S. Government sponsorship under
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.1   25 Nov 1998 15:20:56   sally
// change ERROR to difference
// 
//    Rev 1.0   21 Sep 1998 15:06:28   sally
// Initial revision.
//
// $Date$
// $Revision$
// $Author$
//
//=========================================================

#include "Qpf.h"
#include "Qpa.h"

Qpa::Qpa(
const char*  qpaFilename,
FILE*        outFP)
: Qpf(qpaFilename, outFP)
{
    if (_status != Qpf::QPF_OK) return;

    // check the format of the time string
    int hh, mm, ss;
    if (sscanf(_cmdTimeString, "%02d:%02d:%02d", &hh, &mm, &ss) != 3)
    {
        fprintf(outFP, "QPA: %s: bad upload time string in header\n",
                       _filename);
        _status = Qpf::QPA_BAD_TIME_STRING;
        return;
    }

    return;

} // Qpa::Qpa

Qpa::~Qpa()
{
} // Qpa::~Qpa

Qpf::QpfStatusE
Qpa::Match(
const Qpf&       qpf,
FILE*            outFP)
{
    if (qpf.GetFileNumber() != _fileNumber)
    {
        fprintf(outFP, "ERROR: QPF's file number is %d, but QPA's is %d\n",
                    qpf.GetFileNumber(), _fileNumber);
        return Qpf::QPF_QPA_NOT_MATCHED;
    }
    else if (strcmp(qpf.GetMnemonic(), _mnemonic) != 0)
    {
        fprintf(outFP, "ERROR: QPF's mnemonic is %s, but QPA's is %s\n",
                    qpf.GetMnemonic(), _mnemonic);
        return Qpf::QPF_QPA_MATCHED_WITH_ERRORS;
    }
    else return Qpf::QPF_OK;

} // Qpa::Match
