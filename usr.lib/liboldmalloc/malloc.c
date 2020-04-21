/*
 *
 * CONTENTS:
 *	C storage allocator stolen and hacked up from UNIX for the SUN
 *		malloc		)
 *		free		) -> UNIX
 *		realloc		)
 *		allock		) DEBUG ONLY!
 *		SetBrk		! internal fudge to do sbrk()
 *
 * AUTHORS: stolen and hacked by Andrew I. Shore
 *
 * MAINTAINER: shore
 *
 * HISTORY:
 * 03/11/82 AIS replaced calls to UNIX sbrk() with calls to own version
 *          SetBrk() for the SUN & Vkernel
 *
 * 05/03/82 AIS changed to work with new multi-team Vkernel -- rather
 *	    than using emt_getchar and _end, now uses the kernel operations
 *	    GetTeamSize and SetTeamSize which more closely approximate
 *	    UNIX sbrk.
 *
 * 05/04/82 AIS fixed some bugs introduced yesterday. Decided to `free' space
 *	    added if SetTeamSize could not grant entire request.
 *
 * 12/08/83 GMT hacked it to make it go twice as fast
 *
 *  6/07/84 GMT added malloc_error_nomem - if non-null then this user settable
 *		procedure gets called when malloc runs out of memory
 *
 *  6/18/84 GMT rename SetBrk to sbrk and added ifdefs to make work on an IRIS
 *		workstation running UNIX SYSTEM V
 *
 *  6/20/84 GMT added free byte counter (alloc_free) so as to speed it up some
 */

extern char *sbrk();

int (*malloc_error_nomem) ();

#ifdef debug
#define ASSERT(p) if(!(p))botch("p");else
botch(s)
char *s;
{
	printf("assertion botched: %s\n",s);
	abort();
}
#else
#define ASSERT(p)
#endif

/*	avoid break bug */
#ifdef pdp11
#define GRANULE 64
#else
#define GRANULE 0
#endif
/*	C storage allocator
 *	circular first-fit strategy
 *	works with noncontiguous, but monotonically linked, arena
 *	each block is preceded by a ptr to the (pointer of) 
 *	the next following block
 *	blocks are exact number of words long 
 *	aligned to the data type requirements of ALIGN
 *	pointers to blocks must have BUSY bit 0
 *	bit in ptr is 1 for busy, 0 for idle
 *	gaps in arena are merely noted as busy blocks
 *	last block of arena (pointed to by alloct) is empty and
 *	has a pointer to first
 *	idle blocks are coalesced during space search
 *
 *	a different implementation may need to redefine
 *	ALIGN, NALIGN, BLOCK, BUSY, INT
 *	where INT is integer type to which a pointer can be cast
*/
#define INT int
#define ALIGN int
#define NALIGN 1
#define WORD sizeof(union store)
#define BLOCK 1024	/* a multiple of WORD*/
#define BUSY 1
#define NULL 0
#define testbusy(p) ((INT)(p)&BUSY)
#define setbusy(p) (union store *)((INT)(p)|BUSY)
#define clearbusy(p) (union store *)((INT)(p)&~BUSY)

union store { union store *ptr;
	      ALIGN dummy[NALIGN];
	      int calloc;	/*calloc clears an array of integers*/
};

int allocs[2] = { 0, 0 };
int alloc_free = 0;
int alloc_free_cells = 0;
union store *allocp;	/*search ptr*/
union store *alloct;	/*arena top*/
union store *allocx;	/*for benefit of realloc*/

