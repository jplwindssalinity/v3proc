//==============================================================//
// Copyright (C) 1998-2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#ifndef TRACKING_H
#define TRACKING_H

static const char rcs_id_tracking_h[] =
    "@(#) $Id$";

#include <stdio.h>
#include <string.h>
#include "Constants.h"
#include "Array.h"

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
    void SetEndian();
     //-----------//
    // variables //
    //-----------//

    unsigned short  _tableId;
    float**         _scaleArray;    // [term][coef_order]
    T**             _termArray;     // [step][term]
    unsigned int    _steps;
    unsigned short  _dither[2];
    bool            _littleEndian;
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

const float RANGE_GATE_NORMALIZER = 0.049903;

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
             int* beam_idx, int* active_idx);

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
    void   ClearAmpPhase();
    void   QuantizeCenter(float effective_gate_width);

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
#define DEFAULT_TRACKING_CHIRP_RATE  250.73  // kHz/ms

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

    void  SetTrackingChirpRate(double tracking_mu);

    //--------------//
    // input/output //
    //--------------//

    int  ReadGS(const char* filename, DopplerTracker* second_set);
    int  WriteGS(const char* filename, DopplerTracker* second_set);
    int  MroAssemble(unsigned char type, unsigned short offset, char* data,
             int* beam_idx, int* active_idx);

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

    double  trackingChirpRate;
};

//==================//
// Helper functions //
//==================//

int  azimuth_fit(int count, double* terms, double* a, double* p, double* c);
int  write_hex(FILE* fp, char* buffer, int bytes);
int  read_hex(FILE* fp, char* buffer, int bytes);
bool ishex(char c); 

//=============//
// TrackerBase //
//=============//

template <class T>
TrackerBase<T>::TrackerBase()
:   _tableId(0), _scaleArray(NULL), _termArray(NULL), _steps(256)
{
    for (int i = 0; i < 2; i++)
        _dither[i] = 0;
    SetEndian();
    return;
}

template <class T>
TrackerBase<T>::~TrackerBase()
{
    return;
}

// determine whether the machine is little or big Endian
template <class T>
void TrackerBase<T>::SetEndian()
{
    unsigned int x = 1;
    char* y = (char*) &x;
     if((int)(y[0])==1){
      _littleEndian=true;
    }
    else{
      _littleEndian=false;
    }
    
    return;
}


//-----------------------//
// TrackerBase::Allocate //
//-----------------------//

template <class T>
int
TrackerBase<T>::Allocate(
    unsigned int  steps)
{
    //-------------------------------//
    // check for previous allocation //
    //-------------------------------//

    if (_scaleArray || _termArray)
        return(0);

    //----------//
    // allocate //
    //----------//

    _scaleArray = (float **)make_array(sizeof(float), 2, 3, 2);
    if (_scaleArray == NULL)
        return(0);

    _termArray = (T **)make_array(sizeof(T), 2, steps, 3);
    if (_termArray == NULL)
        return(0);

    _steps = steps;

    return(1);
}

//-------------------------//
// TrackerBase::ReadBinary //
//-------------------------//

template <class T>
int
TrackerBase<T>::ReadBinary(
    const char*     filename)
{
    //---------------//
    // open the file //
    //---------------//

    FILE* fp = fopen(filename, "r");
    if (fp == NULL)
        return(0);

    //-------//
    // steps //
    //-------//

    if (fread((void *)&_steps, sizeof(unsigned int), 1, fp) != 1)
    {
        fclose(fp);
        return(0);
    }

    //----------//
    // allocate //
    //----------//

    if (! Allocate(_steps))
    {
        fclose(fp);
        return(0);
    }

    //---------------//
    // scale factors //
    //---------------//

    for (unsigned int term = 0; term < 3; term++)
    {
        if (fread((void *) *(_scaleArray + term), sizeof(float), 2, fp) != 2)
        {
            fclose(fp);
            return(0);
        }
    }

    //-------//
    // terms //
    //-------//

    for (unsigned int step = 0; step < _steps; step++)
    {
        if (fread((void *) *(_termArray + step), sizeof(T), 3, fp) != 3)
        {
            fclose(fp);
            return(0);
        }
    }

    //----------------//
    // close the file //
    //----------------//

    fclose(fp);

    return(1);
}

