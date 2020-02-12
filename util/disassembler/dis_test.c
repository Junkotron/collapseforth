
#include "dis.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

unsigned disasm_fn(void* userp, int index)
{
  const unsigned char* p = (const unsigned char *)userp;
  return (unsigned) p[index];
}

void disasm_test(const unsigned char* p, int expected_n)
{
  char* ret;
  int n_bytes=disasm(&ret, disasm_fn, (void*)p);

  /** Fail if we disagree... */
  assert(expected_n == n_bytes);
  
  printf("%d bytes read, Disassembly: %s\n", n_bytes, ret);
  int i;

  /* Will dump the number of bytes the disassembler decided it took
   * for one opcode
   */
  for (i=0;i<n_bytes;i++)
    {
      printf("%.2X ", p[i]);
    }
  printf("\n\n");
  
}

void disasm_tests()
{
  const unsigned char op1[] = {0x00, 0,    0,  0 };
  const unsigned char op2[] = {0xed, 0xa9, 0,  0 };
  const unsigned char op3[] = {0x01, 47,   11, 0 };
  const unsigned char op4[] = {0xfe, 47,   0,  0 };

  disasm_test((void*)op1, 1);
  disasm_test((void*)op2, 2);
  disasm_test((void*)op3, 3);
  disasm_test((void*)op4, 2);
}

void run_tests()
{
  disasm_tests();
}



/* arguments tells us to use only a part of the opcode list */
int main(int argc, char *argv[])
{
  int from=0, to=0;

  if (argc == 3)
    {
      from = atoi(argv[1]);
      to = atoi(argv[2]);
    }

  parse_ops(from, to);

  //  dump_rows();

  run_tests();
  
  return 0;
}
