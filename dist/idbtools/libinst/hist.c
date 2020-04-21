#include "inst.h"
#include "hist.h"
#include "idb.h"
#include <sys/types.h>
#include <sys/stat.h>

long		sbrk ();
char		*getstr ();
char		*histpath ();

Hist		*hist;
short		format;
int		corrections;
char		*attrmap;
extern int	hwarning;
extern Spec	*spec;
extern long	instdate;

/* compute child allocation limit for a given n; i.e. a value >= n rounded
 * up to some clever multiple.
 */

static int
chlimit (n)
	int		n;
{
	int		lim;

	if (n == 0) return (0);
	if (n >= 32) return (((n + 31) / 32) * 32);
	for (lim = 4; lim < n; lim *= 2) ;
	return (lim);
}

/* Find space for a new child.  First attempt is a reuse of the node of the
 * same name, assuming that such node either has no children, or has children
 * and the new type is a directory.  Second attempt is the same algorithm but
 * without the name check.  Last resort is allocating more space.
 */

static H_node *
newchild (n, type, name, mset)
	H_node		*n;
	int		type;
	char		*name;
	Memset		*mset;
{
	H_node		*ch, *cc;
	int		i;

	for (ch = n->child; ch < n->chend; ++ch) {
		if (ch->type != 0) continue;
		if (strcmp (name, ch->name) == 0 &&
		    (ch->chend == ch->child || type == S_IFDIR))
			break;
	}
	if (ch >= n->chend) {
		for (ch = n->child; ch < n->chend; ++ch) {
			if (ch->type != 0) continue;
			if (ch->chend == ch->child || type == S_IFDIR)
				break;
		}
	}
	if (ch >= n->chend) {
		i = n->chend - n->child;
		if (chlimit (i + 1) > chlimit (i)) {
			n->child = (H_node *) idb_getmore (n->child,
				chlimit (i + 1) * sizeof (H_node), mset);
			n->chend = n->child + i;
			/* all children have moved; tell their children. */
			/* i.e. it's not polite to move without letting your
			   children know where to. */
			for (ch = n->child; ch < n->chend; ++ch) {
				for (cc = ch->child; cc < ch->chend; ++cc) {
					cc->parent = ch;
				}
			}
		}
		ch = n->chend++;
		ch->child = ch->chend = NULL;
	}
	/* save children, clear node, restore children */
	i = ch->chend - ch->child;
	cc = ch->child;
	bzero (ch, sizeof (H_node));
	ch->child = cc;
	ch->chend = cc + i;
	ch->parent = n;
	ch->name = "";
	return (ch);
}

static
readnode (f, parent, mset)
	int		f;
	H_node		*parent;
	Memset		*mset;
{
	register H_node	*n;
	register int	c;
	int		len, hx;
	char		*name, *prev, buff [Strsize];
	struct stat	st;

	prev = parent->name;
	while ((c = getbyte (f)) != EOF && c != H_pop) {
		n = newchild (parent, c << 8, "", mset);
		n->type = c << 8;
		switch (n->type) {
		case S_IFDIR:
		case S_IFREG:
		case S_IFBLK:
		case S_IFCHR:
		case S_IFIFO:
		case S_IFLNK:
			break;
		default:
			sprintf (buff, "bad type byte '%03o'; out of sync after %s",
				c, prev);
			hwarn (buff);
			n->type = 1;
		}
		n->hx = getshort (f);
		if (n->hx < 0 || n->hx >= hist->nhandle) {
			sprintf (buff, "bogus handle refence 0x'%04x' after %s",
				n->hx, prev);
			hwarn (buff);
			n->type = 0;
			return (-1);
		}
		prev = n->name = getstr (f, mset);
		if (strlen (n->name) > 1000) {
			hwarn ("excessive name length");
			n->type = 0;
			return (-1);
		}
		if (n->type == 1) {
			if (lstat (idb_rpath (histpath (n)), &st) < 0) {
				n->type = 0;
				return (-1);
			}
			n->type = st.st_mode & S_IFMT;
			sprintf (buff, "Assuming type '%c' for %s\n",
				idb_typec (n->type), histpath (n));
			hwarn (buff);
		}
		n->chksum = getshort (f);
		n->nattr = getshort (f);
		if (vread (f, n->attr, sizeof (n->attr)) != sizeof (n->attr)) {
			n->type = 0;
			return (-1);
		}
		if (n->type == S_IFDIR) {
			if (readnode (f, n, mset) < 0) return (-1);
		}
	}
	if (c != H_pop) {
		fprintf (stderr, "unexpected EOF in versions\n");
		return (-1);
	} else {
		return (0);
	}
}

