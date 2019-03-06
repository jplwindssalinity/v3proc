![Open Source Love](https://badges.frapsoft.com/os/v1/open-source.png?v=103)
# JPL Ocean Vector Winds and Sea Surface Salinity Processing Suite

Copyright 1997-2019, by the California Institute of Technology. ALL RIGHTS RESERVED. United States Government Sponsorship acknowledged. Any commercial use must be negotiated with the Office of Technology Transfer at the California Institute of Technology.

This software may be subject to U.S. export control laws. By accepting this software, the user agrees to comply with all applicable U.S. export laws and regulations. User has the responsibility to obtain export licenses, or other export authority as may be required before exporting such information to foreign countries or providing access to foreign persons.

# Instructions for building

You will probably need to install some libs to make it build:

hdf-4.2.13
```
./configure --prefix=$PREFIX --enable-netcdf=no --disable-fortran --enable-shared=yes
make -j && make install && make clean
```

IAU SOFA:
go to http://www.iausofa.org/current_C.html and download the most recent c tarball.
```
cd sofa/20180130/c/src 
make -j
cp libsofa_c.a $PREFIX/lib
```

nlopt (most recent 2.5 uses cmake build system)
```
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=$PREFIX .. && make -j && make install
```
Eigen
```
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=$PREFIX .. && make -j && make install
```

HDF5, must be built with the high-level interface enabled
```
./configure --enable-hl --enable-shared --prefix=$PREFIX
make -j && make install
```

Assuming you build and install the various libs into PREFIX enviornment variable.
```
declare -x CPPFLAGS=-fpermissive\ -I$PREFIX/include\ -I$PREFIX/include/eigen3
declare -x LDFLAGS=-L$PREFIX/lib\ -L$PREFIX/lib64\ -lhdf5_hl
declare -x LD_LIBRARY_PATH=$PREFIX/lib:$PREFIX/lib64
./autogen.sh
./configure
make -j
```
* Note that -lhdf5_hl is a hack at the moment, not sure why it is required
