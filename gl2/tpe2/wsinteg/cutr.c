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
#include <sys/termio.h>
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <gl.h>
#include "rpc.h"
#include "term.h"
#include "hostio.h"
#include "pxw.h"
#define BUFSIZE	2400

extern u_char	Ebc2asc[];

struct stat	Fstat;
FILE		*lod = NULL;
int		gcmdcount = 0;	/* start with switch to graphics */
int		cntu = 0;
int		icol, jcol;
int		inmsg = 0;
int		lfd;
int		m, o;
int		ncount = 0;
int		offset = 0;
u_char		buf[BUFSIZE+4], pbuf[256];	
u_char		cbuf[27];

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
				case 'b':
					type = 'b'; /* bin of EBCDIC text */
					break;
				case 'C':
					type = 'C'; /* compress */
					break;
				case 'c':
					type = 'c'; /* create commlog */
					gcmdcount++;
					break;
				case 'm':
					type = 'm'; /* msg parse */
					gcmdcount++;
					break;
				case 'r':
					type = 'r'; /* RDDATA 3274 */
					break;
				case 'w':
					type = 'w'; /* WRDATA 3274 */
					gcmdcount++;
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
	if ((lfd = (stat(argv[2], &Fstat))) < 0) {
		printf("path bad status %d errno %d\n",lfd,errno);
		exit(0);
	}
	if ((lfd = (open(argv[2], O_RDONLY,"r"))) <= 0) {
		printf("EXIT from cutr with bad open \n");
		exit(0);
	}
	if ((lod = (fopen("CUT","w"))) <= 0) {
		printf("EXIT from cutr with bad open \n");
		exit(0);
	}
	o = 0;
	n = Fstat.st_size;
	if (argc == 4 && (argv[3][0] == '+')) {
		arg1 = (u_char *)&argv[3][1];
		while (*arg1)
			buf[m++] = *arg1++;
		sscanf(buf,"%ld",&offset);
		printf ("offset to message #%d",offset);
	}
	printf("\n%s %x hex ",argv[2],n);
	while (n > 0) {
		n = read(lfd,buf,BUFSIZE);
		ncount += n;
		arg1 = buf;
		if (type != 'c' && type != 'm') {
		    for (i=1; i <= n; i++) {
			    c = *arg1++;
			    if (type == 'r') {
				    if (c == 0x10) { /* RESC */
					    if (++gcmdcount > offset) {
/*						    printf("\0339P");
						    printf("%02x",c);
						    printf("\0330@");*/
						    sprintf(pbuf,"\n#%d 10 ",gcmdcount-1);
						    fprintf(lod,pbuf);
					}
				    } else {
					    if (gcmdcount > offset) {
/*						    printf("%02x",c);*/
						    sprintf(pbuf,"%02x ",c);
						    fprintf(lod,pbuf);
					}
				    }
			    } else if (type == 'w') {
				    if (c == 0x7e) { /* RESC */
					    if (++gcmdcount > offset) {
/*						    printf("\0339P");
						    printf("%02x",c);
						    printf("\0330@");*/
						    sprintf(pbuf,"\n#%d 7e ",gcmdcount-1);
						    fprintf(lod,pbuf);
					}
				    } else {
					    if (gcmdcount > offset) {
/*						    printf("%02x",c);*/
						    sprintf(pbuf,"%02x ",c);
						    fprintf(lod,pbuf);
					}
				    }
			    } else if (type == 'C') {
				    C = Ebc2asc[c];
				    if (C <= '9')
					C -= '0';
				else
					C -= '7';
				c = *arg1++;
				i++;
				c = Ebc2asc[c];
				    if (c <= '9')
					c -= '0';
				else
					c -= '7';
				c |= (C << 4);
				m = fwrite(c,1,BUFSIZE,lod);
/*				sprintf(pbuf,"%02x",c);
				fprintf(lod,pbuf);
				if (!(i % 80))
					fprintf(lod,"\n");*/
			    } else if (type == 'b') {
				c = Ebc2asc[c];
				if (c < ' ' || c == '%')
					c = ' ';
				sprintf(pbuf,"%c",c);
				fprintf(lod,pbuf);
				if (!(i % 80))
					fprintf(lod,"\n");
			    }
		    }
		    fflush(lod);
		} else if (type == 'm') {
			for (i = 0; i < n; i += 24) {
				icol = col = i+24 > n ? n-i : 24;
				jcol = icol + 1;
				while (col) {
					c = *arg1++;
					if (c == (u_char)0xef) { /* RAWINP */
						if (++gcmdcount > offset) {
							if (inmsg++) {
								o = icol - col;
								for (m = icol; o; o--) {
									sprintf(pbuf,"%02x ",cbuf[m--]);
									fprintf(lod,pbuf);
								}
							}
							cbuf[col] = c;
/*				fprintf(lod,"icol%d c%d i%d ",icol,col,i);*/
							fprintf(lod,"\n#%d %x ef ",gcmdcount-1,ncount-n + arg1-buf + 3);
							i += icol - col;
							i -= 23;
							col = 1;
						}
					} else if (gcmdcount > offset &&
						i+(jcol-col) >= n) { /* count */
/*				fprintf(lod,"icol%d c%d i%d ",icol,col,i);*/
						i += jcol - col;
						o = jcol - col;
						cbuf[col] = c;
						for (m = icol; o; o--) {
							sprintf(pbuf,"%02x ",cbuf[m--]);
							fprintf(lod,pbuf);
						}
/*						fprintf(lod,"end ");*/
							col = 1;
					} else if (gcmdcount > offset) {
						if (col != 1)
							cbuf[col] = c;
						else {
							cbuf[col] = c;
sprintf(pbuf,"%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x ",
	    cbuf[24],cbuf[23],cbuf[22],cbuf[21],cbuf[20],cbuf[19],cbuf[18],
	    cbuf[17], cbuf[16],cbuf[15],cbuf[14],cbuf[13]);
							fprintf(lod,pbuf);
sprintf(pbuf,"%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n ",
	    cbuf[12],cbuf[11],cbuf[10],cbuf[9],cbuf[8],cbuf[7],cbuf[6],
	    cbuf[5], cbuf[4],cbuf[3],cbuf[2],cbuf[1]);
							fprintf(lod,pbuf);
						}
					}
				col--;
				}
			}
		    fflush(lod);
		} else if (type == 'c') {
			for (i = 0; i < n; i++) {
				c = *arg1++;
				if (c==(u_char)0xef && !inmsg) {/* count flag */
					if (gcmdcount++ > offset) {
						o = (n - i) - 4; /* data left */
						if (o >= 0) {
							cbuf[0] = 0;
							for (o = 1; o < 4; o++)
								cbuf[o] = *arg1++;
							inmsg = cbuf[1];
							inmsg <<= 8;
							inmsg |= cbuf[2];
							inmsg <<= 8;
							inmsg |= cbuf[3];
							inmsg++; /* leading 0 */
							icol = inmsg;
							cbuf[3] = (u_char)inmsg;
							inmsg >>= 8;
							cbuf[2] = (u_char)inmsg;
							inmsg >>= 8;
							cbuf[1] = (u_char)inmsg;
							for (o=3; o >= 0; o--)
								*--arg1 = cbuf[o];
							arg1 += 4;
							i += 3;
							cntu = 0;
							inmsg = icol - 4;
						} else {
							cbuf[0] = 0;
							for (jcol=1,m=o+3;m ; m--)
								cbuf[jcol++] = *arg1++;
							cntu = jcol;
							inmsg++;
						}
					}
				} else if (cntu && gcmdcount > offset) {
					o = cntu;
					for ( ; o < 4; o++)
						cbuf[o] = *arg1++;
					inmsg = cbuf[1];
					inmsg <<= 8;
					inmsg |= cbuf[2];
					inmsg <<= 8;
					inmsg |= cbuf[3];
					inmsg++; /* leading 0 */
					icol = inmsg;
					cbuf[3] = (u_char)inmsg;
					inmsg >>= 8;
					cbuf[2] = (u_char)inmsg;
					inmsg >>= 8;
					cbuf[1] = (u_char)inmsg;
					m = 3 - cntu;
					for (o=3; m >= 0; m--)
						*--arg1 = cbuf[o--];
					arg1 += 4 - cntu;
					i += 3 - cntu;
					cntu = 0;
					inmsg = icol - 4;
				} else if (inmsg > 0 && gcmdcount > offset) {
					inmsg--;
				} else if (inmsg < 0 && gcmdcount > offset) {
					inmsg = 0;
				}
			}
			if (gcmdcount > offset) {
				m = fwrite(buf,sizeof(*buf),BUFSIZE,lod);
				fflush(lod);
			}
		}
	}
	gcmdcount--;
	fflush(lod);
	fclose(lod);
	printf("%d messages cut from %d\n",gcmdcount-offset,gcmdcount);
}

/*
**	Display usage message and exit program
*/
usage()
{
	(void)printf("\007\nUsage: cutr -[bcmrw] infilename [+offset]\n");
	exit(1);
}
