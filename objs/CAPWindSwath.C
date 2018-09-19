#include <algorithm>
#include <vector>
#include "Array.h"
#include "CAPWindSwath.h"

#define FLIPPING_WITHIN_RANGE_THRESHOLD  (5.0*dtr)

CAPWindSwath::CAPWindSwath() : swath(NULL) {
    return;
}

CAPWindSwath::~CAPWindSwath() {
    DeleteEntireSwath();
    return;
}

int CAPWindSwath::Allocate(int cross_track_bins, int along_track_bins) {
    _crossTrackBins = cross_track_bins;
    _alongTrackBins = along_track_bins;
    return(_Allocate());
}

int CAPWindSwath::Add(int cti, int ati, CAPWVC* wvc) {

    // Check in swath, delete it if not in range
    if(cti < 0 || cti >= _crossTrackBins ||
       ati < 0 || ati >= _alongTrackBins) {
       delete wvc;
       return(0);
    }

    if (swath[cti][ati]) {
        fprintf(stderr, "CAPWindSwath::Add: attempted cell replacement\n");
        fprintf(stderr, "  cti = %d, ati = %d\n", cti, ati);
        return(0);    // already a cell there
    }

    swath[cti][ati] = wvc;
    return(1);
}

int CAPWindSwath::ThreshNudge(float thres) {

    for(int cti = 0; cti < _crossTrackBins; ++cti) {
        for(int ati = 0; ati < _alongTrackBins; ++ati) {

            CAPWVC* wvc = swath[cti][ati];

            if(!wvc)
                continue;

            int rank_idx = 0;
            CAPWindVectorPlus* head = wvc->ambiguities.GetHead();

            for (CAPWindVectorPlus* wvp = wvc->ambiguities.GetHead();
                 wvp; wvp = wvc->ambiguities.GetNext()) {
                if(exp(0.5*(wvp->obj - head->obj)) >= thres)
                    rank_idx++;
            }

            if (wvc->nudgeWV==NULL)
                continue;

            wvc->selected = wvc->GetNearestAmbig(wvc->nudgeWV->dir, rank_idx);

        }
    }

    return(1);
}

