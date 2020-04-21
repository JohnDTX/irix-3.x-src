#ifndef lint
/* @(#)makedbm.c	2.1 86/04/16 NFSSRC */
static  char sccsid[] = "@(#)makedbm.c 1.1 86/02/05  Copyr 1985 Sun Micro";
#endif

/* 
 * Copyright (c) 1985 by Sun Microsystems, Inc.
 */

#include <dbm.h>
#undef NULL
#include <stdio.h>
#ifdef sgi
#include <sys/types.h>
#include <sys/inode.h>
#endif
#include <sys/file.h>
#include <sys/param.h>
#include <sys/stat.h>

#define LASCII_DATE 10
#define MAXLINE 4096		/* max length of input line */
#define MAXHOST 255		/*  max length of host name */
static char *get_date();
static char *any();
#ifndef sgi
FILE *fopen(), *fclose();
#endif

main(argc, argv)
	char **argv;
{
	FILE *outfp, *infp;
	datum key, content, tmp;
	char buf[MAXLINE];
	char pagbuf[MAXPATHLEN];
	char tmppagbuf[MAXPATHLEN];
	char dirbuf[MAXPATHLEN];
	char tmpdirbuf[MAXPATHLEN];
	char *p;	
	char *infile, *outfile;
	char *infilename, *outfilename, *mastername, *domainname;
	char local_host[MAXHOST];
	int cnt;

	infile = outfile = NULL; /* where to get files */
	/* name to imbed in database */
	infilename = outfilename = mastername = domainname = NULL; 
	argv++;
	argc--;
	while (argc > 0) {
		if (argv[0][0] == '-' && argv[0][1]) {
			switch(argv[0][1]) {
				case 'i':
					infilename = argv[1];
					break;
				case 'o':
					outfilename = argv[1];
					break;
				case 'm':
					mastername = argv[1];
					break;
				case 'd':
					domainname = argv[1];
					break;
				case 'u':
					unmake(argv[1]);
					return;
				default:
					usage();
			}
			argv++;
			argc--;
		}
		else if (infile == NULL)
			infile = argv[0];
		else if (outfile == NULL)
			outfile = argv[0];
		else
			usage();
		argv++;
		argc--;
	}
	if (infile == NULL || outfile == NULL)
		usage();
	if (strcmp(infile, "-") != 0)
		infp = fopen(infile, "r");
	else
		infp = stdin;
	if (infp == NULL) {
		fprintf(stderr, "makedbm: can't open %s\n", infile);
		exit(1);
	}
	strcpy(tmppagbuf, outfile);
	strcat(tmppagbuf, ".t");
	strcpy(tmpdirbuf, tmppagbuf);
	strcat(tmpdirbuf, ".dir");
	strcat(tmppagbuf, ".pag");
	if ((outfp = fopen(tmpdirbuf, "w")) == NULL) {
	    	fprintf(stderr, "makedbm: can't create %s\n", tmpdirbuf);
		exit(1);
	}
	if ((outfp = fopen(tmppagbuf, "w")) == NULL) {
	    	fprintf(stderr, "makedbm: can't create %s\n", tmppagbuf);
		exit(1);
	}
	strcpy(dirbuf, outfile);
	strcat(dirbuf, ".t");
	if (dbminit(dirbuf) != 0) {
		fprintf(stderr, "makedbm: can't init %s\n", dirbuf);
		exit(1);
	}
	strcpy(dirbuf, outfile);
	strcpy(pagbuf, outfile);
	strcat(dirbuf, ".dir");
	strcat(pagbuf, ".pag");
	while (fgets(buf, sizeof(buf), infp) != NULL) {
		p = buf;
		cnt = strlen(buf) - 1; /* erase trailing newline */
		while (p[cnt-1] == '\\') {
			p+=cnt-1;
			if (fgets(p, sizeof(buf)-(p-buf), infp) == NULL)
				goto breakout;
			cnt = strlen(p) - 1;
		}
		p = any(buf, " \t\n");
		key.dptr = buf;
		key.dsize = p - buf;
		while (1) {
			if (p == NULL || *p == NULL) {
				fprintf(stderr, "makedbm: yikes!\n");
				exit(1);
			}
			if (*p != ' ' && *p != '\t')
				break;
			p++;
		}
		content.dptr = p;
		content.dsize = strlen(p) - 1; /* erase trailing newline */
		tmp = fetch(key);
		if (tmp.dptr == NULL) {
			if (store(key, content) != 0) {
				printf("problem storing %.*s %.*s\n",
				    key.dsize, key.dptr,
				    content.dsize, content.dptr);
				exit(1);
			}
		}
#ifdef DEBUG
		else {
			printf("duplicate: %.*s %.*s\n",
			    key.dsize, key.dptr,
			    content.dsize, content.dptr);
		}
#endif
	}
   breakout:
	addpair("YP_LAST_MODIFIED", get_date(infile));
	if (infilename)
		addpair("YP_INPUT_FILE", infilename);
	if (outfilename)
		addpair("YP_OUTPUT_NAME", outfilename);
	if (domainname)
		addpair("YP_DOMAIN_NAME", domainname);
	if (!mastername) {
		gethostname(local_host, sizeof (local_host) - 1);
		mastername = local_host;
	}
	addpair("YP_MASTER_NAME", mastername);

	if (rename(tmppagbuf, pagbuf) < 0)
		perror("makedbm: rename");
	if (rename(tmpdirbuf, dirbuf) < 0)
		perror("makedbm: rename");
}


/* 
 * scans cp, looking for a match with any character
 * in match.  Returns pointer to place in cp that matched
 * (or NULL if no match)
 */
static char *
any(cp, match)
	register char *cp;
	char *match;
{
	register char *mp, c;

	while (c = *cp) {
		for (mp = match; *mp; mp++)
			if (*mp == c)
				return (cp);
		cp++;
	}
	return ((char *)0);
}

static char *
get_date(name)
	char *name;
{
	struct stat filestat;
	static char ans[LASCII_DATE];/* ASCII numeric string */

	if (strcmp(name, "-") == 0)
		sprintf(ans, "%010d", time(0));
	else {
		if (stat(name, &filestat) < 0) {
			fprintf(stderr, "makedbm: can't stat %s\n", name);
			exit(1);
		}
		sprintf(ans, "%010d", filestat.st_mtime);
	}
	return ans;
}

usage()
{
	fprintf(stderr,
"usage: makedbm -u file\n       makedbm [-i YP_INPUT_FILE] [-o YP_OUTPUT_FILE] [-d YP_DOMAIN_NAME] [-m YP_MASTER_NAME] infile outfile\n");
	exit(1);
}

addpair(str1, str2)
	char *str1, *str2;
{
	datum key;
	datum content;
	
	key.dptr = str1;
	key.dsize = strlen(str1);
	content.dptr  = str2;
	content.dsize = strlen(str2);
	if (store(key, content) != 0){
		printf("makedbm: problem storing %.*s %.*s\n",
		    key.dsize, key.dptr, content.dsize, content.dptr);
		exit(1);
	}
}

unmake(file)
	char *file;
{
	datum key, content;

	if (file == NULL)
		usage();
	
	if (dbminit(file) != 0) {
		fprintf(stderr, "makedbm: couldn't init %s\n", file);
		exit(1);
	}
	for (key = firstkey(); key.dptr != NULL; key = nextkey(key)) {
		content = fetch(key);
		printf("%.*s %.*s\n", key.dsize, key.dptr,
		    content.dsize, content.dptr);
	}
}
