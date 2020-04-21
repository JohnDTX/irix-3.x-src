#ifndef lint
static	char sccsid[] = "@(#)ranlib.c 4.6 5/22/83";
#endif
/*
 * ranlib - create table of contents for archive; string table version
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <ar.h>
#include <ranlib.h>
#include <a.out.h>
#include <stdio.h>
#include <fcntl.h>

struct	ar_hdr	archdr;
#define	OARMAG 0177545
long	arsize;
struct	exec	exp;
FILE	*fi, *fo;
long	off, oldoff;
long	atol(), ftell();
/* GB 7/24/86 - double TABSZ and STRTABSZ for FORTRAN */
/*#define TABSZ	6000 */
#define TABSZ	90000
struct	ranlib tab[TABSZ];
int	tnum;
/*#define	STRTABSZ	75000 */
#define	STRTABSZ	650000
char	tstrtab[STRTABSZ];
int	tssiz;
char	*strtab;
int	ssiz;
int	new;
char	tempnm[] = "__.SYMDEF";
char	firstname[17];
char	touching;		/* ranlib -t libraries... just fix the dates */

main(argc, argv)
char **argv;
{
	char cmdbuf[BUFSIZ];
	/* magbuf must be an int array so it is aligned on an int-ish
	   boundary, so that we may access its first word as an int! */
	int magbuf[(SARMAG+sizeof(int))/sizeof(int)];

	--argc;
	if(argc && strcmp("-t",argv[1]) == 0) {
	    touching = 1;
	    argv++;
	    argc--;
	}
	while(argc--) {
		fi = fopen(*++argv,"r");
		if (fi == NULL) {
			fprintf(stderr, "ranlib: cannot open %s\n", *argv);
			continue;
		}
		off = SARMAG;
		fread((char *)magbuf, 1, SARMAG, fi);
		if (strncmp((char *)magbuf, ARMAG, SARMAG)) {
			if (magbuf[0] == OARMAG)
				fprintf(stderr, "old format ");
			else
				fprintf(stderr, "not an ");
			fprintf(stderr, "archive: %s\n", *argv);
			fclose(fi);
			continue;
		}
		if(touching) {
		    if(fixdate(*argv)) {
			fclose(fi);
			continue;
		    }
		}
		fseek(fi, 0L, 0);
		new = tnum = 0;
		if (nextel(fi) == 0) {
			fclose(fi);
			continue;
		}
		do {
			long o;
			register n;
			struct nlist sym;

			fread((char *)&exp, 1, sizeof(struct exec), fi);
			/*
			if (N_BADMAG(exp))
			*/
			if (N_BADMAG(exp) || !strcmp(tempnm, archdr.ar_name))
				continue;
			if (exp.a_syms == 0) {
				fprintf(stderr, "ranlib: warning: %s(%s): no symbol table\n", *argv, archdr.ar_name);
				continue;
			}
			o = N_STROFF(exp) - sizeof (struct exec);
			if (ftell(fi)+o+sizeof(ssiz) >= off) {
				fprintf(stderr, "ranlib: %s(%s): old format .o file\n", *argv, archdr.ar_name);
				exit(1);
			}
			fseek(fi, o, 1);
			fread((char *)&ssiz, 1, sizeof (ssiz), fi);
			if (ssiz < sizeof ssiz){
				/* sanity check */
				fprintf(stderr, "ranlib: %s(%s): mangled string table\n", *argv, archdr.ar_name);
				exit(1);
			}
			strtab = (char *)calloc(1, ssiz);
			if (strtab == 0) {
				fprintf(stderr, "ranlib: ran out of memory\n");
				exit(1);
			}
			fread(strtab+sizeof(ssiz), ssiz - sizeof(ssiz), 1, fi);
			fseek(fi, -(exp.a_syms+ssiz), 1);
			n = exp.a_syms / sizeof(struct nlist);
			while (--n >= 0) {
				fread((char *)&sym, 1, sizeof(sym), fi);
				if (sym.n_un.n_strx == 0)
					continue;
				sym.n_un.n_name = strtab + sym.n_un.n_strx;
				if ((sym.n_type&N_EXT)==0)
					continue;
				switch (sym.n_type&N_TYPE) {

				case N_UNDF:
					if (sym.n_value!=0)
						stash(&sym);
					continue;

				default:
					stash(&sym);
					continue;
				}
			}
		} while(nextel(fi));
		new = fixsize();
		fclose(fi);
		fo = fopen(tempnm, "w");
		if(fo == NULL) {
			fprintf(stderr, "can't create temporary\n");
			exit(1);
		}
		tnum *= sizeof (struct ranlib);
		fwrite(&tnum, 1, sizeof (tnum), fo);
		tnum /= sizeof (struct ranlib);
		fwrite((char *)tab, tnum, sizeof(struct ranlib), fo);
		fwrite(&tssiz, 1, sizeof (tssiz), fo);
		fwrite(tstrtab, tssiz, 1, fo);
		fclose(fo);
		if(new)
			sprintf(cmdbuf, "ar rlb %s %s %s\n", firstname, *argv, tempnm);
		else
			sprintf(cmdbuf, "ar rl %s %s\n", *argv, tempnm);
		if(system(cmdbuf))
			fprintf(stderr, "ranlib: ``%s'' failed\n", cmdbuf);
		else
			fixdate(*argv);
		unlink(tempnm);
	}
	exit(0);
}

