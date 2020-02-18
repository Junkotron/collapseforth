
#include <stdio.h>
#include <string.h>

char opc[2000][100];


int is_ws(char ch)
{
  // Also \n and \t and \r
  return ( (ch==' ') ||
	   (ch=='\t') ||
	   (ch=='\n') ||
	   (ch=='\r') ||
	   (ch=='\0')
	   );
}

int is_ws_or_comma(char ch)
{
  return ( is_ws(ch) || (ch==',') );
}


char* copy_to_spc(char* name, char* pek)
{
  int i=0;
  while (!is_ws(pek[i]))
    {
      printf("Copying char: %c\n", pek[i]);
      name[i]=pek[i];
      i++;
    }
  name[i]='\0';
  return &pek[i];
}

char* move_to_nonws(char* pek)
{
  int i=0;
  while (is_ws(pek[i]))
    {
      i++;
    }
  return &pek[i];
}


char* copy_to_spc_or_comma(char* name, char* pek)
{
  int i=0;
  while (!is_ws_or_comma(pek[i]))
    {
      printf(",Copying char: %c\n", pek[i]);
      name[i]=pek[i];
      i++;
    }
  name[i]='\0';

  if (pek[i]==',') return &pek[i+1];

  return &pek[i];
}

  
void convert_if_index(char* pek)
{
  if (0==strncmp(pek, "(IX", 3))
    {
      sprintf(pek, "(IX+d)");
    }
  if (0==strncmp(pek, "(IY", 3))
    {
      sprintf(pek, "(IY+d)");
    }
}

static void parse_line(char* linebuff, char* labelname, char* opcname,
		       char* op1, char* op2)
{

  char *pek=linebuff;
  
  if (!is_ws(pek[0]))
    {
      pek=copy_to_spc(labelname, pek);

      printf("labna=%s\n", labelname);
    }

  pek=move_to_nonws(pek);

  pek=copy_to_spc(opcname, pek);
  
  pek=move_to_nonws(pek);

  pek=copy_to_spc_or_comma(op1, pek);
  
  pek=move_to_nonws(pek);

  pek=copy_to_spc(op2, pek);


  // Now lets do some special conversions, first turn any op begining
  // with (IX/IY into (IX/IY+d)

  convert_if_index(op1);
  convert_if_index(op1);
}

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
            
      parse_line(linebuf, labelname, opcname, op1, op2);

      // These lenghts are without spaces and with numbers converted to "X"
      int labellen=strlen(labelname);
      int opcnamelen=strlen(opcname);
      int op1len=strlen(op1);
      int op2len=strlen(op2);

      printf("labelname=%s, labellen=%d\n", labelname, labellen);
      printf("opcodename=%s, opcodelen=%d\n", opcname, opcnamelen);
      printf("op1=%s, op1len=%d\n", op1, op1len);
      printf("op2=%s, op2len=%d\n", op2, op2len);

      return 0;
      
      // Search all >1000 entries for each line, we are not yet at collapse :-)
      for (i=0;i<lim;i++)
	{
	  if (0==strncmp(opcname, &opc[i][11], opcnamelen))
	    {
	      // Match?
	      printf("i match=%d, str=%s\n", i, &opc[i][11]);
	      break;
	    }
	}
    }
  
}
