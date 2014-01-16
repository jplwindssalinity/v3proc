## \namespace pm.constants.inputs Level Processor RDF defaults
"""RDF Constants rdf sub-package,

with some important ENV names (rdf) keys

here."""

## Task ID
TASK_ID = 'SWS_TASK_ID'

## Time Conversion File
TIME_CONVERSION = "SWS_TIME_CONVERSION_DATA_FILESPEC"

## Error log file for all levels.
ERR_LOG = "SWS_ERRLOG_FILESPEC"

## Basic inputs string or string-template
INPUTS = {'00': 'SWS_SPACECRAFT_L0_FILESPEC',
          '1A': 'SWS_LEVEL_0_FILESPEC_%s',
          '1B': 'SWS_LEVEL_1A_FILESPEC',
          '2A': 'SWS_LEVEL_1B_FILESPEC',
          '2B': 'SWS_LEVEL_2A_FILESPEC'}

