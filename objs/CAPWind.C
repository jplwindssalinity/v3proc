#include <vector>
#include "CAPWind.h"
#include "Misc.h"

CAPWVC::CAPWVC() : nudgeWV(NULL), selected(NULL), selected_allocated(0) {
    return;
}

CAPWVC::~CAPWVC() {

    CAPWindVectorPlus* wvp;
    ambiguities.GotoHead();

    while ((wvp = ambiguities.RemoveCurrent()) != NULL) {
        if (wvp == selected)
            selected_allocated = 0;
        delete wvp;
    }

    if (selected_allocated) {
        if (selected != NULL)
            delete selected;
    }

    if (nudgeWV) {
        delete nudgeWV;
        nudgeWV = NULL;
    }

    return;
}

void CAPWVC::FreeContents() {

    CAPWindVectorPlus* wvp;
    ambiguities.GotoHead();
    while ((wvp = ambiguities.RemoveCurrent()))
        delete wvp;
    return;
}

int CAPWVC::GetBestSolution(
    float direction, float* spd, float* sss, float* obj) {

    float azi_spacing = 360/(float)n_azi;

    float fazi = direction * rtd / azi_spacing;
    while(fazi<0) fazi += n_azi;
    while(fazi>=n_azi) fazi -= n_azi;

    float dazi = (fazi - floor(fazi))/azi_spacing;

    int iazi0 = (int)floor(fazi);
    int iazi1 = iazi0 + 1;

    *spd = best_spd[iazi0]*(1-dazi) + dazi*best_spd[iazi1];
    *sss = best_sss[iazi0]*(1-dazi) + dazi*best_sss[iazi1];
    *obj = best_obj[iazi0]*(1-dazi) + dazi*best_obj[iazi1];

    return(1);
}

CAPWindVectorPlus* CAPWVC::GetNearestAmbig(float direction, int rank_idx) {

    float min_diff = 9999;
    CAPWindVectorPlus* nearest_wvp = NULL;
    int rank = 0;
    for(CAPWindVectorPlus* wvp = ambiguities.GetHead(); wvp;
        wvp = ambiguities.GetNext()){

        float this_diff = ANGDIF(wvp->dir, direction);
        if(this_diff < min_diff && rank < rank_idx) {
            nearest_wvp = wvp;
            min_diff = this_diff;
        }
        rank++;
    }
    return(nearest_wvp);
}

