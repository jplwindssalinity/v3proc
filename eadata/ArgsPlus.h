//=========================================================
// Copyright  (C)1995, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.3   06 Apr 1998 16:26:34   sally
// merged with SVT
// 
//    Rev 1.2   27 Mar 1998 14:51:24   sally
// fixed some L1A_Derived stuff
// 
//    Rev 1.1   27 Mar 1998 09:57:22   sally
// added L1A Derived data
// 
//    Rev 1.0   04 Feb 1998 14:14:40   daffer
// Initial checking
// Revision 1.4  1998/02/03 00:12:33  sally
// change NRT to L1AP
//
// Revision 1.3  1998/01/30 22:28:06  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

#ifndef ARGSPLUS_H
#define ARGSPLUS_H

static const char rcs_args_plus_h[]="@(#) $Header$";

#include <stdio.h>

#include "Args.h"
#include "ArgDefs.h"
#include "Itime.h"
#include "Parameter.h"
#include "EALog.h"

class TlmFileList;
class FilterSet;
class ToDoList;
class LimitList;
class CmdList;
class EqxList;
class PolynomialTable;

//==========
// ArgsPlus 
//==========

class ArgsPlus : public Args
{
public:
  ArgsPlus(int argc, char** argv, char* config_filename,
	   ArgInfo* arg_info_array[]);
  ~ArgsPlus();
  
  SourceIdE       GetSourceIdOrExit(char* tlm_type_string);
  Itime           GetStartTime(char* start_time_string);
  Itime           GetStartTimeOrExit(char* start_time_string);
  Itime           GetEndTime(char* end_time_string);
  Itime           GetEndTimeOrExit(char* end_time_string);
  char*           GetTlmFilesOrExit(
				    char*           tlm_files_string,
				    SourceIdE       tlm_type,
				    char*           hkdt_files_string,
				    char*           l1_files_string,
				    char*           l1ap_files_string,
				    char*           l1adrv_files_string);
  TlmFileList*    TlmFileListOrExit(
			  SourceIdE tlm_type,
			  char*     tlm_files,
			  Itime     start_time = INVALID_TIME,
			  Itime     end_time = INVALID_TIME);
  FilterSet*      FilterSetOrNull(
				  SourceIdE   tlm_type,
				  char*       filters_string);
  FILE*           OutputFileOrStdout(char* output_filename);
  int             OutputFdOrStdout(char* output_filename);
  ToDoList*       ToDoListOrExit(const char* filename);
  char*           GetLimitFileOrExit(
		       char*     limit_file_string,
		       SourceIdE limit_type,
		       char*      hkdt_limit_file_string,
		       char*      l1_limit_file_string,
		       char*      l1ap_limit_file_string,
		       char*      l1adrv_limit_file_string);
  LimitList*      LimitListForCheckingOrExit(
				SourceIdE   tlm_type,
				const char* filename,
				FILE*       output_file_pointer);
  LimitList*      LimitListForUpdatingOrExit(
			       SourceIdE   tlm_type,
			       const char* filename,
			       FILE*       output_file_pointer);
  FILE*           AlertFileOrNull(const char* alert_filename);
  CmdList*        CmdlpOrExit(const char* cmdlp_filename);
  EqxList*        EqxListOrExit(const char* eqx_filename);
  void            MailAlertCheck(const char* mail_alert,
				 const char* alert_filename, 
				 const char* mail_address);
  
  PolynomialTable*  PolynomialTableOrNull(char*   polynomialFilename);
  EALog*            GetEALogOrExit(); 
  void              Usage();

  static PolynomialTable*  PolyTable;

private:

  void            _BadOptionCheck();
  LimitList*      _LimitListOrExit(SourceIdE tlm_type, 
				   const char* filename,
				   FILE* output_file_pointer, 
				   const int keep_disabled);
};

#endif
