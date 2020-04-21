/*
 * idb_scn  -  scan an idb file from stdin and then print selected
 *		fields on stdout.
 *
 * Usage:  idb_scn options [ tag ]
 *
 * Options:	-s		print source paths in the idb
 *		-d		print dest paths in the idb
 *		-di		same as -d, but ignore {-,+,@} endings
 *				  in the dest path.
 *		-type		print file type (d, f, etc.)
 *		-nh		no header
 *		-rbase name	name is the location of a destination tree.
 *		-sbase name	name is location of a "built" source tree.
 *		-e		test if source or dest idb file exists
 *				  in sbase or rbase tree.
 *		-m		print filename if actual file permissions
 *				  don't match those in the idb.  (-m ==> -e.)
 *		-t n		print filename if file (in rbase or sbase)
 *				  has not been modified in the last n days. *				  (-t ==> -e.)
 *
 * tag will cause only those idb records containing tag to be examined.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#ifdef DEBUG
#define REGISTER
#else
#define REGISTER register
#endif

char *cp;			/* ptr to current pos in input line */
char line[500];			/* an input line */
char ftype[25], spath[200], dpath[200], attr[200], full_path[300];
char *epath;			/* which path to test for existence */
char tag[25];			/* idb tag */
char *sbase = (char *)NULL;	/* pathname of "built" source tree */
char *rbase = (char *)NULL;	/* pathname of dest tree */
int bad_options, print_src, print_dest, ign_ending, print_typ, base_len,
	test_exist, test_mode, test_time, ntime, file_mode, no_header;
struct stat statbuf;
char *ending = (char *)NULL;	/* either "-", "+", "@", or null string */


