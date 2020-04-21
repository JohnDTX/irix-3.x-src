/* routines for manipulating printer status file in spool directory */

#include	"lp.h"

SCCSID("@(#)pstatus.c	3.1")

static FILE *rp = NULL;
static FILE *wp = NULL;
static long poffset = 0L;

static int plocked = FALSE; /* TRUE ==> pstatus locked, FALSE ==> not locked */

/* endpent() -- clean up after last call to getpent() */

endpent()
{
	if(rp != NULL) {
		fclose(rp);
		rp = NULL;
	}
	if(wp != NULL) {
		fclose(wp);
		wp = NULL;
	}
	if(plocked) {
		unlock(PSTATLOCK);
		plocked = FALSE;
	}
}

/* getpdest(p, dest) -- finds the printer status entry for destination dest
			and returns status info in the supplied structure */

int
getpdest(p, dest)
struct pstat *p;
char *dest;
{
	int ret;

	setpent();
	while((ret = getpent(p)) != EOF && strcmp(p->p_dest, dest) != 0)
		;
	return(ret);
}

/* getpent(p) -- get next entry from printer status file */

int
getpent(p)
struct pstat *p;
{
	if(wp == NULL || rp == NULL || ! plocked)
		setpent();
	else
		ltouch(PSTATLOCK);
	poffset = ftell(rp);
	return(fread((char *)p, sizeof(struct pstat), 1, rp) != 1 ? EOF : 0);
}
/* setpent() -- initialize for subsequent calls to getpent() */

setpent()
{
	if(! plocked) {
		if(trylock(PSTATLOCK, LOCKTIME, LOCKTRIES, LOCKSLEEP) == -1)
			fatal("can't lock printer status", 1);

		plocked = TRUE;
	}

	if((wp == NULL && (wp = fopen(PSTATUS , "r+")) == NULL) ||
	   (rp == NULL && (rp = fopen(PSTATUS, "r")) == NULL))
		fatal("can't open printer status file", 1);
	chmod(PSTATUS, 0644);
	rewind(wp);
	rewind(rp);
	poffset = ftell(rp);
}


/* putpent -- write pstatus entry, overwriting the last record that
	      was returned by getpent() or getpdest() */

putpent(p)
struct pstat *p;
{
	fseek(wp, poffset, 0);
	wrtpent(p, wp);
}

/* addpent -- write pstatus entry to end of pstatus file */

addpent(p)
struct pstat *p;
{
	if(wp == NULL || rp == NULL || ! plocked)
		setpent();
	fseek(wp, 0L, 2);
	wrtpent(p, wp);
}


/* wrtpent -- write an entry in the pstatus file */

wrtpent(p, stream)
struct pstat *p;
FILE *stream;
{
	fwrite((char *)p, sizeof(struct pstat), 1, stream);
}
