//=========================================================
// Copyright  (C)1998, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.29   22 Sep 1998 17:50:40   daffer
// Corrected small problem in ~EALog::(stage_nonrtc_commands) section
// 
//    Rev 1.28   15 Sep 1998 09:44:18   daffer
// reset umask in destructor
// 
//    Rev 1.27   14 Sep 1998 17:14:18   daffer
// Set umask to 002 and make sure log file has permissions 664.
// 
//    Rev 1.26   21 Aug 1998 13:17:14   daffer
// fixed problem with reqi registration, cleanup generally.
// 
//    Rev 1.25   24 Jul 1998 11:33:56   daffer
// Moved calculation of os_id to top of Init_pm.
// 
//    Rev 1.24   08 Jun 1998 17:02:58   daffer
// put in file_status in file registration
// 
//    Rev 1.23   08 Jun 1998 14:36:26   daffer
// Cleaned up some problem in Reqq/Qpf/Rtcf file registration 
// and reporting of errors therefrom.
// 
//    Rev 1.22   08 Jun 1998 13:36:42   sally
// change realloc() to malloc() and copy manually
// 
//    Rev 1.21   05 Jun 1998 09:18:52   daffer
// Added BALL_SIS_ID to file registration for reqq/qpf files
// 
//    Rev 1.20   03 Jun 1998 15:47:36   daffer
// moved REQI registration to before PM initialization for process_reqi
// cleaned up REQQ/QPF/RTCF registration in ~EALog
// 
//    Rev 1.19   02 Jun 1998 17:14:42   daffer
// Put in error_log_filespec in process_reqi section of ~ealog
// 
//    Rev 1.18   29 May 1998 14:22:02   daffer
// added update_cmdlp, update_eqx, match_cmd_effects and detect_effects
// 
//    Rev 1.17   28 May 1998 09:26:30   sally
// update the formats for REQQ, QPF and RTCF
// 
//    Rev 1.16   27 May 1998 11:19:18   sally
// change new to malloc in _addOneFilename()
// 
//    Rev 1.15   27 May 1998 09:57:50   sally
// have AddOutputFilenames and AddInputFilenames call a common function
// 
//    Rev 1.14   19 May 1998 16:21:48   daffer
// Added RTCF and QPF file structs to reqq/reqi processing
// 
//    Rev 1.13   19 May 1998 14:36:54   daffer
// Changed GetUTC and robustified reqq/reqi processing
// 
//    Rev 1.12   28 Apr 1998 15:56:00   daffer
// Added data_type in REQI processing
// 
//    Rev 1.11   20 Apr 1998 14:21:32   sally
// make GNU CC happy
// 
//    Rev 1.10   20 Apr 1998 10:20:52   sally
// change for WindSwatch
// 
//    Rev 1.9   10 Apr 1998 14:04:14   daffer
//   Added AppendToInputFileList to add files to list of input files.
//   Added AppendToOutputFileList for analogous reason
//   Changed GetInputFileList to deal with the possibility that it won't be the 
//   first to the InputFiles variable.
//   Robustified initialization of InputFiles and OutputFiles variables
//   Changed WriteMsg(RunStatusE status) to deal with new status enum.
// 
//    Rev 1.8   23 Feb 1998 11:21:50   daffer
// Added process_completion_status to stopstruct
// 
//    Rev 1.7   23 Feb 1998 10:02:56   daffer
// Removed tabs
// 
//    Rev 1.6   20 Feb 1998 17:04:44   daffer
// Added method VWriteMsg, a variable number of arguments version of
// WriteMsg. Included CM_Control.h to get buid_id Set SPAC message Queue
// state to SPAC_OFF.  Added <progname>: <uid>: time to all message
// written out by any of the WriteMsg methods.
// 
// 
//    Rev 1.5   17 Feb 1998 14:45:30   sally
// NOPM
// 
//    Rev 1.4   05 Feb 1998 16:16:16   sally
// type cast
// 
//    Rev 1.3   05 Feb 1998 16:06:16   sally
// took out printf()
// 
//    Rev 1.2   05 Feb 1998 16:04:08   sally
// initialize the last InputFiles
// 
//    Rev 1.1   04 Feb 1998 17:24:26   daffer
// Used some temp variables to loop through InputFiles,
// added "qs_ea_" in front of filenames and improved
// some of the string handling.
// 
//
// Revision 1.5  1998/02/01 01:45:01  daffer
// *** empty log message ***
//
// Revision 1.4  1998/02/01 01:25:07  daffer
// Changed 'Final Program Status' output line - whd
//
// Revision 1.3  1998/01/30 22:29:01  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

#include <assert.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "EALog.h"

#if 0
#define EA_PM_LOG_DEST_DIR  "./"
#endif
#define EA_PM_LOG_DEST_DIR  "/u/qscat/app-logs/pm/meta/"

int errno;
mode_t _mask;

// Allocate space for the static variables.
EALog::RunStatusE     EALog::_Status;
EALog::LogFileStatusE EALog::LogFileStatus=EALog::LOG_UNKNOWN;
EALog::EA_PM_StatusE  EALog::EA_PM_Connectivity=EALog::EA_PM_NotConnected;
char           EALog::LogFileName[MAX_EAFILENAME_LEN];
char           EALog::ProgName[MAX_EAFILENAME_LEN];
//ofstream       EALog::LogStream;
FILE*          EALog::LogFD;

