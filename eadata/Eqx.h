//=========================================================
// Copyright  (C)1995, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.3   20 Apr 1998 15:18:14   sally
// change List to EAList
// 
//    Rev 1.2   14 Apr 1998 16:40:40   sally
// move back to EA's old list
// 
//    Rev 1.1   06 Apr 1998 16:27:10   sally
// merged with SVT
// 
//    Rev 1.0   04 Feb 1998 14:15:18   daffer
// Initial checking
// Revision 1.2  1998/01/30 22:28:17  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

//-----
// Eqx stands for Equator Crossing.  Each Eqx object contains
// an orbit number and the time of the equator crossing for
// the orbit number.  An EqxList is a list of Eqx objects which
// have the ability to covert between time and Tpg
//-----

#ifndef EQX_H
#define EQX_H

static const char rcs_id_eqx_h[] = "@(#) $Header$";

#include "Itime.h"
#include "EAList.h"
#include "Tpg.h"

#define SEC_PER_ORBIT           3542400.0/585.0
#define LON_ASC_NODE_PATH_585   0.030
#define ORBITS                  585.0

//=====
// Eqx 
//=====

class Eqx
{
    friend int      operator==(const Eqx&, const Eqx&);
    friend int      operator!=(const Eqx&, const Eqx&);
    friend int      operator<(const Eqx&, const Eqx&);
    friend int      operator<=(const Eqx&, const Eqx&);
    friend int      operator>(const Eqx&, const Eqx&);
    friend int      operator>=(const Eqx&, const Eqx&);

public:
    enum ReturnE
    {
        OK,
        END_OF_FILE,
        ERROR_PARSING_TIME,
        ERROR_READING_EQX,
        ERROR
    };

    Eqx();
    Eqx(Itime new_time, unsigned short new_path);
    Eqx(Itime new_time, float eqx_lon);
    ~Eqx();

    ReturnE     Read(FILE* ifp);
    ReturnE     Write(FILE* ifp);

    Itime           EstimateItime(Tpg tpg, double period);
    Tpg EstimateTpg(Itime itime, double period);

    //-----------
    // variables 
    //-----------

    Itime           time;
    unsigned short  path;
};

inline int  operator==(const Eqx& a, const Eqx& b)
    {
        return (a.time == b.time);
    }
inline int  operator!=(const Eqx& a, const Eqx& b)
    {
        return (a.time != b.time);
    }
inline int  operator<(const Eqx& a, const Eqx& b)
    {
        return (a.time < b.time);
    }
inline int  operator<=(const Eqx& a, const Eqx& b)
    {
        return (a.time <= b.time);
    }
inline int  operator>(const Eqx& a, const Eqx& b)
    {
        return (a.time > b.time);
    }
inline int  operator>=(const Eqx& a, const Eqx& b)
    {
        return (a.time >= b.time);
    }

//=========
// EqxList 
//=========

class EqxList : SortedList<Eqx>
{
public:
    enum StatusE
    {
        OK,
        ERROR_OPENING_FILE,
        ERROR_READING_EQX,
        ERROR_ALLOCATING_EQX
    };

    EqxList();
    EqxList(const char* filename);
    ~EqxList();

    StatusE     Read(const char* filename);
    StatusE     Write(const char* filename);
    StatusE     AddEqx(Itime time, float eqx_lon);

    int     TpgToItime(const Tpg tpg, Itime* time);
    int     ItimeToTpg(const Itime itime, Tpg* tpg);
    Eqx*    FindNearestEqx(const Tpg tpg);
    Eqx*    FindNearestEqx(const Itime itime);

    inline StatusE   GetStatus() { return (_status); };
private:
    StatusE     _status;
};

#endif //EQX_H
