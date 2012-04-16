/*********************************************************************
 *
 * ORIGINAL AUTHOR: Thomas Werne
 * COMPANY: Jet Propulsion Laboratory
 * VERSION:
 *
 * File Name:     NetCDF_Attr_Base.h
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

#ifndef NETCDF_ATTR_H
#define NETCDF_ATTR_H

#include <string>

#include "NetCDF.h"

using namespace std;

/*********************************************************************
 * Base class declaration
 ********************************************************************/
class NetCDF_Attr_Base {
public:
    NetCDF_Attr_Base(const char *name, size_t length) :
        name(name), length(length) {};
    virtual ~NetCDF_Attr_Base() {};
    int Write(int ncid, int varid);

protected:
    string name;
    size_t length;

private:
    virtual int WriteMe(int ncid, int varid) = 0;
};

/*********************************************************************
 * Type-specific attribute class
 ********************************************************************/
// A helper function for writing attributes to .nc files
template<typename T>
int Write_Attr(int ncid, int varid, const char *name, size_t length, const T *src);

// Forward declaration for template specialization
template <> int Write_Attr<char>(int ncid, int varid, const char *name, size_t length, const char *src);
template <> int Write_Attr<unsigned char>(int ncid, int varid, const char *name, size_t length, const unsigned char *src);
template <> int Write_Attr<signed char>(int ncid, int varid, const char *name, size_t length, const signed char *src);
template <> int Write_Attr<short>(int ncid, int varid, const char *name, size_t length, const short *src);
template <> int Write_Attr<int>(int ncid, int varid, const char *name, size_t length, const int *src);
template <> int Write_Attr<float>(int ncid, int varid, const char *name, size_t length, const float *src);
template <> int Write_Attr<double>(int ncid, int varid, const char *name, size_t length, const double *src);

// A helper function for allocating space for the NetCDF Attr
template <typename T>
void CreateNetCDF_Attr(T **p, const T *vals, const size_t length) {
    MALLOC(*p, length);
    memcpy(*p, vals, length*sizeof(**p));
}
// Specialize on chars
template <>
void CreateNetCDF_Attr(char **p, const char *str, const size_t length);

// The actual class definition
template <typename T>
class NetCDF_Attr : public NetCDF_Attr_Base {
public:
    NetCDF_Attr(const char *name, const T *vals, const size_t length = 1) :
            NetCDF_Attr_Base(name, length) {

        CreateNetCDF_Attr(&p, vals, length);
    };

    NetCDF_Attr(const char *name, const T val) : NetCDF_Attr_Base(name, 1) {
        MALLOC(p, length);
        *p = val;
    };

    ~NetCDF_Attr() { FREE(p); };
private:
    T *p;
    int WriteMe(int ncid, int varid) {
        return Write_Attr(ncid, varid, name.data(), length, p);
    };
};

#endif

