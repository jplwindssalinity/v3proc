//=========================================================
// Copyright  (C)1998, California Institute of Technology. 
// U.S. Government sponsorship under NASA contract
// NAS7-1260 is acknowledged.               
// 
// CM Log
// $Log$
// Revision 1.1  1999/01/28 20:41:13  sally
// Initial revision
//
// 
//    Rev 1.0   23 Jun 1998 16:04:40   sally
// Initial revision.
// 
// $Date$
// $Revision$
// $Author$
//
//=========================================================
//----------------------------------------------------------------------
// NAME
//		list_parameters
//
// SYNOPSIS
//		list_parameters
//
// DESCRIPTION
//		The list_parameters program generates a list of parameter
//		numbers and their corresponding parameters.
//
// AUTHOR
//		James N. Huddleston
//
//----------------------------------------------------------------------

//-----------------------//
// Configuration Control //
//-----------------------//

static const char rcsid[] = "@(#) $Header$";

//----------//
// INCLUDES //
//----------//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ParTab.h"

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

void usage(
	char	*command,
	int		exit_value);

void print_table(
	const ParTabEntry*	table,
	int					count);

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

//--------------//
// MAIN PROGRAM //
//--------------//

int main(
int argc,
char *argv[])
{
	//------------------------//
	// parse the command line //
	//------------------------//

	if (argc != 1) {
		usage(argv[0], 1);
	}

	printf("\nHK2\n--------\n");
	print_table(HK2ParTab, HK2ParTabSize);

	printf("\nL1A/NRT\n--------\n");
	print_table(L1AParTab, L1AParTabSize);

	printf("\nL1A Derived\n--------\n");
	print_table(L1ADerivedParTab, L1ADerivedTabSize);

    printf("\nL1B\n--------\n");
    print_table(L1BParTab, L1BParTabSize);

	printf("\nL2A\n--------\n");
	print_table(L2AParTab, L2AParTabSize);

	printf("\nL2B\n--------\n");
	print_table(L2BParTab, L2BParTabSize);

	return 0;
}//main

//----------------------------------------------------------------------
// FUNCTION
//		usage
//
// DESCRIPTION
//		Exits the program <command> by first displaying a usage message
//		and then returning the specified <exit_value> to the shell
//
// AUTHOR
//		James N. Huddleston
//----------------------------------------------------------------------

void usage(
char	*command,
int		exit_value)
{
	fprintf(stderr, "usage:\n");
	fprintf(stderr, "%s\n", command);
	exit(exit_value);
	return;
}

//-------------//
// print_table //
//-------------//

#define COUNT 9

void
print_table(
const ParTabEntry*	table,
int					count)
{
	char address[COUNT];
	int length;

	for (int i = 0; i < count; i++)
	{
		for (unsigned int j = 0; j < table[i].numUnitEntries; j++)
		{
			sprintf(address, "%d.%d", table[i].paramId,
				table[i].unitEntries[j].unitId);
			length = strlen(address);
			printf("%s", address);
			for (int k = 0; k < COUNT - length; k++)
				printf(" ");
			printf("%s (%s)\n",
				table[i].paramName, table[i].unitEntries[j].unitName);
		}
	}
	return;
}//print_table
