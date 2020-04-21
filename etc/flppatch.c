/*	$Header: /d2/3.7/src/etc/RCS/flppatch.c,v 1.1 89/03/27 15:37:44 root Exp $		*/
/*	This is a program to replace adb on the bootable floppy.  Space is
 *	tight on the floppy, so this program is as stripped as possible.
 *
 *	Usage: patch kernel rootdev
 *	Note that rootdev is a hex number as in adb
 */

#include <sys/param.h>
#include "a.out.h"
#include "fcntl.h"
#include "machine/cpureg.h"
#include "ctype.h"


/* size of variables */
#define CHAR	1
#define SHORT	2
#define LONG	3

#define	perr(s)	write(2, s, sizeof(s))

main(argc,argv)
int argc;
char *argv[];
{
	int fd;
	struct nlist list[3];
	register long base;
	int dev;

	/* hard code variable names */
	list[0].n_un.n_name = "_rootdev";
	list[1].n_un.n_name = "_rootfs";
	list[2].n_un.n_name = (char *)0;

	/* usage: patch unix rootdev */
	if ( argc != 3 ) {
		perr("Usage: flppatch kernel rootdev\n");
		exit(1);
	}

	/* open unix file */
	if ( (fd = open(argv[1],O_RDWR)) < 0 ) {
		perr("Can't open kernel\n");
		exit(1);
	}

	if ( nlist(argv[1],list) < 0 ) {
		perr("nlist failed\n");
		exit(1);
	}

	/* ASSUMES that padding is already taken care of in the nlist value */
	dev = hatoi (argv [2]);

	base = KERN_VBASE - sizeof(struct exec);
#ifdef iris
	/*
	 * Pm2 kernel loads at 0x400, but namelist values are absolute,
	 * so to get the right file offsets we must adjust namelist
	 * values.
	 */
	base += 0x400;
#endif
	change(fd, list[0].n_value - base, dev, SHORT);
	change(fd, list[1].n_value - base, minor(dev), SHORT);
}

change(fd, addr, val, size)
int fd;
long addr;
int val, size;
{
	int offset;
	unsigned char c;
	unsigned short s;
	unsigned long l;
	

	if ( (offset = lseek(fd,addr,0)) < 0 ) {
		perr("lseek failure\n");
		exit(1);
	}
		
	switch ( size ) {

	case CHAR:
		c = (unsigned char)val;
		if ( write(fd,&c,sizeof(unsigned char)) < 0 )
			break;
		return;

	case SHORT:
		s = (unsigned short)val;
		if ( write(fd, &s, sizeof(unsigned short)) < 0 )
			break;
		return;

	case LONG:
		l = (unsigned long)val;
		if ( write(fd,&l,sizeof(unsigned long)) < 0 )
			break;
		return;
	}
	perr("write failed\n");
	exit(1);
}

/* hex ascii to integer */
hatoi(s)
char *s;
{
	register char c;
	register int i, digit;

	i = 0;

	while ( (c = *s++) !=  '\0' ) {
		if ( c == 'X' || c == 'x' )
			continue;
		if ( isxdigit(c) == 0 )
			break;
		digit = c - (isdigit(c) ? '0' : isupper(c) ? 'A'-10 : 'a'-10);
		i = (i<<4) + digit;
	}
	return( i );
}
