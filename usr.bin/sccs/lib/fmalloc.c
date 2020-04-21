static char sccsid[] = "@(#)fmalloc.c	1.5";

/*
	The functions is this file replace xalloc-xfree-xfreeall from
	the PW library.

	Xalloc allocated words, not bytes, so this adjustment is made
	here.  This inconsistency should eventually be cleaned-up in the
	other source, i.e. each request for memory should be in bytes.

	These functions are complicated by the fact that libc has no
	equivalent to ffreeall.  This requires that pointers to allocated
	arrays be stored here.  If malloc ever has a freeall associated with
	it, most of this code can be discarded.
*/

#define LCHUNK	100
#define NULL	0

unsigned	ptrcnt = 0;
unsigned	listsize = 0;
char	**ptrlist = NULL;
void	free();

char *
fmalloc(asize)
unsigned asize;
{
	char *ptr, *malloc(), *realloc();

	if (listsize == 0) {
		listsize = LCHUNK;
		if ((ptrlist = (char **)malloc(sizeof(char *)*listsize)) == NULL)
			fatal("OUT OF SPACE (ut9)");
	}
	if (ptrcnt >= listsize) {
		listsize += LCHUNK;
		if ((ptrlist = (char **)realloc((char *)ptrlist,
					sizeof(char *)*listsize)) == NULL)
			fatal("OUT OF SPACE (ut9)");
	}

	if ((ptr = malloc(sizeof(int)*asize)) == NULL)
		fatal("OUT OF SPACE (ut9)");
	else
		ptrlist[ptrcnt++] = ptr;
	return(ptr);
}

ffree(aptr)
char *aptr;
{
	register unsigned cnt;

	cnt = ptrcnt;
	while (cnt)
		if (aptr == ptrlist[--cnt]) {
			free(aptr);
			if (cnt == ptrcnt - 1)
				--ptrcnt;
			else
				ptrlist[cnt] = NULL;
			return;
		}
	fatal("ffree: Pointer not pointing to allocated area");
}

ffreeall()
{
	while(ptrcnt)
		if (ptrlist[--ptrcnt] != NULL)
			free(ptrlist[ptrcnt]);
	if (ptrlist != NULL)
		free((char *)ptrlist);
	ptrlist = NULL;
	listsize = 0;
}
