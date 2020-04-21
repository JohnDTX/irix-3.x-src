/*
 * $Source: /d2/3.7/src/sys/config/RCS/mkmakefile.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:27:33 $
 */
#ifndef lint
static char sccsid[] = "@(#)mkmakefile.c	1.30 (Berkeley) 8/11/83";
#endif

/*
 * Build the makefile for the system, from
 * the information in the files files and the
 * additional files for the machine being compiled to.
 */

#include <stdio.h>
#include <ctype.h>
#include "y.tab.h"
#include "config.h"

#define next_word(fp, wd) \
	{ register char *word = get_word(fp); \
	  if (word == (char *)EOF) \
		return; \
	  else \
		wd = word; \
	}

static	struct file_list *fcur;
char *tail();

/*
 * Lookup a file, by make.
 */
struct file_list *
fl_lookup(file)
	register char *file;
{
	register struct file_list *fp;

	for (fp = ftab ; fp != 0; fp = fp->f_next) {
		if (eq(fp->f_fn, file))
			return (fp);
	}
	return (0);
}

/*
 * Lookup a file, by final component name.
 */
struct file_list *
fltail_lookup(file)
	register char *file;
{
	register struct file_list *fp;

	for (fp = ftab ; fp != 0; fp = fp->f_next) {
		if (eq(tail(fp->f_fn), tail(file)))
			return (fp);
	}
	return (0);
}

/*
 * Make a new file list entry
 */
struct file_list *
new_fent()
{
	register struct file_list *fp;

	fp = (struct file_list *) malloc(sizeof *fp);
	fp->f_needs = 0;
	fp->f_next = 0;
	fp->f_flags = 0;
	fp->f_type = 0;
	if (fcur == 0)
		fcur = ftab = fp;
	else
		fcur->f_next = fp;
	fcur = fp;
	return (fp);
}

char	*COPTS;

/*
 * Build the makefile from the skeleton
 */
makefile()
{
	FILE *ifp, *ofp;
	char line[BUFSIZ];
	struct opt *op;
#ifdef	sgi
	extern char *graphics_name;
#endif

	read_files();
	strcpy(line, "../conf/makefile.");
	(void) strcat(line, machinename);
	ifp = fopen(line, "r");
	if (ifp == 0) {
		perror(line);
		exit(1);
	}
	(void)rename(path("makefile"), path("makefile.bak"));
	ofp = fopen(path("makefile"), "w");
	if (ofp == 0) {
		perror(path("makefile"));
		exit(1);
	}
	fprintf(ofp, "IDENT=");
	if (ident)
		fprintf(ofp, " -D%s", raise(ident));
	if (profiling)
		if (machine == MACHINE_VAX)
			fprintf(ofp, " -DGPROF");
		else
			fprintf(ofp, " -DPROF");
	if (cputype == 0) {
		printf("cpu type must be specified\n");
		exit(1);
	}
	{ struct cputype *cp;
	  for (cp = cputype; cp; cp = cp->cpu_next)
		fprintf(ofp, " -D%s", cp->cpu_name);
	}
	for (op = opt; op; op = op->op_next)
		if (op->op_value)
			fprintf(ofp, " -D%s=\"%s\"", op->op_name, op->op_value);
		else
			fprintf(ofp, " -D%s", op->op_name);
	fprintf(ofp, "\n");
	if (hadtz == 0)
		printf("timezone not specified; gmt assumed\n");
	fprintf(ofp, "PARAM=-DMAXUSERS=%d\n", maxusers);
	if (graphics_name)
		fprintf(ofp, "KERNEL_GL=%s\n", graphics_name);

	while (fgets(line, BUFSIZ, ifp) != 0) {
		if (*line == '%')
			goto percent;
		if (profiling && strncmp(line, "COPTS=", 6) == 0) {
			register char *cp;

			fprintf(ofp, 
			    "GPROF.EX=/usr/src/lib/libc/%s/csu/gmon.ex\n",
			    machinename);
			cp = strchr(line, '\n');
			if (cp)
				*cp = 0;
			cp = line + 6;
			while (*cp && (*cp == ' ' || *cp == '\t'))
				cp++;
			COPTS = malloc((unsigned)(strlen(cp) + 1));
			if (COPTS == 0) {
				printf("config: out of memory\n");
				exit(1);
			}
			strcpy(COPTS, cp);
			if (machine == MACHINE_VAX)
				fprintf(ofp, "%s -pg\n", line);
			else
				fprintf(ofp, "%s -p\n", line);
			continue;
		}
		fprintf(ofp, "%s", line);
		continue;
	percent:
		if (eq(line, "%RULES\n"))
			do_rules(ofp);
		else if (eq(line, "%LOAD\n"))
			do_load(ofp);

		else if (eq(line, "%SYSOBJS\n"))
			do_objs(ofp, XT_SYSTEM);
		else if (eq(line, "%DEVOBJS\n"))
			do_objs(ofp, XT_DEVICE);
		else if (eq(line, "%OEMOBJS\n"))
			do_objs(ofp, XT_OEM);

		else if (eq(line, "%SYSFILES\n"))
			do_cfiles(ofp, XT_SYSTEM);
		else if (eq(line, "%DEVFILES\n"))
			do_cfiles(ofp, XT_DEVICE);
		else if (eq(line, "%OEMFILES\n"))
			do_cfiles(ofp, XT_OEM);

		else if (eq(line, "%LIBS\n"))
			do_libraries(ofp);
		else if (eq(line, "%DEPEND\n")) {
			register FILE *bfp;
#define DEPLINE1 "# DO NOT DELETE THIS LINE -- make depend uses it\n"
#define DEPLINE2 "# DO NOT DELETE THIS 2nd LINE -- make depend uses it\n"
			fprintf(ofp,DEPLINE1);
			bfp = fopen(path("makefile.bak"), "r");
			if (bfp != 0) {
				while (fgets(line, BUFSIZ, bfp) != 0
				       && !eq(line, DEPLINE1))
					continue;
				while (fgets(line, BUFSIZ, bfp) != 0
				       && !eq(line, DEPLINE2))
					fprintf(ofp,line);
				(void) fclose(bfp);
			}
			fprintf(ofp,DEPLINE2);
		} else
			fprintf(stderr,
			    "Unknown %% construct in generic makefile: %s",
			    line);
	}
	(void) fclose(ifp);
	(void) fclose(ofp);
}

