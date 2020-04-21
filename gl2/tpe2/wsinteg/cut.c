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
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#define BUFSIZE	16380
#define COLS		80
#define ERROR		-1
#ifdef DEBUG
#define	DT	printf
#define	DT1
#endif

/*typedef	unsigned char	u_char;
typedef	unsigned short	u_short;*/

/*
**	Local variables
*/
int		lfd, dlfd = -1;
u_char		lbuffer[16384];
u_char		nbuffer[16384];	/* double buffered reads */
FILE		*lod = NULL;
int		m, o;
u_char		buf[BUFSIZE+4];

main(argc, argv)
int argc;
char *argv[];
{
	char type = 'a';
	register int col, i, n;
	register u_char c, C;
	register u_char *arg1;

	if (argc > 1) {
		if (argv[1][0] == '-') {
			for (i=1; argv[1][i]!='\0'; ++i) {
				switch (argv[1][i])
				{
				case 'C':
					type = 'C'; /* compress */
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
	}
	if (argc <= 2) {
		usage();
		exit(0);
	}
/*	if ((lfd = (open(argv[2], O_RDONLY,"r"))) <= 0) {
		printf("EXIT from cut with bad open \n");
		exit(0);
	}
	if ((lod = (fopen("CUT","w"))) <= 0) {
		printf("EXIT from cut with bad open \n");
		exit(0);
	}*/
	prep_text(argv[2]);
/*	o = 0;
	n = 2000000;
/*	n = Fstat.st_size;*/
/*	if (argc == 4 && (argv[3][0] == '+')) {
		arg1 = (u_char *)&argv[3][1];
		while (*arg1)
			buf[m++] = *arg1++;
		sscanf(buf,"%ld",&offset);
		printf ("offset to message #%d",offset);
	}
	printf("\n%s %x hex ",argv[2],n);
	while (n > 0) {
		n = read(lfd,buf,BUFSIZE);
		arg1 = buf;
		    for (i=1; i <= n; i++) {
			    c = *arg1++;
			    if (type == 'C') {
				m = fwrite(c,1,BUFSIZE,lod);
/*				sprintf(pbuf,"%02x",c);
				fprintf(lod,pbuf);
				if (!(i % 80))
					fprintf(lod,"\n");
			    }
		    }
		    fflush(lod);
	}
	fflush(lod);
	fclose(lod);*/
}

/*
**	Display usage message and exit program
*/
usage()
{
	(void)printf("\007\nUsage: cut -[C] infilename\n");
	exit(1);
}


/*
**	Prep_text prepares a text file after download, and
**	works on the basis of 80 col mainframe records.
*/
prep_text(ufile)
char *ufile;
{
	register u_char *bp, *path, *str;
	register short flen, length, slength;
	u_char c, firstnib;
	int inblanks, iread, iwrite, jread, uppr;
	long llength;

	inblanks = iread = iwrite = uppr = 0;
/*	if ((lfd = (stat(ufile, &Fstat))) < 0) {
		printf(" File open failed\n");
		return;
	}
	llength = Fstat.st_size;*/
	llength = 200000;
	if ((lfd = (open(ufile, O_RDONLY,"r"))) <= 0) {
		printf(" File open failed\n");
		return;
	}

	nbuffer[0] = 'C';
	nbuffer[1] = 'U';
	nbuffer[2] = 'T';
	nbuffer[3] = 0;
	path = nbuffer;
	dlfd = ERROR;
	if ((dlfd = (open(path, (O_RDWR|O_CREAT),0x1b6))) <= 0) {
		printf("Creat of %s ", path);
		perror("failed\n");
		return;
	}
	printf("dlfd %d ",dlfd);

	length = 0;
	str = nbuffer;
	firstnib = 1;
	while (llength > 0) {
		if ((iread = read(lfd, lbuffer, sizeof(lbuffer))) <= 0) {
			if (iread) {
				printf ("read %s ",ufile);
				perror ("failed ");
			}
			printf("read 0 llength %d ",llength);
			break;
		}
#ifdef DEBUG
		DT("r%d ",iread);
#endif
		jread = iread;
		bp = lbuffer;
		while (iread--) {
			if (firstnib) {
				if (*bp > (u_char)0xef)
					c = *bp++ & 0x0f;
				else
					c = (*bp++ & 0x07) + 9;
				firstnib = 0;
			} else {
				c <<= 4;
				if (*bp > (u_char)0xef)
					c |= *bp++ & 0x0f;
				else
					c |= (*bp++ & 0x07) + 9;
				*str++ = c;
				length++;
				c = 0;
				firstnib = 1;
				length = length % COLS ? length : 0;
				if (!length) {
					inblanks = 0;
				if ((iwrite = write(dlfd,nbuffer, str - nbuffer)) < 0) {
#ifdef DEBUG
					DT("bad write %d ",errno);
#endif
					printf("str %x nbuffer %x * %02x\n",str,nbuffer,*nbuffer);
					perror("write failed ");
					break;
				}
#ifdef DEBUG
					DT("w%d i%d\n",str-nbuffer,iread);
#endif
					*str = 0;
					str = nbuffer;
				}
			}
		}
		llength -= jread;
	}
	if ((iwrite = write(dlfd,nbuffer, str - nbuffer)) < 0) {
#ifdef DEBUG
		DT("bad write %d ",errno);
#endif
		perror("write failed ");
	}
#ifdef DEBUG
	DT("w-%d\n",iwrite);
	DT(" done ");
#endif
	printf(" done ");
	if (dlfd > 0)
		close(dlfd);
}
