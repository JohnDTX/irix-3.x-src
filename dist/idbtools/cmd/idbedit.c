/* idbedit [ -i idb ] [ -e selexp ] fieldname ...
 *
 * By default, all records are selected.  The "-e" argument can be used to
 * supply the expression describing the records to be selected.  Following
 * the command line options is a list of field names to be edited.  stdin
 * is expected to contain lines of n+1 fields, where the first is the
 * dstpath (upon which the idb must be sorted), and the remainder are
 * the new values of the named fields, followed by a list of zero or more
 * attribute values to be replaced in the record.  The attributes may
 * be followed by parenthesis.  The new idb is written on stdout, with
 * unselected lines passed through as-is.  A "join" is effectively done
 * between the selected records and the input stream, thus some records
 * in either may skipped.
 */

#include "idb.h"

#define		LPAR		'('
#define		RPAR		')'

#define		Dstpath		1
#define		Srcpath		2
#define		Type		3
#define		Mode		4
#define		User		5
#define		Group		6

extern char	*optarg;
extern int	optind;

char		*nextifield ();
Attr		*iattr ();

char		selexpfile [1024] = "";
char		*selexp = "1";
int		more = 1;
char		ibuff [1024];
char		*ip;
char		fieldbuff [1024];
int		unget = 0;
char		idest [1024];

struct {
	char		*name;
	int		field;
} nametable [] = {
	{ "dstpath",	Dstpath },
	{ "srcpath",	Srcpath },
	{ "type",	Type },
	{ "mode",	Mode },
	{ "user",	User },
	{ "group",	Group },
};
int		nnames = (sizeof (nametable) / sizeof (*nametable));

int		field [128];
int		nfields;

main (argc, argv)
	int		argc;
	char		*argv [];
{
	char		*p, buff [1024];
	int		i, t;
	Node		*sel;
	Memset		*pset, *xset;
	FILE		*g;
	Rec		*rec;

	while ((t = getopt (argc, argv, "r:s:i:f:e:")) != EOF) {
		switch (t) {
		case 'r': strcpy (rbase, optarg); break;
		case 's': strcpy (sbase, optarg); break;
		case 'i': strcpy (idb, optarg); break;
		case 'f': strcpy (selexpfile, optarg); break;
		case 'e': strcpy (selexp, optarg); break;
		}
	}
	idb_setbase ();
	nfields = 0;
	while (optind < argc) {
		for (i = 0; i < nnames; ++i) {
			if (strcmp (argv [optind], nametable [i].name) == 0) {
				field [nfields++] = nametable [i].field;
				break;
			}
		}
		if (i >= nnames) {
			fprintf (stderr, "Unrecognized field name '%s'\n",
				argv [optind]);
			exit (1);
		}
		if (nfields >= sizeof (field) / sizeof (*field)) {
			fprintf (stderr, "Too many args; ignoring some.\n");
			break;
		}
		++optind;
	}
	pset = idb_newset ();
	xset = idb_newset ();
	if ((g = fopen (idb, "r")) == NULL) {
		perror (idb); exit (1);
	}
	if (*selexpfile != '\0') {
		if ((sel = idb_parsef (selexpfile, pset, Bool)) == NULL) {
			perror (selexpfile); exit (1);
		}
	} else {
		if ((sel = idb_parses (selexp, pset, Bool)) == NULL) {
			fprintf (stderr, "Can't parse '%s'\n", selexp);
			exit (1);
		}
	}
	nextirec ();
	while ((rec = idb_read (g, xset)) != NULL) {
		if (!idb_expr (rec, sel, xset)) continue;
		if (more) {
			while (more && (t = strcmp (idest, rec->dstpath)) < 0)
				nextirec ();
			if (more && t == 0) makechanges (rec, xset);
		}
		idb_write (stdout, rec);
	}
	idb_freeset (pset);
	exit (0);
}

makechanges (rec, set)
	Rec		*rec;
	Memset		*set;
{
	int		i;
	char		*p;
	Attr		*at;

	for (i = 0; i < nfields; ++i) {
		switch (field [i]) {
		case Dstpath:
			rec->dstpath = idb_stash (nextifield (), set);
			break;
		case Srcpath:
			rec->srcpath = idb_stash (nextifield (), set);
			break;
		case Type:
			rec->type = idb_typei (*nextifield ());
			break;
		case Mode:
			rec->mode = (unsigned short) strtol (nextifield ());
			break;
		case User:
			rec->user = idb_stash (nextifield (), set);
			break;
		case Group:
			rec->group = idb_stash (nextifield (), set);
			break;
		}
	}
	p = nextifield ();
	while (*p != '\0') {
		at = idb_addattr (rec, p, 0, NULL, set);
		if (*(p = nextifield ()) == LPAR) {
			while (*(p = nextifield ()) != RPAR && *p != '\0') {
				idb_addarg (at, p, set);
			}
			p = nextifield ();
		}
	}
}

nextirec ()
{
	if (!more) return;
	if (fgets (ibuff, sizeof (ibuff), stdin) == NULL) {
		more = 0;
		return;
	}
	ip = ibuff;
	strcpy (idest, nextifield ());
}

char *
nextifield ()
{
	char		*p;

	if (unget) { unget = 0; return (fieldbuff); }
	while (*ip == ' ' || *ip == '\t' || *ip == '\n') ++ip;
	if (*ip == '\0') return ("");
	if (*ip == LPAR) {
		++ip; return ("(");
	} else if (*ip == RPAR) {
		++ip; return (")");
	} else {
		for (p = fieldbuff; *ip != ' ' && *ip != '\t' && *ip != '\n' &&
		    *ip != LPAR && *ip != RPAR && *ip != '\0'; ++ip) {
			*p++ = *ip;
		}
		*p = '\0';
	}
	return (fieldbuff);
}

ungetifield (s)
	char		*s;
{
	if (s != fieldbuff) strcpy (fieldbuff, s);
	unget = 1;
}
