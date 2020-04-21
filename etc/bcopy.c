char _Origin_[] = "System V";
/* $Source: /d2/3.7/src/etc/RCS/bcopy.c,v $ */
static	char	*Sccsid = "@(#)$Revision: 1.1 $";
/* $Date: 89/03/27 15:37:31 $ */

/*	@(#)bcopy.c	1.1	*/
#include <sys/param.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/sysmacros.h>

#undef	BSIZE
#undef	BSHIFT
#define	BSIZE	512		/* documentation says 512-byte blocks! */
#define	BSHIFT	9

long number();
int	fi;
int	fo;
int	o;
long	lso;
int	n;
int	null;
int	error;
char	ibuf[100];
char	obuf[100];
char	nbuf[100];
char	buf[BSIZE];

main()
{

loop1:
	printf("to: ");
	string(obuf);
	if(null)
		exit(0);
	fo = open(obuf, 1);
	if(fo < 0) {
		fo = creat(obuf, 0666);
		if(fo < 0) {
			perror("creat");
			goto loop1;
		}
		printf("%s created\n", obuf);
	}
	printf("offset: ");
	lso = number();
	if(error) {
		close(fo);
		goto loop1;
	}
	lseek(fo, lso<<BSHIFT, 0);

loop2:
	printf("from: ");
	string(ibuf);
	if(null) {
		close(fo);
		goto loop1;
	}
	fi = open(ibuf, 0);
	if(fi < 0) {
		perror("open");
		goto loop2;
	}
	printf("offset ");
	lso = number();
	if(error)
		goto loop3;
	lseek(fi, lso<<BSHIFT, 0);
	printf("count ");
	lso = number();
	if(error)
		goto loop3;
	while(lso--) {
		n = read(fi, buf, BSIZE);
		if(n < 0) {
			perror("read");
			goto loop3;
		}
		if(n == 0) {
			printf("EOF\n");
			break;
		}
		n = write(fo, buf, n);
		if(n < 0) {
			perror("write");
			goto loop3;
		}
	}

loop3:
	close(fi);
	goto loop2;
}

string(cp)
char *cp;
{
	register char *rcp;

	rcp = cp;
	do
		if(read(0, rcp, 1) != 1)
			exit(1);
	while(*rcp++ != '\n');
	*--rcp = '\0';
	null = cp == rcp;
}

long
number()
{
	register char *cp;
	long rn;

	error = 0;
	cp = nbuf;
	string(cp);
	rn = 0;
	while(*cp <= '9' && *cp >= '0')
		rn = rn*10 + *cp++ - '0';
	if(*cp != '\0') {
		printf("bad character in number\n");
		error++;
	}
	return(rn);
}
