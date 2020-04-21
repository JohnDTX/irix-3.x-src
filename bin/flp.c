/*
** 	flop.c  - Copyright (C) Silicon Graphics - Mountain View, CA 94043
**		- Author: chase bailey
**		- Date: May 1984
**		- Any use, copy or alteration is strictly prohibited
**		- and gosh darn awfully -- morally inexcusable
**		- unless authorized by SGI.
*/

static	char	*Sccsid = "@(#)$Header: /d2/3.7/src/bin/RCS/flp.c,v 1.1 89/03/27 14:50:30 root Exp $";
/*
 * $Log:	flp.c,v $
 * Revision 1.1  89/03/27  14:50:30  root
 * Initial check-in for 3.7
 * 
 * Revision 1.3  85/12/04  12:56:07  chase
 * Changed the default of flp to be the /dev/rfloppy device.
 * chase.
 * 
 * Revision 1.2  85/06/17  12:11:29  bob
 * Chase changed to use device /dev/rmf0a. I changed help message.
 * 
 * Revision 1.1  85/05/10  17:00:53  bob
 * Initial revision
 * 
 */

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/flpio.h>
#include <fcntl.h>

#define DEFFLOP "/dev/rfloppy"

#define HELP 8

#define	equal(s1,s2)	(strcmp(s1, s2) == 0)

struct commands {
	char *c_name;
	int c_code;
} com[] = {
	{ "format",	FLP_FORMAT},
	{ "help",	FLP_HELP},
	{ 0 }
};

int flpfd;
struct flpop flp_com;
char *floppy;

main(argc, argv)
	int argc;
	char **argv;
{
	register char *cp;
	register struct commands *comp;
	char *getenv();

	if (argc > 2 && (equal(argv[1], "-t"))) {
		argc -= 2;
		floppy = argv[2];
		argv += 2;
	} else
		if ((floppy = getenv("FLOPPY")) == NULL)
			floppy = DEFFLOP;
	if (argc < 2) usage();
	cp = argv[1];
	for (comp = com; comp->c_name != NULL; comp++)
		if (strncmp(cp, comp->c_name, strlen(cp)) == 0)
			break;
	if (comp->c_name == NULL) {
		usage();
		exit(1);
	}
	if(comp->c_code == HELP) {
		usage();
		exit(1);
	}
	if ((flpfd = open(floppy, O_RDWR)) < 0) {
		perror(floppy);
		exit(1);
	}
	if (comp->c_code == FLP_FORMAT) {
		flp_com.flp_op = comp->c_code;
		flp_com.flp_count = 0;
		if (ioctl(flpfd, FLP_IOCTOP, &flp_com) < 0) {
			fprintf(stderr, "%s %s ", floppy, comp->c_name);
			perror("failed");
			exit(2);
		}
	}
}

usage()
{
	printf("\n\t*** flp [-t dev] command\n");
	printf("\t*** commands ***\n\n");
	printf("\tformat     - Format the Floppy Drive's Diskette\n");
	printf("\thelp       - Help -- printout of this message\n");
	printf("\n");
	exit(1);
}
