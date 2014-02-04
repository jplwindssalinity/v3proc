#!/usr/bin/perl
#
# File:        L1A3.pl
#
# Description: Process control script for the Level 1A Processor
#
# Author:      Generated by JEB, for test3
#
# Date:        Oct 31.
#

# Input Parameters (will be h5.attrs.get('ancillary_data_descriptors_GLOSDS'))


$ENV{"SWS_L1A_CONFIG_FILESPEC"} = "/u/polecat0/belz/rapidscat_science_data_processing/test3/app-res/QS_PC1A0004";
$ENV{"SWS_L1A_METACONFIG_FILESPEC"} = "/u/polecat0/belz/rapidscat_science_data_processing/test3/app-res/QS_MC1A0001";

# h5.attrs.get('input_data_granules_GLOSDS')) (which specs A, but we got da B)
$ENV{"SWS_NUM_LEVEL_0_FILES"} = "2";
$ENV{"SWS_LEVEL_0_FILESPEC_01"} = "/u/pawpaw-z0/fore/QS_spinning/data4eric_130930/L00/QS_S0A20130310616.20130311336";
$ENV{"SWS_LEVEL_0_FILESPEC_02"} = "/u/pawpaw-z0/fore/QS_spinning/data4eric_130930/L00/QS_S0A20130311108.20130311846";


# ZERO forever.
$ENV{"SWS_NUMBER_DATA_SKIPS"} = "0";

$ENV{"SWS_NUM_EPHEMERIS_FILES"} = "3";

#  h5.attrs.get('ancillary_data_descriptors_GLOSDS'))
$ENV{"SWS_EPHEMERIS_FILESPEC_01"} = "INPUTS/gpsephm/QS_SEPHG20130310243.20130311006.le";
$ENV{"SWS_EPHEMERIS_FILESPEC_02"} = "INPUTS/gpsephm/QS_SEPHG20130310921.20130311643.le";
$ENV{"SWS_EPHEMERIS_FILESPEC_03"} = "INPUTS/gpsephm/QS_SEPHG20130311108.20130311835.le";

# does order count?
$ENV{"SWS_NUM_ATTITUDE_FILES"} = "3";
# (will be h5.attrs.get('ancillary_data_descriptors_GLOSDS'))
$ENV{"SWS_ATTITUDE_FILESPEC_01"} = "INPUTS/att/QS_SATT_20130310243.20130311006.le";
$ENV{"SWS_ATTITUDE_FILESPEC_02"} = "INPUTS/att/QS_SATT_20130310921.20130311643.le";
$ENV{"SWS_ATTITUDE_FILESPEC_03"} = "INPUTS/att/QS_SATT_20130311108.20130311835.le";

# why are these not LE?
$ENV{"SWS_TELEMETRY_CONSTANTS_FILESPEC"} = "/u/polecat0/belz/rapidscat_science_data_processing/test2/tables/QS_CNTM0019";
$ENV{"SWS_CALIBRATION_MODE_MAP_FILESPEC"} = "/u/polecat0/belz/rapidscat_science_data_processing/test2/tables/QS_CALM0001";
$ENV{"SWS_DN_EU_CONVERSION_FILESPEC"} = "/u/polecat0/belz/rapidscat_science_data_processing/test2/tables/QS_DNEU0005";
$ENV{"SWS_LEAP_SECOND_FILESPEC"} = "/u/polecat0/belz/rapidscat_science_data_processing/test2/tables/LEAP0004";
$ENV{"SWS_TIME_CONVERSION_DATA_FILESPEC"} = "/u/polecat0/belz/rapidscat_science_data_processing/test2/tables/QS_SCLK_UTC";
$ENV{"SWS_GLOBAL_CONSTANTS_FILESPEC"} = "/u/polecat0/belz/rapidscat_science_data_processing/test2/tables/GLOB0003";

# Output Parameters

$ENV{"SWS_LEVEL_1A_FILEPATH"} = "/u/polecat0/belz/rapidscat_science_data_processing/test3/L1A/data";
# really? "qarpt" how about QA_REPORT/
$ENV{"SWS_OUTPUT_REPORT_PATH"} = "/u/polecat0/belz/rapidscat_science_data_processing/test3/L1A/qarpt";
# wha't a CALP?
$ENV{"SWS_CAL_PULSE_FILEPATH"} = "/u/polecat0/belz/rapidscat_science_data_processing/test3/CALP/data";
# this is a silly path
$ENV{"SWS_STATUSLOG_PATH"} = "/u/polecat0/belz/rapidscat_science_data_processing/test3/app-logs/pm/meta";
# lgo file
$ENV{"SWS_ERRLOG_FILESPEC"} = "/u/polecat0/belz/rapidscat_science_data_processing/test3/L1A/log/L1A.log";
#$ENV{"SWS_ERRLOG_FILESPEC"} = "./last.log";

# Processing Parameters

# the ultimate primtive obsession: 1,2 = rev/pass based
$ENV{"SWS_LEVEL_1A_EXTENT"} = "1";
# seconds before and after data file that ephemeris is neede for good interpolation
$ENV{"SWS_REV_GRANULE_OVERLAP_TIME"} = "120";
# DI: who knows this number fisrt?
$ENV{"SWS_FIRST_REV_NUMBER"} = "70934";
$ENV{"SWS_FIRST_REV_NUMBER"} = "70932";
# totally unknown why thi sexists
$ENV{"SWS_FIRST_REV_TIME"} = "";
#  get time from earliest range starting time in the level0filespec
$ENV{"SWS_BEGIN_TIME_SPAN"} = "2013-031T08:00:00.000";
$ENV{"SWS_LENGTH_TIME_SPAN"} = "28800";
$ENV{"SWS_PROCESS_ENVIRONMENT"} = "SeaPAC";
$ENV{"SWS_PARAM_FILESPEC"} = $0;
$ENV{"SWS_TASK_ID"} = "794212";

# Monitor Parameters

$ENV{"SWS_MON_SWS"} = "MON_L_OFF";
$ENV{"SWS_MON_L1A_LP"} = "MON_L_OFF";
$ENV{"SWS_MON_CAL_PULSE"} = "MON_L_OFF";
$ENV{"SWS_MON_INTERP_TELEM"} = "MON_L_OFF";
$ENV{"SWS_MON_PROC_TELEM"} = "MON_L_OFF";

# PM Variables

$ENV{"PM_SERVER"} = "SPAC_INVENTORY";
$ENV{"PM_DB"} = "inventory";
$ENV{"PM_PMS"} = "PMSERVER";

# Call the executable to run the Level 1A Processor.

# This is a WTF input.
$ENV{"SWS_ERRMSG_DIR"} = "/u/polecat0/belz/rapidscat_science_data_processing/test2/gen/";

system("/u/polecat0/belz/new_pmachines/bin/qs_lp_L1A") == 0
    or die "Level 1A Processor terminated";
