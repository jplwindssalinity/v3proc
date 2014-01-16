"""Wrapper for data files, such as
QS_P1A20122861735.20132811504
QS_S0A20130302337.20132731220
QS_S0B20122861735.20132821036
QS_S1A70934.20130311850
QS_S1A70934.h5
QS_SATT_20123172249.20123180617
QS_SEPHG20122861735.20122870117
qst20121013005332p51sci.dat
qst20130131030817p62sci.dat
qst20130131062807p62sci.dat
"""
## \namespace pm.data.files Class that know data files and their relations

import collections
import operator
from pm.constants import configure
from pm.names import names
from pm.names import aux

## Class to ID strings
## @param tag A string that ID's a file's purpose from its name
## @param func A callable that returns True if func(name, tag) is name
## is ID'd as this type of file.
class ID(collections.namedtuple("ID", "tag func")):
    ## ID(src) --> ID.func(src, ID.tag)
    def __call__(self, src):
        return self.func(src, str(self))
    def __str__(self):
        return str(self.tag)
    

__TODO__ = "Fix inputs vs. outputs:"

## Input file ID for pm.contstnats.configure.LEVELS
LEVEL_OUTPUTS = {level: id_ for level, id_ in zip(configure.LEVELS,
                                                 (
            ID("%s_S0A" % names.MISSION_ID, str.startswith),
            ID("%s_S1A" % names.MISSION_ID, str.startswith),
            ID("%s_S1B" % names.MISSION_ID, str.startswith),
            ID("%s_S2A" % names.MISSION_ID, str.startswith)
            )
                                                 )
                }

          
## True IFF name is a GPSEPHM file
is_gpsephm = ID(aux.GPSephm.HEADER, str.startswith)
## True IFF name is a ATTitude file
is_attitude = ID(aux.ATT.HEADER, str.startswith)


def classify(src):
    if is_gpsephm(src):
        return 'GPSEPHM'
    elif is_attitude(src):
        return 'ATT'
    else:
        pass
    for key, finder in LEVEL_OUTPUTS.iteritems():
        if finder(src):
            return key
        continue
    return "ID failed"
        
        