//--------------------------//
// TrackerBase::WriteBinary //
//--------------------------//

template <class T>
int
TrackerBase<T>::WriteBinary(
    const char*  filename)
{
    //---------------//
    // open the file //
    //---------------//

    FILE* fp = fopen(filename, "w");
    if (fp == NULL)
        return(0);

    //-------//
    // steps //
    //-------//

    if (fwrite((void *)&_steps, sizeof(unsigned int), 1, fp) != 1)
    {
        fclose(fp);
        return(0);
    }

    //---------------//
    // scale factors //
    //---------------//

    for (unsigned int term = 0; term < 3; term++)
    {
        if (fwrite((void *) *(_scaleArray + term), sizeof(float), 2, fp) != 2)
        {
            fclose(fp);
            return(0);
        }
    }

    //-------//
    // terms //
    //-------//

    for (unsigned int step = 0; step < _steps; step++)
    {
        if (fwrite((void *) *(_termArray + step), sizeof(T), 3, fp) != 3)
        {
            fclose(fp);
            return(0);
        }
    }

    //----------------//
    // close the file //
    //----------------//

    fclose(fp);

    return(1);
}

//----------------------------//
// TrackerBase::ReadOldBinary //
//----------------------------//

template <class T>
int
TrackerBase<T>::ReadOldBinary(
    const char*  filename)
{
    //---------------//
    // open the file //
    //---------------//

    FILE* fp = fopen(filename, "r");
    if (fp == NULL)
        return(0);

    //-------//
    // steps //
    //-------//

    unsigned short steps;
    if (fread((void *)&steps, sizeof(unsigned short), 1, fp) != 1)
    {
        fclose(fp);
        return(0);
    }
    _steps = (unsigned int)steps;

    //----------//
    // allocate //
    //----------//

    if (! Allocate(_steps))
    {
        fclose(fp);
        return(0);
    }

    //---------------------------------------------------------------//
    // read the terms in the following order: bias, amplitude, phase //
    //---------------------------------------------------------------//

    unsigned int term[3] = { 2, 0, 1 };
    for (int term_idx = 0; term_idx < 3; term_idx++)
    {
        if (fread((void *) ( *(_scaleArray + term[term_idx]) + 1 ),
                sizeof(float), 1, fp) != 1 ||
            fread((void *) ( *(_scaleArray + term[term_idx]) ),
                sizeof(float), 1, fp) != 1)
        {
            fclose(fp);
            return(0);
        }

        for (unsigned int step = 0; step < _steps; step++)
        {
            if (fread((void *) ( *(_termArray + step) + term[term_idx] ),
                sizeof(T), 1, fp) != 1)
            {
                fclose(fp);
                return(0);
            }
        }
    }

    //----------------//
    // close the file //
    //----------------//

    fclose(fp);

    return(1);
}

//-----------------------------//
// TrackerBase::WriteOldBinary //
//-----------------------------//

