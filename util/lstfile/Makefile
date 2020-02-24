
all: test mklst


test: mk_symbols.h mk_symbols.c mk_symbols_test.c
	gcc -o test mk_symbols.c mk_symbols_test.c

mklst: mk_symbols.h mk_symbols.c mk_symbols_main.c
	gcc -o mklst mk_symbols.c mk_symbols_main.c


clean:
	rm -f test mklst *.o a.out *~