#ifndef NOPM
SPAC_MSGDESC  *EALog::msgDesc;
#endif

struct utsname EALog::uname_info;

static const char rcsid_EALog_C[] = "@(#) $Header$";

//
//
// =========== Constructor  ==============
//
//

EALog::EALog( char *program_name, char *logfile,
              int argc, char ** argv ) 
: InputFiles(0), nInputFiles(0), OutputFiles(0), nOutputFiles(0)
{

  // cout << " cout: In EALog constructor\n";
  // printf(" In EALog Constructor\n");
    _mask=umask(002);
    _blank="";
    _Status=EA_SUCCESS;
    groupd="EA";
    RunStatus_Start=EA_FAILURE;// The start of the enum, nothing more
    
    char* tempP=0;
    if ((tempP = strrchr(program_name, '/')))
        (void) strcpy( ProgName, ++tempP);
    else
        (void) strcpy( ProgName, program_name );
    
    _pid=getpid();
    _uid=getuid();
    
    if (LogFileStatus == LOG_UNKNOWN) {
        // Get the uname structure.
        (void )uname( &uname_info );
        cpu_start_time = clock();
        start_time     = time(0);
        tm_struct      = localtime(&start_time);
        (void) strftime( wall_clock_start_time, PADDED_UTC_TIME_LEN,
                         "%Y-%jT%H:%M:%S.000", tm_struct);
        
        if (logfile != NULL) 
            (void) strcpy( LogFileName, logfile );
        else {
            (void) strcat( LogFileName, "./");
            (void) strcat( LogFileName, program_name );
            (void) strcat( LogFileName, ".log");      
        }
        
        struct stat stat_buf;
        int retstat1=stat( LogFileName, &stat_buf );
        if (retstat1!=0){
            if (errno==ENOENT) {
                // File doesn't exist.
                int fd=creat( LogFileName, (mode_t) 0664);
                if (fd != -1) {
                  if ( (LogFD=fdopen(fd,"a+")) == NULL) {
                      fprintf(stderr,"Error open EA Log file %s\n",LogFileName);
                      LogFileStatus=LOG_FAILURE;
                      _Status=EA_FAILURE;
                      fprintf(stderr,"Logfile is NOT open\n");
                  } else {
                      LogFileStatus=LOG_OPEN;
                      WriteMsg("**** Starting *****\n" );
                      VWriteMsg("User ID: %d\n",_uid);
                      VWriteMsg("Process ID: %d\n",_pid);
                      VWriteMsg( "Wall Clock Start Time: %s\n\n",
                                 wall_clock_start_time);

                      sprintf( _CheckedBuf, "Arguments for this run are: \n" );
                      for (int i=0;i<argc;i++) {
                          strcat( _CheckedBuf, *(argv+i) );
                          strcat( _CheckedBuf, " " );
                      }
                      strcat( _CheckedBuf, "\n");
                      WriteMsg(_CheckedBuf);
                  }
                } else {
                    fprintf(stderr,"Can't creat Logfile %s\n",LogFileName);
                    fprintf(stderr,"Errno=%d\n",errno);
                    LogFileStatus=LOG_FAILURE;
                    _Status=EA_FAILURE;
                    fprintf(stderr,"Logfile is NOT open\n");

                }
            } else {
                // There's some problem other  than the file doesn't exist.
                fprintf(stderr,"Error 'stat'ing EA Log file %s\n",LogFileName);
                fprintf(stderr,"Errno = %d\n",stat);
                LogFileStatus=LOG_FAILURE;
                _Status=EA_FAILURE;
                fprintf(stderr,"Logfile is NOT open\n");
            }
        } else {
            // We can 'stat' the file. See if we own it. If we do, make sure 
            // it has the correct permissions (0642). If it doesn't, change them.
            uid_t uid=getuid();
            gid_t gid=getgid();
            if (stat_buf.st_uid == uid && 
                stat_buf.st_gid == gid) {
                mode_t mask=S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
                if ((stat_buf.st_mode & mask) != mask )  {
                    (void) chmod( LogFileName, (mode_t) 0664);
                }
            } 
            if ( (LogFD=fopen(LogFileName,"a+")) == NULL) {
                fprintf(stderr,"Error open EA Log file %s\n",LogFileName);
                LogFileStatus=LOG_FAILURE;
                _Status=EA_FAILURE;
                fprintf(stderr,"Logfile is NOT open\n");
                
            } else {
                LogFileStatus=LOG_OPEN;
                WriteMsg("**** Starting *****\n" );
                VWriteMsg("User ID: %d\n",_uid);
                VWriteMsg("Process ID: %d\n",_pid);
                VWriteMsg( "Wall Clock Start Time: %s\n\n",
                           wall_clock_start_time);
                
                sprintf( _CheckedBuf, "Arguments for this run are: \n" );
                for (int i=0;i<argc;i++) {
                    strcat( _CheckedBuf, *(argv+i) );
                    strcat( _CheckedBuf, " " );
                }
                strcat( _CheckedBuf, "\n");
                WriteMsg(_CheckedBuf);
            }
        }
        
    } else 
        _Status=EA_SUCCESS;
}; // EALog Constructor

//
//
// =========== EALog Destructor ===================
//
//