int CAPWindSwath::MedianFilterTBWinds(
    int half_window, int max_passes, int start_pass, int skip_dirth_pass){

    CAPWindVectorPlus*** new_selected = (CAPWindVectorPlus***)make_array(
        sizeof(CAPWindVectorPlus*), 2, _crossTrackBins, _alongTrackBins);

    char** change = (char**)make_array(
        sizeof(char), 2, _crossTrackBins, _alongTrackBins);

    char** filter = (char**)make_array(
        sizeof(char), 2, _crossTrackBins, _alongTrackBins);

    char** influence = (char**)make_array(
        sizeof(char), 2, _crossTrackBins, _alongTrackBins);

    int total_passes = 0;

    // Pass 0: Only filter high-winds, non-land/ice
    // Pass 1: Fix high-winds, filter low-winds non-land/ice
    // Pass 2: Filter low+high winds non-land/ice
    // Pass 3: Fix all non-land/ice, filter land/ice
    // Pass 4: DIR processing
    for(int ipass = start_pass; ipass < 5; ++ipass) {

        // Do DIR on last pass
        int special = (ipass == 4) ? 1 : 0;

        if(special && skip_dirth_pass)
            continue;

        for(int cti = 0; cti < _crossTrackBins; cti++) {
            for (int ati = 0; ati < _alongTrackBins; ati++) {
                CAPWVC* wvc = swath[cti][ati];

                if(!wvc) {
                    change[cti][ati] = 0;
                    influence[cti][ati] = 0;
                    continue;
                }

                int is_landice = 
                    wvc->s0_flag & (L2B_QUAL_FLAG_LAND|L2B_QUAL_FLAG_ICE);

                int is_low_winds = wvc->nudgeWV->spd < 12.5;

                if(is_landice) {
                    if(ipass < 3) {
                        change[cti][ati] = 0;
                        filter[cti][ati] = 0;
                        influence[cti][ati] = 0;

                    } else if(ipass == 3) {
                        change[cti][ati] = 1;
                        filter[cti][ati] = 1;
                        influence[cti][ati] = 1;

                    } else {
                        change[cti][ati] = 1;
                        filter[cti][ati] = 1;
                        influence[cti][ati] = 0;
                    }

                } else {

                    if(is_low_winds) {
                        if(ipass == 0) {
                            change[cti][ati] = 0;
                            filter[cti][ati] = 0;
                            influence[cti][ati] = 0;

                        } else if(ipass == 1 || ipass == 2) {
                            change[cti][ati] = 1;
                            filter[cti][ati] = 1;
                            influence[cti][ati] = 1;

                        } else if(ipass == 3) {
                            change[cti][ati] = 0;
                            filter[cti][ati] = 0;
                            influence[cti][ati] = 1;

                        } else if(ipass == 4) {
                            change[cti][ati] = 1;
                            filter[cti][ati] = 1;
                            influence[cti][ati] = 1;

                        }

                    } else {
                        if(ipass == 0) {
                            change[cti][ati] = 1;
                            filter[cti][ati] = 1;
                            influence[cti][ati] = 1;

                        } else if(ipass == 1) {
                            change[cti][ati] = 0;
                            filter[cti][ati] = 0;
                            influence[cti][ati] = 1;

                        } else if(ipass == 2) {
                            change[cti][ati] = 1;
                            filter[cti][ati] = 1;
                            influence[cti][ati] = 1;

                        } else if(ipass == 3) {
                            change[cti][ati] = 0;
                            filter[cti][ati] = 0;
                            influence[cti][ati] = 1;

                        } else if(ipass == 4) {
                            change[cti][ati] = 1;
                            filter[cti][ati] = 1;
                            influence[cti][ati] = 1;

                        }
                    }
                }
            }
        }

        // Apply the median filter until done
        int pass = 0;
        while (pass < max_passes) {
            int flips = MedianFilterPass(
                half_window, new_selected, change, filter, influence, special);

            if (flips == 0)
                break;

            pass++;
        }
        printf("On medfilt pass: %d; passes: %d\n", ipass, pass);
        total_passes += pass;
    }

    free_array(new_selected, 2, _crossTrackBins, _alongTrackBins);
    free_array(change, 2, _crossTrackBins, _alongTrackBins);
    free_array(filter, 2, _crossTrackBins, _alongTrackBins);
    free_array(influence, 2, _crossTrackBins, _alongTrackBins);

    return(total_passes);
}

int CAPWindSwath::MedianFilter(
    int half_window, int max_passes, int start_pass, int skip_dirth_pass){

    CAPWindVectorPlus*** new_selected = (CAPWindVectorPlus***)make_array(
        sizeof(CAPWindVectorPlus*), 2, _crossTrackBins, _alongTrackBins);

    char** change = (char**)make_array(
        sizeof(char), 2, _crossTrackBins, _alongTrackBins);

    char** filter = (char**)make_array(
        sizeof(char), 2, _crossTrackBins, _alongTrackBins);

    char** influence = (char**)make_array(
        sizeof(char), 2, _crossTrackBins, _alongTrackBins);

    int total_passes = 0;

    // Pass 1: Only filter non-land/ice, ignore land/ice WVCs
    // Pass 2: Fix non-land/ice selections, filter land/ice WVCs
    // Pass 3: DIR processing
    for(int ipass = start_pass; ipass < 3; ++ipass) {

        // Do DIR on last pass
        int special = (ipass == 2) ? 1 : 0;

        if(special && skip_dirth_pass)
            continue;

        for(int cti = 0; cti < _crossTrackBins; cti++) {
            for (int ati = 0; ati < _alongTrackBins; ati++) {
                CAPWVC* wvc = swath[cti][ati];

                if(!wvc) {
                    change[cti][ati] = 0;
                    influence[cti][ati] = 0;
                    continue;
                }

                if(wvc->s0_flag & (L2B_QUAL_FLAG_LAND|L2B_QUAL_FLAG_ICE)) {
                    if(ipass == 0) {
                        change[cti][ati] = 0;
                        filter[cti][ati] = 0;
                        influence[cti][ati] = 0;

                    } else if(ipass == 1 || ipass == 2) {
                        change[cti][ati] = 1;
                        filter[cti][ati] = 1;
                        influence[cti][ati] = 1;
                    }

                } else {
                    if(ipass == 0 || ipass == 2) {
                        change[cti][ati] = 1;
                        filter[cti][ati] = 1;
                        influence[cti][ati] = 1;

                    } else if(ipass == 1) {
                        change[cti][ati] = 0;
                        filter[cti][ati] = 0;
                        influence[cti][ati] = 1;
                    }
                }
            }
        }

        // Apply the median filter until done
        int pass = 0;
        while (pass < max_passes) {
            int flips = MedianFilterPass(
                half_window, new_selected, change, filter, influence, special);

            if (flips == 0)
                break;

            pass++;
        }
        printf("On medfilt pass: %d; passes: %d\n", ipass, pass);
        total_passes += pass;
    }

    free_array(new_selected, 2, _crossTrackBins, _alongTrackBins);
    free_array(change, 2, _crossTrackBins, _alongTrackBins);
    free_array(filter, 2, _crossTrackBins, _alongTrackBins);
    free_array(influence, 2, _crossTrackBins, _alongTrackBins);

    return(total_passes);
}

