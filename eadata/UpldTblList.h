//==================================================
//
// UpldTblList.h
// Purpose: Header for UpldTblList.C
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

#ifndef UPLDTBL_LIST_H
#define UPLDTBL_LIST_H
static const char upldtbllist_h_rcsid[]="$Header$";
#include <stdio.h>
#include "EAList.h"
#include "UpldTbl.h"
#include "Itime.h"
#include "CommonDefs.h"


typedef struct QpxDirEnt{
    int real_number;
    int sort_number;
    char filename[MAX_FILENAME_LEN];
};

class UpldTblList : public SortedList<UpldTbl>
{

 public:
    enum UpldTblList_StatusE {
        UPLDTBLLIST_OK,
        UPLDTBLLIST_ERROR,
        UPLDTBLLIST_ERROR_OPENING_FILE,
        UPLDTBLLIST_ERROR_READING_FILE,
        UPLDTBLLIST_ERROR_READING_TABLE,
        UPLDTBLLIST_ERROR_APPENDING_TABLE,
        UPLDTBLLIST_ERROR_WRITING_TABLE
    };

    UpldTblList();

    UpldTblList( const char * directory,
                 const Itime start_time=INVALID_TIME,
                 const Itime end_time=INVALID_TIME ); 


    UpldTblList( const char * qpa_directory,
                 const char * qpf_directory,
                 const Itime start_time=INVALID_TIME,
                 const Itime end_time=INVALID_TIME ); 

    virtual ~UpldTblList();

    UpldTblList_StatusE GetStatus() {return _status; };
    int                 GetCount();
    UpldTbl* FindMatchingTable( UpldTbl *); // return node in list
                                           // with the input table's 
                                           // table type, activity 
                                           // status and for which 
                                           // the table data matches

    UpldTblList*  FindNearest2( EAUpldTbl_TypeE, Itime& );
    int           IsQpa(const char * filename);
    UpldTblList_StatusE  MergeInQpx(const char * directory,
                                    Itime start_time, 
                                    Itime end_time );

    char *        GetQpfDirectory(){ return (_qpf_directory);};
    char *        GetQpaDirectory(){ return (_qpa_directory);};


 private: 

    char *_qpf_directory;
    char *_qpa_directory;

    UpldTblList_StatusE _status;
    
                 
};
extern int QpxDirEntry_Compare( const void *, const void * );    



#endif
