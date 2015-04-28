#include <vector>
#include "CAPWind.h"

CAPWVC::CAPWVC() : nudgeWV(NULL) {
    return;
}

CAPWVC::~CAPWVC() {
    return;
}

void CAPWVC::FreeContents() {

    CAPWindVectorPlus* wvp;
    ambiguities.GotoHead();
    while ((wvp = ambiguities.RemoveCurrent()))
        delete wvp;
    return;
}

int CAPWVC::BuildSolutions() {

    FreeContents();

    float pdf[360], sum_pdf = 0;

    std::vector<int> left_idx;
    std::vector<int> right_idx;

    // search through best (spd, sss, obj) curves
    for(int iazi = 0; iazi < 360; ++iazi) {
        int previdx = (iazi==0) ? 359 : iazi - 1;
        int nextidx = (iazi==359) ? 0 : iazi + 1;

        if(best_obj[iazi] < best_obj[previdx] &&
           best_obj[iazi] < best_obj[nextidx]) {

            // Found a local minima, append it to ambiguities
            CAPWindVectorPlus* this_ambig = new CAPWindVectorPlus();
            this_ambig->spd = best_spd[iazi];
            this_ambig->dir = dtr * (float)iazi;
            this_ambig->sss = best_sss[iazi];
            this_ambig->obj = best_obj[iazi];
            ambiguities.Append(this_ambig);
            left_idx.push_back(iazi);
            right_idx.push_back(iazi);
        }
        pdf[iazi] = exp(-best_obj[iazi]);
        sum_pdf += pdf[iazi];
    }

    // Build out the direction ranges
    float pdf_included = 0;
    for(int iamb = 0; iamb < ambiguities.NodeCount(); ++iamb) {
        pdf_included += pdf[left_idx[iamb]];
    }

    while(pdf_included < 0.99 * sum_pdf) {
        for(int iamb = 0; iamb < ambiguities.NodeCount(); ++iamb) {

            // Step them out 1 degree on each side
            int left_ = (left_idx[iamb]==0) ? 359 : left_idx[iamb] - 1;
            int right_ = (right_idx[iamb]==359) ? 0 : right_idx[iamb] + 1;

            // Check for angle intervals hitting each other
            int step_out_left = 1;
            int step_out_right = 1;

            for(int jamb = 0; jamb < ambiguities.NodeCount(); ++jamb) {
                if(jamb != iamb) {
                    if(left_ == left_idx[jamb])
                        step_out_left = 0;

                    if(right_ == right_idx[jamb])
                        step_out_right = 0;
                }
            }

            if(step_out_left) {
                left_idx[iamb] = left_;
                pdf_included += pdf[left_idx[iamb]];
            }

            if(step_out_right) {
                right_idx[iamb] = right_;
                pdf_included += pdf[right_idx[iamb]];
            }
        }
    }

    directionRanges.FreeContents();
    for(int iamb = 0; iamb < ambiguities.NodeCount(); ++iamb) {
        AngleInterval* this_dir_range = new AngleInterval();
        this_dir_range->SetLeftRight(left_idx[iamb]*dtr, right_idx[iamb]*dtr);
        directionRanges.Append(this_dir_range);
    }

    return(1);
}

CAPWindVectorPlus::CAPWindVectorPlus() {
    return;
}

CAPWindVectorPlus::~CAPWindVectorPlus() {
    return;
}