EALog::~EALog()
{
    _mask=umask(_mask);
#ifdef DEBUG
    printf("number of inputfiles = %d\n", nInputFiles);
    char** tempPtr = InputFiles;
    for (int k=0; k < nInputFiles; k++, tempPtr++)
    {
        if (*tempPtr)
            printf("input files = %s\n", *tempPtr);
    }
    
    printf("number of outputfiles = %d\n", nOutputFiles);
    tempPtr = OutputFiles;
    for (k=0; k < nOutputFiles; k++, tempPtr++)
    {
        if (*tempPtr)
            printf("output files = %s\n", *tempPtr);
    }
#endif //DEBUG

    cpu_end_time=clock();
    double elapsed_cpu_time = 
        ((double)(cpu_end_time-cpu_start_time))/CLOCKS_PER_SEC;
    long elapsed_wall_clock_secs = (long) difftime( time(NULL), start_time );
    
    
#ifndef NOPM
    int i;
    char **filePtr;
    if (EA_PM_Connectivity == EA_PM_Connected) {  
        if (strcmp(ProgName,"qs_ea_detect_effects") == 0) {
            pm_status=SPAC_OK;
        } else if (strcmp(ProgName,"qs_ea_error_check") == 0) {
            ErrorCheckStopStruct *stopstruct;
            stopstruct = &logStatusData.errorCheckStopStatus;
            stopstruct->status_type=ERRORCHKPROCSTOP;
            stopstruct->proc_type=ERRORCHKPROC;
            stopstruct->msg_type = PROCSTOPSTATUS;
            stopstruct->task_id =PMTASK_ID;
            stopstruct->cpu_time     = (long) elapsed_cpu_time;
            stopstruct->elapsed_time = elapsed_wall_clock_secs;
            stopstruct->process_completion_status = _Status;
            strcpy( stopstruct->errorlog_file_spec, LogFileName );
            pm_status = pm_logStatus( msgDesc, (StatusStruct*) stopstruct, 
                                      EA_PM_LOG_DEST_DIR );
            
        } else if (strcmp(ProgName,"qs_ea_extract") == 0) {
            ExtractStopStruct *stopstruct;
            stopstruct = &logStatusData.extractStopStatus;
            stopstruct->status_type=EXTRACTPROCSTOP;
            stopstruct->proc_type=EXTRACTPROC;
            stopstruct->msg_type = PROCSTOPSTATUS;
            stopstruct->task_id =PMTASK_ID;
            stopstruct->cpu_time     = (long) elapsed_cpu_time;
            stopstruct->elapsed_time = elapsed_wall_clock_secs;
            stopstruct->process_completion_status = _Status;
            strcpy( stopstruct->errorlog_file_spec, LogFileName );
            pm_status = pm_logStatus( msgDesc, (StatusStruct*) stopstruct, 
                                      EA_PM_LOG_DEST_DIR );
            
        } else if (strcmp(ProgName,"qs_ea_full_frame") == 0) {
            FullFrameStopStruct *stopstruct;
            stopstruct = &logStatusData.fullFrameStopStatus;
            stopstruct->status_type=FULLFRAMEPROCSTOP;
            stopstruct->proc_type=FULLFRAMEPROC;
            stopstruct->msg_type = PROCSTOPSTATUS;
            stopstruct->task_id =PMTASK_ID;
            stopstruct->cpu_time     = (long) elapsed_cpu_time;
            stopstruct->elapsed_time = elapsed_wall_clock_secs;
            stopstruct->process_completion_status = _Status;
            strcpy( stopstruct->errorlog_file_spec, LogFileName );
            pm_status = pm_logStatus( msgDesc, (StatusStruct*) stopstruct, 
                                      EA_PM_LOG_DEST_DIR);
        } else if (strcmp(ProgName,"qs_ea_gen_reqi") == 0) {
            pm_status=SPAC_OK;
        } else if (strcmp(ProgName,"qs_ea_match_cmd_effects") == 0) {
            pm_status=SPAC_OK;
        } else if (strcmp(ProgName,"qs_ea_process_reqi") == 0) {

            EAStopStruct *stopstruct;
            stopstruct = &logStatusData.eaStopStatus;
            stopstruct->status_type=EAPROCSTOP;
            stopstruct->proc_type=REQQPROC;
            stopstruct->msg_type = PROCSTOPSTATUS;
            stopstruct->task_id =PMTASK_ID;
            stopstruct->cpu_time     = (long) elapsed_cpu_time;
            stopstruct->elapsed_time = elapsed_wall_clock_secs;
            stopstruct->process_completion_status = _Status;
            strcpy( stopstruct->errorlog_file_spec, LogFileName );
            pm_status = pm_logStatus( msgDesc, (StatusStruct*) stopstruct, 
                                      EA_PM_LOG_DEST_DIR );
            if (pm_status < SPAC_WARNING) {
                WriteMsg(
                  "bad status from PM, clearing error: continuing\n");
                pm_status=SPAC_OK;
            }
            
            
            // Register the REQI file with PM
            if (_Status != EA_FAILURE ) {

                char time_str[PADDED_UTC_TIME_LEN+1];
                GetUTC(time_str);
                char fileNameArray[][5] = {"REQQ","QPF","RTCF"};
                int fileStatusArray[3]={SPAC_OK,SPAC_OK,SPAC_OK};

                filePtr = OutputFiles;

                for (i=0; i < nOutputFiles && filePtr && *filePtr;
                                  i++, filePtr++) {
                    char *ptr = strstr(*filePtr, "REQQ");

                    // If there is an REQQ file, register it with PM
                    if (ptr != NULL) {
                        REQQFileStruct *reqq;
                        reqq              = &logStatusData.reqqFileStatus;
                        reqq->status_type = REQQFILE;
                        reqq->task_id     = PMTASK_ID;
                        reqq->msg_type    = FILEMETADATA;
                        reqq->file_status = SPAC_OK;
                        (void) strcpy( reqq->output_file_spec, *filePtr);
                        (void) strcpy( reqq->data_type, REQQ_FILE_DATA_TYPE);
                        (void) strcpy( reqq->ProductionDateTime, time_str );
                        (void) strcpy( reqq->sis_id, BALL_SIS_ID);
                        pm_status = pm_logStatus( msgDesc, (StatusStruct*) reqq, 
                                                  EA_PM_LOG_DEST_DIR );
                        fileStatusArray[0]=pm_status;
                    }

                    // If there are any QPF file, register it with PM
                    ptr = strstr(*filePtr, "QPF");
                    if (ptr != NULL) {
                        SWPFileStruct *qpf;
                        qpf              = &logStatusData.swpFileStatus;
                        qpf->status_type = SWPFILE;
                        qpf->task_id     = PMTASK_ID;
                        qpf->msg_type    = FILEMETADATA;
                        qpf->file_status = SPAC_OK;
                        (void) strcpy( qpf->output_file_spec, *filePtr);
                        (void) strcpy( qpf->data_type, QPF_FILE_DATA_TYPE);
                        (void) strcpy( qpf->ProductionDateTime, time_str );
                        (void) strcpy( qpf->sis_id, BALL_SIS_ID);
                        pm_status = pm_logStatus( msgDesc, (StatusStruct*) qpf, 
                                                  EA_PM_LOG_DEST_DIR );
                        fileStatusArray[1]=pm_status;

                    }// Register any QPF files

                    // If there is an RTCF file, register it with PM
                    ptr = strstr( *filePtr, "RTCF");
                    if (ptr != NULL) {
                        RTCFileStruct *rtcf;
                        rtcf              = &logStatusData.rtcFileStatus;
                        rtcf->status_type = RTCFILE;
                        rtcf->task_id     = PMTASK_ID;
                        rtcf->msg_type    = FILEMETADATA;
                        rtcf->file_status = SPAC_OK;
                        (void) strcpy( rtcf->output_file_spec, *filePtr);
                        (void) strcpy( rtcf->data_type, RTCF_FILE_DATA_TYPE);
                        (void) strcpy( rtcf->ProductionDateTime, time_str );
                        pm_status = pm_logStatus( msgDesc, (StatusStruct*) rtcf, 
                                                  EA_PM_LOG_DEST_DIR );
                        fileStatusArray[2]=pm_status;
                    } // Register any RTCF files.

                } // Loop over output files.

                // Check status, report errors.
                for (int j=0;j<3;j++) {
                    if (fileStatusArray[j] < SPAC_WARNING) {
                        VWriteMsg(
                                  "Error registering %s with PM, clearing & continuing\n",
                                  fileNameArray[j] );
                        pm_status=SPAC_OK;
                    }
                }
            } // Test on EALog::_Status 
            

        } else if (strcmp(ProgName,"qs_ea_limit_check") == 0) {
            LimitCheckStopStruct *stopstruct;
            stopstruct = &logStatusData.limitCheckStopStatus;
            stopstruct->status_type=LIMITCHKPROCSTOP;
            stopstruct->proc_type=LIMITCHKPROC;
            stopstruct->msg_type = PROCSTOPSTATUS;
            stopstruct->task_id =PMTASK_ID;
            stopstruct->cpu_time     = (long) elapsed_cpu_time;
            stopstruct->elapsed_time = elapsed_wall_clock_secs;
            stopstruct->process_completion_status = _Status;
            strcpy( stopstruct->errorlog_file_spec, LogFileName );
            pm_status = pm_logStatus( msgDesc, (StatusStruct*) stopstruct, 
                                      EA_PM_LOG_DEST_DIR );

        } else if (strcmp(ProgName,"qs_ea_stage_nonrtc_commands") == 0) {
            if ( _Status != EA_FAILURE ) {

                REQQSendInfoStruct *eventstruct;
                eventstruct = &logStatusData.reqqSendInfoStatus;
                eventstruct->status_type=REQQSENDINFO;
                eventstruct->msg_type = MISSIONEVENT;
                eventstruct->task_id =PMTASK_ID;
                strcpy( eventstruct->reqq_file_spec, *InputFiles );
                char time_str[PADDED_UTC_TIME_LEN+1];
                (void) GetUTC(time_str);
                strcpy( eventstruct->send_time, time_str );
                eventstruct->send_status=_Status;
                pm_status = pm_logStatus( msgDesc, (StatusStruct*) eventstruct, 
                                          EA_PM_LOG_DEST_DIR );
                if (pm_status < SPAC_WARNING) {
                    WriteMsg(
                             "bad status from PM, clearing error: continuing\n");
                    pm_status=SPAC_OK;
                }
            }

        } else if (strcmp(ProgName,"qs_ea_update_cmdlp") == 0) {
            pm_status=SPAC_OK;
        } else if (strcmp(ProgName,"qs_ea_update_eqx") == 0) {
            pm_status=SPAC_OK;

        } else {
            // When all else failes, use Common struct.
            CommonStopStatusStruct *stopstruct;
            stopstruct = &logStatusData.commonStopStatus;
            stopstruct->status_type=COMMONSTOP;
            stopstruct->msg_type = PROCSTOPSTATUS;
            stopstruct->task_id =PMTASK_ID;
            stopstruct->cpu_time     = (long) elapsed_cpu_time;
            stopstruct->elapsed_time = elapsed_wall_clock_secs;
            stopstruct->process_completion_status = _Status;
            strcpy( stopstruct->errorlog_file_spec, LogFileName );
            pm_status = pm_logStatus( msgDesc, (StatusStruct*) stopstruct, 
                                      EA_PM_LOG_DEST_DIR );
            
        }
        
        if (pm_status < SPAC_WARNING) {
            fprintf(stderr, 
                    "Failure sending stop  data to PM system --- Aborting!\n");
            WriteMsg( 
                     "Failure sending stop data to PM system --- Aborting!\n");
            SetStatus( EA_FAILURE );
        }
    } // check on EA_PM_Connectivity
#endif
    
    VWriteMsg("CPU Time: %f\n",elapsed_cpu_time);
    VWriteMsg("Wall Clock: %d\n",elapsed_wall_clock_secs);
    
    char * EARunStatusS[] = { "EA_FAILURE",//Strings for use with WriteMsg.
                              "EA_ERROR",     
                              "EA_WARNING",
                              "EA_MSGFAIL",
                              "EA_SUCCESS" ,
                              "EA_INFO",
                              0
    };
    
    VWriteMsg( ": Final Program Status: %s\n\n", 
               EARunStatusS[_Status-RunStatus_Start]) ;
    WriteMsg("**** Done! *****\n" );
    fclose(LogFD);
    
}; // EALog Destructor


