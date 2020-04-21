/*
 * fontram.c --- manage the font ram from the users space
 *			Peter Broadwell, Sept 27, 1984.
 *
 */
/*
     How to manage a limited resource between several users? First allocate
the most commonly used items statically and share them with everybody as in
(Font0,Cursor0,Pattern0).  Then have dynamic allocation of contiguous chunks 
of fontram for each user.  Next the user manage that hunk in whatever fashion.
When (if) the area fills up ask the kernel to make it bigger.  

    The fashion chosen is to have a small hash table for each type of object 
that ends up in fontram.  The hash table contains heads of a linked list.  
Each list element associates a user's index of an item with an offset into 
the area of contiguous storage.  (Offsets are used so the kernel can move 
a chunk when growing it or whatever.) Fontram storage is used up from and
freed onto a linked list of holes.

    Fonts have character description tables that need to be stored in addition
to the bitmaps in the fontram.  These and elements of the linked structures 
are malloced and freed as needed.  (Since the linestyle command gets used 
the same way it is in here too even though it doesn't end up in fontram.)

*/

#include "globals.h"
#include "immed.h"
#include "shmem.h"
#include "glerror.h"
#include "uc4.h"
#include "imattrib.h"
#include "grioctl.h"

static fontrec fontrec0 = {
    0,
    0,
    0,
    0,
    gl_defont_offsets,
    13,				/* max height */
    2,				/* max descender */
    10,	/*?*/			/* max width */
    MAXFONTNC			/* maxchars */
};

#define im_wssetup	register windowstate *WS = gl_wstatep

#define ALIGN_FOR_16	4
#define ALIGN_FOR_32	6
#define ALIGN_FOR_64	8

#define FONT_HASH_SIZE	128		/* should be power of two */
#define FONT_HASH(x)	(x & (FONT_HASH_SIZE - 1))

typedef struct genericrec {
	struct genericrec *next;
	long index;		/* fontrec index */
	short offset;		/* where thing is in fontram */
	short size;		/* how many words it takes up */
} genericrec;
	
typedef struct patternrec {
	struct patternrec *next;
	long index;		/* pattern index */
	short offset;		/* where thing is in fontram */
	short size;		/* how many words it takes up */
} patternrec;

typedef struct cursorrec {
	struct cursorrec *next;
	long index;		/* cursor index */
	short offset;		/* where thing is in fontram */
	short size;		/* how many words it takes up */
	char xoff, yoff;	/* hot spot inside cursor */
} cursorrec;

typedef struct fontramhole {
	struct fontramhole *next;
	short start;		/* begining of hole in fontram */
	short end;		/* (end of hole) + 1 */
} fontramhole;

typedef struct linestylerec {
	struct linestylerec *next;
	long index;		/* linestyle index */
	short offset;		/* where thing is in fontram */
	short size;		/* how many words it takes up */
	unsigned short bitpattern; /* bit pattern for linestyle */
} linestylerec;

static fontramhole *holetab;		/* start/end list (kept sorted) */
static long usedlength;

patternrec	*gl_patterntab[FONT_HASH_SIZE];
cursorrec	*gl_cursortab[FONT_HASH_SIZE];
fontrec		*gl_fonttab[FONT_HASH_SIZE];
linestylerec	*gl_lstab[FONT_HASH_SIZE];

/*
 * gl_fontreset --- reset all structures asociated with fontram management
 */
gl_fontreset()
{
    register genericrec **start;
    register genericrec *ptr;
    register genericrec *nxt;

#define FOREVERY(tb,tp)			\
	for(tp=(genericrec **)tb; tp<(genericrec **)&tb[FONT_HASH_SIZE]; tp++)

#define FREEEACH(tp,p,n)	{					\
				    for(p=(genericrec *)*tp; p; p=n) { 	\
					n=p->next;			\
					free(p);			\
				    }					\
				    *tp = 0;				\
				}

    FOREVERY(gl_patterntab,start) 
	FREEEACH(start,ptr,nxt);
    FOREVERY(gl_cursortab,start)
	FREEEACH(start,ptr,nxt);
    FOREVERY(gl_fonttab,start)
	FREEEACH(start,ptr,nxt);
    FOREVERY(gl_lstab,start)
	FREEEACH(start,ptr,nxt);
    FREEEACH(&holetab,ptr,nxt);
    if( (holetab = (fontramhole *)malloc(sizeof(fontramhole))) == 0 ) {
	gl_outmem("gl_fontreset");
	return;
    }
    holetab->next = 0;
    holetab->start = 0;
    holetab->end = FONTRAM_STARTSIZE;
}

fontrec *gl_findfont();

