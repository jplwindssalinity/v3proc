//=========================================================
// Copyright  (C)1998, California Institute of Technology. 
// U.S. Government sponsorship under NASA contract
// NAS7-1260 is acknowledged.               
// 
// CM Log
// $Log$
// 
//    Rev 1.17   02 Jun 1999 16:23:20   sally
// add leap second adjustment
// 
//    Rev 1.16   10 May 1999 16:51:28   sally
// add "Append" option
// 
//    Rev 1.15   07 May 1999 13:14:32   sally
// 
//    Rev 1.14   12 Apr 1999 11:09:14   sally
// add options for statistics extraction
// 
//    Rev 1.13   01 Mar 1999 11:36:34   sally
// turn off log file report when NOPM is on
// 
//    Rev 1.12   02 Dec 1998 12:51:54   sally
// add option to suppress the printing of XMGR headers
// 
//    Rev 1.11   13 Oct 1998 15:36:54   sally
// add L1B file
// 
//    Rev 1.10   27 Jul 1998 14:01:28   sally
// passing polynomial table to extraction function
// 
//    Rev 1.9   29 May 1998 11:38:46   daffer
// Updated EALog initialization, modified exit() to ealog->SetWriteAndExit'
// 
//    Rev 1.8   10 Apr 1998 14:06:26   daffer
//   Added call to EALog::AppendToInputFileList for poly_table file.
// 
//    Rev 1.7   01 Apr 1998 13:39:20   sally
// use DrvParamList instead of ParameterList
// 
//    Rev 1.6   27 Mar 1998 10:06:06   sally
// added L1A Derived data
// 
//    Rev 1.5   23 Mar 1998 15:43:22   sally
// adapt to derived science data
// 
//    Rev 1.3   20 Feb 1998 11:04:02   sally
// L1 to L1A
// 
//    Rev 1.2   12 Feb 1998 17:05:02   sally
// add start and end time
// Revision 1.7  1998/02/03 00:20:59  sally
// change NRT to L1AP
//
// Revision 1.6  1998/01/31 00:18:25  daffer
// Added pvcs keywords, fixed header and rcs id string
//
// $Date$
// $Revision$
// $Author$
//
//=========================================================


//----------------------------------------------------------------------
// NAME
//      qs_ea_extract
//
// DESCRIPTION
//      The qs_ea_extract program extracts a parameter versus time (or versus
//      the x_parameter) from telemetry frames between start_time and
//      end_time and writes it to the output_filename in ASCII.  Format
//      commands for ACE/gr are provided at the beginning of the output
//      file.
//
// AUTHOR
//      James N. Huddleston
//
//----------------------------------------------------------------------

//-----------------------
// Configuration Control 
//-----------------------

static const char rcsid[] =
    "@(#) $Header$";

//----------
// INCLUDES 
//----------

#include <stdlib.h>
#include <string.h>

#include "Parameter.h"
#include "Itime.h"
#include "DrvParamList.h"
#include "ParTab.h"
#include "TlmFileList.h"
#include "TlmHdfFile.h"
#include "ArgsPlus.h"
#include "Application.h"

#include "Filter.h"
#include "PolyTable.h"
#include "LeapSecTable.h"
#include "EALog.h"
#include "SafeString.h"

//-----------
// CONSTANTS 
//-----------

//--------
// MACROS 
//--------

//------------------
// TYPE DEFINITIONS 
//------------------

//-----------------------
// FUNCTION DECLARATIONS 
//-----------------------

//------------------
// OPTION VARIABLES 
//------------------

//------------------
// GLOBAL VARIABLES 
//------------------

