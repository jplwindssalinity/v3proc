## \namespace pm.level0.injector Inject Level 0 Dependencies
"""Inject level 0 dependencies"""

# python imports
import functools
import os

# package imports
from pm.utils import helper
from pm.constants import inputs
from pm.constants.configure import L00

# sub-package imports
from pm.level0 import INPUT


## Chain all input massages together and run them.
## @param scidat The science telemetry file name.
## @param template pm.constants.configure.L00.
## @retval rdf_data for level 0 processing.
def inject(scidat, template=L00):
    # create a function chain for this scidat
    func = helper.chain(functools.partial(_inject_input, scidat),
                        _inject_errlog,
                        _inject_task_id)
    # apply to a copy of template
    return func(template.copy())


## Set Input File
## @param scidat The science telemetry file name.
## @param rdf_data The rdf data
## @retval rdf_data The updated rdf data.
def _inject_input(scidat, rdf_data):
    # update input file
    rdf_data[INPUT] = helper.new_basename(rdf_data[INPUT], scidat)
    return rdf_data


## Set Error Log
## @param rdf_data The rdf data
## @retval rdf_data The updated rdf data.
def _inject_errlog(rdf_data):
    scidat = os.path.basename(rdf_data[INPUT])
    rdf_data[inputs.ERR_LOG] = helper.new_basename(rdf_data[inputs.ERR_LOG],
                                             scidat + '.log')
    return rdf_data


## Set TASK_ID
## @param rdf_data The rdf data
## @retval rdf_data The updated rdf data.
def _inject_task_id(rdf_data):
    rdf_data[inputs.TASK_ID] = '123456'
    return rdf_data
