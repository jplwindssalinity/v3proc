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
	char newline[STRLEN];
	char *start,*end;
	char *sptr,*wordend;
	char *comment_start;
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

		// Strip comments first.
		sptr = strstr(str,"//");
		comment_start = sptr;
		if (comment_start != NULL) *comment_start = '\0';

		int keyword1 = 0;

		sptr = strstr(str,"new");
		while (sptr != NULL)
		{	// found a possible match
			if (isspace(*(sptr+3)))
			{	// definite match
			keyword1 = 1;
			break;
			}
			else
			{	// look for more on this line
		    sptr = strstr(sptr+3,"new");
			}
		}

		if (keyword1 == 0)
		{
			sptr = strstr(str,"malloc");
			while (sptr != NULL)
			{	// found a possible match
				if (isspace(*(sptr+6)) || *(sptr+6) == '(')
				{	// definite match
				keyword1 = 1;
				break;
				}
				else
				{	// look for more on this line
		    	sptr = strstr(sptr+6,"malloc");
				}
			}
		}

		if (keyword1 == 1)
		{	// found a new or malloc in this line
			end = sptr;
			while (*end != '\0')
			{	// sweep for the end of this statement
				end++;
				if (*end == ';')
				{
					break;
				}
			}
			if (*end == '\0')
			{
				printf("Error: couldn't find the end of %s\n",sptr);
				exit(-1);
			}
			start = sptr;
			while (start != str)
			{	// sweep for the beginning of this statement
				start--;
				if (*start == ';' || *start == '}')
				{
					start++;
					break;
				}
			}
		
			// Find the variable name to use.
	
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

			// Create an addition expression to replace the new or malloc call.
			*end = '\0';
			strcpy(newline,str);
			strcat(newline,"; m_register((void*)");
			strcat(newline,wordstring);
			strcat(newline,",\"");
			strcat(newline,argv[1]);
			strcat(newline,":");
			sprintf(numstring,"%d",line);
			strcat(newline,numstring);
			strcat(newline,"\");");
			strcat(newline,end+1);
		}

		int keyword2 = 0;

		sptr = strstr(str,"delete");
		while (sptr != NULL)
		{	// found a possible match
			if (isspace(*(sptr+6)))
			{	// definite match
			keyword2 = 1;
			break;
			}
			else
			{	// look for more on this line
		    sptr = strstr(sptr+6,"delete");
			}
		}

		if (keyword2 == 0)
		{
			sptr = strstr(str,"free");
			while (sptr != NULL)
			{	// found a possible match
				if (isspace(*(sptr+4)) || *(sptr+4) == '(')
				{	// definite match
				keyword2 = 1;
				break;
				}
				else
				{	// look for more on this line
		    	sptr = strstr(sptr+4,"free");
				}
			}
		}

		if (keyword2 == 1)
		{	// found a delete or free in this line
			end = sptr;
			while (*end != '\0')
			{	// sweep for the end of this statement
				end++;
				if (*end == ';')
				{
					break;
				}
			}
			if (*end == '\0')
			{
				printf("Error: couldn't find the end of %s\n",sptr);
				exit(-1);
			}
			start = sptr;
			while (start != str)
			{	// sweep for the beginning of this statement
				start--;
				if (*start == ';' || *start == '}')
				{
					start++;
					break;
				}
			}
		
			// Find the variable name to use.
	
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

			// Create a compound statement to replace the free or delete call.
			c = *start;
			*start = '\0';
			strcpy(newline,str);
			*start = c;
			strcat(newline,"{m_unregister((void*)");
			strcat(newline,wordstring);
			strcat(newline,",\"");
			strcat(newline,argv[1]);
			strcat(newline,":");
			sprintf(numstring,"%d",line);
			strcat(newline,numstring);
			strcat(newline,"\");");
			c = *(end+1);
			*(end+1) = '\0';
			strcat(newline,start);
			strcat(newline,"}");
			*(end+1) = c;
			strcat(newline,end+1);
		}

		// put back comments if no alterations were made. Also xfer line
		if (keyword1 == 0 && keyword2 == 0)
		{
			if (comment_start != NULL) *comment_start = '/';
			strcpy(newline,str);
		}
		
		if (fputs(newline,outfile) == EOF)
		{
			printf("Error writing line to %s\n",argv[2]);
			exit(-1);
		}

	}

	fclose(infile);
	fclose(outfile);

}

