/*
** 	mt.c	- Copyright (C) Silicon Graphics - Mountain View, CA 94043
**		- Author: chase bailey (hacked by markb)
**		- Date: April 1984
**		- Any use, copy or alteration is strictly prohibited
**		- and gosh darn awfully -- morally inexcusable
**		- unless authorized by SGI.
**
**	$Author: root $
**	$State: Exp $
**	$Source: /d2/3.7/src/bin/RCS/mt.c,v $
**	$Revision: 1.1 $
**	$Date: 89/03/27 14:50:45 $
*/

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/mtio.h>
#include <fcntl.h>

#define DEFTAPE "/dev/rmt2"
#define TMTTAPE	"/dev/rmt4"	/* No rewind device for Tapemaster 1000 */
#define STTTAPE	"/dev/rmt8"	/* No rewind device for Storager Qic-02 */
#define DSDTAPE	"/dev/rmt2"	/* No rewind device for DSD 5217 Qic-02 */

/* Things from DSTATUS command: */
#define	DS_RDY		0x0008		/* Drive Ready */
#define	DS_BOT		0x0010		/* at Beginning Of Tape */
#define DS_EOT		0x0020		/* at End Of Tape */
#define	DS_RWD		0x0040		/* drive is Rewinding */
#define	DS_ONL		0x0080		/* drive is Online */
#define	DS_FBUSY	0x0100		/* Formatter Busy */
#define	DS_DBUSY	0x0200		/* Drive Busy */
#define	DS_DEN		0x0400		/* Density (Interface P2, pin 26) */
#define	DS_SPD		0x0800		/* Speed (Interface P2, pin 40) */
#define	DS_FPT		0x1000		/* Drive is Write Protected */
#define	DS_ID		0x2000		/* Ident (Interface P2, pin 16) */
#define	DS_SGL		0x4000		/* Single head (Interface P2, pin 14) */

#define HELP 16

#define	equal(s1,s2)	(strcmp(s1, s2) == 0)

struct commands {
	char *c_name;
	int c_code;
	int c_ronly;
	int c_qic02;
} com[] = {
	{ "weof",	MTWEOF,		0,	1 },
	{ "eof",	MTWEOF,		0,	1 },
	{ "fsf",	MTFSF,		1,	1 },
	{ "fsr",	MTFSR,		1,	0 },
	{ "bsf",	MTBSF,		1,	0 },
	{ "bsr",	MTBSR,		1,	0 },
	{ "rewind",	MTREW,		1,	1 },
	{ "status",	MTNOP,		1,	1 },
	{ "blksize",	MTBLKSIZE,	1,	1 },
	{ "erase",	MTERASE, 	0,	1 },
	{ "reset",	MTRESET, 	0,	1 },
	{ "retension",	MTRET, 		1,	1 },
	{ "help",	HELP,		0,	0 },
	{ 0 }
};

int mtfd;
int qictape = 1;
struct mtop mt_com;
struct mtget mt_status;
char *tape;

main(argc, argv)
	int argc;
	char **argv;
{
	register char *cp;
	register struct commands *comp = com;
	register struct mtget *bp;
	char *getenv();


	if (argc < 2) {
		usage(comp);
		exit (1);
	}
	if (argc > 2 && (equal(argv[1], "-t"))) {
		if (argc < 4) {
			usage(comp);
			exit(1);
		}
		argc -= 2;
		tape = argv[2];
		argv += 2;
		if (equal(tape, TMTTAPE))
			qictape = 0;
	} else
		if ((tape = getenv("TAPE")) == NULL) {
			tape = DEFTAPE;
		}
	cp = argv[1];
	for (comp = com; comp->c_name != NULL; comp++)
		if (strncmp(cp, comp->c_name, strlen(cp)) == 0)
			break;
	if (comp->c_name == NULL || comp->c_code == HELP) {
		usage(comp);
		exit(1);
	}
	if ((mtfd = open(tape, comp->c_ronly ? 0 : 2)) < 0) {
		perror(tape);
		exit(1);
	}
	if (comp->c_code != MTNOP && comp->c_code != MTBLKSIZE) {
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
	 	mt_status.mt_type = comp->c_code;
		if (ioctl(mtfd, MTIOCGET, (char *)&mt_status) < 0) {
			perror("mt");
			exit(2);
		}
		if (comp->c_code == MTBLKSIZE) {
			bp = &mt_status;
			printf("\n Default Tape Block Size %d\n", bp->mt_blkno);
			return;
		}
		status(&mt_status);
	}
}

usage(comp)
	struct commands *comp;
{
	register struct commands *c = comp;

