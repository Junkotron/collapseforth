
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "mk_symbols.h"

static const int debug=1;

// To be parsed from opcodes.lst
static char opc[2000][100];


int main()
{
  FILE* opcf = fopen("libz80/codegen/opcodes.lst", "r");

  int i=0;
  
  while (!feof(opcf))
    {
      fgets(opc[i], 100, opcf);
      i++;
    }

  int lim = i;

  char linebuf[1024];
  
  while (!feof(stdin))
    {
      fgets(linebuf, 1024, stdin);
      
      char labelname[100];
      char opcname[100];
      char op1[100];
      char op2[100];

      labelname[0]='\0';
      opcname[0]='\0';
      op1[0]='\0';
      op2[0]='\0';
            
      parse_line(linebuf, labelname, opcname, op1, op2);

      // These lenghts are without spaces and with numbers converted to "X"
      int labellen=strlen(labelname);
      int opcnamelen=strlen(opcname);
      int op1len=strlen(op1);
      int op2len=strlen(op2);

      if (debug) printf("labelname=%s, labellen=%d\n", labelname, labellen);
      if (debug) printf("opcodename=%s, opcodelen=%d\n", opcname, opcnamelen);
      if (debug) printf("op1=%s, op1len=%d\n", op1, op1len);
      if (debug) printf("op2=%s, op2len=%d\n", op2, op2len);

      // Search all >1000 entries for each line, we are not yet at collapse :-)
      for (i=0;i<lim;i++)
	{
	  if (0==strncmp(opcname, &opc[i][11], opcnamelen))
	    {
	      // Match?
	      if (debug) printf("i match=%d, str=%s\n", i, &opc[i][11]);
	      break;
	    }
	}
    }
  
}