/*
 * Read in the information about files used in making the system.
 * Store it in the ftab linked list.
 */
read_files()
{
	FILE *fp;
	register struct file_list *tp;
	register struct device *dp;
	char *wd, *this, *needs, *devorprof;
	char fname[32];
	int nreqs, first = 1, configdep;
	int xtype = XT_SYSTEM;

	ftab = 0;
	(void) strcpy(fname, "files");
openit:
	fp = fopen(fname, "r");
	if (fp == 0) {
		perror(fname);
		exit(1);
	}
next:
	/*
	 * filename	[ standard | optional ] [ config-dependent ]
	 *	[ dev* | profiling-routine ] [ device-driver]
	 */
	wd = get_word(fp);
	if (wd == (char *)EOF) {
		(void) fclose(fp);
		if (first == 1) {
			(void) sprintf(fname, "files.%s", machinename);
			first++;
			xtype = XT_DEVICE;
			goto openit;
		}
		if ((first == 2) && ident) {
			(void) sprintf(fname, "files.%s", raise(ident));
			first++;
			xtype = XT_OEM;
			fp = fopen(fname, "r");
			if (fp != 0)
				goto next;
		}
		return;
	}
	if (wd == 0)
		goto next;
	this = ns(wd);
	next_word(fp, wd);
	if (wd == 0) {
		printf("%s: No type for %s.\n",
		    fname, this);
		exit(1);
	}
	if (fl_lookup(this)) {
		printf("%s: Duplicate file %s.\n",
		    fname, this);
		exit(1);
	}
	tp = 0;
	if (first == 3 && (tp = fltail_lookup(this)) != 0)
		printf("%s: Local file %s overrides %s.\n",
		    fname, this, tp->f_fn);
	nreqs = 0;
	devorprof = "";
	configdep = 0;
	needs = 0;
	if (eq(wd, "standard"))
		goto checkdev;
	if (!eq(wd, "optional")) {
		printf("%s: %s must be optional or standard\n", fname, this);
		exit(1);
	}
nextopt:
	next_word(fp, wd);
	if (wd == 0)
		goto doneopt;
	if (eq(wd, "config-dependent")) {
		configdep++;
		goto nextopt;
	}
	devorprof = wd;
	if (eq(wd, "device-driver") || eq(wd, "profiling-routine")) {
		next_word(fp, wd);
		goto save;
	}
	nreqs++;
	if (needs == 0)
		needs = ns(wd);
	for (dp = dtab; dp != 0; dp = dp->d_next)
		if (eq(dp->d_name, wd))
			goto nextopt;
	while ((wd = get_word(fp)) != 0)
		;
	if (tp == 0)
		tp = new_fent();
	tp->f_fn = this;
	tp->f_type = INVISIBLE;
	tp->f_needs = needs;
	goto next;

doneopt:
	if (nreqs == 0) {
		printf("%s: what is %s optional on?\n",
		    fname, this);
		exit(1);
	}

checkdev:
	if (wd) {
		next_word(fp, wd);
		if (wd) {
			if (eq(wd, "config-dependent")) {
				configdep++;
				goto checkdev;
			}
			devorprof = wd;
			next_word(fp, wd);
		}
	}

save:
	if (wd) {
		printf("%s: syntax error describing %s\n",
		    fname, this);
		exit(1);
	}
	if (eq(devorprof, "profiling-routine") && profiling == 0)
		goto next;
	if (tp == 0)
		tp = new_fent();
	tp->f_fn = this;
	if (eq(devorprof, "device-driver"))
		tp->f_type = DRIVER;
	else if (eq(devorprof, "profiling-routine"))
		tp->f_type = PROFILING;
	else
		tp->f_type = NORMAL;
	tp->f_xtype = xtype;
	tp->f_flags = 0;
	if (configdep)
		tp->f_flags |= CONFIGDEP;
	tp->f_needs = needs;
	goto next;
}

