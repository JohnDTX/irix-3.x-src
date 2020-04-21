#include "vchk.h"

#define	REMID	1	/* id record */
#define REMFILE	2	/* file name */
#define REMINFO	3	/* remote print */
#define REMXMIT	4	/* transmit file */
#define REMERR	5	/* error */
#define REMMODE	6	/* file mode */
#define REMDATA	7	/* transmitted data */

#define NETSIZ	4096

struct rembuf {
	char		r_type;
	unsigned char	r_lowct;
	unsigned char	r_highct;
	char		r_data[NETSIZ+2];
} remrbuf, remxbuf;

extern char *sys_id;
extern int rem;

getnet(fn)
char *fn;
{
	register struct rembuf *rp;
	register fp, i, mode, rv;

	rp = &remrbuf;
	if ((mode = creat(fn, 0600)) < 0) {
		X(("can't create %s\n", fn));
		return(1);
	}
	close(mode);
	if ((fp = open(fn, 2)) == NULL) {
		X(("can't reopen %s\n", fn));
		return(1);
	}
	rv = 0;
	remputstr(REMID, sys_id);	/* send id string */
	remputstr(REMFILE, fn);		/* send file name */
	remputfn(REMXMIT);		/* send xmit command */
	for (;;) {
		if (getremote(rp)) {
			X(("network receive failure\n"));
			close(fp);
			return(1);
		}
		switch(rp->r_type) {
		case REMERR:
			X(("%s\n", rp->r_data));
			close(fp);
			return(1);
		case REMINFO:
			X(("reminfo: %s\n", rp->r_data));
			break;
		case REMMODE:
			mode = rp->r_data[0] | (rp->r_data[1] << 8);
			mode &= 07777;
			D(5,("new file mode = 0%o\n", mode));
			break;
		case REMDATA:
			i = rp->r_lowct + (rp->r_highct << 8);
			D(5,("receiving data, count=%d\n", i));
			fprintf(stderr, ".");
			if (i == 0) {
				fprintf(stderr, "\n");
				chmod(fn, mode);
				close(fp);
				return(rv);
			}
			if (write(fp, rp->r_data, i) != i) {
				if (rv == 0) {
					rv = 1;
					X(("`\nfile write failure\n"));
				}
			}
			break;
		default:
			X(("illegal receive function code %d\n", rp->r_type));
		}
	}
}

remputfn(type)
{
	register struct rembuf *rp;

	rp = &remxbuf;
	rp->r_type = type;
	rp->r_lowct = 0;
	rp->r_highct = 0;
	putremote(rp);
}

remputstr(type, x)
char *x;
{
	register struct rembuf *rp;
	register char **a = &x;
	register i;

	rp = &remxbuf;
	rp->r_type = type;
	sprintf(rp->r_data, a[0], a[1], a[2], a[3], a[4], a[5]);
	i = strlen(rp->r_data);
	rp->r_lowct = i;
	rp->r_highct = i >> 8;
	putremote(rp);
}

putremote(rp)
register struct rembuf *rp;
{
	write(rem, &rp->r_type, 3 + rp->r_lowct + (rp->r_highct<<8));
}

getremote(rp)
register struct rembuf *rp;
{
	register unsigned u;
	register char *cp;
	register i;

	if ((rp->r_type = getch()) == EOF)
		return(1);
	if ((i = getch()) == EOF)
		return(1);
	rp->r_lowct = i;
	if ((i = getch()) == EOF)
		return(1);
	rp->r_highct = i;
	u = rp->r_lowct + (rp->r_highct << 8);
	if (u > NETSIZ) {
		X(("illegal receive count\n"));
		return(1);
	}
	cp = &rp->r_data[0];
	while (u-- != 0) {
		if ((i = getch()) == EOF)
			return(1);
		*cp++ = i;
	}
	*cp = 0;
	return(0);
}

getch()
{
	static char buf[sizeof(remrbuf)];
	static char *cp = 0;
	static ct = 0;
	
	if (ct <= 0) {
		if ((ct = read(rem, buf, sizeof(remrbuf))) <= 0)
			return(EOF);
		cp = buf;
	}
	ct--;
	return(*cp++ & 0xFF);
}
