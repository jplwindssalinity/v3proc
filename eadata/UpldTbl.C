//==================================================
//
// UpldTbl.C
// Purpose: Used in manipulating the uploaded tables
// Author:  William Daffer
//
// CM Log
//
// $Log$
// 
//    Rev 1.2   23 Jun 1999 11:54:52   sally
// fixed * = const*
// 
//    Rev 1.1   26 Apr 1999 15:50:00   sally
// port to GNU compiler
// 
//    Rev 1.0   25 Mar 1999 13:56:50   daffer
// Initial Revision
//
// $Date$
// $Revision$
// $Author$
//
//
//
//==================================================

static const char UpldTbl_C_rcsid[]="@(#) $Header$";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

#include "SafeString.h"
#include "ArgsPlus.h"
#if 0
#include "UpldTbl.h"
#endif
#include "Command.h"
#include "UpldTblList.h"

#define BIG_SIZE 1024
#define MAX_TABLE_MASK_LOCATIONS 10


typedef struct Ignorable {
    int location;
    int mask;
};

typedef struct TableCompareMask {
    EAUpldTbl_TypeE tabletype;
    int numlocations;
    Ignorable locations[MAX_TABLE_MASK_LOCATIONS];
};

const TableCompareMask TableCompareMaskMap[] = {
    {EA_TBLTYPE_SESALL, 
     2,
     { 
         {4,0xff00},
         {6,0xff00},
     }
    },
    {EA_TBLTYPE_UNKNOWN,0}
};

struct EAUpldTblCmd {
    EACommandE    CommandId;
    EffectE       EffectId;
    unsigned short CmdHex;
    char *         CmdMnemonic;
    
};

struct EAUpldTblMapEntry {
    EAUpldTbl_TypeE    tabletype;
    EffectE       finalEffectId;
    EAUpldTblCmd  uploadcmd;
    EAUpldTblCmd  switchcmd;
    
};

const char * PPMap[] = 
{ "Unknown", 
   "Active PRF External Standby",
    "Active PRF External WOM",
    "Active PRF External CAL",
    "Active PRF External RCV",
    "Active SES All",
    "Active SES Standby",
    "Active SES Wom",
    "Active SES Cal",
    "Active SES Receive",
    "Active SERDIGENGTLM",
    "Active SERDIGSTATLM",
    "Active DOPPLER A",
    "Active DOPPLER B",
    "Active RANGE A",
    "Active RANGE B",
    "CDS Constants",
    "Inactive PRF External STANDBY",
    "Inactive PRF External WOM",
    "Inactive PRF External CAL",
    "Inactive PRF External RCV",
    "Inactive SES All",
    "Inactive SES Standby",
    "Inactive SES Wom",
    "Inactive SES Cal",
    "Inactive SES Receive",
    "Inactive Serial Digital Eng TLM",
    "Inactive SERial DIGital Status TLM",
    "Inactive DOPPLER A",
    "Inactive DOPPLER B",
    "Inactive RANGE A",
    "Inactive RANGE B"

}; 
const EAUpldTblMapEntry TableMap[] = 
{ 
    { EA_TBLTYPE_PRFEXTSTANDBY,
      EFF_PRF_STANDBY_TABLE_FINAL,
      { EA_CMD_SCPSTTBL, EFF_PRF_STANDBY_TABLE_UPDATE, 0X55FA, "SCPSTTBL" },  
      { EA_CMD_SCPSTSW ,  EFF_PRF_STANDBY_TABLE_CHANGE, 0X0FEA, "SCPSTSW" }
    },
    { EA_TBLTYPE_PRFEXTWOM,
      EFF_PRF_WOM_TABLE_FINAL,
      { EA_CMD_SCPWOTBL, EFF_PRF_WOM_TABLE_UPDATE, 0X55FD, "SCPWOTBL" },  
      { EA_CMD_SCPWOSW , EFF_PRF_WOM_TABLE_CHANGE, 0X0FED, "SCPWOSW" }
    },
    { EA_TBLTYPE_PRFEXTCAL,
      EFF_PRF_CAL_TABLE_FINAL,
      { EA_CMD_SCPCLTBL, EFF_PRF_CAL_TABLE_UPDATE, 0X55FB, "SCPCLTBL" },  
      { EA_CMD_SCPCLSW , EFF_PRF_CAL_TABLE_CHANGE ,0X0FEB, "SCPCLSW" } 
    },
    { EA_TBLTYPE_PRFEXTRCV,
      EFF_PRF_RCV_TABLE_FINAL,
      { EA_CMD_SCPRCTBL, EFF_PRF_RCV_TABLE_UPDATE, 0X55FC, "SCPRCTBL" },  
      { EA_CMD_SCPRCSW , EFF_PRF_RCV_TABLE_CHANGE, 0X0FEC, "SCPRCSW" }
    },
    { EA_TBLTYPE_SESALL,
      EFF_SES_ALL_TABLE_FINAL,
      { EA_CMD_SCSALLTBL, EFF_SES_ALL_TABLE_UPDATE, 0X55CE, "SCSALLTBL" },
      { EA_CMD_SCSALLSW , EFF_SES_ALL_TABLE_CHANGE, 0X0FBE, "SCSALLSW" }
    },
    { EA_TBLTYPE_SESSTANDBY,
      EFF_SES_STANDBY_TABLE_FINAL,
      { EA_CMD_SCSSTTBL, EFF_SES_STANDBY_TABLE_UPDATE, 0X55CA, "SCSSTTBL" },  
      { EA_CMD_SCSSTSW , EFF_SES_STANDBY_TABLE_CHANGE, 0X0FBA, "SCSSTSW" }
    },
    { EA_TBLTYPE_SESWOM,
      EFF_SES_WOM_TABLE_FINAL,
      { EA_CMD_SCSWOTBL, EFF_SES_CAL_TABLE_UPDATE, 0X55CD, "SCSWOTBL" },  
      { EA_CMD_SCSWOSW , EFF_SES_CAL_TABLE_CHANGE, 0X0FBD, "SCSWOSW" }
    },
    { EA_TBLTYPE_SESCAL,
      EFF_SES_CAL_TABLE_FINAL,      
      { EA_CMD_SCSCLTBL, EFF_SES_ALL_TABLE_UPDATE, 0X55CB, "SCSCLTBL" },  
      { EA_CMD_SCSCLSW , EFF_SES_ALL_TABLE_CHANGE, 0X0FBB, "SCSCLSW" }
    },
    { EA_TBLTYPE_SESRCV,
      EFF_SES_RCV_TABLE_FINAL,
      { EA_CMD_SCSRCTBL, EFF_SES_RCV_TABLE_UPDATE, 0X55CC, "SCSRCTBL" },  
      { EA_CMD_SCSRCSW , EFF_SES_RCV_TABLE_CHANGE, 0X0FBF, "SCSRCSW" } 
    },
    { EA_TBLTYPE_SERDIGENGTLM,
      EFF_SER_DIG_ENG_TLM_TABLE_FINAL,
      { EA_CMD_SCENGTBL, EFF_SER_DIG_ENG_TLM_TABLE_UPDATE, 0X55F8, "SCENGTBL" },  
      { EA_CMD_SCENGSW , EFF_SER_DIG_ENG_TLM_TABLE_CHANGE, 0X0FE8, "SCENGSW" }
    },
    { EA_TBLTYPE_SERDIGSTATLM,
      EFF_SER_DIG_ST_TLM_TABLE_FINAL,
      { EA_CMD_SCSTATBL, EFF_SER_DIG_ST_TLM_TABLE_UPDATE, 0X55F9, "SCSTATBL" },  
      { EA_CMD_SCSTASW , EFF_SER_DIG_ST_TLM_TABLE_CHANGE, 0X0FE9, "SCSTASW"}
    },
    { EA_TBLTYPE_DOPPLER_A,
      EFF_DOPPLER_A_TABLE_FINAL,
      { EA_CMD_SCBDATBL, EFF_DOPPLER_A_TABLE_UPDATE, 0X55F2, "SCBDATBL" },  
      { EA_CMD_SCBDASW , EFF_DOPPLER_A_TABLE_CHANGE, 0X0FE2, "SCBDASW"}
    },
    { EA_TBLTYPE_DOPPLER_B,
      EFF_DOPPLER_B_TABLE_FINAL,
      { EA_CMD_SCBDBTBL, EFF_DOPPLER_B_TABLE_UPDATE, 0X55F3, "SCBDBTBL" },  
      { EA_CMD_SCBDBSW , EFF_DOPPLER_B_TABLE_CHANGE, 0X0FE3, "SCBDBSW"}
    },
    { EA_TBLTYPE_RANGE_A,
      EFF_RANGEGATE_A_TABLE_FINAL,
      { EA_CMD_SCBRATBL, EFF_RANGEGATE_A_TABLE_UPDATE, 0X55E6, "SCBRATBL" },  
      { EA_CMD_SCBRASW , EFF_RANGEGATE_A_TABLE_CHANGE, 0X0FD6, "SCBRASW"}
    },
    { EA_TBLTYPE_RANGE_B,
      EFF_RANGEGATE_B_TABLE_FINAL,
      { EA_CMD_SCBRBTBL, EFF_RANGEGATE_B_TABLE_UPDATE, 0X55E7, "SCBRBTBL" },  
      { EA_CMD_SCBRBSW , EFF_RANGEGATE_B_TABLE_CHANGE, 0X0FD7, "SCBRBSW"}
    },
    { EA_TBLTYPE_UNKNOWN, EFF_UNKNOWN,
      { EA_CMD_UNKNOWN, EFF_UNKNOWN, 0, "" },  
      { EA_CMD_UNKNOWN, EFF_UNKNOWN, 0, "" }  
    }
};