template <class T>
int
TrackerBase<T>::WriteOldBinary(
    const char*  filename)
{
    //---------------//
    // open the file //
    //---------------//

    FILE* fp = fopen(filename, "w");
    if (fp == NULL)
        return(0);

    //-------//
    // steps //
    //-------//

    unsigned short steps = (unsigned short)_steps;
    if (fwrite((void *)&steps, sizeof(unsigned short), 1, fp) != 1)
    {
        fclose(fp);
        return(0);
    }

    //----------------------------------------------------------------//
    // write the terms in the following order: bias, amplitude, phase //
    //----------------------------------------------------------------//

    unsigned int term[3] = { 2, 0, 1 };
    for (int term_idx = 0; term_idx < 3; term_idx++)
    {
        if (fwrite((void *) ( *(_scaleArray + term[term_idx]) + 1 ),
                sizeof(float), 1, fp) != 1 ||
            fwrite((void *) ( *(_scaleArray + term[term_idx]) ),
                sizeof(float), 1, fp) != 1)
        {
            fclose(fp);
            return(0);
        }

        for (unsigned int step = 0; step < _steps; step++)
        {
            if (fwrite((void *) ( *(_termArray + step) + term[term_idx] ),
                sizeof(T), 1, fp) != 1)
            {
                fclose(fp);
                return(0);
            }
        }
    }

    //----------------//
    // close the file //
    //----------------//

    fclose(fp);

    return(1);
}

//----------------------//
// TrackerBase::ReadHex //
//----------------------//

template <class T>
int
TrackerBase<T>::ReadHex(
    const char*  filename)
{
    //---------------//
    // open the file //
    //---------------//

    FILE* fp = fopen(filename, "r");
    if (fp == NULL)
        return(0);

    //----------//
    // table id //
    //----------//

    if (! read_hex(fp, (char *)&_tableId, sizeof(unsigned short)))
    {
        fclose(fp);
        return(0);
    }

    //-----------//
    // file size //
    //-----------//

    unsigned short file_size;
    if (! read_hex(fp, (char *)&file_size, sizeof(unsigned short)))
    {
        fclose(fp);
        return(0);
    }

    //-------//
    // spare //
    //-------//

    unsigned short spare[SPARE_WORDS];
    if (! read_hex(fp, (char *)spare, SPARE_WORDS * sizeof(unsigned short)))
    {
        fclose(fp);
        return(0);
    }

    //--------//
    // dither //
    //--------//

    if (! read_hex(fp, (char *)_dither, 2 * sizeof(unsigned short)))
    {
        fclose(fp);
        return(0);
    }

    //----------//
    // allocate //
    //----------//

    _steps = DEFAULT_STEPS;
    if (! Allocate(_steps))
    {
        fclose(fp);
        return(0);
    }

    //-------//
    // scale //
    //-------//


    // fixed to avoid word swapping problem if little endian machine is used
    // files written on a different Endian are still incompatible
    // but MATLAB code for converting from one to the other exists
    // Contact Bryan W. Stiles 
    //  
    if (! _littleEndian ){
      if (! read_hex(fp, (char *)(*(_scaleArray + AMPLITUDE_INDEX) + 1),
		     sizeof(float)) ||
	  ! read_hex(fp, (char *)(*(_scaleArray + AMPLITUDE_INDEX) + 0),
		     sizeof(float)) ||
	  ! read_hex(fp, (char *)(*(_scaleArray + PHASE_INDEX) + 1),
		     sizeof(float)) ||
	  ! read_hex(fp, (char *)(*(_scaleArray + PHASE_INDEX) + 0),
		     sizeof(float)) ||
	  ! read_hex(fp, (char *)(*(_scaleArray + BIAS_INDEX) + 1),
		     sizeof(float)) ||
	  ! read_hex(fp, (char *)(*(_scaleArray + BIAS_INDEX) + 0),
		     sizeof(float)))
	{
	  fclose(fp);
	  return(0);
	}
    }
    else {
      if (! read_hex(fp, (char *)(*(_scaleArray + AMPLITUDE_INDEX) + 1)+2,
		      sizeof(short)) ||
	  ! read_hex(fp, (char *)(*(_scaleArray + AMPLITUDE_INDEX) + 1),
		     sizeof(short)) ||
	  ! read_hex(fp, (char *)(*(_scaleArray + AMPLITUDE_INDEX) + 0)+2,
		     sizeof(short)) ||
	  ! read_hex(fp, (char *)(*(_scaleArray + AMPLITUDE_INDEX) + 0),
		     sizeof(short)) ||
	  ! read_hex(fp, (char *)(*(_scaleArray + PHASE_INDEX) + 1)+2,
		     sizeof(short)) ||
	  ! read_hex(fp, (char *)(*(_scaleArray + PHASE_INDEX) + 1),
		     sizeof(short)) ||
	  ! read_hex(fp, (char *)(*(_scaleArray + PHASE_INDEX) + 0)+2,
		     sizeof(short)) ||
	  ! read_hex(fp, (char *)(*(_scaleArray + PHASE_INDEX) + 0),
		     sizeof(short)) ||
	  ! read_hex(fp, (char *)(*(_scaleArray + BIAS_INDEX) + 1)+2,
		     sizeof(short)) ||
	  ! read_hex(fp, (char *)(*(_scaleArray + BIAS_INDEX) + 1),
		     sizeof(short)) ||
	  ! read_hex(fp, (char *)(*(_scaleArray + BIAS_INDEX) + 0)+2,
		     sizeof(short)) ||
	  ! read_hex(fp, (char *)(*(_scaleArray + BIAS_INDEX) + 0),
		     sizeof(short)))
	
	{
	  
	  fclose(fp);
	  return(0);
	}
    }
    //-------//
    // terms //
    //-------//

    unsigned int array_size = 3 * _steps * sizeof(T);
    unsigned char* term_array = (unsigned char *)malloc(array_size);
    unsigned char* swap_term_array = (unsigned char *)malloc(array_size);
    if (term_array == NULL)
    {
        fclose(fp);
        return(0);
    }

    if (! read_hex(fp, (char *)swap_term_array, array_size))
    {
        fclose(fp);
        return(0);
    }
    // because read_hex writes one short at a time, shorts are output
    // correctly regardless of swapping, but characters are not so ..
    for(unsigned int i=0;i<array_size;i++){
      int iswap=2*(i/2)+!(i%2);
      if(_littleEndian && sizeof(T)==1){
	term_array[i]=swap_term_array[iswap];
      }
      else{
	term_array[i]=swap_term_array[i];
      }
    }

    unsigned int bytes = 0;
    for (int term = 0; term < 3; term++)
    {
        for (unsigned int step = 0; step < _steps; step++)
        {
            memcpy((*(_termArray + step) + term), term_array + bytes,
                sizeof(T));
            bytes += sizeof(T);
        }
    }

    free(term_array);
    free(swap_term_array);
    //------------//
    // close file //
    //------------//

    fclose(fp);

    return(1);
}