//
//
// ===================== EALog Methods ================
//
//
//

EALog::RunStatusE 
EALog::GetStatus()
{
    return (_Status);
};


void EALog::SetStatus(RunStatusE _status)
{
    _Status=_status;
};

void
EALog::WriteMsg(  char *stringbuf ) 
{
  // Writes message to Log file
    if (LogFileStatus) {
        time_t now=time(NULL);
        char *time= ctime( &now );
        int len=strlen(time);
        *(time+len-1)=(char) NULL;
        fprintf(LogFD,"<%s>: <%ld>: %s, %s",
                ProgName, _pid, time, EALog::CheckBuf( stringbuf ));
    }
    
};

void EALog::WriteMsg( const char *stringbuf ) 
{
  // Writes message to Log file
    if (LogFileStatus)  {
        time_t now = time(NULL);
        char *time = ctime( &now );
        int len=strlen(time);
        *(time+len-1)=(char) NULL;
        EALog::VWriteMsg( "<%s>: <%d>: %s, %s",
                          ProgName, _pid,time, stringbuf );
    }
};

void
EALog::WriteMsg(RunStatusE status ) 
{
    // Writes message to Log file
    char * EARunStatusS[] = { "EA_FAILURE",//Strings for use with WriteMsg.
                              "EA_ERROR",     
                              "EA_WARNING",
                              "EA_MSGFAIL",
                              "EA_SUCCESS" ,
                              "EA_INFO",
                              0
    };
    
    
    
    if (LogFileStatus) {
        time_t now=time(NULL); 
        char * time = ctime( &now );
        int len=strlen(time);
        *(time+len-1)=(char) NULL;
        EALog::VWriteMsg("<%s>: <%d>: %s, %s",
                         ProgName, _pid, time, EARunStatusS[status-RunStatus_Start]);
    }
    
};