nextel(af)
FILE *af;
{
	register r;
	register char *cp;

	oldoff = off;
	fseek(af, off, 0);
	r = fread((char *)&archdr, 1, sizeof(struct ar_hdr), af);
	if (r != sizeof(struct ar_hdr))
		return(0);
	for (cp=archdr.ar_name; cp < & archdr.ar_name[sizeof(archdr.ar_name)]; cp++)
		if (*cp == ' ')
			*cp = '\0';
	arsize = atol(archdr.ar_size);
	if (arsize & 1)
		arsize++;
	off = ftell(af) + arsize;
	return(1);
}

stash(s)
	struct nlist *s;
{
	int i;
	register char *cp;

	if(tnum >= TABSZ) {
		fprintf(stderr, "ranlib: symbol table overflow\n");
		exit(1);
	}
	tab[tnum].ran_un.ran_strx = tssiz;
	tab[tnum].ran_off = oldoff;
	for (cp = s->n_un.n_name; tstrtab[tssiz++] = *cp++;)
		if (tssiz > STRTABSZ) {
			fprintf(stderr, "ranlib: string table overflow\n");
			exit(1);
		}
	tnum++;
}

fixsize()
{
	int i;
	off_t offdelta;

	if (tssiz&1)
		tssiz++;
	offdelta = sizeof(archdr) + sizeof (tnum) + tnum * sizeof(struct ranlib) +
	    sizeof (tssiz) + tssiz;
	off = SARMAG;
	nextel(fi);
	if(strncmp(archdr.ar_name, tempnm, sizeof (archdr.ar_name)) == 0) {
		new = 0;
		offdelta -= sizeof(archdr) + arsize;
	} else {
		new = 1;
		strncpy(firstname, archdr.ar_name, sizeof(archdr.ar_name));
	}
	for(i=0; i<tnum; i++)
		tab[i].ran_off += offdelta;
	return(new);
}

/* patch time */
fixdate(s)
	char *s;
{
	long time();
	char buf[24];
	struct	ar_hdr	tmparchdr;
	int fd;
	long tm;
	struct stat stat;

	fd = open(s, O_RDWR);
	if(fd < 0) {
		fprintf(stderr, "ranlib: can't reopen %s\n", s);
		return;
	}
	
	lseek(fd, (long)SARMAG, 0);
	read(fd, &tmparchdr, sizeof(tmparchdr));
	if(strncmp(tmparchdr.ar_name, tempnm, sizeof (tempnm)-1) ==0) {
	    /*
	     * NFS time skew fix
	     *
	     * The modify time is set by the file system on the server,
	     * but the time in the header is set from the clock on the
	     * client side.  The server's clock may be ahead, so use the
	     * later of the system time on the client and the file modify
	     * time as the time in the ranlib header.
	     */
	    if (fstat(fd, &stat) < 0) {
		fprintf(stderr, "ranlib: can't stat %s\n", s);
		return;
	    }
	    tm = time((long *)NULL);
	    tm = (tm > stat.st_mtime) ? tm : stat.st_mtime;

	    sprintf(buf, "%-*ld", sizeof(tmparchdr.ar_date), tm + 5);
	    lseek(fd, (long)SARMAG + ((char *)tmparchdr.ar_date - (char *)&tmparchdr), 0);
	    write(fd, buf, sizeof(tmparchdr.ar_date));
	} else {
	    close(fd);
	    return 0;		/* never before ranlibed */
	}
	close(fd);
	return 1;
}
