//==============================================================//
// Copyright (C) 1998-1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_qtracking_c[] =
    "@(#) $Id$";

#include <math.h>
#include <malloc.h>
#include "Tracking.h"
#include "Constants.h"
#include "Array.h"

#define COS_TABLE_SIZE         256
#define COS_TABLE_SIZE_MOD(A)  ((A) & 0xFF)

//---------------------------------------------------------------//
// the integer representation of the floating point cosine table //
//---------------------------------------------------------------//

int int_cos_table[COS_TABLE_SIZE] =
{
    0x3f800000, 0x3f7fec46, 0x3f7fb107, 0x3f7f4e66,
    0x3f7ec472, 0x3f7e132b, 0x3f7d3ab4, 0x3f7c3b2e,
    0x3f7b14ba, 0x3f79c79b, 0x3f7853f4, 0x3f76ba06,
    0x3f74fa05, 0x3f731444, 0x3f710907, 0x3f6ed8a1,
    0x3f6c8366, 0x3f6a09ab, 0x3f676bd3, 0x3f64aa54,
    0x3f61c593, 0x3f5ebe06, 0x3f5b9421, 0x3f58485a,
    0x3f54db38, 0x3f514d40, 0x3f4d9ef9, 0x3f49d10b,
    0x3f45e3fc, 0x3f41d873, 0x3f3daef7, 0x3f396840,
    0x3f3504f7, 0x3f3085b2, 0x3f2beb4a, 0x3f273659,
    0x3f226794, 0x3f1d7fc7, 0x3f187fbb, 0x3f136827,
    0x3f0e39d6, 0x3f08f590, 0x3f039c41, 0x3efc5d20,
    0x3ef15af3, 0x3ee6336a, 0x3edae87d, 0x3ecf7bc0,
    0x3ec3ef28, 0x3eb84428, 0x3eac7cd9, 0x3ea09aed,
    0x3e94a03c, 0x3e888e9b, 0x3e78d003, 0x3e605c46,
    0x3e47c5ef, 0x3e2f10aa, 0x3e1640a7, 0x3dfab29e,
    0x3dc8bda9, 0x3d96a93f, 0x3d48fb87, 0x3cc90c4e,
    0x358637bd, 0xbcc9081c, 0xbd48fa7b, 0xbd96a8b9,
    0xbdc8bc9d, 0xbdfab218, 0xbe164021, 0xbe2f1066,
    0xbe47c569, 0xbe605bc0, 0xbe78cf7d, 0xbe888e58,
    0xbe949ff9, 0xbea09aaa, 0xbeac7c95, 0xbeb84407,
    0xbec3eee5, 0xbecf7b9e, 0xbedae85c, 0xbee63348,
    0xbef15aaf, 0xbefc5cff, 0xbf039c30, 0xbf08f590,
    0xbf0e39c5, 0xbf136816, 0xbf187faa, 0xbf1d7fc7,
    0xbf226784, 0xbf273648, 0xbf2beb3a, 0xbf3085b2,
    0xbf3504e6, 0xbf396840, 0xbf3daef7, 0xbf41d873,
    0xbf45e3fc, 0xbf49d10b, 0xbf4d9ef9, 0xbf514d40,
    0xbf54db38, 0xbf58485a, 0xbf5b9421, 0xbf5ebe06,
    0xbf61c593, 0xbf64aa54, 0xbf676bd3, 0xbf6a09ab,
    0xbf6c8366, 0xbf6ed8a1, 0xbf710907, 0xbf731444,
    0xbf74fa16, 0xbf76ba06, 0xbf7853f4, 0xbf79c79b,
    0xbf7b14ba, 0xbf7c3b2e, 0xbf7d3ab4, 0xbf7e132b,
    0xbf7ec472, 0xbf7f4e77, 0xbf7fb118, 0xbf7fec46,
    0xbf800000, 0xbf7fec46, 0xbf7fb107, 0xbf7f4e66,
    0xbf7ec472, 0xbf7e131b, 0xbf7d3aa3, 0xbf7c3b1d,
    0xbf7b14ba, 0xbf79c79b, 0xbf7853f4, 0xbf76b9f5,
    0xbf74fa05, 0xbf731434, 0xbf7108f6, 0xbf6ed890,
    0xbf6c8356, 0xbf6a099a, 0xbf676bc2, 0xbf64aa43,
    0xbf61c582, 0xbf5ebdf5, 0xbf5b93ff, 0xbf584838,
    0xbf54db16, 0xbf514d1f, 0xbf4d9ee9, 0xbf49d0ea,
    0xbf45e3da, 0xbf41d851, 0xbf3daed5, 0xbf39681f,
    0xbf3504c5, 0xbf308590, 0xbf2beb18, 0xbf273626,
    0xbf226762, 0xbf1d7fa6, 0xbf187f88, 0xbf1367f5,
    0xbf0e39a4, 0xbf08f55e, 0xbf039bfe, 0xbefc5c9a,
    0xbef15a6c, 0xbee632e4, 0xbedae7f7, 0xbecf7b39,
    0xbec3ee80, 0xbeb843a2, 0xbeac7c31, 0xbea09a46,
    0xbe949f95, 0xbe888df3, 0xbe78ce70, 0xbe605ab4,
    0xbe47c45d, 0xbe2f0f5a, 0xbe163f14, 0xbdfaaf79,
    0xbdc8ba84, 0xbd96a61a, 0xbd48f53c, 0xbcc8ffb9,
    0x36c9539c, 0x3cc916ca, 0x3d4901d2, 0x3d96abde,
    0x3dc8c048, 0x3dfab5c4, 0x3e164239, 0x3e2f123c,
    0x3e47c73f, 0x3e605d96, 0x3e78d152, 0x3e888f65,
    0x3e94a106, 0x3ea09bb7, 0x3eac7da2, 0x3eb844f2,
    0x3ec3eff2, 0x3ecf7caa, 0x3edae947, 0x3ee63455,
    0x3ef15bbc, 0x3efc5dea, 0x3f039ca6, 0x3f08f606,
    0x3f0e3a3b, 0x3f13688c, 0x3f18801f, 0x3f1d803d,
    0x3f2267f9, 0x3f2736bd, 0x3f2bebaf, 0x3f308616,
    0x3f35054b, 0x3f3968a5, 0x3f3daf5c, 0x3f41d8c7,
    0x3f45e460, 0x3f49d170, 0x3f4d9f5e, 0x3f514d94,
    0x3f54db7b, 0x3f58489d, 0x3f5b9464, 0x3f5ebe49,
    0x3f61c5d6, 0x3f64aa97, 0x3f676c16, 0x3f6a09dd,
    0x3f6c8399, 0x3f6ed8d3, 0x3f710939, 0x3f731477,
    0x3f74fa37, 0x3f76ba38, 0x3f785426, 0x3f79c7bd,
    0x3f7b14db, 0x3f7c3b3f, 0x3f7d3ac5, 0x3f7e133c,
    0x3f7ec482, 0x3f7f4e77, 0x3f7fb118, 0x3f7fec46
};