const char * UpldTblStatusStrings[] = 
{
    "Unknown",
    "Ok",
    "Error",
    "Table type Mismatch",
    "Num Words Mismatch",
    "Table ID Mismatch",
    "Unknowable QPX",
    "Init Error",
    "Error Parsing Filename",
    "File Open Error",
    "Error Reading File",
    "Memory Error",
    "Bad Time",
    "Bad Table Id",
    "Null Directory name",
    "Null File name",
    "Invalid Mnemonic in Qpx File",
    "Initialization of QPx list failed"
    };


QpxDirEntry **UpldTbl::_qpxlist=0;

//=========================
// Constructors
//=========================

UpldTbl::UpldTbl()
    :
    _file_creation_time(INVALID_TIME),_begin_date(INVALID_TIME),
    _end_date(INVALID_TIME),_command_date(INVALID_TIME),
    _switch_time(INVALID_TIME), _upload_time(INVALID_TIME),
    _first_seen(INVALID_TIME), _last_seen(INVALID_TIME),
    _isFile(0), _isQpa(1),_complete(0), _ignore_till_next_time(0),
    _defined(0),  _activity_state(ACT_UNKNOWN),_cmdHex(0), 
    _number(0),_tableid(EA_TBL_UNKNOWN), 
    _tabletype(EA_TBLTYPE_UNKNOWN), _table(NULL), 
    _filled(NULL), _uqpx_dir(NULL), _uqpx_filename(NULL),
    _filename(NULL), _directory(NULL),
    _qpf_directory(NULL), _qpa_directory(NULL), 
    _status(EA_UPLDTBL_OK), 
    _tot_num_entries(0), _num_entries(0)
{
    return;
}
    
UpldTbl::UpldTbl( EAUpldTbl_TypeE table_type, ActivityStateE activity )
    :
    _file_creation_time(INVALID_TIME),_begin_date(INVALID_TIME),
    _end_date(INVALID_TIME),_command_date(INVALID_TIME),
    _switch_time(INVALID_TIME), _upload_time(INVALID_TIME),
    _first_seen(INVALID_TIME), _last_seen(INVALID_TIME),
    _isFile(0), _isQpa(1),_complete(0), _ignore_till_next_time(0),
    _defined(0),  _activity_state(ACT_UNKNOWN),_cmdHex(0), 
    _number(0),_tableid(EA_TBL_UNKNOWN), 
    _tabletype(EA_TBLTYPE_UNKNOWN), _table(NULL), 
    _filled(NULL), _uqpx_dir(NULL), _uqpx_filename(NULL),
    _filename(NULL), _directory(NULL),
    _qpf_directory(NULL), _qpa_directory(NULL), 
    _status(EA_UPLDTBL_OK), 
    _tot_num_entries(0), _num_entries(0)

//     _file_creation_time(INVALID_TIME),_begin_date(INVALID_TIME),
//     _end_date(INVALID_TIME),_command_date(INVALID_TIME),
//     _switch_time(INVALID_TIME), _upload_time(INVALID_TIME),
//     _isFile(0), _isQpa(1),_complete(0), _ignore_till_next_time(0),
//     _defined(0),  _activity_state(ACT_UNKNOWN),_cmdHex(0), 
//     _filename(NULL), 
//     _directory(NULL),_number(0),_status(EA_UPLDTBL_OK), 
//     _qpf_directory(NULL), _qpa_directory(NULL), 
//     _tot_num_entries(0), _num_entries(0),_tableid(EA_TBL_UNKNOWN), 
//     _tabletype(EA_TBLTYPE_UNKNOWN), _table(NULL), 
//     _filled(NULL), _uqpx_dir(NULL), _uqpx_filename(NULL) 

{
    _activity_state=activity;
    _tabletype=table_type;
    _tot_num_entries=FindNumWords();
 
}

UpldTbl::UpldTbl(char *qpxfile, int load_table)
    :
    _file_creation_time(INVALID_TIME),_begin_date(INVALID_TIME),
    _end_date(INVALID_TIME),_command_date(INVALID_TIME),
    _switch_time(INVALID_TIME), _upload_time(INVALID_TIME),
    _first_seen(INVALID_TIME), _last_seen(INVALID_TIME),
    _isFile(0), _isQpa(1),_complete(0), _ignore_till_next_time(0),
    _defined(0),  _activity_state(ACT_UNKNOWN),_cmdHex(0), 
    _number(0),_tableid(EA_TBL_UNKNOWN), 
    _tabletype(EA_TBLTYPE_UNKNOWN), _table(NULL), 
    _filled(NULL), _uqpx_dir(NULL), _uqpx_filename(NULL),
    _filename(NULL), _directory(NULL),
    _qpf_directory(NULL), _qpa_directory(NULL), 
    _status(EA_UPLDTBL_OK), 
    _tot_num_entries(0), _num_entries(0)

