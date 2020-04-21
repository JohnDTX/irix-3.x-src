#include <stdio.h>
#include "globals.h"
#include "sym.h"

char *calloc();
/*
	alloc.c - allocation routines for the SGI mc68020 assembler.

	A number of types of heap area are used by as20.  These memory
	areas are gotten from a malloc() call.  No memory is ever returned.
	The following types of heap space are dynamically allocated:

		* symbol table space
		* statement structures
		* loc_s structures (stat,sym users)
		* binary data (for code, both text and data)
		* string table space (for symbol strings)
		* condensation information (a single call to allocate
		  a large byte array for condensation info).

	All types of heap space are allocated in segments.  
	When a segment runs out of room, a large chunk of memory is malloc'd
	and passed out a structure at a time (or string, or binary chunk).

*/


/**************************************************************************

	SYMBOL TABLE ALLOCATION ROUTINE

***************************************************************************/

/*  allocate SYMTAB_EXTENT symbol structures in a segment  */

#define SYMTAB_EXTENT 256
typedef struct symtab_s ssegment[SYMTAB_EXTENT];

/* The current segment is in cur_seg, and a pointer to the next available 
   symtab_s is in nextsym.  The number of the next symbol (in this segment)
   is kept in nextsymno.
*/
ssegment *cur_sseg = 0;
symtabptr nextsym = 0;
int nextsymno;

symtabptr 
allocate_symbol(istemp)
int istemp;
{
	register symtabptr s;
	if ((cur_sseg == (ssegment *)0) ||
	    (nextsymno == SYMTAB_EXTENT))
	{
		/* allocate a current segment */
		if ((cur_sseg = (ssegment *)calloc(sizeof(struct symtab_s),SYMTAB_EXTENT)) == 
		    (ssegment *)0)
			fatal((tokentype *)0,
			    "out of space for symbols");
		/* and set it up to be passed out */
		nextsym = &*cur_sseg[0];
		nextsymno = 0;
	}

	/* simply return the next symbol slot.  Zeroed by calloc */
	s = nextsym++;
	nextsymno++;
	if (istemp) 
	{
		s->syminfo.istemp=1;
	}
	return (s );
}

/************************************************************************

	BINARY DATA (code, data space) ALLOCATION ROUTINES 

*************************************************************************/

/*  BIN_EXTENT bytes of binary data space are allocated at a time.
    A pointer to the base of the current binary segment is kept in
    binseg, and to the next available location in binptr.
    A count of the number of bytes REMAINING in the current segment
    is kept in nbytesinseg.  If we need to allocate more bytes than
    remain in the current segment, we just throw away the rest of the
    segment and allocate a new one
*/

#define BIN_EXTENT 0x1000
typedef unsigned char *binseg;
static unsigned char *binptr;
static int nbytesinseg = (-1);
binseg cur_binseg = (binseg)0;

unsigned char *
allocate_binary(nbytes)
int nbytes;
{
	/*  allocate binary of nbytes.  Actual allocation of
	    binary is done in large chunks
	*/
	unsigned char *cptr;

	nbytes++;
	if ((nbytes) > BIN_EXTENT)
		fatal((tokentype *)0,
		    "cant alloc binary of %d bytes",nbytes);

	if (nbytes <= 1)
		fatal((tokentype *)0,
		    "attempt to alloc less than one byte of binary space");

	if (cur_binseg != (binseg)0)
		*binptr++ = 0;

	if ((nbytesinseg - (nbytes+1)) < 0)
	{
		/* alloc a new binary segment */
		if ((cur_binseg = (binseg)malloc(BIN_EXTENT)) == (binseg)0)
			fatal((tokentype *)0,
			    "out of space in alloc_binary");
		binptr = cur_binseg;
		nbytesinseg = BIN_EXTENT;
	}
	if (((int)binptr)&1) {binptr++; nbytesinseg--;}
	cptr = binptr;
	binptr += (nbytes - 1);
	nbytesinseg -= nbytes;
	return(cptr);
}

/****************************************************************************

	STRING TABLE SPACE ALLOCATION ROUTINES


****************************************************************************/

/*#define STR_EXTENT 0x1000 /* NOTE! This MUST match the declaration in code.c */
typedef char *strseg;
static char *strptr;
static int nbytesinstrseg = (-1);
int next_strsegno = 0;

struct strseglist 
{
	strseg seg;
	struct strseglist *next;
};
struct strseglist *cur_strseg = (struct strseglist *)0;
struct strseglist *strseglist_head = (struct strseglist *)0;