float* cos_table = (float *)int_cos_table;

//=============//
// TrackerBase //
//=============//

template <class T>
TrackerBase<T>::TrackerBase()
:   _tableId(0), _scaleArray(NULL), _termArray(NULL), _steps(256)
{
    for (int i = 0; i < 2; i++)
        _dither[i] = 0;

    return;
}

template <class T>
TrackerBase<T>::~TrackerBase()
{
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

    //-------//
    // terms //
    //-------//

    unsigned int array_size = 3 * _steps * sizeof(T);
    unsigned char* term_array = (unsigned char *)malloc(array_size);
    if (term_array == NULL)
    {
        fclose(fp);
        return(0);
    }

    if (! read_hex(fp, (char *)term_array, array_size))
    {
        fclose(fp);
        return(0);
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

    //-------//
    // terms //
    //-------//

    unsigned int array_size = 3 * _steps * sizeof(T);
    unsigned char* term_array = (unsigned char *)malloc(array_size);
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
            memcpy(term_array + bytes, (*(_termArray + step) + term),
                sizeof(T));
            bytes += sizeof(T);
        }
    }

    if (! write_hex(fp, (char *)term_array, array_size))
    {
        fclose(fp);
        return(0);
    }

    free(term_array);

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

//==============//
// RangeTracker //
//==============//

RangeTracker::RangeTracker()
:   rxRangeMem(0.0)
{
    return;
}

RangeTracker::~RangeTracker()
{
    return;
}

//----------------------//
// RangeTracker::ReadGS //
//----------------------//