//     _file_creation_time(INVALID_TIME),_begin_date(INVALID_TIME),
//     _end_date(INVALID_TIME),_command_date(INVALID_TIME),
//     _switch_time(INVALID_TIME), _upload_time(INVALID_TIME),
//     _isFile(0), _isQpa(1),_complete(0), _ignore_till_next_time(0),
//     _defined(0),  _activity_state(ACT_UNKNOWN),_cmdHex(0), 
//     _filename(NULL), 
//     _directory(NULL), _number(0),_status(EA_UPLDTBL_OK), 
//     _qpf_directory(NULL), _qpa_directory(NULL), 
//     _tot_num_entries(0), _num_entries(0),_tableid(EA_TBL_UNKNOWN), 
//     _tabletype(EA_TBLTYPE_UNKNOWN), _table(NULL), 
//     _filled(NULL), _uqpx_dir(NULL) ,_uqpx_filename(NULL) 

{

    _isFile=1;
    _isQpa = IsQpa(qpxfile);    
    _status=ReadQpx(qpxfile, load_table);
    return;
}



//=========================
// Destructors
//=========================


UpldTbl::~UpldTbl() {
    if (_table != NULL) free(_table);
    if (_filename != NULL) free( _filename);
    if (_directory != NULL) free( _directory);
//     if (_qpxlist != NULL ) {
//         int i=0;
//         while ( _qpxlist[i] ){
//             free (_qpxlist[i]->filename);
//             free (_qpxlist[i]);
//             i++;
//         }
//         free (_qpxlist);
    //    }
    if (_qpf_directory) free( _qpf_directory);
    if (_qpa_directory) free( _qpa_directory);
    if (_uqpx_dir) free(_uqpx_dir);
    if (_uqpx_filename) free(_uqpx_filename);
}



//====================
// ReadQpf 
//====================

EAUpldTbl_StatusE 
UpldTbl::ReadQpx( char * qpxfile,  int load_table ) {
    _isQpa=IsQpa(qpxfile);
    if (ParseQpxFilename( qpxfile ) == EA_UPLDTBL_OK)
        return( _status=Read( load_table ) );
    else
        return( _status=EA_UPLDTBL_FILENAME_PARSE_ERROR);
    

}



//====================
// ParseQpxFilename
//====================

EAUpldTbl_StatusE 
UpldTbl::ParseQpxFilename( char *qpxfile) 

{
    char * tmpfile=strdup( qpxfile );
    char *slash=strrchr( tmpfile, (int) '/');
    if (slash != NULL) {
        _filename=strdup( slash+1 );
        *(slash+1)='\0';
        _directory=strdup( tmpfile );
    } else {
        _filename=strdup(tmpfile );
    }
    if (_directory == NULL )
        return(EA_UPLDTBL_NULL_DIRECTORY);
    else if ( _filename == NULL)
        return (EA_UPLDTBL_FILENAME_PARSE_ERROR);
    else
        return( EA_UPLDTBL_OK );
}

//====================
// Read
//====================

EAUpldTbl_StatusE
UpldTbl::Read(int load_table ) {
    FILE *ifp;
    _status=EA_UPLDTBL_OK;

    if (_filename == NULL) {
        fprintf(stderr,"Null filename\n");
        return (_status=EA_UPLDTBL_ERROR);
    }

    char filename[BIG_SIZE+1];
    sprintf( filename, "%s/%s", _directory, _filename);
    if ( (ifp=fopen( filename, "r" ) ) == NULL ) {
        fprintf(stderr,"Error opening Qpf file %s\n", filename);
        return (_status= EA_UPLDTBL_FILE_OPEN_ERROR);
    }

    int reclen, numlines,version_number,numWordsInParamFile,
        repetition;
    char project_name[6], ao_provider_code[5], nasda_tacc_code[5],
        file_creation_date[9], file_creation_time[9],
        begin_date_of_request[9],end_date_of_request[9],
        file_format_version_date[9],date_of_commanding[9], time_of_commanding[9],
        pathstring[4],mnemonic_string[10],reserved[7];
    _isQpa=IsQpa(filename);
    char line[BIG_SIZE];
    char *cp=fgets(line, BIG_SIZE, ifp);
    if (cp != line) {
        (void) fclose(ifp);
        fprintf( stderr, "Error reading %s\n", filename );
        return( _status=EA_UPLDTBL_FILE_READ_ERROR);
    }   

    int nwords;
    if (_isQpa) {
        nwords=sscanf(line,
    "QPA%07d %6s %4s %4s %8s %8s %4d %5d %8s %8s %8s V%02d %8s %8s %9s %6d %2d%6s\n",
                          &_number, project_name, ao_provider_code, 
                          nasda_tacc_code, file_creation_date,
                          file_creation_time, &reclen, &numlines,
                          begin_date_of_request, end_date_of_request,
                          file_format_version_date, &version_number,
                          date_of_commanding, time_of_commanding,
                          mnemonic_string, &numWordsInParamFile,
                          &repetition, reserved);


    } else {
        nwords=sscanf(line,
    "QPF%07d %6s %4s %4s %8s %8s %4d %5d %8s %8s %8s V%02d %8s %3s %9s %6d %2d%6s\n",
                          &_number, project_name, ao_provider_code, 
                          nasda_tacc_code, file_creation_date,
                          file_creation_time, &reclen, &numlines,
                          begin_date_of_request, end_date_of_request,
                          file_format_version_date, &version_number,
                          date_of_commanding, pathstring,
                          mnemonic_string, &numWordsInParamFile,
                          &repetition, reserved);

    }

    if (nwords < 17) {
        // reserved may or may not be present.
        (void) fclose(ifp);
        fprintf( stderr, "Error reading %s\n", filename );
        return( _status=EA_UPLDTBL_FILE_READ_ERROR);
    }   

    

    int loop=-1;
    while (TableMap[++loop].tabletype != EA_TBLTYPE_UNKNOWN) {
        if (strcmp(TableMap[loop].uploadcmd.CmdMnemonic,mnemonic_string)==0) {
            _cmdHex=TableMap[loop].uploadcmd.CmdHex;
            _tabletype=TableMap[loop].tabletype;
            break;
        }
    }     
    
    if (TableMap[loop].tabletype==EA_TBL_UNKNOWN) {
        fclose(ifp);
        return( _status=EA_UPLDTBL_INVALID_MNEMONIC);
    }

    _tot_num_entries = numWordsInParamFile;
    int year, month, day, hour, min, sec;

    (void) sscanf( file_creation_date, "%04d%02d%02d", &year, &month, &day);
    (void) sscanf( file_creation_time, "%02d:%02d:%02d", 
                   &hour, &min, &sec);

    char time_string[CODEA_TIME_LEN];
    (void) sprintf( time_string, "%04d-%02d-%02dT%02d:%02d:%02d",
                    year, month, day, hour, min, sec);
    _file_creation_time.CodeAToItime( time_string );

    (void) sscanf( begin_date_of_request, "%04d%02d%02d", 
                   &year, &month, &day );
    (void) sprintf( time_string, "%04d-%02d-%02dT00:00:00", 
                    year, month, day );
    _begin_date.CodeAToItime( time_string );

    (void) sscanf( end_date_of_request, "%04d%02d%02d", 
                   &year, &month, &day );
    (void) sprintf( time_string, "%04d-%02d-%02dT23:59:59.999", 
                    year, month, day );
    _end_date.CodeAToItime( time_string );

    (void) sscanf( date_of_commanding, "%04d%02d%02d", 
                   &year, &month, &day );

    if (_isQpa) {
        (void) sprintf( time_string, "%04d-%02d-%02dT",
                        year, month, day);
        (void) strcat( time_string, time_of_commanding);
        _command_date.CodeAToItime( time_string );
    } else 
        (void) sprintf( time_string, 
                        "%04d-%02d-%02dT00:00:00.000", 
                        year, month, day );

    _command_date.CodeAToItime( time_string );

    
    if (load_table)
        _status=LoadTableFromFile(ifp, numlines);
    fclose(ifp);
    return( _status );

    
} // Read