void 
EALog::VWriteMsg( const char * fmt, ... ) {
    FILE* tmpFP;
    int nbytes;
    char *msg;
    va_list args;
    
    if (LogFileStatus) {
        
        if ((tmpFP=fopen("/dev/null","w")) == NULL ) {
            EALog::WriteMsg("Can't open tmpfile in VWriteMsg\n");
        } else {
            
            // Start the variable argument processing.
            va_start(args, fmt );
            nbytes=vfprintf( tmpFP, fmt, args) + 1;
            fclose( tmpFP );
            msg = new char [nbytes];
            if (msg) {
                vsprintf( msg, fmt, args );
                WriteMsg( msg );
                delete msg;
            } else 
                EALog::WriteMsg("Can't allocate buffer in VWriteMsg\n");
            va_end(args);
        }
    }
}; // end VWriteMsg.

void
EALog::SetAndWrite(RunStatusE status, char *stringbuf)
{
    // Sets the Status and Writes message to Log file
    EALog::SetStatus(status);
    if (LogFileStatus) 
        EALog::WriteMsg( stringbuf );
};// End EASetandWrite



void 
EALog::SetWriteAndExit(RunStatusE status, char *stringbuf){
            
            
    if (LogFileStatus) 
        EALog::SetAndWrite( status, stringbuf );
    EALog::SetStatus(status);
    int real_status=status;
    if (status == EA_SUCCESS || status == EA_INFO ) 
        real_status=0;
    else 
        real_status=1;
    exit (real_status);
    
};//end Set_WriteAndExit

