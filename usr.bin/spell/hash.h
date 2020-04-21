/*	@(#)hash.h	1.1	*/
#define HASHWIDTH 27
#define HASHSIZE 134217689L	/*prime under 2^HASHWIDTH*/
#define INDEXWIDTH 9
#define INDEXSIZE (1<<INDEXWIDTH)
#define NI (INDEXSIZE+1)
#define ND ((25750/2)*sizeof(*table))
#define BYTE 8

extern unsigned *table;
extern int index[];	/*into dif table based on hi hash bits*/

extern long hash();

/*
 * MAXIMUM number of characters in a word!
 * Unless you understand how this program works, don't change it!
 */
#define NC 30
