/*********************************************************************
 *
 * ORIGINAL AUTHOR: Thomas Werne
 * COMPANY: Jet Propulsion Laboratory
 * VERSION:
 *
 * File Name:     NetCDF.h
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

#ifndef NETCDF_H_
#define NETCDF_H_

#include <string>

using namespace std;

#define RAD_TO_DEG(x) ((x)*180.0f/M_PI)
#define DEG_TO_RAD(x) ((x)*M_PI/180.0f)

#define FREE(x) \
do {            \
    free(x);    \
    x = NULL;   \
} while(0)

class NC_Attribute {
public:
    NC_Attribute(const char *name, size_t length) :
        name(name), length(length) {};
    virtual ~NC_Attribute() {};
    int Write(int ncid, int varid);

protected:
    string name;
    size_t length;

private:
    virtual int WriteAttr(int ncid, int varid) = 0;
};

class String_NC_Attribute : public NC_Attribute {
public:
    String_NC_Attribute(const char *name, const char *string);
    ~String_NC_Attribute();

private:
    char *p;
    int WriteAttr(int ncid, int varid);
};

class Float_NC_Attribute : public NC_Attribute {
public:
    Float_NC_Attribute(const char *name, const float *vals,
            const size_t length = 1);
    Float_NC_Attribute(const char *name, const float val);
    ~Float_NC_Attribute();

private:
    float *p;
    int WriteAttr(int ncid, int varid);
};

// Really belongs in a different header
typedef struct {
    float lambda_0;
    float inclination;
    float rev_period;
    int xt_steps;
    double at_res;
    double xt_res;
} latlon_config;

void bin_to_latlon(int at_ind, int ct_ind,
        const latlon_config *config, float *lat, float *lon);
#endif

