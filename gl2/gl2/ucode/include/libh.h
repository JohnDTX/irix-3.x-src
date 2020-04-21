/* libh.h -- for hashing functions in libh.c, symbols.c
 */

struct nlist	{		/* each block entered */
	char *sym;		/* name of symbol */
	short val;		/* integer value, e.g. bit position */
	struct nlist *next;	/* link pointer */
};

struct slist	{		/* each block entered */
	char *sym;		/* name of symbol */
	short val;		/* integer value, e.g. bit position */
	short filedec;		/* whether declared in this file */
	struct slist *next;	/* link pointer */
};

#define HASHSIZE 128			/* no. of buckets */
#define HASHMASK 0x7f			/* corresponding bitmask */
