#ifndef CAPWIND_H
#define CAPWIND_H
#include "List.h"
#include "Wind.h"
#include "AngleInterval.h"

class CAPWindVectorPlus : public WindVectorPlus {
public:
    CAPWindVectorPlus();
    ~CAPWindVectorPlus();

    AngleInterval directionRange;
    float sss;
};

class CAPWVC {
public:

    CAPWVC();
    ~CAPWVC();

    void FreeContents();
    int BuildSolutions();
    int SortByObj();
    CAPWindVectorPlus* GetNearestAmbig(float direction, int rank_idx = 4);
    int GetBestSolution(float direction, float* spd, float* sss, float* obj);

    static const unsigned int n_azi = 180;

    float best_spd[n_azi];
    float best_obj[n_azi];
    float best_sss[n_azi];

    WindVectorPlus* nudgeWV;
    List<CAPWindVectorPlus> ambiguities;

    CAPWindVectorPlus* selected;

    int selected_allocated;
    unsigned int s0_flag;
    unsigned char cap_flag;

};

#endif