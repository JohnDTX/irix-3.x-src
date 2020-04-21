/*
 * (C) 1983 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

/* @(#)dsrch.c	1.6 */
#include "vchk.h"
/* Search for directory in internal tree to find appropriate
 * modes and ownership for contents.  The filesystem is used the first time
 * a directory is encountered.
 */

extern char *rest;
char *realname();

dsrch (fn)
	char *fn;
{
	int prtf, m, mode, fail=0;
	int madeit = 0;
	char *path = fn;
	char **sp, *f;
	register struct tree_node *t, *nt;
	struct tree_node *rt, *pt, *getnode();
	struct stat sb;
	char *index();
	char *pindex();

	D(3,("--IN dsrch <%s> %s\n",fn,rest));
	f = fn;
	nt = t = 0;
	rt = first_node;
	t = rt;
	pt = 0;

	while (f = index(fn,'/')) {		/* while more dirs in path */
		*f = '\0';
		if (*fn == '\0') fn = "/";	/* the root */
		D(9,("searching for dir %s: [",fn));
		pt = (struct tree_node *)0;
		while (t) {
			D(9,("%s, ",t->t_name));
			pt = t;				/* save previous */
			if (strcmp(fn,t->t_name) == 0) {
				D(9,("\b\b] matched, descending\n"));
				rt = t;			/* save parent */
				t = t->t_list;
				goto descend;
			} else t = t->t_next;
		}
			/* Directory we haven't checked yet */
		D(9,("\b\b] must setup\n"));
		if (!rt) {
			D(2,("Establishing root node (%s)\n",fn));
			if (stat(realname(fn),&sb))
				F(("No `%s' directory: %s\n",fn,SE));
			nt = first_node = getnode(fn);
			cps(nt->t_name,fn);	/* name of this directory */
			nt->t_dmode = nt->t_mode = sb.st_mode & ~S_IFMT;
			nt->t_duid = nt->t_uid = sb.st_uid;
			nt->t_dgid = nt->t_gid = sb.st_gid;
			D(9,("Root(%s): M=%o U=%d G=%d m=%o u=%d g=%d\n",fn,nt->t_dmode,nt->t_duid,nt->t_dgid,nt->t_mode,nt->t_uid,nt->t_gid));
		} else {
			if (rt->t_ls)
				eliminate(fn,rt);
			nt = getnode(path);
			cps(nt->t_name,fn);	/* name of this directory */
			nt->t_duid = rt->t_duid; /* inherit parents status */
			nt->t_dgid = rt->t_dgid;
			nt->t_dmode = rt->t_dmode;
			nt->t_uid = rt->t_uid;	
			nt->t_gid = rt->t_gid;
			nt->t_mode = rt->t_mode;
			nt->t_parent = rt;
			if (pt) {
				D(9,("set prev(%s) to %s\n",pt->t_name,nt->t_name));
				pt->t_next = nt;
			} else {
				D(9,("set parent(%s) to %s\n",rt->t_name,nt->t_name));
				rt->t_list = nt;
				rt->t_size++;
			}
		}
		nt->t_idnum = ++cur_idnum;
		Curdir = nt;
		if (!rt || stat(realname(path),&sb) == 0) {
			if ((sb.st_mode & S_IFMT) != S_IFDIR) {
				X(("GAK! %s exists and is not a directory!!\n",path));
				return -1;
			}
			if (bflag) {
				nt->t_flags |= TF_SETUP;
				nt->t_mode = sb.st_mode & ~S_IFMT;
				nt->t_uid = sb.st_uid;
				nt->t_gid = sb.st_gid;
			} else {
/* D(0,("RT=0x%x --- f[1]=%o\n",rt,f[1])); */
				if (!rt)	/* if very first dir */
					chkdir(fn,&sb,nt);
				else if (f[1])	/* or non terminal dir */
					chkdir(path,&sb,nt);
			}
		} else {			/* will have to make it */
			madeit = 1;		/* made dir so no check */
			if (!bflag) {
				if (!f[1]) {	/* explicit dir */
					fillmodes(nt);
					nt->t_flags |= TF_SETUP;
				}
				fail = cexec("mkdir %s",path);
				if (!fail) {
					cexec("chmod %o %s",nt->t_mode,path);
					cexec("chown %s %s",acct(nt->t_uid),path);
					cexec("chgrp %d %s",nt->t_gid,path);
				}
			}
		}
		rt = nt;
	descend:
		*f++ = '/';
		fn = f;
	}
	if (rt) Curdir = rt;
/* D(0,("--- bflag=%d madeit=%d Curtype=%d\n",bflag, madeit, Curtype)); */
	if (!bflag && !madeit && (Curtype == DIRECTORY || Curtype == CONTENTS)){
		fillmodes(rt);
		if (stat(path,&sb) == 0)
			chkdir(path,&sb,rt);
	}
	D(9,(" LEAVING dsrch <%s>\n",path));
	return 0;
}