int
RangeTracker::ReadGS(
    const char*    filename,
    RangeTracker*  second_set)
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

    _steps = DEFAULT_STEPS;

    //----------//
    // allocate //
    //----------//

    if (! Allocate(_steps) || ! second_set->Allocate(_steps))
    {
        fclose(fp);
        return(0);
    }

    //-------------------------------//
    // dummy for fortran unformatted //
    //-------------------------------//

    int dummy;
    if (fread((void *)&dummy, sizeof(int), 1, fp) != 1)
    {
        fclose(fp);
        return(0);
    }

    //---------------//
    // scale factors //
    //---------------//

    if (fread((void *) (*(_scaleArray + BIAS_INDEX) + 1),
        sizeof(float), 1, fp) != 1 ||
        fread((void *) (*(second_set->_scaleArray + BIAS_INDEX) + 1),
            sizeof(float), 1, fp) != 1 ||

        fread((void *) (*(_scaleArray + BIAS_INDEX) + 0), sizeof(float), 1,
            fp) != 1 ||
        fread((void *) (*(second_set->_scaleArray + BIAS_INDEX) + 0),
            sizeof(float), 1, fp) != 1 ||

        fread((void *) (*(_scaleArray + AMPLITUDE_INDEX) + 1), sizeof(float), 1,
            fp) != 1 ||
        fread((void *) (*(second_set->_scaleArray + AMPLITUDE_INDEX) + 1),
            sizeof(float), 1, fp) != 1 ||

        fread((void *) (*(_scaleArray + AMPLITUDE_INDEX) + 0), sizeof(float), 1,
            fp) != 1 ||
        fread((void *) (*(second_set->_scaleArray + AMPLITUDE_INDEX) + 0),
            sizeof(float), 1, fp) != 1 ||

        fread((void *) (*(_scaleArray + PHASE_INDEX) + 1), sizeof(float), 1,
            fp) != 1 ||
        fread((void *) (*(second_set->_scaleArray + PHASE_INDEX) + 1),
            sizeof(float), 1, fp) != 1 ||

        fread((void *) (*(_scaleArray + PHASE_INDEX) + 0), sizeof(float), 1,
            fp) != 1 ||
        fread((void *) (*(second_set->_scaleArray + PHASE_INDEX) + 0),
            sizeof(float), 1, fp) != 1)
    {
        fclose(fp);
        return(0);
    }

    //------------------------------------------------//
    // the ground system used 2 bytes for range terms //
    //------------------------------------------------//

    unsigned short tmp_array_1[DEFAULT_STEPS];
    unsigned short tmp_array_2[DEFAULT_STEPS];

    //-------//
    // terms //
    //-------//

    unsigned int term[3] = { AMPLITUDE_INDEX, PHASE_INDEX, BIAS_INDEX };
    for (int term_idx = 0; term_idx < 3; term_idx++)
    {
        if (fread((void *)tmp_array_1, _steps * 2, 1, fp) != 1 ||
            fread((void *)tmp_array_2, _steps * 2, 1, fp) != 1)
        {
            fclose(fp);
            return(0);
        }
        for (unsigned int step = 0; step < _steps; step++)
        {
            *(*(_termArray + step) + term[term_idx]) =
                (unsigned char)tmp_array_1[step];
            *(*(second_set->_termArray + step) + term[term_idx]) =
                (unsigned char)tmp_array_2[step];
        }
    }

    //----------------//
    // close the file //
    //----------------//

    fclose(fp);

    return(1);
}

//-----------------------//
// RangeTracker::WriteGS //
//-----------------------//

int
RangeTracker::WriteGS(
    const char*    filename,
    RangeTracker*  second_set)
{
    //---------------//
    // open the file //
    //---------------//

    FILE* fp = fopen(filename, "w");
    if (fp == NULL)
        return(0);

    //-------------------------------//
    // dummy for fortran unformatted //
    //-------------------------------//

    int dummy = 3120;
    if (fwrite((void *)&dummy, sizeof(int), 1, fp) != 1)
    {
        fclose(fp);
        return(0);
    }

    //---------------//
    // scale factors //
    //---------------//

    if (fwrite((void *) (*(_scaleArray + BIAS_INDEX) + 1),
        sizeof(float), 1, fp) != 1 ||
        fwrite((void *) (*(second_set->_scaleArray + BIAS_INDEX) + 1),
            sizeof(float), 1, fp) != 1 ||

        fwrite((void *) (*(_scaleArray + BIAS_INDEX) + 0), sizeof(float), 1,
            fp) != 1 ||
        fwrite((void *) (*(second_set->_scaleArray + BIAS_INDEX) + 0),
            sizeof(float), 1, fp) != 1 ||

        fwrite((void *) (*(_scaleArray + AMPLITUDE_INDEX) + 1), sizeof(float),
            1, fp) != 1 ||
        fwrite((void *) (*(second_set->_scaleArray + AMPLITUDE_INDEX) + 1),
            sizeof(float), 1, fp) != 1 ||

        fwrite((void *) (*(_scaleArray + AMPLITUDE_INDEX) + 0), sizeof(float),
            1, fp) != 1 ||
        fwrite((void *) (*(second_set->_scaleArray + AMPLITUDE_INDEX) + 0),
            sizeof(float), 1, fp) != 1 ||

        fwrite((void *) (*(_scaleArray + PHASE_INDEX) + 1), sizeof(float), 1,
            fp) != 1 ||
        fwrite((void *) (*(second_set->_scaleArray + PHASE_INDEX) + 1),
            sizeof(float), 1, fp) != 1 ||

        fwrite((void *) (*(_scaleArray + PHASE_INDEX) + 0), sizeof(float), 1,
            fp) != 1 ||
        fwrite((void *) (*(second_set->_scaleArray + PHASE_INDEX) + 0),
            sizeof(float), 1, fp) != 1)
    {
        fclose(fp);
        return(0);
    }

    //------------------------------------------------//
    // the ground system used 2 bytes for range terms //
    //------------------------------------------------//

    unsigned short tmp_array_1[DEFAULT_STEPS];
    unsigned short tmp_array_2[DEFAULT_STEPS];

    //-------//
    // terms //
    //-------//

    unsigned int term[3] = { AMPLITUDE_INDEX, PHASE_INDEX, BIAS_INDEX };
    for (int term_idx = 0; term_idx < 3; term_idx++)
    {
        for (unsigned int step = 0; step < _steps; step++)
        {
            tmp_array_1[step] =
                (unsigned short) *(*(_termArray + step) + term[term_idx]);
            tmp_array_2[step] =
                (unsigned short) *(*(second_set->_termArray + step) +
                term[term_idx]);
        }
        if (fwrite((void *)tmp_array_1, _steps * 2, 1, fp) != 1 ||
            fwrite((void *)tmp_array_2, _steps * 2, 1, fp) != 1)
        {
            fclose(fp);
            return(0);
        }
    }

    //-------------------------------//
    // dummy for fortran unformatted //
    //-------------------------------//

    if (fwrite((void *)&dummy, sizeof(int), 1, fp) != 1)
    {
        fclose(fp);
        return(0);
    }

    //----------------//
    // close the file //
    //----------------//

    fclose(fp);

    return(1);
}

