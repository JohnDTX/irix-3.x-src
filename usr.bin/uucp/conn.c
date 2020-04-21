/* @(#)conn.c	1.10 */
static	char	*Sccsid = "@(#)$Header: /d2/3.7/src/usr.bin/uucp/RCS/conn.c,v 1.1 89/03/27 18:30:16 root Exp $";
/*
 * $Log:	conn.c,v $
 * Revision 1.1  89/03/27  18:30:16  root
 * Initial check-in for 3.7
 * 
 * Revision 1.1  87/09/23  17:04:29  vjs
 * Initial revision
 * 
 * Revision 1.8  85/02/07  22:47:14  root
 * Fixed typo.
 * 
 * Revision 1.7  85/02/07  22:42:45  bob
 * Added all of 4.2 backslash sequences and \n which they left out.
 * 
 */

#define CONN
#include <stdio.h>
extern	int	Debug;
#define	MYBSIZ	1024
char	Mybuf[MYBSIZ];
int	Myi	= 0;
char	*Mypc;
int	fixitgreg = -2;	/* != 0 for xns since it must do 512 byte reads!!!*/
FILE	*oopsout = NULL;

int
Read(fd, buf, count)
int	fd;
char	*buf;
int	count;
{
	int	i;
	int	ret;
	extern	int	Ifn;
	char	c;	/* debugging */

	if (fd != fixitgreg && fd != Ifn) {
		return read(fd, buf, count);
	} else {
		if (count <= 0) {
			return count;
		}
		ret = 0;
		while (count > 1) {
			i = Read(fd, buf, 1);
			if (i < 1) {
				if (i == 0)
					return ret;
				return i;
			}
			buf++;
			count--;
			ret++;
		}
		if (Myi <= 0) {
			i = read(fd, Mybuf, MYBSIZ);
			if (i < 1) {
				if (i == 0)
					return ret;
				return i;
			}
			Myi = i;
			Mypc = Mybuf;
			if (Debug > 0) {
				if (oopsout == NULL) {
					i = umask(0);
					oopsout = fopen(
					  "/usr/spool/uucp/xnsdebug","w");
					umask(i);
				}
				fprintf(oopsout,"Read:'");
			}
			for (i=0; i<Myi; i++) {
				c = Mybuf[i];
				if (Debug > 0) {
					if (c & 0200) {
						fprintf(oopsout,"M-");
						c &= ~0200;
					}
					if ((c<' ' || c>126) && c != '\n')
						fprintf(oopsout,"\\%03o", c);
					else
						fprintf(oopsout,"%c", c);
				}
			}
			if (Debug > 0)
				fprintf(oopsout,"'\n");
		}
		*buf = *Mypc++;
		Myi--;
		ret++;
		return ret;
	}
}

int
Write(fd, buf, count)
int	fd;
char	*buf;
int	count;
{
	char	*p;
	char	c;
	int	i;
	extern	int	Ofn;

	if (fd == Ofn) {
		p = buf;
		i = count;
		if (Debug > 0) {
			if (oopsout == NULL) {
				i = umask(0);
				oopsout = fopen("/usr/spool/uucp/xnsdebug","w");
				umask(i);
			}
			fprintf(oopsout,"Write(%d, %d, count): Ofn=%d\n",
			  fd, buf, count, Ofn);
			fprintf(oopsout,"Write:'");
			while (i-- > 0) {
				c = *p++;
				if (c & 0200) {
					fprintf(oopsout,"M-");
					c &= ~0200;
				}
				if ((c<' ' || c>126) && c != '\n')
					fprintf(oopsout,"\\%03o", c);
				else
					fprintf(oopsout,"%c", c);
			}
			fprintf(oopsout,"'\n");
		}
	}
	return write(fd, buf, count);
}

#define	read	Read
#define	write	Write

#include "uucp.h"
#include <signal.h>
#include <setjmp.h>
#include <ctype.h>
#include <sys/types.h>
#include <time.h>
#include <fcntl.h>

#include <sys/stat.h>

#ifdef	XNS
#include "xns/Xns.h"
#include <sys/ioctl.h>
#include "xns/Xnsioctl.h"
#endif

#ifdef DATAKIT
#include <dk.h>
#endif

#define MAXPH 60

#define F_NAME 0
#define F_TIME 1
#define F_LINE 2
#define F_CLASS 3	/* an optional prefix and the speed */
#define F_PHONE 4
#define F_LOGIN 5