gl_defreset()
{
    short dummy, height, nc;
    im_setup;

    gl_getcharinfo(&dummy,&height,&dummy);	/* get kernel defont info */
    gl_getnumchars(&nc);			/* kernel's defont_nc value */
    fontrec0.maxheight = height;
    fontrec0.maxchars = nc;
    gl_fontreset();
    grioctl(GR_FONTMEM,FONTRAM_STARTSIZE);
    usedlength = WS->fontramlength;
    WS->curatrdata.currentfont = gl_findfont(0);
    im_outfontbase(WS->fontrambase);
    im_cleanup;
}

/*
 * gl_addto --- add a index/offset/size triplet to a table
 */
genericrec *gl_addto(table,index,offset,size,recsize)
    genericrec *table[FONT_HASH_SIZE];
    long index;
    short offset, size;
    short recsize;
{
    register genericrec *entry, **tabptr;

    if( (entry = (genericrec *)malloc(recsize)) == 0 ) {
	gl_outmem("gl_addto");
	return;
    }
    entry->index = index;
    entry->offset = offset;
    entry->size = size;
    tabptr = &(table[FONT_HASH(index)]);
    entry->next = *tabptr;
    *tabptr = entry;
    return entry;
}

/*
 * gl_find --- find a record in table with index == n
 *	Move found record to front of table so next search is faster.
 *	return pointer to record.
 */
genericrec *gl_find(index,table)
    register long index;
    genericrec *table[FONT_HASH_SIZE];
{
    register genericrec *tptr, *otptr, **stptr;

    stptr = &table[FONT_HASH(index)];
    for(otptr = tptr = *stptr; tptr != 0; otptr = tptr, tptr = tptr->next) {
	if(tptr->index == index) {
	    if(tptr != otptr) {/* if not there, move to front of list */
		otptr->next = tptr->next;
		tptr->next = *stptr;
		*stptr = tptr;
	    }
	    return(tptr);
	}
    }
    return((genericrec *)0);
}

void gl_takefrom(index,table)
    long index;
    genericrec *table[FONT_HASH_SIZE];
{
    register genericrec *tptr, **stptr;

    if(tptr = gl_find(index,table)) {
	stptr = &table[FONT_HASH(index)];
	*stptr = tptr->next;
	gl_holeinsert(tptr->offset,tptr->size);
	free(tptr);
    }
}

void
defpattern(n,sz,tx)
    long n, sz;
    unsigned short *tx;
{
    register short offset;
    register short masklength;
    /* space enough for the biggest pattern */ 
    unsigned short txbuf[PATTERN_64_SIZE];

    if(n == 0) {
	gl_ErrorHandler(ERR_CHANGEINDEX0, WARNING, "pattern");
	return;
    }
    gl_takefrom(n,gl_patterntab);
    switch(sz) {
	case PATTERN_16:
	    offset = gl_fontalloc(ALIGN_FOR_16,masklength=PATTERN_16_SIZE);
	    break;
	case PATTERN_32:
	    offset = gl_fontalloc(ALIGN_FOR_32,masklength=PATTERN_32_SIZE);
	    gl_arrange_32(tx,txbuf);
	    tx = txbuf;
	    break;
	case PATTERN_64:
	    offset = gl_fontalloc(ALIGN_FOR_64,masklength=PATTERN_64_SIZE);
	    gl_arrange_64(tx,txbuf);
	    tx = txbuf;
	    break;
	default:
	    {
		char badpattern[12];

		sprintf(badpattern,"0x%x",sz);
		gl_ErrorHandler(ERR_BADPATTERN, WARNING, badpattern);
		return;
	    }
	    
    }
    if(offset == -1) return;
    if(gl_addto(gl_patterntab,n,offset,masklength,sizeof(patternrec)))
	gl_loadmasks(offset,masklength,tx);
}

/*
 * gl_arrange_32 --- rearrange 32 by 32 bitmap for hardware needs.
 *		tx comes in assuming left->right, bottom->top
 *		ordering, goes out broken into PATTERN_16 size
 *		pieces going bottom->top, but left->right in
 *		each PATTERN_16 height row.
 */
gl_arrange_32(pre, post_tx)
    register short *pre, *post_tx;
{
    register short i, *post1, *post2;

    post1 = post_tx;
    post2 = post_tx + PATTERN_16_SIZE;
    for(i=0;i<PATTERN_16_SIZE;i++) {
	*post1++ = *pre++;
	*post2++ = *pre++;
    }
    post1 = post_tx + 2*PATTERN_16_SIZE;
    post2 = post_tx + 3*PATTERN_16_SIZE;
    for(i=0;i<PATTERN_16_SIZE;i++) {
	*post1++ = *pre++;
	*post2++ = *pre++;
    }
}

