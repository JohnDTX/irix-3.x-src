/*	@(#)trenter.c	1.3	*/
static	char	*Sccsid = "@(#)$Header: /d2/3.7/src/usr.bin/trenter/RCS/trenter.c,v 1.1 89/03/27 18:29:51 root Exp $";
/*
 * $Log:	trenter.c,v $
 * Revision 1.1  89/03/27  18:29:51  root
 * Initial check-in for 3.7
 * 
 * Revision 1.2  85/02/06  19:11:27  bob
 * Customized for SGI (untested)
 * 
 */
#include	"Param.h"
#include	"Object.h"
#include	<time.h>
#include	<grp.h>
#include	<setjmp.h>
#include	<signal.h>

/*
**	File Names
*/

#define	SPOOLDIR	"/usr/spool/trenter"
#define	TRSAVEF		"/usr/spool/trenter/Trarch"
#define	TRTMPL		"/usr/spool/trenter/Trtmpl"

#define	MOTD		"/usr/spool/trenter/Motd"
#define	TRDEF		".trdef"

				/* Destination directory on receiving machine */
#define	S_MDIR		"/usr/spool/trenter/receive"

/*
**	Various strings/messages
*/

#define	TR		"TrobRpt"
#define	REPRMSG		"Would you like a reprint?"
#define	ANMSG		"Do you wish to enter another report?"
#define	U_SEV1MSG	"See your administrator for severity 1 handling!"
#define	A_SEV1MSG	"Contact Customer Support to report severity 1!"
#define	U_ATTMSG	"Please send attachments to your administrator."
#define	A_ATTMSG	"Please send attachments to Customer Support."

#define	H_ADMOPT	\
"Options:\n\
	  d  delete TRs\n\
	  e  edit TRs\n\
	  l  list known TR numbers\n\
	  n  enter TRs (does not imply send)\n\
	  p  print TRs\n\
	  q  exit trenter\n\
	  s  send TRs to Customer Support\n\
	  ?  this message"

#define	ADMGRP		"adm"	/* The group receiving adm priviledge */
#define	SCHAR		'$'	/* shorthand char for TR#s in adm cmds */
#define	TRPCOL		3	/* # of TRs per column for adm 'l' cmd */

/*
**	The following are links to Objlib
*/

/*
**	Definition of AdmOpt and TrobRpt.  If TMPFL is true, the def of
**	TrobRpt will come from the file TRTMPL.  Otherwise, it is compiled
**	in from Trtmpl.h.
*/

OBJDEFS

#ifndef		TMPLF
#include	"Trtmpl.h"
#endif

DOBJECT(AdmOpt)

"REQUEST",  IN,	"Request",   REQ,  HELP,  H_ADMOPT

ENDOB

char	*Edit = "/bin/ed";	/* Default editor */

extern	int	tverify();	/* Definitions of action routines and table */
extern	int	postmsg();

Acttab	Oftab[] = {
	"tverify",	tverify,
	"postmsg",	postmsg,
	NULL
};				/* End of Objlib stuff */

char	sflg;
char	rdt;
char	Admf;
char	*Cdate;
char	*Home;
char	*Locnum;
char	*Logn;
char	*Mach;
char	*Node;
char	*get_lnum();
OBJECT	Trobj;
OBJECT	Prevobj;
OBJECT	lastn;
OBJTYPE	Tytr;
OBJECTQ	aoptp();
struct	tm	*Timep;
struct	tm	*localtime();
struct	group	*getgrgid();
jmp_buf	jbuf;
jmp_buf	ajbuf;

