#include "Array.h"
#include "CAPWindSwath.h"

CAPWindSwath::CAPWindSwath() {
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
