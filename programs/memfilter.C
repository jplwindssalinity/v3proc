//
// memfilter
//
// This program reads lines of C-source code from one file, and writes
// them out to another file.  Whenever a memory management call is
// found (new, malloc, delete, free), an additional expression immediately
// following the memory management call is added to the output file.
// The additional expression calls a memory tracking routine to aid in
// tracking down memory management problems.  The additional expression
// is separated from the original call by a comma.
//

static const char memfilter_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define STRLEN 1024

int main(int argc, char *argv[])

{

	if (argc != 3)
	{
		printf("Usage: memfilter infile outfile\n");
		exit(-1);
	}

	FILE *infile = fopen(argv[1],"r");
	if (infile == NULL)
	{
		printf("Error opening input file %s\n",argv[1]);
		exit(-1);
	}

	FILE *outfile = fopen(argv[2],"w");
	if ((infile == NULL) || (outfile == NULL))
	{
		printf("Error opening output file %s\n",argv[2]);
		exit(-1);
	}

	char str[STRLEN];
	char numstring[STRLEN];
	char wordstring[STRLEN];
	char *sptr,*sptr1,*wordend;
	char c;

	// Write out the #include "Malloc1.h" directive at the start of the output.
	strcpy(str,"#include \"Malloc1.h\"\n");
	if (fputs(str,outfile) == EOF)
	{
		printf("Error writing line to %s\n",argv[2]);
		exit(-1);
	}

	int line = 0;
	while (1)
	{
		line++;
		if (fgets(str,STRLEN,infile) == NULL)
		{
			break;
		}

		int keyword = 0;

		sptr = strstr(str,"new");
		while (sptr != NULL)
		{	// found a possible match
			if (isspace(*(sptr+3)))
			{	// definite match
			keyword = 1;
			break;
			}
			else
			{	// look for more on this line
		    sptr = strstr(sptr+3,"new");
			}
		}

		if (keyword == 0)
		{
			sptr = strstr(str,"malloc");
			while (sptr != NULL)
			{	// found a possible match
				if (isspace(*(sptr+6)) || *(sptr+6) == '(')
				{	// definite match
				keyword = 1;
				break;
				}
				else
				{	// look for more on this line
		    	sptr = strstr(sptr+6,"malloc");
				}
			}
		}

		if (keyword == 1)
		{	// found a new or malloc in this line
			while (sptr != str)
			{	// sweep backward for the '='.
				sptr--;
				if (*sptr == '=') break;
			}
			while (sptr != str)
			{	// sweep backward for the end of the assigned variable.
				sptr--;
				if (isalnum(*sptr) || *sptr == '_') break;
			}
			if (sptr != str)
			{
				wordend = sptr+1;
				c = *wordend;
				*wordend = '\0';	// set end of string marker
			}
			while (sptr != str)
			{	// sweep backward for the beginning of the assigned variable.
				sptr--;
				if (!(isalnum(*sptr) || *sptr == '_')) break;
			}
			if (sptr == str)
			{
				printf("Error backing up in line: %s\n",str); 
				exit(-1);
			}
			sptr++;	// move back to beginning of assigned variable

			strcpy(wordstring,sptr);
			*wordend = c;	// repair str

			sptr1 = sptr;
			while (*sptr1 != ';')
			{	// sweep for the end of this statement
				sptr1++;
			}
			*sptr1 = '\0';	// overwrite ';' with end of string marker.

			// Write out the modified line.
			strcat(str,",m_register((void*)");
			strcat(str,wordstring);
			strcat(str,",\"");
			strcat(str,argv[1]);
			strcat(str,":");
			sprintf(numstring,"%d",line);
			strcat(str,numstring);
			strcat(str,"\");\n");
		}

		keyword = 0;

		sptr = strstr(str,"delete");
		while (sptr != NULL)
		{	// found a possible match
			if (isspace(*(sptr+6)))
			{	// definite match
			keyword = 1;
			break;
			}
			else
			{	// look for more on this line
		    sptr = strstr(sptr+6,"delete");
			}
		}

		if (keyword == 0)
		{
			sptr = strstr(str,"free");
			while (sptr != NULL)
			{	// found a possible match
				if (isspace(*(sptr+4)) || *(sptr+4) == '(')
				{	// definite match
				keyword = 1;
				break;
				}
				else
				{	// look for more on this line
		    	sptr = strstr(sptr+4,"free");
				}
			}
		}

		if (keyword == 1)
		{	// found a delete or free in this line
			while (1)
			{	// sweep forward for the end of the delete or free 
				sptr++;
				if (!isalpha(*sptr)) break;
			}
			while (1)
			{	// sweep forward for the beginning of the assigned variable.
				sptr++;
				if (isalnum(*sptr) || *sptr == '_') break;
			}
			wordend = sptr;	// remember start of variable
			while (1)
			{	// sweep forward for the end of the assigned variable.
				wordend++;
				if (!(isalnum(*wordend) || *wordend == '_')) break;
			}
			c = *wordend;
			*wordend = '\0';	// set end of string marker

			strcpy(wordstring,sptr);
			*wordend = c;	// repair str

			sptr1 = sptr;
			while (*sptr1 != ';')
			{	// sweep for the end of this statement
				sptr1++;
			}
			*sptr1 = '\0';	// overwrite ';' with end of string marker.

			// Write out the additional line.
			strcat(str,",m_unregister((void*)");
			strcat(str,wordstring);
			strcat(str,",\"");
			strcat(str,argv[1]);
			strcat(str,":");
			sprintf(numstring,"%d",line);
			strcat(str,numstring);
			strcat(str,"\");\n");
		}

		if (fputs(str,outfile) == EOF)
		{
			printf("Error writing line to %s\n",argv[2]);
			exit(-1);
		}

	}

	fclose(infile);
	fclose(outfile);

}

