//=========================================================
// Copyright  (C)1995, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.0   27 Mar 1998 09:55:18   sally
// Initial revision.
// $Date$
// $Revision$
// $Author$
//
//=========================================================
#ifndef SAFE_STRING_H
#define SAFE_STRING_H

char* safe_strtok(char*          string,  // string to be parsed
                  const char*    delim,   // token
                  char**         lasts);  // holds next string

#endif /* SAFE_STRING_H */