char *
allocate_strspace(nbytes)
int nbytes;
{
	/*
	    String space is the only type of heap space which must eventually be
	    rejoined into a logically contiguous space area, as it is directly 
	    written to the a.out file as the string table.   This is accom-
	    plished by allocating a small amount of extra memory at the
	    front of the segment for a small link structure (struct strseglist).
	    This is used to link together the string segments.  When the
	    symbols are to be generated, an array of the  base addresses
	    of each string segment is created so that string pointers can
	    be transformed into string table indices for the a.out file.  The
	    string table is then written by simply dumping out each string
	    segment in succession.  Each symbol has in it the segment number
	    its string was allocated from.
	*/
	char *cptr;

	nbytes++;
	if ((nbytes) > STR_EXTENT)
		fatal((tokentype *)0,
		    "cant alloc string table space of %d bytes",nbytes);

	if (nbytes <= 1)
		fatal((tokentype *)0,
		    "attempt to alloc less than one byte of string table space");

	if (cur_strseg != (struct strseglist *)0)
		*strptr++ = 0;

	if ((nbytesinstrseg - (nbytes+1)) < 0)
	{
		struct strseglist *next_strseg;

		/* alloc space for a string table segment and an instance of the
		   linking structure (strseglist).
		*/
		if ((next_strseg = 
		  (struct strseglist *)malloc(
			STR_EXTENT+sizeof(struct strseglist))) == 
				(struct strseglist *)0)
			fatal((tokentype *)0,
			    "out of space in alloc_strspace");

		/* set the beginning of the string segment to just
		   past the linking structure 
		*/
		strptr = (strseg)(next_strseg+1);

		/* link the string segments together */
		if (strseglist_head == (struct strseglist *)0)
			strseglist_head = next_strseg;
		else
			cur_strseg->next = next_strseg;

		/* keep track of the current string segment base */
		cur_strseg = next_strseg;

		/* mark as the last */
		cur_strseg->next = (struct strseglist *)0;
		/* and include a pointer to the string segment itself
		   (this could be calculated, but what the heck
		*/
		cur_strseg->seg = strptr;

		/* bump the number of segments */
		next_strsegno++;
		nbytesinstrseg = STR_EXTENT;
	}
	/* every symbol string starts on an even boundary */
	if (((int)strptr)&1) {strptr++; nbytesinstrseg--;}
	cptr = strptr;
	strptr += (nbytes - 1);
	nbytesinstrseg -= nbytes;
	return(cptr);
}


char **generate_strseg_array()
{
	/*  transform the strseglist to an array of bases for
	    the various string segments.  Return this dynamically
	    allocated array.  Also save the number of bytes used
	    in the last string segment.  (To know how many bytes
	    to write out to the a.out file for this last segment.)
	*/

	char **strseg_array = (char **)malloc((next_strsegno+1)*sizeof(char *));
	struct strseglist *cur_seg = strseglist_head;
	register char **curstrsegptr; 

	nbinlaststrseg = STR_EXTENT - nbytesinstrseg;
	if (strseg_array == (char **)0)
		fatal(0,"out of space in generate_strseg_array");

	curstrsegptr = strseg_array;

	while (cur_seg != (struct strseglist *)0)
	{
		*curstrsegptr++ = cur_seg->seg;
		cur_seg = cur_seg->next;
	}
	*curstrsegptr = (char *)0;
	return(strseg_array);
}

/*************************************************************************

	LOCATION INFO STRUCTURE ALLOCATION  ROUTINES

**************************************************************************/

#define LOC_EXTENT 256
typedef struct loc_s lsegment[LOC_EXTENT];

/* location info slots are allocated in segments.  The current segment
   is in cur_lseg, and a pointer to the next available loc_s is in nextlocs.
*/
lsegment *cur_lseg;
struct loc_s *nextlocs;
int nextlocsno;


struct loc_s  *
allocate_locs()
{
	/*  allocate a new location token.  A pointer to the token
	    is returned 
	*/

	register struct loc_s *a;
	if ((cur_lseg == (lsegment *)0) ||
	    (nextlocsno == LOC_EXTENT))
	{
		/* allocate a current segment */
		if ((cur_lseg = (lsegment *)calloc(sizeof(struct loc_s),LOC_EXTENT)) == 
		    (lsegment *)0)
			fatal((tokentype *)0,
			    "out of space for address tokens");
		/* and set it up to be passed out */
		nextlocs = &*cur_lseg[0];
		nextlocsno = 0;
	}

	/* simply return the next addr token slot.  Zeroed by calloc.*/
	a = nextlocs++;
	nextlocsno++;
	return (a );
}


