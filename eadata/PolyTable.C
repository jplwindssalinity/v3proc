//=========================================================
// Copyright  (C)1998, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.1   12 Feb 1998 16:48:58   sally
// add wrappers for "C" functions
// 
//    Rev 1.0   04 Feb 1998 14:16:46   daffer
// Initial checking
// Revision 1.3  1998/01/30 22:29:21  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

static const char rcs_id_PolyTable_C[] =
    "@(#) $Header$";

#include <stdio.h>
#include <string.h>

#include "CommonDefs.h"
#include "PolyTable.h"

PolynomialTable::PolynomialTable(
const char*             filename,   // IN
EA_PolynomialErrorNo&   status)     // OUT: return status
{
    // open the file and read in the varName, unitName, and polynomial
    FILE* fp = fopen(filename, "r");
    if (fp == NULL)
    {
        status = EA_POLY_FILE_OPEN_FAILED;
        return;
    }
    char line[BIG_SIZE];
    char polynomialLine[BIG_SIZE];
    char varName[STRING_LEN], unitName[STRING_LEN];
    while (fgets(line, BIG_SIZE, fp) != NULL)
    {
        // skip comments lines (#|*) and blank lines
        if(line[0] == '#' || line[0] == '*' || line[0] == '\n')
            continue;

        // read varName, unitName and the rest of the line
        if (sscanf(line, " %s %s %[^\n]",
                   varName, unitName, polynomialLine) != 3)
        {
            fprintf(stderr, "Insufficient arguments: %s\n", line);
            status = EA_POLY_FILE_NOT_ENOUGH_ARGS;
            return;
        }

        // count the number of factors and allocate the space for them
        char tmpString[BIG_SIZE];
        (void)strcpy(tmpString, polynomialLine);
        int num=0;
        char* subString;
        for (subString = (char*)strtok(tmpString, ","); subString;
                    subString = (char*)strtok(0, ","))
        {
            num++;
        }
        float* array = new float[num];
        if (array == 0)
        {
            status = EA_POLY_OUT_OF_MEMORY;
            return;
        }

        // now read the numbers in and create a Polynomial object for it
        int i=0;
        (void)strcpy(tmpString, polynomialLine);
        for (subString = (char*)strtok(tmpString, ","); subString;
                    subString = (char*)strtok(0, ","), i++)
        {
            if (sscanf(subString, " %g", &(array[i])) != 1)
            {
                fprintf(stderr, "Error reading polynomial: %s\n", subString);
                status = EA_POLY_ERR_READ_COEFFICIENTS;
                return;
            }
        }
        Polynomial* polynomial = new Polynomial(varName, unitName,
                                        array, num);
        if (polynomial == 0)
        {
            status = EA_POLY_OUT_OF_MEMORY;
            return;
        }
        delete [] array;

        // save it in the list
        Append(polynomial);
    }

    status = EA_POLY_OK;
    fclose(fp);

}//PolynomialTable::PolynomialTable

PolynomialTable::~PolynomialTable()
{
    // ~List() will empty the list

}//PolynomialTable::~PolynomialTable

const Polynomial*
PolynomialTable::SelectPolynomial(
const char*   varName,         // IN
const char*   unitName)        // IN
{
    for (Polynomial* current=GetHead(); current != 0; current=GetNext())
    {
        if (strcmp(current->GetVarName(), varName) == 0 &&
            strcmp(current->GetUnitName(), unitName) == 0)
            return current;
    }
    return 0;

}//PolynomialTable::SelectPolynomial
