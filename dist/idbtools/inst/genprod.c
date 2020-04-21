#include "inst.h"
#include "idb.h"
#include <sys/types.h>
#include <sys/stat.h>

extern Spec	*parsespec ();

extern int	optind;
extern char	*optarg;

char		obase [1024] = ".";

main (argc, argv)
	int		argc;
	char		*argv [];
{
	int		c, e, f;
	Spec		*spec;
	Prod		*pr;
	Image		*im;
	Subsys		*ss;
	struct stat	st;
	char		buff [Strsize];

	while ((c = getopt (argc, argv, "r:s:i:o:")) != EOF) {
		switch (c) {
		case 'r': strcpy (rbase, optarg); break;
		case 's': strcpy (sbase, optarg); break;
		case 'i': strcpy (idb, optarg); break;
		case 'o': strcpy (obase, optarg); break;
		}
	}
	if (argc - optind != 1) usage (argv [0]);
	if ((spec = parsespec (argv [optind], NULL)) == NULL) {
		fprintf (stderr, "%s: couldn't parse\n", argv [optind]);
		exit (1);
	}
	e = 0;
	for (pr = spec->prod; pr < spec->prodend; ++pr) {
		sprintf (buff, "%s/%s", obase, pr->name);
		if ((f = vcreat (buff, 0644)) < 0) {
			perror (buff);
			continue;
		}
		for (im = pr->image; im < pr->imgend; ++im) {
			sprintf (buff, "%s/%s.%s", obase, pr->name, im->name);
			if (stat (buff, &st) < 0) {
				fprintf (stderr, "%s: can't get length\n",
					buff);
				im->length = 0;
			} else {
				im->length = st.st_size;
			}
			for (ss = im->subsys; ss < im->subend; ++ss) {
				if (strlen (ss->exp) == 0) {
					sprintf (buff, "%s.%s.%s", pr->name,
						im->name, ss->name);
					ss->exp = idb_stash (buff, NULL);
				}
			}
		}
		if (writeprod (f, pr) < 0) {
			fprintf (stderr, "%s: couldn't write product file\n",
				pr->name);
			++e;
		}
		vclose (f);
	}
	exit (e);
}

usage (name)
	char		*name;
{
	fprintf (stderr, "usage: %s [ -r rbase ] [ -s sbase ] [ -i ibase ] ",
		name);
	fprintf (stderr, "specfile\n");
	exit (1);
}
