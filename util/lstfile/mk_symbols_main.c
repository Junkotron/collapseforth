
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "mk_symbols.h"

static const int debug=1;

// To be parsed from opcodes.lst
static char opc[2000][100];

static int calc_score(char* linebuff)
{
  // Parse the first columns to determine number of bytes taken
  int score=0;
  int i;
  
  for (i=0;i<11;i++)
    {
      // 0..9 A..F scores one point a..z scores two points
      // result is divided by two to get # bytes in this mnemonic
      char ch = linebuff[i];

      if ( (ch >= '0' && ch <= '9') || (ch>='A' && ch <='F') )
	{
	  score++;
	}

      if (ch >= 'a' && ch <= 'z')
	{
	  score += 2;
	}
    }
  return score/2;
}


int main()
{
  FILE* opcf = fopen("opcodes.lst", "r");

  int i=0;
  
  while (!feof(opcf))
    {
      fgets(opc[i], 100, opcf);
      i++;
    }

  int lim = i;

  char linebuf[1024];

  int dollar_pc=0;
  
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

      // Set a null at a semi-colon, terminating line
      int quote_on=0;
      int i;
      for (i=0;;i++)
	{
	  if (linebuf[i]=='\0') break;
	  
	  if (quote_on==0)
	    {
	      if (linebuf[i]==';')
		{
		  linebuf[i]='\0';
		  break;
		}
	    }
	  if (linebuf[i]=='"')
	    {
	      quote_on = !quote_on;
	    }
	}

      char *rest;
      parse_line(linebuf, labelname, opcname, op1, op2, &rest);

      // First check for pseudo directive at label (column 0)
      if (0==strcmp(labelname, ".org"))
	{
	  // TODO: Check symbol numeric values (for opcode position)
	  sscanf(opcname, "%x", &dollar_pc);
	  continue;
	}
      
      // These lenghts are without spaces and with numbers converted to "X"
      int labellen=strlen(labelname);
      int opcnamelen=strlen(opcname);
      int op1len=strlen(op1);
      int op2len=strlen(op2);

      if (debug) printf("labelname=%s, labellen=%d\n", labelname, labellen);
      if (debug) printf("opcodename=%s, opcodelen=%d\n", opcname, opcnamelen);
      if (debug) printf("op1=%s, op1len=%d\n", op1, op1len);
      if (debug) printf("op2=%s, op2len=%d\n", op2, op2len);

      // TODO: If label!="" set symbol value to current pc ($) and dump to lst file
      
      if (opcname[0] != '\0')
	{
	  char buff[1024];
	  
	  if (op1[0]!='\0')
	    {
	      if (op2[0]!='\0')
		{
		  sprintf(buff,"%s %s,%s",
			  opcname,op1,op2);
		}
	      else
		{
		  sprintf(buff,"%s %s",
			  opcname,op1);

		}
	    }
	  else
	    {
	      sprintf(buff,"%s",
		      opcname);
	    }
	  if (debug) printf("buff=%s\n", buff);

	  int bufflen=strlen(buff);
	  
	  int found=0;
	  
	  // Search all >1000 entries for each line, we are not yet at collapse :-)
	  for (i=0;i<lim;i++)
	    {
	      // Match?
	      if (0==strncmp(buff, &opc[i][11], bufflen))
		{
		  int n = calc_score(opc[i]);
		  dollar_pc += n;
		  if (debug) printf("i match=%d, str=%s # bytes: %d\n",
				    i, buff, n);
		  found = 1;
		  break;
		}
	    }
	  
	  if (!found)
	    {
	      // Check pseudo ops

	      // We have at least one item
	      int n=1;
	      
	      if (0==strcmp(".DB", opcname) || 0==strcmp(".DW", opcname))
		{
		  int ix=0;

		  int state = 0;
		  
		  while (1)
		    {
		      if (rest[ix]=='\0') break;
		      if (state == 0 && rest[ix]==',')
			{
			  n++;
			}
		      else if (state == 0 && rest[ix]=='"')
			{
			  // Start quote
			  state = 1;
			}
		      else if (state == 1 && rest[ix]=='"')
			{
			  state = 0;
			  n--;
			  // End quote
			}
		      else if (state == 1)
			{
			  n++;
			}
		      ix++;
		    }
		  // Double up on bytes?
		  if (0==strcmp(".DW", opcname))
		    {
		      n *= 2;
		    }
		  dollar_pc += n;
		}
	    }
	    
	}
    }
  printf("$=%x\n", dollar_pc);
}