//-----------------------//
// TrackerBase::WriteHex //
//-----------------------//

template <class T>
int
TrackerBase<T>::WriteHex(
    const char*  filename)
{
    //---------------//
    // open the file //
    //---------------//

    FILE* fp = fopen(filename, "w");
    if (fp == NULL)
        return(0);

    //----------//
    // table id //
    //----------//

    if (! write_hex(fp, (char *)&_tableId, sizeof(unsigned short)))
    {
        fclose(fp);
        return(0);
    }

    //-----------//
    // file size //
    //-----------//

    unsigned short id_size = sizeof(unsigned short);
    unsigned short size_size = sizeof(unsigned short);
    unsigned short spare_size = SPARE_WORDS * sizeof(unsigned short);
    unsigned short dither_size = 2 * sizeof(unsigned short);
    unsigned short terms_size = 3 * _steps * sizeof(T);
    unsigned short scale_size = 6 * sizeof(float);
    unsigned short file_size = id_size + size_size + spare_size +
        dither_size + terms_size + scale_size;
    if (! write_hex(fp, (char *)&file_size, sizeof(unsigned short)))
    {
        fclose(fp);
        return(0);
    }

    //-------//
    // spare //
    //-------//

    unsigned short spare[SPARE_WORDS];
    for (int i = 0; i < SPARE_WORDS; i++)
    {
        spare[i] = 0;
    }
    if (! write_hex(fp, (char *)spare, SPARE_WORDS * sizeof(unsigned short)))
    {
        fclose(fp);
        return(0);
    }

    //--------//
    // dither //
    //--------//

    if (! write_hex(fp, (char *)_dither, 2 * sizeof(unsigned short)))
    {
        fclose(fp);
        return(0);
    }

    //-------//
    // scale //
    //-------//
    // updated to avoid swapping words when little endian version is run
    //--------------------------
    if(! _littleEndian){
      if (! write_hex(fp, (char *)(*(_scaleArray + AMPLITUDE_INDEX) + 1),
		      sizeof(float)) ||
	  ! write_hex(fp, (char *)(*(_scaleArray + AMPLITUDE_INDEX) + 0),
		      sizeof(float)) ||
	  ! write_hex(fp, (char *)(*(_scaleArray + PHASE_INDEX) + 1),
		      sizeof(float)) ||
	  ! write_hex(fp, (char *)(*(_scaleArray + PHASE_INDEX) + 0),
		      sizeof(float)) ||
	  ! write_hex(fp, (char *)(*(_scaleArray + BIAS_INDEX) + 1),
		      sizeof(float)) ||
	  ! write_hex(fp, (char *)(*(_scaleArray + BIAS_INDEX) + 0),
		      sizeof(float)))
	{

	  fclose(fp);
	  return(0);
	}
    }
    else{
      if (! write_hex(fp, (char *)(*(_scaleArray + AMPLITUDE_INDEX) + 1)+2,
		      sizeof(short)) ||
	  ! write_hex(fp, (char *)(*(_scaleArray + AMPLITUDE_INDEX) + 1),
		      sizeof(short)) ||
	  ! write_hex(fp, (char *)(*(_scaleArray + AMPLITUDE_INDEX) + 0)+2,
		      sizeof(short)) ||
	  ! write_hex(fp, (char *)(*(_scaleArray + AMPLITUDE_INDEX) + 0),
		      sizeof(short)) ||
	  ! write_hex(fp, (char *)(*(_scaleArray + PHASE_INDEX) + 1)+2,
		      sizeof(short)) ||
	  ! write_hex(fp, (char *)(*(_scaleArray + PHASE_INDEX) + 1),
		      sizeof(short)) ||
	  ! write_hex(fp, (char *)(*(_scaleArray + PHASE_INDEX) + 0)+2,
		      sizeof(short)) ||
	  ! write_hex(fp, (char *)(*(_scaleArray + PHASE_INDEX) + 0),
		      sizeof(short)) ||
	  ! write_hex(fp, (char *)(*(_scaleArray + BIAS_INDEX) + 1)+2,
		      sizeof(short)) ||
	  ! write_hex(fp, (char *)(*(_scaleArray + BIAS_INDEX) + 1),
		      sizeof(short)) ||
	  ! write_hex(fp, (char *)(*(_scaleArray + BIAS_INDEX) + 0)+2,
		      sizeof(short)) ||
	  ! write_hex(fp, (char *)(*(_scaleArray + BIAS_INDEX) + 0),
		      sizeof(short)))

	{

	  fclose(fp);
	  return(0);
	}
    }
    //-------//
    // terms //
    //-------//

    unsigned int array_size = 3 * _steps * sizeof(T);
    unsigned char* term_array = (unsigned char *)malloc(array_size);
    unsigned char* swap_term_array = (unsigned char *)malloc(array_size);
    if (term_array == NULL)
    {
        fclose(fp);
        return(0);
    }

    unsigned int bytes = 0;
    for (int term = 0; term < 3; term++)
    {
        for (unsigned int step = 0; step < _steps; step++)
        {
            memcpy(swap_term_array + bytes, (*(_termArray + step) + term),
                sizeof(T));
            bytes += sizeof(T);

            
        }
    }

    // because write_hex writes one short at a time, shorts are output
    // correctly regardless of swapping, but characters are not so ..
    for(unsigned int i=0;i<array_size;i++){
      int iswap=2*(i/2)+!(i%2);
      if(_littleEndian && sizeof(T)==1){
	term_array[i]=swap_term_array[iswap];
      }
      else{
	term_array[i]=swap_term_array[i];
      }
    }

  
    if (! write_hex(fp, (char *)term_array, array_size))
    {
        fclose(fp);
        return(0);
    }

    free(term_array);
    free(swap_term_array);
    //------------//
    // close file //
    //------------//

    fclose(fp);

    return(1);
}

