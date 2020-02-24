
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "mk_symbols.h"

static const int debug=0;

// To be parsed from opcodes.lst
static char opc[2000][100];

// Trim nl
static void trim(char* str)
{
  int i;
  for (i=0;;i++)
    {
      if (str[i] == '\r')
	{
	  str[i]='\0';
	  return;
	}
      if (str[i] == '\n')
	{
	  str[i]='\0';
	  return;
	}
      if (str[i] == '\0')
	{
	  return;
	}
    }
}

// replace nn into numbers
void replace_num(char *arg)
{
  if (0==strcmp(arg,"nn"))
    {
      sprintf(arg, "4711");
      return;
    }
  if (0==strcmp(arg,"(nn)"))
    {
      sprintf(arg, "(4711)");
      return;
    }
  if (0==strcmp(arg,"n"))
    {
      sprintf(arg, "42");
      return;
    }
  if (0==strcmp(arg,"(n)"))
    {
      sprintf(arg, "(42)");
      return;
    }
  if (0==strcmp(arg,"PC+e"))
    {
      sprintf(arg, "123");
      return;
    }
  if (0==strcmp(arg,"(IX+d)"))
    {
      sprintf(arg, "(IX-42)");
      return;
    }
  if (0==strcmp(arg,"(IY+d)"))
    {
      sprintf(arg, "(IY+42)");
      return;
    }
  
}

void run_test(int subst)
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

  // test with each mnem in list
  for (i=0;i<lim;i++)
    {
      printf("Parsing: %s\n",opc[i]);
      sprintf(linebuf, "%s", &opc[i][10]);
      
      char labelname[100];
      char opcname[100];
      char op1[100];
      char op2[100];

      labelname[0]='\0';
      opcname[0]='\0';
      op1[0]='\0';
      op2[0]='\0';
            

      // replace nn etc. with numbers
      if (subst)
	{
	  if (!is_weird(opcname, op1, op2))
	    {
	      replace_num(op1);
	      replace_num(op2);
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

      // Ok, make this into a string and compare with the original
      char back[1024];
      char b1[1024];
      char b2[1024];
      char orig[1024];

      back[0]='\0';
      b1[0]='\0';
      b2[0]='\0';
      orig[0]='\0';

      sprintf(back, "%s", opcname);
      if (op1[0]!='\0')
	{
	  sprintf(b1, "%s %s", back, op1);
	}
      else
	{
	  sprintf(b1, "%s", back);
	}
      if (op2[0]!='\0')
	{
	  sprintf(b2, "%s,%s", b1, op2);
	}
      else
	{
	  sprintf(b2, "%s", b1);	      
	}

      sprintf(orig, "%s", &opc[i][11]);

      trim(b2);
      trim(orig);
      
      if (strcmp(b2, orig))
	{
	  printf("Test Failed: >>>%s<<< original= >>>%s<<<\n", b2, orig);
	  exit(1);
	}

    }
}

int main()
{
  run_test(0);
  run_test(1);
}