//---------------------------//
// RangeTracker::MroAssemble //
//---------------------------//
// For assemling a tracking table from the memory readout data

#define RGC_MRO_ARRAY_SIZE  804
#define FINAL_RGC_OFFSET    800

int
RangeTracker::MroAssemble(
    unsigned char   type,
    unsigned short  offset,
    char*           data,
    int*            beam_idx,
    int*            active_idx)
{
    //------------//
    // initialize //
    //------------//

    static int beam = 0;    // 0 = A, 1 = B
    static int active = 0;    // 0 = inactive, 1 = active
    static unsigned short expected_offset = 0;
    static char mro[RGC_MRO_ARRAY_SIZE];

    //------------------//
    // check table type //
    //------------------//

    switch (type)
    {
    case 0x0D:
        // Active Range Tracking Table for Beam A
        beam = 0;
        active = 1;
        break;
    case 0x0E:
        // Active Range Tracking Table for Beam B
        beam = 1;
        active = 1;
        break;
    case 0x1D:
        // Inactive Range Tracking Table for Beam A
        beam = 0;
        active = 0;
        break;
    case 0x1E:
        // Inactive Range Tracking Table for Beam B
        beam = 1;
        active = 0;
        break;
    default:
        // some other table
        return(0);
        break;
    }

    //--------------//
    // read in data //
    //--------------//

    if (offset != expected_offset)
    {
        expected_offset = 0;    // reset
        return(0);
    }
    memcpy(mro + offset, data, 4);
    expected_offset += 4;
    if (offset == FINAL_RGC_OFFSET)
    {
        // convert array to table
        SetFromMro(mro);
        expected_offset = 0;    // reset
        *beam_idx = beam;
        *active_idx = active;
        return(1);
    }
    return(0);
}


//-----------//
// operator= //
//-----------//

RangeTracker&
RangeTracker::operator=(
    const RangeTracker&  from)
{
    rxRangeMem=from.rxRangeMem;
    if (_scaleArray!=NULL)
        free_array((void*)_scaleArray,2,3,2);
    if (_termArray!=NULL)
        free_array((void*)_termArray,2,_steps,3);
    _tableId=from._tableId;
    _steps=from._steps;
    _dither[0]=from._dither[0];
    _dither[1]=from._dither[1];
    if (from._scaleArray==NULL)
        _scaleArray=NULL;
    else
    {
        _scaleArray=(float**)make_array(sizeof(float),2,3,2);
        for (int i=0;i<3;i++)
        {
            for (int j=0;j<2;j++)
            {
                _scaleArray[i][j]=from._scaleArray[i][j];
            }
        }
    }
    if (from._termArray==NULL)
        _termArray=NULL;
    else
    {
        _termArray = (unsigned char**)make_array(sizeof(unsigned char), 2,
            _steps, 3);
        for (unsigned int i=0;i<_steps;i++)
        {
            for (int j=0;j<3;j++)
            {
                _termArray[i][j]=from._termArray[i][j];
            }
        }
    }
    return(*this);
}

//------------------------------//
// RangeTracker::GetRxGateDelay //
//------------------------------//

#define RANGE_GATE_NORMALIZER  0.049903
#define TRACKING_PI            3.141592654

int
RangeTracker::GetRxGateDelay(
    unsigned short  range_step,
    unsigned short  azimuth_step,
    unsigned char   rx_gate_width_dn,
    unsigned char   tx_pulse_width_dn,
    unsigned char*  rx_gate_delay_dn,
    float*          rx_gate_delay_fdn)
{
    unsigned char* char_ptr = *(_termArray + range_step);

    unsigned char a_dn = *(char_ptr + AMPLITUDE_INDEX);
    unsigned char c_dn = *(char_ptr + BIAS_INDEX);
    unsigned char p_dn = *(char_ptr + PHASE_INDEX);

    float ab = *(*(_scaleArray + AMPLITUDE_INDEX) + 0);
    float am = *(*(_scaleArray + AMPLITUDE_INDEX) + 1);
    float cb = *(*(_scaleArray + BIAS_INDEX) + 0);
    float cm = *(*(_scaleArray + BIAS_INDEX) + 1);
    float pb = *(*(_scaleArray + PHASE_INDEX) + 0);
    float pm = *(*(_scaleArray + PHASE_INDEX) + 1);

    float A = am * (float)a_dn + ab;
    float C = cm * (float)c_dn + cb;
    float P = pm * (float)p_dn + pb;

    float ttf = (2.0 * TRACKING_PI * (float)azimuth_step) / 32768.0 + P;
    ttf = fabs(ttf);
    ttf *= (256.0 / (2.0 * TRACKING_PI));

    unsigned short tindex = (unsigned short)ttf;

    float cos1 = cos_table[COS_TABLE_SIZE_MOD(tindex)];
    float cos2 = cos_table[COS_TABLE_SIZE_MOD(tindex + 1)];

    float range_predict = C + A * ((ttf - (long)ttf) * (cos2 - cos1) + cos1);
    float rx_range_mem = range_predict / RANGE_GATE_NORMALIZER;
    float rx_gate_width_fdn = (float)rx_gate_width_dn;
    float tx_pulse_width_fdn = (float)tx_pulse_width_dn;
    *rx_gate_delay_fdn = rx_range_mem -
        (rx_gate_width_fdn - tx_pulse_width_fdn) / 2.0;
    *rx_gate_delay_dn = (unsigned char)(*rx_gate_delay_fdn + 0.5);

    //------------------------------//
    // remember the rx range number //
    //------------------------------//

    rxRangeMem = rx_range_mem;

    return(1);
}