char * EALog::CheckBuf( char * stringbuf ) {
    
    int ll=0;
    if (!stringbuf) {
        return _blank;
    } else {
        ll=strlen(stringbuf);
        if ( ll > MAX_EALOG_STRING_LEN-1) {
            ll=MAX_EALOG_STRING_LEN-1;
            char* p = new char[1024];
            int nchar = sprintf( p, 
                                 "strlen(string)>%d, too long! Trimming to %d chars\n",ll,ll );
            p[nchar]='\n';
            p[nchar+1]='\0';
            EALog::WriteMsg(p);
            strncpy( _CheckedBuf, stringbuf, ll );
        } else
            strncpy( _CheckedBuf, stringbuf, ll );
    }
    *(_CheckedBuf+ll)='\0';
    return _CheckedBuf;
}


// Initialize the PM interface.

EALog::RunStatusE
EALog::Init_PM()
{
    
    RunStatusE status=EA_SUCCESS;  
#ifndef NOPM
    int i;
    char **filePtr;
    char os_id[OS_ID_LEN+1];
    strcpy( os_id, uname_info.sysname );
    strcat( os_id, " ");
    strcat( os_id, uname_info.release );
    
    if (EA_PM_Connectivity==EALog::EA_PM_NotConnected) {
        msgDesc=spac_initMessage( ProgName, // current running program 
                                  groupd,   // Group identifier (EA)
                                  SPAC_ON,  // Write to stderr (on)
                                  SPAC_OFF,  // Write to Queue (off)
                                  LogFD );  // FILE* point to dump log
        if (msgDesc == NULL) {
            // System is hosed, log it and exit
            EALog::WriteMsg("NULL pointer returned by spac_initMessage\n");
            EALog::WriteMsg("Can't continue!!\n");
            status=EA_FAILURE;
        }
        
        int junk = pm_getNextTaskId( msgDesc, &PMTASK_ID );

        if (strcmp(ProgName,"qs_ea_detect_effects") == 0) {
            pm_status = SPAC_OK;
        } else if (strcmp(ProgName,"qs_ea_extract") == 0) {
            
            ExtractStartStruct *startstruct;
            startstruct = &logStatusData.extractStartStatus;
            startstruct->status_type=EXTRACTPROCSTART;
            startstruct->proc_type = EXTRACTPROC;
            startstruct->task_id  = PMTASK_ID;
            startstruct->msg_type = PROCSTARTSTATUS;
            strcpy( startstruct->os_id, os_id);
            strcpy( startstruct->node_id, uname_info.nodename );
            strcpy( startstruct->process_start_time, wall_clock_start_time );
            strcpy( startstruct->errorlog_file_spec, LogFileName );
            strcpy( startstruct->build_id, BUILD_ID );
            
            filePtr = InputFiles;
            for (i=0; i < nInputFiles && filePtr && *filePtr; i++, filePtr++) {
                (void)strcpy(startstruct->input_file_specs[i], *filePtr);
            }
            
            pm_status = pm_logStatus( msgDesc, (StatusStruct*) startstruct, 
                                      EA_PM_LOG_DEST_DIR);
            
            
        } else if (strcmp(ProgName,"qs_ea_error_check") == 0) {
            
            ErrorCheckStartStruct *startstruct;
            startstruct = &logStatusData.errorCheckStartStatus;
            startstruct->status_type=ERRORCHKPROCSTART;
            startstruct->proc_type = ERRORCHKPROC;
            startstruct->task_id  = PMTASK_ID;
            startstruct->msg_type = PROCSTARTSTATUS;
            strcpy( startstruct->os_id, os_id);
            strcpy( startstruct->node_id, uname_info.nodename );
            strcpy( startstruct->process_start_time, wall_clock_start_time );
            strcpy( startstruct->errorlog_file_spec, LogFileName );
            strcpy( startstruct->build_id, BUILD_ID );
            
            filePtr = InputFiles;
            for (i=0; i < nInputFiles && filePtr && *filePtr; i++, filePtr++) {
                (void)strcpy(startstruct->input_file_specs[i], *filePtr);
            }
            
            pm_status = pm_logStatus( msgDesc, (StatusStruct*) startstruct, 
                                      EA_PM_LOG_DEST_DIR);
            
            
        } else if (strcmp(ProgName,"qs_ea_full_frame") == 0) {
            
            FullFrameStartStruct *startstruct;
            startstruct = &logStatusData.fullFrameStartStatus;
            startstruct->status_type=FULLFRAMEPROCSTART;
            startstruct->proc_type = FULLFRAMEPROC;
            startstruct->task_id  = PMTASK_ID;
            startstruct->msg_type = PROCSTARTSTATUS;
            strcpy( startstruct->os_id, os_id);
            strcpy( startstruct->node_id, uname_info.nodename );
            strcpy( startstruct->process_start_time, wall_clock_start_time );
            strcpy( startstruct->errorlog_file_spec, LogFileName );
            strcpy( startstruct->build_id, BUILD_ID );
            
            filePtr = InputFiles;
            for (i=0; i < nInputFiles && filePtr && *filePtr; i++, filePtr++) {
                (void)strcpy(startstruct->input_file_specs[i], *filePtr);
            }
            
            pm_status = pm_logStatus( msgDesc, (StatusStruct*) startstruct, 
                                      EA_PM_LOG_DEST_DIR);
            
            
        } else if (strcmp(ProgName,"qs_ea_gen_reqi") == 0) {
            pm_status=SPAC_OK;

        } else if (strcmp(ProgName,"qs_ea_limit_check") == 0) {
            
            LimitCheckStartStruct *startstruct;
            startstruct = &logStatusData.limitCheckStartStatus;
            startstruct->status_type=LIMITCHKPROCSTART;
            startstruct->proc_type = LIMITCHKPROC;
            startstruct->task_id  = PMTASK_ID;
            startstruct->msg_type = PROCSTARTSTATUS;
            strcpy( startstruct->os_id, os_id);
            strcpy( startstruct->node_id, uname_info.nodename );
            strcpy( startstruct->process_start_time, wall_clock_start_time );
            strcpy( startstruct->errorlog_file_spec, LogFileName );
            strcpy( startstruct->build_id, BUILD_ID );
            
            filePtr = InputFiles;
            for (i=0; i < nInputFiles && filePtr && *filePtr; i++, filePtr++){
                (void)strcpy(startstruct->input_file_specs[i], *filePtr);
            }
            
            pm_status = pm_logStatus( msgDesc, (StatusStruct*) startstruct, 
                                      EA_PM_LOG_DEST_DIR);
            
        } else if (strcmp(ProgName,"qs_ea_match_cmd_effects") == 0){
            pm_status=SPAC_OK;
        } else if (strcmp(ProgName,"qs_ea_process_reqi") == 0) {

            char time_str[PADDED_UTC_TIME_LEN+1];
            GetUTC(time_str);
            if (*InputFiles != NULL) {
                // Register the REQI Input File.
                REQIFileStruct *reqi;
                filePtr = InputFiles;
                // Process_reqi always puts reqi file FIRST in list!
                char *PtrToReqiFilename=*filePtr; 
                filePtr++; // increment filePtr
                char *ptr = strstr(PtrToReqiFilename, "reqi");
                if (ptr == NULL) {
                    fprintf(stderr,
                            "Misnamed REQI file? (%s - no 'reqi' in name)\n", 
                            PtrToReqiFilename );
                    VWriteMsg("Misnamed REQI file? (%s - no 'reqi' in name)\n", 
                            PtrToReqiFilename );                              
                    SetWriteAndExit(EA_FAILURE," -- Aborting --\n");
                } else {
                    StatusStruct logStatusData2;
                    reqi              = &logStatusData2.reqiFileStatus;
                    reqi->task_id     = -1;
                    reqi->msg_type    = FILEMETADATA;
                    reqi->status_type = REQIFILE;
                    reqi->file_status = SPAC_OK;
                    (void) strcpy( reqi->output_file_spec, PtrToReqiFilename );
                    (void) strcpy( reqi->data_type,  REQI_FILE_DATA_TYPE);
                    (void) strcpy( reqi->ProductionDateTime, time_str );

                    pm_status = pm_logStatus( msgDesc, (StatusStruct*) reqi, 
                                              EA_PM_LOG_DEST_DIR );
                    if (pm_status < SPAC_WARNING) {
                        WriteMsg(
                           "Error registering REQI with PM , clearing & continuing\n");
                        pm_status=SPAC_OK;
                    } // Register REQI
                }
                EAStartStruct *startstruct;
                startstruct = &logStatusData.eaStartStatus;
                startstruct->status_type=EAPROCSTART;
                startstruct->proc_type = REQQPROC;
                startstruct->task_id  = PMTASK_ID;
                startstruct->msg_type = PROCSTARTSTATUS;
                strcpy( startstruct->os_id, os_id );
                strcpy( startstruct->node_id, uname_info.nodename );
                strcpy( startstruct->process_start_time, wall_clock_start_time );
                strcpy( startstruct->errorlog_file_spec, LogFileName );
                strcpy( startstruct->build_id, BUILD_ID );


                // When PM registers the REQI file, it copies it to a save
                // directory. It is the saved file which is registered,
                // not the original input file. So, we have to get the
                // path to that file. Soon PM will have a function that
                // will return that path, but in the mean time, we'll
                // construct it ourselves.

                // char path[1024],name[1024];
                char savedReqiName[1024];
                savedReqiName[0]='\0';
                char *pp =strrchr( PtrToReqiFilename, (int) '/');
                if (pp == NULL)
                    pp=PtrToReqiFilename;
                else
                    pp++;
                (void) strcpy( savedReqiName, "/pm/save/REQI/");
                (void) strcat( savedReqiName, pp );
                (void)strcpy(startstruct->input_file_specs[0], savedReqiName);
                for (i=1; i < nInputFiles && filePtr && *filePtr; i++, filePtr++){
                    (void)strcpy(startstruct->input_file_specs[i], *filePtr);
                }

                pm_status = pm_logStatus( msgDesc, (StatusStruct*) startstruct, 
                                          EA_PM_LOG_DEST_DIR);
                if (pm_status < SPAC_WARNING) {
                    WriteMsg(
                      "bad status initializing PM, clearing error: continuing\n");
                    pm_status=SPAC_OK;
                }
            } else {
                fprintf(stderr, "EALog: No Input REQI file!\n");
                SetWriteAndExit(EA_FAILURE,"No Input REQI file!\n");
            }
        } else if (strcmp(ProgName,"qs_ea_stage_nonrtc_commands") == 0) {
            pm_status=SPAC_OK;
        } else if (strcmp(ProgName,"qs_ea_update_cmdlp") == 0){
            pm_status=SPAC_OK;
        } else if (strcmp(ProgName,"qs_ea_update_eqx") == 0){
            pm_status=SPAC_OK;
        } else {
            
            CommonStartStatusStruct *startstruct;
            startstruct = &logStatusData.commonStartStatus;
            startstruct->status_type=COMMONSTART;
            startstruct->task_id  = PMTASK_ID;
            startstruct->msg_type = PROCSTARTSTATUS;
            strcpy( startstruct->os_id, os_id);
            strcpy( startstruct->node_id, uname_info.nodename );
            strcpy( startstruct->process_start_time, wall_clock_start_time );
            strcpy( startstruct->errorlog_file_spec, LogFileName );
            strcpy( startstruct->build_id, BUILD_ID );
            
            filePtr = InputFiles;
            for (i=0; i < nInputFiles && filePtr && *filePtr; i++, filePtr++){
                (void)strcpy(startstruct->input_file_specs[i], *filePtr);
            }
            
            pm_status = pm_logStatus( msgDesc, (StatusStruct*) startstruct, 
                                      EA_PM_LOG_DEST_DIR);
            
            
        }
        
        
        if (pm_status < SPAC_WARNING ) {
            fprintf(stderr, 
              "Can't connect to PM system \n");
            EALog::WriteMsg(
              "Can't connect to PM System \n");
            status=EA_FAILURE;
        } else {
            EA_PM_Connectivity = EA_PM_Connected;
        }
        
    }
#endif
    return (status);
}