Hist *
histread (fname, tree, corr, mset)
	char		*fname;
	int		tree;
	int		corr;
	Memset		*mset;
{
	int		f;
	char		*p, pname [Strsize], iname [Strsize], sname [Strsize];
	Prod		*pr;
	Image		*im;
	Subsys		*ss;
	int		i, j, n;

	if ((f = vopen (fname, 0)) < 0) {
		return (NULL);
	}
	hist = histnew (mset);
	format = getshort (f);
	if (format < 4) return (hist);
	corrections = corr;
	hist->nhandle = getshort (f);
	hist->handle = (Handle *) idb_getmem (hist->nhandle * sizeof (Handle),
		mset);
	for (i = 0; i < hist->nhandle; ++i) {
		hist->handle [i].name = getstr (f, mset);
		hist->handle [i].pr = NULL;
		hist->handle [i].im = NULL;
		hist->handle [i].ss = NULL;
	}
	hist->nattr = getshort (f);
	hist->attr = (char **) idb_getmem (hist->nattr * sizeof (char *), mset);
	for (i = 0; i < hist->nattr; ++i) {
		hist->attr [i] = getstr (f, mset);
	}
	hist->spec = (Spec *) idb_getmem (sizeof (Spec), mset);
	n = getshort (f);
	if (n == 0) {
		hist->spec->prod = NULL;
	} else {
		hist->spec->prod = (Prod *)
			idb_getmem (n * sizeof (Prod), mset);
	}
	hist->spec->prodend = hist->spec->prod + n;
	for (pr = hist->spec->prod; pr < hist->spec->prodend; ++pr) {
		if (readprod (f, pr, mset) < 0) break;
	}
	sethandles (hist);
	hist->format = Hformat;
	if (tree && readnode (f, hist->root, mset) < 0) {
#ifdef notdef
		idb_freeset (mset);
		hist = histnew (mset);
#endif
		; /* salvage whatever we can */
	}
	vclose (f);
	return (hist);
}

hwarn (msg)
	char		*msg;
{
	if (!hwarning) return;
	fprintf (stderr, "versions read warning: %s\n", msg);
}

Hist *
histnew (mset)
	Memset		*mset;
{
	Hist		*h;

	idb_freeset (mset);
	h = (Hist *) idb_getmem (sizeof (Hist), mset);
	bzero (h, sizeof (*h));
	h->format = Hformat;
	h->nattr = 1;
	h->attr = (char **) idb_getmem (sizeof (char *), mset);
	h->attr [0] = idb_stash ("?", mset);		/* bogus attribute */
	h->root = (H_node *) idb_getmem (sizeof (H_node), mset);
	bzero (h->root, sizeof (*h->root));
	h->root->parent = NULL;
	h->root->name = idb_stash (".", mset);
	h->spec = (Spec *) idb_getmem (sizeof (Spec), mset);
	h->spec->prod = h->spec->prodend = NULL;
	return (h);
}

void
histaddattr (h, attr, mset)
	Hist		*h;
	char		*attr;
	Memset		*mset;
{
	int		i, free;

	for (free = -1, i = 0; i < h->nattr; ++i) {
		if (strcmp (h->attr [i], attr) == 0) return;
		if (free < 0 && h->attr [i][0] == '\0') free = i;
	}
	if (free < 0) {
		h->attr = (char **) idb_getmore (h->attr,
			++h->nattr * sizeof (char *), mset);
	} else {
		i = free;
	}
	h->attr [i] = idb_stash (attr, mset);
}

