/* routines for manipulating qstatus file in spool directory */

#include	"lp.h"

SCCSID("@(#)qstatus.c	3.1")

static FILE *rq = NULL;
static FILE *wq = NULL;
static long qoffset = 0L;

static int qlocked = FALSE; /* TRUE ==> qstatus locked, FALSE ==> not locked */

/* endqent() -- clean up after last call to getqent() */

endqent()
{
	if(rq != NULL) {
		fclose(rq);
		rq = NULL;
	}
	if(wq != NULL) {
		fclose(wq);
		wq = NULL;
	}
	if(qlocked) {
		unlock(QSTATLOCK);
		qlocked = FALSE;
	}
}

/* getqdest(q, dest) -- finds the acceptance status entry for destination dest
			and returns status info in the supplied structure */

int
getqdest(q, dest)
struct qstat *q;
char *dest;
{
	int ret;

	setqent();
	while((ret = getqent(q)) != EOF && strcmp(q->q_dest, dest) != 0)
		;
	return(ret);
}

/* getqent(q) -- get next entry from acceptance status file */

int
getqent(q)
struct qstat *q;
{
	if(wq == NULL || rq == NULL || ! qlocked)
		setqent();
	else
		ltouch(QSTATLOCK);
	qoffset = ftell(rq);
	return(fread((char *)q, sizeof(struct qstat), 1, rq) != 1 ? EOF : 0);
}
/* setqent() -- initialize for subsequent calls to getqent() */

setqent()
{
	if(! qlocked) {
		if(trylock(QSTATLOCK, LOCKTIME, LOCKTRIES, LOCKSLEEP) == -1)
			fatal("can't lock acceptance status", 1);

		qlocked = TRUE;
	}

	if((wq == NULL && (wq = fopen(QSTATUS , "r+")) == NULL) ||
	   (rq == NULL && (rq = fopen(QSTATUS, "r")) == NULL))
		fatal("can't open acceptance status file", 1);
	chmod(QSTATUS, 0644);
	rewind(wq);
	rewind(rq);
	qoffset = ftell(rq);
}


/* putqent -- write qstatus entry, overwriting the last record that
	      was returned by getqent() or getqdest() */

putqent(q)
struct qstat *q;
{
	fseek(wq, qoffset, 0);
	wrtqent(q, wq);
}

/* addqent -- write qstatus entry to end of qstatus file */

addqent(q)
struct qstat *q;
{
	if(wq == NULL || rq == NULL || ! qlocked)
		setqent();
	fseek(wq, 0L, 2);
	wrtqent(q, wq);
}


/* wrtqent -- write an entry in the qstatus file */

wrtqent(q, stream)
struct qstat *q;
FILE *stream;
{
	fwrite((char *)q, sizeof(struct qstat), 1, stream);
}