int CAPWindSwath::MedianFilterPass(
    int half_window, CAPWindVectorPlus*** new_selected, char** change,
    char** filter, char** influence, int special) {

    int flips = 0;
    for (int cti = 0; cti < _crossTrackBins; cti++) {

        int cti_min = std::max(cti - half_window, 0);
        int cti_max = std::min(cti + half_window + 1, _crossTrackBins);

        for (int ati = 0; ati < _alongTrackBins; ati++) {

            int ati_min = std::max(ati - half_window, 0);
            int ati_max = std::min(ati + half_window + 1, _alongTrackBins);

            new_selected[cti][ati] = NULL;

            CAPWVC* wvc = swath[cti][ati];

            if (!wvc || !filter[cti][ati])
                continue;  // goto next cell

            int sum_change = 0;
            int sum_influence = 0;
            for(int i = cti_min; i < cti_max; ++i) {
                for(int j = ati_min; j < ati_max; ++j) {
                    sum_change += change[i][j];
                    sum_influence += influence[i][j];
                }
            }

            // Nothing to do
            if(sum_change == 0 || sum_influence == 0)
                continue;

            if(special == 0) {
                float min_vector_dif_sum = (float)HUGE_VAL;

                for(CAPWindVectorPlus* wvp = wvc->ambiguities.GetHead(); wvp;
                    wvp = wvc->ambiguities.GetNext()) {

                    float vector_dif_sum = 0.0;
                    float x1 = wvp->spd * cos(wvp->dir);
                    float y1 = wvp->spd * sin(wvp->dir);

                    for(int i = cti_min; i < cti_max; ++i) {
                        for(int j = ati_min; j < ati_max; ++j) {

                            CAPWVC* other_wvc = swath[i][j];

                            if(!other_wvc || !influence[i][j])
                                continue;

                            CAPWindVectorPlus* other_wvp = other_wvc->selected;
                            if (!other_wvp)
                                continue;

                            float x2 = other_wvp->spd * cos(other_wvp->dir);
                            float y2 = other_wvp->spd * sin(other_wvp->dir);

                            float dx = x2 - x1;
                            float dy = y2 - y1;
                            vector_dif_sum += sqrt(dx*dx + dy*dy);
                        }
                    }

                    if(vector_dif_sum < min_vector_dif_sum) {
                        min_vector_dif_sum = vector_dif_sum;
                        new_selected[cti][ati] = wvp;
                    }
                }

            } else {
                // DIR processing
                // Compute median wind vector in window
                std::vector<float> x_window, y_window;
                for(int i = cti_min; i < cti_max; ++i) {
                    for(int j = ati_min; j < ati_max; ++j) {

                        CAPWVC* other_wvc = swath[i][j];

                        if(!other_wvc || !influence[i][j])
                            continue;

                        CAPWindVectorPlus* other_wvp = other_wvc->selected;
                        if (!other_wvp)
                            continue;

                        x_window.push_back(other_wvp->spd*cos(other_wvp->dir));
                        y_window.push_back(other_wvp->spd*sin(other_wvp->dir));
                    }
                }

                std::sort(x_window.begin(), x_window.end());
                std::sort(y_window.begin(), y_window.end());

                float x_med, y_med;
                int n_ele = x_window.size();
                if(n_ele % 2 == 1) {
                    x_med = x_window[n_ele/2];
                    y_med = y_window[n_ele/2];

                } else {
                    x_med = 0.5*(x_window[n_ele/2-1] + x_window[n_ele/2]);
                    y_med = 0.5*(y_window[n_ele/2-1] + y_window[n_ele/2]);
                }

                float median_dir = atan2(y_med, x_med);

                // wrap it to [0, two_pi) interval
                if(median_dir<0) median_dir += two_pi;
                if(median_dir>two_pi) median_dir -= two_pi;

                float left_dir = wvc->selected->directionRange.left;
                float right_dir = wvc->selected->directionRange.right;

                // allocate storage for filtered CAPWindVectorPlus
                CAPWindVectorPlus* wvp = new CAPWindVectorPlus;

                // Copy over direction range object
                wvp->directionRange.SetLeftRight(left_dir, right_dir);

                // Pick nerest direction in wvc->selected->directionRange
                if(wvc->selected->directionRange.InRange(median_dir)) {
                    wvp->dir = median_dir;

                } else {

                    float delta_left = ANGDIF(left_dir, median_dir);
                    float delta_right = ANGDIF(right_dir, median_dir);

                    if(fabs(delta_left) < fabs(delta_right)) {
                        wvp->dir = wvc->selected->directionRange.left;

                    } else {
                        wvp->dir = wvc->selected->directionRange.right;
                    }
                }

                // Interpolate spd, sss, obj to this direction
                wvc->GetBestSolution(wvp->dir, &wvp->spd, &wvp->sss, &wvp->obj);

                new_selected[cti][ati] = wvp;
            }
        }
    }

    for (int cti = 0; cti < _crossTrackBins; cti++) {
        for (int ati = 0; ati < _alongTrackBins; ati++) {

            change[cti][ati] = 0;

            if(new_selected[cti][ati] != NULL) {

                if(special) {

                    if (swath[cti][ati]->selected_allocated) {
                        double dif = ANGDIF(
                            swath[cti][ati]->selected->dir,
                            new_selected[cti][ati]->dir);

                        if (dif > FLIPPING_WITHIN_RANGE_THRESHOLD)
                            change[cti][ati]=1;
                        else
                            change[cti][ati]=0;

                        delete swath[cti][ati]->selected;

                    } else {
                        swath[cti][ati]->selected_allocated = 1;
                        change[cti][ati] = 1;
                    }
                    swath[cti][ati]->selected = new_selected[cti][ati];
                    flips += change[cti][ati];

                } else {
                    if (new_selected[cti][ati] != swath[cti][ati]->selected) {
                        change[cti][ati] = 1;
                        swath[cti][ati]->selected = new_selected[cti][ati];
                        flips++;
                    }
                }
            }
        }
    }
    printf("Flips %d\n", flips);
    fflush(stdout);
    return(flips);
}