//--------------------------------//
// RangeTracker::SetRoundTripTime //
//--------------------------------//

int
RangeTracker::SetRoundTripTime(
    double**  terms)
{
    double mins[3];
    double maxs[3];

    //-----------------------------------------//
    // calculate the minimum and maximum terms //
    //-----------------------------------------//

    for (int term_idx = 0; term_idx < 3; term_idx++)
    {
        mins[term_idx] = *(*(terms + 0) + term_idx);
        maxs[term_idx] = mins[term_idx];
    }

    for (unsigned int range_step = 0; range_step < _steps;
        range_step++)
    {
        for (int term_idx = 0; term_idx < 3; term_idx++)
        {
            if (*(*(terms + range_step) + term_idx) < mins[term_idx])
                mins[term_idx] = *(*(terms + range_step) + term_idx);

            if (*(*(terms + range_step) + term_idx) > maxs[term_idx])
                maxs[term_idx] = *(*(terms + range_step) + term_idx);
        }
    }

    //------------------------//
    // generate scale factors //
    //------------------------//

    for (int term_idx = 0; term_idx < 3; term_idx++)
    {
        *(*(_scaleArray + term_idx) + 0) = mins[term_idx];

        *(*(_scaleArray + term_idx) + 1) = (maxs[term_idx] - mins[term_idx]) /
            255.0;
    }

    //------------------------//
    // calculate scaled terms //
    //------------------------//

    for (unsigned int range_step = 0; range_step < _steps;
        range_step++)
    {
        for (int term_idx = 0; term_idx < 3; term_idx++)
        {
            *(*(_termArray + range_step) + term_idx) =
                (unsigned char)(( *(*(terms + range_step) +
                term_idx) - *(*(_scaleArray + term_idx) + 0)) /
                *(*(_scaleArray + term_idx) + 1) + 0.5);
        }
    }

    return(1);
}

//================//
// DopplerTracker //
//================//

DopplerTracker::DopplerTracker()
:   tableFrequency(0.0)
{
    return;
}

DopplerTracker::~DopplerTracker()
{
    return;
}

//------------------------//
// DopplerTracker::ReadGS //
//------------------------//

int
DopplerTracker::ReadGS(
    const char*      filename,
    DopplerTracker*  second_set)
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

    _steps = DEFAULT_STEPS;

    //----------//
    // allocate //
    //----------//

    if (! Allocate(_steps) || ! second_set->Allocate(_steps))
    {
        fclose(fp);
        return(0);
    }

    //-------------------------------//
    // dummy for fortran unformatted //
    //-------------------------------//

    int dummy;
    if (fread((void *)&dummy, sizeof(int), 1, fp) != 1)
    {
        fclose(fp);
        return(0);
    }

    //---------------//
    // scale factors //
    //---------------//

    if (fread((void *) (*(_scaleArray + BIAS_INDEX) + 1),
        sizeof(float), 1, fp) != 1 ||
        fread((void *) (*(second_set->_scaleArray + BIAS_INDEX) + 1),
            sizeof(float), 1, fp) != 1 ||

        fread((void *) (*(_scaleArray + BIAS_INDEX) + 0), sizeof(float), 1,
            fp) != 1 ||
        fread((void *) (*(second_set->_scaleArray + BIAS_INDEX) + 0),
            sizeof(float), 1, fp) != 1 ||

        fread((void *) (*(_scaleArray + AMPLITUDE_INDEX) + 1), sizeof(float), 1,
            fp) != 1 ||
        fread((void *) (*(second_set->_scaleArray + AMPLITUDE_INDEX) + 1),
            sizeof(float), 1, fp) != 1 ||

        fread((void *) (*(_scaleArray + AMPLITUDE_INDEX) + 0), sizeof(float), 1,
            fp) != 1 ||
        fread((void *) (*(second_set->_scaleArray + AMPLITUDE_INDEX) + 0),
            sizeof(float), 1, fp) != 1 ||

        fread((void *) (*(_scaleArray + PHASE_INDEX) + 1), sizeof(float), 1,
            fp) != 1 ||
        fread((void *) (*(second_set->_scaleArray + PHASE_INDEX) + 1),
            sizeof(float), 1, fp) != 1 ||

        fread((void *) (*(_scaleArray + PHASE_INDEX) + 0), sizeof(float), 1,
            fp) != 1 ||
        fread((void *) (*(second_set->_scaleArray + PHASE_INDEX) + 0),
            sizeof(float), 1, fp) != 1)
    {
        fclose(fp);
        return(0);
    }

    //--------------------------------------------------//
    // the ground system uses 4 bytes for Doppler terms //
    //--------------------------------------------------//

    unsigned int tmp_array_1[DEFAULT_STEPS];
    unsigned int tmp_array_2[DEFAULT_STEPS];

    //-------//
    // terms //
    //-------//

    unsigned int term[3] = { AMPLITUDE_INDEX, PHASE_INDEX, BIAS_INDEX };
    for (int term_idx = 0; term_idx < 3; term_idx++)
    {
        if (fread((void *)tmp_array_1, _steps * 4, 1, fp) != 1 ||
            fread((void *)tmp_array_2, _steps * 4, 1, fp) != 1)
        {
            fclose(fp);
            return(0);
        }
        for (unsigned int step = 0; step < _steps; step++)
        {
            *(*(_termArray + step) + term[term_idx]) =
                (unsigned short)tmp_array_1[step];
            *(*(second_set->_termArray + step) + term[term_idx]) =
                (unsigned short)tmp_array_2[step];
        }
    }

    //----------------//
    // close the file //
    //----------------//

    fclose(fp);

    return(1);
}

