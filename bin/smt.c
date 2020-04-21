/*
** smt.c	- Copyright (C) Silicon Graphics - Mountain View, CA 94043
**		- Author: chase bailey
**		- Date: March 1984
**		- Any use, copy or alteration is strictly prohibited
**		- and gosh darn awfully -- morally inexcusable
**		- unless authorized by SGI.
**
**	$Author: root $
**	$State: Exp $
**	$Source: /d2/3.7/src/bin/RCS/smt.c,v $
**	$Revision: 1.1 $
**	$Date: 89/03/27 14:51:08 $
*/

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/mtio.h>
#include <fcntl.h>

/* DSD 5217 */
#define DEFTAPE	"/dev/rmt2"		/* No rewind device for Qic-02 */
/* Interphase Storager */
/* #define DEFTAPE "/dev/rmt6"		/* No rewind device */
#define HELP 16				/*
					** Increased Help to 16 to avoid any
					** new conflicts
					*/

#define	equal(s1,s2)	(strcmp(s1, s2) == 0)

struct commands {
	char *c_name;
	int c_code;
	int c_ronly;
} com[] = {
	{ "eof",	MTWEOF,	0 },
	{ "weof",	MTWEOF,	0 },
	{ "fsf",	MTFSF,	1 },
	{ "fsr",	MTFSR,	1 },
	{ "rewind",	MTREW,	1 },
	{ "status",	MTNOP,	1 },
	{ "erase",	MTERASE, 0 },
	{ "retension",	MTRET, 1 },
	{ "help",	HELP,	0 },
	{ 0 }
};

int mtfd;
struct mtop mt_com;
struct mtget mt_status;
char *tape;

#define DEBUG
#ifdef DEBUG
char mtioctl_debug = 1;
#endif

main(argc, argv)
	int argc;
	char **argv;
{
	register char *cp;
	register struct commands *comp;
	char *getenv();

	if (argc > 2 && (equal(argv[1], "-t"))) {
		argc -= 2;
		tape = argv[2];
		argv += 2;
	} else
		if ((tape = getenv("TAPE")) == NULL)
			tape = DEFTAPE;
	if (argc < 2) {
		usage();
		exit(1);
	}
	cp = argv[1];
	for (comp = com; comp->c_name != NULL; comp++)
		if (strncmp(cp, comp->c_name, strlen(cp)) == 0)
			break;
	if (comp->c_name == NULL) {
		usage();
		/*
		** fprintf(stderr, "mt: don't fucking have a command: \"%s\"\n",
		** 	cp);
		*/
		exit(1);
	}
	if(comp->c_code == HELP) {
		usage();
		exit(1);
	}
	if ((mtfd = open(tape, comp->c_ronly ? 0 : 2)) < 0) {
		perror(tape);
		exit(1);
	}
	if (comp->c_code != MTNOP) {
		mt_com.mt_op = comp->c_code;
		mt_com.mt_count = (argc > 2 ? atoi(argv[2]) : 1);
		if (mt_com.mt_count < 0) {
			fprintf(stderr, "mt: negative repeat count\n");
			exit(1);
		}
		if (ioctl(mtfd, MTIOCTOP, &mt_com) < 0) {
			fprintf(stderr, "%s %s %d ", tape, comp->c_name,
				mt_com.mt_count);
			perror("failed");
			exit(2);
		}
	} else {
		if (ioctl(mtfd, MTIOCGET, (char *)&mt_status) < 0) {
			perror("mt");
			exit(2);
		}
		status(&mt_status);
	}
}

usage()
{
	printf("\n smt [-t /dev/tapename] command [count]\n");
	printf("\t           *** commands ***\n");
	printf("\teof        - Write [count] end-of-file marks\n");
	printf("\tfsf        - Space forward [count] file marks\n");
	printf("\tfsr        - Space forward [count] records\n");
	printf("\trewind     - Rewind tape device\n");
	printf("\tstatus     - Read tape status\n");
	printf("\terase      - Erase tape\n");
	printf("\tretension  - Retension tape\n");
	printf("\thelp       - Help printout of this message\n");
}

struct tape_desc {
	short	t_type;		/* type of magtape device */
	char	*t_name;	/* tape drive printing name */
	char	*c_name;	/* controller name */
} tapes[] = {
	{ MT_ISTS,	"Qic-02 Tape Drive",	"DSD 5217"},
	{ MT_ISHT,	"Qic-02 Tape Drive",	"Interphase Storager"},
	{ 0 }
};
/*
** Interpret the status buffer returned
*/
status(bp)
	register struct mtget *bp;
{
	register struct tape_desc *mt;

	for (mt = tapes; mt->t_type; mt++)
		if (mt->t_type == bp->mt_type)
			break;
	if (mt->t_type == 0) {
		printf("unknown tape drive type (%d)\n", bp->mt_type);
		return;
	}
	fprintf(stderr, "\n *** TAPE STATUS ***\n");
	fprintf(stderr, "\tController: %s Drive: %s\n",
		mt->c_name, mt->t_name);
	fprintf(stderr, "\tErrors: hard0(%x), hard1(%x), soft0(%x)\n",
		bp->mt_hard_error0, bp->mt_hard_error1, bp->mt_soft_error0);
	fprintf(stderr, "\tTape Cartridge: %s, %s, ",
	   bp->mt_at_bot?"At BOT":"Not At BOT",
	   (bp->mt_hard_error1&0x80)?"Write Protected":"Not Write Protected");
	fprintf(stderr, "File Number (%d)\n", bp->mt_fileno);
}