int CAPWindSwath::DeleteEntireSwath() {
    if(! DeleteWVCs())
        return(0);

    if(! _Deallocate())
        return(0);

    return(1);
}

int CAPWindSwath::DeleteWVCs() {
    for (int i = 0; i < _crossTrackBins; i++) {
        for (int j = 0; j < _alongTrackBins; j++) {
            CAPWVC* wvc = *(*(swath + i) + j);
            if (wvc == NULL)
                continue;

            delete wvc;
            *(*(swath + i) + j) = NULL;
        }
    }
    return(1);
}

int CAPWindSwath::_Allocate() {
    if(swath != NULL)
        return(0);

    swath = (CAPWVC ***)make_array(
        sizeof(CAPWVC *), 2, _crossTrackBins, _alongTrackBins);

    if(swath == NULL)
        return(0);

    for(int i = 0; i < _crossTrackBins; i++) {
        for (int j = 0; j < _alongTrackBins; j++) {
            swath[i][j] = NULL;
        }
    }

    return(1);
}

int CAPWindSwath::_Deallocate() {
    if(swath == NULL)
        return(1);

    free_array((void *)swath, 2, _crossTrackBins, _alongTrackBins);
    swath = NULL;
    _crossTrackBins = 0;
    _alongTrackBins = 0;
    return(1);
}
