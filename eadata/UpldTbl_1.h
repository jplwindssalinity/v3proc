//==================================================
//
// UpldTbl_1.h
// Purpose: Defines a subset of the information needed 
// jointly by the UpldTbl and Command.h objects.
//
// Author: William Daffer
//
// CM Log
//
// $Log$
// 
//    Rev 1.0   25 Mar 1999 13:54:58   daffer
// Initial Revision
//
// $Date$
// $Revision$
// $Author$
//
//
//
//==================================================


#ifndef UPLDTBL_1_H
#define UPLDTBL_1_H
static const char upldtbl_1_h_rcsid[]="$Header$";

enum EAUpldTbl_TypeE 
{
    EA_TBLTYPE_UNKNOWN=-1,
    EA_TBLTYPE_PRFEXTSTANDBY = 0,
    EA_TBLTYPE_PRFEXTWOM,
    EA_TBLTYPE_PRFEXTCAL,
    EA_TBLTYPE_PRFEXTRCV,
    EA_TBLTYPE_SESALL,
    EA_TBLTYPE_SESSTANDBY,
    EA_TBLTYPE_SESWOM,
    EA_TBLTYPE_SESCAL,
    EA_TBLTYPE_SESRCV,
    EA_TBLTYPE_SERDIGENGTLM,
    EA_TBLTYPE_SERDIGSTATLM,
    EA_TBLTYPE_DOPPLER_A,
    EA_TBLTYPE_DOPPLER_B,
    EA_TBLTYPE_RANGE_A,
    EA_TBLTYPE_RANGE_B,
    EA_TBLTYPE_CDS
};

enum EAUpldTbl_StatusE
{
    EA_UPLDTBL_UNKNOWN=0,
    EA_UPLDTBL_OK,
    EA_UPLDTBL_ERROR,
    EA_UPLDTBL_TABLETYPE_MISMATCH,
    EA_UPLDTBL_NUM_WORDS_MISMATCH,
    EA_UPLDTBL_TABLEID_MISMATCH,
    EA_UPLDTBL_TABLE_NOQPX,
    EA_UPLDTBL_INIT_ERROR,
    EA_UPLDTBL_FILENAME_PARSE_ERROR,
    EA_UPLDTBL_FILE_OPEN_ERROR,
    EA_UPLDTBL_FILE_READ_ERROR,
    EA_UPLDTBL_MEMORY_ERROR, 
    EA_UPLDTBL_BAD_TIME, 
    EA_UPLDTBL_BAD_TABLE_ID,
    EA_UPLDTBL_NULL_DIRECTORY,
    EA_UPLDTBL_NULL_FILENAME,
    EA_UPLDTBL_INVALID_MNEMONIC,
    EA_UPLDTBL_BAD_QPX_LIST_INIT, 
    EA_UPLDTBL_BAD_WRITE
};

enum EAUpldTbl_Table_StatusE
{
    EA_UPLDTBL_TABLE_UNKNOWN=0,
    EA_UPLDTBL_TABLE_MATCH,
    EA_UPLDTBL_TABLE_PARTIAL_MATCH,
    EA_UPLDTBL_TABLE_MISMATCH
};

#endif