/*
 * gl_arrange_64 --- rearrange 64 by 64 bitmap for hardware needs.
 *		tx comes in assuming left->right, bottom->top
 *		ordering, goes out broken into four PATTERN_32
 *		size pieces going left->right, bottom->top, each
 *		arranged the same as arrange_32 leaves things.
 */
gl_arrange_64(pre, post_tx)
    register short *pre;
    short *post_tx;
{
    register short i, j, *post1, *post2, *post3, *post4;
    static short starts[] = {0,2,8,10};

    for(j=0;j<4;j++) {
	post1 = post_tx + starts[j] * PATTERN_16_SIZE;
	post2 = post1 + 1 * PATTERN_16_SIZE;
	post3 = post1 + 4 * PATTERN_16_SIZE;
	post4 = post1 + 5 * PATTERN_16_SIZE;
	for(i=0;i<(PATTERN_16_SIZE);i++) {
	    *post1++ = *pre++;
	    *post2++ = *pre++;
	    *post3++ = *pre++;
	    *post4++ = *pre++;
	}
    }
}

void
defcursor(n,curs)
    long n;
    Cursor curs;
{
    register short offset;
    register cursorrec *cur;

    if(n == 0) {
	gl_ErrorHandler(ERR_CHANGEINDEX0, WARNING, "cursor");
	return;
    }
    gl_takefrom(n,gl_cursortab);
    offset = gl_fontalloc(ALIGN_FOR_16,(sizeof(Cursor)/2));
    if(offset == -1) return;
    if(cur = (cursorrec *)gl_addto(gl_cursortab,n,offset,
				(sizeof(Cursor)/2),sizeof(cursorrec))) {
	gl_loadmasks(offset,sizeof(Cursor)/2,curs);
	cur->xoff = 0;
	cur->yoff = 0;
    }
}

void
curorigin(n,xoff,yoff)
    long n;
    short xoff,yoff;
{
    register cursorrec *cur;

    if(n == 0) {
	gl_ErrorHandler(ERR_CHANGEINDEX0, WARNING, "curorigin");
	return;
    }
    if( (cur = (cursorrec *)gl_find(n,gl_cursortab)) == 0 ) {
	gl_ErrorHandler(ERR_BADCURSOR, WARNING, "curorigin");
	return;
    }
    cur->xoff = xoff;
    cur->yoff = yoff;
}

void
defrasterfont(n,ht,nc,chars,nr,raster)
    long n,ht,nc,nr;
    Fontchar chars[];
    short raster[];
{
    register short offset;
    register Fontchar *charsp;
    register fontrec *fnt;
    static char nm[] = "rasterfont";

    if(n == 0) {
	gl_ErrorHandler(ERR_CHANGEINDEX0, WARNING, nm);
	return;
    }
    gl_freefont(n);
    if(nc == 0) {
	return;
    }
    offset = gl_fontalloc(ALIGN_FOR_16,nr);
    if(offset == -1) return;
    if(charsp = (Fontchar *)malloc(nc*sizeof(Fontchar))) {
	bcopy(chars,charsp,nc*sizeof(Fontchar));
    } else {
	gl_outmem(nm);
	return;
    }
    if(fnt = (fontrec *)gl_addto(gl_fonttab,n,offset,nr,sizeof(fontrec))){
	gl_loadmasks(offset,nr,raster);
	fnt->maxheight = ht;
	/* hacked to supply descender and width info 
		Wed Dec 11 10:50:03 PST 1985 -- hpm */
	fnt->maxdescender = gl_finddescender(charsp, nc);
	fnt->maxwidth = gl_findwidth(charsp, nc);
	fnt->chars = charsp;
	fnt->maxchars = nc;
    }
}

gl_finddescender(chars, nc)
Fontchar *chars;
long nc;
{
    int i,descender;

    descender = 0;
    for (i = 0; i < nc; i++) {
	if (descender > chars->yoff)
	    descender = chars->yoff;
	chars++;
    }
    return(-descender);
}


gl_findwidth(chars, nc)
Fontchar *chars;
long nc;
{
    short i,width;

    width = 0;
    for (i = 0; i < nc; i++) {
	if (width < chars->width )
	    width = chars->width;
	chars++;
    }
    return(width);
}

void
deflinestyle(n,ls)
    long n;
    Linestyle ls;
{
    register linestylerec *lsptr;

    if(n == 0) {
	gl_ErrorHandler(ERR_CHANGEINDEX0, WARNING, "linestyle");
	return;
    }
    gl_takefrom(n,gl_lstab);
    if( (lsptr=(linestylerec *)gl_addto(gl_lstab,n,0,0,sizeof(linestylerec))) != 0)
	lsptr->bitpattern = ls;
}