int CAPWVC::BuildSolutions() {

    FreeContents();
    float azi_spacing = 360 / (float)n_azi;

    double obj_smooth[n_azi];
    for(int iazi = 0; iazi < n_azi; ++iazi) {
        int previdx = (iazi==0) ? n_azi-1 : iazi - 1;
        int nextidx = (iazi==n_azi-1) ? 0 : iazi + 1;
        obj_smooth[iazi] = (
            best_obj[previdx]+best_obj[iazi]+best_obj[nextidx]) / 3.0;
    }

    for(int iazi = 0; iazi < n_azi; ++iazi) {
        int previdx = (iazi==0) ? n_azi-1 : iazi - 1;
        int nextidx = (iazi==n_azi-1) ? 0 : iazi + 1;
        obj_smooth[iazi] = (
            obj_smooth[previdx]+obj_smooth[iazi]+obj_smooth[nextidx]) / 3.0;
    }

    // search through best (spd, sss, obj) curves
    for(int iazi = 0; iazi < n_azi; ++iazi) {
        int previdx = (iazi==0) ? n_azi-1 : iazi - 1;
        int previdx2 = (previdx==0) ? n_azi-1 : previdx - 1;

        int nextidx = (iazi==n_azi-1) ? 0 : iazi + 1;

        // check for maxima
        if(obj_smooth[iazi] > obj_smooth[previdx] &&
           obj_smooth[iazi] > obj_smooth[nextidx]) {

            // Found a local maxima, append it to ambiguities
            CAPWindVectorPlus* this_ambig = new CAPWindVectorPlus();
            this_ambig->spd = best_spd[iazi];
            this_ambig->dir = dtr * (float)iazi * azi_spacing;
            this_ambig->sss = best_sss[iazi];
            this_ambig->obj = obj_smooth[iazi];
            ambiguities.Append(this_ambig);

        } else if(
            // check for rare case when two have same max value (look backwards)
            obj_smooth[iazi] == obj_smooth[previdx] &&
            obj_smooth[iazi] > obj_smooth[nextidx] &&
            obj_smooth[iazi] > obj_smooth[previdx2]) {

            // put the ambig right in the middle
            float this_dir = dtr * ((float)iazi-0.5) * azi_spacing;
            if(this_dir<0) this_dir+=two_pi;

            CAPWindVectorPlus* this_ambig = new CAPWindVectorPlus();
            this_ambig->spd = 0.5*(best_spd[iazi]+best_spd[previdx]);
            this_ambig->dir = this_dir;
            this_ambig->sss = 0.5*(best_sss[iazi]+best_sss[previdx]);
            this_ambig->obj = obj_smooth[iazi];
            ambiguities.Append(this_ambig);
        }
    }

    // quit if no ambigs (will segfault below)
    if(ambiguities.NodeCount() == 0)
        return(0);

    // Sort by objective function value
    SortByObj();

    // Remove the smallest obj ambigs if more than 4
    if(ambiguities.NodeCount() > 4) {
        CAPWindVectorPlus* wvp = NULL;

        ambiguities.GotoHead();
        for(int iamb = 0; iamb < 4; ++iamb)
            wvp = ambiguities.GetNext();

        while(wvp != NULL) {
            wvp = ambiguities.RemoveCurrent();
            delete(wvp);
        }
    }

    // Convert the objective function values to psuedo-PDF
    double pdf[n_azi];
    double max_obj = obj_smooth[0];

    for(int iazi = 1; iazi < n_azi; ++iazi)
        if(obj_smooth[iazi] > max_obj)
            max_obj = obj_smooth[iazi];

    double sum_pdf = 0;
    for(int iazi = 0; iazi < n_azi; ++iazi) {
        pdf[iazi] = exp((obj_smooth[iazi]-max_obj)/2);
        sum_pdf += pdf[iazi];
    }

    for(int iazi = 0; iazi < n_azi; ++iazi)
        pdf[iazi] /= sum_pdf;

    // Build out the direction ranges
    double pdf_included = 0;
    std::vector<int> left_idx;
    std::vector<int> right_idx;
    for(CAPWindVectorPlus* wvp = ambiguities.GetHead(); wvp;
        wvp = ambiguities.GetNext()){

        int idx = (int)round(rtd * wvp->dir / azi_spacing);
        left_idx.push_back(idx);
        right_idx.push_back(idx);
        pdf_included += pdf[idx];
    }

    while(pdf_included < 0.99) {

        double current_max_pdf = 0;
        int current_max_idx;
        int current_max_ambig;
        int current_max_is_left;

        for(int iamb = 0; iamb < ambiguities.NodeCount(); ++iamb) {

            int left_ = (left_idx[iamb] == 0) ? n_azi-1 : left_idx[iamb] - 1;
            int right_ = (right_idx[iamb] == n_azi-1) ? 0 : right_idx[iamb] + 1;

            // Check for angle intervals hitting each other
            int step_out_left = 1;
            int step_out_right = 1;

            for(int jamb = 0; jamb < ambiguities.NodeCount(); ++jamb) {
                if(jamb != iamb) {
                    if(left_ == right_idx[jamb])
                        step_out_left = 0;

                    if(right_ == left_idx[jamb])
                        step_out_right = 0;
                }
            }

            if(step_out_left && pdf[left_] >= current_max_pdf) {
                current_max_pdf = pdf[left_];
                current_max_idx = left_;
                current_max_ambig = iamb;
                current_max_is_left = 1;
            }

            if(step_out_right && pdf[right_] >= current_max_pdf) {
                current_max_pdf = pdf[right_];
                current_max_idx = right_;
                current_max_ambig = iamb;
                current_max_is_left = 0;
            }
        }

        pdf_included += pdf[current_max_idx];

        if(current_max_is_left) {
            left_idx[current_max_ambig] = current_max_idx;
        } else {
            right_idx[current_max_ambig] = current_max_idx;
        }
    }

    int this_ambig = 0;
    for(CAPWindVectorPlus* wvp = ambiguities.GetHead(); wvp;
        wvp = ambiguities.GetNext()){

        wvp->directionRange.SetLeftRight(
            (float)left_idx[this_ambig]*azi_spacing*dtr,
            (float)right_idx[this_ambig]*azi_spacing*dtr);

        this_ambig++;
    }
    return(1);
}

// cribbed from Wind.C
int CAPWVC::SortByObj() {
    int need_sorting;
    do {
        need_sorting = 0;
        for(CAPWindVectorPlus* wvp = ambiguities.GetHead(); wvp;
            wvp = ambiguities.GetNext()) {

            CAPWindVectorPlus* next_wvp = ambiguities.GetNext();
            if (next_wvp) {
                ambiguities.GotoPrev();
                if (next_wvp->obj > wvp->obj) {
                    ambiguities.SwapCurrentAndNext();
                    need_sorting = 1;
                }
            }
        }
    } while (need_sorting);
    return(1);
}

CAPWindVectorPlus::CAPWindVectorPlus() {
    return;
}

CAPWindVectorPlus::~CAPWindVectorPlus() {
    return;
}
