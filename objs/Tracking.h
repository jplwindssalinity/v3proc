//==============================================================//
// Copyright (C) 1998-1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#ifndef TRACKING_H
#define TRACKING_H

static const char rcs_id_tracking_h[] =
    "@(#) $Id$";

#include <stdio.h>

//======================================================================
// CLASSES
//    TrackerBase, RangeTracker, DopplerTracker
//======================================================================

float  Cosine(float angle);

//======================================================================
// CLASS
//    TrackerBase
//
// DESCRIPTION
//    The TrackerBase class is a base class for the RangeTracker and
//    DopplerTracker objects.
//======================================================================

#define SPARE_WORDS  2

template <class T>
class TrackerBase
{
public:

    enum { AMPLITUDE_INDEX = 0, PHASE_INDEX, BIAS_INDEX };

    //--------------//
    // construction //
    //--------------//

    TrackerBase();
    ~TrackerBase();

    int  Allocate(unsigned int steps);

    //--------------//
    // input/output //
    //--------------//

    int  ReadBinary(const char* filename);
    int  WriteBinary(const char* filename);
    int  ReadOldBinary(const char* filename);
    int  WriteOldBinary(const char* filename);
    int  ReadHex(const char* filename);
    int  WriteHex(const char* filename);
    int  WriteAscii(const char* filename);

    unsigned short  GetTableId()  { return(_tableId); };

    int  SetFromMro(char* mro);

protected:

    //-----------//
    // variables //
    //-----------//

    unsigned short  _tableId;
    float**         _scaleArray;    // [term][coef_order]
    T**             _termArray;     // [step][term]
    unsigned int    _steps;
    unsigned short  _dither[2];
};

#define DEFAULT_STEPS  256

//======================================================================
// CLASS
//    RangeTracker
//
// DESCRIPTION
//    The RangeTracker object is used to store the Range Tracking
//    Constants and convert them into receiver gate widths and delays.
//======================================================================

class RangeTracker : public TrackerBase<unsigned char>
{
 public:

    enum { AMPLITUDE_INDEX = 0, PHASE_INDEX, BIAS_INDEX };

    //--------------//
    // construction //
    //--------------//

    RangeTracker();
    RangeTracker(const RangeTracker& from) { *this = from; return; }
    ~RangeTracker();

    //--------------//
    // input/output //
    //--------------//

    int  ReadGS(const char* filename, RangeTracker* second_set);
    int  WriteGS(const char* filename, RangeTracker* second_set);
    int  MroAssemble(unsigned char type, unsigned short offset, char* data,
             int* beam_idx);

    //-----------//
    // operators //
    //-----------//

    RangeTracker&  operator=(const RangeTracker& from);

    //------------//
    // algorithms //
    //------------//

    int    GetRxGateDelay(unsigned short range_step,
             unsigned short azimuth_step, unsigned char rx_gate_width_dn,
             unsigned char tx_pulse_width_dn, unsigned char* rx_gate_delay_dn,
             float* rx_gate_delay_fdn);
    int    SetRoundTripTime(double** terms);

    //-----------//
    // variables //
    //-----------//

    float  rxRangeMem;
};


//======================================================================
// CLASS
//    DopplerTracker
//
// DESCRIPTION
//    The DopplerTracker object is used to store the Doppler Tracking
//    Constants and convert them into command Doppler frequencies.
//======================================================================

#define DOPPLER_TRACKING_RESOLUTION  2000    // Hz

class DopplerTracker : public TrackerBase<unsigned short>
{
public:

    enum { AMPLITUDE_INDEX = 0, PHASE_INDEX, BIAS_INDEX };

    //--------------//
    // construction //
    //--------------//

    DopplerTracker();
    DopplerTracker(const DopplerTracker& from) { *this = from; return; }
    ~DopplerTracker();

    //--------------//
    // input/output //
    //--------------//

    int  ReadGS(const char* filename, DopplerTracker* second_set);
    int  WriteGS(const char* filename, DopplerTracker* second_set);
    int  MroAssemble(unsigned char type, unsigned short offset, char* data,
             int* beam_idx);

    //-----------//
    // operators //
    //-----------//

    DopplerTracker& operator=(const DopplerTracker& from);

    //------------//
    // algorithms //
    //------------//

    int  GetCommandedDoppler(unsigned short doppler_step,
             unsigned short azimuth_step, unsigned char rx_gate_delay_dn,
             float rx_gate_delay_fdn, short* commanded_doppler_dn);
    int  SetTerms(double** terms);
    int  GetTerms(double** terms);

    //----------//
    // variable //
    //----------//

    float  tableFrequency;    // for access to intermediate info
};

//==================//
// Helper functions //
//==================//

int  azimuth_fit(int count, double* terms, double* a, double* p, double* c);
int  write_hex(FILE* fp, char* buffer, int bytes);
int  read_hex(FILE* fp, char* buffer, int bytes);

#endif
