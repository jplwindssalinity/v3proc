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

const float cos_table[COS_TABLE_SIZE] =
{
    1.000000, 0.999699, 0.998795, 0.997290, 0.995185, 0.992480, 0.989177,
    0.985278, 0.980785, 0.975702, 0.970031, 0.963776, 0.956940, 0.949528,
    0.941544, 0.932993, 0.923880, 0.914210, 0.903989, 0.893224, 0.881921,
    0.870087, 0.857729, 0.844854, 0.831470, 0.817585, 0.803207, 0.788346,
    0.773010, 0.757209, 0.740951, 0.724247, 0.707107, 0.689540, 0.671559,
    0.653173, 0.634393, 0.615231, 0.595699, 0.575808, 0.555570, 0.534997,
    0.514103, 0.492898, 0.471397, 0.449611, 0.427555, 0.405241, 0.382684,
    0.359895, 0.336890, 0.313682, 0.290285, 0.266713, 0.242981, 0.219102,
    0.195091, 0.170962, 0.146731, 0.122411, 0.098018, 0.073565, 0.049068,
    0.024542, 0.000001, -0.024540, -0.049067, -0.073564, -0.098016,
    -0.122410, -0.146729, -0.170961, -0.195089, -0.219100, -0.242979,
    -0.266711, -0.290283, -0.313680, -0.336888, -0.359894, -0.382682,
    -0.405240, -0.427554, -0.449610, -0.471395, -0.492897, -0.514102,
    -0.534997, -0.555569, -0.575807, -0.595698, -0.615231, -0.634392,
    -0.653172, -0.671558, -0.689540, -0.707106, -0.724247, -0.740951,
    -0.757209, -0.773010, -0.788346, -0.803207, -0.817585, -0.831470,
    -0.844854, -0.857729, -0.870087, -0.881921, -0.893224, -0.903989,
    -0.914210, -0.923880, -0.932993, -0.941544, -0.949528, -0.956941,
    -0.963776, -0.970031, -0.975702, -0.980785, -0.985278, -0.989177,
    -0.992480, -0.995185, -0.997291, -0.998796, -0.999699, -1.000000,
    -0.999699, -0.998795, -0.997290, -0.995185, -0.992479, -0.989176,
    -0.985277, -0.980785, -0.975702, -0.970031, -0.963775, -0.956940,
    -0.949527, -0.941543, -0.932992, -0.923879, -0.914209, -0.903988,
    -0.893223, -0.881920, -0.870086, -0.857727, -0.844852, -0.831468,
    -0.817583, -0.803206, -0.788344, -0.773008, -0.757207, -0.740949,
    -0.724245, -0.707104, -0.689538, -0.671556, -0.653170, -0.634390,
    -0.615229, -0.595696, -0.575805, -0.555567, -0.534994, -0.514099,
    -0.492894, -0.471393, -0.449607, -0.427551, -0.405237, -0.382679,
    -0.359891, -0.336885, -0.313677, -0.290280, -0.266708, -0.242975,
    -0.219096, -0.195085, -0.170957, -0.146725, -0.122405, -0.098012,
    -0.073559, -0.049062, -0.024536, 0.000006, 0.024547, 0.049074,
    0.073570, 0.098023, 0.122417, 0.146737, 0.170968, 0.195096, 0.219107,
    0.242986, 0.266719, 0.290291, 0.313688, 0.336896, 0.359901, 0.382690,
    0.405248, 0.427561, 0.449618, 0.471403, 0.492904, 0.514109, 0.535004,
    0.555576, 0.575814, 0.595705, 0.615238, 0.634399, 0.653179, 0.671565,
    0.689546, 0.707112, 0.724253, 0.740957, 0.757214, 0.773016, 0.788352,
    0.803213, 0.817590, 0.831474, 0.844858, 0.857733, 0.870091, 0.881925,
    0.893228, 0.903993, 0.914213, 0.923883, 0.932996, 0.941547, 0.949531,
    0.956943, 0.963779, 0.970034, 0.975704, 0.980787, 0.985279, 0.989178,
    0.992481, 0.995186, 0.997291, 0.998796, 0.999699
};

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

//-----------//
// operators //
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
            *(*(_termArray + step) + term[term_idx]) =
                (unsigned short)tmp_array_1[step];
            *(*(second_set->_termArray + step) + term[term_idx]) =
                (unsigned short)tmp_array_2[step];
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

//------------------------------//
// RangeTracker::GetRxGateDelay //
//------------------------------//

#define RANGE_GATE_NORMALIZER  0.049903

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

    float ttf = (2.0 * M_PI * (float)azimuth_step) / 32768.0 + P;
    ttf = fabs(ttf);
    ttf *= (256.0 / (2.0 * M_PI));

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
{
    return;
}

DopplerTracker::~DopplerTracker()
{
    return;
}

//-----------//
// operators //
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
            *(*(_termArray + step) + term[term_idx]) =
                (unsigned short)tmp_array_1[step];
            *(*(second_set->_termArray + step) + term[term_idx]) =
                (unsigned short)tmp_array_2[step];
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

    float ttf = (2.0 * M_PI * (float)azimuth_step) / 32768.0 + P;
    ttf = fabs(ttf);
    ttf *= (256.0 / (2.0 * M_PI));

    unsigned short tindex = (unsigned short)ttf;

    float cos1 = cos_table[COS_TABLE_SIZE_MOD(tindex)];
    float cos2 = cos_table[COS_TABLE_SIZE_MOD(tindex + 1)];

    float doppler_predict = C + A * ((ttf - (long)ttf) * (cos2 - cos1) + cos1);

    float rx_gate_error = (rx_gate_delay_fdn - (float)rx_gate_delay_dn) *
        RANGE_GATE_NORMALIZER * MU * HZ_PER_KHZ;
    float cmd_doppler_fdn = (doppler_predict + rx_gate_error) / 2000.0;

    short cmd_doppler_dn;
    if (cmd_doppler_fdn > 0.0)
        cmd_doppler_dn = (short)(cmd_doppler_fdn + 0.5);
    else
        cmd_doppler_dn = (short)(cmd_doppler_fdn - 0.5);
    *commanded_doppler_dn = -1 * cmd_doppler_dn;
    return(1);
}

//---------------------//
// DopplerTracker::Set //
//---------------------//

int
DopplerTracker::Set(
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
    for (unsigned int doppler_step = 0; doppler_step < _steps; doppler_step++)
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
    int         count,
    double*     terms,
    double*     a,
    double*     p,
    double*     c)
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
