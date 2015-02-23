#include "Meas.h"
#include "Array.h"

class CAPGMF {
public:
    CAPGMF();
    ~CAPGMF();

    int ReadFlat(const char* filename);
    int ReadRough(const char* filename);

    int GetTBFlat(
        Meas::MeasTypeE met, float inc, float sst, float sss, float* tbflat);

    int GetDTB(
        Meas::MeasTypeE met, float inc, float wspd, float relazi, float* dtb);

    int GetTB(
        Meas::MeasTypeE met, float inc, float sst, float sss, float wspd,
        float relazi, float* tb);

protected:

    int _Allocate();
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

    static const int _metCount = 2;

    float**** _tbflat;
};