struct tree_node *
findnode(fn)
	char *fn;
{
	char *f;
	register struct tree_node *t;
	char *index();

	D(1,("findnode(%s)\n",fn));
	if (!first_node) E(("LOGIC(findnode): called with no tree [%s]\n",fn));
	t = first_node;
	if (*fn == '.') fn++;
	if (*fn != '/') E(("LOGIC(findnode): incorrect path `%s'\n",fn));
	f = ++fn;
	while (f = index(fn,'/')) {		/* while more dirs in path */
		t = t->t_list;
		*f = '\0';
		while (t) {
			if (strcmp(fn,t->t_name) == 0) goto descend;
			t = t->t_next;
		}
		return 0;
	descend:
		*f++ = '/';
		fn = f;
	}
	return t;
}

/* remove node t from the incore tree.  This does not affect unresolved
 * links since they are maintained speparately.
 * Returned is a pointer the the list of younger siblings (or null)
 */
struct tree_node *
prune (t)
	struct tree_node *t;
{
	struct tree_node *p, *s;

	if (!t) E(("LOGIC(prune): called with null tree\n"));
	D(3,("prune(%d:%s) has %d subdirs\n",t->t_idnum,t->t_name,t->t_size));

	for (s=t->t_list; s; s=s->t_next)
		prune(s);
	if (t->t_ls) free(t->t_ls);
	s = t->t_next;
	free(t);
	return s;
}

rmtree(t)
	struct tree_node *t;
{
	struct tree_node *p, *s;
	struct tree_node *prevnode(), *prune();

	D(3,("rmtree `%s'\n",p->t_name));
	p = prevnode(t);
	s = prune(t);

	if (!p) first_node = s;
	else if (p->t_list == t) p->t_list = s;
	else if (p->t_next == t) p->t_next = s;
	else E(("LOGIC(rmtree):`%s':%d is not prev of `%s'\n",p->t_name,p->t_idnum,t->t_name));
}

struct tree_node *
prevnode(t)		/* return ptr to node containing t_next ptr to t */
	struct tree_node *t;
{
	struct tree_node *p, *s;

	if (first_node == t) return 0;
	p = t->t_parent;
	if (p) {			/* this is a top level node */
		if (s = p->t_next) {	/* search for tree as sibling */
			if (s == t) return p;
			while (s->t_next != t)
				s = s->t_next;
			if (s) return s;
		}
		if (s = p->t_list) {	/* find our older sibling */
			if (s == t) return p;
		topkludge:
			while (s->t_next != t) 	/* find ptr to t */
				s = s->t_next;
			if (s) return s;
		}
	} else {			/* for multiple trees */
		s = first_node;
		goto topkludge;
	}
	E(("LOGIC(prevnode): %s not in tree\n",t->t_name));
}

scratch (fn)			/* like eliminate but first finds dir */
	char *fn;
{
	struct tree_node *t, *findnode();

	D(8,("SCRATCH (%s)\n",fn));
	t = findnode(fn);
	if (!t) return 0;
	return eliminate(fname(fn),t);
}

