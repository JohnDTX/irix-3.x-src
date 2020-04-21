/*
 * $Source: /d2/3.7/src/stand/lib/dev/RCS/cpio.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:14:21 $
 */

#include "stand.h"
#include "cpiohdr.h"
#include "dprintf.h"

#define TBSIZE	512	/* XXX somewhere else should depend on controller */
#define TBMASK	(TBSIZE-1)
#define MAXREAD	0x40000	/* 256K	*/
#define toeven(x)	( ((x)+1)&(~0x01) )

/* a structure to buffer cpio data and make life pleasant */
struct cpiobuf {
	char	*c_ptr;		/* pointer to data in buffer */
	int	c_cnt;		/* count of data left	*/
	char	*c_base;	/* base for io		*/
	int	c_bufsize;	/* maximum amount in buffer	*/
	off_t	c_off;		/* offset into cpio archive member */
	off_t	c_len;		/* length of archive member being read */
	char	c_found;
} ci;

struct cpiohdr hdr;		/* XXX malloc	*/


/*
 * open the tape and position to correct spot
 */
cpioopen(io,path,lsflag)
struct iob *io;
char *path;
int lsflag;
{
	register struct inode *ip;
	register struct cpiobuf *cip;
	char *mbmalloc();


	/* initialize the cpio buffer structure	*/
	cip = &ci;
	cip->c_off = 0;
	cip->c_len = 0;
	cip->c_bufsize = TBSIZE;
	cip->c_found = 0;
	cip->c_cnt = 0;
	if ( cip->c_base == 0 ) {
		if ( (cip->c_base = mbmalloc(cip->c_bufsize)) == 0 ) {
			io->i_error = ENOMEM;
			return(-1);
		}
	}
	cip->c_ptr = cip->c_base;

	/* now scan for the file	*/
	if ( lsflag || (path != NULL) ) {
		if ( cpioscan(io,path,lsflag) < 0 ) {
			return(-1);
		}
	}

	return(0);
}

cpiolist(fd,path)
int fd;
char *path;
{
	register struct iob *io;

	io = &iobuf[fd - 3];

	cpioopen(io,path,1);
}

cpio_rw(io,flag)
struct iob *io;
int flag;
{
	register struct cpiobuf *cip;

	cip = &ci;

	if ( cip->c_off >= cip->c_len )
		return(0);

	if ( io->i_count + cip->c_off > cip->c_len )
		io->i_count = cip->c_len - cip->c_off;

	switch ( flag ) {

	case READ:
		return(cpio_read(io));

	default:
		printf("action not supported\n");
		io->i_error = EPERM;
		return(-1);
	}
}

/*
 * Scan the cpio image for the headers. Try and find the named file
 */
cpioscan(io,path,lsflag)
register struct iob *io;
char *path;
int lsflag;
{
	register struct cpiohdr *h;
	register struct cpiobuf *cip;
	register int pathlen;
	int size;

	cip = &ci;
	h = &hdr;

	if ( path == NULL )
		pathlen = 0;
	else
		pathlen = strlen(path);
	dprintf(("cpio scan: path -%s- len %d, %s\n",path==NULL ? "NULL":path,pathlen,lsflag?"for ls":"to read"));
	while ( 1 ) {
		/* read the header */
		io->i_base = (char *)h;
		io->i_count = sizeof(struct cpiohdr) - sizeof(h->cpioname);
		if ( cpio_read(io) < 0 )
			return(-1);

		size = h->cpiofilesize[0]<<16 | h->cpiofilesize[1];
		if ( h->cpiomagic != CPIOMAGIC )
			return(-1);
		/* read in name */
		io->i_base = h->cpioname;
		io->i_count = toeven(h->cpionamesize);
		if ( cpio_read(io) < 0 )
			return(-1);
		dprintf(("name %s\n",h->cpioname));

		/* are we at end? */
		if ( (size == 0) && (strcmp(h->cpioname,"TRAILER!!!") == 0) )
			break;

		/* is this the header we want? */
		if ( lsflag ) {
			if ( (pathlen == 0) ||
				 (strncmp(h->cpioname,path,pathlen) == 0) )
				printf("%s\n",h->cpioname);
		} else {	/* must be full match */
			if ( (strcmp(h->cpioname,path) == 0) ) {
				cip->c_len = size;
				cip->c_off = 0;
				cip->c_found = 1;
				break;
			}
		}
		/*
		 * The name did not match so skip to next header.
		 * just advance tape to correct spot
		 */
		
		io->i_base = (char *)-1;
		io->i_count = toeven(size);
		if ( cpio_read(io) < 0 )
			return(-1);
	}

	if ( !lsflag && !cip->c_found ) {
		io->i_error = ENOENT;
		return(-1);
	} else {
		return(0);
	}
}

/* a base pointer of -1 is a flag to not copy the data to a buffer (ie a seek)*/
cpio_read(io)
register struct iob *io;
{
	register char *tp;
	register struct cpiobuf *cip;
	register int len;
	register int cnt;		/* temp count	*/
	int tot;

	cip = &ci;
	tp = io->i_base;
	tot = len = io->i_count;

	while ( len > 0 ) {
		/* see if anything in buffer */
		if ( cip->c_cnt > 0 ) {
			cnt = min(len,cip->c_cnt);
			dprintf(("buf read cnt %d, buf 0x%x\n",cnt,tp));
			if ( (int)tp != -1 ) {		/* if -1 is a seek */
				bcopy(cip->c_ptr,tp,cnt);
				tp += cnt;
			}
			len -= cnt;
			cip->c_ptr += cnt;
			cip->c_cnt -= cnt;
			if ( cip->c_len > 0 )
				cip->c_off += cnt;
			continue;			/* go around while */
		}

		/* read in multiples of blocks	*/
		if ( len >= TBSIZE ) {
			cnt = len & ~TBMASK;
			cnt = min(cnt,MAXREAD);
			if ( (int)tp == -1) /* seek - use low core as scratch*/
				io->i_base = 0;
			else
				io->i_base = tp;
			dprintf(("read cnt %d, buf 0x%x\n",cnt,tp));
			io->i_count = cnt;
			if ( (cnt = rdwr(io)) < 0 ) {
				return(-1);
			}
			if ( cip->c_len > 0 )
				cip->c_off += cnt;
			len -= cnt;
			if ( (int)tp != -1 )
				tp += cnt;
		} else { 				/* fill the buffer */
			io->i_base = cip->c_base;
			io->i_count = cip->c_bufsize;
			dprintf(("buffer fill cnt %d, buf 0x%x\n",
				io->i_count,io->i_base));
			if ( (cnt = rdwr(io)) < 0 ) {
				return(-1);
			}
			cip->c_cnt = cnt;
			cip->c_ptr = cip->c_base;
		}
	}
	
	dprintf(("cpio_read: returns %d\n", tot));
	return(tot);
}
