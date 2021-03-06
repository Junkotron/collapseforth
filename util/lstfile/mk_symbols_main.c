
#include "store.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "mk_symbols.h"

static const int debug=0;

// To be parsed from opcodes.lst

static char opc[2000][100];

// How far to go in opc vector
static int lim=0;

static int latest_equ=0; // used with @ construct


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


// May be called recursively by the ".inc" pseudo
int read_file(const char* filename, int dollar_pc)
{
  char linebuf[1024];

  FILE* file=fopen(filename, "r");
  if (file==NULL)
    {
      fprintf(stderr, "File: >>%s<< could not be opened\n", filename);
      exit(1);
    }
  
  while (1)
    {
      fgets(linebuf, 1024, file);

      if (feof(file)) break;
      
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

      if (0==strcmp(labelname, ".equ"))
	{
	  int x;
	  sscanf(op1,"%x",&x);
	  if (op1[0]=='@')
	    {
	      latest_equ+=x;
	      store_insert(opcname, latest_equ);
	    }
	  else
	    {
	      store_insert(opcname, x);
	    }
	  continue;
	}
      
      // First check for pseudo directive at label (column 0)
      if (0==strcmp(labelname, ".org"))
	{
	  if (isdigit(opcname[0]))
	    {
	      sscanf(opcname, "%x", &dollar_pc);
	    }
	  else
	    {
	      dollar_pc=store_search(opcname);
	    }
	  continue;	  
	}
      
      if (0==strcmp(labelname, ".inc"))
	{
	  // Remove fnutts, ""
	  char newfname[1024];
	  int newi=0;
	  for (i=0;;i++)
	    {
	      if (linebuf[i+5]=='\0')
		{
		  newfname[newi++]=linebuf[i+5];
		  break;
		}
	      if ((linebuf[i+5]=='\n') ||
		  (linebuf[i+5]=='\r') ||
		  (linebuf[i+5]=='\t'))
		{
		  newfname[newi++]='\0';
		  break;
		}
	      if (linebuf[i+5]!='"') newfname[newi++]=linebuf[i+5];
	    }
	  
	  dollar_pc=read_file(newfname, dollar_pc);
	  continue;
	}

      // Finally we output the labels
      // according to either the current pc
      // or .equ directive but only if
      // label exists for this line
      if (strlen(labelname)!=0)
	{
	  printf("%s %x\n", labelname, dollar_pc);
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

  return dollar_pc;
}


int main(int argc, char* argv[])
{
  FILE* opcf = fopen("opcodes.lst", "r");

  store_init();
  
  int dollar_pc=0;
  int i=0;

  // Add some corrections call (nn) => call nn
  // ld hl,(nn) not with the ED opcode
  // TODO: This wont work if we are going to search via intervall halving
  // then the old JP and CALL has to be replaced and the ED LD version
  // removed
  sprintf(opc[i++], "CD nn      CALL nn\r\n");
  sprintf(opc[i++], "DC nn      CALL C,nn\r\n");
  sprintf(opc[i++], "FC nn      CALL M,nn\r\n");
  sprintf(opc[i++], "D4 nn      CALL NC,nn\r\n");
  sprintf(opc[i++], "C4 nn      CALL NZ,nn\r\n");
  sprintf(opc[i++], "F4 nn      CALL P,nn\r\n");
  sprintf(opc[i++], "EC nn      CALL PE,nn\r\n");
  sprintf(opc[i++], "E4 nn      CALL PO,nn\r\n");
  sprintf(opc[i++], "CC nn      CALL Z,nn\r\n");

  sprintf(opc[i++], "C3 nn      JP nn\r\n");
  sprintf(opc[i++], "DA nn      JP C,nn\r\n");
  sprintf(opc[i++], "FA nn      JP M,nn\r\n");
  sprintf(opc[i++], "D2 nn      JP NC,nn\r\n");
  sprintf(opc[i++], "C2 nn      JP NZ,nn\r\n");
  sprintf(opc[i++], "F2 nn      JP P,nn\r\n");
  sprintf(opc[i++], "EA nn      JP PE,nn\r\n");
  sprintf(opc[i++], "E2 nn      JP PO,nn\r\n");
  sprintf(opc[i++], "CA nn      JP Z,nn\r\n");
  sprintf(opc[i++], "2A nn      LD HL,nn\r\n");
  
  while (1)
    {
      fgets(opc[i], 100, opcf);
      if (feof(opcf)) break;
      fprintf(stderr, "opci=%s\n", opc[i]);
      i++;
    }

  lim = i;
  
  if (argc==2)
    {
      dollar_pc=read_file(argv[1], dollar_pc);
    }
  else
    {
      fprintf(stderr, "Usage: %s <assembler file>\n", argv[0]);
      exit(1);
    }

  store_printall();
  
  store_destroy();  
  printf("$=%x\n", dollar_pc);
}
