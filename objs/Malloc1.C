//
// This file contains functions useful for debugging memory management problems
//

static const char rcs_id_malloc1_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "Malloc1.h"

/* Global memory heap */
MEMBLOCK *lastmemblock = NULL;
int memblockcounter = 0;

/*
 * m_register()
 *
 * Log a memory allocation.
 *
 * INPUT: ptr = pointer to the allocated memory.
 *        str = identifying string (can be NULL)
 *
 */

void m_register(void* ptr, char *str)

{
MEMBLOCK *newmemblock = NULL;
int n;

if (ptr == NULL) return;

newmemblock = (MEMBLOCK *)malloc(sizeof(MEMBLOCK));
if (newmemblock == NULL)
  {
  printf("Error allocating a new memory block\n");
  exit(-1);
  }

newmemblock->count = memblockcounter;	/* store the current index */

if (str == NULL)
  {
  newmemblock->str = NULL;
  }
else
  {
  n = strlen(str);
  if (n == 0)
    {
    newmemblock->str = NULL;
    }
  else
    {
    newmemblock->str = (char *)malloc((n+1)*sizeof(char));
    if (newmemblock->str == NULL)
      {
      printf("Error allocating space for an memory block ID string\n");
      free(newmemblock);
      exit(-1);
      }
    strcpy(newmemblock->str,str);
    }
  }

if (lastmemblock == NULL)
  {
  lastmemblock = newmemblock;
  newmemblock->next = NULL;
  newmemblock->prev = NULL;
  }
else
  {
  lastmemblock->next = newmemblock;
  newmemblock->next = NULL;
  newmemblock->prev = lastmemblock;
  lastmemblock = newmemblock;
  }

lastmemblock->mem = ptr;
memblockcounter++;
return;

}

/*
 * m_unregister()
 *
 * Unlog a memory block.
 *
 * INPUT: ptr = pointer to the block of memory to free.
 *        str = identifying string (can be NULL)
 *
 */

void m_unregister(void *ptr, char *str)

{
MEMBLOCK *memblockptr;

if (ptr == NULL)
  return;

memblockptr = lastmemblock;
while (memblockptr != NULL)
  {
  if (ptr == memblockptr->mem)
    {
    if (memblockptr->str != NULL)
      {
      free(memblockptr->str);
      }

    if (memblockptr->prev != NULL)
      {
      memblockptr->prev->next = memblockptr->next;
      }

    if (memblockptr->next != NULL)
      {
      memblockptr->next->prev = memblockptr->prev;
      }
    else
      {	/* need to relocate the lastmemblock pointer also */
      lastmemblock = memblockptr->prev;
      if (lastmemblock != NULL)
        lastmemblock->next = NULL;
      }

    free(memblockptr);
    return;
    }
  else
    {
    memblockptr = memblockptr->prev;
    }
  }

printf("Error: Attempted to free invalid pointer = %p\n",ptr);
printf("       id string = %s\n",str);
exit(-1);

}

/*
 * memcheck()
 *
 * Check for allocated memory and print an error message if any is found.
 * Then free the memory.
 */

void memcheck()

{
int i;

i = 0;
if (lastmemblock != NULL)
  {
  printf("Warning: Program exited with some memory still allocated\n");
  printf("  The id strings and counts for the allocated blocks are:\n");
  while (lastmemblock != NULL)
    {
    i++;
    if (lastmemblock->mem != NULL)
      {
      free(lastmemblock->mem);
      }

    if (lastmemblock->str == NULL)
      {
      printf("%d: count: %d  no id\n",i,lastmemblock->count);
      }
    else
      {
      printf("%d: count: %d  %s\n",i,lastmemblock->count,lastmemblock->str);
      free(lastmemblock->str);
      }

    if (lastmemblock->prev == NULL)
      {
      free(lastmemblock);
      lastmemblock = NULL;
      }
    else
      {
      lastmemblock = lastmemblock->prev;
      free(lastmemblock->next);
      }
    } 
  }

}
