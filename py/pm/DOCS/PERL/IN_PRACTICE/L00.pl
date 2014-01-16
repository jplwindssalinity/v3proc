#!/usr/bin/perl
#
# File:        L002012287012737-782006.pl
#
# Description: Process control script for the Level 0 Processor.
#
# Author:      Generated by the pm_startAutoProc application.
#
# Date:        10/13/2012 01:27:37.000
#

# Input Parameters

$ENV{"SWS_SPACECRAFT_L0_FILESPEC"} = "./qst20130131062807p62sci.dat";
$ENV{"SWS_TIME_CONVERSION_DATA_FILESPEC"} = "./tables/QS_SCLK_UTC";
$ENV{"SWS_LEAP_SECOND_FILESPEC"} = "./tables/LEAP0004";

# Output Parameters

$ENV{"SWS_LEVEL_0_FILEPATH"} = "./data";
$ENV{"SWS_OUTPUT_REPORT_PATH"} = "./qarpt";
$ENV{"SWS_STATUSLOG_PATH"} = "./meta";
$ENV{"SWS_ERRLOG_FILESPEC"} = "./log/L00.log";

# Processing Parameters

$ENV{"SWS_PARAM_FILESPEC"} = $0;
$ENV{"SWS_TASK_ID"} = "000000";

# Monitor Parameters

$ENV{"SWS_MON_SWS"} = "MON_L_LOW";
$ENV{"SWS_MON_L0_PP"} = "MON_L_OFF";

# PM Variables

$ENV{"PM_SERVER"} = "SPAC_INVENTORY";
$ENV{"PM_DB"} = "inventory";
$ENV{"PM_PMS"} = "PMSERVER";

# Call the executable to run the Level 0 Processor.

$ENV{"SWS_ERRMSG_DIR"} = "./gen";

system ("qs_pp_L0") == 0
    or die "Level 0 Preprocessor terminated";

