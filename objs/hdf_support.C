//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#include <stdio.h>
#include "hdf_support.h"

//----------------------//
// qs_get_integer_value //
//----------------------//

int
qs_get_integer_value(
    Attribute*  attr)
{
    char str[1024];
    char* substr;
    char* separators = "\n";
    int ival;

    strcpy(str, attr->val.vchar);
    substr = strtok(str, separators);
    if (strcmp(substr, "int") != 0)
    {
        fprintf(stderr, "Attribute %s not an integer\n", attr->name);
        exit(1);
    }

    substr = strtok(0, separators);
    if (strcmp(substr, "1") != 0)
    {
        fprintf(stderr, "Attribute %s not an scalar\n", attr->name);
        exit(1);
    }

    substr = strtok(0, separators);
    sscanf(substr, "%d", &ival);

    return(ival);
}

//--------------------//
// qs_get_float_value //
//--------------------//

float
qs_get_float_value(
    Attribute*  attr)
{
    char str[1024];
    char* substr;
    char* separators = "\n";
    float fval;

    strcpy(str, attr->val.vchar);
    substr = strtok(str, separators);
    if (strcmp(substr, "float") != 0)
    {
        fprintf(stderr, "Attribute %s not an float\n", attr->name);
        exit(1);
    }

    substr = strtok(0, separators);
    if (strcmp(substr, "1") != 0)
    {
        fprintf(stderr, "Attribute %s not an scalar\n", attr->name);
        exit(1);
    }

    substr = strtok(0, separators);
    sscanf(substr, "%f", &fval);

    return(fval);
}


//------------------//
// get_SD_attribute //
//------------------//

void
get_SD_attribute(
    int32       object_id,
    char*       name,
    Attribute*  attr)
{
    int i = SDfindattr(object_id, name);
    if (i == -1)
    {
        fprintf(stderr, "Variable %s not found in HDF\n", name);
        exit(1);
    }

    SDattrinfo(object_id, i, attr->name, &attr->type, &attr->count);
    switch (attr->type)
    {
    case DFNT_CHAR:
    case DFNT_UCHAR:
        SDreadattr(object_id, i, attr->val.vchar);
        attr->val.vchar[attr->count] = 0;
        break;
    case DFNT_INT8:
        if (attr->count <= 1024)
            SDreadattr(object_id, i, attr->val.vchar);
        break;
    case DFNT_UINT8:
        if (attr->count <= 256)
            SDreadattr(object_id, i, attr->val.vuchar);
        break;
    case DFNT_INT16:
        if (attr->count <= 256)
            SDreadattr(object_id, i, attr->val.vshort);
        break;
    case DFNT_UINT16:
        if (attr->count <= 256)
            SDreadattr(object_id, i, attr->val.vushort);
        break;
    case DFNT_INT32:
        if (attr->count <= 256)
            SDreadattr(object_id, i, attr->val.vint);
        break;
    case DFNT_UINT32:
        if (attr->count <= 256)
            SDreadattr(object_id, i, attr->val.vuint);
        break;
    case DFNT_FLOAT32:
        if (attr->count <= 256)
            SDreadattr(object_id, i, attr->val.vfloat);
        break;
    case DFNT_FLOAT64:
        if (attr->count <= 256)
            SDreadattr(object_id, i, attr->val.vdouble);
        break;
    }
}

//---------------//
// UpdateDataSet //
//---------------//

int
UpdateDataSet(
    const char*  filename,
    char*        name,
    int32        index,
    float*       array)
{
    int retval = 0;
    HDF_update_file hdf_struct;
    if (hdf_struct.open(filename))
    {
        int32 datasets[500];
        int32 attr[500];
        int num = SDfileinfo(hdf_struct.sd_id, datasets, attr);
        for (int c = 0; c < num; c++)
        {
            char name[50];
            int32 rank;
            int32 dims[3];
            int32 nt, nattr;
            SDgetinfo(datasets[c], name, &rank, dims, &nt, &nattr);
            fprintf(stderr, "%s\n", name);
        }
        retval = hdf_struct.update(name, index, array);
    }
    else
        return(0);

    hdf_struct.close();
    return(retval);
}

//---------------//
// UpdateDataSet //
//---------------//

