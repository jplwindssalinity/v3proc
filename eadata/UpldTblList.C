//==================================================
//
// UpldTblList.C
// Purpose: Manipulates List of UpldTble Objects
// Author: William Daffer
//
// CM Log
//
// $Log$
// 
//    Rev 1.1   26 Apr 1999 15:50:18   sally
// port to GNU compiler
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

int errno;
static const char UpldTblList_C_rcsid[]="@(#) $Header$";

#include "UpldTblList.h"



//====================
// Constructors
//====================



UpldTblList::UpldTblList()
    : _status(UPLDTBLLIST_OK)
{
    return;
}


UpldTblList::UpldTblList(const char *directory,
                         const Itime start_time,
                         const Itime end_time )
    : _status(UPLDTBLLIST_OK)

{

    char* _directory;

    if (directory != NULL) {
        if ( (_directory = strdup(directory)) != NULL) {

            DIR *dirptr;
            
            int isqpa = IsQpa(directory);
            if (isqpa)
                _directory = strdup(directory);
            else
                _directory = strdup(directory);
            
            // create a list entry for each
            // QPF/QPA file in that directory.
            int _number;
            int nchars;

            dirptr = opendir( directory );
            if (dirptr) {
                struct dirent *dirEntry;
                int nfiles=0;
                while (dirEntry = readdir(dirptr)) {
                    if (*(dirEntry->d_name) != '.') {
                        if (isqpa) 
                            nchars = sscanf( dirEntry->d_name,"QPA%07d", &_number);
                        else
                            nchars = sscanf( dirEntry->d_name,"QPF%07d", &_number);
                        if (nchars==1) 
                            nfiles++;
                    }
                    
                }
                rewinddir(dirptr);
                QpxDirEnt *qpxdirentry=new QpxDirEnt[nfiles+1];
                //*QpxDirEnt[nfiles]=NULL;
                if (qpxdirentry == NULL) {
                    fprintf(stderr,"UpldTblList: Error Allocating QpxDirEnt\n");
                    _status=UPLDTBLLIST_ERROR;
                    return;
                }
                int ii=0;
                while (dirEntry=readdir(dirptr)) {
                    if (*(dirEntry->d_name) != '.') {
                        if (isqpa) 
                            nchars = sscanf( dirEntry->d_name,"QPA%07d", &_number);
                        else
                            nchars = sscanf( dirEntry->d_name,"QPF%07d", &_number);
                        if (nchars == 1) {
                            qpxdirentry[ii].real_number=_number;
                            if (_number<9000000) _number+=10000000;
                            qpxdirentry[ii].sort_number=_number;
                            char string[MAX_FILENAME_LEN-1];
                            (void) snprintf( string, 
                                             MAX_FILENAME_LEN-1, "%s/%s", 
                                             directory,dirEntry->d_name );

                            (void) strncpy( qpxdirentry[ii].filename, string,
                                     MAX_FILENAME_LEN-1);
                            ii++;
                        }
                    }     
                    
                }
                ii--;
                closedir(dirptr);

                if (ii>0) {
                  // sort the qpxdirentry array
                  qsort( qpxdirentry, ii+1, sizeof(QpxDirEnt),
                       (int(*)(const void*, const void*))QpxDirEntry_Compare );

                  // Add each QPA file to the list, in reverse order
                  // (most recent to least recent)

                  for (int jj=0; jj<=ii; jj++) {
                      UpldTbl *tbl = new UpldTbl( qpxdirentry[jj].filename, 1);
                      if (tbl->GetStatus() == EA_UPLDTBL_OK) {
                          if ( start_time != INVALID_TIME && 
                               end_time != INVALID_TIME &&
                               tbl->GetUploadTime() != INVALID_TIME &&
                               tbl->GetSwitchTime() != INVALID_TIME ) {
                              if ( tbl->GetUploadTime() <= end_time ||
                                   tbl->GetSwitchTime() <= end_time ) 
                                  Append( tbl );
                          } else
                              Append( tbl );
                      } else {
                          fprintf(stderr,
                                  "UpldTblList: Error Creating entry from file %s\n",
                                  qpxdirentry[jj].filename);
                          _status=UPLDTBLLIST_ERROR_READING_FILE;
                          break;
                      }
                  }
                }
                delete qpxdirentry;

            } else // come from if (dirptr)
                _status=UPLDTBLLIST_ERROR;
        } else // come from if ((_directory=strdup(directory))!=NULL)
            _status=UPLDTBLLIST_ERROR;           
    } else // come from if (directory==NULL)
        _status=UPLDTBLLIST_ERROR;
    
    return;
}


UpldTblList::UpldTblList(const char *qpa_directory,
                         const char *qpf_directory,
                         const Itime start_time,
                         const Itime end_time )
    : _status(UPLDTBLLIST_OK)