//-------------------------//
// DopplerTracker::WriteGS //
//-------------------------//

int
DopplerTracker::WriteGS(
    const char*      filename,
    DopplerTracker*  second_set)
{
    //---------------//
    // open the file //
    //---------------//

    FILE* fp = fopen(filename, "w");
    if (fp == NULL)
        return(0);

    //-------------------------------//
    // dummy for fortran unformatted //
    //-------------------------------//

    int dummy = 6192;
    if (fwrite((void *)&dummy, sizeof(int), 1, fp) != 1)
    {
        fclose(fp);
        return(0);
    }

    //---------------//
    // scale factors //
    //---------------//

    if (fwrite((void *) (*(_scaleArray + BIAS_INDEX) + 1),
        sizeof(float), 1, fp) != 1 ||
        fwrite((void *) (*(second_set->_scaleArray + BIAS_INDEX) + 1),
            sizeof(float), 1, fp) != 1 ||

        fwrite((void *) (*(_scaleArray + BIAS_INDEX) + 0), sizeof(float), 1,
            fp) != 1 ||
        fwrite((void *) (*(second_set->_scaleArray + BIAS_INDEX) + 0),
            sizeof(float), 1, fp) != 1 ||

        fwrite((void *) (*(_scaleArray + AMPLITUDE_INDEX) + 1), sizeof(float),
            1, fp) != 1 ||
        fwrite((void *) (*(second_set->_scaleArray + AMPLITUDE_INDEX) + 1),
            sizeof(float), 1, fp) != 1 ||

        fwrite((void *) (*(_scaleArray + AMPLITUDE_INDEX) + 0), sizeof(float),
            1, fp) != 1 ||
        fwrite((void *) (*(second_set->_scaleArray + AMPLITUDE_INDEX) + 0),
            sizeof(float), 1, fp) != 1 ||

        fwrite((void *) (*(_scaleArray + PHASE_INDEX) + 1), sizeof(float), 1,
            fp) != 1 ||
        fwrite((void *) (*(second_set->_scaleArray + PHASE_INDEX) + 1),
            sizeof(float), 1, fp) != 1 ||

        fwrite((void *) (*(_scaleArray + PHASE_INDEX) + 0), sizeof(float), 1,
            fp) != 1 ||
        fwrite((void *) (*(second_set->_scaleArray + PHASE_INDEX) + 0),
            sizeof(float), 1, fp) != 1)
    {
        fclose(fp);
        return(0);
    }

    //--------------------------------------------------//
    // the ground system used 4 bytes for Doppler terms //
    //--------------------------------------------------//

    unsigned int tmp_array_1[DEFAULT_STEPS];
    unsigned int tmp_array_2[DEFAULT_STEPS];

    //-------//
    // terms //
    //-------//

    unsigned int term[3] = { AMPLITUDE_INDEX, PHASE_INDEX, BIAS_INDEX };
    for (int term_idx = 0; term_idx < 3; term_idx++)
    {
        for (unsigned int step = 0; step < _steps; step++)
        {
            tmp_array_1[step] =
                (unsigned int) *(*(_termArray + step) + term[term_idx]);
            tmp_array_2[step] =
                (unsigned int) *(*(second_set->_termArray + step) +
                term[term_idx]);
        }
        if (fwrite((void *)tmp_array_1, _steps * 4, 1, fp) != 1 ||
            fwrite((void *)tmp_array_2, _steps * 4, 1, fp) != 1)
        {
            fclose(fp);
            return(0);
        }
    }

    //-------------------------------//
    // dummy for fortran unformatted //
    //-------------------------------//

    if (fwrite((void *)&dummy, sizeof(int), 1, fp) != 1)
    {
        fclose(fp);
        return(0);
    }

    //----------------//
    // close the file //
    //----------------//

    fclose(fp);

    return(1);
}

//-----------------------------//
// DopplerTracker::MroAssemble //
//-----------------------------//
// For assemling a tracking table from the memory readout data

