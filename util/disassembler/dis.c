
#include "dis.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MAX_OP_BYTES 4
#define TOTAL_OP_ROWS 1269


struct row {
  char* ops[MAX_OP_BYTES];
  char* mnem;
};


struct row r[TOTAL_OP_ROWS];
int ix=0;

// Create a string until null via malloc
char* mk_string(char* str)
{
  int i=0;
  
  while(1)
    {
      if (str[i]=='\0')
	{
	  break;
	}
      i++;
    }

  char* retval = (char*)malloc(i*sizeof(char));

  i=0;
  while(1)
    {
      retval[i] = str[i];
      if (str[i]=='\0')
	{
	  break;
	}
      i++;
    }

  return retval;
  
}


void add_row(char ops[MAX_OP_BYTES][3], char* mnem)
{
  int i;

  for (i=0;i<MAX_OP_BYTES;i++)
    {
      r[ix].ops[i] = mk_string(ops[i]);
    }
  r[ix].mnem = mk_string(mnem);
  ix++;
}

void dump_rows()
{
  int i;

  for (i=0;i<ix;i++)
    {
      printf("%s %s %s %s : %s\n",
	     r[i].ops[0],
	     r[i].ops[1],
	     r[i].ops[2],
	     r[i].ops[3],
	     r[i].mnem
	     );
    }
}



/* This is a dummy, we are supposed to read a file..*/
/*

void add_rows_dummy()
{

  char* tmp1[] = { "00", "", "", ""};
  add_row(tmp1, "NOP");

  char* tmp2[] = { "ED", "A9", "", ""};
  add_row(tmp2, "CPD");

  char* tmp3[] = { "ED", "B9", "", ""};
  add_row(tmp3, "CPDR");

  char* tmp4[] = { "FE", "n", "n", "" };
  add_row(tmp4, "LD HL,(nn)");

}
*/


static int is_caps_hexnum(char h)
{
  return ( (h>='A' && h<='F') || (h>='0' && h<='9') );
}


void parse_ops(int be, int en)
{
  FILE* lst = fopen("opcodes.lst_sorted", "r");
  char buf[100];

  /* skip be number of lines */
  int i;
  for (i=0;i<be;i++){
    fgets(buf, 100, lst);
  }
  
  // Sans spaces
  char opcbuff[10];

  char opcodes[4][3];

  int lncnt=0;
  
  while (1)
    {
      lncnt++;

      
      int i;
      for (i=0;i<10;i++) opcbuff[i]='\0';

      for (i=0;i<4;i++) opcodes[i][0]='\0';
      
      fgets(buf, 100, lst);

      for (i=0;i<100;i++)
	{
	  if (buf[i]==10)
	    {
	      buf[i]='\0';
	      break;
	    }
	  if (buf[i]==13)
	    {
	      buf[i]='\0';
	      break;
	    }
	}

      int hexpek=0;
      
      // Move all opcodes in buffert without spaces
      int parseix=0;
      for (i=0;i<10;i++)
	{
	  if (buf[i]!=' ')
	    {
	      opcbuff[parseix++]=buf[i];
	    }
	}
      
      int opix=0;
      i=0;
      while (1)
	{
	  if (opcbuff[i]==0) break;
	  
	  if (is_caps_hexnum(opcbuff[i]))
	    {
	      // Is there a second?
	      if (!is_caps_hexnum(opcbuff[i+1]))
		{
		  fprintf(stderr, "Half baked hex digit number, bailing out\n");
		  exit(1);
		}
	      else
		{
		  // Throw these into one box
		  opcodes[opix][0]=opcbuff[i];
		  opcodes[opix][1]=opcbuff[i+1];
		  opcodes[opix][2]='\0';

		  opix++;
		  i+=2;
		}
	    }
	  else
	    {
	      // If we get here we have a wildcard
	      opcodes[opix][0]=opcbuff[i];
	      opcodes[opix][1]='\0';

	      opix++;
	      i++;
	    }
	}
      if (en==lncnt) break;
      if (feof(lst)) break;
      add_row(opcodes, &buf[10]);
    }

  fclose(lst);
}

// Will set from and to to including limits of this "database select"
void search(const char* opc, int pos, int* from, int* to)
{
  int i;
  
  int froms = *from;
  int tos = *to;
  
  // Set to zero when first match found
  int first_match=1;

  for (i=froms;i<=tos;i++)
    {
      // Check if they are equal or if table contains wildcards
      // (All lower case letters are wildcards)
      char* tabletmp = r[i].ops[pos];
      
      if (!strcmp(opc, tabletmp) || (tabletmp[0] >= 'a') )
	{
	  if (first_match)
	    {
	      *from = i;
	      first_match=0;
	    }
	  *to = i;
	}
    }

  // No match means we return -1, -1
  if (first_match)
    {
      *to=*from=-1;
    }
  
}


const char* get_mnem(int ix)
{
  return r[ix].mnem;
}

int disasm(char** dis, get_bytes_fn fn, void* userp)
{
  int from=0, to=ix-1;
  int oldfrom, oldto;
  int i;
  
  for (i=0;i<4;i++)
    {
      unsigned char op = fn(userp, i);
      char opstr[100];
      sprintf(opstr, "%.2X", op);
      search(opstr, i, &from, &to);
      if (from==-1) break;
      oldfrom = from;
      oldto = to;
    }
  
  *dis=r[oldfrom].mnem;
  return i;
}