/*
 * gl_fontalloc --- find wcount worth of fontram words alligned
 *	on boundary size pieces.
 *	Returns offset in words where hole is.
 *	Returns -1 on error.
 */
gl_fontalloc(boundary,wcount)
    int boundary;
    int wcount;
{
    im_wssetup;
    register short offset;

#define CHUNKS(x)	((x+255) & ~255)	/* round up to 256 boundary */

    if((offset = gl_fontsearch(boundary,wcount)) < 0) {
	if(grioctl(GR_FONTMEM, usedlength + CHUNKS(wcount))) {
	    gl_ErrorHandler(ERR_NOFONTRAM, WARNING, "fontalloc(internal)");
	    return(-1);
	}
	/* grioctl will have made more room by now */
	usedlength = WS->fontramlength;
	offset = gl_fontsearch(boundary,wcount);
    }
    return(offset);
}

/*
 * gl_fontsearch --- look through list for good hole and use it.
 *	Also coalese holes.
 *	Return offset of hole, -1 on error.
 */
gl_fontsearch(boundary,wcount)
    int boundary;
    int wcount;
{
    register fontramhole *hptr, *ohptr;
    register short goodhole;

    gl_holecoalese();
    for(ohptr = hptr = holetab; hptr != 0; ohptr = hptr, hptr = hptr->next) {
	if(((goodhole = aligned(boundary,hptr->start))+wcount) <= hptr->end) {
	    /* take this hole out */
	    if(hptr->start != goodhole)	
		gl_addhole(ohptr,hptr->start,goodhole);
	    hptr->start = goodhole + wcount;
	    return(goodhole);
	}
    }
    return(-1);
}

/*
 * gl_addhole --- stuff a new hole onto a given list
 */
gl_addhole(list,start,end)
    register fontramhole *list;
    register long start;
    short end;
{
    register fontramhole *hptr;

    if( (hptr = (fontramhole *)malloc(sizeof(fontramhole))) == 0 ) {
	gl_outmem("gl_addhole");
	return;
    }
    hptr->start = start;
    hptr->end = end;
    if(list == holetab) {	/* list is at the head */
	holetab = hptr;
	hptr->next = list;
    }
    else {
	hptr->next = list->next;
	list->next = hptr;
    }
}

/*
 * gl_holeinsert --- stuff offset into a tab so as to keep the
 *		list the sorted (by offset).
 */
gl_holeinsert(offset,size)
    register long offset;
    short size;
{
    register fontramhole *hptr, *ohptr;

    if(size == 0)
	return;
    for(ohptr = hptr = holetab; hptr != 0; ohptr = hptr, hptr = hptr->next) {
	if(hptr->start > offset)
	    break;
    }
    gl_addhole(ohptr,offset,offset+size);
}

/*
 * gl_holecoalese --- coalese all holes together
 *	Also make sure that last hole extends to end of allocated fontram.
 */
gl_holecoalese()
{
    register fontramhole *hptr, *ohptr;

    for(ohptr = hptr = holetab; hptr != 0; ohptr = hptr, hptr = hptr->next) {
	if(ohptr->end == hptr->start) {
	    if(ohptr != hptr) {		/* not head of list */
		ohptr->end = hptr->end;
		ohptr->next = hptr->next;
		free(hptr);
		hptr = ohptr;		/* need this [GMT]	*/
	    }
	}
	if(hptr->start == hptr->end) {	/* null entry */
	    if(ohptr != hptr && hptr->next) {
			/* not head (or tail, peter 9/30/85) of list */
		ohptr->next = hptr->next;
		free(hptr);
		hptr = ohptr;		/* need this [GMT]	*/
	    }
	}
    }
    if(ohptr->end < gl_shmemptr->ws.fontramlength) {
	/* FIXME says peter, 5/17/85. need to be smarter about end holes */
	ohptr->end = gl_shmemptr->ws.fontramlength;
	/* gl_ErrorHandler(ERR_FONTHOLES, WARNING, "gl_holecoalese(internal)");
	exit(1);	/* FIXME */
    }
    if(ohptr->end >= gl_shmemptr->ws.fontramlength) {
	if((usedlength = CHUNKS(ohptr->start)) < FONTRAM_STARTSIZE) {
	    usedlength = FONTRAM_STARTSIZE;
	}
    }
}

