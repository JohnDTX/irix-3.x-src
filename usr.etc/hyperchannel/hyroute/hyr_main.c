/*
 * $Author: root $
 * $Date: 89/03/27 18:32:26 $
 * $Header: /d2/3.7/src/usr.etc/hyperchannel/hyroute/RCS/hyr_main.c,v 1.1 89/03/27 18:32:26 root Exp $
 * $Log:	hyr_main.c,v $
 * Revision 1.1  89/03/27  18:32:26  root
 * Initial check-in for 3.7
 * 
 * Revision 1.1  87/10/12  13:30:47  ciemo
 * Initial revision
 * 
 * $Revision: 1.1 $
 * $Source: /d2/3.7/src/usr.etc/hyperchannel/hyroute/RCS/hyr_main.c,v $
 * $State: Exp $
 */
#ifndef lint
static char sccsid[] = "@@(#)hyr_main.c PAG - 12-MAR-86 4.2 & S5";
static char bsccsid[] = "@@(#)hyr_main.c	4.2 (Berkeley) 7/8/83";
static char origsccsid[] = "@@(#)hyr_main.c	2.1 Hyperchannel Routing Daemon 82/11/29";
#endif

#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <net/soioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#ifdef	SYSTEM5
#define compare_gates	comp_gates
#define	compare_table	comp_table
#include <multibusif/if_hy.h>
#include <multibusif/hyroute.h>
#else
#include <vaxif/if_hy.h>
#endif	SYSTEM5
#include <netinet/in.h>
#include <ctype.h>
#define MAIN
#include "hyr_sym.h"

int cflag = 0;
struct hy_route hy_route;
struct hy_route ker_hy_route;
struct hyrsetget hysg;
int sflag = 0;
int pflag = 0;
int dflag = 0;
int lexdebug;
int lex_error;
int maxgate = 0;
char *progname = "hyroute";
char *interface;

/*
 * hash a hyperchannel address into the table
 * return NULL if table is full or entry not found and adding a new one
 */
struct hyr_hash *
rhash(key, new, r)
	unsigned key;
	int new;
	register struct hy_route *r;
{
	register struct hyr_hash *rh = &r->hyr_hash[HYRHASH(key)];
	int n = 0;

	while (rh->hyr_key != key) {
		if ((rh->hyr_flags & HYR_INUSE) == 0)
			return(new ? rh : NULL);
		if (++n > HYRSIZE) {	    /* dab */
			return(NULL);
		} else {
			if (++rh >= &r->hyr_hash[HYRSIZE]) /* dab */
				rh = &r->hyr_hash[0];
		}
	}
	return(rh);
}

/*
 * add a direct entry to the hash table using the specified key,
 * destination, control and access fields, return 0 if successful
 */
int
add_direct(key, dst, ctl, access, r)
	unsigned key;
	unsigned dst;
	unsigned ctl;
	unsigned access;
	register struct hy_route *r;
{
	register struct hyr_hash *kh = rhash(key, 1, r);
  	if (kh == NULL) {
  		printf("Hash table full.  ");
  		return(1);
  	}
	if ((kh->hyr_flags & HYR_INUSE) == 0) {
		kh->hyr_flags = (HYR_INUSE | HYR_DIR);
		kh->hyr_key = key;
		kh->hyr_dst = dst;
		kh->hyr_ctl = ctl;
		kh->hyr_access = access;
		return(0);
	}
  	printf("Duplicate key.  ");
	return(1);
}

/*
 * compare function for the qsort in add_gates, see below
 */
int
compare_gates(a, b)
	unsigned *a, *b;
{
	if (*a < *b)
		return(-1);
	else if (*a > *b)
		return(1);
	else
		return(0);
}

/*
 * add a gatewayed entry to the hash table using the sicified array of
 * gateway keys.  reuse space so as to make the gateway table small as
 * possible.  return 0 if successful
 */
