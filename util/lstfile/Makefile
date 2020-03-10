
all: test mklst


test: mk_symbols.h mk_symbols.c mk_symbols_test.c
	gcc -g -o test mk_symbols.c mk_symbols_test.c


mklst: mk_symbols.h mk_symbols.c mk_symbols_main.c store.c
	gcc -g -o mklst mk_symbols.c mk_symbols_main.c store.c


clean:
	rm -f test mklst *.o a.out *~

