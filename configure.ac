#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([rapidscat-sds-v3], [0.1], [thomas.a.werne@jpl.nasa.gov])
AC_CONFIG_SRCDIR([eadata/Args.h])
AC_CONFIG_HEADERS([config.h])

AC_CONFIG_MACRO_DIR([m4])

# Default values
CPPFLAGS+=" -I/usr/include/hdf -I/home/fore/software/sofa/20120301_a/c/src/ -I/home/fore/include"
LDFLAGS+=" -L/usr/lib64/hdf -L/home/fore/software/sofa/20120301_a/c/src/ -L/home/fore/lib"

# Checks for programs.
AC_PROG_AWK
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

AM_INIT_AUTOMAKE([1.10 subdir-objects -Wall foreign])
AM_PROG_CC_C_O
LT_INIT

# Checks for libraries.
AC_CHECK_LIB([m], [sin], [], AC_MSG_ERROR([Could not find math library]))
AC_SEARCH_LIBS([deflate], [z df], [],
             AC_MSG_ERROR([Could not find zip library]))
AC_CHECK_LIB([jpeg], [jpeg_abort], [], AC_MSG_ERROR([Could not find jpeg library]))
AC_CHECK_LIB([df], [Vopen], [], AC_MSG_ERROR([Could not find df library]))
#AC_CHECK_LIB([gsl], [gsl_complex_sin], [], AC_MSG_ERROR([Could not find df library]))
#AC_CHECK_LIB([gslcblas], [cblas_dgemm], [], AC_MSG_ERROR([Could not find libgsl library]))
AC_CHECK_LIB([hdf5], [HDfprintf], [], AC_MSG_ERROR([Could not find gslcblas library]))
AC_CHECK_LIB([mfhdf], [SDend], [], AC_MSG_ERROR([Could not find libmfhdf]))
AC_CHECK_LIB([netcdf], [nc_close], [], AC_MSG_ERROR([Could not find libnetcdf]))
AC_CHECK_LIB([sofa_c], [iauPpp], [], AC_MSG_ERROR([Could not find libsofa]))
AC_CHECK_LIB([nlopt], [nlopt_stop_f], [], AC_MSG_ERROR([Could not find nlopt]))

# FIXME: Replace `main' with a function in `-lnsl':
#AC_CHECK_LIB([nsl], [main])
# FIXME: Replace `main' with a function in `-lsz':
#AC_CHECK_LIB([sz], [main])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h limits.h malloc.h memory.h stddef.h stdlib.h string.h strings.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_UID_T
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_C_RESTRICT
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FSEEKO
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_FUNC_REALLOC
AC_CHECK_FUNCS([floor gettimeofday memset mkdir pow rint select setenv sqrt strcasecmp strchr strdup strncasecmp strrchr strstr tzset uname])

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
