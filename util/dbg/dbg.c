
#include "dbg.h"
#include "dis.h"
#include "emul.h"
#include <termios.h>


#include <stdio.h>
#include <stdlib.h>

#define MAX_BP 100

static int breakpoints[MAX_BP];


unsigned fn(void* userp, int ix)
{
  byte* b=(byte*)userp;

  // Simple way to avoid buffer overruns
  return b[ix&0xffff];
}



void set_bp(int addr)
{
  int i;
  for (i=0;i<MAX_BP;i++)
    {
      if (breakpoints[i] == -1)
	{
	  breakpoints[i]=addr;
	  break;
	}
    }
}

static void print_dasm(Machine* m)
{
  char *str;
  int n = disasm(&str, fn, &m->mem[m->cpu.PC & 0xffff]);

  printf("%s\n", str);

  printf("%.4x ", m->cpu.PC);

  int i;
  for (i=0;i<n;i++)
    {
      printf("%.2x ", m->mem[m->cpu.PC + i]);
    }
  printf("\n");
  
}

static void execute(Machine* m)
{
  Z80Execute(&m->cpu);
  
  ushort newsp = m->cpu.R1.wr.SP;
  if (newsp != 0 && newsp < m->minsp) {
    m->minsp = newsp;
  }
  
}

static void run_prompt_once(Machine *m)
{
  char buf[1024];

  int i;

  print_dasm(m);
  
  printf("Prompt> ");
  fflush(stdout);

  struct termios termInfo;
  if (tcgetattr(0, &termInfo) == -1) {
    fprintf(stderr, "tcgetattr failed\n");
    exit(1);
  }
  
  termInfo.c_lflag |= ECHO;
  termInfo.c_lflag |= ICANON;
  tcsetattr(0, TCSAFLUSH, &termInfo);

  for (i=0;i<1023;i++)
    {
      buf[i]=getchar();
      if (buf[i]=='\r') break;
      if (buf[i]=='\n') break;
      putchar(buf[i]);
    }
  
  putchar('\n');
  buf[i]='\0';

  termInfo.c_lflag &= ~ECHO;
  termInfo.c_lflag &= ~ICANON;
  tcsetattr(0, TCSAFLUSH, &termInfo);
  
  switch(buf[0])
    {
    case 'h':
      {
	printf("h - This help text\n");
	printf("b - set breakpoint (addr)\n");
	printf("e - erase breakpoint[num]\n");
	printf("a - delete all breakpoints\n");
	printf("l - list breakpoints\n");
	printf("c - continue execution\n");
	printf("n - next instruction\n");
	printf("d - disassemble [(addr)]\n");
	printf("r - register dump\n");
	printf("x - examine memory (addr)\n");
      }
      break;
    case 'b':
      {
	int bpset=0;
	// Find first free slot (-1)
	for (i=0;i<MAX_BP;i++)
	  {
	    if (breakpoints[i]==-1)
	      {
		int bpaddr;
		int status = sscanf(&buf[1],"%x", &bpaddr);

		if (status != 1)
		  {
		    printf("Hex address not given\n");

		    // Just to avoid fault report below
		    bpset = 1;
		    break;
		  }

		printf("Setting breakpoint %d at %.4x\n", i, bpaddr);
		breakpoints[i]=bpaddr;
		bpset = 1;
		break;
	      }
	  }
	if (!bpset) printf("Warning, bp not set we have %d bp's...\n", MAX_BP);
	break;
      }
    case 'e':
      {
	int bpdel;
	int status = sscanf(&buf[1], "%d", &bpdel);
	if (status != 1)
	  {
	    printf("No bp number given, use 'l' to list active bp's\n");
	    break;
	  }
	if (breakpoints[bpdel] == -1)
	  {
	    printf("Break point not active\n");
	    break;
	  }
	printf("Erasing breakpoint %d, with add=%.4x\n", bpdel, breakpoints[bpdel]);
	breakpoints[bpdel] = -1;
	break;
      }
    case 'a':
      {
	printf("Deleting all breakpoints\n");
	for (i=0;i<MAX_BP;i++)
	  {
	    breakpoints[i]=-1;
	  }
	// Give a last prompt to allow for a bp set
	return;
      }
    case 'l':
      {
	printf("Breakpoint list:\n");
	for (i=0;i<MAX_BP;i++)
	  {
	    if (breakpoints[i]!=-1)
	      {
		printf(" Number: %d, addr: %.4x\n", i, breakpoints[i]);
	      }
	  }
	break;
      }
    case 'c':
      {
	/* Step passed current op */
	execute(m);
	
	int flag=0;
	while (1)
	  {
	
	    for (i=0;i<MAX_BP;i++)
	      {
		if (breakpoints[i]!=-1)
		  {
		    if (m->cpu.PC == breakpoints[i])
		      {
			flag=1;
			break;
		      }
		  }
	      }

	    if (flag) break;
	    execute(m);
	    
	  }
	break;
      }
    case 'n':
      {
	ushort op = m->mem[m->cpu.PC];

	// Just check for plain "CALL nn" for now
	if (op == 0xCD)
	  {
	    int retaddr = m->cpu.PC+3;

	    while (retaddr != m->cpu.PC)
	      {
		execute(m);
	      }
	  }
	else
	  {
	    // Just one instruction
	    execute(m);
	  }

	return;
	break;
      }
    case 'd':
      {
	int addr;
	int status = sscanf(&buf[1], "%x", &addr);

	if (status != 1)
	  {
	    addr = m->cpu.PC;
	  }

	char *str;
	int n = disasm(&str, fn, &m->mem[addr]);
	    
	printf("addr %.4x: ", addr);
	for (i=0;i<n;i++)
	  {
	    printf("%.2x ", m->mem[addr+i]);
	  }
	printf("%s\n", str);
	    
	break;
      }
    case 'r':
      {
	printf("AF=%.4x BC=%.4x DE=%.4x HL=%.4x\n", m->cpu.R1.wr.AF, m->cpu.R1.wr.BC, m->cpu.R1.wr.DE, m->cpu.R1.wr.HL);
	printf("IX=%.4x IY=%.4x SP=%.4x PC=%.4x\n", m->cpu.R1.wr.IX, m->cpu.R1.wr.IY, m->cpu.R1.wr.SP, m->cpu.PC);
	break;
      }
    case 'x':
      {
	int addr;
	int status = sscanf(&buf[1], "%x", &addr);

	if (status != 1)
	  {
	    addr = m->cpu.PC;
	  }

	printf("addr %.4x: ", addr);
	for (i=0;i<16;i++)
	  {
	    printf("%.2x ", m->mem[addr+i]);
	  }
	printf("\n");
	    
	break;
      }
    default:
      {
	printf("Unknown command\n");
	break;
      }
    }
  return;
}

void run_prompt(Machine* m)
{
  parse_ops(0,0);
  bp_init();

  while(1)
    {
      run_prompt_once(m);
    }
}

void bp_init()
{
  int i;
  for (i=0;i<MAX_BP;i++)
    {
      breakpoints[i]=-1;
    }
}