	printf("\n mt [-t /dev/tapename] command [count]\n");
	printf("\t           *** commands ***\n");
	printf("\teof        - Write [count] end-of-file marks\n");
	printf("\tfsf        - Space forward [count] file marks\n");
	if (!qictape) {
		printf("\tfsr        - Space forward [count] records\n");
		printf("\tbsf        - Space backward [count] file marks\n");
		printf("\tbsr        - Space backward [count] records\n");
	}
	printf("\trewind     - Rewind tape device\n");
	printf("\tstatus     - Read tape status\n");
	if (qictape) {
		printf("\terase      - Erase tape\n");
		printf("\tretension  - Retension tape\n");
	}
	printf("\tblksize    - Return default tape block size\n");
	if (!qictape)
		printf("\treset      - Reset the controller\n");
	printf("\thelp       - Help printout of this message\n");
	exit(1);
}

struct tape_desc {
	short	t_type;		/* type of magtape device */
	char	*t_name;	/* tape drive printing name */
	char	*c_name;	/* controller name */
} tapes[] = {
	{ MT_ISDSD,	"Qic-02 Tape Drive",	"DSD 5217"},
	{ MT_ISTMT,	"Half-Inch Tape Drive",	"Tapemaster 1000"},
	{ MT_ISSTT,	"Qic-02 Tape Drive", "Storager Tape Controller"},
#ifdef NOTDEF
	{ MT_ISMT,	"(not used)", "(not used)"},
	{ MT_ISUT,	"(not used)", "(not used)"},
	{ MT_ISCPC,	"(not used)", "(not used)"},
	{ MT_ISAR,	"(not used)", "(not used)"},
#endif NOTDEF
	{ 0 }
};

/*
** Interpret the status buffer returned
*/
status(bp)
	register struct mtget *bp;
{
	register struct tape_desc *mt;
	register unsigned short status;

	for (mt = tapes; mt->t_type; mt++)
		if (mt->t_type == bp->mt_type)
			break;
	if (mt->t_type == 0) {
		printf("unknown tape drive type (%d)\n", bp->mt_type);
		return;
	}
	if (mt->t_type == MT_ISDSD) {
		fprintf(stderr, "\t*** Qic-02 Tape Status ***\n");
		fprintf(stderr, "\tController: %s\n", mt->c_name);
		fprintf(stderr, "\tDrive     : %s\n", mt->t_name);
		fprintf(stderr, "\tErrors    : byte 0 0x%x, byte 1 0x%x, soft 0x%x\n",
			bp->mt_hard_error0, bp->mt_hard_error1,
			bp->mt_soft_error0);
		fprintf(stderr, "\tCartridge : %s, %s\n",
	   	     bp->mt_at_bot?"at BOT":"not at BOT",
	   	    (bp->mt_hard_error1&0x80)?"write protected":"writable");
		fprintf(stderr, "\tPosition  : file number (%d)\n",
			bp->mt_fileno);
	} else if (mt->t_type == MT_ISTMT) {
		status = (bp->mt_hard_error0|((bp->mt_soft_error0<<8)&0xff00));
		fprintf(stderr, "\t*** Half-Inch Tape Status ***\n");
		fprintf(stderr, "\tController: %s\n", mt->c_name);
		fprintf(stderr, "\tDrive     : %s\n", mt->t_name);
		fprintf(stderr, "\tStatus    : %s, %s, %s, %s\n",
			status&DS_RDY?"ready":"not ready",
			status&DS_ONL?"online":"not online",
			status&DS_BOT?"BOT":"not at BOT",
			status&DS_RWD?"rewinding":"not rewinding");

		fprintf(stderr, "\tStatus    : %s, %s, %s\n",
			status&DS_FBUSY?"busy":"not busy",
		/*	status&DS_DEN?"GCR (6250 bpi)":"PE (1600 bpi)", */
			status&DS_FPT?"write protected":"writable",
			status&DS_DBUSY?"drive busy":"drive not busy");

		fprintf(stderr, "\tPosition  : file no (%d), block no (%d)\n",
			bp->mt_fileno, bp->mt_blkno);
	} else if (mt->t_type == MT_ISSTT) {
		fprintf(stderr, "\t*** QIC-02 Tape Status ***\n");
		fprintf(stderr, "\tController: %s\n", mt->c_name);
		fprintf(stderr, "\tDrive     : %s\n", mt->t_name);
		fprintf(stderr, "\tCartridge : %s, %s\n",
		/*(bp->mt_status & NO_TAPE)?"no cartridge installed":"ready",*/
	   	     bp->mt_at_bot?"at BOT":"not at BOT",
		     (bp->mt_status & NOT_ONLINE)?"not on line":"on line");
		fprintf(stderr, "\tStatus    : %s, ",
	   	     (bp->mt_status & WR_PROT)?
			"write protected":"writable");
		fprintf(stderr, "file number (%d)\n", bp->mt_fileno);
	} else {
		fprintf(stderr, "\tController: %s Drive: %s\n",
			mt->c_name, mt->t_name);
		exit(1);
	}
}


