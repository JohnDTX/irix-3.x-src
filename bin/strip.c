char _Origin_[] = "System III";

/*
**	strip -- remove symbol table from object files
*/

#include	<a.out.h>
#include	<signal.h>
#include	<stdio.h>
#include	<ar.h>
#include 	<sys/types.h>
#include 	<sys/nami.h>

#define		MISCCONST	512
#define		BSIZE	512
#define		RELOC	struct relocation_info
#define		RSIZE	sizeof(RELOC)
#define BADMAG(x) (x.a_magic!=A_MAGIC1 && x.a_magic!=A_MAGIC2)
#define __SYMDEF "__.SYMDEF"

struct relocation_info *reloc;

struct	exec	head;
struct	ar_hdr	arhd;
struct	nlist	sym;
#ifdef NOTDEF
char	symbol[SYMLENGTH];
struct	lar_hdr {
	char	lar_name[16];
	long	lar_date;
	u_short	lar_uid;
	u_short	lar_gid;
	u_short	lar_mode;
	long	lar_size;
} larbuf;
#endif


char	*tname;
char	*tname2;
char	*mktemp();
char	*malloc();

int	status;

FILE	*tf;

main(argc, argv)
char **argv;
{
	register i;

	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	tname = mktemp("/tmp/sXXXXXX");
	close(creat(tname, 0600));
	tf = fopen(tname, "w+");
	if(tf == NULL) {
		fprintf(stderr, "strip: cannot create temp file\n");
		exit(2);
	}
	for(i=1; i<argc; i++) {
		strip(argv[i]);
		if(status > 1)
			break;
	}
	fclose(tf);
	unlink(tname);
	if (tname2 != (char *) 0) unlink(tname2);
	exit(status?2:0);
}

