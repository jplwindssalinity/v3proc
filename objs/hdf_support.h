//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef HDF_SUPPORT_H
#define HDF_SUPPORT_H

#include "hdf.h"
#include "mfhdf.h"

#define MAX_HDF_RETURN  1024
#define MAX_HDF_STRING  1024

typedef unsigned int    uint;
typedef unsigned short  ushort;
typedef unsigned char   uchar;

typedef struct value_struct
{
    char   name[1024];
    int32  type;
    int32  count;

    union
    {
        char    vchar[MAX_HDF_RETURN];
        short   vshort[MAX_HDF_RETURN];
        int     vint[MAX_HDF_RETURN];

        uint    vuint[MAX_HDF_RETURN];
        ushort  vushort[MAX_HDF_RETURN];
        uchar   vuchar[MAX_HDF_RETURN];

        float   vfloat[MAX_HDF_RETURN];
        double  vdouble[MAX_HDF_RETURN];
    } val;
} Attribute;

typedef struct sd_attribute_struct
{
    char    name[256];
    int32   rank;
    int32   dimensions[6];
    int32   type;
    int32   num_attributes;
    int32   index;

    double  scale;
    double  offset;
} SD_attributes;

int    qs_get_integer_value(Attribute *attr);
float  qs_get_float_value(Attribute *attr);
void   get_SD_attribute(int32 object_id, char *name,Attribute *attr);
int    UpdateDataSet(const char* filename, char *name, int32 index,
           float *array);
int    UpdateDataSet(const char* filename, char *name, int32 index,
           int *array);

class HDF_update_file
{
public:
    int  open(const char *filename);
    int  get_dataset_attributes(char *dataset_name, SD_attributes &attr);
    int  get_data(char *dataset_name,int *array);
    int  get_data(char *dataset_name,float *array);
    int  update(char *dataset_name,int index, int *array);
    int  update(char *dataset_name,int index, float *array);
    int  close();

    // protected:
    int32  sd_id;
};

#endif    /* _HDF_SUPPORT */