#ifndef	sgi
do_objs(fp)
	FILE *fp;
#else
do_objs(fp, xtype)
	FILE *fp;
	char xtype;
#endif
{
	register struct file_list *tp, *fl;
	register int lpos, len;
	register char *cp, och, *sp;
	char swapname[32];

#ifndef	sgi
	fprintf(fp, "OBJS=");
	lpos = 6;
#else
	switch (xtype) {
	  case XT_SYSTEM:
		fprintf(fp, "SYSOBJS= ");
		break;
	  case XT_DEVICE:
		fprintf(fp, "DEVOBJS= ");
		break;
	  case XT_OEM:
		fprintf(fp, "OEMOBJS= ");
		break;
	}
	lpos = 9;
#endif
	for (tp = ftab; tp != 0; tp = tp->f_next) {
		if (tp->f_type == INVISIBLE)
			continue;
#ifdef	sgi
		if (tp->f_xtype != xtype)
			continue;
#endif
		sp = tail(tp->f_fn);
		for (fl = conf_list; fl; fl = fl->f_next) {
			if (fl->f_type != SWAPSPEC)
				continue;
			(void) sprintf(swapname, "swap%s.c", fl->f_fn);
			if (eq(sp, swapname))
				goto cont;
		}
		cp = sp + (len = strlen(sp)) - 1;
		och = *cp;
		*cp = 'o';
		if (len + lpos > 72) {
#ifndef	sgi
			lpos = 8;
#else
			lpos = 9;
#endif
			fprintf(fp, "\\\n\t");
		}
		fprintf(fp, "%s ", sp);
		lpos += len + 1;
		*cp = och;
cont:
		;
	}
	if (lpos != 8)
		putc('\n', fp);
}

#ifndef	sgi
do_cfiles(fp)
	FILE *fp;
#else
do_cfiles(fp, xtype)
	FILE *fp;
	char xtype;
