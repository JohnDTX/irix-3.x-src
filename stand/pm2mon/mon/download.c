# include "a.out.h"
# include "Qglobals.h"
# include "IrisConf.h"
# include "irisboot.h"
# include "common.h"
# include "pmIImacros.h"


# define DEFLOADPT	0x1000
# define DEFSTACK	0x1000

# define PACKETSPERDOT	10



# define DEBUG do_debug
# include "dprintf.h"


struct
{
    char *ptr;
    long cnt;
    char *base;
    long maxsize;

    int (*read)(),(*open)(),(*close)();
} B;

int DLpackets;
int DLtotal;





/*
 * callboot() --
 * entry from monitor boot routine.
 */
callboot(pref,ext,file)
    char *pref,*ext,*file;
{
    register int oldpri;

    printf("Boot: %s.%s:%s\n",pref,ext,file);

    oldpri = spl5();
    SET_USER_PROGRAM;

    loadgo(pref,ext,file);

    CLEAR_USER_PROGRAM;
    splx(oldpri);
}

int
loadgo(pref,ext,file)
    char *pref,*ext,*file;
{
    long initstack;		/* formerly this must stay here! */
    long entrypoint;		/* formerly this too! */
    long placehold;

    register int status;

    if( opendownload(pref,ext,file) < 0 )
	return -1;
    printf("OK");
    
    status = download(&entrypoint,&initstack);

    closedownload();

    if( status < 0 )
	return -1;

    if( VERBOSE(switches) )
    printf("Received %d/%d",DLtotal,DLpackets);
    printf(":\n");
    if( VERBOSE(switches) )
    printf("Stack at 0x%x; Starting at 0x%x\n\n"
	    ,initstack,entrypoint);

# ifdef DEBUG
    if(do_debug&0x80) monitor(0);
# endif DEBUG

    /*disable;*/
    spl7();

    ResetExcelan();

    ResetDuarts();

    if( ISMICROSW )
    {
	TermComm();
	ScreenComm();
    }

    /*
     * set up initial stack pointer (no args)
     * and call, hopefully never to return.
     */
    LaunchStack(entrypoint,initstack,0);

    printf("? Image bombed (CROAK)!\n");
    return status;
}



/*
 * Initdownload() --
 * allocate memory for download buffer.  called by startup.
 */
int
Initdownload()
{
    extern char *gmalloc();

    if( (B.base = gmalloc(MAXBSIZE)) == 0 )
	return FALSE;

    B.maxsize = MAXBSIZE;
    return TRUE;
}

/*
 * opendownload() --
 * set up to read the bootfile.
 */
int
opendownload(pref,ext,file)
    char *pref,*ext,*file;
{
    if( !Initdownload() )
	return -1;

    if( bootlookup(pref,&B.open,&B.close,&B.read) < 0 )
    {
	printf("\"%s\" is not a legal boot device\n",pref);
	return -1;
    }

    B.ptr = 0; B.cnt = 0;
    DLpackets = 0;
    DLtotal = 0;

    if( (*B.open)(ext,file) < 0 )
    {
	/* (*B.close)(); */
	return -1;
    }

    return 0;
}

/*
 * closedownload() --
 * clean up after reading the bootfile.
 */
closedownload()
{
	(*B.close)();
}

/*
 * readdownload() --
 * read a (possibly large) amount from the bootfile via a driver
 * read routine.  the routine is told how many bytes are desired
 * and where they should go.  it sends back the actual count and
 * where it put them.  if possible, read directly to the target
 * area.  otherwise use the buffer provided by Initdownload().
 * never ask for less than MAXBSIZE, buffering if necessary.
 */
int
readdownload(target,nbytes)
    register char *target;
    register int nbytes;
{
    register int xcnt;

    while( nbytes > 0 )
    {
	if( B.cnt == 0 )
	{
	    /* reassuring noise */
	    if( DLpackets++ % PACKETSPERDOT == 0 )
		printf(".");

	    /*
	     * always ask for at least one block (maximum size).
	     * if close to the end, read to a temp area and
	     * copy it to the true target.
	     */
	    B.ptr = B.base; B.cnt = B.maxsize;
	    if( nbytes >= B.maxsize )
	    {
		B.ptr = target; B.cnt = nbytes;
	    }
/*dprintf(("[%x@%x]",B.cnt,B.ptr));*/
	    B.cnt = (*B.read)(&B.ptr,B.cnt);
/*dprintf(("<%x@%x>",B.cnt,B.ptr));*/
	    if( B.cnt < 0 )
	    {
		B.cnt = 0;
		return -1;
	    }
	}

	xcnt = B.cnt<nbytes ? B.cnt : nbytes;

	if( B.ptr != target )
	{
/*dprintf(("copy%d;%x->%x",xcnt,B.ptr,target));*/
	    bcopy(B.ptr,target,xcnt);
	}
	B.ptr += xcnt; B.cnt -= xcnt;	

	DLtotal += xcnt;
	target += xcnt; nbytes -= xcnt;
    }

    return 0;
}