char *
malloc(nbytes)
unsigned nbytes;
{
	register union store *p, *q, *pstart;
	register nb;
	static temp;	/*coroutines assume no auto*/

	if(allocs[0]==0) {	/*first time*/
		allocs[0] = (int)setbusy(&allocs[1]);
		allocs[1] = (int)setbusy(&allocs[0]);
		alloct = (union store *)&allocs[1];
		allocp = (union store *)&allocs[0];
	}
	nb = ((nbytes+WORD+WORD-1)>>2) << 2;
	ASSERT(allocp>=(union store *)allocs && allocp<=alloct);
	ASSERT(allock());
	for(p=allocp; ; ) {
		if (nb > alloc_free - (alloc_free_cells << 4)) goto mapmore;
		pstart = clearbusy(p);
	    loop:	/* loop once around thru memory	*/
		if(!testbusy(p->ptr)) {
			while(!testbusy((q=p->ptr)->ptr)) {
				ASSERT(q>p&&q<alloct);
				p->ptr = q->ptr;
				alloc_free += WORD;
				alloc_free_cells--;
				if (q == pstart) {
				    q=p->ptr;
				    if ((INT)q >= (INT)p + nb)
					goto found;
				    goto mapmore;
				}
			}
			if ((INT)q >= (INT)p + nb)
			    goto found;
		}
		p = clearbusy(p->ptr);
		if (p != pstart) goto loop;

	    mapmore:
		/* now try mapping more pages	*/
		temp = (nb & 0xFFC000) + 0x4000;
	    try_again:
		q = (union store *)sbrk(temp);
		if((INT)q == -1) {
		    if (malloc_error_nomem) {
			malloc_error_nomem();
			temp = (nb & 0xFF00) + 0x100;
			goto try_again;
		    }
		    return(NULL);
		}
		ASSERT(q>alloct);
		alloct->ptr = q;
		if(q!=alloct+1)
		    alloct->ptr = setbusy(alloct->ptr);
		alloct = q->ptr = (union store *)((INT)q+temp-WORD);
		alloct->ptr = setbusy((union store *)allocs);
		alloc_free += temp - (WORD+WORD);
		alloc_free_cells += 2;
	}
found:
	allocp = (union store *)((INT)p + nb);
	ASSERT(allocp<=alloct);
	if(q>allocp) {
		allocx = allocp->ptr;
		allocp->ptr = p->ptr;
		alloc_free -= WORD;
	}
	else alloc_free_cells--;
	p->ptr = setbusy(allocp);
	alloc_free -= nb - WORD;
	return((char *)(p+1));
}

/*	freeing strategy tuned for LIFO allocation
*/
free(ap)
register char *ap;
{
	register union store *p = (union store *)ap;

	ASSERT(p>clearbusy(((union store *)allocs)[1].ptr)&&p<=alloct);
	ASSERT(allock());
	allocp = --p;
	ASSERT(testbusy(p->ptr));
	p->ptr = clearbusy(p->ptr);
	ASSERT(p->ptr > allocp && p->ptr <= alloct);
	alloc_free += (INT)p -> ptr - (INT)p - WORD;
	alloc_free_cells++;
}

/*	realloc(p, nbytes) reallocates a block obtained from malloc()
 *	and freed since last call of malloc()
 *	to have new size nbytes, and old content
 *	returns new location, or 0 on failure
*/

char *
realloc(p, nbytes)
register union store *p;
unsigned nbytes;
{
	register union store *q;
	register int *s, *t;
	register unsigned onw;
	register unsigned nw;

	if(testbusy(p[-1].ptr))
		free((char *)p);
	onw = ((INT)p[-1].ptr - (INT)p) >> 2;
	q = (union store *)malloc(nbytes);
	if(q==NULL || q==p)
		return((char *)q);
	s = (int *)p;
	t = (int *)q;
	nw = (nbytes+WORD-1)>>2;
	if(nw<onw)
		onw = nw;
	while(onw--!=0)
		*t++ = *s++;
	if(q<p && q+nw>=p)
	    ((union store *)((INT)q+((INT)q+(nw<<2)-(INT)p)))->ptr = allocx;
	return((char *)q);
}

#ifdef debug
allock()
{
#ifdef longdebug
	register union store *p;
	int x;
	x = 0;
	for(p= &((union store *)allocs)[0];
		clearbusy(p->ptr) > p; p=clearbusy(p->ptr)) {
		if(p==allocp)
			x++;
	}
	ASSERT(p==alloct);
	return((x==1)||(p==allocp));
#else
	return(1);
#endif
}
#endif