/* Eliminate filename fn from directory contents list of node t
 * This returns true or false.
 */
eliminate (fn, t)
	char *fn;
	struct tree_node *t;
{
	char **sp;
	int v;

	D(3,("ELIMINATE %s from %s(%d) ",fn,t->t_name,t->t_idnum));
	if (t->t_ls)
		for (sp=t->t_ls[0]; sp<t->t_ls[1]; sp++) {
			if (!*sp) continue;
			if ((v = fncomp(*sp,fn)) == 0) {
				*sp = 0;
				D(3,("succedded\n"));
				return 1;
			}
			if (v > 0) break;
		}
	else
		D(3,(" [unloaded]"));
	D(3,("failed\n"));
	return 0;
}

fncomp(a, b)
	register char *a, *b;
{
	register int dif;

	while (*b && *a) {
		if (*b == '\\' && b[1]) b++;
		if (dif = (*a++ - *b++)) return dif;
	}
	return (*a - *b);
}

/* Return pointer to static area containing pathname after escape sequences
 * are removed.
 */
char *
realname(p)
	register char *p;
{
	static char buf[PATHSIZ];
	register char *r = buf;

	while (*p) {
		if ((*r = *p++) == '\\')
			if (*p) *r = *p++;
		r++;
	}
	*r = '\0';
	return buf;
}

#ifdef DEBUG
prttree(n)
	struct tree_node *n;
{
	if (n) {
		fprintf(stderr,"INTERNAL TREE:\n");
		fprintf(stderr,"/");
		ptree(n);
	} else fprintf(stderr,"<empty>");
	fprintf(stderr,"\n");
}

ptree(n)
	struct tree_node *n;
{
	if (!n) return;
	fprintf(stderr,"%s:%o",n->t_name,n->t_duid,n->t_dmode);
	if (n->t_size) {
		fprintf(stderr,"=<");
		ptree(n->t_list);
		fprintf(stderr,">");
	}
	if (n->t_next) {
		fprintf(stderr,",");
		ptree(n->t_next);
	}
}
#endif

char *
pwd ()
{
	static char buf[PATHSIZ];
	register char *s, *n, *b = buf+sizeof(buf);
	register struct tree_node *t = Curdir;

	*--b = '\0';
	while (t) {
		s = t->t_name;
		for (n=s; *n; n++);	/* setup to copy backwards */
		while (n > s)
			*--b = *--n;
		*--b = '/';
		t = t->t_parent;
		if (b < buf+18) {
			*--b = '.';
			*--b = '.';
			*--b = '.';
			return b;
		}
	}
	b++;
	if (*b == '/' && b[1] == '/')
		b++;
	return b;
}

/* Initialize or reset dir and contents info from input spec
 */
fillmodes (t)
	register struct tree_node *t;
{
	char *np, *rp = 0;
	struct plist *pp, *pwlookup();
	int cf = 0, mode;
	char buf[LINESIZ];
	int saverrno;

	D(7,("fillmodes(%s: %s)",pwd(),rest));

	t->t_flags &= ~TF_HADERR;	/* line needs reprinting if more errs */

	if (rest && *rest) {
		cps(buf,rest);
		rp = buf;
	} else return;

	while (rp && *rp) {
		if (np = pindex(rp,' '))
			*np++ = '\0';
		if (*rp == '.') {
#ifdef DEBUG
			if (cf == 0) D(7,(" contents")) else D(7,(","));
#endif
			cf = 1;
			rp++;
		} else {
			cf = 0;
	/*		if (t->t_flags & TF_SETUP)
				return;
	*/
		}
		if (*rp == '~' && rp[1] == '\0') {
			D(7,(" <any owner>"));
			if (cf) {
				t->t_duid = ANYOWNER;
				t->t_dgid = ANYOWNER;
				t->t_flags |= TF_ANYOWND;
			} else {
				t->t_uid = ANYOWNER;
				t->t_gid = ANYOWNER;
				t->t_flags |= TF_ANYOWNC;
			}
		} else
		if (pp = pwlookup(rp)) {
			D(7,(" %s",rp));
			if (cf) {
				t->t_duid = pp->uid;
				t->t_dgid = pp->gid;
			} else {
				t->t_uid = pp->uid;
				t->t_gid = pp->gid;
			}
		} else {
			if (!isdigit(*rp)) {
				D(7,(" skipping over[%s]",rp));
				goto next;
			}
			saverrno = errno;
			sscanf(rp,"%o",&mode);
			errno = saverrno;
			D(7,(" %o",mode));
			if (cf) t->t_dmode = mode;
			else t->t_mode = mode;
		}
	next:
		rp = np;
	}
	t->t_flags &= ~TF_HADERR;	/* line needs reprinting if more errs */
	return;
}