int
UpdateDataSet(
    const char*  filename,
    char*        name,
    int32        index,
    int*         array)
{
    int retval = 0;
    HDF_update_file hdf_struct;
    if (hdf_struct.open(filename))
    {
        int32 datasets[500];
        int32 attr[500];
        int num = SDfileinfo(hdf_struct.sd_id, datasets, attr);
        for (int c = 0; c < num; c++)
        {
            char name[50];
            int32 rank;
            int32 dims[3];
            int32 nt, nattr;
            SDgetinfo(datasets[c], name, &rank, dims, &nt, &nattr);
            fprintf(stderr, "%s\n", name);
        }
        retval = hdf_struct.update(name, index, array);
    }
    else
    {
        return(0);
    }
    hdf_struct.close();
    return(retval);
}

//-----------------------//
// HDF_update_file::open //
//-----------------------//

int
HDF_update_file::open(
    const char*  filename)
{
    sd_id = SDstart(filename, DFACC_RDWR);
    if (sd_id == FAIL)
    {
        fprintf(stderr, "Unable to open file for access 'ALL'\n");
        return(0);
    }
    else
        return(1);
}

//-----------------------------------------//
// HDF_update_file::get_dataset_attributes //
//-----------------------------------------//

int HDF_update_file::get_dataset_attributes(char *name,SD_attributes &attr)
{
    int32    dataset_id;
    int32    dataset_index;

    char    attr_name[256];
    double    temp0=0;

    int    i;
    char    temp_char[256];
    float    temp_float[256];
    double    temp_double[256];
    int    temp_int[256];
    short    temp_short[256];
    uint    temp_uint[256];
    ushort    temp_ushort[256];
    uchar    temp_uchar[256];
    int32    attr_type,attr_count;

    dataset_index = SDnametoindex(sd_id,name);
    if (dataset_index == FAIL)
    {
        fprintf(stderr,"ERROR - No such dataset: %s\n",name);
        return 0;
    }
    attr.index = dataset_index;
    dataset_id = SDselect(sd_id,dataset_index);

    SDgetinfo(dataset_id,attr.name,&attr.rank
        ,attr.dimensions,&attr.type,&attr.num_attributes);

    for (i = 0; i < attr.num_attributes ; i++)
    {
        SDattrinfo(dataset_id,i,attr_name,&attr_type,&attr_count);

        switch (attr_type)
        {
        case DFNT_CHAR:
        case DFNT_UCHAR:
            SDreadattr(dataset_id,i,&temp_char);
            temp_char[attr_count] = 0;
            break;

        case DFNT_INT8:
            if (attr_count <=256)
            {
                SDreadattr(dataset_id,i,&temp_char);
                temp0 = temp_char[0];
            }
            break;

        case DFNT_UINT8:
            if (attr_count <=256)
            {
                SDreadattr(dataset_id,i,&temp_uchar);
                temp0 = temp_uchar[0];
            }

            break;

        case DFNT_INT16:
            if (attr_count <=256)
            {
                SDreadattr(dataset_id,i,&temp_short);
                temp0 = temp_short[0];
            }

            break;

        case DFNT_UINT16:
            if (attr_count <=256)
            {
                SDreadattr(dataset_id,i,&temp_ushort);
                temp0 = temp_ushort[0];
            }
            break;

        case DFNT_INT32:
            if (attr_count <=256)
            {
                SDreadattr(dataset_id,i,&temp_int);
                temp0 = temp_int[0];
            }
            break;

        case DFNT_UINT32:
            if (attr_count <=256)
            {
                SDreadattr(dataset_id,i,&temp_uint);
                temp0 = temp_uint[0];
            }
            break;

        case DFNT_FLOAT32:
            if (attr_count <=256)
            {
                SDreadattr(dataset_id,i,&temp_float);
                temp0 = temp_float[0];
            }
            break;

        case DFNT_FLOAT64:
            if (attr_count <=256)
            {
                SDreadattr(dataset_id,i,&temp_double);
                temp0 = temp_double[0];
            }
            break;

        }

        if ( strcmp(attr_name,"scale_factor") == 0 )
        {
            attr.scale = temp0;
        }
        else if ( strcmp(attr_name,"add_offset") == 0 )
        {
            attr.offset = temp0;
        }

    }
    SDendaccess(dataset_id);

    return 1;
}