#define DTC_MRO_ARRAY_SIZE  1600
#define FINAL_DTC_OFFSET    1568

int
DopplerTracker::MroAssemble(
    unsigned char   type,
    unsigned short  offset,
    char*           data,
    int*            beam_idx,
    int*            active_idx)
{
    //------------//
    // initialize //
    //------------//

    static int beam = 0;    // 0 = A, 1 = B
    static int active = 0;    // 0 = inactive, 1 = active
    static unsigned short expected_offset = 0;
    static char mro[DTC_MRO_ARRAY_SIZE];

    //------------------//
    // check table type //
    //------------------//

    switch (type)
    {
    case 0x0B:
        // Active Doppler Tracking Table for Beam A
        beam = 0;
        active = 1;
        break;
    case 0x0C:
        // Active Doppler Tracking Table for Beam B
        beam = 1;
        active = 1;
        break;
    case 0x1B:
        // Inactive Doppler Tracking Table for Beam A
        beam = 0;
        active = 0;
        break;
    case 0x1C:
        // Inactive Doppler Tracking Table for Beam B
        beam = 1;
        active = 0;
        break;
    default:
        // some other table
        return(0);
        break;
    }

    //--------------//
    // read in data //
    //--------------//

    if (offset != expected_offset)
    {
        expected_offset = 0;    // reset
        return(0);
    }
    memcpy(mro + offset, data, 4);
    expected_offset += 4;
    if (offset == FINAL_DTC_OFFSET)
    {
        // convert array to table
        SetFromMro(mro);
        expected_offset = 0;    // reset
        *beam_idx = beam;
        *active_idx = active;
        return(1);
    }
    return(0);
}

//-----------//
// operator= //
//-----------//

DopplerTracker&
DopplerTracker::operator=(
    const DopplerTracker&  from)
{
    if (_scaleArray != NULL)
        free_array((void*)_scaleArray, 2, 3, 2);
    if (_termArray!=NULL)
        free_array((void*)_termArray, 2, _steps, 3);
    _tableId = from._tableId;
    _steps = from._steps;
    _dither[0] = from._dither[0];
    _dither[1] = from._dither[1];
    if (from._scaleArray == NULL)
        _scaleArray=NULL;
    else
    {
        _scaleArray=(float**)make_array(sizeof(float),2,3,2);
        for (int i=0;i<3;i++)
        {
            for (int j=0;j<2;j++)
            {
                _scaleArray[i][j]=from._scaleArray[i][j];
            }
        }
    }
    if (from._termArray==NULL)
        _termArray=NULL;
    else
    {
        _termArray = (unsigned short**)make_array(sizeof(unsigned short), 2,
            _steps, 3);
        for (unsigned int i=0;i<_steps;i++){
              for (int j=0;j<3;j++){
                _termArray[i][j]=from._termArray[i][j];
            }
        }
    }
    return(*this);
}

//-------------------------------------//
// DopplerTracker::GetCommandedDoppler //
//-------------------------------------//

#define MU          250.73
#define HZ_PER_KHZ  1000

int
DopplerTracker::GetCommandedDoppler(
    unsigned short  doppler_step,
    unsigned short  azimuth_step,
    unsigned char   rx_gate_delay_dn,
    float           rx_gate_delay_fdn,
    short*          commanded_doppler_dn)
{
    unsigned short* short_ptr = *(_termArray + doppler_step);

    unsigned short a_dn = *(short_ptr + AMPLITUDE_INDEX);
    unsigned short c_dn = *(short_ptr + BIAS_INDEX);
    unsigned short p_dn = *(short_ptr + PHASE_INDEX);

    float ab = *(*(_scaleArray + AMPLITUDE_INDEX) + 0);
    float am = *(*(_scaleArray + AMPLITUDE_INDEX) + 1);
    float cb = *(*(_scaleArray + BIAS_INDEX) + 0);
    float cm = *(*(_scaleArray + BIAS_INDEX) + 1);
    float pb = *(*(_scaleArray + PHASE_INDEX) + 0);
    float pm = *(*(_scaleArray + PHASE_INDEX) + 1);

    float A = am * (float)a_dn + ab;
    float C = cm * (float)c_dn + cb;
    float P = pm * (float)p_dn + pb;

    float ttf = (2.0 * TRACKING_PI * (float)azimuth_step) / 32768.0 + P;
    ttf = fabs(ttf);
    ttf *= (256.0 / (2.0 * TRACKING_PI));

    unsigned short tindex = (unsigned short)ttf;

    float cos1 = cos_table[COS_TABLE_SIZE_MOD(tindex)];
    float cos2 = cos_table[COS_TABLE_SIZE_MOD(tindex + 1)];

    tableFrequency = C + A * ((ttf - (long)ttf) * (cos2 - cos1) + cos1);

    float rx_gate_error = (rx_gate_delay_fdn - (float)rx_gate_delay_dn) *
        RANGE_GATE_NORMALIZER * MU * HZ_PER_KHZ;
    float cmd_doppler_fdn = (tableFrequency + rx_gate_error) / 2000.0;

    short cmd_doppler_dn;
    if (cmd_doppler_fdn > 0.0)
        cmd_doppler_dn = (short)(cmd_doppler_fdn + 0.5);
    else
        cmd_doppler_dn = (short)(cmd_doppler_fdn - 0.5);

    *commanded_doppler_dn = -1 * cmd_doppler_dn;
    return(1);
}