extern jmp_buf Sjbuf;
unsigned alarm();
unsigned sleep();
int alarmtr();
char *strchr();

#ifdef XNS
/* uses direct() */
int xnscls();	/* maybe not System V */
#endif

/*
 * place a telephone call to system and
 * login, etc.
 * returns:
 *	CF_SYSTEM	-> don't know system
 *	CF_TIME		-> wrong time to call
 *	CF_DIAL		-> call failed
 *	CF_LOGIN	-> login/password dialog failed
 *	>0  - file no.  -  connect ok
 */
int	unacu;
int	acuspeed;
long	tnoacu;
#define NOACUS		-2
char	*fdig();
conn(system)
char *system;
{
	register FILE *fsys;
	register int nf, fn;
	int ret;
	int fcode = 0;
	char *flds[50];

	fsys = fopen(SYSFILE, "r");
	ASSERT(fsys != NULL, "CAN'T OPEN", SYSFILE, 0);
	DEBUG(4, "finds %s\n", "called");
	while((nf = finds(fsys, system, flds)) > 0) {
		fcode = 0;
		DEBUG(4, "getto %s\n", "called");
		if ((fn = getto(flds)) <= 0) {
			fcode = CF_DIAL;
			DEBUG(8, "getto %s\n", "failed");
			continue;	/* connect failed - try another line */
		}
		DEBUG(4, "login %s\n", "called");
		if (login(nf, flds, fn) == FAIL) {
			close(fn);
			fcode = CF_LOGIN;/* login failed - try another line */
			DEBUG(8, "login %s\n", "failed");
			continue;
		}
		break;	/* successfully logged in */
	}
	fclose(fsys);
	if (nf < 0 || fcode)	/* couldn't connect & login over any entry */
		return(fcode ? fcode : nf);

	return(fn);
}


	/*
	 * tells about a device
	 */
struct Devices {
	int D_speed;
	char D_type[10];
	char D_line[10];
	char D_calldev[10];
	char D_class[10];
};
struct dlines{
	int	d_speed;
	char	d_type;
	char	d_prot;
	char	*d_line;
	char	*d_calldev;
};
struct dlines dlines[NDLINES];
#define D_ACU		1
#define D_DIRECT	2
#define D_PROT		4
int	nacu;
int	ndirect;
struct dlines *pdirect;
struct dlines *pacu;
int	dinit;
struct Devices d;
struct Devices *dev = &d;

/*
 * cannect to remote machine
 * returns:
 *	0	-> file number ok
 *	FAIL	-> failed
 */
getto(flds)
register char *flds[];
{
	DEBUG(4, "call: no. %s ", flds[F_PHONE]);
	DEBUG(4, "for sys %s ", flds[F_NAME]);

	if(dinit == 0){
		linit();
		dinit++;
	}
	if (prefix("ACU", flds[F_LINE]))
		return(call(flds));

#ifdef DATAKIT
	else if (prefix("DK", flds[F_LINE]))
		return(dkcall(flds));
#endif

	else
		return(direct(flds));
}
/*
 * call remote machine
 *	flds	-> contains the call information (name
 *	 	-> type, speed, phone no. ...
 *	Ndev	-> has the device no.
 * returns:
 *	0	-> file number ok
 *	FAIL	-> failed
 */
call(flds)
register char *flds[];
{
	register int dcr, i;
	long	t;
	char phone[MAXPH+1];
	int	speed;

#ifndef NOROT
	if(unacu){
		speed = atoi(fdig(flds[F_CLASS]));
		if(speed == acuspeed){
			if(time((long *)0)  > (tnoacu + ACULTNCY)){
				unacu = 0;
			}else{
				DEBUG(4, "PREVIOUS ACU LOCKUP %d\n",acuspeed);
				logent("PREVIOUS ACU LOCKUP", "FAILED");
				return(FAIL);
			}
		}else
			unacu = 0;
	}
#endif
	exphone(flds[F_PHONE], phone);
	DEBUG(4, "call fnc %s\n", phone);
	for (i = 0; i < TRYCALLS; i++) {
		DEBUG(4, "Dial %s\n", phone);
		t = times(&nstat.t_tga);
		dcr = dialup(phone, flds);
		nstat.t_tacu += times(&nstat.t_tga) - t;
		nstat.t_ndial++;
		DEBUG(4, "dcr returned as %d\n", dcr);
		if(dcr == NOACUS){
			unacu++;
			tnoacu = time((long *)0);
			return(FAIL);
		}
		if (dcr != FAIL)
			break;
		sleep(5);
	}
	return(dcr);

}

