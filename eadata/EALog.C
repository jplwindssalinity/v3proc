//=========================================================
// Copyright  (C)1998, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
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

#include "EALog.h"
#include <time.h>

#if 0
#define EA_PM_LOG_DEST_DIR  "./"
#endif
#define EA_PM_LOG_DEST_DIR  "/u/qscat/app-logs/pm/meta/"


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
{

  // cout << " cout: In EALog constructor\n";
  // printf(" In EALog Constructor\n");
    nInputFiles=0;
    InputFiles= new char *[1];
    *InputFiles=0;
    nOutputFiles=0;
    OutputFiles=new char *[1];
    *OutputFiles=0;
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
        (void) strftime( wall_clock_start_time, PADDED_UTC_TIME_LEN+1,
                         "%Y-%jT%H:%M:%S.000", tm_struct);
        
        if (logfile != NULL) 
            (void) strcpy( LogFileName, logfile );
        else {
            (void) strcat( LogFileName, "./");
            (void) strcat( LogFileName, program_name );
            (void) strcat( LogFileName, ".log");      
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
            VWriteMsg( "Wall Clock Start Time: %s\n\n",wall_clock_start_time);
            
            sprintf( _CheckedBuf, "Arguments for this run are: \n" );
            for (int i=0;i<argc;i++) {
                strcat( _CheckedBuf, *(argv+i) );
                strcat( _CheckedBuf, " " );
            }
            strcat( _CheckedBuf, "\n");
            WriteMsg(_CheckedBuf);
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
    cpu_end_time=clock();
    double elapsed_cpu_time = 
        ((double)(cpu_end_time-cpu_start_time))/CLOCKS_PER_SEC;
    long elapsed_wall_clock_secs = (long) difftime( time(NULL), start_time );
    
    
#ifndef NOPM
    int i;
    char **filePtr;
    if (EA_PM_Connectivity == EA_PM_Connected) {  
        if (strcmp(ProgName,"qs_ea_extract") == 0) {
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
            
        } else if (strcmp(ProgName,"qs_ea_stage_nonrtc_commands") == 0) {

            REQQSendInfoStruct *eventstruct;
            eventstruct = &logStatusData.reqqSendInfoStatus;
            eventstruct->status_type=REQQSENDINFO;
            eventstruct->msg_type = MISSIONEVENT;
            eventstruct->task_id =PMTASK_ID;
            // strcpy( eventstruct->errorlog_file_spec, LogFileName );
            strcpy( eventstruct->reqq_file_spec, *InputFiles );
            // char *time_str=GetUTC();
            // strcpy( eventstruct->drn_send_date, time_str );
            pm_status = pm_logStatus( msgDesc, (StatusStruct*) eventstruct, 
                                      EA_PM_LOG_DEST_DIR );
            if (pm_status < SPAC_WARNING) {
                WriteMsg(
                  "bad status from PM, clearing error: continuing\n");
                pm_status=SPAC_OK;
            }

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
            //            strcpy( stopstruct->errorlog_file_spec, LogFileName );
            pm_status = pm_logStatus( msgDesc, (StatusStruct*) stopstruct, 
                                      EA_PM_LOG_DEST_DIR );
            if (pm_status < SPAC_WARNING) {
                WriteMsg(
                  "bad status from PM, clearing error: continuing\n");
                pm_status=SPAC_OK;
            }
            
            
            // Register the REQI file with PM
            REQIFileStruct *reqi;
            REQQFileStruct *reqq;
            reqi= &logStatusData.reqiFileStatus;
            filePtr = InputFiles;
            for (i=0; *filePtr != NULL && i<MAX_INPUT_FILES; i++, filePtr++){
                (void)strcpy(&(reqi->output_file_spec[i]), *filePtr);
            }
            reqi->task_id=PMTASK_ID;
            reqi->msg_type=FILEMETADATA;
            reqi->status_type=REQIFILE;
            char* time_str=GetUTC();
            (void) strcpy( reqi->ProductionDateTime, time_str );
            
            pm_status = pm_logStatus( msgDesc, (StatusStruct*) reqi, 
                                      EA_PM_LOG_DEST_DIR );
            if (pm_status < SPAC_WARNING) {
                WriteMsg(
                  "bad status from PM registering REQI, clearing error & continuing\n");
                pm_status=SPAC_OK;
            }
            
            
            // Register the REQQ file just made with PM
            reqq= &logStatusData.reqqFileStatus;
            filePtr = OutputFiles;
            for (i=0; *filePtr != NULL && i<MAX_OUTPUT_FILES; i++, filePtr++){
                (void)strcpy(&(reqq->output_file_spec[i]), *filePtr);
            }
            reqq->status_type=REQQFILE;
            reqq->task_id=PMTASK_ID;
            reqq->msg_type=FILEMETADATA;
            (void) strcpy( reqq->ProductionDateTime, time_str );
            pm_status = pm_logStatus( msgDesc, (StatusStruct*) reqq, 
                                      EA_PM_LOG_DEST_DIR );
            if (pm_status < SPAC_WARNING) {
                WriteMsg(
                  "bad status from PM registering REQQ, clearing error & continuing\n");
                pm_status=SPAC_OK;
            }
            

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
            
        } else if (strcmp(ProgName,"qs_ea_gen_reqi") == 0) {
            pm_status=SPAC_OK;
        } else {
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
        
        
        if (strcmp(ProgName,"qs_ea_extract") == 0) {
            
            ExtractStartStruct *startstruct;
            startstruct = &logStatusData.extractStartStatus;
            startstruct->status_type=EXTRACTPROCSTART;
            startstruct->proc_type = EXTRACTPROC;
            startstruct->task_id  = PMTASK_ID;
            startstruct->msg_type = PROCSTARTSTATUS;
            strcpy( startstruct->os_id, uname_info.sysname );
            strcat( startstruct->os_id, " ");
            strcat( startstruct->os_id, uname_info.release );
            strcpy( startstruct->node_id, uname_info.nodename );
            strcpy( startstruct->process_start_time, wall_clock_start_time );
            strcpy( startstruct->errorlog_file_spec, LogFileName );
            strcpy( startstruct->build_id, BUILD_ID );
            
            filePtr = InputFiles;
            for (i=0; *filePtr != NULL && i < MAX_INPUT_FILES; i++, filePtr++) {
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
            strcpy( startstruct->os_id, uname_info.sysname );
            strcat( startstruct->os_id, " ");
            strcat( startstruct->os_id, uname_info.release );
            strcpy( startstruct->node_id, uname_info.nodename );
            strcpy( startstruct->process_start_time, wall_clock_start_time );
            strcpy( startstruct->errorlog_file_spec, LogFileName );
            strcpy( startstruct->build_id, BUILD_ID );
            
            filePtr = InputFiles;
            for (i=0; *filePtr != NULL && i < MAX_INPUT_FILES; i++, filePtr++) {
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
            strcpy( startstruct->os_id, uname_info.sysname );
            strcat( startstruct->os_id, " ");
            strcat( startstruct->os_id, uname_info.version );
            strcpy( startstruct->node_id, uname_info.nodename );
            strcpy( startstruct->process_start_time, wall_clock_start_time );
            strcpy( startstruct->errorlog_file_spec, LogFileName );
            strcpy( startstruct->build_id, BUILD_ID );
            
            filePtr = InputFiles;
            for (i=0; *filePtr != NULL && i < MAX_INPUT_FILES; i++, filePtr++) {
                (void)strcpy(startstruct->input_file_specs[i], *filePtr);
            }
            
            pm_status = pm_logStatus( msgDesc, (StatusStruct*) startstruct, 
                                      EA_PM_LOG_DEST_DIR);
            
            
        } else if (strcmp(ProgName,"qs_ea_limit_check") == 0) {
            
            LimitCheckStartStruct *startstruct;
            startstruct = &logStatusData.limitCheckStartStatus;
            startstruct->status_type=LIMITCHKPROCSTART;
            startstruct->proc_type = LIMITCHKPROC;
            startstruct->task_id  = PMTASK_ID;
            startstruct->msg_type = PROCSTARTSTATUS;
            strcpy( startstruct->os_id, uname_info.sysname );
            strcat( startstruct->os_id, " ");
            strcat( startstruct->os_id, uname_info.release );
            strcpy( startstruct->node_id, uname_info.nodename );
            strcpy( startstruct->process_start_time, wall_clock_start_time );
            strcpy( startstruct->errorlog_file_spec, LogFileName );
            strcpy( startstruct->build_id, BUILD_ID );
            
            filePtr = InputFiles;
            for (i=0; *filePtr != NULL && i < MAX_INPUT_FILES; i++, filePtr++){
                (void)strcpy(startstruct->input_file_specs[i], *filePtr);
            }
            
            pm_status = pm_logStatus( msgDesc, (StatusStruct*) startstruct, 
                                      EA_PM_LOG_DEST_DIR);
            
        } else if (strcmp(ProgName,"qs_ea_process_reqi") == 0) {
            
            EAStartStruct *startstruct;
            startstruct = &logStatusData.eaStartStatus;
            startstruct->status_type=EAPROCSTART;
            startstruct->proc_type = REQQPROC;
            startstruct->task_id  = PMTASK_ID;
            startstruct->msg_type = PROCSTARTSTATUS;
            strcpy( startstruct->os_id, uname_info.sysname );
            strcat( startstruct->os_id, " ");
            strcat( startstruct->os_id, uname_info.release );
            strcpy( startstruct->node_id, uname_info.nodename );
            strcpy( startstruct->process_start_time, wall_clock_start_time );
            strcpy( startstruct->errorlog_file_spec, LogFileName );
            strcpy( startstruct->build_id, BUILD_ID );
            
            filePtr = InputFiles;
            for (i=0; *filePtr != NULL && i < MAX_INPUT_FILES; i++, filePtr++){
                (void)strcpy(startstruct->input_file_specs[i], *filePtr);
            }
            
            pm_status = pm_logStatus( msgDesc, (StatusStruct*) startstruct, 
                                      EA_PM_LOG_DEST_DIR);
        } else if (strcmp(ProgName,"qs_ea_gen_reqi") == 0) {
            pm_status=SPAC_OK;
        } else if (strcmp(ProgName,"qs_ea_stage_nonrtc_commands") == 0) {
            pm_status=SPAC_OK;
        } else {
            
            CommonStartStatusStruct *startstruct;
            startstruct = &logStatusData.commonStartStatus;
            startstruct->status_type=COMMONSTART;
            startstruct->task_id  = PMTASK_ID;
            startstruct->msg_type = PROCSTARTSTATUS;
            strcpy( startstruct->os_id, uname_info.sysname );
            strcat( startstruct->os_id, " ");
            strcat( startstruct->os_id, uname_info.release );
            strcpy( startstruct->node_id, uname_info.nodename );
            strcpy( startstruct->process_start_time, wall_clock_start_time );
            strcpy( startstruct->errorlog_file_spec, LogFileName );
            strcpy( startstruct->build_id, BUILD_ID );
            
            filePtr = InputFiles;
            for (i=0; *filePtr != NULL && i < MAX_INPUT_FILES; i++, filePtr++){
                (void)strcpy(startstruct->input_file_specs[i], *filePtr);
            }
            
            pm_status = pm_logStatus( msgDesc, (StatusStruct*) startstruct, 
                                      EA_PM_LOG_DEST_DIR);
            
            
        }
        
        
        // Do stuff common to all startstructs here.
        if (pm_status < SPAC_WARNING ) {
            fprintf(stderr, 
              "Can't connect to PM system --- Aborting!\n");
            EALog::WriteMsg(
              "Can't connect to PM System --- Aborting!\n");
            status=EA_FAILURE;
        } else {
            EA_PM_Connectivity = EA_PM_Connected;
        }
        
    }
#endif
    return (status);
}

void EALog::GetInputFileList( TlmFileList* tlm_file_list ) 
{
    TlmHdfFile *tlmfile;
    char ** p;
    int n=0;
    for (tlmfile=tlm_file_list->GetHead(); tlmfile != NULL; 
         tlmfile=tlm_file_list->GetNext(), n++);
    if (n > 0) {
        if (nInputFiles > 0) {
            // Already some files in the input list.
            // Append the TLM files. (We could call AppendToInputList for 
            // each file, but that would be slower than duplicating 
            // it's code here
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
    
} // GetInputFileList

void
EALog::AppendToInputFileList(
const char*    filename )
{
    if (filename) {
        char **filePtr = InputFiles;
        nInputFiles+=1;
        InputFiles = new char * [nInputFiles+1];
        char **p=InputFiles;
        
        if (nInputFiles>1) {
            for (; *filePtr!=NULL; p++,filePtr++) 
                *p = *filePtr;
        }
        *p=new char[MAX_EAFILENAME_LEN];
        (void) strcpy( *p, filename );
        p++;
        // make sure the last one is 0
        *p = 0;
        delete filePtr;
        
    }
    
}// AppendToInputFileList


void
EALog::AppendToOutputFileList(
const char * filename )
{
    if (filename) {
        char **filePtr = OutputFiles;
        nOutputFiles+=1;
        OutputFiles = new char * [nOutputFiles+1];
        char **p=OutputFiles;
        
        if (nOutputFiles>1) {
            for (; *filePtr!=NULL; p++,filePtr++) 
                *p = *filePtr;
        }
        *p=new char[MAX_EAFILENAME_LEN];
        (void) strcpy( *p, filename );
        p++;
        // make sure the last one is 0
        *p = 0;
        delete filePtr;
        
    }
    
}// AppendToOutputFileList

char * 
EALog::GetUTC() 
{   
    
    char time_str[PADDED_UTC_TIME_LEN+1];
    ttime = time(0);
    tm_struct      = localtime(&ttime);
    (void) strftime( time_str, PADDED_UTC_TIME_LEN+1,
                         "%Y-%jT%H:%M:%S.000", tm_struct);
    return (time_str);
} //GetTime