ArgInfo tlm_type_arg = TLM_TYPE_ARG;
ArgInfo tlm_files_arg = TLM_FILES_ARG;
ArgInfo l1a_files_arg = L1A_FILES_ARG;
ArgInfo hk2_files_arg = HK2_FILES_ARG;
ArgInfo l1ap_files_arg = L1AP_FILES_ARG;
ArgInfo l1adrv_files_arg = L1A_DERIVED_FILES_ARG;
ArgInfo l1b_files_arg = L1B_FILES_ARG;
ArgInfo y_parameters_arg = Y_PARAMETERS_ARG;
ArgInfo x_parameter_arg = X_PARAMETER_ARG;
ArgInfo start_time_arg = START_TIME_ARG;
ArgInfo end_time_arg = END_TIME_ARG;
ArgInfo filter_arg = FILTER_ARG;
ArgInfo output_file_arg = OUTPUT_FILE_ARG;
ArgInfo poly_table_arg = POLY_TABLE_ARG;
ArgInfo no_gr_header_arg = NO_GR_HEADER_ARG;
ArgInfo statistics_arg = STATISTICS_ARG;
ArgInfo use_avg_stat_arg = USE_AVG_STAT_ARG;
ArgInfo stat_num_frames_arg = STAT_NUM_FRAMES_ARG;
ArgInfo append_output_arg = APPEND_OUTPUT_ARG;
ArgInfo leap_second_table_arg = LEAP_SECOND_TABLE_ARG;

#ifndef NOPM
ArgInfo logfile_arg = LOG_FILE_ARG;
#endif

ArgInfo* arg_info_array[] =
{
    &tlm_type_arg,
    &tlm_files_arg,
    &y_parameters_arg,
    &x_parameter_arg,
    &start_time_arg,
    &end_time_arg,
    &filter_arg,
    &output_file_arg,
    &poly_table_arg,
    &statistics_arg,
    &use_avg_stat_arg,
    &stat_num_frames_arg,
#ifndef NOPM
    &logfile_arg,
#endif
    &no_gr_header_arg,
    &append_output_arg,
    &leap_second_table_arg,
    0
};

//--------------
// MAIN PROGRAM 
//--------------