#define	trenter	main
trenter(argc, argv)
char	**argv;
{
	reg	int	i;
	auto	long	t;
	struct	group	*grp;
	extern	int	optind;
	extern	char	*optarg;

	Home = getenv("HOME");
	grp = getgrgid(getgid());
	if(str_equ(grp->gr_name, ADMGRP))
		Admf++;
#ifdef	TMPLF
	o_init(TRTMPL, 0);
#else
	o_init(NULL, 0);
#endif
	Tytr = otlook(TR);
	while((i = getopt(argc, argv, "s")) != EOF)
		switch(i) {
		case 's':	/* skip filled fields during prompts */
			sflg++;
			break;

		default:
			usage();
			exit(2);
		}
	time(&t);
	Timep = localtime(&t);
	Cdate = get_some(char, 12);
	sprintf(Cdate, "%d/%d/%d", Timep->tm_mon + 1, Timep->tm_mday,
								Timep->tm_year);
	Logn = cuserid(NULL);
	if(!Logn)
		Logn = "???";
	Node = rdcmd("uname -n", NULL);
	if(!Node)
		Node = "???";
	printf("\n\t\tTrouble Report Entry System\n\n");
	if(Admf) {
		auto	OBJTYPE	aot;
		auto	OBJECT	aop = NULL;
		auto	char	**wv = NULL;

		aot = otlook("AdmOpt");
		if(!aot)
			error("cannot find Adm option template.");
		printf("\t\t\tAdministration\n\n");
		signal(SIGINT, SIG_IGN);
		otread(Tytr, NULL);
		otlock(Tytr);
		while(1) {
			auto	char	c;
			auto	char	*p;
			auto	OBJECT	tobj;
			auto	OBJECTQ	tq;
			extern	_aintr();

			signal(SIGINT, _aintr);
			if(setjmp(ajbuf))
				continue;
			if(!aop) {
				aop = ocreat(aot, "__aop__");
				if(!aop)
					error("cannot create adm-opt object");
			}
			ofill(aop);
			if(wv)
				q_fre(wv);
			p = (char *) ogetv(aop, "REQUEST");
			if(!p)
				goto out;
			wv = word_vec(p);
			c = wv[0][0];
			osetv(aop, "REQUEST", NULL);
			switch(c)  {
			case 'd':
			case 'e':
			case 'p':
			case 's':
			{
				if(!wv[1]) {
					q_fre(wv);
					wv = NULL;
					if(c == 'd' || c == 's') {
						printf("%s requires an argument.\n", c=='d' ? "delete" : "send");
						continue;
					}
				}
				tq = aoptp(wv);
				if(!tq) {
					printf("TR(s) not found.\n");
					continue;
				}
				if(c == 's')
					trsend(tq);
				else
				for(i = 0; tobj = tq[i]; i++) {
					lastn = tobj;
					switch(c) {
					case 'd':
						orem(tobj);
						break;

					case 'e':
						printf("%s:\n",
							  ogetv(tobj,"_lnum_"));
						ofill(tobj, 0);
						fixdesc(tobj);
			/*
						otwrite(Tytr, tmpf);
			*/
						break;

					case 'p':
						putchar('\n');
						trprt(tobj);
						putchar('\n');
						break;
					}
				}
				q_free(tq);
				break;
			}

			case 'l':
				tq = otqlook(Tytr);
				if(tq) {
					for(i = 0; tobj = tq[i]; i++) {
						printf("%s (%c%d) ", ogetv(tobj,
							"_lnum_"), SCHAR, i);
						if((i + 1) % TRPCOL == 0)
							putchar('\n');
						lastn = tobj;
					}
					putchar('\n');
				}
				break;

			case 'n':
				otunlock(Tytr);
				(void) trent();
				otlock(Tytr);
				lastn = Trobj;
				break;

			case 'q':
out:
				otunlock(Tytr);
				otwrite(Tytr, NULL);
				exit(0);
				/* NOTREACHED */

			default:
				printf("Unknown request (? for help).\n");
				break;
			}
		}
	} else {
		prmotd();
		if(!sflg) {
			printf("Instructions? ");
			if(getyn()) {
				printf("\n%s\n\n", Ohelp);
				printf("Field names are enclosed in brackets in prompts below (e.g., [NAME]).\n\n");
			}
		}
	}
	while(1) {
		if(trent()) {
			printf("\n%s ", REPRMSG);
			if(getyn())
				trprt(Trobj);
		}
		printf("\n%s ", ANMSG);
		if(!getyn())
			break;
	}
}

_aintr()
{
	putchar('\n');
	longjmp(ajbuf, 1);
}