//====================
// InitTable 
//====================

EAUpldTbl_StatusE 
UpldTbl::InitTable() {

    int max_num_words=-1;
    int nwords[2]={-1,-1}, defaults_nwords[2]={-1,-1};
    int toggle=0, defaults_toggle=0;
    if (_tot_num_entries <= 0) {
        
        int numwords=FindNumWords();

        if (numwords == 0 )
            return (_status=EA_UPLDTBL_ERROR);

        if (numwords == -1) {

              // Avoided it long as possible, but we have to load the
              // QPF files from the QPF directory, read through them and
              // find the largest size for this particular table

            _status=InitQpxList();
            if (_status != EA_UPLDTBL_OK) {
                printf("UpldTbl::InitTable, Error Initializing QpxList\n");
                return(_status);
            }

            for (int i=0; _qpxlist[i]; i++) {

                if ( _qpxlist[i]->tabletype == _tabletype) {

                    char *test=strstr( _qpxlist[i]->filename, 
                                       "QPF9" );
                    if (test != NULL) {
                        // This is one of the 'defaults' QPF files.
                        defaults_nwords[defaults_toggle] = _qpxlist[i]->numwords;
                        defaults_toggle += 1;
                        defaults_toggle %= 2;
                    } else {
                        nwords[toggle] = _qpxlist[i]->numwords;
                        toggle += 1;
                        toggle %= 2;
            
                    }
                }
            }

            // 3 Scenarios

            // 1. No files have been uploaded since the initial set
            // (the 'defaults' set, signified by the name QPF9xxxxxx)
            // Take the maximum number of words from those files
            //
            //2. no more than 1 file has been uploaded.  Take the
            //   maximum of the two defaults files and the one
            //   non-defaults file.
            //
            //3. two or more. Take the max from the last of the two
            //   (or more) uploaded after the default set
            
            int scenario =-1;
            if (nwords[0]==-1 && nwords[1]==-1) {
                scenario=1;
            } else if ( nwords[0] != -1 && nwords[1] == -1) {
                scenario=2;
            } else 
                scenario=3;
            switch (scenario) {
            case 1: 
                max_num_words = defaults_nwords[0]>defaults_nwords[1] ? 
                    defaults_nwords[0] : defaults_nwords[1];
                break;
            case 2:
                max_num_words = defaults_nwords[0]>defaults_nwords[1] ? 
                    defaults_nwords[0] : defaults_nwords[1];
                max_num_words=max_num_words>nwords[0]? max_num_words: nwords[0];
                break;
            case 3:
                max_num_words = nwords[0]>nwords[1] ? nwords[0] : nwords[1];
                break;
            default:
                max_num_words=-1;
                break;
                
            }
            if (max_num_words == -1) {
                printf( "UpldTbl::InitTable, max_num_words = -1\n");
                return (EA_UPLDTBL_ERROR);
            }
            numwords=max_num_words;
        }

        _tot_num_entries=numwords;
    }
    if ( (_table =
          ( unsigned short int *) 
          calloc((size_t) 
                 _tot_num_entries, sizeof(short int) ) ) == NULL ) {
        printf("UpldTbl::InitTable, Memory Error\n");
        return (_status=EA_UPLDTBL_MEMORY_ERROR);
    }
    
    if ( (_filled =
          (char *) calloc((size_t) 
                          _tot_num_entries, sizeof(char) ) )  == NULL ) {
        printf("UpldTbl::InitTable, Memory Error\n");
        return (_status=EA_UPLDTBL_MEMORY_ERROR);
    }

    return (_status=EA_UPLDTBL_OK);
}

//====================
// LoadTableFromFile 
//
// Assumes we currently have a QPF/A 
// file open for reading.
//====================

EAUpldTbl_StatusE 
UpldTbl::LoadTableFromFile(FILE *ifp, int numlines) {
    if (_table == NULL ) {
        EAUpldTbl_StatusE status;
        if ( (status=InitTable()) !=EA_UPLDTBL_OK)
            return (status);
    }
    int nwords=0, nexpected=0, wordsleft=_tot_num_entries;
    int totwords=0;
    unsigned short int val;
    char line[QPF_CHAR_PER_LINE+1], *cp;
    char line_copy[QPF_CHAR_PER_LINE+1] ;

    // Because of a feature of sally's SafeString, one must put an
    // extra null at the end of strings, to handle the case where the
    // string doesn't end with the delimiter. Sally is going to fix
    // this, and when she does, I'll change this back to something
    // less strange.

    for (int lineno=numlines;lineno>0;lineno-- ){
        cp=fgets( line, QPF_CHAR_PER_LINE+1, ifp);
        if (cp != line ) {
            fprintf(stderr, "UpldTbl: QPx read failed\n");
            return( _status=EA_UPLDTBL_FILE_READ_ERROR);
        }
        char *newline =strchr( cp, (int) '\n' );
        if (newline != NULL) *newline='\0';
        for (int i=0;i<QPF_CHAR_PER_LINE+1;i++) line_copy[i]='\0';
        (void) strncpy( line_copy, line, QPF_CHAR_PER_LINE);

        if (lineno == 1)
            nexpected=wordsleft;
         else 
             nexpected=QPF_CHAR_PER_LINE/5;
        char *lasts=0;
        int n;
        (void) strncpy( line_copy, line, strlen(line) );
        line_copy[ strlen(line)+1] = '\0';
        int nwordsthisline=0;
        char *string=0;
        int ii=0;
        for (string = (char *) safe_strtok(line_copy," ", &lasts);
             string;
             string=(char *) safe_strtok(0," ", &lasts) ) {
            n=sscanf( string, "%hX",&val );
            if (lineno == numlines && ii == 0) {
                ii++;
                continue;
            }
            nwordsthisline++;
            if (nwordsthisline>nexpected) {
                nwordsthisline--;
                break;
            }
            AddToTable( totwords, 1, &val );
            totwords++;
            
        }
        wordsleft -= nwordsthisline;
        if (lineno!=1 && wordsleft<=0) {
            fprintf(stderr, "Too Many words in QPF/A file\n");
            return (_status= EA_UPLDTBL_NUM_WORDS_MISMATCH );
        }
    }
    if (totwords != _tot_num_entries) {
        fprintf(stderr, "NParam words mismatch in QPF/A file\n");
        return( _status= EA_UPLDTBL_NUM_WORDS_MISMATCH);
    }
    return( _status=EA_UPLDTBL_OK );
    
}

//====================
// LoadTableFromFile 
//
// Assumes we're loading a file whose directory and name we know,
// i.e. we've already opened it once, when we read the header
// information, and now we're reading the table itself.
//
//====================