/*
 * expand phone number for given prefix and number
 * return:
 *	none
 */
exphone(in, out)
register char *in;
char *out;
{
	register FILE *fn;
	register char *s1;
	char pre[MAXPH], npart[MAXPH]; 
	char buf[BUFSIZ], tpre[MAXPH], p[MAXPH];
	char *strcpy(), *strcat();

	if (!isalpha(*in)) {
		strcpy(out, in);
		return;
	}

	s1=pre;
	while (isalpha(*in))
		*s1++ = *in++;
	*s1 = '\0';
	s1 = npart;
	while (*in != '\0')
		*s1++ = *in++;
	*s1 = '\0';

	tpre[0] = '\0';
	fn = fopen(DIALFILE, "r");
	if (fn != NULL) {
		while (fgets(buf, BUFSIZ, fn)) {
		if((buf[0] == '#') || (buf[0] == ' ') || (buf[0] == '\t') || 
			(buf[0] == '\n'))
				continue;
			sscanf(buf, "%s%s", p, tpre);
			if (strcmp(p, pre) == SAME)
				break;
			tpre[0] = '\0';
		}
		fclose(fn);
	}

	strcpy(out, tpre);
	strcat(out, npart);
	return;
}

	/*
	 * file descriptor for call unit
	 */
int Dnf = 0;

/*
 * dial remote machine
 * returns:
 *	file descriptor	-> succeeded
 *	FAIL		-> failed
 */