#endif
{
	register struct file_list *tp;
	register int lpos, len;

#ifndef	sgi
	fprintf(fp, "CFILES=");
	lpos = 8;
#else
	switch (xtype) {
	  case XT_SYSTEM:
		fprintf(fp, "SYSFILES= ");
		break;
	  case XT_DEVICE:
		fprintf(fp, "DEVFILES= ");
		break;
	  case XT_OEM:
		fprintf(fp, "OEMFILES= ");
		break;
	}
	lpos = 9;
#endif
	for (tp = ftab; tp != 0; tp = tp->f_next) {
		if (tp->f_type == INVISIBLE)
			continue;
#ifdef	sgi
		if (binaryconfig && (tp->f_xtype != XT_OEM))
			continue;
		if (tp->f_xtype != xtype)
			continue;
#endif
		if (tp->f_fn[strlen(tp->f_fn)-1] != 'c')
			continue;
		if ((len = 3 + strlen(tp->f_fn)) + lpos > 72) {
			lpos = 8;
			fprintf(fp, "\\\n\t");
		}
		fprintf(fp, "../%s ", tp->f_fn);
		lpos += len + 1;
	}
	if (lpos != 8)
		putc('\n', fp);
}

char *
tail(fn)
	char *fn;
{
	register char *cp;

	cp = strrchr(fn, '/');
	if (cp == 0)
		return (fn);
	return (cp+1);
}

/*
 * Create the makerules for each file
 * which is part of the system.
 * Devices are processed with the special c2 option -i
 * which avoids any problem areas with i/o addressing
 * (e.g. for the VAX); assembler files are processed by as.
 */
do_rules(f)
	FILE *f;
{
	register char *cp, *np, och, *tp;
	register struct file_list *ftp;
	char *extras;

for (ftp = ftab; ftp != 0; ftp = ftp->f_next) {
	if (ftp->f_type == INVISIBLE)
		continue;
#ifdef	sgi
	if (binaryconfig && (ftp->f_xtype != XT_OEM))
		continue;
#endif
	cp = (np = ftp->f_fn) + strlen(ftp->f_fn) - 1;
	och = *cp;
	*cp = '\0';
	fprintf(f, "%so: ../%s%c\n", tail(np), np, och);
	tp = tail(np);
	if (och == 's') {
		fprintf(f, "\t${AS} -o %so ../%ss\n\n", tp, np);
		continue;
	}
	if (ftp->f_flags & CONFIGDEP)
		extras = "${PARAM} ";
	else
		extras = "";
	switch (ftp->f_type) {

	case NORMAL:
		switch (machine) {

		case MACHINE_VAX:
			fprintf(f, "\t${CC} -I. -c -S ${COPTS} %s../%sc\n",
				extras, np);
			fprintf(f, "\t${C2} %ss | sed -f ../%s/asm.sed |",
			    tp, machinename);
			fprintf(f, " ${AS} -o %so\n", tp);
			fprintf(f, "\trm -f %ss\n\n", tp);
			break;

		case MACHINE_SUN:
			break;

		case MACHINE_PM2:
		case MACHINE_IP2:
			fprintf(f, "\t${CC} -I. -c -OkS ${COPTS} %s../%sc\n",
				   extras, np);
			break;
		}
		break;

	case DRIVER:
		switch (machine) {

		case MACHINE_VAX:
			fprintf(f, "\t${CC} -I. -c -S ${COPTS} %s../%sc\n",
				extras, np);
			fprintf(f,"\t${C2} -i %ss | sed -f ../%s/asm.sed |",
			    tp, machinename);
			fprintf(f, " ${AS} -o %so\n", tp);
			fprintf(f, "\trm -f %ss\n\n", tp);
			break;

		case MACHINE_SUN:
			break;
		case MACHINE_PM2:
		case MACHINE_IP2:
			fprintf(f,
				"\t${CC} -I. -c -OkKS ${COPTS} %s../%sc\n",
				extras, np);
			break;
		}
		break;

	case PROFILING:
		if (!profiling)
			continue;
		if (COPTS == 0) {
			fprintf(stderr,
			    "config: COPTS undefined in generic makefile");
			COPTS = "";
		}
		switch (machine) {

		case MACHINE_VAX:
			fprintf(f, "\t${CC} -I. -c -S %s %s../%sc\n",
				COPTS, extras, np);
			fprintf(f, "\tex - %ss < ${GPROF.EX}\n", tp);
			fprintf(f,
			    "\tsed -f ../vax/asm.sed %ss | ${AS} -o %so\n",
			    tp, tp);
			fprintf(f, "\trm -f %ss\n\n", tp);
			break;

		case MACHINE_PM2:
		case MACHINE_IP2:
			fprintf(f,
				"\t${CC} -I. -c -OkS ${COPTS} %s../%sc\n",
				extras, np);
			break;

		case MACHINE_SUN:
			fprintf(stderr,
			    "config: don't know how to profile kernel\n");
			break;
		}
		break;

	default:
		printf("Don't know rules for %s\n", np);
		break;
	}
	*cp = och;
}
}

