/*********************************************************************
 *
 * ORIGINAL AUTHOR: Thomas Werne
 * COMPANY: Jet Propulsion Laboratory
 * VERSION:
 *
 * File Name:     NetCDF_Var.h
 * Creation Date: 13 Apr 2012
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 * Copyright 2009-2012, by the California Institute of Technology.
 * ALL RIGHTS RESERVED.  United States Government Sponsorship
 * acknowledged. Any commercial use must be negotiated with the Office
 * of Technology Transfer at the California Institute of Technology.
 *
 * This software may be subject to U.S. export control laws and
 * regulations.  By accepting this document, the user agrees to comply
 * with all U.S. export laws and regulations.  User has the
 * responsibility to obtain export licenses, or other export authority
 * as may be required before exporting such information to foreign
 * countries or providing access to foreign persons.
 ********************************************************************/

#ifndef NETCDF_VARIABLE_H
#define NETCDF_VARIABLE_H

#include <netcdf.h>
#include <string.h>
#include <string>
#include <vector>

#include "NetCDF.h"
#include "NetCDF_Attr.h"

/*********************************************************************
 * Base class declaration
 ********************************************************************/
class NetCDF_Var_Base {
public:
    NetCDF_Var_Base(const char *name, int ncid, int ndims,
            const int *dim_ids, const int *dim_szs);
    virtual ~NetCDF_Var_Base();
    int Define();
    int Write();
    void AddAttribute(NetCDF_Attr_Base *attr);
    int WriteAttributes();
    string &GetName() { return name; };

protected:
    string name;
    int ncid;
    int ndims;
    int *dim_ids;
    int *dim_szs;
    int varid;
    int nelems;
    vector <NetCDF_Attr_Base *> attrs;

private:
    virtual int WriteMe() = 0;
    virtual nc_type Type() = 0;
};
 
/*********************************************************************
 * Type-specific variable class
 ********************************************************************/
// A helper function for writing variables to .nc files
template<typename T> int Write_Var(int ncid, int varid, const T *src);

// Forward declarations for template specialization
template <> int Write_Var<unsigned char>(int ncid, int varid, const unsigned char *src);
template <> int Write_Var<signed char>(int ncid, int varid, const signed char *src);
template <> int Write_Var<char>(int ncid, int varid, const char *src);
template <> int Write_Var<short>(int ncid, int varid, const short *src);
template <> int Write_Var<int>(int ncid, int varid, const int *src);
template <> int Write_Var<float>(int ncid, int varid, const float *src);
template <> int Write_Var<double>(int ncid, int varid, const double *src);

// The actual class declaration
template <typename T>
class NetCDF_Var : public NetCDF_Var_Base {
public:
    NetCDF_Var(const char *name, int ncid, int ndims, const int *dim_ids,
            const int *dim_szs) :
            NetCDF_Var_Base(name, ncid, ndims, dim_ids, dim_szs) {
        MALLOC(data, nelems);
    };
    ~NetCDF_Var() { FREE(data); };
    void SetData(const size_t *dims, const T val);
private:
    T *data;
    int WriteMe() { return Write_Var(ncid, varid, data); };
    nc_type Type() { return NetCDF_Type<T>(); };
};


// SetData function
template <typename T>
void NetCDF_Var<T>::SetData(const size_t *dims, const T val) {
    size_t offset = dims[0];
    for (int i = 1; i < ndims; i++) {
        offset *= this->dim_szs[i];
        offset += dims[i];
    }
    data[offset] = val;
};

#endif