int HDF_update_file::update(char *name,int index, int *values)
{
    int32        dataset_id;
    SD_attributes    attributes;
    int        num_values;
    int        i;

    char    *temp_char;
    float    *temp_float;
    double    *temp_double;
    short    *temp_short;
    uint    *temp_uint;
    ushort    *temp_ushort;
    uchar    *temp_uchar;

    int32    start[6] = {0,0,0,0,0,0};
    int32    stride[6] = {1,1,1,1,1,1};
    int32    edge[6] = {1,0,0,0,0,0};


    if (! get_dataset_attributes(name,attributes))
    {
        fprintf(stderr,"No such dataset: %s\n",name);
        return 0;
    }

    if (index > attributes.dimensions[0])
    {
        fprintf(stderr,"Out of dimension range: %d\n",index);
        return 0;
    }

    num_values = 1;
    start[0] = index;
    edge[0] = 1;

    for(i=1; i < attributes.rank ; i++)
    {
        edge[i] = attributes.dimensions[i];
        num_values *= attributes.dimensions[i];
    }

    dataset_id = SDselect(sd_id,attributes.index);

    switch (attributes.type)
    {
    case DFNT_CHAR:
    case DFNT_INT8:
        temp_char = new (char[num_values]);
        for (i = 0; i < num_values ; i++)
            temp_char[i] = (char) (values[i] / attributes.scale);

        SDwritedata(dataset_id,start,stride,edge,temp_char);
        delete temp_char;
        break;

    case DFNT_UCHAR:
    case DFNT_UINT8:
        temp_uchar = new (uchar[num_values]);
        for (i = 0; i < num_values ; i++)
            temp_uchar[i] = (uchar) (values[i] / attributes.scale);

        SDwritedata(dataset_id,start,stride,edge,temp_uchar);
        delete temp_uchar;
        break;


    case DFNT_INT16:
        temp_short = new (short[num_values]);
        for (i = 0; i < num_values ; i++)
            temp_short[i] = (short) (values[i] / attributes.scale);

        SDwritedata(dataset_id,start,stride,edge,temp_short);
        delete temp_short;
        break;

    case DFNT_UINT16:
        temp_ushort = new (ushort[num_values]);
        for (i = 0; i < num_values ; i++)
            temp_ushort[i] = (ushort) (values[i] / attributes.scale);

        SDwritedata(dataset_id,start,stride,edge,temp_ushort);
        delete temp_ushort;
        break;

    case DFNT_INT32:
        SDwritedata(dataset_id,start,stride,edge,values);
        break;

    case DFNT_UINT32:
        temp_uint = new (uint[num_values]);
        for (i = 0; i < num_values ; i++)
            temp_uint[i] = (uint) (values[i] / attributes.scale);

        SDwritedata(dataset_id,start,stride,edge,temp_uint);
        delete temp_uint;
        break;

    case DFNT_FLOAT32:
        temp_float = new (float[num_values]);
        for (i = 0; i < num_values ; i++)
            temp_float[i] = (float) (values[i] / attributes.scale);

        SDwritedata(dataset_id,start,stride,edge,temp_float);
        delete temp_float;
        break;

    case DFNT_FLOAT64:
        temp_double = new (double[num_values]);
        for (i = 0; i < num_values ; i++)
            temp_double[i] = (double) (values[i] / attributes.scale);

        SDwritedata(dataset_id,start,stride,edge,temp_double);
        delete temp_double;
        break;

    }

    SDendaccess(dataset_id);

    return 1;
}

