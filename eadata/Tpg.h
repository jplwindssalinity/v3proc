//=========================================================
// Copyright  (C)1995, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.2   26 May 1998 16:43:58   sally
// change MAX_PATH from 585 to 57
// 
//    Rev 1.1   12 Mar 1998 17:16:00   sally
// adapt to the new QSCAT REQI format
// 
//    Rev 1.0   04 Feb 1998 14:17:34   daffer
// Initial checking
// Revision 1.2  1998/01/30 22:28:48  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

//-----
// Tpg stands for Time/Path/Gamma.  It is used to located the spacecraft.
// This class replaces the older Rsp class.
//-----

#ifndef TPG_H
#define TPG_H

static const char rcs_id_tpg_h[] = "@(#) $Header$";

#include "Itime.h"

#define INVALID_TPG     Tpg(Itime(0,0),0,0)
#define INVALID_PATH    0
#define MAX_PATH        57
#define MAX_GAMMA       360.0
#define TPG_LEN         64

//=====
// Tpg 
//=====

class Tpg
{
public:
    Tpg();
    Tpg(Itime time, int path, float gamma);

    Tpg(    const char* codeADateString,
            int         path,
            float       gamma);
    virtual ~Tpg();

    void    Set(Itime   time,
                int     path = 0,
                float   gamma = 0.0);

            //-------------------------------------
            // return 1 if OK, else 0.
            // user must provide space;
            // string format: "CodeADate ddd fff.ff"
            //-------------------------------------
    int     TpgToString(char* string);

            //-------------------------------------
            // return 1 if OK, else 0.
            // user must provide space for string;
            // string format: "ddd fff.ff"
            //-------------------------------------
    int     TpgToPgString(char* string);

            //-------------------------------------
            // return 1 if OK, else 0.
            // user must provide space for string;
            //-------------------------------------
    int     TpgToCodeADate(char* string);

            //-------------------------------------
            // string format: "CodeA ddd fff.ff"
            //-------------------------------------
    int     StringToTpg(const char* string);

    int     InvalidTpg(void);

    friend int  operator==(const Tpg& a, const Tpg& b);
    friend int  operator<(const Tpg& a, const Tpg& b);

    //-----------
    // variables 
    //-----------

    Itime   time;
    int     path;
    float   gamma;
};

//------------------
// helper functions 
//------------------

int     PathToOrbit(int path);
int     OrbitToPath(int orbit);

#endif