OBJECTQ
aoptp(wv)
char	**wv;
{
	reg	int	i = 0;
	reg	char	*s;
	reg	OBJECT	tobj = NULL;
	auto	OBJECTQ	rq = NULL;
	auto	OBJECTQ	tq = otqlook(Tytr);

	if(!tq)
		return(NULL);
	if(!wv) {
		if(!lastn)
			return(NULL);
		q_a(&rq, lastn);
	} else while(s = wv[++i]) {
		if(wv[i][0] == SCHAR) {	/* a TR# escape? */
			reg	char	*p = &wv[i][1];

			if(*p) {
				switch(*p) {
				case '*':		/* All TRs -- easy! */
					q_cat(&rq, tq);
					return(rq);
					/* NOTREACHED */

				default:	/* A number (a q_ind) */
				{
					int	ind;

					if(!isdigit(*p))
						continue;
					ind = atoi(p);
					if(ind > q_len(tq) || !tq[ind])
						continue;
					tobj = tq[ind];
				}}
			} else {		/* last # referenced */
				if(!lastn)
					continue;
				tobj = lastn;
			}
		} else {
			tobj = olook(Tytr, s);
			if(!tobj) {
				printf("%s: not found.\n", s);
				continue;
			}
		}
		q_a(&rq, tobj);
		lastn = tobj;
	}
	return(rq);
}

trent()
{
	auto	int	r = FALSE;
	extern	_intr();

	signal(SIGINT, _intr);
	if(!setjmp(jbuf)) {
		mktrobj();
		if(!rdt)
			rd_trdef(1);
		resetv();
		osetv(Trobj, "_lnum_", Locnum);
		osetv(Trobj, "_date_", Cdate);
		osetv(Trobj, "_logn_", Logn);
		osetv(Trobj, "_uun_", Node);
		ofill(Trobj, sflg);
		signal(SIGINT, SIG_IGN);
		osetv(Trobj, "_xed_", NULL);
		if(str_equ((char *) ogetv(Trobj, "PROD"), "unix"))
			osetv(Trobj, "PROD_REL", NULL);
		fixdesc(Trobj);
		oappend(Tytr, Trobj, 0, NULL);
		printf("\nTrouble report %s entered.\n", Locnum);
		Prevobj = Trobj;
		r = TRUE;
	}
	signal(SIGINT, SIG_DFL);
	return(r);
}

_intr()
{
	printf("\n\t\tTrouble report interrupted.\n");
	longjmp(jbuf, 1);
}

mktrobj()
{
	Locnum = get_lnum();
	Trobj = ocreat(Tytr, Locnum);
}

char *
get_lnum()
{
	reg	char	*ln = get_some(char, 20);
	reg	char	*sid;
	reg	char	*ts = get_some(char, 10);

	sid = Node;
	if(!sid)
		sid = "unk";
	sprintf(ts, "%ld", time((long *) NULL));
	sprintf(ln, "%s.%s", sid, ts + 4);
	cfree(ts);
	return(ln);
}

char	*rflds[] = {"NAME", "ADDR", "CITY", "STATE", "ZIP", "COUNTRY", "PHONE",
		    "ROOM", "CO", "CID", "SID", "CPUNO", "OS_REL", NULL};

resetv()
{
	reg	int	i;
	reg	char	*s;

	if(Prevobj)
		for(i = 0; s = rflds[i]; i++)
			osetv(Trobj, s, ogetv(Prevobj, s));
}

/*
**	Read system and user trdef files.  Set any values found (except those
**	not in rflds above), as well as trenter internal names (e.g., EDITOR).
**	If f is FALSE, read only SPOOL/.trdef (for adm send).
*/

rd_trdef(f)
{
	reg	char	*file;

	if(!Home)
		return;
	if(f) {
		file = get_some(char, strlen(Home) + strlen(TRDEF) + 5);
		sprintf(file, "%s/%s", Home, TRDEF);
		_rdtrd(file);
		cfree(file);
	}
	file = get_some(char, strlen(SPOOLDIR) + strlen(TRDEF) + 5);
	sprintf(file, "%s/%s", SPOOLDIR, TRDEF);
	_rdtrd(file);
	cfree(file);
	if(!Mach) {
		fprintf(stderr, "Destination machine unknown; contact the SGI Hotline for information.\n");
		exit(2);
	}
	rdt = TRUE;
}

