/*********************************************************************
 *
 * ORIGINAL AUTHOR: Thomas Werne
 * COMPANY: Jet Propulsion Laboratory
 * VERSION:
 *
 * File Name:     NetCDF_Var.C
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

#include <netcdf.h>
#include <strings.h>
#include <stdlib.h>

#include "NetCDF_Var.h"

/*********************************************************************
 * Base class implementation
 ********************************************************************/
// Constructor
NetCDF_Var_Base::NetCDF_Var_Base(const char *name, int ncid,
        int ndims, const int *dim_ids, const int *dim_szs) :
    name(name), ncid(ncid), ndims(ndims) {

    MALLOC(this->dim_ids, ndims);
    memcpy(this->dim_ids, dim_ids, ndims*sizeof(*dim_ids));

    MALLOC(this->dim_szs, ndims);
    memcpy(this->dim_szs, dim_szs, ndims*sizeof(*dim_szs));

    nelems = 1;
    for (int i = 0; i < ndims; i++) {
        nelems *= dim_szs[i];
    }
};

// Destructor
NetCDF_Var_Base::~NetCDF_Var_Base() {
    FREE(dim_szs);
    FREE(dim_ids);
    for (int i = attrs.size() - 1; i >= 0; i--) {
        delete attrs[i];
    }
};

// Define the variable in a .nc file
int NetCDF_Var_Base::Define() {
    return nc_def_var(ncid, name.data(), Type(), ndims, dim_ids, &varid);
}

// Write the variable to the .nc file
int NetCDF_Var_Base::Write() {
    return WriteMe();
}

// Add an attribute to the variable
void NetCDF_Var_Base::AddAttribute(NetCDF_Attr_Base *attr) {
    attrs.push_back(attr);
}

// Write the variable's attributes to the .nc file
int NetCDF_Var_Base::WriteAttributes() {
    int rv;
    for (vector <NetCDF_Attr_Base *>::iterator it = attrs.begin();
            it < attrs.end(); it++) {
        rv = (*it)->Write(ncid, varid);
        if (NC_ISSYSERR(rv)) {
            return rv;
        }
    }
    return NC_NOERR;
}

// Type-specific instantiations for Write_Var()
template <>
int Write_Var<unsigned char>(int ncid, int varid, const unsigned char *src) {
    return nc_put_var_uchar(ncid, varid, src); 
}
template <>
int Write_Var<signed char>(int ncid, int varid, const signed char *src) {
    return nc_put_var_schar(ncid, varid, src); 
}
template <>
int Write_Var<char>(int ncid, int varid, const char *src) {
    return nc_put_var_text(ncid, varid, src);
}
template <>
int Write_Var<short>(int ncid, int varid, const short *src) {
    return nc_put_var_short(ncid, varid, src); 
}
template <>
int Write_Var<int>(int ncid, int varid, const int *src) {
    return nc_put_var_int(ncid, varid, src); 
}
template <>
int Write_Var<float>(int ncid, int varid, const float *src) {
    return nc_put_var_float(ncid, varid, src); 
}
template <>
int Write_Var<double>(int ncid, int varid, const double *src) {
    return nc_put_var_double(ncid, varid, src); 
}