void
histdelattr (h, attr)
	Hist		*h;
	char		*attr;
{
	int		i;

	for (i = 1; i < h->nattr; ++i) {
		if (strcmp (h->attr [i], attr) == 0) {
			h->attr [i] = "";
		}
	}
}

static int
cmp (f1, f2)
	H_node		*f1;
	H_node		*f2;
{
	return (strcmp (f1->name, f2->name));
}

static void
writenode (f, n)
	int		f;
	H_node		*n;
{
	H_node		*ch;
	int		i;

	if (n->chend - n->child > 1)
		qsort (n->child, n->chend - n->child, sizeof (H_node), cmp);
	for (ch = n->child; ch < n->chend; ++ch) {
		if (ch->type == 0) continue;
		putbyte (f, ch->type >> 8);
		putshort (f, ch->hx);
		putstr (f, ch->name);
		putshort (f, ch->chksum);
		putshort (f, ch->nattr);
		for (i = 0; i < ch->nattr; ++i) {
			ch->attr [i] = attrmap [ch->attr [i]];
		}
		vwrite (f, ch->attr, sizeof (ch->attr));
		if (ch->type == S_IFDIR) writenode (f, ch);
	}
	putbyte (f, H_pop);
}

static int
cmpprod (pr1, pr2)
	Prod		*pr1;
	Prod		*pr2;
{
	return (strcmp (pr1->name, pr2->name));
}

int
histwrite (h, fname)
	Hist		*h;
	char		*fname;
{
	int		f;
	int		i, n;
	Prod		*pr;

	if ((f = vopen (fname, 1)) == NULL) {
		perror (fname); return (-1);
	}
	putshort (f, Hformat);
	putshort (f, h->nhandle);
	for (i = 0; i < h->nhandle; ++i) {
		putstr (f, h->handle [i].name);
	}
	attrmap = idb_getmem (h->nattr, NULL);
	for (i = n = 0; i < h->nattr; ++i) {
		if (h->attr [i][0] != '\0') {
			attrmap [i] = n;
			++n;
		} else {
			attrmap [i] = 0;
		}
	}
	idb_free (attrmap);
	putshort (f, n);
	for (i = 0; i < h->nattr; ++i)
		if (h->attr [i][0] != '\0') putstr (f, h->attr [i]);
	putshort (f, h->spec->prodend - h->spec->prod);
	qsort (h->spec->prod, h->spec->prodend - h->spec->prod,
		sizeof (*h->spec->prod), cmpprod);
	for (pr = h->spec->prod; pr < h->spec->prodend; ++pr) {
		if (writeprod (f, pr) < 0) break;
	}
	writenode (f, h->root);
	vclose (f);
	sethandles (h); /* since everything was moved in the sort */
	return (0);
}

H_node *
histnode (h, path)
	Hist		*h;
	char		*path;
{
	Memset		*mset;
	char		*argv [256];
	int		i, argc;
	H_node		*n, *ch;

	mset = idb_newset ();
	if (strcmp (path, ".") == 0) path = "";
	argc = breakpath (path, argv, mset);
	n = h->root;
	for (i = 0; i < argc; ++i) {
		for (ch = n->child; ch < n->chend; ++ch) {
			if (ch->type == 0) continue;
			if (strcmp (ch->name, argv [i]) == 0) break;
		}
		if (ch >= n->chend) {
			ch = NULL;
			break;
		}
		n = ch;
	}
	idb_dispose (mset);
	return (ch);
}

char *
histpath (n)
	H_node		*n;
{
	static char	path [Strsize];
	char		*name [256];
	int		nname;
	register char	*p, *s;

	for (nname = 0; n->parent != NULL; n = n->parent) {
		name [nname++] = n->name;
	}
	for (p = path; nname > 0; ) {
		s = name [--nname];
		while (*p = *s++) ++p;
		if (nname > 0) *p++ = '/';
	}
	*p = '\0';
	return (path);
}

