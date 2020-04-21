/*
 * (C) 1982 UniSoft Systems of Berkeley CA
 *
 * UniPlus Source Code. The following source code
 * contains proprietary information and may not be
 * duplicated or used without written permission from
 * UniSoft Systems.
 */

#include "vchk.h"

extern char *OEMLIST;
char *locferr;
char *makepath(), *index();
char ****oem;
#define NMACROS	100
struct mmac {
	char *name;
	char *value;
} mmac[NMACROS];

/* Find the line of OEMLIST that suits this invokation.  If `p' points
 * to a name then that name is used to find the line, otherwise the user
 * id name (not the login name) is used.
 * The complex line pointer (from loadfyl) is returned if the line is found
 * otherwise zero is returned.
 */

char ***Pline;
char *file;

char ***
getoem(p)
	char *p;
{
	char *acctname;
	int i, j;
	int mi;
	char *s;

	if (!oem) {		/* Havent read in oemfile yet */
		if (!(oem = (char ****)loadfyl(OEMLIST,"\n:|")))
			E(("`%s' file is missing or unreadable",OEMLIST));
		if (!*oem || !*oem[0] || !(acctname = *oem[0][0]))
			E(("`%s' file missing first line",OEMLIST));
		oem++;		/** skip `superuser' login name **/
		/* parse macro names */
		mi = 0;
		while (s = index(*oem[0][0], '=')) {
			if (mi >= NMACROS)
				E(("Too many macros (%d)", NMACROS));
			mmac[mi].name = *oem[0][0];
			*s++ = 0;
			mmac[mi++].value = s;
			oem++;
		}
		mmac[mi].name = 0;
	}
	for (i=0; oem[i]; i++)
		for (j=0; s = oem[i][0][j]; j++)
			if (strcmp(s,p) == 0)
				return(oem[i]+1);
	return 0;
}

/* The file we are looking for is in the global `file'.
 * P (our argument) is one line from the oemlist. (minus the name field)
 * We check each colon separated field by calling `oneof' till it returns
 * nonzero.
 */
char path[PATHSIZ];
char *oneof();

char *
anyof(p)
	char ***p;
{
	char *r;
	int i;

	for (i=0; p[i]; i++)
		if ((r = oneof(p[i])) != 0)
			return r;
	r = ".does not exist";
	*r = 0;
	return r;
}

/* Given a list of directories (p) search each for `file' (the global).
 * If found in only one of the directories then the full pathname to that
 * file is returned.  Otherwise if the file exists in more than one directory
 * a string is returned (with the first byte null) expressing the directories
 * it was found in.
 * If the file is not found in any directory then 0 is returned.
 */
char *
oneof(p)
	char **p;
{
	char *rn;
	char *savptr;
	static char errbuf[BUFSIZ];
	char *ep, ***lp;
	int foundit, i;
	int fcnt;

	sprintf(errbuf,".Which one? `%s' exists in",file);
	ep = errbuf + strlen(errbuf);
	*errbuf = 0;
	fcnt = 0;

	for (i=0; rn=p[i]; i++) {
		if (*rn == '@') {	/* goto line for ... (text after @)*/
			if (i != 0 || p[1]) {
				F(("Invalid control file syntax\n"));
			}
			if (lp = getoem(rn+1))
				return anyof(lp);
			else {
				F(("missing control line for `%s'",rn+1));
			}
		}
		rn = makepath(rn, file);
		if (access(rn,4)) continue;
		*ep++ = ' ';
		savptr = ep;
		ep += xcps(ep,rn);
		fcnt++;
	}
	if (fcnt == 0) return 0;
	if (fcnt > 1) return errbuf;
	return savptr;
}

char *
makepath(proto, file)
register char *proto;
char *file;
{
	static char lb[BUFSIZ];
	char mname[BUFSIZ];
	struct mmac *mp;
	char *p, *m;

	p = lb;
	while (*proto) {
		if (*proto == '$') {
			proto++;
			for (m = mname; ; )
				if ( (*proto >= 'A' && *proto <= 'Z') ||
				     (*proto >= 'a' && *proto <= 'z') ||
				     (*proto >= '1' && *proto <= '9') ||
				     *proto == '_')
						*m++ = *proto++;
				else
					break;
			*m++ = 0;
			for (mp = mmac; mp->name; mp++)
				if (strcmp(mp->name, mname) == 0)
					break;
			if (mp->name == 0)
				E(("Undefined macro %s", mname));
			for (m = mp->value; *m; *p++ = *m++) ;
		} else
			*p++ = *proto++;
	}
	*p++ = '/';
	strcpy(p, file);
	return(lb);
}
