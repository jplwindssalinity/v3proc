//=========================================================
// Copyright  (C)1995, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.1   26 Feb 1999 14:35:06   sally
// if the ending char is not a delimeter, it continues...  Bad!
// 
//    Rev 1.0   27 Mar 1998 09:55:20   sally
// Initial revision.
// $Date$
// $Revision$
// $Author$
//
//=========================================================

#include <stdio.h>
#include <string.h>

char* 
safe_strtok(
char*          string,  // string to be parsed
const char*    delim,   // token
char**         lasts)   // holds next string
{
    char* mainString = string;
    // if main string is NULL, then this is the continuing parsing
    // take the next string from lasts
    if (mainString == 0)
        mainString = *lasts;

    // reach the end of string
    if (*mainString == '\0')
        return 0;

    char* returnString=0;
    unsigned int oldLen = strlen(mainString);
    returnString = strtok(mainString, delim);

    // found the token, now adjust the "lasts" pointer
    if (returnString != 0)
    {
        *lasts = mainString + strlen(mainString);
        if (oldLen > strlen(mainString))
            *lasts += strlen(delim);
    }

    return returnString;

} // safe_strtok


#if 0
#include <stdlib.h>

main(
int     ,
char**  )
{
    char* constString = "Hello work world sally chou";
    char string[1024];
    strcpy(string, constString);
    char* lasts=0;
    for (char* subString = safe_strtok(string, " ", &lasts); subString;
                  subString = safe_strtok(0, " ", &lasts))
    {
        char* lasts_2=0;
        char string_2[1024];
        strcpy(string_2, constString);
        for (char* subString_2 = safe_strtok(string_2, " ", &lasts_2);
                   subString_2;
                  subString_2 = safe_strtok(0, " ", &lasts_2))
        {
            printf("sub string 2 = [%s]\n", subString_2);
        }
        printf("sub string 1 = [%s]\n", subString);
    }
    exit(0);
} // main
#endif
