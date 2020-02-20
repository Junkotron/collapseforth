
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "mk_symbols.h"

static const int debug=0;

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

  if (debug) printf("1 pek char on: %d (%d)\n", *pek, *(pek+1));
  if (*pek == '\0') return pek;
  
  int i=0;
  while (!is_ws(pek[i]))
    {
      if (debug) printf("Copying char: %c\n", pek[i]);
      name[i]=pek[i];
      i++;
    }
  name[i]='\0';
  return &pek[i];
}

char* move_to_nonws(char* pek)
{
  if (debug) printf("2 pek char on: %d (%d)\n", *pek, *(pek+1));
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
  if (debug) printf("3 pek char on: %d (%d)\n", *pek, *(pek+1));
  if (*pek == '\0') return pek;

  int i=0;
  while (!is_ws_or_comma(pek[i]))
    {
      if (debug) printf(",Copying char: %c\n", pek[i]);
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


int is_not_reg8(char* id)
{
  static const char* regs[] =
    {
      "A","B","C","D","E","H","L",
      "(HL)", "(IX+d)", "(IY+d)",
      ""
    };
  return !is_any_of(id, regs);
}

int is_in_or_out(char* mnem)
{
  static const char* io[] = { "IN", "OUT", "" };
  return is_any_of(mnem, io);
}

void convert_if_pos_n_or_nn(char* op, char* opcname)
{
  // list of possible reg constructs
  // if recognizable reg we return directly
  static const char* regs[]={
    "A", "B", "C", "D", "E", "H", "L",
    "BC", "DE", "HL", "SP", "IX", "IY", 
    "(BC)", "(DE)", "(HL)", "(SP)", "(IX+d)", "(IY+d)", 
    ""
  };
  if (op[0] == '(')
    {
      if (is_in_or_out(opcname))
	{
	  sprintf(op, "(n)");
	}
      else
	{
	  if (!is_any_of(op, regs))
	    {
	      sprintf(op, "(nn)");
	    }
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

  if (is_any_of(opcname, ops) && is_not_reg8(op1))
    {
      if (debug) printf("logical conv\n");
      sprintf(op1, "n");
    }
}

// if any of ADC, ADD, SBC, SUB we check for
// n but only if op1 is "a"
void convert_if_arithmetic_n(char* op1, char* op2, char* opcname)
{
  const char* ops[] = { "ADC",
			"ADD",
			"SBC",
			"SUB",
			""};

  // Return if anything but A
  if (strcmp(op1,"A")) return;

  if (is_any_of(opcname, ops) && is_not_reg8(op2))
    {
      if (debug) printf("arithm conv\n");
      sprintf(op2, "n");
    }
}

void convert_if_numeral(char* op1, char* op2, char* opcname)
{
  static const char *nreg[]={"A", "B","C", "D","E", "H", "L",
			     "(BC)", "(DE)", "(HL)", "(SP)",
			     "(IX+d)", "(IY+d)",
			     ""};
  
  // only do stuff for the LD mnem
  if (strcmp(opcname, "LD")) return;

  // no changes if any op is (nn)
  if (!strcmp(op1, "(nn)")) return;
  if (!strcmp(op2, "(nn)")) return;
  
  if (!is_any_of(op1, nreg))
    {
      sprintf(op2, "n");
    }
}

void parse_line(char* linebuff, char* labelname, char* opcname,
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

  if (debug) printf("1) op1=%s\n", op1);
  if (debug) printf("1) op2=%s\n", op2);
  
  // Now check for the (nn) or (n) construct in both arguments, only "IN"
  // and "OUT" has (n) all others are (nn)
  convert_if_pos_n_or_nn(op1, opcname);
  convert_if_pos_n_or_nn(op2, opcname);

  if (debug) printf("2) op1=%s\n", op1);
  if (debug) printf("2) op2=%s\n", op2);

  // Now a set of [A,]n opcodes are always n, not nn
  // if any of ADC, ADD, AND, CP, OR, SBC, SUB, XOR
  convert_if_acc_with_n(op1, opcname);
  convert_if_arithmetic_n(op1, op2, opcname);
  
  if (debug) printf("3) op1=%s\n", op1);
  if (debug) printf("3) op2=%s\n", op2);
  // Now for the final peculiarity, is any literal number argument n or nn?
  // for the notorious "LD" opcode, "n", it is if the other opcode is any of
  // ABCDEHL (HL) (DE) (BC) (IX+d) (IY+d)
  convert_if_numeral(op1, op2, opcname);

  
}

