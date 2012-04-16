/*********************************************************************
 *
 * ORIGINAL AUTHOR: Thomas Werne
 * COMPANY: Jet Propulsion Laboratory
 * VERSION:
 *
 * File Name:     NetCDF_Attr.C
 * Creation Date: 11 Apr 2012
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

static const char rcs_id[] =
    "@(#) $Id$";

#include <netcdf.h>
#include <string.h>

#include "NetCDF_Attr.h"

/*********************************************************************
 * Base attribute class implementation
 ********************************************************************/
int NetCDF_Attr_Base::Write(int ncid, int varid) {
    return WriteMe(ncid, varid);
}

// Type-specific instantiation for CreateNetCDF_Attr
template <>
void CreateNetCDF_Attr(char **p, const char *str, const size_t length) {
    *p = strdup(str);
}

// Type-specific instantiations for Write_Attr()
template <>
int Write_Attr<char>(int ncid, int varid, const char *name, size_t length, const char *src) {
    return nc_put_att_text(ncid, varid, name, strlen(src), src);
}
template <>
int Write_Attr<unsigned char>(int ncid, int varid, const char *name, size_t length, const unsigned char *src) {
    return nc_put_att_uchar(ncid, varid, name, NC_BYTE, length, src);
}
template <>
int Write_Attr<signed char>(int ncid, int varid, const char *name, size_t length, const signed char *src) {
    return nc_put_att_schar(ncid, varid, name, NC_BYTE, length, src);
}
template <>
int Write_Attr<short>(int ncid, int varid, const char *name, size_t length, const short *src) {
    return nc_put_att_short(ncid, varid, name, NC_SHORT, length, src);
}
template <>
int Write_Attr<int>(int ncid, int varid, const char *name, size_t length, const int *src) {
    return nc_put_att_int(ncid, varid, name, NC_INT, length, src);
}
template <>
int Write_Attr<float>(int ncid, int varid, const char *name, size_t length, const float *src) {
    return nc_put_att_float(ncid, varid, name, NC_FLOAT, length, src);
}
template <>
int Write_Attr<double>(int ncid, int varid, const char *name, size_t length, const double *src) {
    return nc_put_att_double(ncid, varid, name, NC_DOUBLE, length, src);
}

