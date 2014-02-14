
def wrap(angle):
    """Wraps an angle to [-180,180) interval"""
    while angle >= 180.0: angle -= 360
    while angle < -180.0: angle += 360
    return angle