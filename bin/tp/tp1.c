#include "tp.h"

#define USAGE	"usage: tp [01234567cdfimrtuvwx] name(s)\n"

main(argc,argv)
char **argv;
{
	register char c,*ptr;
	extern cmd(), cmr(),cmx(), cmt();

	tname = tc;
	command = cmr;
	if ((narg = rnarg = argc) < 2)	narg = 2;
	else {
		ptr = argv[1];	/* get first argument */
		parg = &argv[2];	/* pointer to second argument */
		while (c = *ptr++) switch(c)  {
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
				tc[8] = c;
				mt[7] = c;
				continue;

			case 'f':
				tname = *parg++;
				narg--; rnarg--;
				continue;
			case 'c':
				flags |= flc;  continue;
			case 'd':
				setcom(cmd);  continue;
			case 'i':
				flags |= fli;  continue;
			case 'm':
				tname = mt;
				flags |= flm;
				continue;
			case 'r':
				flags &= ~flu;  setcom(cmr);  continue;
			case 's':
				flags |= fls; continue;
			case 't':
				setcom(cmt);  continue;
			case 'u':
				flags |= flu;  setcom(cmr);  continue;
			case 'v':
				flags |= flv;  continue;
			case 'w':
				flags |= flw;  continue;
			case 'x':
				setcom(cmx);  continue;
			default:
				useerr();
		}
	}
	optap();
	top = nptr = nameblk = sbrk(0);
	(*command)();
}

optap()
{
	extern cmr();

	if ((flags & flm) == 0) {	/*  DECTAPE */
		tapsiz = TCSIZ;
		ndirent = TCDIRS;
		fio =open(tc,2);
	} else {			/* MAGTAPE */
		tapsiz = MTSIZ;
		ndirent = MDIRENT;
		if(command == cmr)
			fio = open(tname,1);
		else
			fio = open(tname,0);
	}
	if (fio < 0)  {
		printf("Tape open error\n");
		done();
	}
	ndentb = ndirent/TPB;
	edir = &dir[ndirent];
}

setcom(newcom)
int (*newcom)();
{
	extern cmr();

	if (command != cmr)  	useerr();
	command = newcom;
}

useerr()
{
	printf(USAGE);
	done();
}

/*/* COMMANDS */

cmd()
{
	extern delete();

	if (flags & (flm|flc))	useerr();
	if (narg <= 2)			useerr();
	rddir();
	gettape(delete);
	wrdir();
	check();
}

cmr()
{
	if (flags & (flc|flm))		clrdir();
	else				rddir();
	getfiles();
	update();
	check();
}

cmt()
{
	extern taboc();

	if (flags & (flc|flw))	useerr();
	rddir();
	if (flags & flv)
		printf("   mode    uid gid tapa    size   date    time name\n");
	gettape(taboc);
	check();
}

cmx()
{
	extern extract();

	if (flags & (flc))		useerr();
	rddir();
	gettape(extract);
	done();
}

check()
{
	usage();
	done();
}

done()
{
	printf("End\n");
	exit(0);
}

encode(pname,dptr)	/* pname points to the pathname
			 * nptr points to next location in nameblk
			 * dptr points to the dir entry		   */
char	*pname;
struct	dent *dptr;
{
	register  char *np;
	register n;

	dptr->d_namep = np = nptr;
	if (np > top - NAMELEN)  {
		if(sbrk(BRKINCR) == (char *)-1) {
			printf("Out of core\n");
			done();
		} else
			top += BRKINCR;
	}
	if((n=strlen(pname)) > NAMELEN) {
		printf("Pathname too long - %s\nFile ignored\n",pname);
		clrent(dptr);
	}
	else {
		nptr += n+1;
		strcpy(np, pname);
	}
}

decode(pname,dptr)	/* dptr points to the dir entry
			 * name is placed in pname[] */
char	*pname;
struct	dent *dptr;
{

	strcpy(pname, dptr->d_namep);
}
