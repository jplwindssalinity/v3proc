#!/usr/bin/perl
#
# File:        L2A2013032011929-783813.pl
#
# Description: Process control script for the Level 2A Processor.
#
# Author:      Generated by the pm_startAutoProc application.
#
# Date:        02/01/2013 01:19:30.000
#

# Input Parameters

$ENV{"SWS_L2A_CONFIG_FILESPEC"} = "/u/qscat/app-res/QS_PC2A0005.PULS";
$ENV{"SWS_L2A_METACONFIG_FILESPEC"} = "/u/qscat/app-res/QS_MC2A0001";
$ENV{"SWS_LEVEL_1B_FILESPEC"} = "/seapac2/disk5/L1B/data/QS_S1B70935.20130320058";

$ENV{"SWS_NUMBER_DATA_SKIPS"} = "0";

$ENV{"SWS_SEA_ICE_MAP_FILESPEC"} = "/calctbl/ice-map/data/NRT_ICEM2013031";
$ENV{"SWS_ATTENUATION_MAP_FILESPEC"} = "/stt-tables/ATTN0001";
$ENV{"SWS_LAND_SEA_MAP_FILESPEC"} = "/stt-tables/LMAP1111";
$ENV{"SWS_KPR_PARAMETERS_FILESPEC"} = "/stt-tables/QS_KPRP0002";
$ENV{"SWS_LEAP_SECOND_FILESPEC"} = "/stt-tables/LEAP0004";
$ENV{"SWS_L2A_CONSTANTS_FILESPEC"} = "/stt-tables/QS_CN2A2M30.PULS";
$ENV{"SWS_GLOBAL_CONSTANTS_FILESPEC"} = "/stt-tables/GLOB0003";

# Output Parameters

$ENV{"SWS_LEVEL_2A_FILEPATH"} = "/seapac2/disk5/L2A/data";
$ENV{"SWS_OUTPUT_REPORT_PATH"} = "/seapac2/disk5/L2A/qarpt";
$ENV{"SWS_STATUSLOG_PATH"} = "/u/qscat/app-logs/pm/meta";
$ENV{"SWS_ERRLOG_FILESPEC"} = "/seapac2/disk5/L2A/log/L2A2013032011929-783813.log";

# Processing Parameters

$ENV{"SWS_NUM_PARTS"} = "2";
$ENV{"SWS_WVC_ROW_OVERLAP"} = "78";
$ENV{"SWS_PARAM_FILESPEC"} = $0;
$ENV{"SWS_TASK_ID"} = "783813";

# Monitor Parameters

$ENV{"SWS_MON_SWS"} = "MON_L_OFF";
$ENV{"SWS_MON_L2A_LP"} = "MON_L_OFF";
$ENV{"SWS_MON_REGROUP_SIG"} = "MON_L_OFF";
$ENV{"SWS_MON_DETERMINE_ATTN"} = "MON_L_OFF";

# PM Variables

$ENV{"PM_SERVER"} = "SPAC_INVENTORY";
$ENV{"PM_DB"} = "inventory";
$ENV{"PM_PMS"} = "PMSERVER";

# Call the executable to run the Level 2A Processor.

$ENV{"SWS_ERRMSG_DIR"} = "/u/qscat/app-res/gen";

system ("/u/qscat/bin/sparc/qs_lp_2A") == 0
    or die "Level 2A Processor terminated";
