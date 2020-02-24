
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
	  
	  // Search all >1000 entries for each line, we are not yet at collapse :-)
	  for (i=0;i<lim;i++)
	    {
	      // Match?
	      if (0==strncmp(buff, &opc[i][11], bufflen))
		{
		  int n = calc_score(opc[i]);
		  if (debug) printf("i match=%d, str=%s # bytes: %d\n",
				    i, buff, n);
		  break;
		}
	    }
	}
    }
  
}
