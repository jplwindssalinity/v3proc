//=========================================================
// Copyright  (C)1998, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.6   20 Apr 1998 10:21:14   sally
// change for WindSwatch
// 
//    Rev 1.5   10 Apr 1998 14:04:16   daffer
//   Changed status codes to conform to (odd) PM/SEAPAC standard
//   Added internal variables nInputFiles, OutputFiles, nOutputFiles
// 
//    Rev 1.4   23 Feb 1998 10:03:04   daffer
// Removed tabs
// 
//    Rev 1.3   20 Feb 1998 17:04:46   daffer
// Added method VWriteMsg, a variable number of arguments version of
// WriteMsg. Included CM_Control.h to get buid_id Set SPAC message Queue
// state to SPAC_OFF.  Added <progname>: <uid>: time to all message
// written out by any of the WriteMsg methods.
// 
// 
//    Rev 1.2   17 Feb 1998 14:45:36   sally
// NOPM
// 
//    Rev 1.1   05 Feb 1998 11:16:30   daffer
// no change
// 
//    Rev 1.0   04 Feb 1998 14:15:14   daffer
// Initial checking
// Revision 1.2  1998/01/30 22:28:16  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

#ifndef EALOG_H
#define EALOG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>
#include <time.h>
#include <stdarg.h>
#include <sys/types.h>
#include <unistd.h>
//#include <iostream.h>
//#include <fstream.h>

static const char rcsid_EALog_h[] = "@(#) $Header$";
#define MAX_EALOG_STRING_LEN 1024
#define MAX_EAFILENAME_LEN 256
#define MAXVARARGS 31
#define MAX_OUTPUT_FILES 30

// Non-system include files

#include "TlmFileList.h"
#include "TlmHdfFile.h"
                                                      
                                                      
#ifndef NOPM
// PM Log Status  include file.
#include "pm_logStatusClient.h" 
#include "pm_getNextTaskId.h"
#include "CM_Control.h"
#endif

#ifndef BUILD_ID
#define BUILD_ID "Private_1.1.0"
#endif
        
#ifndef PADDED_UTC_TIME_LEN
#define PADDED_UTC_TIME_LEN 21
#endif

//#include <iostream.h>
//#include <fstream.h>

class EALog {

public:

  // Run Status of EA Programs being logged by EALog.
    //    enum RunStatusE { 
    //  EA_FAILURE=-3,
    //  EA_ERROR,     
    //  EA_WARNING,
    //  EA_SUCCESS 
    // };
    // Have to use PM's error status'
    enum RunStatusE {
        EA_FAILURE=-3,
        EA_ERROR,
        EA_WARNING,
        EA_MSGFAIL,
        EA_SUCCESS,
        EA_INFO 
    };
        
    
    enum EA_PM_StatusE { EA_PM_NotConnected,
                         EA_PM_Connected };
    
    enum LogFileStatusE { // Status of EA Log File
        LOG_FAILURE,
        LOG_UNKNOWN,
        LOG_NOTOPEN,  
        LOG_OPEN };
    
    
    
    //Methods
    
    EALog( char *program_name, char *logfile, 
           int argc, char** argv) ; //constructor
    ~EALog();                                    //destructor
    RunStatusE GetStatus();                    
    RunStatusE Init_PM();
    void       SetStatus( RunStatusE status );                       
    void       WriteMsg( char * stringbuf );                         
    void       WriteMsg( const char * stringbuf );                   
    void       WriteMsg( RunStatusE status );
    void       VWriteMsg( const char * fmt,... );
    void       SetAndWrite( RunStatusE status, 
                            char *stringbuf );    
    void       SetWriteAndExit( RunStatusE status, 
                                char *stringbuf );
    void       GetInputFileList( TlmFileList* );
    void       AppendToInputFileList(const char* filename);
    void       AppendToOutputFileList(const char* filename);
    
private:
#ifndef NOPM
    static SPAC_MSGDESC *msgDesc;
#endif
    int RunStatus_Start;
    int pm_status;
    static char             LogFileName[MAX_EAFILENAME_LEN];
    static char             ProgName[MAX_EAFILENAME_LEN];
    //  static ofstream         LogStream;
    static FILE*            LogFD; // log file descriptor
    static RunStatusE       _Status;
    static LogFileStatusE   LogFileStatus;
    static EA_PM_StatusE    EA_PM_Connectivity;
    static struct utsname   uname_info;
    static char             *OutputFilename;
    
    
    char                    *CheckBuf(char *stringbuf);
    char                    *GetUTC();
    char                    *groupd;
    char*                   _blank;
    char                    _CheckedBuf[MAX_EALOG_STRING_LEN+1];
    
    clock_t                 cpu_start_time, cpu_end_time;
    double                  elapsed_time;
    char                    wall_clock_start_time[PADDED_UTC_TIME_LEN+1];
    struct  tm              *tm_struct;
    time_t                  start_time, ttime;
    int                     PMTASK_ID;
    pid_t                   _pid;
    uid_t                   _uid;
    char **                 InputFiles;
    int                     nInputFiles;
    char **                 OutputFiles;
    int                     nOutputFiles;
    
#ifndef NOPM
    StatusStruct            logStatusData; // PM Status structure
#endif
    
};// Class EALog

#endif //EALOG_H