/*
 * Create the load strings
 */
do_load(f)
	register FILE *f;
{
	register struct file_list *fl;
	int first = 1;
	struct file_list *do_systemspec();

	fl = conf_list;
	while (fl) {
		if (fl->f_type != SYSTEMSPEC) {
			fl = fl->f_next;
			continue;
		}
		fl = do_systemspec(f, fl, first);
		if (first)
			first = 0;
	}
	fprintf(f, "TARGETS=");
	for (fl = conf_list; fl != 0; fl = fl->f_next)
		if (fl->f_type == SYSTEMSPEC)
			fprintf(f, " %s", fl->f_needs);
	fprintf(f, "\n\nall:${TARGETS}\n");
}

struct file_list *
do_systemspec(f, fl, first)
	FILE *f;
	register struct file_list *fl;
	int first;
{
	extern int libraries_saved;

	switch (machine) {
	case MACHINE_PM2:
	case MACHINE_IP2:
	    /* generate default rule */
		fprintf(f, "default: %s\n\n", 
			binaryconfig ? "binary" : fl->f_needs);

	    /* generate system.o rule for binary systems */
		fprintf(f, "%s.o: makefile machine ${SYSOBJS}\n", fl->f_needs);
		fprintf(f, "\t@echo creating %s.o\n", fl->f_needs);
		fprintf(f, "\t@rm -f %s.o ../kernels/${TYPE}%s.o\n",
			   fl->f_needs, fl->f_needs);
		fprintf(f, "\t@${LD} -x -X -r -o %s.o ${SYSOBJS}\n",
			   fl->f_needs);
		fprintf(f, "\t@ln %s.o ../kernels/${TYPE}%s.o\n",
			   fl->f_needs, fl->f_needs);
		fprintf(f, "\t@ls -l ../kernels/${TYPE}%s.o\n\n",
			   fl->f_needs);

	    /* generate target system rule (standard source config rule) */
		if (libraries_saved)
			fprintf(f, "${LIBS}: anything\n\n");
		break;
	}
	do_swapspec(f, fl->f_fn);
	do {
		fl = fl->f_next;
	} while (fl && (fl->f_type == SWAPSPEC));
	return (fl);
}

do_swapspec(f, name)
	FILE *f;
	register char *name;
{
	switch (machine) {
	  case MACHINE_VAX:
		if (!eq(name, "generic")) {
			fprintf(f, "swap%s.o: swap%s.c\n", name, name);
			fprintf(f, "\t${CC} -I. -c -O ${COPTS} swap%s.c\n\n",
				   name);
			break;
		}
		fprintf(f, "swapgeneric.o: ../%s/swapgeneric.c\n", machinename);
		fprintf(f, "\t${CC} -I. -c -S ${COPTS} ");
		fprintf(f, "../%s/swapgeneric.c\n", machinename);
		fprintf(f, "\t${C2} swapgeneric.s | sed -f ../%s/asm.sed",
		    machinename);
		fprintf(f, " | ${AS} -o swapgeneric.o\n");
		fprintf(f, "\trm -f swapgeneric.s\n\n");
		break;
	  case MACHINE_SUN:
		if (!eq(name, "generic")) {
			fprintf(f, "swap%s.o: swap%s.c\n", name, name);
			fprintf(f, "\t${CC} -I. -c -O ${COPTS} swap%s.c\n\n",
				   name);
		}
		fprintf(f, "swapgeneric.o: ../%s/swapgeneric.c\n", machinename);
		fprintf(f, "\t${CC} -I. -c -O ${COPTS} ");
		fprintf(f, "../%s/swapgeneric.c\n\n", machinename);
		break;
	  case MACHINE_PM2:
	  case MACHINE_IP2:
		break;
	}
}

char *
raise(str)
	register char *str;
{
	register char *cp = str;

	while (*str) {
		if (islower(*str))
			*str = toupper(*str);
		str++;
	}
	return (cp);
}