EAUpldTbl_StatusE 
UpldTbl::LoadTableFromFile() {
    if (_filename == NULL || _directory == NULL )
        return (_status=EA_UPLDTBL_ERROR);

    if (_table == NULL ) {
        if ( (_status=InitTable()) !=EA_UPLDTBL_OK)
            return (_status);
    }

    char *filename = new char [BIGSIZE+1];
    (void) sprintf( filename , "%s/%s", _directory, _filename);
    FILE * ifp;
    if ((ifp=fopen(filename,"r"))==NULL) 
        return(_status=EA_UPLDTBL_FILE_OPEN_ERROR);
    char *rec;
    int nchar=BIGSIZE;
    rec=fgets( rec, nchar, ifp);
    int reclen, numlines,numWordsInParamFile;
    char mnemonic_string[10];

    _isQpa=IsQpa(_filename);
    int nwords;

    if (_isQpa) {
        nwords=fscanf(ifp,
    "QPA%*07d %*6s %*4s %*4s %*8s %*8s %4d %5d %*8s %*8s %*8s V%*02d %*8s %*8s %9s %6d %*2d %*6s\n",
                          &reclen, &numlines, mnemonic_string, 
                          &numWordsInParamFile);


    } else {
        nwords=fscanf(ifp,
    "QPF%*07d %*6s %*4s %*4s %*8s %*8s %4d %5d %*8s %*8s %*8s V%*02d %*8s %*3s %9s %6d %*2d %*6s\n",
                          &reclen, &numlines,
                          mnemonic_string, &numWordsInParamFile);


    }
    if (nwords != 4) {
        fprintf( stderr, "Error reading %s\n", filename );
        return( _status=EA_UPLDTBL_FILE_READ_ERROR);
    }   

    

    LoadTableFromFile(ifp, numlines);
    return( _status );
}


//==================== 
// ReallocTable 
//
// For use after we've downloaded a table from telemetry. Since we
// don't know, a priori, how big it is, e.g. the PRF tables, we'll
// have to allocate the table to the largest size we might need, then
// reallocate it after we think we've downloaded all of it.
// ====================


EAUpldTbl_StatusE 
UpldTbl::ReallocTable()
{
    // If the current table count is less than the total number of
    // entries and all of those have been filled, reallocate the
    // table. Otherwise, leave it untouched.

    if (_num_entries < _tot_num_entries ){
        int isFilled=1;
        for (int i=0; i< _num_entries; i++) {
            if ( *(_filled+i) == 0 ) {
                isFilled=0;
                break;
            }
        }
        if (isFilled) {
            _tot_num_entries = _num_entries;
            _table=(unsigned short *) 
                realloc( _table, 
                         _num_entries*sizeof(unsigned short int ) );
            if (_table==NULL) 
                return (_status=EA_UPLDTBL_MEMORY_ERROR);
        } else 
            return (_status=EA_UPLDTBL_OK);
    }
    return( _status=EA_UPLDTBL_OK);
}


//====================
// SetUploadTime
//====================


EAUpldTbl_StatusE 
UpldTbl::SetUploadTime(const char *l1_time) 
{
    if (l1_time == NULL ) 
        return (_status=EA_UPLDTBL_ERROR);
    //    _upload_time = new Itime;
    int status=_upload_time.L1ToItime( l1_time );
    if (status == 0) {
        fprintf(stderr,"UpldTbl: Bad upload time %s\n", l1_time );
        return (_status=EA_UPLDTBL_BAD_TIME);
    }
    
}



//====================
// SetSwitchTime
//====================

EAUpldTbl_StatusE 
UpldTbl::SetSwitchTime(const char *l1_time) 
{
    if (l1_time == NULL ) 
        return (_status=EA_UPLDTBL_ERROR);
    // _switch_time = new Itime;
    int status=_switch_time.L1ToItime( l1_time );
    if (status == 0) {
        fprintf(stderr,"UpldTbl: Bad switch time %s\n", l1_time );
        return (_status=EA_UPLDTBL_BAD_TIME);
    }
    
};


//====================
// AddToTable
//====================

EAUpldTbl_StatusE 
UpldTbl::AddToTable(int loc, int nentries, unsigned short int *buf) {

    if (_table==NULL) 
        if ((_status=InitTable()) != EA_UPLDTBL_OK)
            return (_status);

        

    if (loc+nentries-1 > _tot_num_entries){
        fprintf(stderr, "UpldTbl: Tot_num_entries=%d, loc+nentries=%d\n",
                _tot_num_entries, loc+nentries);
        return (_status=EA_UPLDTBL_NUM_WORDS_MISMATCH);
    }
    for (int i=0;i<nentries;i++) {
        *(_table+loc+i) = *(buf+i);
        if (*(_filled+loc+i) == 0) 
            _num_entries += 1;
        *(_filled+loc+i)=1;
    }

    return(_status=EA_UPLDTBL_OK);
    
}


//====================
// CompareTable
//====================
EAUpldTbl_Table_StatusE 
UpldTbl::CompareTable(UpldTbl *tbl, int *num_matched) {
    unsigned short int val;
    int filled=0;
    int num_missing=0;
    *num_matched=0;
    int numlocs=-1;


    if (_tabletype != tbl->GetTableType())
        return (_tablestatus=EA_UPLDTBL_TABLE_MISMATCH);

    int i=-1;
    while (TableCompareMaskMap[++i].tabletype != EA_TBLTYPE_UNKNOWN)
        if (TableCompareMaskMap[i].tabletype == _tabletype ){
            numlocs=TableCompareMaskMap[i].numlocations;
            break;
        }
    const Ignorable *ignore_these=0;
    ignore_these= TableCompareMaskMap[i].locations;

    if (_tot_num_entries != tbl->GetNumEntries())
        return (_tablestatus=EA_UPLDTBL_TABLE_MISMATCH);
    
    int mask_ptr=0;
    for (i=0; i<_tot_num_entries; i++) {
        val=tbl->GetTableEntry(i,&filled);
        if (ignore_these != 0) {
            if (ignore_these[mask_ptr].location<i && mask_ptr < numlocs-1)
                mask_ptr++;
        }
        if ( *(_filled+i) && filled) {
            if ( *(_table+i) != val) {
                if (ignore_these != 0) {
                    if (ignore_these[mask_ptr].location==i) {
                        if (val & ignore_these[mask_ptr].mask != 
                            *(_table+i) & ignore_these[mask_ptr].mask ) 
                            // No need to go further.
                            return (_tablestatus=EA_UPLDTBL_TABLE_MISMATCH);
                    }
                } else 
                    // No need to go further.
                    return (_tablestatus=EA_UPLDTBL_TABLE_MISMATCH);
            }
            else 
                *num_matched += 1;
        } else {
            if (!(*(_filled+i)))
                num_missing++;
        }
    }
    if(num_missing != 0 ) 
        return (_tablestatus=EA_UPLDTBL_TABLE_PARTIAL_MATCH);
    else
        return (_tablestatus=EA_UPLDTBL_TABLE_MATCH);
    
} // end CompareTable( UpldTbl *tbl, int *nummatched)



//====================
// CompareTable
//====================
EAUpldTbl_Table_StatusE 
UpldTbl::CompareTable(UpldTbl *tbl ) {
    int junk;
    return( _tablestatus=CompareTable( tbl, &junk) );
} // end CompareTable( UpldTbl *tbl )