int
add_gates(key, numgates, gates, r)
	unsigned key;
	unsigned numgates;
	unsigned gates[256];
	register struct hy_route *r;
{
	register struct hyr_hash *kh = rhash(key, 1, r);
	register struct hyr_hash *rh;
	int i, j;

	if (kh == NULL)
		return(1);
	for (i = 0; i < numgates; i++) {
		rh = rhash(gates[i], 1, r);
		if (rh == NULL)
			return(1);
		gates[i] = rh - &r->hyr_hash[0];
	}
	qsort(gates, numgates, sizeof(unsigned), compare_gates);
	/*
	 * loop through all existing hash table entries to find one that
	 * matches the currently requested list
	 */
	for (rh = &r->hyr_hash[0]; rh < &r->hyr_hash[HYRSIZE]; rh++) {
		if (rh->hyr_flags & HYR_GATE) {
			if ((rh->hyr_egate - rh->hyr_pgate + 1) == numgates) {
				for (i = 0, j = rh->hyr_pgate; i < numgates ; i++, j++) {
					if (gates[i] != r->hyr_gateway[j])
						goto skipit;
				}
				/*
				 * found a match, just use it
				 */
				kh->hyr_flags = (HYR_INUSE | HYR_GATE);
				kh->hyr_key = key;
				kh->hyr_pgate = rh->hyr_pgate;
				kh->hyr_egate = rh->hyr_egate;
				kh->hyr_nextgate = rh->hyr_nextgate;
				return(0);
			}
		}
	skipit:
		;
	}
	/*
	 * didn't find anything, if there is room add a new entry
	 */
	if (numgates + maxgate > 256)
		return(1);
	kh->hyr_flags = (HYR_INUSE | HYR_GATE);
	kh->hyr_key = key;
	kh->hyr_pgate = maxgate;
	kh->hyr_egate = maxgate + numgates - 1;
	kh->hyr_nextgate = maxgate;
	for (i = 0; i < numgates; i++, maxgate++)
		r->hyr_gateway[maxgate] = gates[i];
	return(0);
}
/*
 * set the kernel table
 */
settable(r)
	struct hy_route *r;
{
	int fd;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket in settable");
		exit(1);
	}
	strcpy(hysg.hyrsg_name, interface, sizeof(hysg.hyrsg_name));
	hysg.hyrsg_ptr = r;
	hysg.hyrsg_len = sizeof(*r);
	if (ioctl(fd, HYSETROUTE, (char *)&hysg) < 0) {
		perror("ioctl HYSETROUTE in settable");
		exit(1);
	}
	if (close(fd) < 0) {
		perror("socket close in settable");
		exit(1);
	}
}

/*
 * get the kernel table
 */
gettable(r)
	struct hy_route *r;
{
	int fd;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket in gettable");
		exit(1);
	}
	strcpy(hysg.hyrsg_name, interface, sizeof(hysg.hyrsg_name));
	hysg.hyrsg_ptr = r;
	hysg.hyrsg_len = sizeof(*r);
	if (ioctl(fd, HYGETROUTE, (char *)&hysg) < 0) {
		perror("ioctl HYGETROUTE in gettable");
		exit(1);
	}
	if (close(fd) < 0) {
		perror("socket close in gettable");
		exit(1);
	}
}


/*
 * print a somewhat readable version of the routine table
 * that the kernel uses (mostly for debugging)
 */
print_table(r)
	register struct hy_route *r;
{
	register struct hyr_hash *rh;
	register int i;
	extern char *ctime();

	if (r->hyr_lasttime)
		printf("table set time: %s", ctime(&r->hyr_lasttime));
	else
		printf("time not set\n");

	for (i = 0; i < HYRSIZE; i++) {
		rh = &r->hyr_hash[i];
		if (rh->hyr_flags & HYR_INUSE) {
			printf("hash %d key %04x flags %x", i, rh->hyr_key, rh->hyr_flags);
			if (rh->hyr_flags & HYR_DIR)
				printf("\tdst %04x ctl %04x access %04x\n",
					ntohs(rh->hyr_dst),
					ntohs(rh->hyr_ctl),
					ntohs(rh->hyr_access));
			else if (rh->hyr_flags & HYR_GATE)
				printf("\tpgate %d egate %d nextgate %d\n",
					rh->hyr_pgate,
					rh->hyr_egate,
					rh->hyr_nextgate);
		}
	}

	for (i = 0; i < 256; i++) { 
		printf("gate[%d] = %d\n", i, r->hyr_gateway[i]);
		if (r->hyr_gateway[i] == 0 && r->hyr_gateway[i+1] == 0)
			break;
	}
}

/*
 * comnpare teo routing tables tom insure that they are the same
 */
