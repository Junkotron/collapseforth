#ifndef mk_symbols__h
#define mk_symbols__h

// is this an unofficial instruction?
// used only by test externally
// "bit 0,(IX-42) will not return true...
int is_weird(char* opc, char* op1, char* op2);

void parse_line(char* linebuff, char* labelname, char* opcname,
		       char* op1, char* op2);


#endif