char	phone[MAXPH+2];
dialup(ph, flds)
char *ph, *flds[];
{
	register struct dlines *dp;
	register int i;
	unsigned nw, lt;
	unsigned timelim;
	int	speed;
	int	dcdelay, dcf;
	char dcname[20], dnname[20];
	char	cb[128+MAXPH];
	long	ts,te;
	int	ind;
#ifdef RT
	int	pid, status, npid;
#endif

	Pprot[0] = 0;
	speed = atoi(fdig(flds[F_CLASS]));
	dp = pacu;
#ifndef NOROT
	if(Debug == 0){
		ind = 0;
		if(nacu)
			ind = (time((long *)0))%(long)nacu;
		dp = &pacu[ind];
	}
#endif
DEBUG(8,"rot %d\n",ind);
DEBUG(8,"SPEED %d\n",speed);
DEBUG(8,"LINE %s\n",flds[F_CLASS]);
DEBUG(8,"nacu %d\n",nacu);
	if(pacu == NULL)
		goto err;
	acuspeed = speed;
	for(i=0;i<nacu;i++,dp++) {
		if(dp >= &pacu[nacu])
			dp = pacu;
DEBUG(8,"line %s\n",dp->d_line);
DEBUG(8,"acu %s\n",dp->d_calldev);
DEBUG(8,"acu %d\n",dp->d_speed);
		if(dp->d_speed != speed)
			continue;
DEBUG(4,"mlock %s\n",dp->d_line);
		if (mlock(dp->d_line) == FAIL)
			continue;
DEBUG(4,"d_type %s\n",dp->d_line);
		if(dp->d_type&D_PROT)
			Pprot[0] = dp->d_prot;
		sprintf(dnname, "/dev/%s", dp->d_calldev);
		sprintf(dcname, "/dev/%s", dp->d_line);
DEBUG(4,"open %s\n",dp->d_line);
		if ((Dnf = open(dnname, 1)) > 0){
			strncpy(&dc[0], dp->d_line, 10);
			goto fnd;
		}
DEBUG(4,"didn'twork %s\n",dp->d_line);
		delock(dp->d_line);
	}
err:
	logent("DEVICE", "NO");
	DEBUG(4, "NO DEVICE %s", "");
	return(NOACUS);

fnd:
	sprintf(phone, "%s%s", ph, ACULAST);
	DEBUG(4, "dc - %s, ", dcname);
	DEBUG(4, "acu - %s\n", dnname);
	time(&ts);
	tconv = ts;
#ifdef RT
	if (setjmp(Sjbuf)) {
		DEBUG(1, "DN write %s\n", "timeout");
		kill(pid, 9);
		time(&te);
		sprintf(cb, "%s %s P%s %ld", "DIALUP ON write", dc, phone, te-ts);
		logent(cb, "TIMEOUT");
		delock(dp->d_line);
		close(Dnf);
		Dnf = 0;
		close(dcdelay);
		close(dcf);
		return(FAIL);
	}
	signal(SIGALRM, alarmtr);
	timelim = 5 * strlen(phone);
	alarm(timelim < 30 ? 30 : timelim);
	if ((pid = fork()) == 0) {
		sleep(2);
		fclose(stdin);
		fclose(stdout);
		nw = write(Dnf, phone, lt = strlen(phone));
		if (nw != lt) {
			DEBUG(1, "ACU write %s\n", "error");
			sprintf(cb, "%s %s P%s %ld", "DIALUP ACU write", dc, phone, te-ts);
			logent(cb, "FAILED");
			exit(1);
		}
		DEBUG(4, "ACU write ok%s\n", "");
		exit(0);
	}
	/*  open line - will return on carrier */
	/* RT needs a sleep here because it returns immediately from open */

	sleep(15);
	dcf = open(dcname, 2);
	DEBUG(4, "dcf is %d\n", dcf);
	if (dcf < 0) {
		alarm(0);
		kill(pid, 9);
		DEBUG(1, "Line open %s\n", "failed");
		time(&te);
		sprintf(cb, "%s %s P%s %ld", "DIALUP LINE open", dc, phone, te-ts);
		logent(cb, "FAILED");
		delock(dp->d_line);
		return(FAIL);
	}
	sethup(dcf);
	while ((npid = wait(&status)) != pid && npid != -1)
		;
	alarm(0);
	time(&te);
	fflush(stdout);
	fixline(dcf, dp->d_speed);
	DEBUG(4, "Fork Stat %o\n", lt);
	if (status != 0) {
		close(dcf);
		close(Dnf);
		delock(dp->d_line);
		return(FAIL);
	}
	return(dcf);
#else
	if (setjmp(Sjbuf)) {
		DEBUG(1, "DN write %s\n", "timeout");
		time(&te);
		sprintf(cb, "%s %s P%s %ld", "DIALUP ON write", dc, phone, te-ts);
		logent(cb, "TIMEOUT");
		delock(dp->d_line);
		close(Dnf);
		Dnf = 0;
		close(dcdelay);
		close(dcf);
		return(FAIL);
	}
	signal(SIGALRM, alarmtr);
	timelim = 5 * strlen(phone);
	alarm(timelim < 30 ? 30 : timelim);
	dcdelay = open(dcname, O_RDWR | O_NDELAY);
	nw = write(Dnf, phone, lt = strlen(phone));
	if (nw != lt) {
		alarm(0);
		DEBUG(1, "ACU write %s\n", "error");
		time(&te);
		sprintf(cb, "%s %s P%s %ld", "DIALUP ACU write", dc, phone, te-ts);
		logent(cb, "FAILED");
		close(Dnf);
		Dnf = 0;
		close(dcdelay);
		delock(dp->d_line);
		return(FAIL);
	}else 
		DEBUG(4, "ACU write ok%s\n", "");

	dcf = open(dcname, 2);
	close(dcdelay);
	close(Dnf);
	Dnf = 0;
	DEBUG(4, "dcf is %d\n", dcf);
	if (dcf < 0) {
		alarm(0);
		DEBUG(1, "Line open %s\n", "failed");
		time(&te);
		sprintf(cb, "%s %s P%s %ld", "DIALUP LINE open", dc, phone, te-ts);
		logent(cb, "FAILED");
		delock(dp->d_line);
		return(FAIL);
	}
	sethup(dcf);
	alarm(0);
	time(&te);
	fflush(stdout);
	fixline(dcf, dp->d_speed);
	sprintf(cb, "%s %s P%s %ld", "DIAL", dc, phone, te-ts);
	logent(cb, "OK");
	return(dcf);
#endif
}