{
   
    if (qpa_directory != NULL) {
        if ( (_qpa_directory = strdup(qpa_directory)) != NULL) {

            DIR *dirptr;

            // if the file name is a directory, create a list entry for each
            // QPF/QPA file in that directory.
            int _number;
            int nchars;

            dirptr = opendir( qpa_directory );
            if (dirptr) {
                struct dirent *dirEntry;
                int nfiles=0;
                while (dirEntry = readdir(dirptr)) {
                    if (*(dirEntry->d_name) != '.') {
                        nchars = sscanf( dirEntry->d_name,"QPA%07d", &_number);
                        if (nchars==1) 
                            nfiles++;
                    }
                    
                }
                rewinddir(dirptr);
                QpxDirEnt *qpxdirentry=new QpxDirEnt[nfiles+1];
                //*QpxDirEnt[nfiles]=NULL;
                if (qpxdirentry == NULL) {
                    fprintf(stderr,"UpldTblList: Error Allocating QpxDirEntry\n");
                    _status=UPLDTBLLIST_ERROR;
                    return;
                }
                int ii=0;
                while (dirEntry=readdir(dirptr)) {
                    if (*(dirEntry->d_name) != '.') {
                        nchars = sscanf( dirEntry->d_name,"QPA%07d", &_number);
                        if (nchars == 1) {
                            qpxdirentry[ii].real_number=_number;
                            if (_number<9000000) _number+=10000000;
                            qpxdirentry[ii].sort_number=_number;
                            char string[MAX_FILENAME_LEN-1];
                            (void) snprintf( string, 
                                             MAX_FILENAME_LEN-1, "%s/%s", 
                                             qpa_directory,dirEntry->d_name );
                            (void) strncpy( qpxdirentry[ii].filename,string,
                                     MAX_FILENAME_LEN-1);
                            ii++;
                        }
                    }     
                    
                }
                ii--;
                closedir(dirptr);

                if (ii > 0) {
                  // sort the qpxdirentry array
                  qsort( qpxdirentry, ii+1, sizeof(QpxDirEnt),
                       (int(*)(const void*, const void*))QpxDirEntry_Compare );

                  // Add each QPA file to the list, in reverse order
                  // (most recent to least recent)

                  for (int jj=0; jj<=ii; jj++) {
                      UpldTbl *tbl = new UpldTbl( qpxdirentry[jj].filename, 1);
                      if (tbl->GetStatus() == EA_UPLDTBL_OK) {
                          if ( start_time != INVALID_TIME && 
                               end_time != INVALID_TIME &&
                               tbl->GetUploadTime() != INVALID_TIME &&
                               tbl->GetSwitchTime() != INVALID_TIME ) {
                              if ( tbl->GetUploadTime() <= end_time ||
                                   tbl->GetSwitchTime() <= end_time ) 
                                  Append( tbl );
                          } else
                              Append( tbl );
                      } else {
                          fprintf(stderr,
                                  "UpldTblList: Error Creating entry from file %s\n",
                                  qpxdirentry[jj].filename);
                          _status=UPLDTBLLIST_ERROR_READING_FILE;
                          break;
                      }
                  }
                }
                delete qpxdirentry;

                // Merge in qpf directory.

                if (qpf_directory != NULL) {
                    if ( (_qpf_directory = strdup( qpf_directory ))==NULL)
                        _status=UPLDTBLLIST_ERROR;
                    else
                        (void) MergeInQpx( _qpf_directory, 
                                           start_time, end_time );
                } else 
                    _status=UPLDTBLLIST_ERROR;
            } else // come from if (dirptr)
                _status=UPLDTBLLIST_ERROR;
        } else // come from if ((_qpa_directory=strdup(qpa_directory))!=NULL)
            _status=UPLDTBLLIST_ERROR;           
    } else // come from if (qpa_directory==NULL)
        _status=UPLDTBLLIST_ERROR;
    
    return;
}


//====================
// Destructors
//====================

UpldTblList::~UpldTblList() {
    if (_qpa_directory)
        free ( _qpa_directory);
    if (_qpf_directory)
        free (_qpf_directory);

    return;
}


//====================
// GetCount
//====================

int 
UpldTblList::GetCount() 
{
    int cnt;
    UpldTbl *tbl;
    for (cnt=0, tbl=GetHead(); tbl; tbl=GetNext(), cnt++)
    return cnt;

}


//====================
// FindMatchingTable
//====================

UpldTbl *
UpldTblList::FindMatchingTable( UpldTbl *intbl )
{
    UpldTbl *tbl=0;
    for (tbl=GetHead(); tbl; tbl=GetNext() ){
        if (tbl->GetTableType() == intbl->GetTableType() &&
            tbl->GetActivity() == intbl->GetActivity() ) {
            EAUpldTbl_Table_StatusE status=
                tbl->CompareTable( intbl );
            if (status == EA_UPLDTBL_TABLE_MATCH)
                break;
        }
    }
    return tbl;

}


//====================
// FindNearest2
//
// Find the two tables in the list whose commanded time is <= to input
// time having the input table type. 
//
//====================