//====================
// MergeTables
//====================
EAUpldTbl_StatusE
UpldTbl::MergeTable(UpldTbl *tbl) {
    short int val;
    int filled=0;
    if (_tabletype != tbl->GetTableType())
        return (_status=EA_UPLDTBL_TABLETYPE_MISMATCH);

    if (_tot_num_entries != tbl->GetNumEntries())
        return (_status=EA_UPLDTBL_NUM_WORDS_MISMATCH);
    
    for (int i=0; i<_tot_num_entries; i++) {
        val=tbl->GetTableEntry(i,&filled);
        if (filled) {
            *(_table+i) = val;
            *(_filled+i) = 1;
        } 
    }
    return (_status=EA_UPLDTBL_OK);
}

//====================
// GetTableEntry
//====================
unsigned short int 
UpldTbl::GetTableEntry(int loc, int *filled ) {
    unsigned short int retval=0;
    *filled=0;
    if (loc<_tot_num_entries) {
        *filled=*(_filled+loc);
        retval=*(_table+loc);
    }
    return( retval );
}


//====================
// FindTableType
//
// Given the Command history (4 short ints) cmdHex values, return
// the active table associated with the first table switch command
// found, or return EA_TBLTYPE_UNKNOWN.
//
//====================

EAUpldTbl_TypeE
UpldTbl::FindTableType( unsigned short int *cmd_hist){
    

    int loop1=0, loop2=0;

    while (loop1<4 ){
        //TableMap[loop2].switchcmd.CmdHex != cmd_hist[loop1] ) {
        while ( TableMap[loop2].tabletype != 
                EA_TBLTYPE_UNKNOWN) {
            if(TableMap[loop2].switchcmd.CmdHex ==
               cmd_hist[loop1] ) {
                break;
            } 
            loop2++;
        }
        if(TableMap[loop2].switchcmd.CmdHex ==
           cmd_hist[loop1] ) {
            break;
        } 
        loop1++;
    }    
    
    return(TableMap[loop2].tabletype);
    
} // FindTableType


//====================
// FindNumWords
//====================

int 
UpldTbl::FindNumWords() 
{
    // Returns the number of words expected for this table.
    // If this is a file, get the info from the header data. If not,
    // get this information from the cmdArgsTable of the Command object.
    // returns -1 if there can be a variable number of words.
    int numwords=0; // start out undefined

    if (_isFile) {
        numwords = _tot_num_entries;
    } else {
        int loop=0;
        if (TableMap[_tabletype].tabletype != EA_TBLTYPE_UNKNOWN) {
            EACommandE cmdid=TableMap[_tabletype].uploadcmd.CommandId;
            for (int i=0;i<Command::numCmdArgsEntries;i++)
            {
                if (Command::cmdArgsTable[i].cmdID == cmdid ) {
                    numwords=Command::cmdArgsTable[i].numWordsInParamFile;
                    break;
                }
            }
        }
    }

    return (numwords);

} // FindNumWords



//====================
// returns 1 if all entries in the table 
// have been filled, else returns 0
//====================


int 
UpldTbl::CompletelyFilled()
{
    for (int i=0;i<_tot_num_entries;i++)
        if (*(_filled+i) == 0)
            return(0);
    return(1);

} // Completely Filled



//====================
// SetFilename
//====================
EAUpldTbl_StatusE
UpldTbl::SetFilename(const char *filename) 
{
    if (filename==NULL) 
        return (_status=EA_UPLDTBL_NULL_FILENAME);
    if ( (_filename=strdup( filename )) == NULL )
        return (_status=EA_UPLDTBL_MEMORY_ERROR);
    _isQpa=IsQpa(_filename);
    return (_status=EA_UPLDTBL_OK);
}

//====================
// SetDirectory
//====================
EAUpldTbl_StatusE
UpldTbl::SetDirectory(const char *directory) 
{
    if (directory==NULL) 
        return (_status=EA_UPLDTBL_NULL_DIRECTORY);
    if ( (_directory=strdup( directory )) == NULL )
        return (_status=EA_UPLDTBL_MEMORY_ERROR);
    _isQpa=IsQpa(directory);
    return (_status=EA_UPLDTBL_OK);
}

//====================
// SetDirectorySearchList
//====================

EAUpldTbl_StatusE
UpldTbl::SetDirectorySearchList(const char *qpa_directory,
                                const char *qpf_directory) 
{
    if (qpa_directory==NULL )
        return( _status=EA_UPLDTBL_NULL_DIRECTORY);
    else if ((_qpa_directory=strdup( qpa_directory )) == NULL)
        return (_status=EA_UPLDTBL_MEMORY_ERROR);

    if (qpf_directory==NULL )
        return( _status=EA_UPLDTBL_NULL_DIRECTORY);
    else if ((_qpf_directory=strdup( qpf_directory )) == NULL)
        return (_status=EA_UPLDTBL_MEMORY_ERROR);

    return (_status=EA_UPLDTBL_OK);
}

//====================
// SetTableId
//====================
EAUpldTbl_StatusE
UpldTbl::SetTableId(EAUpldTblE tableid ) 
{
    if (tableid != _tableid) {
        _tableid=tableid;
        _tot_num_entries=0;
        if (_tableid != EA_TBLTYPE_CDS)
            return (_status=InitTable());
    }
    return (EA_UPLDTBL_OK);
}


//====================
// SetTableType
//====================
EAUpldTbl_StatusE
UpldTbl::SetTableType(EAUpldTbl_TypeE tabletype ) 
{
    if (tabletype != _tabletype) {
        _tabletype=tabletype;
        _tot_num_entries=0;
        return (_status=InitTable());
    }
    return (EA_UPLDTBL_OK);
}


//====================
// SetActivity
//====================
void
UpldTbl::SetActivity(ActivityStateE act ) 
{
    _activity_state=act;
    return;
}

//====================
// ToggleActivity
//====================
void
UpldTbl::ToggleActivity() 
{
    if (_activity_state != ACT_UNKNOWN){
        int junk=(int)_activity_state;
        junk += 1;
        junk %= 2;
        _activity_state = (ActivityStateE) junk;
        _tableid = (EAUpldTblE) (((int)_activity_state<<4) + 
                                 (int) _tabletype);
    }

}

//====================
// TableUpload
//====================
EAUpldTbl_TypeE 
UpldTbl::TableUpload( unsigned short int cmd ) {
  int i=-1;
  while ( TableMap[++i].uploadcmd.CmdHex != cmd) {
    if (TableMap[i].tabletype == EA_TBLTYPE_UNKNOWN)
        break;
  }
  return( TableMap[i].tabletype );
  
}

//====================
// TableSwitch
//====================
EAUpldTbl_TypeE 
UpldTbl::TableSwitch( unsigned short int cmd ) {
  int i=-1;
  while (TableMap[++i].switchcmd.CmdHex != cmd)
      if (TableMap[i].tabletype == EA_TBLTYPE_UNKNOWN)
          break;
  return( TableMap[i].tabletype );

}

