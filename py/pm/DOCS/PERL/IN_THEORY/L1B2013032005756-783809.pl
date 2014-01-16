#!/usr/bin/perl
#
# File:        L1B2013032005756-783809.pl
#
# Description: Process control script for the Level 1B Processor.
#
# Author:      Generated by the pm_spacOps application.
#
# Date:        02/01/2013 00:57:56.000
#

# Input Parameters

$ENV{"SWS_L1B_CONFIG_FILESPEC"} = "/u/qscat/app-res/QS_PC1B0003";
$ENV{"SWS_L1B_METACONFIG_FILESPEC"} = "/u/qscat/app-res/QS_MC1B0001";
$ENV{"SWS_LEVEL_1A_FILESPEC"} = "/seapac2/disk5/L1A/data/QS_S1A70932.20130311850";

$ENV{"SWS_NUMBER_DATA_SKIPS"} = "0";

$ENV{"SWS_NUM_EPHEMERIS_FILES"} = "1";
$ENV{"SWS_EPHEMERIS_FILESPEC_01"} = "/calctbl/hk2/gpsephm/QS_SEPHG20130310616.20130311323";

$ENV{"SWS_NUM_ATTITUDE_FILES"} = "0";

$ENV{"SWS_NUM_CAL_PULSE_FILES"} = "3";
$ENV{"SWS_CAL_PULSE_FILESPEC_01"} = "/seapac2/disk5/CALP/data/QS_SCAL70931.20130311024";
$ENV{"SWS_CAL_PULSE_FILESPEC_02"} = "/seapac2/disk5/CALP/data/QS_SCAL70932.20130311850";
$ENV{"SWS_CAL_PULSE_FILESPEC_03"} = "/seapac2/disk5/CALP/data/QS_SCAL70933.20130311850";

$ENV{"SWS_DOPPLER_RANGE_DELAY_FILESPEC"} = "/stt-tables/QS_DOPL0010";
$ENV{"SWS_L1B_CONSTANTS_FILESPEC"} = "/stt-tables/QS_CN1B1213";
$ENV{"SWS_TELEMETRY_CONSTANTS_FILESPEC"} = "/stt-tables/QS_CNTM0019";
$ENV{"SWS_ANTENNA_PATTERN_FILESPEC"} = "/stt-tables/QS_ANTP0001";
$ENV{"SWS_BASEBAND_FREQ_CORR_FILESPEC"} = "/stt-tables/QS_BFOC0003";
$ENV{"SWS_CAL_TEMPERATURE_FILESPEC"} = "/stt-tables/QS_CTMP0002";
$ENV{"SWS_EARTH_TOPO_MAP_FILESPEC"} = "/stt-tables/TOPO0002";
$ENV{"SWS_KPC_PARAMETERS_FILESPEC"} = "/stt-tables/QS_KPCP0001";
$ENV{"SWS_LEAP_SECOND_FILESPEC"} = "/stt-tables/LEAP0004";
$ENV{"SWS_S_FACTOR_FILESPEC"} = "/stt-tables/QS_SFAC0001";
$ENV{"SWS_SLICE_NOISE_FRACTION_FILESPEC"} = "/stt-tables/QS_SNFR0001";
$ENV{"SWS_X_FACTOR_FILESPEC"} = "/stt-tables/QS_XFAC3007";
$ENV{"SWS_GLOBAL_CONSTANTS_FILESPEC"} = "/stt-tables/GLOB0003";

# Output Parameters

$ENV{"SWS_LEVEL_1B_FILEPATH"} = "/seapac2/disk5/L1B/data";
$ENV{"SWS_OUTPUT_REPORT_PATH"} = "/seapac2/disk5/L1B/qarpt";
$ENV{"SWS_STATUSLOG_PATH"} = "/u/qscat/app-logs/pm/meta";
$ENV{"SWS_ERRLOG_FILESPEC"} = "/seapac2/disk5/L1B/log/L1B2013032005756-783809.log";

# Processing Parameters

$ENV{"SWS_NUM_PARTS"} = "1";
$ENV{"SWS_ECHO_TRACK_FLAG"} = "2";
$ENV{"SWS_REV_GRANULE_OVERLAP_TIME"} = "120";
$ENV{"SWS_PROCESS_ENVIRONMENT"} = "SeaPAC";
$ENV{"SWS_PARAM_FILESPEC"} = $0;
$ENV{"SWS_TASK_ID"} = "783809";

# Monitor Parameters

$ENV{"SWS_MON_SWS"} = "MON_L_OFF";
$ENV{"SWS_MON_L1B_LP"} = "MON_L_OFF";
$ENV{"SWS_MON_CONVERG"} = "MON_L_OFF";
$ENV{"SWS_MON_SIGMA0_AND_KP"} = "MON_L_OFF";
$ENV{"SWS_MON_CELL_GEOM"} = "MON_L_OFF";
$ENV{"SWS_MON_FRAME_PARAMS"} = "MON_L_OFF";
$ENV{"SWS_MON_ECHO_TRACK"} = "MON_L_OFF";

# PM Variables

$ENV{"PM_SERVER"} = "SPAC_INVENTORY";
$ENV{"PM_DB"} = "inventory";
$ENV{"PM_PMS"} = "PMSERVER";

# Call the executable to run the Level 1B Processor.

$ENV{"SWS_ERRMSG_DIR"} = "/u/qscat/app-res/gen";

system ("/u/qscat/bin/sparc/qs_lp_1B") == 0
    or die "Level 1B Processor terminated";

