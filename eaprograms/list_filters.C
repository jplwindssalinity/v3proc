//----------------------------------------------------------------------
// NAME
//		list_filters
//
// SYNOPSIS
//		list_filters
//
// DESCRIPTION
//		The list_filters program generates a list of filters
//		abbreviations and their corresponding names.
//
// AUTHOR
//		Sally H. Chou
//
//----------------------------------------------------------------------

//-----------------------//
// Configuration Control //
//-----------------------//

static const char rcsid[] = "@(#) $Id$";

//----------//
// INCLUDES //
//----------//

#include <stdlib.h>
#include <string.h>
#include "Filter.h"

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

void usage(
	char	*command,
	int		exit_value);

void print_table(
	const FilTabEntry*	table);

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
	print_table(HK2FilTab);

	printf("\nL1A/L1AP\n--------\n");
	print_table(L1AFilTab);

	return 0;
}

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

#define COUNT 20

void print_table(
	const FilTabEntry*	table)
{
	char abbreviation[COUNT];
	int filter_index = 0;
    char printString[40];
    sprintf(printString, "%%-%ds %%s\n", COUNT);
	while (table[filter_index].filterName != NULL)
	{
		sprintf(abbreviation, "%s", table[filter_index].filterAbbr);
        printf(printString, table[filter_index].filterAbbr,
                            table[filter_index].filterName);
		filter_index++;
	} // while
	return;
}