linit()
{
	register FILE *dfp;
	register int na, i;
	struct dlines *dp;
	int	compa();
	char *fdig();
	char buf[BUFSIZ];

	DEBUG(8, "linit: %s", "");
	dfp = fopen(DEVFILE, "r");
	ASSERT(dfp != NULL, "CAN'T OPEN", DEVFILE, 0);
	while (fgets(buf, BUFSIZ, dfp) != NULL) {
		DEBUG(8, " got line - %s", "");
		if((buf[0] == '#') || (buf[0] == ' ') || (buf[0] == '\t') || 
			(buf[0] == '\n'))
			continue;
		na = sscanf(buf, "%s%s%s%s%s", dev->D_type, dev->D_line,
		  dev->D_calldev, dev->D_class, Pprot);
		ASSERT(na >= 4, "BAD LINE", buf, 0);
		DEBUG(8, "%s ", dev->D_type);
		DEBUG(8, "%s ", dev->D_line);
		DEBUG(8, "%s ", dev->D_calldev);
		DEBUG(8, "%s ", dev->D_class);
		DEBUG(8, "%s\n", Pprot);
		if(nacu + ndirect >= NDLINES)
			logent("NOT ENOUGH DLINES","");
		ASSERT(nacu+ndirect < NDLINES, "NOT ENOUGH DLINES", DEVFILE, 0);
		dp = &dlines[nacu+ndirect];
		if(strcmp(dev->D_type, "DIR") == SAME){
			ndirect++;
			dp->d_type |= D_DIRECT;
		}else
		if(strcmp(dev->D_type, "ACU") == SAME){
			nacu++;
			dp->d_type |= D_ACU;
			dp->d_calldev = (char *)calloc(strlen(dev->D_calldev)+1, 1);
			if(dp->d_calldev)
				strcpy(dp->d_calldev, dev->D_calldev);
		}else
			continue;
		if(Pprot[0]){
			dp->d_type |= D_PROT;
			dp->d_prot = Pprot[0];
		}
		dp->d_line = (char *)calloc(strlen(dev->D_line)+1, 1);
		if(dp->d_line)
			strcpy(dp->d_line, dev->D_line);
		dp->d_speed = atoi(fdig(dev->D_class));
	}
	qsort(&dlines[0], nacu+ndirect, sizeof(struct dlines), compa);
	dp = &dlines[0];
	for(i=0;i<nacu+ndirect;i++,dp++){
		if(pdirect == NULL)
			if(dp->d_type&D_DIRECT)
				pdirect = dp;
		if(pacu == NULL)
			if(dp->d_type&D_ACU)
				pacu = dp;
	}
	fclose(dfp);
	return(FAIL);
}
compa(p, q)
register struct dlines *p, *q;
{
	int	a, b;

	a = p->d_type&D_ACU?0:1;
	b = q->d_type&D_ACU?0:1;
	if(a == b)
		return(0);
	if(a < b)
		return(1);
	return(-1);
}

/*
 * close call unit
 * return:
 *	none
 */
clsacu()
{
	if (Dnf > 0) {
		close(Dnf);
		sleep(5);
		Dnf = 0;
	}
	return;
}

/*
 * connect to hardware line
 * return:
 *	0	-> file number  ok
 *	FAIL	-> failed
 */
