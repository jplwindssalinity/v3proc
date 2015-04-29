#ifndef CAPWIND_H
#define CAPWIND_H
#include "List.h"
#include "Wind.h"
#include "AngleInterval.h"

class CAPWindVectorPlus : public WindVectorPlus {
public:
    CAPWindVectorPlus();
    ~CAPWindVectorPlus();

    float sss;
};

class CAPWVC {
public:

    CAPWVC();
    ~CAPWVC();

    void FreeContents();
    int BuildSolutions();
    int SortByObj();

    float best_spd[360];
    float best_obj[360];
    float best_sss[360];

    WindVectorPlus* nudgeWV;
    List<CAPWindVectorPlus> ambiguities;
    AngleIntervalList directionRanges;

    unsigned int s0_flag;
    unsigned char cap_flag;

};

#endif