struct tree_node *
getnode (fn)
	char *fn;
{
	static struct tree_node *node;
	register char *cp;
	register short i;
	char ***d, ***rdir();		/* libucsc fnc to load dir names */
	int lsrt();
	char *rn;

	i = sizeof (struct tree_node);
	node = (struct tree_node *) getmem(i,1,"dir node",fn);
	cp = (char *)node;
	i = sizeof(struct tree_node) - 1;
	while (--i != -1) *cp++ = '\0';
	if (!lflag && (!rest || *rest != '^')) { /* ^ stops dir listing */
		if (node->t_ls = rdir(rn = realname(fn))) {
			d = node->t_ls;
			if (strcmp(d[0][0],".") || strcmp(d[0][1],".."))
				X((". and .. entrys of dir %s screwed up\n",fn))
			d[0] += 2;
			qsort(d[0],d[1]-d[0],sizeof(char **),lsrt);
		} else if (errno == 0) {
			lflag = 1;
			X(("no core to load filenames for %s\n",fn));
		}
	}
	return node;
}

char *erraray[] = {
	0,
	"mode",
	"user id",
	"mode and uid",
	"group id",
	"mode and gid",
	"owner",
	"mode and owner"
};

chkdir (p, b, n)
	char *p;
	struct stat *b;
	struct tree_node *n;
{
	int m, errs = 0, fail = 0;
	char buf[LINESIZ], *bp = buf;
	
	if (n->t_flags & TF_SETUP) {
		return;
	/* 	F(("ATTEMPT TO CHECK DIR TWICE!!\n")); */
	}
/* 	n->t_flags |= TF_SETUP; */
	D(1,("chkdir lmode=0%o rmode=0%o\n",b->st_mode,n->t_mode));
	if ((m = (b->st_mode & ~S_IFMT)) != n->t_mode) {
		errs |= 1;
		sprintf(bp,"(%o->%o) ",m,n->t_mode);
		while (*bp) bp++;
	}
	if (n->t_uid != ANYOWNER) {
		if (b->st_uid != n->t_uid) {
			errs |= 2;
			sprintf(bp,"(%s->",acct(b->st_uid));
			while (*bp) bp++;
			sprintf(bp,"%s) ",acct(n->t_uid));
			while (*bp) bp++;
		}
		if (b->st_gid != n->t_gid) {
			errs |= 4;
			sprintf(bp,"(gid %d->%d) ",b->st_gid,n->t_gid);
			while (*bp) bp++;
		}
	}
	if (!errs) return;
	*--bp = '\0';

	X(("%s of dir %s is incorrect %s\n",erraray[errs],p,buf));
	fail = 0;
	if (errs & 1) fail = cexec("chmod %o %s",n->t_mode,p);
	if (!fail) {
		if (errs & 2) cexec("chown %s %s",acct(n->t_uid),p);
		if (errs & 4) cexec("chgrp %d %s",n->t_gid,p);
	}
}