int main(
int argc,
char *argv[])
{
    //-------------------
    // get the arguments 
    //-------------------

    char* config_filename = getenv(ENV_CONFIG_FILENAME);
    static ArgsPlus args_plus = ArgsPlus(
			       argc, argv, config_filename, arg_info_array);
#ifndef NOPM
    EALog *ealog=args_plus.GetEALog();
#endif

    if (argc == 1)
    {
        args_plus.Usage();
        exit(1);
    }

    char* tlm_type_string = args_plus.GetOrExit(tlm_type_arg);
    char* tlm_files_string = args_plus.Get(tlm_files_arg);
    char* l1a_files_string = args_plus.Get(l1a_files_arg);
    char* hk2_files_string = args_plus.Get(hk2_files_arg);
    char* l1ap_files_string = args_plus.Get(l1ap_files_arg);
    char* l1adrv_files_string = args_plus.Get(l1adrv_files_arg);
    char* l1b_files_string = args_plus.Get(l1b_files_arg);
    char* y_parameters_string = args_plus.GetOrExit(y_parameters_arg);
    char* x_parameter_string = args_plus.GetOrExit(x_parameter_arg);
    char* start_time_string = args_plus.Get(start_time_arg);
    char* end_time_string = args_plus.Get(end_time_arg);
    char* filter_string = args_plus.Get(filter_arg);
    char* output_file_string = args_plus.Get(output_file_arg);
    char* poly_table_string = args_plus.Get(poly_table_arg);
    char* leap_second_table_string = args_plus.Get(leap_second_table_arg);

    //-------------------
    // convert arguments 
    //-------------------
    args_plus.LeapSecTableOrExit(leap_second_table_string);

    SourceIdE tlm_type = args_plus.GetSourceIdOrExit(tlm_type_string);
    Itime start_time = args_plus.GetStartTime(start_time_string);
    Itime end_time = args_plus.GetEndTime(end_time_string);
    char* tlm_files = args_plus.GetTlmFilesOrExit(tlm_files_string, tlm_type,
                           hk2_files_string, l1a_files_string,
                           l1ap_files_string, l1adrv_files_string,
                           l1b_files_string);

    //---------------
    // use arguments 
    //---------------

    TlmFileList* tlm_file_list = args_plus.TlmFileListOrExit(tlm_type,
                                    tlm_files, start_time, end_time);

    char* append_output_string = args_plus.Get(append_output_arg);
    int append_output = 0;
    if (append_output_string && strcmp(append_output_string, "0") != 0)
        append_output = 1;
    FILE* output_fp = args_plus.OutputFileOrStdout(
                                         output_file_string, append_output);

    FilterSet* filter_set = args_plus.FilterSetOrNull(tlm_type, filter_string);
    PolynomialTable* polyTable = args_plus.PolynomialTableOrNull(
                                                      poly_table_string);
 
    char* no_gr_header_string = args_plus.Get(no_gr_header_arg);
    int no_gr_header = 0;
    if (no_gr_header_string && strcmp(no_gr_header_string, "0")!= 0)
        no_gr_header = 1;
 
    char* statistics_string = args_plus.Get(statistics_arg);
    int stat_extract = 0;
    if (statistics_string && strcmp(statistics_string, "0") != 0)
        stat_extract = 1;
 
    char* use_avg_stat_string = args_plus.Get(use_avg_stat_arg);
    int use_avg_stat = 0;
    if (use_avg_stat_string && strcmp(use_avg_stat_string, "0") != 0)
        use_avg_stat = 1;

    char* stat_num_frames_string = args_plus.Get(stat_num_frames_arg);
    int stat_num_frames = 1;
    if (stat_num_frames_string)
        stat_num_frames = atoi(stat_num_frames_string);

#ifndef NOPM
    ealog->AppendToInputFileList(leap_second_table_string);
    ealog->AppendToInputFileList(poly_table_string);
    ealog->AppendToOutputFileList(output_file_string);

    if (ealog->Init_PM() == EALog::EA_FAILURE)
        ealog->SetWriteAndExit(EALog::EA_FAILURE,
                 "Error connecting to PM --- Aborting ---\n");
#endif

    //----------------------------------------
    // look up x parameter in parameter table 
    //----------------------------------------

    Parameter *param=0;
    param = ParTabAccess::GetParameter(tlm_type, x_parameter_string);
    if (param == NULL)
    {
        fprintf(stderr, "%s: unknown x parameter %s\n", argv[0],
                x_parameter_string);
#ifndef NOPM
	    ealog->VWriteMsg(": unknown x parameter %s\n", x_parameter_string);
	    ealog->SetWriteAndExit(EALog::EA_FAILURE, "--- Aborting ---\n");
#endif
        }

    ParameterList* plist;
    if (tlm_type == SOURCE_L1A_DERIVED)
        plist = new DerivedParamList();
    else
        plist = new ParameterList();
    plist->Append(param);

    //-----------------------------------------
    // look up y parameters in parameter table 
    //-----------------------------------------
    char parse_string[BIG_SIZE];
    (void) strncpy(parse_string, y_parameters_string, BIG_SIZE - 1);
    parse_string[BIG_SIZE - 1] = '\0';
    char* lasts = 0;
    for (char* ystring = safe_strtok(parse_string, " ", &lasts); ystring;
                        ystring = safe_strtok(0, " ", &lasts))
    {
        param = ParTabAccess::GetParameter(tlm_type, ystring);

        if (param == NULL)
        {
            fprintf(stderr, "%s: error looking up parameter %s",
                                                   argv[0], ystring);
            fprintf(stderr, " in the %s parameter table\n",
                                             source_id_map[tlm_type]);
#ifndef NOPM
            ealog->VWriteMsg(
               "Error looking up parameter %s\n   in the %s parameter table\n",
                ystring, source_id_map[tlm_type]);
            ealog->SetWriteAndExit(EALog::EA_FAILURE,
                                   "---Aborting---\n");
#endif
        }
        plist->Append(param);
    }

    //------------------------
    // extract the parameters 
    //------------------------

    TlmFileList* tlm = tlm_file_list;

    for (TlmHdfFile* tlmFile=tlm->GetHead(); tlmFile;
                         tlmFile=tlm->GetNext())
    {
        if (plist->OpenParamDataSets(tlmFile) != ParameterList::OK)
        {
            fprintf(stderr, "%s: parameter list: open datasets failed\n",
                                  argv[0]);
#ifndef NOPM
            ealog->SetWriteAndExit(EALog::EA_FAILURE,
            "parameter list: open datasets failed -- Aborting -- \n");
#endif
        }

        if (filter_set)
        {
            // open the filter parameter datasets for this TLM file
            if (filter_set->OpenParamDataSets(tlmFile) == 0)
            {
                fprintf(stderr, "%s: filter set: open datasets failed\n",
                                      argv[0]);
#ifndef NOPM
                ealog->SetWriteAndExit(EALog::EA_FAILURE,
                                "parameter list: open datasets failed\n");
#endif
            }

            int32 nextIndex=HDF_FAIL;
            while(tlmFile->GetNextIndex(nextIndex) == HdfFile::OK)
            {
                if (filter_set->Pass(tlmFile, nextIndex))
                {
                    ParameterList::StatusE paramStatus =
                        plist->HoldExtract(tlmFile, nextIndex, polyTable);
                    if (paramStatus != ParameterList::OK &&
                        paramStatus != ParameterList::ERROR_EXTRACTING_NOTHING)
                    {
                        fprintf(stderr,
                            "%s: extracting parameter list failed\n", argv[0]);
#ifndef NOPM
                        ealog->SetWriteAndExit(EALog::EA_FAILURE,
                             "extracting parameter list failed\n");
#endif
                    }
                }
            }

            // ignore the closing error
            (void)filter_set->CloseParamDataSets(tlmFile);
        }
        else
        {
            int32 nextIndex=HDF_FAIL;
            while(tlmFile->GetNextIndex(nextIndex) == HdfFile::OK)
            {
                ParameterList::StatusE paramStatus =
                        plist->HoldExtract(tlmFile, nextIndex, polyTable);
                if (paramStatus != ParameterList::OK &&
                        paramStatus != ParameterList::ERROR_EXTRACTING_NOTHING)
                {
                    fprintf(stderr,
                        "%s: extracting parameter list failed\n", argv[0]);
#ifndef NOPM
                    ealog->SetWriteAndExit(EALog::EA_FAILURE,
                         "extracting parameter list failed\n");
#endif
                }
            }
        }

        // ignore the closing error
        (void)plist->CloseParamDataSets(tlmFile);
    }

    //----------------
    // delete objects 
    //----------------

    delete tlm_file_list;
    if (filter_set)
        delete filter_set;

    //------------------------
    // apply polynomial table 
    //------------------------
    if (polyTable == 0)
    {
        if (plist->NeedPolynomial())
        {
            fprintf(stderr, 
             "%s: Polynomial Table is required, aborting...\n",
                                     argv[0]);
#ifndef NOPM
	    ealog->SetWriteAndExit(EALog::EA_FAILURE,
             " Polynomial Table is required --- Aborting --- \n");
#endif
        }
    }
    else
    {
        ParameterList::StatusE paramListStatus =
                       plist->ApplyPolynomialTable(polyTable);
        if (paramListStatus != ParameterList::OK)
        {
            fprintf(stderr,
              "%s: Applying Polynomial Table failed, aborting...\n", 
                 argv[0]);
#ifndef NOPM
	    ealog->SetWriteAndExit(EALog::EA_FAILURE,
                    "Applying Polynomial Table failed. --- Aborting ---\n");
#endif
        }
        delete polyTable;
    }

    //------------------------
    // write the data to file 
    //------------------------

    char* comment = filter_string;
    plist->PrePrint();

    if (stat_extract)
    {
        if (plist->PrintStatisticsACEgr(output_fp, stat_num_frames,
                   use_avg_stat, start_time, end_time, comment,
                   no_gr_header) != ParameterList::OK)
        {
            fprintf(stderr, "%s: print output failed\n", argv[0]);
#ifndef NOPM
            ealog->SetWriteAndExit(EALog::EA_FAILURE,
                        "print output failed. --- Aborting ---\n");
#else
            exit(1);
#endif
        }
    }
    else
    {
        if (no_gr_header)
            plist->Print(output_fp);
        else
            plist->PrintACEgr(output_fp, start_time, end_time, comment);
    }

    delete plist;

    //-----------------------
    // close the output file 
    //-----------------------

    fclose(output_fp);

    return (0);
}