//--------------------------//
// DopplerTracker::SetTerms //
//--------------------------//

int
DopplerTracker::SetTerms(
    double**    terms)
{
    double mins[3];
    double maxs[3];

    //-----------------------------------------//
    // calculate the minimum and maximum terms //
    //-----------------------------------------//

    for (int term_idx = 0; term_idx < 3; term_idx++)
    {
        mins[term_idx] = *(*(terms + 0) + term_idx);
        maxs[term_idx] = mins[term_idx];
    }

    for (unsigned int orbit_step = 0; orbit_step < _steps; orbit_step++)
    {
        for (int term_idx = 0; term_idx < 3; term_idx++)
        {
            double value = *(*(terms + orbit_step) + term_idx);

            if (value < mins[term_idx])
                mins[term_idx] = value;

            if (value > maxs[term_idx])
                maxs[term_idx] = value;
        }
    }

    //------------------------//
    // generate scale factors //
    //------------------------//

    for (int term_idx = 0; term_idx < 3; term_idx++)
    {
        *(*(_scaleArray + term_idx) + 0) = mins[term_idx];

        *(*(_scaleArray + term_idx) + 1) = (maxs[term_idx] - mins[term_idx]) /
            65535.0;
    }

    //------------------------//
    // calculate scaled terms //
    //------------------------//

    for (unsigned int orbit_step = 0; orbit_step < _steps; orbit_step++)
    {
        for (int term_idx = 0; term_idx < 3; term_idx++)
        {
            *(*(_termArray + orbit_step) + term_idx) =
                (unsigned short)( (*(*(terms + orbit_step) + term_idx) -
                *(*(_scaleArray + term_idx) + 0)) /
                *(*(_scaleArray + term_idx) + 1) + 0.5);
        }
    }

    return(1);
}

//--------------------------//
// DopplerTracker::GetTerms //
//--------------------------//
// For manipulating the Doppler tracking constants

int
DopplerTracker::GetTerms(
    double**    terms)
{
    float ab = *(*(_scaleArray + AMPLITUDE_INDEX) + 0);
    float am = *(*(_scaleArray + AMPLITUDE_INDEX) + 1);
    float cb = *(*(_scaleArray + BIAS_INDEX) + 0);
    float cm = *(*(_scaleArray + BIAS_INDEX) + 1);
    float pb = *(*(_scaleArray + PHASE_INDEX) + 0);
    float pm = *(*(_scaleArray + PHASE_INDEX) + 1);

    for (unsigned int doppler_step = 0; doppler_step < _steps; doppler_step++)
    {
        unsigned short* short_ptr = *(_termArray + doppler_step);

        unsigned short a_dn = *(short_ptr + AMPLITUDE_INDEX);
        unsigned short c_dn = *(short_ptr + BIAS_INDEX);
        unsigned short p_dn = *(short_ptr + PHASE_INDEX);

        float A = am * (float)a_dn + ab;
        float C = cm * (float)c_dn + cb;
        float P = pm * (float)p_dn + pb;

        *(*(terms + doppler_step) + AMPLITUDE_INDEX) = A;
        *(*(terms + doppler_step) + BIAS_INDEX) = C;
        *(*(terms + doppler_step) + PHASE_INDEX) = P;
    }

    return(1);
}

//==================//
// Helper Functions //
//==================//

//-------------//
// azimuth_fit //
//-------------//

int
azimuth_fit(
    int      count,
    double*  terms,
    double*  a,
    double*  p,
    double*  c)
{
    double wn = two_pi / (double) count;
    double real[2], imag[2];

    for (int i = 0; i < 2; i++)
    {
        real[i] = 0.0;
        imag[i] = 0.0;
        for (int j = 0; j < count; j++)
        {
            double arg = wn * (double)i * (double)j;
            double c = cos(arg);
            double s = sin(arg);
            real[i] += terms[j] * c;
            imag[i] += terms[j] * s;
        }
    }

    *a = 2.0 * sqrt(real[1] * real[1] + imag[1] * imag[1]) / (double)count;
    *p = -atan2(imag[1], real[1]);
    *c = real[0] / (double)count;

    return(1);
}

//-----------//
// write_hex //
//-----------//

int
write_hex(
    FILE*  fp,
    char*  buffer,
    int    bytes)
{
    int words = bytes / 2;
    unsigned short* ptr = (unsigned short *)buffer;
    for (int i = 0; i < words; i++)
    {
        fprintf(fp, "%04hx\n", *(ptr + i));
    }
    return(1);
}

//----------//
// read_hex //
//----------//

int
read_hex(
    FILE*  fp,
    char*  buffer,
    int    bytes)
{
    int words = bytes / 2;
    unsigned short* ptr = (unsigned short *)buffer;
    for (int i = 0; i < words; i++)
    {
        if (fscanf(fp, " %hx", ptr + i) != 1)
            return(0);
    }
    return(1);
}
