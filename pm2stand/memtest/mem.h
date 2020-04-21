/*
 * Each byte of memfound represents 1/2 Meg of memory.
 *  1 -> the corresponding 1/2 meg exists
 *  0 -> the corresponding 1/2 meg does not exist
 */
char *memfound;

#ifndef SETJMP
#include "setjmp.h"
#define SETJMP
#endif

jmp_buf	bejmp, aejmp, iejmp, tejmp;

unsigned short mbw_page;
