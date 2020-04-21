/* routines for manipulating output queue in spool directory */

#include	"lp.h"

SCCSID("@(#)outputq.c	3.2")

static FILE *ro = NULL;
static FILE *wo = NULL;
static long ooffset = 0L;

static int olocked = FALSE; /* TRUE ==> outputq locked, FALSE ==> not locked */

/* endoent() -- clean up after last call to getoent() */

endoent()
{
	if(ro != NULL) {
		fclose(ro);
		ro = NULL;
	}
	if(wo != NULL) {
		fclose(wo);
		wo = NULL;
	}
	if(olocked) {
		unlock(OUTQLOCK);
		olocked = FALSE;
	}
}

/* getodest(o, dest) -- finds the next output queue entry for destination dest
			and returns status info in the supplied structure */

int
getodest(o, dest)
struct outq *o;
char *dest;
{
	int ret;

	setoent();
	while((ret = getoent(o)) != EOF && strcmp(o->o_dest, dest) != 0)
		;
	return(ret);
}

/* getoid(o, dest, seqno) -- finds the output queue entry for request id
			and returns status info in the supplied structure */

int
getoid(o, dest, seqno)
struct outq *o;
char *dest;
int seqno;
{
	setoent();
	while(getoent(o) != EOF)
		if(o->o_seqno == seqno && strcmp(o->o_dest, dest) == 0)
			return(0);
	return(EOF);
}

/* getoent(o) -- get next entry from output queue */

int
getoent(o)
struct outq *o;
{
	if(wo == NULL || ro == NULL || ! olocked)
		setoent();
	else
		ltouch(OUTQLOCK);
	o->o_flags |= O_DEL;
	while(o->o_flags & O_DEL) {	/* skip deleted records */
		ooffset = ftell(ro);
		if(fread((char *)o, sizeof(struct outq), 1, ro) != 1)
			return(EOF);
	}
	return(0);
}
/* setoent() -- initialize for subsequent calls to getoent() */

setoent()
{
	struct stat sbuf;

	if(! olocked) {
		if(trylock(OUTQLOCK, LOCKTIME, LOCKTRIES, LOCKSLEEP) == -1)
			fatal("can't lock output queue", 1);

		olocked = TRUE;
	}

	/*
		if OUTPUTQ doesn't exist, create it....
	*/

	if(stat(OUTPUTQ, &sbuf) == -1)
	{
		creat(OUTPUTQ, 0644);
	}

	if((wo == NULL && (wo = fopen(OUTPUTQ , "r+")) == NULL) ||
	   (ro == NULL && (ro = fopen(OUTPUTQ, "r")) == NULL))
		fatal("can't open output queue file", 1);
	chmod(OUTPUTQ, 0644);
	rewind(wo);
	rewind(ro);
	ooffset = ftell(ro);
}


/* putoent -- write output queue entry, overwriting the last record that
	      was returned by getoent() or getoid() */

putoent(o)
struct outq *o;
{
	fseek(wo, ooffset, 0);
	wrtoent(o, wo);
}

/* addoent -- write output queue entry to end of output queue file */

addoent(o)
struct outq *o;
{
	if(wo == NULL || ro == NULL || ! olocked)
		setoent();
	fseek(wo, 0l, 2);
	wrtoent(o, wo);
}


/* wrtoent -- write an output queue entry to the named stream */

wrtoent(o, stream)
struct outq *o;
FILE *stream;
{
	fwrite((char *)o, sizeof(struct outq), 1, stream);
}
