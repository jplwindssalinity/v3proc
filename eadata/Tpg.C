//=========================================================
// Copyright  (C)1995, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.1   12 Mar 1998 17:15:56   sally
// adapt to the new QSCAT REQI format
// 
//    Rev 1.0   04 Feb 1998 14:17:32   daffer
// Initial checking
// Revision 1.2  1998/01/30 22:29:26  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================
 
static const char rcs_id_Tpg_C[] = "@(#) $Header$";

#include "Tpg.h"

//=============
// Tpg methods 
//=============

Tpg::Tpg()
:   time(0,0), path(0), gamma(0.0)
{
    return;
}

Tpg::Tpg(
    Itime   newTime,
    int     newPath,
    float   newGamma)
{
    Set(newTime, newPath, newGamma);
    return;
}

Tpg::Tpg(
const char*     codeADateString,
int             newPath,
float           newGamma)
: path(newPath), gamma(newGamma)
{
    time.CodeAToItime(codeADateString);

}//Tpg::Tpg

Tpg::~Tpg()
{
    return;
}

//-------------
// TpgToString 
//-------------

int
Tpg::TpgToString(
    char*   string)
{
    if (time == INVALID_TIME)
        return (0);

    char time_string[CODEA_TIME_LEN];
    time.ItimeToCodeADate(time_string);
    if (path == INVALID_PATH)
        sprintf(string, "%s", time_string);
    else
        sprintf(string, "%s %d %.2f", time_string, path, gamma);
    return (1);
}

//----------------
// TpgToPgString  
//----------------

int
Tpg::TpgToPgString(
    char*   string)
{
    if (path == INVALID_PATH)
        return (0);
    else
        sprintf(string, "%d %.2f", path, gamma);

    return (1);

}//Tpg::TpgToPgString

//----------------
// TpgToCodeADate 
//----------------

int
Tpg::TpgToCodeADate(
    char*   string)
{
    if (time == INVALID_TIME)
        return (0);

    return(time.ItimeToCodeADate(string));

}//Tpg::TpgToCodeADate

//-------------
// StringToTpg 
//-------------

int
Tpg::StringToTpg(
    const char* string)
{
    char time_string[CODEA_TIME_LEN];
    switch (sscanf(string, " %s %d %g", time_string, &path, &gamma))
    {
    case 3:
        break;
    case 1:
        path = 0;
        gamma = 0.0;
        break;
    default:
        return (0);
        break;
    }
    return (time.CodeAToItime(time_string));
}

//-----
// Set 
//-----

void
Tpg::Set(
    Itime   newTime,
    int     newPath,
    float   newGamma)
{
    time = newTime;
    path = newPath;
    gamma = newGamma;
    return;
}

//------------
// InvalidTpg 
//------------

int
Tpg::InvalidTpg()
{
    if (path < 1 || path > MAX_PATH || gamma < 0.0 || gamma >= MAX_GAMMA)
        return (1);
    else
        return (0);
}

//-----------
// operators 
//-----------

int
operator==(
    const Tpg&  a,
    const Tpg&  b)
{
    return (a.path == b.path && a.gamma == b.gamma);
}

int
operator<(
    const Tpg&  a,
    const Tpg&  b)
{
    if (a.time.StartOfDay() < b.time.StartOfDay())
        return (1);
    if (a.time.StartOfDay() > b.time.StartOfDay())
        return (0);

    // the dates are the same, use the paths
    int orbit_a = PathToOrbit(a.path);
    int orbit_b = PathToOrbit(b.path);

    int orbit_dif = ((orbit_b - orbit_a) + MAX_PATH) % MAX_PATH;
    if (orbit_dif < MAX_PATH/2)
        return (1);
    else
        return (0);
}

//-------------
// PathToOrbit 
//-------------
 
int
PathToOrbit(
    int     path_number)
{
    for (int c = 0; c < 41; c++)
    {
        int true_path = c*MAX_PATH + path_number;
        if ((true_path - 1)%41 == 0)
            return ( (true_path - 1) / 41 + 1);
    }
    return (0);
}
 
//-------------
// OrbitToPath 
//-------------
 
int
OrbitToPath(
    int     orbit)
{
    return ( (41 * (orbit - 1) + 1) % MAX_PATH);
}
