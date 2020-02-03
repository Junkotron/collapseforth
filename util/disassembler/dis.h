#ifndef DIS__H
#define DIS__H


/** Need to be sorted, will load opcodes.lst */
void parse_ops(int be, int en);

/** Just a debug of parsing, normally not used */
void dump_rows();

/* When doing disassembly, this util will call user here everytime
   it needs a new byte with the indexin second arg going from 0 and up
*/
typedef unsigned (*get_bytes_fn)(void *, int);

/**
 *
 * dis -  The "*dis is filled in with the mnemonic string
 * fn  -  Is a user provided function to feed the disassembler with one byte
 *        at a time
 * userp - A user provided data structure that gets called in fn
 * 
 * returns: the number of bytes of this opcode
 */
int disasm(char** dis, get_bytes_fn fn, void* userp);

#endif
