//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.0   21 Sep 1998 15:06:30   sally
// Initial revision.
// 
// $Date$
// $Revision$
// $Author$
//
//=========================================================

#ifndef QPA_H
#define QPA_H

#include <stdio.h>
#include "Qpf.h"

static const char rcs_id_qpa_h[] =
    "@(#) $Header$";


//============
// Qpa class 
//============

class Qpa : public Qpf
{
public:

    // read from QPA file
    Qpa(const char*  qpaFilename,    // qpa file path + name
        FILE*        outFP);

    virtual ~Qpa();

    Qpf::QpfStatusE  Match(const Qpf& qpf, FILE* outFP);

};

#endif
