#ifndef MALLOC1_H
#define MALLOC1_H

static const char rcs_id_malloc1_h[] =
	"@(#) $Id$";

typedef struct memblock
  {
  void *mem;
  char *str;
  int count;
  struct memblock *next;
  struct memblock *prev;
  } MEMBLOCK;

extern MEMBLOCK *lastmemblock;

int m_register(void* ptr, char *str);
void m_unregister(void *ptr, char *str);
void memcheck();

#endif