//-------------------------//
// TrackerBase::WriteAscii //
//-------------------------//

template <class T>
int
TrackerBase<T>::WriteAscii(
    const char*  filename)
{
    //---------------//
    // open the file //
    //---------------//

    FILE* fp = fopen(filename, "w");
    if (fp == NULL)
        return(0);

    //----------//
    // table id //
    //----------//

    fprintf(fp, "Table ID: %d\n", _tableId);
    fprintf(fp, "Dither: %d %d\n", _dither[0], _dither[1]);
    fprintf(fp, "Scale Factors:\n");
    fprintf(fp, "  Amplitude: m = %g, b = %g\n",
        *(*(_scaleArray + AMPLITUDE_INDEX) + 1),
        *(*(_scaleArray + AMPLITUDE_INDEX) + 0));
    fprintf(fp, "  Phase: m = %g, b = %g\n",
        *(*(_scaleArray + PHASE_INDEX) + 1),
        *(*(_scaleArray + PHASE_INDEX) + 0));
    fprintf(fp, "  Bias: m = %g, b = %g\n",
        *(*(_scaleArray + BIAS_INDEX) + 1),
        *(*(_scaleArray + BIAS_INDEX) + 0));
    fprintf(fp, "Orbit Step  Terms\n");
    for (unsigned int step = 0; step < _steps; step++)
    {
        fprintf(fp, "      %4d", step);
        for (int term = 0; term < 3; term++)
        {
            fprintf(fp, " %6d", *(*(_termArray + step) + term));
        }
        fprintf(fp, "\n");
    }

    //------------//
    // close file //
    //------------//

    fclose(fp);

    return(1);
}

