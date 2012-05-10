/*********************************************************************
 *
 * ORIGINAL AUTHOR: Thomas Werne
 * COMPANY: Jet Propulsion Laboratory
 * VERSION:
 *
 * File Name:     NetCDF.C
 * Creation Date: 15 Apr 2012
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

#include <math.h>
#include <netcdf.h>

#include "Constants.h"
#include "NetCDF.h"

// Type-specific instantiations for NetCDF_Type()
template <> nc_type NetCDF_Type<unsigned char>() {return NC_BYTE;};
template <> nc_type NetCDF_Type<signed char>() {return NC_BYTE;};
template <> nc_type NetCDF_Type<char>() {return NC_CHAR;};
template <> nc_type NetCDF_Type<short>() {return NC_SHORT;};
template <> nc_type NetCDF_Type<int>() {return NC_INT;};
template <> nc_type NetCDF_Type<float>() {return NC_FLOAT;};
template <> nc_type NetCDF_Type<double>() {return NC_DOUBLE;};