/*
 * download() --
 * load a boot file (after being set up to read from it).
 */
int
download(_entry,_stackp)
    long (*_entry),(*_stackp);
{
    union
    {
	struct ibhdr ibhead;
	struct exec exec;
	char c[1];
    }   x;
    struct exec exec;
    register int bad,done;
    register long entry,loadptr;

    *_stackp = -1;
    entry = -1;

    bad = done = 0;

    while( !(bad || done) )
    {
	if( readdownload(x.c,sizeof x) < 0 )
	{
	    printf("? Error reading irisboot header\n");
	    bad++;
	    break;
        }

	switch( x.ibhead.fmagic )
	{
	case IBLAST:
	    done++;
	    break;

	case FMAGIC:
	    exec = x.exec;
	    entry = exec.a_entry;

	    /*
	     * kluge for backward compatibility:
	     * "vkernel" style FMAGIC files load from DEFLOADPT
	     * expect bss cleared, and stack at DEFSTACK.
	     * "unix" style FMAGIC files load from the entry
	     * point, expect bss not cleared, and stack at some
	     * "safe place."
	     */
	    *_stackp = DEFSTACK;
	    loadptr = DEFLOADPT;
	    if( entry == 0x400 || entry == 0x1000 || entry == 0x2000 )
	    {
		*_stackp = (long)&exec;
		loadptr = entry;
		exec.a_bss = 0;
	    }

	    /*
	     * kluge for future expansion:
	     * if the header's relocation word is non-0, use
	     * it as the entry point and the entry word as
	     * the load point.
	     */
	    if( exec.a_drsize != 0 )
	    {
		loadptr = entry;
		entry = exec.a_drsize;
	    }
	    exec.a_syms = 0;

	    bad = load_fmagic(loadptr
		    ,exec.a_text,exec.a_data,exec.a_bss
		    ,exec.a_syms) < 0;
	    done++;
	    break;

	case IBMAGIC:
	    if( readdownload((char *)&exec,sizeof(struct exec)) < 0 )
	    {
		printf("? Error reading b.out header\n");
		bad++;
		break;
	    }

	    if( exec.a_magic != FMAGIC )
	    {
		printf("? Bad magic number 0x%x in b.out header\n"
			,exec.a_magic);
		bad++;
		break;
	    }

	    if( *_stackp == -1 )
		*_stackp = x.ibhead.initstack;
	    if( entry == -1 )
		entry = exec.a_entry;
	    if( x.ibhead.doloadheader )
		*(struct exec *)x.ibhead.headerloc = exec;
	    if( !x.ibhead.doloadsymbols )
		exec.a_syms = 0;
	    loadptr = x.ibhead.loadloc;

	    bad = load_fmagic(loadptr
		    ,exec.a_text,exec.a_data,exec.a_bss
		    ,exec.a_syms) < 0;
	    break;

	default:
	    printf("? Bad magic number 0x%x in irisboot header\n"
		    ,x.ibhead.fmagic);
	    bad++;
	    break;
        }
    }

    *_entry = entry;
    return -bad;
}

int
load_fmagic(ptr,textlen,datalen,bsslen,symlen)
    register char *ptr;
    long textlen,datalen,bsslen,symlen;
{
if( VERBOSE(switches) )
printf("\nLoading 0x%x+0x%x+0x%x at 0x%x\n"
,textlen,datalen,bsslen,ptr);
    datalen += textlen;
    if( readdownload(ptr,datalen)  < 0 )
    {
	printf("? Error reading text+data\n");
	return -1;
    }
    ptr += datalen;

    bzero(ptr,bsslen);
    ptr += bsslen;

    if( symlen != 0 ) /* (sigh) GB this wont work with 4.2 symbols */
	if( readdownload(ptr,symlen) < 0 )
	{
	    printf("? Error reading program symbols\n");
	    return -1;
	}
    ptr += symlen;

    return 0;
}
