/* libh -- hashing functions for mapping symbols to integers  */
/* see Kernighan/Ritchie  */

#include <stdio.h>
#include "libh.h"

extern FILE symf;
static struct nlist *hashtab[HASHSIZE];		/* table of buckets */

hash(s)			/* come up with a hash code (quickly) */
   register char *s;
{
   register int hashval;

   for (hashval = 0; *s != '\0';)
	hashval = (hashval<<1) + *s++;	/* the "as" scheme */
   return(hashval & HASHMASK);		/* modulo no. of buckets */
}

struct nlist *lookup(s)		/* find pointer to existing entry */
   char *s;
{
   struct nlist *np;

   for (np = hashtab[hash(s)]; np != NULL; np = np->next)
	if (strcmp(s, np->sym) == 0)	/* search all blocks w/ s's code */
	   return(np);
   return(NULL);		/* not found */
}

ilookup(s)		/* find integer field of an existing entry */
   char *s;
{
   struct nlist *np;

   for (np = hashtab[hash(s)]; np != NULL; np = np->next)
	if (strcmp(s, np->sym) ==0)
	   return(np->val);
   return(-1);			/* not found */
}

struct nlist *install(name, value)	/* hash new symbol into table */
   char *name;
   short value;
{
   struct nlist *np, *lookup();
   char *strsave(), *malloc();
   int hashval;

   if ((np = lookup(name)) == NULL)
      {
	np = (struct nlist *) malloc(sizeof(*np));
	if (np == NULL) return(NULL);
	if ((np->sym = strsave(name)) == NULL) return(NULL); /* store name */
	hashval = hash(np->sym);
	np->next = hashtab[hashval];	/* insert new block at head of list */
	hashtab[hashval] = np;		/* insert ptr to new block into tab */
      }
   else
      {
	printf("symbol %s redefined\n",name);	/* symbol must be NEW */
	fprintf(symf,"symbol %s redefined\n",name);
	return(NULL);
      };
   np->val = value;			/* store numeric value */
   return(np);
}

 char *strsave(s)		/* allocate space for a string */
   char *s;
{
   char *p, *malloc();

   if ((p = malloc(strlen(s)+1)) != NULL)
	strcpy(p,s);
   return(p);
}