/*************************************************************************

	STATEMENT STRUCTURE ALLOCATION ROUTINES

**************************************************************************/

#define STAT_EXTENT 256
typedef struct statement_s statsegment[STAT_EXTENT];

/* location info slots are allocated in segments.  The current segment
   is in cur_statseg, and a pointer to the next available statement_s is in nextstat.
*/
statsegment *cur_statseg;
struct statement_s *nextstat;
int nextstatno;


struct statement_s  *
allocate_statement()
{
	/*  allocate a new statement struct.  A pointer to the statement
	    is returned 
	*/

	register struct statement_s *a;
	if ((cur_statseg == (statsegment *)0) ||
	    (nextstatno == STAT_EXTENT))
	{
		/* allocate a current segment */
		if ((cur_statseg = (statsegment *)calloc(sizeof(struct statement_s),STAT_EXTENT)) == 
		    (statsegment *)0)
			fatal((tokentype *)0,
			    "out of space for address tokens");
		/* and set it up to be passed out */
		nextstat = &*cur_statseg[0];
		nextstatno = 0;
	}

	/* simply return the next addr token slot. Zeroed by calloc. */
	a = nextstat++;
	nextstatno++;
	return (a );
}

/***********************************************************************

	MISCELLANEOUS ALLOCATION ROUTINES 

************************************************************************/

allocate_condensation_info(nb)
unsigned long nb;
{
	/* allocate nb bytes of zeroed statement condensation info */

	if ( (condensation_info = (unsigned char *)calloc(nb,1)) == 
		(unsigned char *)0)
		fatal(0,"out of space for condensation info ");

}


/*  Temporary labels are assigned to any statement for which a displacement
    must be calculated, if that statement does not already have a label.  These
    labels cease to exist after the assembly is complete.  They are 
    distinguished from non-temporary labels by beginning with the (otherwise
    illegal) character '#'.

*/

static int next_tempno = 0;
static tokentype temptok ;

tokentype *allocate_templab()
{
	tokentype *tl = &temptok;
	int nb;
	sprintf(&temptok.u.cptr[2],"%x\0",next_tempno++);
	temptok.length = strlen(temptok.u.cptr);
	return(tl);
}

init_alloc() 
{
	temptok.tokennum = 0;
	temptok.line = 0;
	temptok.col = 0;
	temptok.u.cptr = "#TXXXXXXXXX";
	temptok.length = strlen(temptok.u.cptr);
}

static char *heapptr = 0;
static int nbinheap = 0;
#define HEAPPAGES 0x10
#define HEAPPGPOW2 12

malloc(nbytes)
int nbytes;
{
	/* our own malloc.  This is helpful, since we never return
	   the space we allocate.  It looks just like malloc.  We
	   also define the rest of the routines in the normal malloc
	   package, so that the modules never get pulled in tfrom the 
	   C library.

	   more space is just retrieved by sbrk'ing for HEAPPAGES * 0x1000
	   bytes.  
	*/

	int nb,nk;
	char * newend;
	if ((nbytes < 0) || (nbytes > 0x100000)) fatal(0,"cant allocate %d bytes",nbytes);
	/* always return an even pointer */
	if ((int)heapptr & 1) { heapptr++; nbinheap--;}
	/* if there isn't enough in the current data segment, 
	   sbrk for more
	*/
	if (nbytes > nbinheap)
	{
		/* allocate a new heap segment */
		/* if the request is for more than the usual maximum,
		   round up.
		*/
		nk = (nbytes +0xfff ) >> HEAPPGPOW2;
		if (nk < HEAPPAGES) nk = HEAPPAGES;
		nb = nk << HEAPPGPOW2;
		if ((newend = (char *)sbrk(nb)) == (char *)(-1))
			fatal(0,"out of memory");
		nbinheap += nb;
		if (heapptr == 0) 
		{	heapptr = newend;
#ifdef DEBUG
			fprintf(stderr,
			   "\ninitial break value: 0x%x\n",(int)heapptr);
#endif
		}

	}
	nbinheap -= nbytes;
	newend = heapptr;
	heapptr += nbytes;
	return((int)newend);
}

char *calloc(elemsz,nelem)
int elemsz,nelem;
{
	/* simply allocate elemsz*nelem bytes and zero-fill it */

	int nb=nelem * elemsz;
	char *retval;

	retval = (char *)malloc(nb);
	bzero(retval,nb);
	return(retval);
}

/* routines just for anyone else who wants to use them.  Free is
   a no-op and realloc would be disasterous if called 
*/
free()
{}

realloc()
{
	fatal(0,"realloc called");
}