int HDF_update_file::update(char *name,int index, float *values)
{
    int32        dataset_id;
    SD_attributes    attributes;
    int        num_values;
    int        i;

    char    *temp_char;
    double    *temp_double;
    short    *temp_short;
    int    *temp_int;
    uint    *temp_uint;
    ushort    *temp_ushort;
    uchar    *temp_uchar;

    int32    start[6] = {0,0,0,0,0,0};
    int32    stride[6] = {1,1,1,1,1,1};
    int32    edge[6] = {1,0,0,0,0,0};


    if (! get_dataset_attributes(name,attributes))
    {
        fprintf(stderr,"No such dataset: %s\n",name);
        return 0;
    }

    if (index > attributes.dimensions[0])
    {
        fprintf(stderr,"Out of dimension range: %d\n",index);
        return 0;
    }

    num_values = 1;
    start[0] = index;
    edge[0] = 1;

    for(i=1; i < attributes.rank ; i++)
    {
        edge[i] = attributes.dimensions[i];
        num_values *= attributes.dimensions[i];
    }

    dataset_id = SDselect(sd_id,attributes.index);

    switch (attributes.type)
    {
    case DFNT_CHAR:
    case DFNT_INT8:
        temp_char = new (char[num_values]);
        for (i = 0; i < num_values ; i++)
            temp_char[i] = (char) (values[i] / attributes.scale);

        SDwritedata(dataset_id,start,stride,edge,temp_char);
        delete temp_char;
        break;

    case DFNT_UCHAR:
    case DFNT_UINT8:
        temp_uchar = new (uchar[num_values]);
        for (i = 0; i < num_values ; i++)
            temp_uchar[i] = (uchar) (values[i] / attributes.scale);

        SDwritedata(dataset_id,start,stride,edge,temp_uchar);
        delete temp_uchar;
        break;


    case DFNT_INT16:
        temp_short = new (short[num_values]);
        for (i = 0; i < num_values ; i++)
            temp_short[i] = (short) (values[i] / attributes.scale);

        SDwritedata(dataset_id,start,stride,edge,temp_short);
        delete temp_short;
        break;

    case DFNT_UINT16:
        temp_ushort = new (ushort[num_values]);
        for (i = 0; i < num_values ; i++)
            temp_ushort[i] = (ushort) (values[i] / attributes.scale);

        SDwritedata(dataset_id,start,stride,edge,temp_ushort);
        delete temp_ushort;
        break;

    case DFNT_INT32:
        temp_int = new (int[num_values]);
        for (i = 0; i < num_values ; i++)
            temp_int[i] = (int) (values[i] / attributes.scale);

        SDwritedata(dataset_id,start,stride,edge,temp_int);
        delete temp_int;
        break;

    case DFNT_UINT32:
        temp_uint = new (uint[num_values]);
        for (i = 0; i < num_values ; i++)
            temp_uint[i] = (uint) (values[i] / attributes.scale);

        SDwritedata(dataset_id,start,stride,edge,temp_uint);
        delete temp_uint;
        break;

    case DFNT_FLOAT32:
        SDwritedata(dataset_id,start,stride,edge,values);
        break;

    case DFNT_FLOAT64:
        temp_double = new (double[num_values]);
        for (i = 0; i < num_values ; i++)
            temp_double[i] = (double) (values[i] / attributes.scale);

        SDwritedata(dataset_id,start,stride,edge,temp_double);
        delete temp_double;
        break;

    }

    SDendaccess(dataset_id);
    return 1;
}

int HDF_update_file::close(){
  SDend(sd_id);
  return(1);
}

int HDF_update_file::get_data(char *name,int *values)
{
	int32		dataset_id;
	SD_attributes	attributes;
	int		num_values;
	int		i;

	char	*temp_char;
	float	*temp_float;
	double	*temp_double;
	short	*temp_short;
	uint	*temp_uint;
	ushort	*temp_ushort;
	uchar	*temp_uchar;
	int	*temp_int;

	int32	start[6] = {0,0,0,0,0,0};
	int32	stride[6] = {1,1,1,1,1,1};
	int32	edge[6] = {1,0,0,0,0,0};


	if (! get_dataset_attributes(name,attributes))
	{
		fprintf(stderr,"No such dataset: %s\n",name);
		return 0;
	}

	num_values = 1;
	start[0] = 1;
	edge[0] = 1;

	for(i=1; i < attributes.rank ; i++)
	{
		edge[i] = attributes.dimensions[i];
		num_values *= attributes.dimensions[i];
	}

	dataset_id = SDselect(sd_id,attributes.index);

	switch (attributes.type)
	{
	case DFNT_CHAR:
	case DFNT_INT8:
		temp_char = new (char[num_values]);
                SDreaddata(dataset_id,start,stride,edge,temp_char);
		for (i = 0; i < num_values ; i++)
			values[i] = (int) (temp_char[i]*attributes.scale + attributes.offset);
		delete temp_char;
		break;

	case DFNT_UCHAR:
	case DFNT_UINT8:
		temp_uchar = new (uchar[num_values]);
                SDreaddata(dataset_id,start,stride,edge,temp_uchar);
		for (i = 0; i < num_values ; i++)
			values[i] = (int) (temp_uchar[i]*attributes.scale + attributes.offset);
		delete temp_uchar;
		break;


	case DFNT_INT16:
		temp_short = new (short[num_values]);
                SDreaddata(dataset_id,start,stride,edge,temp_short);
		for (i = 0; i < num_values ; i++)
			values[i] = (int) (temp_short[i]*attributes.scale + attributes.offset);
		delete temp_short;
		break;

	case DFNT_UINT16:
		temp_ushort = new (ushort[num_values]);
                SDreaddata(dataset_id,start,stride,edge,temp_ushort);
		for (i = 0; i < num_values ; i++)
			values[i] = (int) (temp_ushort[i]*attributes.scale + attributes.offset);
		delete temp_ushort;
		break;

	case DFNT_INT32:
		temp_int = new (int[num_values]);
                SDreaddata(dataset_id,start,stride,edge,temp_int);
		for (i = 0; i < num_values ; i++)
			values[i] = (int) (temp_int[i]*attributes.scale + attributes.offset);
		delete temp_ushort;
		break;

	case DFNT_UINT32:
		temp_uint = new (uint[num_values]);
                SDreaddata(dataset_id,start,stride,edge,temp_uint);
		for (i = 0; i < num_values ; i++)
			values[i] = (int) (temp_uint[i]*attributes.scale + attributes.offset);
		delete temp_uint;
		break;

	case DFNT_FLOAT32:
		temp_float = new (float[num_values]);
		SDreaddata(dataset_id,start,stride,edge,temp_float);
		for (i = 0; i < num_values ; i++)
			values[i] = (int) (temp_float[i]*attributes.scale + attributes.offset);
		delete temp_float;
		break;

	case DFNT_FLOAT64:
		temp_double = new (double[num_values]);
		SDreaddata(dataset_id,start,stride,edge,temp_double);
		for (i = 0; i < num_values ; i++)
			values[i] = (int) (temp_double[i]*attributes.scale + attributes.offset);
		delete temp_double;
		break;
	}

	SDendaccess(dataset_id);

	return 1;
}

