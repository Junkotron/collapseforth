
#include <stdio.h>
#include <string.h>
#include <ctype.h>





// To be parsed from opcodes.lst
static char opc[2000][100];


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

  printf("1 pek char on: %d (%d)\n", *pek, *(pek+1));
  if (*pek == '\0') return pek;
  
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
  printf("2 pek char on: %d (%d)\n", *pek, *(pek+1));
  if (*pek == '\0') return pek;

  int i=0;
  while (is_ws(pek[i]))
    {
      if (pek[i] == '\0') break;
      i++;
    }
  return &pek[i];
}


char* copy_to_spc_or_comma(char* name, char* pek)
{
  printf("3 pek char on: %d (%d)\n", *pek, *(pek+1));
  if (*pek == '\0') return pek;

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

int is_any_of(char* str, const char *lst[])
{
  int i=0;
  while (lst[i][0] != '\0')
    {
      if (0==strcmp(str, lst[i])) return 1;
      i++;
    }
  return 0;
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

void convert_to_upper(char* pek)
{
  int i=0;

  while (pek[i]!='\0')
    {
      pek[i]=toupper(pek[i]);
      i++;
    }
}

 
int is_relative_jump(char* opcname)
{
  static const char* jrdjnz[] = { "JR", "DJNZ", "" };
  return is_any_of(opcname, jrdjnz);
}

int is_c_nc_z_nz(char* flag)
{
  static const char* z_c_flag[] = { "C", "NC", "Z", "NZ", ""};
  return is_any_of(flag, z_c_flag);
}


int is_not_reg(char* id)
{
  static const char* regs[] = { "A","B","C","D","E","H","L",
			      "BC","DE","HL","SP",
			      "IX", "IY", 
			      ""};
  return !is_any_of(id, regs);
}

int is_in_or_out(char* mnem)
{
  static const char* io[] = { "IN", "OUT", "" };
  return is_any_of(mnem, io);
}

void convert_if_pos_n_or_nn(char* op, char* opcname)
{
  if (op[0] == '(' && is_not_reg(&op[1]))
    {
      if (is_in_or_out(opcname))
	{
	  sprintf(op, "(n)");
	}
      else
	{
	  sprintf(op, "(nn)");	  
	}
    }
}

// if any of AND, CP, OR, XOR
void convert_if_acc_with_n(char* op1, char* opcname)
{
  const char* ops[] = { "AND",
			"CP",
			"OR",
			"XOR",
			""};

  if (is_any_of(opcname, ops) && is_not_reg(op1))
    {
      sprintf(op1, "n");
    }
}

// if any of ADC, ADD, SBC, SUB
void convert_if_arithmetic_n(char* op2, char* opcname)
{
  const char* ops[] = { "ADC",
			"ADD",
			"SBC",
			"SUB",
			""};
    
  if (is_any_of(opcname, ops) && is_not_reg(op2))
    {
      sprintf(op2, "n");
    }
}


void convert_if_numeral(char* op1, char* op2, char* opcname)
{
  
}

static void parse_line(char* linebuff, char* labelname, char* opcname,
		       char* op1, char* op2)
{

  char *pek=linebuff;
  
  if (!is_ws(pek[0]))
    {
      pek=copy_to_spc(labelname, pek);
    }

  pek=move_to_nonws(pek);

  pek=copy_to_spc(opcname, pek);
  
  pek=move_to_nonws(pek);

  pek=copy_to_spc_or_comma(op1, pek);
  
  pek=move_to_nonws(pek);

  pek=copy_to_spc(op2, pek);


  // Convert mnem and operands to upper case, labels are kept case-sensitive
  convert_to_upper(opcname);
  convert_to_upper(op1);
  convert_to_upper(op2);
  
  // Now lets do some special conversions, first turn any op begining
  // with (IX/IY... into (IX/IY+d)

  convert_if_index(op1);
  convert_if_index(op2);

  // Now lets deal with the silly jr and djnz
  // Just check if op1 is "C/NC/Z/NZ", ignore that djnz does not take flag
  // args..
  if (is_relative_jump(opcname))
    {
      if (is_c_nc_z_nz(op1))
	{
	  // If so, convert op2 into "(PC+e)" pattern
	  sprintf(op2, "(PC+e)");
	}
      else
	{
	  // else convert op1 for djnz and unconditional
	  sprintf(op1, "(PC+e)");
	}
    }

  // Now check for the (nn) or (n) construct in both arguments, only "IN"
  // and "OUT" has (n) all others are (nn)
  convert_if_pos_n_or_nn(op1, opcname);
  convert_if_pos_n_or_nn(op2, opcname);

  // Now a set of [A,]n opcodes are always n, not nn
  // if any of ADC, ADD, AND, CP, OR, SBC, SUB, XOR
  convert_if_acc_with_n(op1, opcname);
  convert_if_arithmetic_n(op2, opcname);
  
  // Now for the final peculiarity, is any literal number argument n or nn
  // for the notorious "LD" opcode, "n" it is if the other opcode is any of
  // ABCDEHL (HL) (DE) (BC) (IX+d) (IY+d)
  convert_if_numeral(op1, op2, opcname);
  convert_if_numeral(op2, op1, opcname);
  
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
