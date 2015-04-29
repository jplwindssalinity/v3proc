#ifndef CAPWINDSWATH_H
#define CAPWINDSWATH_H

#include "CAPWind.h"

class CAPWindSwath {
public:

    CAPWindSwath();
    ~CAPWindSwath();

    int Allocate(int cross_track_bins, int along_track_bins);
    int Add(int cti, int ati, CAPWVC* wvc);
    int ThreshNudge(float thres);
    int MedianFilter(int half_window_size, int max_passes);
    int MedianFilterPass(
        int half_window_size, CAPWindVectorPlus*** selected, char** change,
        char** influence);

    int DIRFilter(int half_window_size, int max_passes);
    int DIRFilterPass(int half_window_size, CAPWindVectorPlus*** selected);


    int DeleteEntireSwath();
    int DeleteWVCs();

    CAPWVC***  swath;

protected:

    int _Allocate();
    int _Deallocate();
    int  _crossTrackBins;
    int  _alongTrackBins;

};

#endif