compare_table(r1, r2)
	register struct hy_route *r1, *r2;
{
	register struct hyr_hash *rh1, *rh2;
	register int i;
	int ndiffs = 0;

	for (i = 0; i < HYRSIZE; i++) {
		rh1 = &r1->hyr_hash[i];
		rh2 = &r2->hyr_hash[i];
		if (rh1->hyr_flags != rh2->hyr_flags) {
			fprintf(stderr, "%s: hash entry %d - flags differ (%x vs %x)\n", progname, i, rh1->hyr_flags, rh2->hyr_flags);
			ndiffs++;
		} else if ((rh1->hyr_flags & HYR_INUSE) && (rh1->hyr_flags & HYR_DIR)) {
			if (rh1->hyr_dst != rh2->hyr_dst ||
			    rh1->hyr_ctl != rh2->hyr_ctl ||
			    rh1->hyr_access != rh2->hyr_access) {
				fprintf(stderr, "%s: direct hash entry %d - fields differ\n", progname, i);
				fprintf(stderr, "\t(t,k)dst: %04x vs %04x\tctl: %04x vs %04x\taccess: %04x vs %04x\n",
					ntohs(rh1->hyr_dst), ntohs(rh2->hyr_dst),
					ntohs(rh1->hyr_ctl), ntohs(rh2->hyr_ctl),
					ntohs(rh1->hyr_access), ntohs(rh2->hyr_access));
				ndiffs++;
			}
		} else if ((rh1->hyr_flags & HYR_INUSE) && (rh1->hyr_flags & HYR_GATE)) {
			if (rh1->hyr_pgate != rh2->hyr_pgate ||
			    rh1->hyr_egate != rh2->hyr_egate ||
			    rh1->hyr_nextgate < rh1->hyr_pgate ||
			    rh1->hyr_nextgate > rh1->hyr_egate ||
			    rh2->hyr_nextgate < rh2->hyr_pgate ||
			    rh2->hyr_nextgate > rh2->hyr_egate) {
				fprintf(stderr, "%s: gate hash entry %d - fields differ\n", progname, i);
				fprintf(stderr, "\t(t,k)pgate: %04x vs %04x\tegate: %04x vs %04x\tnextgate: %04x vs %04x\n",
					rh1->hyr_pgate, rh2->hyr_pgate,
					rh1->hyr_egate, rh2->hyr_egate,
					rh1->hyr_nextgate, rh2->hyr_nextgate);
				ndiffs++;
			}
		}
	}
	for (i = 0; i < 256; i++) {
		if (r1->hyr_gateway[i] != r2->hyr_gateway[i]) {
			fprintf(stderr, "%s: gate[%d] = %d v2 %d\n", progname, i,
				r1->hyr_gateway[i], r2->hyr_gateway[i]);
		}
	}
	return(ndiffs);
}

main(argc, argv)
	int argc;
	char *argv[];
{
	char *filename = NULL;		/* input file name (default stdin) */
	char *cp;

	if (argc)
		progname = argv[0];
	else
		progname = "hyroute";

	argc--; argv++;
	if (argc < 1)
		usage();
	interface = *argv;
	argc--; argv++;
	while (argc) {
		if (argv[0][0] == '-' && argv[0][1] != '\0') {
			cp = &argv[0][0];
			switch(*++cp) {

			case 's':		/* set the kernel table */
				sflag++;	
				break;

			case 'd':		/* dump the kernel table */
				dflag++;
				break;

			case 'p':		/* print symbol table */
				pflag++;
				break;

			case 'c':		/* compare with kernel table */
				cflag++;
				break;

			case 'l':		/* check the parser */
				lexdebug++;
				break;

			default:
				fprintf(stderr, "%s: unrecognized switch -%c\n", progname, *cp);
				usage();
			}
		} else if (filename == NULL) {
			filename = argv[0];
		} else {
			fprintf(stderr, "%s: extra arguments starting with %s\n", progname, argv[0]);
			usage();
		}
		argc--; argv++;
	}

	if (filename != NULL || sflag || cflag)
		readin(filename, &hy_route);
	else if (!pflag && !dflag)
		usage();

	if (pflag)
		symtab_print();

	if (sflag)
		settable(&hy_route);

	if (dflag || cflag)
		gettable(&ker_hy_route);

	if (dflag)
		print_table(filename == NULL ? &ker_hy_route : &hy_route);

	if (cflag)
		compare_table(&hy_route, &ker_hy_route);
}

usage()
{
	fprintf(stderr, "usage: hyroute interface [ -s ] [ -p ] [ -c ] [ -d ] [ file ]\n");
	exit(1);
}

/*
 * read in the control file named filename into structure r
 */
readin(filename, r)
	char *filename;
	register struct hy_route *r;
{
	register char *cp;
	register struct sym *s;
	unsigned gates[256];
	char buf[512];
	unsigned i;
	extern FILE *yyin;

	if (filename == NULL || *filename == '\0' || strcmp(filename, "-") == 0) {
		yyin = stdin;
	} else {
		yyin = fopen(filename, "r");
		if (yyin == NULL) {
			perror(progname);
			exit(1);
		}
	}

	maxgate = 0;
#ifndef	CRAY
	bzero((char *)r, sizeof(*r));
#else	CRAY
	bzero((char *)r, sizeof(*r)/8);
#endif	CRAY

	lex_error = 0;
	yylex();
	if (lex_error) {
		fprintf(stderr, "hyroute: syntax errors, aborting operation\n");
		exit(1);
	}

	for (s = sym_head; s != NULL; s = s->s_next) {
		if (s->s_flags & HS_DIR) {
		    if (add_direct(inet_lnaof(s->s_fulladdr), s->s_dst, s->s_ctl, s->s_access, r))
				printf("Host %s: not added\n", s->s_name);
		} else if (s->s_flags & HS_INDIR) {
			for (i = 0; i < s->s_ngate; i++)
				gates[i] = inet_lnaof(s->s_gate[i]->s_fulladdr);
			add_gates(inet_lnaof(s->s_fulladdr), s->s_ngate, gates, r);
		}
	}
}