UpldTblList *
UpldTblList::FindNearest2( EAUpldTbl_TypeE tabletype, Itime& time )
{
    UpldTbl *tbl=0;
    UpldTblList *tbllist=new UpldTblList();
    int ii=0;
    for (tbl=GetTail(); tbl; tbl=GetPrev() ){
        if (tbl->GetTableType() == tabletype &&
            tbl->GetCommandDate() <= time ) {
            tbllist->AddSorted(tbl);
            ii+=1;
            if (ii == 2)
                break;
        }
    }
    return tbllist;
}


//====================
// IsQpa
//   Returns true is input filename is a QPF file 
//   or is in a QPF directory
//====================

int
UpldTblList::IsQpa(const char *filename) {
  return ( strstr(filename, "QPA") != NULL );
} // end 


//====================
// MergeInQpx
//
//   Merge the QPx (most likely QPF) files contained in the 
//   directory specified in the input variable 
//   into the already existing list.
//
//====================

UpldTblList::UpldTblList_StatusE
UpldTblList::MergeInQpx(const char *directory, 
                        Itime start_time, 
                        Itime end_time) {

    DIR *dirptr;
    UpldTbl *tbl=0;
    int number, nfiles=0;
    int nchars=0;
            
    // create a list entry for each
    // QPF file in that directory.

    dirptr = opendir( directory );
    if (dirptr) {
        struct dirent *dirEntry;
        while ((dirEntry = readdir(dirptr))){
            if (*(dirEntry->d_name) != '.') {
                nchars = sscanf( dirEntry->d_name,"QPF%07d", &number);
                if (nchars == 1) {
                    nfiles++;
                }
            }
        }
        rewinddir(dirptr);
        QpxDirEnt *qpxdirentry=new QpxDirEnt[nfiles];
        if (qpxdirentry == NULL) {
            fprintf(stderr,"UpldTblList: Error creating qpxdirentry array\n");
            return(_status=UPLDTBLLIST_ERROR);
        }
        int ii=0;
        while ((dirEntry = readdir(dirptr))) {
            if (*(dirEntry->d_name) != '.') {
                nchars = sscanf( dirEntry->d_name,"QPF%07d", &number);
                if (nchars == 1) {
                    qpxdirentry[ii].real_number=number;
                    if (number<9000000) number+=10000000;
                    qpxdirentry[ii].sort_number=number;
                    char string[MAX_FILENAME_LEN-1];
                    (void) snprintf( string, 
                                     MAX_FILENAME_LEN-1, "%s/%s", 
                                     directory,dirEntry->d_name );
                    
                    (void) strncpy( qpxdirentry[ii].filename, string,
                                    MAX_FILENAME_LEN-1);
                    ii++;
                }
            }
        }
        ii--;
        closedir(dirptr);
            
        if (ii>=0) {
            // sort the qpxdirentry array
            qsort( qpxdirentry, ii+1, sizeof(QpxDirEnt),
                   (int(*)(const void*, const void*))QpxDirEntry_Compare );
            
            for (int jj=0;jj<=ii;jj++) {
                for ( tbl=GetTail(); tbl; tbl=GetPrev() ) {
                    if (tbl->GetNumber() == qpxdirentry[jj].real_number ) 
                        break;
                }
                if (tbl==NULL) {
                    tbl = new UpldTbl(qpxdirentry[jj].filename, 1);
                    if (tbl->GetStatus() == EA_UPLDTBL_OK) {
                        if ( start_time != INVALID_TIME && 
                             end_time != INVALID_TIME &&
                             tbl->GetUploadTime() != INVALID_TIME &&
                             tbl->GetSwitchTime() != INVALID_TIME ) {
                            if ( tbl->GetUploadTime() <= end_time ||
                                 tbl->GetSwitchTime() <= end_time ) 
                                Append( tbl );
                        } else
                            Append( tbl );
                    } else {
                        fprintf(stderr,
                                "UpldTblList: Error Creating entry from file %s\n",
                                qpxdirentry[jj].filename);
                        _status=UPLDTBLLIST_ERROR_READING_FILE;
                        break;
                    }
                }
            } // Loop over qpxdirentry's
        }// from if(ii>=0)
        delete qpxdirentry;
    } else {
        fprintf( stderr,
                 "UpldTblList::MergeInQpx. %s is not a directory!\n",
                 directory );
        _status=UPLDTBLLIST_ERROR;
    }
    
    return (_status);
} // end MergeInQpf


int  QpxDirEntry_Compare( const void * entry1, 
                          const void * entry2)
{
    QpxDirEnt *p1 = (QpxDirEnt *) entry1;
    QpxDirEnt *p2 = (QpxDirEnt *) entry2;

    
    if (p1->sort_number > p2->sort_number )
        return(1);
    if (p1->sort_number < p2->sort_number) 
        return(-1);
    else
        return(0);
}