void EALog::GetTlmFileList( TlmFileList* tlm_file_list ) 
{
    TlmHdfFile *tlmfile;
    char ** p;
    int n=0;
    for (tlmfile=tlm_file_list->GetHead(); tlmfile != NULL; 
         tlmfile=tlm_file_list->GetNext(), n++);
    if (n > 0) {
        if (nInputFiles > 0) {
            // Already some files in the input list.
            // Append the TLM files. 
            char **filePtr = InputFiles;
            nInputFiles+=n;
            InputFiles = new char * [nInputFiles+1];
            p=InputFiles;
            
            for (; *filePtr!=NULL; p++,filePtr++) 
                    *p = *filePtr;
            delete filePtr;
        } else {
            nInputFiles+=n;
            InputFiles = new char * [nInputFiles+1];
            p=InputFiles;
        }
        
        for (tlmfile=tlm_file_list->GetHead(); tlmfile != NULL; 
             tlmfile=tlm_file_list->GetNext(), p++ ) 
            *p = (char*)tlmfile->GetFileName();
        // make sure the last one is 0
        *p = 0;

    } else {
        fprintf(stderr,
                "%s: No Input TLM Files!... Aborting\n", ProgName);
        SetAndWrite(EA_FAILURE,ProgName);
        WriteMsg(": No Input Files!... Aborting\n");
    }
    
} // GetTlmFileList

