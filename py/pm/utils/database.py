## \namespace pm.utils.database Database Interface
"""helper/ module has python helper functions"""
import sqlite3

def register(func):
    def registered_func(*arg, **kwargs):
        result = func(*arg, **kwargs)
        save(result)
        return result
    return registered_func



def save(dum):
    print "I saved ", dum
    return None


