
#ifndef __STORE__H
#define __STORE__H

void store_init();

void store_destroy();

void store_insert(const char *key, int val);

int store_search(const char*key);

void store_printall();

#endif