strip(name)
char *name;
{
	register	FILE	*f;
	int		i;
	long		size;
	char mag[SARMAG];
	char *strtbl;
	long strtblsz,pos;

	f = fopen(name, "r");
	if(f == NULL) {
		fprintf(stderr, "strip: cannot open %s\n", name);
		status = 1;
		goto out;
	}
	fread(mag, 1, sizeof(mag), f);	/* grab magic # */
	if (!strncmp(mag, ARMAG, SARMAG))  {			/* it's an archive */
		FILE	*tf2 = (FILE *) 0;
		RELOC	*rend, *r;
		int	*extind;
		int	rsize;
		long	asize;

		asize = sizeof(mag);
		if (tname2 == (char *) 0) {
			tname2 = mktemp("/tmp/sbXXXXXX");
		}
		close(creat(tname2, 0600));
		tf2 = fopen(tname2, "w+");	/* update mode */
		if(tf2 == NULL) {
			fprintf(stderr, "strip: cannot create second temp\n");
			exit(2);
		}
		fwrite(mag, 1, sizeof(mag), tf);
						/* read archive headers */
		while(fread((char *)&arhd, 1, sizeof(arhd), f)) {
			char	nbuf[MAXPATHLEN+sizeof(arhd.ar_name)+1];
			int	n,c;
			int	loc = 0;
			int	a = 0;		/* 0 on sys3, 1 on 4.2 (sigh) GB */
			char	*s;
			int 	nextext;
			int ar_size;

			if (strncmp(arhd.ar_fmag, ARFMAG, sizeof(arhd.ar_fmag))) {
				fprintf(stderr, "strip: malformed archive at %d.\n",
						ftell(f));
				exit(1);
			}

#ifdef NOTDEF
			strncpy(larbuf.lar_name, name, sizeof(larbuf.lar_name));
			sscanf(arbuf.ar_date, "%ld", &larbuf.lar_date);
			sscanf(arbuf.ar_uid, "%hd", &larbuf.lar_uid);
			sscanf(arbuf.ar_gid, "%hd", &larbuf.lar_gid);
			sscanf(arbuf.ar_mode, "%ho", &larbuf.lar_mode);
#endif
			sscanf(arhd.ar_size, "%ld", &ar_size);
			asize += sizeof(arhd);
			fread((char *)&head, 1, sizeof(head), f);
			strcpy(nbuf, name);
			strcat(nbuf, ":");
			strncat(nbuf,arhd.ar_name,sizeof(arhd.ar_name));
#ifdef DEBUG
			fprintf(stderr,"%s\n",nbuf);
#endif
			if (N_BADMAG(head)
				|| !strncmp(__SYMDEF, arhd.ar_name, strlen(__SYMDEF))) {
						/* not an a.out, continue */
				goto cont;
			}
						/* if a member is already */
						/* stripped, continue	  */
			if((head.a_syms == 0) && (head.a_trsize == 0) &&
			   (head.a_drsize == 0)) {
cont:
				fwrite((char *)&arhd, 1, sizeof(arhd), tf);
				fwrite((char *)&head, 1, sizeof(head), tf);
				size = ar_size - sizeof(head);
				if(ar_size&01) {
					asize++;
					size++;
				}
				copy(nbuf, f, tf, size);
				asize += ar_size;
				continue;
			}
			size = (long) head.a_text + head.a_data;
			rsize = head.a_trsize + head.a_drsize;
						/* copy T, D and read reloc   */
						/* leaving us at start of     */
						/* symbol table		      */
			copy(nbuf, f, tf2, size);
			n = head.a_syms / (sizeof(sym)); /* max sz for syms */
#ifdef DEBUG
			fprintf(stderr,"allocing 0x%x for reloc info renum for 0x%x syms\n",
					n* sizeof(int), n);
			fprintf(stderr,"allocing 0x%x for reloc info\n",rsize);
#endif
			if((extind = (int *)malloc(n * sizeof(int))) == NULL ||
			   (reloc = (RELOC *)malloc(rsize)) == NULL) {
				fprintf(stderr, "strip: no memory!\n");
				status = 2;
				goto out;
			}
			/* read reloc info (text and data) */
			if(fread((char *)reloc, 1, rsize, f) != rsize) {
				fprintf(stderr, "strip:read error on %s\n", nbuf);
				status = 2;
				goto out;
			}
			/* read symbol by symbol */
			for(i = head.a_syms, nextext = 0; i > 0;
				 i -= sizeof(sym), nextext++) {
				fread((char *)&sym, 1, sizeof(sym), f);
#ifdef NOTDEF
				s = symbol;
				while (c = getc(f)) {
					if (c == EOF) {
						fprintf(stderr,
						  "strip:premature EOF on %s\n",
						  nbuf);
						status = 2;
						goto out;
					}
					*s++ = c;
				}
				*s++ = 0;
				c = s - symbol;
#endif
				if((sym.n_type&N_EXT) == 0) { /* got a local */
					loc = 1;
#ifdef DEBUG
					fprintf(stderr,"removing symbol %d\n",nextext);
#endif
					head.a_syms -= sizeof(sym);
					ar_size -= sizeof(sym);
					continue;
				}
				else {
#ifdef DEBUG
					fprintf(stderr,"symbol #0x%x moved to 0x%x\n",
							nextext,a);
#endif
					extind[nextext] = a++;
				}
				fwrite((char *)&sym, 1, sizeof(sym), tf2);
#ifdef NOTDEF
				fwrite((char *)symbol, 1, c, tf2);
#endif
			}
			rend = &reloc[rsize / RSIZE];
			if(loc)
			    for(r = reloc; r < rend; r++) {
				    if(r->r_symbolnum) {
						if (r->r_extern) {
							/* need to renumber the symbol */
#ifdef DEBUG
							fprintf(stderr,
								"reloc info refed sym #0x%x, now 0x%x\n",
								r->r_symbolnum, extind[r->r_symbolnum]);
#endif
					    	r->r_symbolnum = extind[r->r_symbolnum];
						}
#ifdef DEBUG
						else 
							/* symbolnum is really type info.  Leave it */
							fprintf(stderr,
								"reloc info refed local segment type 0x%x\n",
								r->r_symbolnum);
#endif
					}
			    }
			/* get the string table */
			if (ftell(f) &1) {
				fprintf(stderr,"strip: odd offset in %s at string tbl\n",name);
				status = 3;
				goto out;
			}
			pos = ftell(f);
			if (fread(&strtblsz,1,4,f) != 4) {
				fprintf(stderr, "strip: eof hit reading string table size in %s\n",
						name);
				status = 4;
				goto out;
			}


			sprintf(arhd.ar_size,"%7ld", ar_size);
			fwrite((char *)&arhd, 1, sizeof(arhd), tf);
			fwrite((char *)&head, 1, sizeof(head), tf);
			asize += sizeof(head);
			fseek(tf2, 0L, 0);
			copy(nbuf, tf2, tf, size);
			fwrite((char *)reloc, 1, rsize, tf);
			copy(nbuf, tf2, tf, (long)head.a_syms);
			/* go to string table */
			fseek(f,pos,0);	
			/* copy */
			copy(nbuf, f, tf, strtblsz);
			/* archives are even-byted */
			if(ftell(f) & 01)
				c = getc(f);

			/* even-byted archives, output side */
			if (ftell(tf) & 01) {
				putc(c, tf);
				asize++;
			}
			asize += head.a_syms + size + rsize + strtblsz;
			fseek(tf2, 0L, 0);
#ifdef NOTDEF
			free(reloc);
			free(extind);
#endif
		}
		fflush(tf);
		size = asize;
	} else  {
		fseek(f, 0L, 0);
		fread((char *)&head, 1, sizeof(head), f);
		if(N_BADMAG(head)) {
			fprintf(stderr, "strip: %s not in a.out format\n",name);
			status = 1;
			goto out;
		}
		if ((head.a_syms == 0) && (head.a_trsize == 0) && (head.a_drsize ==0)) {
			fprintf(stderr, "strip: %s already stripped\n", name);
			/* GB status = 1; */
			goto out;
		}
		size = (long)head.a_text + head.a_data;
		head.a_syms = 0;
		head.a_trsize = head.a_drsize = 0;

		fwrite((char *)&head, 1, sizeof(head), tf);
		if(copy(name, f, tf, size)) {
			status = 1;
			goto out;
		}
		size += sizeof(head);
	}
	fclose(f);
	f = fopen(name, "w");
	if(f == NULL) {
		fprintf(stderr, "strip: cannot recreate %s\n", name);
		status = 2;
		goto out;
	}
	fseek(tf, 0L, 0);
	if(copy(name, tf, f, size))
		status = 2;
	fseek(tf, 0L, 0);

out:
	if(f)
		fclose(f);
}

copy(name, fr, to, size)
char *name;
FILE *fr, *to;
long size;
{
	register s, n;
	char buf[512];

	while(size != 0) {
		s = 512;
		if(size < 512)
			s = (int)size;
		n = fread(buf, 1, s, fr);
		if(n != s) {
			fprintf(stderr, "strip: unexpected eof in %s\n", name);
			return(1);
		}
		n = fwrite(buf, 1, s, to);
		if(n != s) {
			fprintf(stderr, "strip: write error in %s\n", name);
			return(1);
		}
		size -= s;
	}
	fflush(to);
	return(0);
}