H_node *
histadd (h, rec, pr, im, ss, mset)
	Hist		*h;
	Rec		*rec;
	Prod		*pr;
	Image		*im;
	Subsys		*ss;
	Memset		*mset;
{
	Memset		*tmpset;
	int		i, j, argc;
	H_node		*ch, *n;
	char		buff [1024], *argv [256];
	Attr		*at;

	tmpset = idb_newset ();
	argc = breakpath (rec->dstpath, argv, tmpset);
	n = h->root;
	for (i = 0; i < argc - 1; ++i) {
		for (ch = n->child; ch < n->chend; ++ch) {
			if (ch->type == 0) continue;
			if (strcmp (ch->name, argv [i]) == 0) break;
		}
		if (ch >= n->chend) {
			ch = newchild (n, S_IFDIR, argv [i], mset);
			ch->type = S_IFDIR;
			ch->name = idb_stash (argv [i], mset);
		}
		n = ch;
	}
	for (ch = n->child; ch < n->chend; ++ch) {
		if (ch->type == 0) continue;
		if (strcmp (ch->name, argv [argc - 1]) == 0) break;
	}
	if (ch >= n->chend) {
		ch = newchild (n, rec->type, argv [argc - 1], mset);
		ch->name = idb_stash (argv [argc - 1], mset);
	}
	ch->type = rec->type;
	ch->hx = findhandle (h, pr->name, im->name, ss->name);
	ch->chksum = idb_intat (rec, "sum");
	for (at = rec->attr; at < rec->attr + rec->nattr; ++at) {
		for (i = 0; i < h->nattr; ++i) {
			if (strcmp (at->atname, h->attr [i]) == 0) break;
		}
		if (i >= h->nattr) continue;
		for (j = 0; j < ch->nattr; ++j) {
			if (ch->attr [j] == i) break;
		}
		if (j >= ch->nattr && ch->nattr < sizeof (ch->attr)) {
			ch->attr [ch->nattr++] = i;
		}
	}
	idb_dispose (tmpset);
	return (ch);
}

findhandle (h, pname, iname, sname)
	Hist		*h;
	char		*pname;
	char		*iname;
	char		*sname;
{
	int		i;
	char		*name;

	name = cat (pname, iname, sname);
	for (i = 0; i < h->nhandle; ++i) {
		if (strcmp (h->handle [i].name, name) == 0) return (i);
	}
	return (-1);
}

sethandles (h)
	Hist		*h;
{
	int		i;
	char		pname [Strsize], iname [Strsize], sname [Strsize];

	for (i = 0; i < h->nhandle; ++i) {
		uncat (h->handle [i].name, pname, iname, sname);
		if (locate (pname, iname, sname, h->spec, &h->handle [i].pr,
		    &h->handle [i].im, &h->handle [i].ss, 0) < 0) {
#ifdef DEBUGFUNCS
			fprintf (stderr, "Can't find product %s\n",
				h->handle [i].name);
#endif
			h->handle [i].name = "unknown.unknown.unknown";
		}
	}
}

addhandle (h, pr, im, ss, mset)
	Hist		*h;
	Prod		*pr;
	Image		*im;
	Subsys		*ss;
	Memset		*mset;
{
	int		i, free;
	char		*name;

	name = cat (pr->name, im->name, ss->name);
	free = -1;
	for (i = 0; i < h->nhandle; ++i) {
		if (strcmp (h->handle [i].name, name) == 0) {
			h->handle [i].pr = pr;
			h->handle [i].im = im;
			h->handle [i].ss = ss;
			return;
		}
		if (h->handle [i].name [0] == '\0') free = i;
	}
	if (free == -1) {
		h->handle = (Handle *) idb_getmore (h->handle,
			(h->nhandle + 1) * sizeof (Handle), mset);
		free = h->nhandle++;
	}
	h->handle [free].name = idb_stash (name, mset);
	h->handle [free].pr = pr;
	h->handle [free].im = im;
	h->handle [free].ss = ss;
}