void
EALog::AppendToInputFileList(
const char*    filename )
{
    if (filename)
    {
        if (_addOneFilename(filename, nInputFiles, InputFiles))
            nInputFiles++;
        else
        {
            fprintf(stderr,
              "%s: Append to Input File List failed!... Aborting\n", ProgName);
            SetAndWrite(EA_FAILURE,ProgName);
            WriteMsg(": Append to Input File List failed!... Aborting\n");
        }
    }
    return;
}// AppendToInputFileList


void
EALog::AppendToOutputFileList(
const char * filename )
{
    if (filename)
    {
        if (_addOneFilename(filename, nOutputFiles, OutputFiles))
            nOutputFiles++;
        else
        {
            fprintf(stderr,
              "%s: Append to Output File List failed!... Aborting\n", ProgName);
            SetAndWrite(EA_FAILURE,ProgName);
            WriteMsg(": Append to Output File List failed!... Aborting\n");
        }
    }
    return;
}// AppendToOutputFileList

int 
EALog::GetUTC(char * time_str) 
{   
    
    ttime = time(0);
    tm_struct      = localtime(&ttime);
    (void) strftime( time_str, PADDED_UTC_TIME_LEN+1,
                         "%Y-%jT%H:%M:%S.000", tm_struct);
    return (1);
} //GetTime

int
EALog::_addOneFilename(
const char*  newFilename,      // IN: the new filename to be added
int          currentNum,       // IN: current string number in the name list
char**&      filenamesPtr)     // IN/OUT: the name list itself
{
    assert(newFilename != 0 && currentNum >= 0);

    //-----------------------------------------------------------
    // this is the first one, do a new, else realloc the space
    //-----------------------------------------------------------
    if (filenamesPtr == 0)
    {
        if (currentNum != 0)
            return 0;
        else
        {
            filenamesPtr = (char**)malloc(sizeof(char**));
            assert(filenamesPtr != 0);
        }
    }
    else
    {
        char** newPtr = (char**)malloc((currentNum + 1) * (sizeof(char**)));
        assert(newPtr != 0);
        for (int i=0; i < currentNum; i++)
        {
            newPtr[i] = filenamesPtr[i];
        }
        free((char*)filenamesPtr);
        filenamesPtr = newPtr;
    }
    char** tail = filenamesPtr + currentNum;
    *tail = new char[MAX_EAFILENAME_LEN];
    (void) strncpy(*tail, newFilename, MAX_EAFILENAME_LEN - 1);
    (*tail)[MAX_EAFILENAME_LEN - 1] = '\0';
    return 1;

} // EALog::_addOneFilenameSpace
