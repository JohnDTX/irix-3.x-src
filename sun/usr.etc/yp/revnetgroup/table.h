/*  @(#)table.h 1.1 86/02/05 (C) 1985 Sun Microsystems, Inc. */
/* @(#)table.h	2.1 86/04/16 NFSSRC */ 


#define NUMLETTERS 27 /* 26 letters  + 1 for anything else */
#define TABLESIZE (NUMLETTERS*NUMLETTERS)

typedef struct tablenode *tablelist;
struct tablenode {
	char *key;
	char *datum;
	tablelist next;
};
typedef struct tablenode tablenode;

typedef tablelist stringtable[TABLESIZE];

int tablekey();
char *lookup();
void store();
