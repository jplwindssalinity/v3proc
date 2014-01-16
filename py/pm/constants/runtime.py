## \namespace pm.constants.runtime Run-time State
"""Module to load run-time configuration:

HOME 
LOGIN
PLATFORM
HOST
"""
import os
import platform
import socket
import sys


HOME = os.getenv('HOME')

LOGIN = os.getlogin()

PLATFORM = sys.platform

HOST = socket.gethostname() or platform.uname()[1]


## Just for kicks and development
print "Run-time configuration: "
print "$HOME =", HOME
print "LOGIN =", LOGIN
print "PLATFORM =", PLATFORM
print "HOST =", HOST