direct(flds)
register char *flds[];
{
	register int dcr;
	register int i;
	register struct dlines *dp;
	char dcname[20];
	char	cb[128+MAXPH];
	long	ts,te;
	register int speed;
#ifdef	XNS
	struct xns_setup dial;
#endif

	Pprot[0] = 0;
	strcpy(phone, "direct");
	speed = atoi(fdig(flds[F_CLASS]));
	dp = pdirect;
DEBUG(8,"SPEED %d\n",speed);
DEBUG(8,"LINE %s\n",flds[F_CLASS]);
DEBUG(8,"ndirect %d\n",ndirect);
	if(pdirect == NULL)
		goto err;
	for(i=0;i<ndirect;i++,dp++) {
DEBUG(8,"direct %s\n",dp->d_line);
DEBUG(8,"direct %d\n",dp->d_speed);
		if(dp->d_speed != speed)
			continue;
		if(strcmp(flds[F_LINE], dp->d_line) != SAME)
			continue;
		if (
#ifdef	XNS
		  strcmp(flds[F_CLASS],XNSCLASS) == SAME ||
#endif
		  mlock(dp->d_line) != FAIL) {
			if(dp->d_type&D_PROT)
				Pprot[0] = dp->d_prot;
			goto fnd;
		}
	}
err:
	logent("DEVICE", "NO");
	DEBUG(4, "NO DEVICE %s", "");
	return(FAIL);
fnd:
	strncpy(&dc[0], dp->d_line, 10);
	time(&ts);
	tconv = ts;

	sprintf(dcname, "/dev/%s", dp->d_line);
	signal(SIGALRM, alarmtr);
	alarm(20);
	if (setjmp(Sjbuf)) {
		time(&te);
		sprintf(cb, "%s %s %ld", "DIRECT LINE WRITE", dc, te-ts);
		logent(cb, "TIMEOUT");
#ifdef	XNS
				/* XNS device is multiplexed; don't lock */
		if (strcmp(flds[F_CLASS],XNSCLASS) != SAME)
#endif
			delock(dp->d_line);
		return(FAIL);
	}
	/*
	 * read/write
	 */
#ifdef	XNS
	if (strcmp(flds[F_CLASS],XNSCLASS) == SAME) {
		DEBUG(8, "About to open xns channel with xnsfile()\n", NULL);
		dcr = xnsfile();
		if (dcr < 0) {
			DEBUG(1, "Can't open xns network file\n", NULL);
			logent("xns network file","CAN'T OPEN");
			return FAIL;
		}
		dial.internet.socket = LOGINSOCKET;
		strncpy(dial.name, flds[F_NAME], NSIZE);
		DEBUG(8, "About to connect to system %s\n", flds[F_NAME]);
		if (ioctl(dcr, NXCONNECT, &dial) < 0) {
			DEBUG(1, "Can't connect to %s\n", flds[F_NAME]);
			logent(flds[F_NAME], "CAN'T CONNECT TO OVER XNS");
			close(dcr);
			DEBUG(4, "diropn returning FAIL\n", NULL);
			return FAIL;
		}
		DEBUG(8, "xns dcr is %d\n", dcr);
		DEBUG(8, "Fix It Greg!!!\n", NULL);
		fixitgreg = dcr;
	} else {
		DEBUG(8, "About to open direct line to %s\n", dcname);
#endif
		dcr = open(dcname, 2); /* read/write */
#ifdef	XNS
	}
#endif
	alarm(0);
/*	next_fd = -1; 4.2 */
	DEBUG(8, "xns dcr is still %d\n", dcr);
	if (dcr < 0) {
		DEBUG(4, "OPEN FAILED %s\n", dcname);
		time(&te);
		sprintf(cb, "%s %s %ld", "DIRECT LINE OPEN", dc, te-ts);
		logent(cb, "FAILED");
#ifdef	XNS
		if (strcmp(flds[F_CLASS],XNSCLASS) != SAME)
#endif
			delock(dev->D_line);
		return(FAIL);
	}
	fflush(stdout);
	DEBUG(8, "fixline(fd,dev->D_speed=%d)\n",dev->D_speed);
	DEBUG(8, "fixline(fd,dp->d_speed=%d)\n",dp->d_speed);
#ifdef	XNS
	if (strcmp(flds[F_CLASS],XNSCLASS) != SAME) {
#endif
		DEBUG(8, "direct:serial line:calling fixline(dcr,speed:%d)\n", dp->d_speed);
/*		fixline(dcr, dev->D_speed); 4.2 */
		fixline(dcr, dp->d_speed);
#ifdef	XNS
	} else {
		DEBUG(8, "direct:xns line:calling fixline(dcr,XNSSPEED:%d)\n", XNSSPEED);
		fixline(dcr, XNSSPEED);
	}
#endif
	time(&te);
	sprintf(cb, "%s %s %ld", "DIRECT", dc, te-ts);
	logent(cb, "OK");
	return(dcr);
}


#ifdef DATAKIT

#define DKTRIES 2

/*
 * make datakit connection
 * return:
 *	0	-> file number ok
 *	FAIL	-> failed
 */
dkcall(flds)
char *flds[];
{
	register struct dkaddr *dkphone;
	register ret, i;

	if (setjmp(Sjbuf))
		return(FAIL);
	signal(SIGALRM, alarmtr);
	dkphone = netname(flds[F_NAME]);
	DEBUG(4, "dkphone (%d) ", dkphone->serv);
	for (i = 0; i < DKTRIES; i++) {
		ret = dkdial(D_UU, dkphone);
		DEBUG(4, "dkdial (%d)\n", ret);
		if (ret > -1)
			break;
	}
	return(ret);
}
#endif

#define MAXC 1024

