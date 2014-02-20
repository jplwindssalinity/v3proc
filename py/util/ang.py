import numpy as np

def wrap(angles):
    """
    Wraps scalar angle or iterable container of angles to [-180,180)
    interval.
    """
    ## Not sure how to make the out class the same class as the in class???
    try:
        angles = np.asarray([_wrap(item) for item in angles])
    except TypeError:
        angles = _wrap(angles)
    return angles


def _wrap(angle):
    """Wraps a scalar angle to [-180,180) interval."""
    while angle >= 180.0: angle -= 360
    while angle < -180.0: angle += 360
    return angle