//====================
// InitQpxList
//
// Initializes the Qpx (QPF/A) directory array, if not 
// already initialized
//
//====================
EAUpldTbl_StatusE
UpldTbl::InitQpxList()
{
    int i;
    if (_qpxlist==NULL) {

        UpldTblList *qpxlist = new UpldTblList( _qpa_directory, 
                                                _qpf_directory );

        if (qpxlist->GetStatus() != UpldTblList::UPLDTBLLIST_OK)
            return (_status=EA_UPLDTBL_BAD_QPX_LIST_INIT);
        int n_nodes = qpxlist->NodeCount();
        _qpxlist=new QpxDirEntry * [n_nodes+1];
        for (i=0;i<n_nodes;i++) {
            _qpxlist[i] = new QpxDirEntry;
            if ( _qpxlist[i] == NULL)
                return (_status=EA_UPLDTBL_MEMORY_ERROR);
        }
        UpldTbl *tbl;
        for (i = 0, tbl=qpxlist->GetHead(); 
             tbl; 
             i++, tbl=qpxlist->GetNext()) {
            (_qpxlist[i])->filename=strdup( tbl->GetFilename() );
            (_qpxlist[i])->numwords=tbl->GetNumEntries();
            (_qpxlist[i])->tabletype=tbl->GetTableType();
            (_qpxlist[i])->time=tbl->GetCommandDate();
        }        

        _qpxlist[i]=0;

        delete qpxlist;


    } 
    return (_status=EA_UPLDTBL_OK);
}

//====================
// SetCommandDate
//====================

EAUpldTbl_StatusE 
UpldTbl::SetCommandDate(const char *l1_time ) {
    if (l1_time == NULL ) {
        fprintf(stderr,"UpldTbl: NULL l1_time!\n");
        return (_status=EA_UPLDTBL_ERROR);
    }
    int status=_command_date.L1ToItime( l1_time );
    if (status == 0) {
        fprintf(stderr,"UpldTbl: Bad command date %s\n", l1_time );
        return (_status=EA_UPLDTBL_BAD_TIME);
    }
    
}// end SetCommandDate


//====================
// IsQpa
// Returns 1 is filename has 
// string 'QPA' in it.
//====================

int 
UpldTbl::IsQpa( const char *filename) {
    char * p = strstr( filename, "QPA");
    return (p != NULL );

} // end IsQpa


//====================
// TableTypeToEffectId
//====================

EffectE 
UpldTbl::TableTypeToEffectId( EAUpldTbl_TypeE table_type, 
                              unsigned short cmdHex ) {
 int loop=-1;
 EffectE effectid;
 do {
     loop += 1;
 } while (TableMap[loop].tabletype != table_type && 
          TableMap[loop].tabletype != EA_TBL_UNKNOWN );

 if (TableMap[loop].uploadcmd.CmdHex == cmdHex)
     effectid = TableMap[loop].uploadcmd.EffectId;
 else if (TableMap[loop].switchcmd.CmdHex == cmdHex)
     effectid = TableMap[loop].switchcmd.EffectId;
 else
     effectid = EFF_UNKNOWN;

             
 return (effectid);
}



//====================
// TableTypeToCmdHex
//====================

unsigned short 
UpldTbl::TableTypeToCmdHex() {

    int i=-1;
    unsigned short cmdHex;
    while (TableMap[++i].tabletype != _tabletype)
        cmdHex = TableMap[i].uploadcmd.CmdHex;
    return (cmdHex);
}

//====================
// FinalEffectId
//====================

EffectE 
UpldTbl::FinalEffectId() {

    int i=-1;
    while (TableMap[++i].tabletype != _tabletype) {
        if (TableMap[i].tabletype == EA_TBLTYPE_UNKNOWN )
            break;
    }
    return (TableMap[i].finalEffectId);
}




//====================
// PrettyPrintFinalStatus
//====================
EAUpldTbl_StatusE
UpldTbl::PrettyPrintFinalStatus(FILE *ofp, char *fill_string) {
    char first_seen_string[CODEA_TIME_LEN];
    char last_seen_string[CODEA_TIME_LEN];
    _first_seen.ItimeToCodeA(first_seen_string); 
    _last_seen.ItimeToCodeA(last_seen_string); 
    EAUpldTbl_StatusE tmp_status=EA_UPLDTBL_OK, tmp_status2;
    if (ofp == NULL) {
        fprintf( stderr, 
           "UpldTbl::PrettyPrintFinalStatus, NULL OUTPUT FILE Pointer\n");
        tmp_status=EA_UPLDTBL_BAD_WRITE;
    } else {
      fprintf( ofp, "%sTable Id: %s\n", fill_string,   PPMap[ (int) _tableid+1 ]);
      fprintf( ofp, "%sFirst Seen: %s\n", fill_string, first_seen_string );
      fprintf( ofp, "%sLast  Seen: %s\n", fill_string, last_seen_string );
      switch (_tablestatus) {
      case EA_UPLDTBL_TABLE_UNKNOWN:
          fprintf( ofp, 
                   "%sUnknown Table!\n Table Data ...\n",
                   fill_string);
          break;
      case EA_UPLDTBL_TABLE_MATCH:
          fprintf(ofp, 
                  "%sMatching Qpx: %s/%s\n",
                  fill_string, 
                  GetDirectory(),
                  GetFilename() );
          break;
      case EA_UPLDTBL_TABLE_PARTIAL_MATCH:
          fprintf(ofp, 
                  "%s(Partial) Match, Qpx %s/%s\n",
                  fill_string,
                  GetDirectory(),
                  GetFilename() );
          break;
      case EA_UPLDTBL_TABLE_MISMATCH:
          fprintf(ofp,
                  "%sAll Tables Mismatch\n Table Data ... \n",
                  fill_string);
          break;
      }                
    }
    tmp_status2=WriteTableForHumans(ofp, fill_string);
    if (tmp_status != EA_UPLDTBL_OK) 
        return( tmp_status );
    else
        return( tmp_status2 );

    
} // PrettyPrintFinalStatus


//====================
// PrettyPrintUpdateStatus
//====================
EAUpldTbl_StatusE
UpldTbl::PrettyPrintUpdateStatus(FILE *ofp, char *fill_string) {

    EAUpldTbl_StatusE tmp_status=EA_UPLDTBL_OK, tmp_status2;
    if (ofp == NULL) {
        fprintf( stderr, 
           "UpldTbl::PrettyPrintUpdateStatus, NULL FILE Pointer\n");
        tmp_status=EA_UPLDTBL_BAD_WRITE;
    } else {
        fprintf( ofp, "%sTable Id: %s\n", fill_string,   PPMap[ (int) _tableid+1 ]);
        switch (_tablestatus) {
        case EA_UPLDTBL_TABLE_UNKNOWN:
            fprintf( ofp, 
                     "%sUnknown Table!\n Table Data ...\n",
                     fill_string);
            
            break;
        case EA_UPLDTBL_TABLE_MATCH:
            fprintf(ofp, 
                    "%sMatching Qpx: %s/%s\n",
                    fill_string, 
                    GetDirectory(),
                    GetFilename() );
            return( _status=EA_UPLDTBL_OK);
            break;
        case EA_UPLDTBL_TABLE_PARTIAL_MATCH:
            fprintf(ofp, 
                    "%s(Partial) Match, Qpx %s/%s\n",
                    fill_string,
                    GetDirectory(),
                    GetFilename() );
            return( _status=EA_UPLDTBL_OK);
            break;
        case EA_UPLDTBL_TABLE_MISMATCH:
            fprintf(ofp,
                    "%sAll Tables Mismatch\n %sTable Data ... \n",
                    fill_string, fill_string);
            return( _status=WriteTableForHumans(ofp, fill_string));
            break;
        }             
    }
    tmp_status2=WriteTableForHumans(ofp, fill_string);
    if (tmp_status != EA_UPLDTBL_OK)
        return(tmp_status);
    else
        return( tmp_status2);

} // PrettyPrintUpdateStatus


