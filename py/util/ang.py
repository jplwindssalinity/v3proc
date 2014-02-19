import numpy as np

def wrap(angle):
    """Wraps an angle to [-180,180) interval"""
    try:
        angle = _wrap(angle)
    except TypeError:
        while angle >= 180.0: angle -= 360
        while angle < -180.0: angle += 360
    return angle
    
def _wrap(angles):
    """Wraps an iterable container of angles to [-180,180) interval"""
    angles = np.asarray([wrap(item) for item in angles])
    return angles
    