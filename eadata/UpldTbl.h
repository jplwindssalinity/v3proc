//==================================================
//
// UpldTbl.h
// Purpose: Header for Uploaded Tables Object
// Author: William Daffer
//
// CM Log
//
// $Log$
// 
//    Rev 1.0   25 Mar 1999 13:56:54   daffer
// Initial Revision
//
// $Date$
// $Revision$
// $Author$
//
//
//
//==================================================

#ifndef UPLDTBL_H
#define UPLDTBL_H
static const char upldtbl_h_rcsid[]="$Header$";

#include "UpldTbl_1.h"
#include "Itime.h"
#include "Command_1.h"

#define QPF_CHAR_PER_LINE   82 // 2 for slop!
#define QPF_WORDS_PER_LINE              16
#define BIGSIZE 1024

enum EAUpldTblE 
{
    EA_TBL_UNKNOWN=-1,
    EA_TBL_ACT_PRFEXTSTANDBY = 0,
    EA_TBL_ACT_PRFEXTWOM,
    EA_TBL_ACT_PRFEXTCAL,
    EA_TBL_ACT_PRFEXTRCV,
    EA_TBL_ACT_SESALL,
    EA_TBL_ACT_SESSTANDBY,
    EA_TBL_ACT_SESWOM,
    EA_TBL_ACT_SESCAL,
    EA_TBL_ACT_SESRCV,
    EA_TBL_ACT_SERDIGENGTLM,
    EA_TBL_ACT_SERDIGSTATLM,
    EA_TBL_ACT_DOPPLER_A,
    EA_TBL_ACT_DOPPLER_B,
    EA_TBL_ACT_RANGE_A,
    EA_TBL_ACT_RANGE_B,
    EA_TBL_CDS,
    EA_TBL_INACT_PRFEXTSTANDBY,
    EA_TBL_INACT_PRFEXTWOM,
    EA_TBL_INACT_PRFEXTCAL,
    EA_TBL_INACT_PRFEXTRCV,
    EA_TBL_INACT_SESALL,
    EA_TBL_INACT_SESSTANDBY,
    EA_TBL_INACT_SESWOM,
    EA_TBL_INACT_SESCAL,
    EA_TBL_INACT_SESRCV,
    EA_TBL_INACT_SERDIGENGTLM,
    EA_TBL_INACT_SERDIGSTATLM,
    EA_TBL_INACT_DOPPLER_A,
    EA_TBL_INACT_DOPPLER_B,
    EA_TBL_INACT_RANGE_A,
    EA_TBL_INACT_RANGE_B      
        
};



typedef struct QpxDirEntry {
    char           *filename;
    int             numwords;
    Itime           time;
    EAUpldTbl_TypeE tabletype;
    short int *     data;
};

enum ActivityStateE {
    ACT_UNKNOWN  = -1,
    ACT_ACTIVE, 
    ACT_INACTIVE
};

class UpldTbl {

 public:
    UpldTbl();
    
    UpldTbl( EAUpldTbl_TypeE, ActivityStateE );
    UpldTbl(char *qpxfile,    // path + name
            int load_table = 0 ); 
    
    ~UpldTbl();
    
    EAUpldTbl_StatusE   GetStatus() { return _status;};
    Itime               GetUploadTime() {return _upload_time;};
    Itime               GetSwitchTime() {return _switch_time;};
    Itime               GetCommandDate() {return _command_date;};
    Itime               GetFileCreationTime() {return _file_creation_time;};
    EAUpldTblE          GetTableId() {return _tableid;};
    EAUpldTbl_TypeE     GetTableType() {return _tabletype;};
    ActivityStateE      GetActivity() {return _activity_state;};
    int                 GetNumEntries() { return _tot_num_entries;};
    int                 GetNumber() { return (_number);};
    unsigned short int  GetTableEntry(int loc, int *filled);
    EAUpldTbl_TypeE     FindTableType(unsigned short int *cmd_history);
    int                 Ignore( EAUpldTbl_TypeE );
    EAUpldTbl_TypeE     TableUpload( unsigned short int); // returns table type                                                          // if last cmd
                                                         // was a table upload 
    EAUpldTbl_TypeE    TableSwitch( unsigned short int); // returns table type 
                                                         // if last cmd was 
                                                         // a table switch
    unsigned short    TableTypeToCmdHex();

    int               IsQpa( const char * );
    
    char *            GetFilename() {return _filename;};
    char *            GetDirectory(){return (_directory);};

    void              SetStatus(EAUpldTbl_StatusE status) 
                              {_status=status; return;};
    void              SetTableStatus(EAUpldTbl_Table_StatusE tablestatus) 
                              {_tablestatus=tablestatus; return;};
    void              SetActivity( ActivityStateE );

