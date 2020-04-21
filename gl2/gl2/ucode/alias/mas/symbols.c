/* symbols.c
 *		hashing and table-making for scratch ram symbols
 *		IMPORTS:  hash(s)
 *			  char *strsave(s)	from libh.c
 *
 *	define NOFILECHECK to override current-file declaration check.
 */

#include <stdio.h>
#include "libh.h"

struct sortlist {
    struct slist *kp;		/* points into symtab */
    struct sortlist *next;
};

static struct slist *symtab[HASHSIZE];
static struct sortlist *symsort, *numsort, *symend, *numend;
static struct slist nullsym = {"",0,NULL};
extern short filecounter;


struct slist *_slookup(s)		/* find pointer to existing entry */
   char *s;
{
   register struct slist *np;

   for (np = symtab[hash(s)]; np != NULL; np = np->next)
	if (strcmp(s, np->sym) == 0)	/* search all blocks w/ s's code */
	   return(np);
   return(NULL);		/* not found */
}


scratchlookup(s)
    char *s;
{
	register struct slist *sp;
	struct slist *_slookup();

	if ( (sp = _slookup(s)) == NULL) {
		printf("undefined scratch symbol %s ",s);
		printstate();
		return(0);
	}
#ifndef NOFILECHECK
	if (sp->filedec < filecounter) {
		printf("warning: undeclared scratch symbol %s ",s);
		printstate();
	}
#endif
	return(sp->val);
}


struct slist *scratchinstall(name, value, externflag)
   char *name;				/* hash new symbol into table */
   short value;
   short externflag;
{
   struct slist *np, *_slookup();
   char *strsave(), *malloc();
   int hashval;

   if ((np = _slookup(name)) == NULL)
      {
	if (externflag)
		printf("'external' symbol %s not previously defined\n",name);
	np = (struct slist *) malloc(sizeof(*np));
	if (np == NULL) return(NULL);
	if ((np->sym = strsave(name)) == NULL) return(NULL); /* store name */
	np->filedec = filecounter;
	hashval = hash(np->sym);
	np->next = symtab[hashval];	/* insert new block at head of list */
	symtab[hashval] = np;		/* insert ptr to new block into tab */
	np->val = value;
	return(np);
      }
   else {	/* symbol exists */
	if (!externflag)
	    printf("symbol %s redefined\n",name);  /* symbol must be NEW */
	np->filedec = filecounter;	/* just update file tag */
	return(np);
   }
}


printsymtab(nsyms)
    short nsyms;
{
	short i;
	struct slist *np;
	struct sortlist *sp;

	printf("\nScratch Symbols (%d):\n",nsyms);
	if (nsyms==0) return(0);
	numsort = (struct sortlist *) malloc((nsyms+2)*sizeof(*numsort));
	symsort = (struct sortlist *) malloc((nsyms+2)*sizeof(*symsort));
	symsort->next = NULL;
	numsort->next = NULL;
	symsort->kp = &nullsym;
	numsort->kp = &nullsym;
	symend = symsort;
	numend = numsort;

	for (i=0; i<HASHSIZE; i++) {
		if ((np=symtab[i])==NULL) continue;
		while (np != NULL) {
			insort(np);
			np = np->next;
		}
	}

printf("\nNumeric:\n");
	dosymprint(numsort);
printf("\n\nAlphabetic:\n");
	dosymprint(symsort);
}


insort(np)	/* insertion-sort one symbol table entry */
    struct slist *np;
{
    register short nkey;
    register char *skey;
    register struct sortlist *lp;

/* first numeric sort */

    lp = numsort;
    nkey = np->val;
    while (lp->next != NULL) {
	if (lp->next->kp->val > nkey) break;
	lp = lp->next;
    }
    (++numend)->kp = np;	/* stash the entry in new table locn. */
    numend->next = lp->next;	/* new insertion points to next higher */
    lp->next = numend;		/* next lower item points to new one */


/* now alpha sort */

    lp = symsort;
    skey = np->sym;
    while (lp->next != NULL) {
	if (strcmp(lp->next->kp->sym,skey)>0) break;
	lp = lp->next;
    }
    (++symend)->kp = np;	/* stash the entry in new table locn. */
    symend->next = lp->next;	/* new insertion points to next higher */
    lp->next = symend;		/* next lower item points to new one */
}


dosymprint(pp)
    struct sortlist *pp;
{
    register struct sortlist *sp;
    register i,len;

	len = -1;	/* hack to skip 1st bogus item */
	for (sp = pp,i = 0; sp != NULL; sp = sp->next) {
		if (len<0) {
			len = 0;
			continue;
		}
		printf("%03x %s",sp->kp->val,sp->kp->sym);
		if ((i++ % 3)==2) putchar('\n');
		else {
			putchar('\t');
			if ((len=strlen(sp->kp->sym)) < 4) putchar('\t');
			if (len < 12) putchar('\t');
		}
	}
}