//-------------------------//
// TrackerBase::SetFromMro //
//-------------------------//

template <class T>
int
TrackerBase<T>::SetFromMro(
    char*  mro)
{
    char* ptr = mro;

    memcpy(&_tableId, ptr, 2);
    ptr += 2;

    unsigned short filesize;
    memcpy(&filesize, ptr, 2);
    ptr += 2;

    unsigned short spare[SPARE_WORDS];
    memcpy(spare, ptr, 2 * SPARE_WORDS);
    ptr += 2 * SPARE_WORDS;

    memcpy(_dither, ptr, 4);
    ptr += 4;

    memcpy(*(_scaleArray + AMPLITUDE_INDEX) + 1, ptr, 4);
    ptr += 4;
    memcpy(*(_scaleArray + AMPLITUDE_INDEX) + 0, ptr, 4);
    ptr += 4;
    memcpy(*(_scaleArray + PHASE_INDEX) + 1, ptr, 4);
    ptr += 4;
    memcpy(*(_scaleArray + PHASE_INDEX) + 0, ptr, 4);
    ptr += 4;
    memcpy(*(_scaleArray + BIAS_INDEX) + 1, ptr, 4);
    ptr += 4;
    memcpy(*(_scaleArray + BIAS_INDEX) + 0, ptr, 4);
    ptr += 4;

    for (int term = 0; term < 3; term++)
    {
        for (unsigned int step = 0; step < _steps; step++)
        {
            memcpy(*(_termArray + step) + term, ptr, sizeof(T));
            ptr += sizeof(T);
        }
    }

    return(1);
}

#endif