    EAUpldTbl_StatusE SetFilename( const char *);
    EAUpldTbl_StatusE SetDirectory( const char *);    
    EAUpldTbl_StatusE SetSwitchTime(const char *);
    EAUpldTbl_StatusE SetTableTime(const char *);
    EAUpldTbl_StatusE SetTableId( EAUpldTblE );
    EAUpldTbl_StatusE SetUploadTime(const char *);
    EAUpldTbl_StatusE SetCommandDate(const char *);
    EAUpldTbl_StatusE SetTableType( EAUpldTbl_TypeE );
    EAUpldTbl_StatusE SetDirectorySearchList( const char *qpa_directory, 
                                              const char* qpf_directory );
    void              SetUqpxDir( const char *);



    EAUpldTbl_StatusE InitTable();
    EAUpldTbl_StatusE AddToTable(int loc, int nentries, 
                                 unsigned short int *buf );
    EAUpldTbl_StatusE LoadTableFromFile(FILE *ifp, int numlines);
    EAUpldTbl_StatusE LoadTableFromFile();
    EAUpldTbl_StatusE ReallocTable();    
    EAUpldTbl_StatusE InitQpxList();
    int               CompletelyFilled();
    EAUpldTbl_StatusE LoadQxfList();
    EAUpldTbl_StatusE ReadQpx(char *qpffile, int load_table=0);
    //    EAUpldTbl_StatusE ReadQpf(Qpf& qpf, int load_table=0);
    EAUpldTbl_StatusE Read(int load_table=0);
    int               FindNumWords();
    int               NumFilled() {return (_num_entries); };
    EAUpldTbl_StatusE ParseQpxFilename( char *filename );
    EAUpldTbl_StatusE MergeTable(UpldTbl *tbl );

    void              ToggleActivity();
    EffectE           TableTypeToEffectId( EAUpldTbl_TypeE TableType,
                                           unsigned short int );
    EffectE           FinalEffectId();

    EAUpldTbl_Table_StatusE CompareTable(UpldTbl *tbl );
    EAUpldTbl_Table_StatusE CompareTable(UpldTbl *tbl, int *num_matched );
    EAUpldTbl_Table_StatusE GetTableStatus() { return _tablestatus;};

    EAUpldTbl_StatusE  PrettyPrintFinalStatus(FILE *, char*);
    EAUpldTbl_StatusE  PrettyPrintUpdateStatus(FILE *,char*);
    EAUpldTbl_StatusE  WriteTableForHumans(FILE *, char *);
    EAUpldTbl_StatusE  WriteTableForHumans(FILE *);
    EAUpldTbl_StatusE  WriteTableToUqpxDir();
    void               _WriteTable(FILE *, char *fill="");
    
    // 

    Itime            _file_creation_time; 
    Itime            _begin_date;    // Request time begin data from tile
    Itime            _end_date;      // Request time End date from file
    Itime            _command_date;  // Command Date from file
                                     // hh:mm:ss.ccc invalid if 
                                     // QPf file
    Itime            _switch_time;   // Time switch command is seen.  
    Itime            _upload_time;   // Time Upload command is seen.
    Itime            _first_seen;
    Itime            _last_seen;
    
    int             _isFile; // True is we're reading QPF/A files and not
                              // a table from the telemetry stream.
    int             _isQpa;   // 1 if file is a QPA file.
    int             _complete; // 1 if all elements of table are filled

    int             _ignore_till_next_time; // true if 
    int             _defined; // set to false when an upload is seen

                               // (and/or switch, in certain circumstances.)
    ActivityStateE   _activity_state; // (-1=unknown, 0=inactive, 1=active)
    unsigned short   _cmdHex; // Stores cmd_history values.

    int              _number;    // qpf/a number associated 
                                // with this table

    EAUpldTblE        _tableid;     // The table id
    EAUpldTbl_TypeE   _tabletype;   // the table type (exclusive of 
                                    // active/inactive distinction)
    unsigned short   *_table;       // The table data.
    char             *_filled;      // flag, 1=filled ,0=not
    char            *_uqpx_dir; // Place to write tables that don't
                                         // match any qpx
    char            *_uqpx_filename; // name of file where unmatched table 
                                     // data was written, if any.
    char            *_filename;   // name of qpf/a  associated 
                                 // with this table (file w/o path)
    char            *_directory; // its directory 



 private:
      // Qpf/Qpa directory search list.
      // Used in InitQpxList
    char            *_qpa_directory; // qpa directory used in InitQpxList
    char            *_qpf_directory; // qpf directory used in InitQpxList
    EAUpldTbl_StatusE _status;
    EAUpldTbl_Table_StatusE _tablestatus;

    int               _tot_num_entries; // either the number of 
                                        // elements is qpf file 
                                        // or in table appearing in telemetry.
    int               _num_entries; // number of locations currently 
                                    // filled in table.
    static QpxDirEntry **_qpxlist;     // Array containing information about 
                                      // files in Qpf/a dir  such as
                                      // creation time and number of words 
                                      // in file.

};

int operator< (UpldTbl &, UpldTbl &);
int operator<=(UpldTbl &, UpldTbl &);
int operator> (UpldTbl &, UpldTbl &);
int operator>=(UpldTbl &, UpldTbl &);
int operator==(UpldTbl &, UpldTbl &);


#endif



