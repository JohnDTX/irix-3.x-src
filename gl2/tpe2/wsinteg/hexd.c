/**************************************************************************
 *									  *
 * 		 Copyright (C) 1985, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/


#include <sys/types.h>
/*#include <sys/termio.h>*/
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
/*#include <gl.h>*/
#include "pxw.h"
#define BUFSIZE	1024

/*
**	Externals
*/
extern int	errno;
extern char	*getenv();
extern int	strcmp();
extern u_char	Display_xlat[];
extern u_char	Ebc2asc[];


struct stat	Fstat;
int		gcmdcount = 0, j, nohilite = 0, vt100 = 0;
char		check, hilite, type = 'a';
int		lfd, m, o, starflag = 0, zeroflag = 0;
char		digit[] = {'0','1','2','3','4','5','6','7',
			   '8','9','a','b','c','d','e','f'};
char		c1, nbuf[128] = {0};
short		ncount;

main(argc, argv)
int argc;
char *argv[];
{
	u_char buf[BUFSIZE], pbuf[17], *preprint;
	register col, i, n;
	register u_char c, cprevline;
	register u_char *arg1;
	char *evaluep;

	if (argc > 1) {
		if (argv[1][0] == '-') {
			for (i=1; argv[1][i]!='\0'; ++i) {
				switch (argv[1][i])
				{
				case 'a':
					type = 'a'; /* ascii chars */
					break;
				case 'A':
					type = 'A'; /* ascii chars */
					break;
				case 'e':
				case 'E':
					type = 'E'; /* ebcdic */
					break;
				case 'i':
					type = 'i'; /* ascii chars */
					break;
				case 'n':
					nohilite = 1; /* to be printed */
					break;
				case 'o':
					type = 'o'; /* ascii chars */
					break;
				case 'q':
				case 'Q':
					type = 'Q'; /* raw 3274 */
					break;
				case 'r':
					type = 'r'; /* RDDATA */
					break;
				case 's':
					type = 's'; /* special 3274 */
					break;
				case 't':
					type = 't'; /* transmit 3274 */
					break;
				case 'w':
					type = 'w'; /* WRDATA */
					break;
				default:
					usage();
					break;
				}
			}
			if (i==1)
				usage();
		} else
			usage();
	} else
		usage();

	if ((evaluep = getenv("TERM"))!=NULL) {
#ifdef MSDOS
		if (strcmp(evaluep, "ibmpc") == 0)
			vt100++;
#else
		if (strcmp(evaluep, "vt100") == 0)
			vt100++;
#endif /* MSDOS */
	}
	if ((lfd = (stat(argv[2], &Fstat))) < 0) {
		printf("path bad status %d errno %d\n",lfd,errno);
		exit(0);
	}
#ifdef MSDOS
	if ((lfd = (open(argv[2], O_BINARY | O_RDONLY,"r"))) <= 0) {
#else
	if ((lfd = (open(argv[2], O_RDONLY,"r"))) <= 0) {
#endif /* MSDOS */
		printf("EXIT from hexd with bad open \n");
		exit(0);
	}
	m = o = 0;
	n = Fstat.st_size;
	printf("\n%s 0x%x",argv[2],n);
	if (argc == 4) {
		arg1 = (u_char *)&argv[3][0];
		if (argv[3][0] == 'x')
			arg1 = (u_char *)&argv[3][1];
		else if (argv[3][0] == '0' && argv[3][1] == 'x')
			arg1 = (u_char *)&argv[3][2];
		while (*arg1)
			buf[m++] = *arg1++;
		sscanf(buf,"%lx",&m);
		m &= 0xfffffc00;	/* BUFSIZE is unit of offset */
		if (m >= n) {
			printf(" offset exceeds file size\n");
			exit(1);
		}
		if (m)
			printf(" offset 0x%x",m);
		else
			printf(" offset < %x is ignored",BUFSIZE);
/*		m >>= 10;		/* BUFSIZE is unit of offset */
/*		while (m > 0 && n > 0) {
			read(lfd,buf,BUFSIZE);
			m--;
			n -= BUFSIZE;
			o += BUFSIZE;
		}*/
		lseek(lfd,m,0);
		n -= m;
		o += m;
	}
	hilite = m = o;
	if (type != 'A' && type != 'E' && type != 'Q') {
		sprintf(nbuf,"\n%07x  ",m);
		ncount = 10;
	while (n > 0) {
		n = read(lfd,buf,BUFSIZE);
		arg1 = buf;
		for (col = 1, i=1; i <= n; col++, i++) {
			c = *arg1++;
			if (type == 'r') {
				if (c == 5 ||c == 0x10) { /* buck */
					hithis(c);
				} else
					nprintf(c);
				pbuf[col] = c;
				if ((char)c < ' ' || 0x7f <= (char)c ||c == '%')
					pbuf[col] = '.';
			} else if (type == 'w') {
				if (c == 5 ||c == 0x7e) { /* buck */
					hithis(c);
				} else
					nprintf(c);
				pbuf[col] = c;
				if ((char)c < ' ' || 0x7f <= (char)c ||c == '%')
					pbuf[col] = '.';
			} else if (type == 's') {
				if (c == 0xe6) { /* end_write */
					if ( (*(arg1+4) != (u_char)0xc3)
						&& (*(arg1+8)!=(u_char)0xc3)) {
						hion();
						hilite++;
					}
					nprintf(c);
				} else if (c == 0xc3) { /* begin_write */
					nprintf(c);
					if (hilite) {
						hioff();
						hilite = 0;
					}
				} else
					nprintf(c);
				pbuf[col] = Display_xlat[c];
				if (c < 7 || pbuf[col] == 0x0a)
					pbuf[col] = ' ';
				if (pbuf[col] == '%')
					pbuf[col] = '.';
			} else if (type == 't') {
				if (c == 0xeb || c == 0xef ) {
					hithis(c);
				} else
					nprintf(c);
				pbuf[col] = Display_xlat[c];
				if (c < 7 || pbuf[col] == 0x0a)
					pbuf[col] = ' ';
				if (pbuf[col] == '%')
					pbuf[col] = '.';
			} else if (type=='i' || type=='o') {
				if (c ==0x05||c==0x10||c == 0x7e) { /* RESC */
					hithis(c);
					if (c == 0x10 || c == 0x7e)
						gcmdcount++;
				} else
					nprintf(c);
				pbuf[col] = c;
				if ((char)c < ' ' || 0x7f <= (char)c ||c == '%')
					pbuf[col] = '.';
			} else if (type == 'a') {
				if (c < ' ' ||c > 0x7e) { /* buck */
					hithis(c);
				} else
					nprintf(c);
				pbuf[col] = c;
				if ((char)c < ' ' || 0x7f <= (char)c ||c == '%')
					pbuf[col] = '.';
			} else {
				if (c == (u_char)0xef) { /* cent */
					hithis(c);
				} else
					nprintf(c);
				pbuf[col] = Ebc2asc[c];
				if (pbuf[col] <= ' ')
					pbuf[col] = ' ';
				if (pbuf[col] == '%')
					pbuf[col] = '.';
			}
			if (col == 16) {
				if (hilite)
					hioff();
				nbuf[ncount++] = ' ';
				nbuf[ncount++] = ' ';
				for (col = 1; col < 17; col++)
					nbuf[ncount++] = pbuf[col];
				m += 16;
				if (gcmdcount) {
					nbuf[ncount++] = ' ';
					nbuf[ncount++] = ' ';
					ncount += 5;
					j = gcmdcount;
					nbuf[ncount--] = (j % 10) + '0';
					j /= 10;
					nbuf[ncount--] = (j % 10) + '0';
					j /= 10;
					nbuf[ncount--] = (j % 10) + '0';
					j /= 10;
					nbuf[ncount--] = (j % 10) + '0';
					j /= 10;
					nbuf[ncount--] = (j % 10) + '0';
					j /= 10;
					nbuf[ncount] = (j % 10) + '0';
					ncount += 6;
				}
				nbuf[ncount] = 0;
				printf(nbuf);
				sprintf(nbuf,"\n%07x  ",m);
				ncount = 10;
				if (hilite)
					hion();
				col = 0;
			} else if (col % 2 == 0)
				nbuf[ncount++] = ' ';
		}
	}
	} else {
	cprevline = check = 0;
	starflag = 0;
	while (n > 0) {
		n = read(lfd,buf,BUFSIZE);
		arg1 = buf;
		for (col = 1, i = 1; i <= n; col++, i++) {
			if (col == 1 && *arg1 == cprevline) {
				preprint = arg1;
				for (check = 1; check < 17; check++) {
					if (*preprint++ != cprevline) {
						col = 1;
						cprevline = *(preprint - 1);
						starflag = 0;
						break;
					}
				}
				if (check == 17) {
					if (++starflag == 1)
						printf("\n      *  %02x",cprevline);
					if (starflag != 0) {
						arg1 = preprint;
						col = 0;
						m += 16;
						i += 15;
					} else
						col = 1;
				}
			} else if (col == 1) {
				check = 1;
				starflag = 0;
			}
			if (col == 1) {
				sprintf(nbuf,"\n%07x  ",m);
				ncount = 10;
			}
			if (check != 17 || starflag == 0) {
				c = *arg1++;
				if (type == 'A') {
					if (c == (u_char)0xef) { /* cent */
						hithis(c);
					} else
						nprintf(c);
					pbuf[col] = c;
				if ((char)c < ' ' || 0x7f <= (char)c ||c == '%')
						pbuf[col] = '.';
				} else if (type == 'Q') {
					if (c == 0x45 || c == (u_char)0xef || c == 0x46) { /* cent was 0x1b now 0x45 also add inbound 0x46 wpc */
						hithis(c);
					} else
						nprintf(c);
					pbuf[col] = Display_xlat[c];
					if (pbuf[col] <= ' ')
						pbuf[col] = ' ';
					if (pbuf[col] == '%')
						pbuf[col] = '.';
				} else {
					if (c == (u_char)0xef) { /* cent */
						hithis(c);
					} else
						nprintf(c);
					pbuf[col] = Ebc2asc[c];
					if (pbuf[col] <= ' ')
						pbuf[col] = ' ';
					if (pbuf[col] == '%')
						pbuf[col] = '.';
				}
				if (col == 16) {
					nbuf[ncount++] = ' ';
					nbuf[ncount++] = ' ';
					for (col = 1; col < 17; col++)
						nbuf[ncount++] = pbuf[col];
					col = 0;
					cprevline = c;
					m += 16;
					nbuf[ncount] = 0;
					printf(nbuf);
				} else if (col % 2 == 0)
					nbuf[ncount++] = ' ';
			}
		}
	}
	}
	nbuf[ncount++] = '\n';
	nbuf[ncount] = 0;
	if (col != 1 || ncount <= 50) {
		printf(nbuf);
	} else
		printf("\n");
}

hioff()
{
	if (nohilite)
		return;
	else if (vt100) {
		nbuf[ncount++] = 0x1b;
		nbuf[ncount++] = '[';
		nbuf[ncount++] = '0';
		nbuf[ncount++] = 'm';
	} else {
		nbuf[ncount++] = 0x1b;
		nbuf[ncount++] = '0';
		nbuf[ncount++] = '@';
	}
}

hion()
{
	if (nohilite)
		return;
	else if (vt100) {
		if (nbuf[ncount-4] == 0x1b) {
			ncount -= 4;
			return;
		} else if (nbuf[ncount-1] == ' ' && nbuf[ncount-5] == 0x1b) {
			ncount -= 5;
			nbuf[ncount++] = ' ';
			return;
		}
		nbuf[ncount++] = 0x1b;
		nbuf[ncount++] = '[';
		nbuf[ncount++] = '1';
		nbuf[ncount++] = 'm';
	} else {
		if (nbuf[ncount-3] == 0x1b) {
			ncount -= 3;
			return;
		} else if (nbuf[ncount-1] == ' ' && nbuf[ncount-4] == 0x1b) {
			ncount -= 4;
			nbuf[ncount++] = ' ';
			return;
		}
		nbuf[ncount++] = 0x1b;
		nbuf[ncount++] = '9';
		nbuf[ncount++] = 'P';
	}
}

hithis(c)
register u_char c;
{
	if (nohilite)
		nprintf(c);
	else {
		hion();
		nprintf(c);
		hioff();
	}
}

/*
**	Display usage message and exit program
*/
usage()
{
	(void)printf("\007\nUsage: hexd -[aeinorw] filename [hex offset]\n");
	exit(1);
}

nprintf(c)
register u_char c;
{

	c1 = c >> 4;
	nbuf[ncount++] = digit[c1];
	c1 = c & 0x0f;
	nbuf[ncount++] = digit[c1];
}
