
// TODO: This is a very inefficient module
// we just use linear search. Could be replaced
// with hashbuckets or similar.

#include "store.h"

#include <stdio.h>
#include <stdlib.h>

#include <string.h>

static int cardi=100;
static int ix=0;

typedef struct {
  char *key;
  int value;
} pair;

static pair *p;

void store_init()
{
  int i;
  p = (pair*) malloc(cardi*sizeof(pair));
  for (i=0;i<100;i++)
    {
      p[i].key=NULL;
    }
}

void store_destroy()
{
  int i;
  for (i=0;i<ix;i++)
    {
      free(p[i].key);
    }
  free(p);
}

void store_insert(const char *key, int val)
{
  if (ix==cardi)
    {
      fprintf(stderr, "TODO: realloc\n");
      exit(1);
    }
  
  char *tmp=(char*)malloc(strlen(key)+1);
  strcpy(tmp, key);
  p[ix].key=tmp;
  p[ix].value=val;

  ix++;
}

int store_search(const char* key)
{
  int i;
  for (i=0;i<ix;i++)
    {
      if (!strcmp(p[i].key, key))
	{
	  return p[i].value;
	}
    }
}

// should probably take a function pointer
// iterator
void store_printall()
{
  int i;
  for (i=0;i<ix;i++)
    {
      printf("%s %x\n", p[i].key, p[i].value);
    }
}
