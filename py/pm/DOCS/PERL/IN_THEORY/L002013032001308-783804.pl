#!/usr/bin/perl
#
# File:        L002013032001308-783804.pl
#
# Description: Process control script for the Level 0 Processor.
#
# Author:      Generated by the pm_spacOps application.
#
# Date:        02/01/2013 00:13:08.000
#

# Input Parameters

$ENV{"SWS_SPACECRAFT_L0_FILESPEC"} = "/fetch/safs/L00/qst20130131213156p51sci.dat";
$ENV{"SWS_TIME_CONVERSION_DATA_FILESPEC"} = "/calctbl/hk2/time/QS_SCLK_UTC";
$ENV{"SWS_LEAP_SECOND_FILESPEC"} = "/stt-tables/LEAP0004";

# Output Parameters

$ENV{"SWS_LEVEL_0_FILEPATH"} = "/seapac2/disk5/L00/data";
$ENV{"SWS_OUTPUT_REPORT_PATH"} = "/seapac2/disk5/L00/qarpt";
$ENV{"SWS_STATUSLOG_PATH"} = "/u/qscat/app-logs/pm/meta";
$ENV{"SWS_ERRLOG_FILESPEC"} = "/seapac2/disk5/L00/log/L002013032001308-783804.log";

# Processing Parameters

$ENV{"SWS_PARAM_FILESPEC"} = $0;
$ENV{"SWS_TASK_ID"} = "783804";

# Monitor Parameters

$ENV{"SWS_MON_SWS"} = "MON_L_LOW";
$ENV{"SWS_MON_L0_PP"} = "MON_L_OFF";

# PM Variables

$ENV{"PM_SERVER"} = "SPAC_INVENTORY";
$ENV{"PM_DB"} = "inventory";
$ENV{"PM_PMS"} = "PMSERVER";

# Call the executable to run the Level 0 Processor.

$ENV{"SWS_ERRMSG_DIR"} = "/u/qscat/app-res/gen";

system ("/u/qscat/bin/sparc/qs_pp_L0") == 0
    or die "Level 0 Preprocessor terminated";