delhandle (h, pr, im, ss)
	Hist		*h;
	Prod		*pr;
	Image		*im;
	Subsys		*ss;
{
	int		i;
	char		*pat;

	pat = cat (pr->name, im ? im->name : "*", ss ? ss->name : "*");
	for (i = 0; i < h->nhandle; ++i) {
		if (filematch (h->handle [i].name, pat)) {
			h->handle [i].name [0] = '\0';
		}
	}
}

histaddss (h, pr, im, ss, mset)
	Hist		*h;
	Prod		*pr;
	Image		*im;
	Subsys		*ss;
	Memset		*mset;
{
	Prod		*prx;
	Image		*imx;
	Subsys		*ssx;
	int		hit, n;

	setscan (pr->name, im->name, ss->name, h->spec);
	while (hit = nextscan (&prx, &imx, &ssx)) {
		if (imx->version != im->version) continue;
		break;
	}
	if (prx == NULL) {
		n = h->spec->prodend - h->spec->prod + 1;
		h->spec->prod = (Prod *) idb_getmore (h->spec->prod,
			n * sizeof (Prod), mset);
		h->spec->prodend = h->spec->prod + n;
		prx = h->spec->prodend - 1;
		prx->name = idb_stash (pr->name, mset);
		prx->image = prx->imgend = imx = NULL;
	}
	prx->id = idb_stash (pr->id, mset);
	prx->flags = pr->flags & ~(Deleted | Select);
	if (imx == NULL) {
		n = prx->imgend - prx->image + 1;
		prx->image = (Image *) idb_getmore (prx->image,
			n * sizeof (Image), mset);
		prx->imgend = prx->image + n;
		imx = prx->imgend - 1;
		imx->subsys = imx->subend = ssx = NULL;
	}
	imx->name = idb_stash (im->name, mset);
	imx->flags = im->flags & ~(Deleted | Select);
	imx->id = idb_stash (im->id, mset);
	imx->format = im->format;
	imx->order = im->order;
	imx->version = im->version;
	imx->length = im->length;
	imx->padsize = im->padsize;
	if (ssx == NULL) {
		n = imx->subend - imx->subsys + 1;
		imx->subsys = (Subsys *) idb_getmore (imx->subsys,
			n * sizeof (Subsys), mset);
		imx->subend = imx->subsys + n;
		ssx = imx->subend - 1;
		ssx->name = idb_stash (ss->name, mset);
		ssx->rep = ssx->repend = NULL;
		ssx->prereq = ssx->preend = NULL;
	}
	ssx->flags = ss->flags & ~(Deleted | Select);
	ssx->id = idb_stash (ss->id, mset);
	ssx->exp = idb_stash ("", mset);
	ssx->instdate = instdate;
	addhandle (h, pr, im, ss, mset);
	sethandles (h);	/* things may have moved */
}

histdelss (h, pr, im, ss)
	Hist		*h;
	Prod		*pr;
	Image		*im;
	Subsys		*ss;
{
	int		nss, nim;
	Prod		*p;
	Image		*i;
	Subsys		*s;

	for (p = h->spec->prod; p < h->spec->prodend; ++p) {
		if (p->flags & Deleted) continue;
		if (strcmp (p->name, pr->name) != 0) continue;
		nim = 0;
		for (i = p->image; i < p->imgend; ++i) {
			if (i->flags & Deleted) continue;
			++nim;
			if (strcmp (i->name, im->name) != 0) continue;
			nss = 0;
			for (s = i->subsys; s < i->subend; ++s) {
				if (s->flags & Deleted) continue;
				++nss;
				if (strcmp (s->name, ss->name) == 0) {
					s->flags |= Deleted;
					--nss;
				}
			}
			if (nss == 0) {
				i->flags |= Deleted;
				--nim;
			}
		}
		if (nim == 0) {
			p->flags |= Deleted;
		} 
	}
}