int HDF_update_file::get_data(char *name,float *values)
{
	int32		dataset_id;
	SD_attributes	attributes;
	int		num_values;
	int		i;

	char	*temp_char;
	float	*temp_float;
	double	*temp_double;
	short	*temp_short;
	uint	*temp_uint;
	ushort	*temp_ushort;
	uchar	*temp_uchar;
	int	*temp_int;

	int32	start[6] = {0,0,0,0,0,0};
	int32	stride[6] = {1,1,1,1,1,1};
	int32	edge[6] = {1,0,0,0,0,0};


	if (! get_dataset_attributes(name,attributes))
	{
		fprintf(stderr,"No such dataset: %s\n",name);
		return 0;
	}

	num_values = 1;
	start[0] = 1;
	edge[0] = 1;

	for(i=1; i < attributes.rank ; i++)
	{
		edge[i] = attributes.dimensions[i];
		num_values *= attributes.dimensions[i];
	}

	dataset_id = SDselect(sd_id,attributes.index);

	switch (attributes.type)
	{
	case DFNT_CHAR:
	case DFNT_INT8:
		temp_char = new (char[num_values]);
                SDreaddata(dataset_id,start,stride,edge,temp_char);
		for (i = 0; i < num_values ; i++)
			values[i] = temp_char[i]*attributes.scale + attributes.offset;
		delete temp_char;
		break;

	case DFNT_UCHAR:
	case DFNT_UINT8:
		temp_uchar = new (uchar[num_values]);
                SDreaddata(dataset_id,start,stride,edge,temp_uchar);
		for (i = 0; i < num_values ; i++)
			values[i] = temp_uchar[i]*attributes.scale + attributes.offset;
		delete temp_uchar;
		break;


	case DFNT_INT16:
		temp_short = new (short[num_values]);
                SDreaddata(dataset_id,start,stride,edge,temp_short);
		for (i = 0; i < num_values ; i++)
			values[i] = temp_short[i]*attributes.scale + attributes.offset;
		delete temp_short;
		break;

	case DFNT_UINT16:
		temp_ushort = new (ushort[num_values]);
                SDreaddata(dataset_id,start,stride,edge,temp_ushort);
		for (i = 0; i < num_values ; i++)
			values[i] = temp_ushort[i]*attributes.scale + attributes.offset;
		delete temp_ushort;
		break;

	case DFNT_INT32:
		temp_int = new (int[num_values]);
                SDreaddata(dataset_id,start,stride,edge,temp_int);
		for (i = 0; i < num_values ; i++)
			values[i] = temp_int[i]*attributes.scale + attributes.offset;
		delete temp_int;
		break;

	case DFNT_UINT32:
		temp_uint = new (uint[num_values]);
                SDreaddata(dataset_id,start,stride,edge,temp_uint);
		for (i = 0; i < num_values ; i++)
			values[i] = temp_uint[i]*attributes.scale + attributes.offset;
		delete temp_uint;
		break;

	case DFNT_FLOAT32:
		temp_float = new (float[num_values]);
		SDreaddata(dataset_id,start,stride,edge,temp_float);
		for (i = 0; i < num_values ; i++)
			values[i] = temp_float[i]*attributes.scale + attributes.offset;
		delete temp_float;
		break;

	case DFNT_FLOAT64:
		temp_double = new (double[num_values]);
		SDreaddata(dataset_id,start,stride,edge,temp_double);
		for (i = 0; i < num_values ; i++)
			values[i] = temp_double[i]*attributes.scale + attributes.offset;
		delete temp_double;
		break;
	}

	SDendaccess(dataset_id);

	return 1;
}
