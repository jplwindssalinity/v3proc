"""    
Default is to use the 2, 1, 3 order of rotations i.e.
1st rotate about z, second about x, last about y, or
    
rotmat = [rot @ y] [rot @ x] [rot @ z]

Notes:
0: Angles are in radians!
1: quaternions are in [w,x,y,z] order.
2: This is the rotation order used in the QuikSCAT SDS.
3: Makes heavy use of the cspice libraries to not reinvent wheels and stuff.
4: By convention spice considers rotations of the coordinate system, 
not of vectors.  We use opposite convention for attitude rotations.
5: Read spice docs: http://naif.jpl.nasa.gov/pub/naif/toolkit_docs/FORTRAN/req/rotation.html
"""
__version__ = '$Id$'

import spice
import numpy as np

DEFAULT_ORDER = (2,1,3)

def q2m(quat):
    """Quaternion to rotation matrix"""
    return spice.q2m(quat)

def m2q(mat):
    """Rotation matrix to Quaternion"""
    return spice.m2q(mat)

def m2eul(mat,order=DEFAULT_ORDER):
    """Converts a rotation matrix to Euler angles"""
    return spice.m2eul(mat,*order)

def eul2m(angles,order=DEFAULT_ORDER):
    """Converts Euler angles to a rotation matrix."""
    # Why can't I use (*angles,*order)??  De-uglify this
    return spice.eul2m(angles[0],angles[1],angles[2],*order)

def q2eul(quat,order=DEFAULT_ORDER):
    """Converts a quaterion = [w,x,y,z] to Euler angles"""
    return m2eul(q2m(quat),order)

def eul2q(angles,order=DEFAULT_ORDER):
    """Converts Euler angles to a quaternion"""
    return m2q(eul2m(angles,order))

def q2att(quat,order=DEFAULT_ORDER):
    """Converts Euler angles to attitude angles (opposite sign)"""
    return [-item for item in q2eul(quat,order)]

def att2q(angles,order=DEFAULT_ORDER):
    """Converts attitude angles to Euler angles (opposite sign)"""
    new_angles = [-item for item in angles]
    return eul2q(new_angles,order)