_rdtrd(f)
char	*f;
{
	auto	FILE	*fp;
	auto	char	buf[120];
	auto	char	*ptr;
	extern	char	*getenv();

	if((fp = fopen(f, "r")) == NULL)
		return;
	while(fgets(buf, 120, fp) != NULL) {
		reg	char	*p = strchr(buf, '=');

		if(!p)
			continue;
		zapnl(buf);
		*p++ = NULL;
		if(str_equ(buf, "EDITOR"))
			Edit = str_cpy(p);
		else if(str_equ(buf, "NODE"))
			Mach = str_cpy(p);
		else if(word_ind(buf, rflds) != -1)
			osetv(Trobj, buf, p);
				/* env overides */
		if (p=getenv("EDITOR"))
			Edit = str_cpy(ptr);
		if (p=getenv("NODE"))
			Mach = str_cpy(ptr);
		if (!Edit)
			Edit = "vi";
	}
	fclose(fp);
}

/*
**	Handle any desired reformatting of description field.  Currently,
**	we're only looking for .ES/.EE (example start/end).
*/

fixdesc(obj)
OBJECT	obj;
{
	reg	char	*p = (char *) ogetv(obj, "DESC");
	reg	char	*s, *t;
	reg	char	*os = NULL;
	auto	int	esc = 0;

	if(!p)
		return;
	s = str_cpy(p);
	while(p = strtok(s, "\n")) {
		s = NULL;
		if(str_equ(p, ".ES")) {
			esc++;
			continue;
		}
		if(str_equ(p, ".EE")) {
			if(esc)
				esc--;
			continue;
		}
		if(esc) {
			reg	int	i = 0;
			reg	char	*ts = get_some(char, esc + 2);

			while(i < esc)
				ts[i++] = '\t';
			os = str_cat(os, ts);
			cfree(ts);
		}
		os = str_cat(os, p);
		t = &p[strlen(p)];
		do {
			os = str_cat(os, "\n");
		} while(*++t == '\n');
	}
	osetv(obj, "DESC", os);
	if(os)
		cfree(os);
}

/*
**	Internal print routine for TRs (hack due to lack of obj formatter)
*/

trprt(tp)
OBJECT	tp;
{
	reg	char	*s;

	printf("Trouble Report # %s\t\tDate: %s\n\n",	ogetv(tp, "_lnum_"),
							ogetv(tp, "_date_"));
	printf("TYPE: %s\t\tSEVERITY: %s", ogetv(tp, "TYPE"),
						s = (char *) ogetv(tp, "SEV"));
	if(*s == '2')
		printf("\tDATE REQ'D: %s", ogetv(tp, "RDATE"));
	printf("\nPRODUCT: %s\tOS REL: %s", s = (char *) ogetv(tp, "PROD"),
							ogetv(tp, "OS_REL"));
	if(!str_equ(s, "unix"))
		printf("\tPROD (%s) REL: %s", s, ogetv(tp, "PROD_REL"));
	printf("\nORIG NAME: %s\tPHONE: %s\tCOMPANY: %s\n", ogetv(tp, "NAME"),
					ogetv(tp, "PHONE"), ogetv(tp, "CO"));
	printf("ROOM: %s\tADDRESS: %s\n", ogetv(tp, "ROOM"), ogetv(tp, "ADDR"));
	printf("\t\t\t%s %s %s %s\n", ogetv(tp, "CITY"),
		    ogetv(tp, "STATE"), ogetv(tp, "ZIP"), ogetv(tp, "COUNTRY"));
	printf("LOGIN: %s\tUUCP NODE: %s\n", ogetv(tp, "_logn_"),
							  ogetv(tp, "_uun_"));
	printf("CUSTOMER ID: %s\tSITE ID: %s\n", ogetv(tp, "CID"),
							  ogetv(tp, "SID"));
	printf("CPU SERIAL # %s\t\t\tMACHINE: %s\n", ogetv(tp, "CPUNO"),
							ogetv(tp, "MACH"));
	printf("ATTACHMENTS: %s\n\nABSTRACT: %s\n\n", ogetv(tp, "ATT"),
							ogetv(tp, "ABS"));
	printf("DESCRIPTION:\n%s\n", ogetv(tp, "DESC"));
}