//-----------------
// WriteTableForHumans
//-----------------

EAUpldTbl_StatusE
UpldTbl::WriteTableForHumans( FILE* fp)
{
    return( WriteTableForHumans( fp, NULL) );
}//WriteTableForHumans


//-----------------
// WriteTableForHumans
//-----------------

EAUpldTbl_StatusE
UpldTbl::WriteTableForHumans( FILE* fp, char * fill_string)
{

    EAUpldTbl_StatusE tmp_status, tmp_status2;

    if (fill_string == NULL)
        fill_string = "";
    if (fp != NULL) {
        if (_cmdHex == 0) 
            _cmdHex=TableTypeToCmdHex();
      
        if (_tablestatus != EA_UPLDTBL_TABLE_MATCH)
            _WriteTable(fp,fill_string);

    } else 
        tmp_status=EA_UPLDTBL_BAD_WRITE;

    tmp_status2=WriteTableToUqpxDir();
    if (tmp_status != EA_UPLDTBL_OK)
        return(_status=tmp_status);
    else
        return(_status=tmp_status2);


}//WriteTableForHumans



//====================
// WriteTableToUqpxDir
//====================
EAUpldTbl_StatusE
UpldTbl::WriteTableToUqpxDir() {
    if (_tablestatus != EA_UPLDTBL_TABLE_MATCH) {
        if (_uqpx_dir==NULL) {
            return (_status=EA_UPLDTBL_ERROR);
        }
        DIR *dirptr;
        dirptr=opendir(_uqpx_dir);
        if (dirptr) {
            struct dirent *dirEntry;
            int maxnum=-1,number=-1;
            while (dirEntry = readdir(dirptr)) {
                if (*(dirEntry->d_name) != '.') {
                    int nchars = sscanf( dirEntry->d_name,"UQPX%07d", &number);
                    if (nchars==1) 
                        maxnum=number>maxnum?number:maxnum;
                }
                
            }
            maxnum++;
            closedir(dirptr);
            char filename[MAX_FILENAME_LEN+1];
            snprintf( filename, MAX_FILENAME_LEN, "%s/UQPX%07d",
                      _uqpx_dir, maxnum );
            _uqpx_filename = strdup( filename );
            FILE *fp; 
            if ( (fp = fopen( filename, "w" )) != NULL ) {
                if (_cmdHex == 0) 
                    _cmdHex = TableTypeToCmdHex();
                fprintf(fp, "Table_ID: %d\n", _tableid );
                char time_string[CODEA_TIME_LEN+1];
                (void) _first_seen.ItimeToCodeA(time_string);
                fprintf(fp, "First Seen: %s\n",time_string );
                (void) _last_seen.ItimeToCodeA(time_string);
                fprintf(fp, "Last Seen: %s\n",time_string );
                _WriteTable(fp);
                fclose(fp);
            } else {
                fprintf(stderr,"UpldTbl: Can't open new file in unmatched qpx dir %s\n",
                        _uqpx_dir);
                return(_status=EA_UPLDTBL_BAD_WRITE);
            }
        } else {
            fprintf(stderr, "UpldTbl: Can't open unmatched qpx dir %s\n",
                    _uqpx_dir);
            return(_status=EA_UPLDTBL_ERROR);
        }
    }
}


void
UpldTbl::_WriteTable(FILE* fp, char* fill_string) {

    int i, j=0;
    for (i=0; i<_tot_num_entries; i++) {
        if (j == 0) {
            if (i == 0){
                fprintf(fp, "\n%s%04X ", fill_string,_cmdHex);
                j+=1;
            } else
                fprintf(fp, "\n%s", fill_string);
        }
        if (*(_filled+i)) 
            fprintf( fp, "%04X ", *(_table+i) );
        else
            fprintf( fp, "**** ");
        j= (j+1) % 16;
    }
    fprintf(fp, "\n" );

}

//====================
// SetUqpxDir
//====================

void
UpldTbl::SetUqpxDir( const char * directory ) {
    if (directory != NULL)
        _uqpx_dir = strdup( directory );
} //SetUqpxDir


//====================
// Operators
//====================

int
operator<(
     UpldTbl& a,
     UpldTbl& b)
{
    Itime atime, btime;
    if (a._command_date == INVALID_TIME)
        atime=a._first_seen;
    else
        atime=a._command_date;
    if (b._command_date == INVALID_TIME)
        btime=b._first_seen;
    else
        btime=b._command_date;

    if ((a._number != 0 && a._number < 9000000) && 
        (b._number != 0 && b._number < 9000000) )
        return ((atime < btime) &&
                (a._number < b._number ) );
    else {
        return (atime < btime);
    }
}
 
int
operator<=(
     UpldTbl& a,
     UpldTbl& b)
{
    Itime atime, btime;
    if (a._command_date == INVALID_TIME)
        atime=a._first_seen;
    else
        atime=a._command_date;
    if (b._command_date == INVALID_TIME)
        btime=b._first_seen;
    else
        btime=b._command_date;

    if ((a._number != 0 && a._number < 9000000) && 
        (b._number != 0 && b._number < 9000000) )
        return ((atime <= btime) &&
                (a._number <= b._number ) );
    else {
        return (atime <= btime);
    }
}


 
int
operator==(
     UpldTbl& a,
     UpldTbl& b)
{
    Itime atime, btime;
    if (a._command_date == INVALID_TIME)
        atime=a._first_seen;
    else
        atime=a._command_date;
    if (b._command_date == INVALID_TIME)
        btime=b._first_seen;
    else
        btime=b._command_date;

    if ( (a._number != 0 && a._number < 9000000) && 
         (b._number != 0 && b._number < 9000000) )
        return ((atime == btime) &&
                (a._number == b._number ) );
    else {
        return (atime == btime);
    }
}



int
operator>(
    UpldTbl& a,
    UpldTbl& b)
{
    Itime atime, btime;
    if (a._command_date == INVALID_TIME)
        atime=a._first_seen;
    else
        atime=a._command_date;
    if (b._command_date == INVALID_TIME)
        btime=b._first_seen;
    else
        btime=b._command_date;

    if ((a._number != 0 && a._number < 9000000) && 
        (b._number != 0 && b._number < 9000000) )
        return ((atime > btime) &&
                (a._number > b._number ) );
    else {
        return (atime > btime);
    }
}

 

int
operator>=(
    UpldTbl& a,
    UpldTbl& b)
{
    Itime atime, btime;
    if (a._command_date == INVALID_TIME)
        atime=a._first_seen;
    else
        atime=a._command_date;
    if (b._command_date == INVALID_TIME)
        btime=b._first_seen;
    else
        btime=b._command_date;

    if ((a._number != 0 && a._number < 9000000) && 
        (b._number != 0 && b._number < 9000000 ) )
        return ((atime >= btime) &&
                (a._number >= b._number ) );
    else {
        return (atime >= btime);
    }

}
