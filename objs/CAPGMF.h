#include "Meas.h"
#include "Array.h"

class CAPGMF {
public:
    CAPGMF();
    ~CAPGMF();

    int ReadFlat(const char* filename);
    int ReadRough(const char* filename);

    int _MetToIndex(Meas::MeasTypeE met);

    int GetTBFlat(
        Meas::MeasTypeE met, float inc, float sst, float sss, float* tbflat);

    int GetDTB(
        Meas::MeasTypeE met, float inc, float sst, float spd, float dir,
        float* dtb);

    int GetTB(
        Meas::MeasTypeE met, float inc, float sst, float sss, float spd,
        float dir, float* tb);

protected:

    int _AllocateFlat();
    int _AllocateRough();
    int _Deallocate();

    static const int _incCount = 11;
    static const float _incMin = 35;
    static const float _incStep = 1.0;

    static const int _sstCount = 43;
    static const float _sstMin = -2.0;
    static const float _sstStep = 1.0;

    static const int _sssCount = 41;
    static const float _sssMin = 0.0;
    static const float _sssStep = 1.0;

    static const int _spdCount = 501;
    static const float _spdMin = 0.0;
    static const float _spdStep = 0.1;

    static const int _dirCount = 360;
    static const float _dirMin = 0.0;
    static const float _dirStep = 1.0;

    static const int _metCount = 2;

    float**** _tbflat;
    float**** _erough;
};