/*
**	Verify select fields (required date)
*/

#define	MAXYR	99	/* Maximum sensible year for req'd date (- 1900) */

tverify(fname, s)
char	*fname;
char	*s;
{
	reg	char	*p;

	if(str_equ(fname, "RDATE")) {
		auto	int	mon, day, yr;
		auto	int	pm = Timep->tm_mon + 1;
		auto	int	pd = Timep->tm_mday;
		auto	int	py = Timep->tm_year;

		p = strchr(s, '/');
		if(!p)
			goto err;
		*p = NULL;
		mon = atoi(s);
		if(mon < 1 || mon > 12)
			goto err;
		*p++ = '/';
		s = p;
		p = strchr(s, '/');
		if(!p)
			goto err;
		*p = NULL;
		day = atoi(s);
		if(day < 1 || day > 31)
			goto err;
		*p++ = '/';
		if(!p)
			goto err;
		yr = atoi(p);
		if(yr > MAXYR)
			goto err;
		if(yr < py)
			goto err;
		if(yr > py) {
			if(pm < 12 || mon > 1 || pd < 26 || day > 3)
				goto ok;
			goto err;
		}
		if(mon < pm)
			goto err;
		if(mon > pm) {
			if(pd < 26 || day > 3)
				goto ok;
			goto err;
		}
		if((day - pd) > 6)
			goto ok;
	}
err:
	return(FALSE);
ok:
	return(TRUE);
}

/*
**	Perform post msg handling for Severity and Attachments
*/

postmsg(fname, val)
char	*fname;
char	*val;
{
	if(!strcmp(fname, "SEV") && !strcmp(val, "1"))
		printf("%s\n", Admf ? A_SEV1MSG : U_SEV1MSG);
	else if(!strcmp(fname, "ATT") && (*val == 'y' || *val == 'Y'))
		printf("%s\n", Admf ? A_ATTMSG : U_ATTMSG);
}

usage()
{
	fprintf(stderr, "usage: trenter [-s]\n");
}

prmotd()
{
	if(access(MOTD, 04) == 0) {
		char	*s = rdcmd("cat", MOTD);

		if(s) {
			printf("%s\n\n", s);
			cfree(s);
		}
	}
}

trsend(oq)
OBJECTQ	oq;
{
	reg	int	i;
	reg	OBJECT	obj;
	reg	char	*p;
	reg	char	*r;
	reg	char	*ofile;

	if(!rdt)
		rd_trdef(0);
	ofile = get_some(char, strlen(SPOOLDIR) + strlen(p) + 20);
	sprintf(ofile, "%s/send", SPOOLDIR);
	if(access(ofile, 0) == -1) {
		printf("cannot access %s -- nothing sent.\n", ofile);
		return;
	}
	for(i = 0; obj = oq[i]; i++) {
		p = (char *) ogetv(obj, "_lnum_");
		strcat(ofile, "/");
		strcat(ofile, p);
		if(access(ofile, 0) == -1)
			break;
		r = strrchr(ofile, '/');
		*r = NULL;
	}
	if(!obj) {
		printf("Send file name clash -- clean %s and try again.\n",
									ofile);
		cfree(ofile);
		return;
	}
	signal(SIGINT, SIG_IGN);
	printf("Sending TRs:\n");
	for(i = 0; obj = oq[i]; i++) {
		oappend(Tytr, obj, 0, ofile);
		printf("%s (%c%d) ", ogetv(obj, "_lnum_"), SCHAR, i);
		if((i + 1) % TRPCOL == 0)
			putchar('\n');
	}
	putchar('\n');
	p = get_some(char, 120);
	sprintf(p, "uucp -m %s %s!%s", ofile, Mach, S_MDIR);
	r = rdcmd(p, NULL);
	if(Rdrc == 0) {
		printf("Packet %s sent\n", ofile);
		for(i = 0; obj = oq[i]; i++)
			orem(obj);
	} else
		printf("uucp error -- nothing sent.\n");
	cfree(ofile);
	cfree(p);
}