/*
 * set system attribute vector
 * return:
 *	0	-> number of arguments in vector succeeded
 *	CF_SYSTEM	-> system name not found
 *	CF_TIME		-> wrong time to call
 */
finds(fsys, sysnam, flds)
register char *flds[];
FILE *fsys;
char *sysnam;
{
	register int na;
	register int flg;
	static char info[MAXC];
	int fcode = 0;
	char sysn[32];

	/*
	 * format of fields
	 *	0	-> name;
	 *	1	-> time
	 *	2	-> acu/hardwired
	 *	3	-> speed
	 */
	flg = 0;
	if(strlen(sysnam) > 4)
		flg++;
	while (fgets(info, MAXC, fsys) != NULL) {
		if((info[0] == '#') || (info[0] == ' ') || (info[0] == '\t') || 
			(info[0] == '\n'))
			continue;
		if(flg)
			if(sysnam[4] != info[4])
				continue;
		if(info[0] != sysnam[0])
			continue;
		na = getargs(info, flds);
		sprintf(sysn, "%.*s", SYSNSIZE, flds[F_NAME]);
		if (strncmp(sysnam, sysn, SYSNSIZE) != SAME)
			continue;

		/*
		 * found a good entry
		 */
		if (ifdate(flds[F_TIME]))
			return(na);
		logent(sysnam, "WRONG TIME TO CALL");
		fcode = CF_TIME;
	}
	return(fcode ? fcode : CF_SYSTEM);
}

/*
 * do login conversation
 * returns:
 *	0	-> success
 *	FAIL	-> failure
 */
login(nf, flds, fn)
register int nf;
int fn;
char *flds[];
{
	register int k, ok;
	char *want, *altern;
	char	cb[128+MAXPH];

	ASSERT(nf > 4, "TOO FEW LOG FIELDS", "", nf);
	for (k = F_LOGIN; k < nf; k += 2) {
		want = flds[k];
		ok = FAIL;
		while (ok != 0) {
			altern = strchr(want, '-');
			if (altern != NULL)
				*altern++ = '\0';
			DEBUG(4, "wanted %s ", want);
			ok = expect(want, fn);
			DEBUG(4, "got %s\n", ok ? "?" : "that");
			if (ok == 0)
				break;
			if (altern == NULL) {
				sprintf(cb,  "LOGIN (%s, P%s)", dc,  phone);
				logent(cb, "FAILED");
				return(FAIL);
			}
			want = strchr(altern, '-');
			if (want != NULL)
				*want++ = '\0';
			sendthem(altern, fn);
		}
		sleep(2);
		sendthem(flds[k+1], fn);
	}
	return(0);
}


#define MR 300


/*
 * look for expected string
 * returns:
 *	0	-> found
 *	FAIL	-> lost line or too many characters read
 *		->some character  timed out
 */
expect(str, fn)
int fn;
char *str;
{
	static char rdvec[MR];
	register int kr;
	register char *rp = rdvec;
	char nextch;

	if (strcmp(str, "\"\"") == SAME)
		return(0);
	*rp = 0;
	if (setjmp(Sjbuf)) {
		logent("LOGIN", "TIMEOUT");
		return(FAIL);
	}
	signal(SIGALRM, alarmtr);
	alarm(MAXMSGTIME);
	while (notin(str, rdvec)) {
		kr = read(fn, &nextch, 1);
		if (kr <= 0) {
			alarm(0);
			DEBUG(4, "lost line kr - %d\n, ", kr);
			logent("LOGIN", "LOST LINE");
			return(FAIL);
		}
		*rp = (nextch & 0177);
		DEBUG(4, "%c", *rp >= 040 && *rp < 0177 || *rp == '\n' ? *rp : '_');
		if (*rp != '\0')
			rp++;
		*rp = '\0';
		if (rp >= rdvec + MR){
			alarm(0);
			return(FAIL);
		}
	}
	alarm(0);
	return(0);
}


/*
 * catch alarm routine for "expect".
 */
alarmtr()
{
	longjmp(Sjbuf, 1);
}


/*
 * send line of login sequence
 * return
 *	none
 */