char dnpath[PATHSIZ];
char *
apnd (p, f)
	char *p, *f;
{
	if (!f) return 0;
	if (p > dnpath && p[-1] != '/') {
		if (p < dnpath+sizeof(dnpath)-1)
			*p++ = '/';
	}
	while (*f && p < dnpath+sizeof(dnpath)-1)
		*p++ = *f++;
	*p = '\0';
	if (!*f) return p;
	X(("path[%15s ... %s] > %d chars\n",dnpath,p-15,sizeof(dnpath)));
	return 0;
}
		
prtrmdr(n)
	struct tree_node *n;
{
	char *p = dnpath;

	if (!n) return;
	*p = '\0';
	dumptree(n,p);
}

dumptree(n, p)
	struct tree_node *n;
	char *p;
{
	char *fn, **sp;

	if (!n || !p) return;

	if ((fn = apnd(p,n->t_name)) == 0)
		return;

	if (n->t_ls)
		for (sp=n->t_ls[0]; sp<n->t_ls[1]; sp++)
			if (*sp) {
				prtdcont(fn,sp,n->t_ls[1]);
				break;
			}
	
	if (n->t_size) dumptree(n->t_list, fn);	/* contents */
	if (n->t_next) dumptree(n->t_next, p);	/* next at this level */
}

prtdcont (fn, fl, el)
	char *fn;
	char **fl, **el;
{
	int i, j, x;
	char **sp, *isalink();
	int lsrt();
	int rem, ncol, tot, nrow, maxlen;

	qsort(fl,el-fl,sizeof(char **),lsrt);
	maxlen = 0;
	for (sp=fl; *sp && sp<el; sp++) {
		i = strlen(*sp);
		if (maxlen < i) maxlen = i;
	}
	el = sp;
	fprintf(stderr,"---- %d Files remaining in `%s':\n",el-fl,dnpath);

	for (sp=fl; sp<el; sp++) {	/* filter out understandable stuff */
		if (!*sp) continue;
		if (apnd(fn,*sp) == 0) continue; /* build anchored path name */
		if (!stat(dnpath,&sibuf)) {
			if ((sibuf.st_mode & S_IFMT) == S_IFDIR) {
				fprintf(stderr,"  %*s:  d%o %s\n", maxlen, *sp,
					sibuf.st_mode&~S_IFMT, acctname(sibuf.st_uid));
				*sp = 0;
			} else if ((rem = sibuf.st_nlink) > 1) {
				
				if (chklink(dnpath,sibuf.st_ino,sibuf.st_dev,rem)==0)
					fprintf(stderr,"  %*s:  linked to %s\n",
						maxlen, *sp, prtlinks(dnpath));
				*sp = 0;
			}
		} else {	/* file removed since rdir executed */
			*sp = 0;
		}
	}
	qsort(fl,el-fl,sizeof(char **),lsrt);
	for (sp=fl; *sp && sp<el; sp++) {
		i = strlen(*sp);
		if (maxlen < i) maxlen = i;
	}
	el = sp;
	tot = el - fl;
	ncol = 78 / ++maxlen;
	nrow = (tot + ncol - 1) / ncol;
	rem = (nrow * ncol) - tot;
	if (rem > nrow>>1) {
		ncol--;
		nrow = (tot + ncol - 1) / ncol;
		rem = (nrow * ncol) - tot;
	}
	for (i=0; i<nrow; i++) {
		fprintf(stderr,"  ");
		for (j=0; j<ncol; j++) {
			x = j * nrow + i;
			if (x >= tot) continue;
			if (j+1 >= ncol)
				fprintf(stderr,"%s",fl[x]);
			else if (nrow == 1)
				fprintf(stderr,"%s ",fl[x]);
			else
				fprintf(stderr,"%-*s",maxlen,fl[x]);
		}
		fprintf(stderr,"\n");
	}
}

lsrt (a, b)
	char **a, **b;
{
	if (!*a) {
		if (!*b) return 0;
		return 1;
	}
	if (!*b) return -1;
	return strcmp(*a, *b);
}