/*
 * aligned --- returns the next larger boundary aligned number
 *	ONLY works with boundries that are powers of 2
 *	boundary is number of bits to knock off
 *
 *	Thanks to Greg Boyd for talking me through this, peter.
 */
static
aligned(boundary,offset)
    int boundary;
    int offset;
{
    register int border = ((1<<boundary)-1);

    return((offset + border) & ~border);
}

/*
 * gl_freefont --- release fontram memory and 68K table associated
 *		with font.
 */
gl_freefont(n)
    long n;
{
    register fontrec *fnt;

    fnt = (fontrec *)gl_find(n,gl_fonttab);
    if (fnt) free(fnt->chars);
    gl_takefrom(n,gl_fonttab);
}

gl_loadmasks(ramaddr, nw, words)
    register short ramaddr;
    register short nw;
    register short *words;
{
    register short amount;
    im_setup;

    if(ramaddr == -1) return;	/* thanks hpm, peter 12/12/85 */
/* printf("gl_loadmasks(addr = %d(0x%x), nw = %d, words = 0x%x)\n",
		 ramaddr,ramaddr,nw,words); /**/
    im_outfontbase(WS->fontrambase);
    while (nw) {
	amount = nw > 120 ? 120 : nw;
	im_passthru(amount + 2);
	im_outshort(FBCloadmasks);
	im_outshort(ramaddr);
	ramaddr += amount;		/* bump up ram address	*/
	nw -= amount;			/* dec word counter	*/
	while (--amount != -1) {	/* send the words	*/
	    im_outshort(*words++);
	}	
	amount = amount;		/* persuade into dbra	*/
    }	
    im_freepipe;
    im_cleanup;
}

/*
 The following routines return the relative address in fontram of the
 desired object. Linestyle is an odd ball ...
 Assumes fontram is initialized by the kernel to contain:
    pattern0	at location 0
    cursor0	at location 0 + sizeof(Pattern16)/2
    font0	at location 0 + sizeof(Pattern16)/2 + sizeof(Cursor)/2
 fontrambase is in shared memeory and is an absolute address where
 this users chunk is located, so to get to cursor0 the relative address
 from a users point of view is minus fontrambase etc.
 */

unsigned short gl_findpattern(n,cfr)
    long n;
    register long *cfr;
{
    im_wssetup;
    register patternrec * ptptr;

    *cfr &= ~(UC_PATTERN32 | UC_PATTERN64);
    if ( (ptptr = (patternrec *)gl_find(n,gl_patterntab)) == 0 ) {
	gl_wstatep->curatrdata.mytexture = 0;
	return(-WS->fontrambase);
    }
    switch(ptptr->size) {
	case PATTERN_16_SIZE :
	    break;	    /* both cleared is 16x16 */
	case PATTERN_32_SIZE :
	    *cfr |= UC_PATTERN32;
	    break;
	case PATTERN_64_SIZE :
	    *cfr |= (UC_PATTERN32 | UC_PATTERN64);
	    break;
	default:
	    gl_ErrorHandler(ERR_BADPATTERN,WARNING,"bad pattern");
    }
    return(ptptr->offset);
}

gl_findcursor(n,xoff,yoff)
    long n;
    short *xoff, *yoff;
{
    im_wssetup;
    register cursorrec * cptr;

    if ( (cptr = (cursorrec *)gl_find(n,gl_cursortab)) == 0 ) {
	*xoff = 0;
	*yoff = 15;
	return(-WS->fontrambase + sizeof(Pattern16)/2);
    }
    *xoff = cptr->xoff;
    *yoff = cptr->yoff;
    return(cptr->offset);
}

fontrec *gl_findfont(n)
    long n;
{
    im_wssetup;
    fontrec *fptr;

    if ( (fptr=(fontrec *)gl_find(n,gl_fonttab)) == 0 ) {
	WS->fontbase = 0;
	return(&fontrec0);
    } else {
	WS->fontbase = WS->fontrambase+fptr->offset;
	return(fptr);
    }
}

unsigned short gl_findlinestyle(n)
    long n;
{
    register linestylerec *lsptr;

    if ( (lsptr=(linestylerec *)gl_find(n,gl_lstab) ) == 0) {
	gl_wstatep->curatrdata.mylstyle = 0;
	return(0xffff);
    }
    return(lsptr->bitpattern);
}

long getfont()
{
    return(gl_wstatep->curatrdata.currentfont->index);
}


long getheight()
{
    return(gl_wstatep->curatrdata.currentfont->maxheight);
}

long getdescender()
{
    return(gl_wstatep->curatrdata.currentfont->maxdescender);
}

long getwidth()
{
    return(gl_wstatep->curatrdata.currentfont->maxwidth);
}
