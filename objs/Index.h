//==============================================================//
// Copyright (C) 1998-2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#ifndef INDEX_H
#define INDEX_H

static const char rcs_id_index_h[] =
    "@(#) $Id$";

#include <stdio.h>

//======================================================================
// CLASSES
//    Index
//======================================================================

//======================================================================
// CLASS
//    Index
//
// DESCRIPTION
//    The Index object handles the conversion of a real value to
//    an index value for table access.
//======================================================================

class Index
{
public:

    //--------------//
    // construction //
    //--------------//

    Index();
    ~Index();

    //-----------------//
    // setting/getting //
    //-----------------//

    int  SpecifyEdges(float min, float max, int bins);
    int  SpecifyCenters(float min, float max, int bins);
    int  SpecifyWrappedCenters(float min, float max, int bins);
    int  SpecifyNewBins(Index* index, int bins);

    float  GetMin() { return(_min); };
    float  GetMax() { return(_max); };
    int    GetBins() { return(_bins); };
    float  GetStep() { return(_step); };

    int*    MakeIntArray();
    float*  MakeFloatArray();

    //--------------//
    // input/output //
    //--------------//

    int  Read(FILE* fp);
    int  ReadAscii(FILE* fp);
    int  Write(FILE* fp);
    int  WriteAscii(FILE* fp);

    //--------//
    // access //
    //--------//

    int  GetLinearCoefsStrict(float value, int idx[2], float coef[2]);
    int  GetLinearCoefsWrapped(float value, int idx[2], float coef[2]);
    int  GetLinearCoefsClipped(float value, int idx[2], float coef[2]);

    int   GetNearestIndexStrict(float value, int* idx);
    int   GetNearestIndexWrapped(float value, int* idx);
    void  GetNearestIndexClipped(float value, int* idx);

    int  IndexToValue(int idx, float* value);
    int  IndexToRange(int idx, float* bin_min, float* bin_max);

    //--------------//
    // operators    //
    //--------------//

    friend int operator==(const Index& a, const Index& b);
    friend int operator!=(const Index& a, const Index& b);

protected:

    //-----------//
    // variables //
    //-----------//

    float  _min;     // the minimum actual value (for index 0)
    float  _max;     // the maximum actual value (for index _count-1)
    int    _bins;    // the number of table elements
    float  _step;    // the actual value step size
};

#endif