main(argc, argv)
int argc;
char *argv[];
{
	char *argp;
	int print_name;
	long cur_clock = time(0L);
	long tim_limit;

	while (--argc > 0) {
		argp = *++argv;
		if (*argp == '-') {
			++argp;
			if (strcmp(argp, "sbase") == 0) {
				if (--argc > 0)
					sbase = *++argv;
				else {
					fprintf(stderr, "pathname \
required after -sbase option\n");
					exit (1);
				}
			} else if (strcmp(argp, "rbase") == 0) {
				if (--argc > 0)
					rbase = *++argv;
				else {
					fprintf(stderr, "pathname \
required after -rbase option\n");
					exit (1);
				}
			} else if (strcmp(argp, "di") == 0) {
					print_dest++;
					ign_ending++;
			} else if (strcmp(argp, "type") == 0) {
					print_typ++;
			} else if (strcmp(argp, "nh") == 0) {
					no_header++;
			} else if (argp[1] != '\0')
				bad_options++;
			else {
			    /* remaining options are single chars */
			    switch (*argp) {
				case 's':
					print_src++;
					break;
				case 'd':
					print_dest++;
					break;
				case 'e':
					test_exist++;
					break;
				case 'm':
					test_exist++;
					test_mode++;
					break;
				case 't':
					test_exist++;
					test_time++;
					if (--argc > 0) {
					    ntime = atoi(*++argv);
					    tim_limit = ntime * 24 * 3600L;
					} else {
					    fprintf(stderr, "n \
required after -t option\n");
					    exit (1);
					}
					break;
				default:
					bad_options++;
					break;
			    }
			}
		} else {	/* found an arg, not an option */
			if (tag[0] != '\0') {
				fprintf(stderr, "Only 1 tag allowed\n");
				exit (1);
			}
			strcpy(tag, argp);
		}
	}

	if (bad_options || !(print_src || print_dest)) {
		fprintf(stderr, "invalid options\n");
		exit (1);
	}
	if (test_exist) {
		if (print_dest) {
			if (rbase == (char *)NULL) {
				fprintf(stderr, "rbase required\n");
				exit (1);
			}
			if (stat(rbase, &statbuf) < 0) {
				fprintf(stderr, "can't stat rbase\n");
				exit (1);
			}
			if ( !(statbuf.st_mode & S_IFDIR)) {
				fprintf(stderr, "rbase is not a directory\n");
				exit (1);
			}
			strcpy(full_path, rbase);
			epath = dpath;
		} else if (print_src) {
			if (sbase == (char *)NULL) {
				fprintf(stderr, "sbase required\n");
				exit (1);
			}
			if (stat(sbase, &statbuf) < 0) {
				fprintf(stderr, "can't stat sbase\n");
				exit (1);
			}
			if ( !(statbuf.st_mode & S_IFDIR)) {
				fprintf(stderr, "sbase is not a directory\n");
				exit (1);
			}
			strcpy(full_path, sbase);
			epath = spath;
		} else {
			fprintf(stderr, "invalid options\n");
			exit (1);
		}
	} else {			/* if (!test_exist) */
		if (rbase || sbase) {
			fprintf(stderr, "rbase or sbase is useless \
without the -e option\n");
			exit (1);
		}
	}

	if (!no_header) {
	    printf("idb %s filenames", print_dest ? "dest" : "source");
	    if (tag[0] != '\0')
		    printf(" for tag %s", tag);
	    putchar('\n');
	    if (test_exist)
		    printf("  which do not exist in the tree at %s\n",
			    full_path);
	    if (test_mode)
		    printf("  or which have incorrect file permissions\n");
	    if (test_time)
		    printf("  or which are more than %d day%s old\n",
			    ntime, ((ntime > 1) ? "s" : ""));
	    printf("---------\n");
	}

	strcat(full_path, "/");
	base_len = strlen(full_path);

	while (fgets(line, 200, stdin) != (char *)NULL) {
		cp = line;

		if (print_typ) {		/* file mode */
			(void) copy_field(ftype);
			print_typ++;
		} else
			skip_field();

		if (test_mode) {		/* file mode */
			(void)copy_field(attr);
			sscanf(attr, "%o", &file_mode);
		} else
			skip_field();
		skip_field();			/* skip owner */
		skip_field();			/* skip group */

		print_name = 0;
		ending = (char *)NULL;
		if (print_dest) {		/* dest path */
			(void) copy_field(dpath);
			print_name++;
		} else
			skip_field();

		if (print_src) {		/* src path */
			(void) copy_field(spath);
			if (!print_dest && (strcmp(spath, "nosource") != 0)
			 && (strcmp(spath, "nullfile") != 0))
				print_name++;
		} else
			skip_field();

		if (print_name) {

			/* look at attributes */
			if (tag[0] != '\0')
				print_name = 0;
			while (copy_field(attr)) {
				if ((strcmp(attr, "2000") == 0)
				  ||(strcmp(attr, "1000") == 0)) {
					print_name = 0;
					break;
				}
				if ((tag[0] != '\0')
				 && (strcmp(attr, tag) == 0))
					print_name++;

				if (print_dest && !ign_ending) {
					if (strcmp(attr, "config") == 0)
						ending = "-";
					else if (strcmp(attr, "shared") == 0)
						ending = "+";
					else if (strcmp(attr, "after") == 0)
						ending = "@";
				}
			}
		}

		if (print_name) {
			if (ending != (char *)NULL)
				strcat(dpath, ending);
			/*
			 * Note:  ending == 0 implies
				(ign_ending == 0 && print_dest).
			 */
			if (test_exist) {
				strcpy(full_path + base_len, epath);
				if (stat(full_path, &statbuf) == 0) {
					print_name = 0;
					if (test_mode &&
				((statbuf.st_mode & 07777) != file_mode))
						print_name++;
					if (test_time &&
				(cur_clock - statbuf.st_mtime > tim_limit))
						print_name++;
				}
			}
		}

		if (print_name) {
			if (print_typ) {
				fputs(ftype, stdout);
				putchar(' ');
			}
			if (dpath[0] != '\0') {
				fputs(dpath, stdout);
				if (spath[0] != '\0') {
					int len = strlen(dpath);
					int num_tabs = (len < 40) ?
						((39 - len) / 8 + 1) : 0;
					int i;

					for (i = 0; i < num_tabs; i++)
						putchar('\t');
					fputs(" <- ", stdout);
					fputs(spath, stdout);
				}
			} else if (spath[0] != '\0')
				fputs(spath, stdout);

			putchar('\n');
		}
	}
}


skip_field()
{
	REGISTER int in_paren = 0;

	/* first skip extraneous white space */
	while (*cp != '\0' && *cp != '\n' && (*cp == ' ' || *cp == '\t'))
		cp++;

	/* skip the current field */

	while (*cp != '\0' && *cp != '\n') {
		if (*cp == '(')
			in_paren++;
		else if (*cp == ')')
			in_paren = 0;
		if (!in_paren && (*cp == ' ' || *cp == '\t'))
			break;
		cp++;
	}

	/* go to beginning of next field */
	while (*cp != '\0' && *cp != '\n' && (*cp == ' ' || *cp == '\t'))
		cp++;

	return;
}


copy_field(dest)
  REGISTER char *dest;
{
	REGISTER int in_paren = 0;
	char *dest_sav = dest;

	/* first skip extraneous white space */
	while (*cp != '\0' && *cp != '\n' && (*cp == ' ' || *cp == '\t'))
		cp++;

	/* copy field to dest */
	while (*cp != '\0' && *cp != '\n') {
		if (*cp == '(')
			in_paren++;
		else if (*cp == ')')
			in_paren = 0;
		if (!in_paren && (*cp == ' ' || *cp == '\t'))
			break;
		*dest++ = *cp++;
	}
	*dest = '\0';				/* make a valid string */

	/* now go to beginning of next field */
	while (*cp != '\0' && *cp != '\n' && (*cp == ' ' || *cp == '\t'))
		cp++;

	return (*dest_sav != '\0');	/* return true if dest_sav
					 *	is non-nil
					 */
}