sendthem(str, fn)
register int fn;
char *str;
{
struct stat statbuf;
	register int nw = 1;
	register char *strptr;
	int nlw = 1;
	int i;
	int n;

	if (prefix("BREAK", str)) {

		/*
		 * send break
		 */
		genbrk(fn);
		return;
	}

	if (strcmp(str, "EOT") == SAME) {
		write(fn, EOTMSG, (unsigned) strlen(EOTMSG));
		return;
	}

	for (strptr = str; *strptr; strptr++) {
		if (*strptr == '\\') switch(*++strptr) {
		case 's':
			DEBUG(5, "BLANK\n", "");
			*strptr = ' ';
			break;
		case 'r':
			DEBUG(5, "RETURN\n", "");
			*strptr = '\r';
			break;
		case 'n':
			DEBUG(5, "NEWLINE\n", "");
			*strptr = '\n';
			break;
		case 'b':
			if (isdigit(*(strptr+1))) {
				i = (*++strptr - '0');
				if (i <= 0 || i > 10)
					i = 3;
			} else
				i = 3;
				/* send break (second parameter ignored) */
			genbrk(fn, i);
			continue;
		case 'd':
			DEBUG(5, "DELAY\n", "");
			sleep(1);
			continue;
		case 'c':
			if (*(strptr+1) == '\0') {
			DEBUG(5, "NO NL\n", "");
				nlw = 0;
				continue;
			}
			DEBUG(5, "NO NL - MIDDLE IGNORED\n", "");
		default:
			if (isdigit(strptr[1])) {
				i = 0;
				n = 0;
				while (isdigit(strptr[1]) && ++n <= 3)
					i = i*8 + (*++strptr - '0');
				*strptr = i;
				continue;
			}
			DEBUG(5, "BACKSLASH\n", "");
			strptr--;
		}
		nw = write(fn, strptr, 1);
		ASSERT(nw == 1, "BAD WRITE", str, 0);
	}
	if (nlw) write(fn, "\n", 1);
	return;
}

/*
 * check for ccurrence of substring "sh"
 * return:
 *	0	-> found the string
 *	1	-> not in the string
 */
notin(sh, lg)
register char *sh, *lg;
{
	while (*lg != '\0') {
		if (prefix(sh, lg))
			return(0);
		else
			lg++;
	}
	return(1);
}


/*
 * check a string (s) like "MoTu0800-1730"
 * to see if the present time is within the given limits.
 * 	SIDE EFFECT	-> Retrytime is set
 * String alternatives:
 *	Wk	-> Mo thru Fr
 *	zero	-> all day
 *	one	-> all day
 *	Any	-> any day
 * return:
 *	0	-> not within limits
 *	1	-> within limits
 */
ifdate(s)
register char *s;
{
	static char *days[]={
		"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa", 0
	};
	register int i;
	time_t clock, time();
	int rtime;
	int tl, th, tn, flag, dayok=0;
	struct tm *localtime();
	struct tm *tp;
	char *p;

	/*
	 * pick up rety time for failures
	 * global variable Retrytime is set here
	 */
	if ((p = strchr(s, ',')) == NULL) {
		Retrytime = RETRYTIME;
	} else {
		i = sscanf(p+1, "%d", &rtime);
		if (i < 1 || rtime < 5)
			rtime = 5;
		Retrytime  = rtime * 60;
	}

	time(&clock);
	tp = localtime(&clock);
	while (isalpha(*s)) {
		for (i = 0; days[i]; i++) {
			if (prefix(days[i], s))
				if (tp->tm_wday == i)
					dayok = 1;
		}

		if (prefix("Wk", s))
			if (tp->tm_wday >= 1 && tp->tm_wday <= 5)
				dayok = 1;
		if (prefix("Any", s))
			dayok = 1;
		s++;
	}

	if (dayok == 0)
		return(0);
	i = sscanf(s, "%d-%d", &tl, &th);
	tn = tp->tm_hour * 100 + tp->tm_min;
	if (i < 2)
		return(1);
	if (th < tl)

		/*
		 * set up for crossover 2400 test
		 */
		flag = 0;
	else
		flag = 1;
	if ((tn >= tl && tn <= th)
	  || (tn >= th && tn <= tl)) /* test for crossover 2400 */
		return(flag);
	else
		return(!flag);
}

/*
 * find first digit in string
 * return:
 *	pointer to first digit in
 *	string or end of string
 */
char *
fdig(cp)
register char *cp;
{
	register char *c;

	for (c = cp; *c; c++)
		if (*c >= '0' && *c <= '9')
			break;
	return(c);
}
