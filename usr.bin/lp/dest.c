/* routines for manipulating destination structure info */

#include	"lp.h"
#include	"lpsched.h"

SCCSID("@(#)dest.c	3.1")

struct dest dest = {
	NULL, 0, NULL, 0, NULL,
	&dest, &dest,
	NULL, NULL, NULL, NULL
};

struct dest printer = {
	NULL, 0, NULL, 0, NULL, NULL, NULL,
	&printer, &printer,
	NULL, NULL
};

struct dest class = {
	NULL, 0, NULL, 0, NULL, NULL, NULL,
	&class, &class,
	NULL, NULL
};

/* newdest(name) -- returns pointer to new destination structure:
	new structure is filled in with name and is initialized with
	empty class and output lists. */

struct dest *
newdest(name)
char *name;
{
	struct dest *d;
	struct destlist *newdl();
	struct outlist *newol();
	char *s, *strcpy();

	if((s = malloc((unsigned)(strlen(name) + 1))) == NULL ||
	   (d = (struct dest *) malloc(sizeof(struct dest))) == NULL)
		fatal(CORMSG, 1);

	strcpy(s, name);
	d->d_dname = s;
	d->d_pid = 0;
	d->d_print = NULL;
	d->d_device = NULL;

	/* link to dest list */

	d->d_dnext = &dest;
	d->d_dprev = dest.d_dprev;
	(d->d_dprev)->d_dnext = d;
	dest.d_dprev = d;

	/* set up new class list */

	d->d_class = newdl();

	/* set up new output list */

	d->d_output = newol();

	return(d);
}

/* insert(list, item) -- inserts item into list, linked in the type field.
	This routine is used to build the class and printer lists.	*/

insert(list, item)
struct dest *list;
struct dest *item;
{
	item->d_tnext = list;
	item->d_tprev = list->d_tprev;
	list->d_tprev = item;
	(item->d_tprev)->d_tnext = item;
}

/* getd(name) -- returns pointer to the dest structure in the dest list
	which has the specified name.  getd returns NULL if this dest is
	not in the destination list		*/

struct dest *
getd(name)
char *name;
{
	struct dest *d;

	FORALLD(d)
		if(strcmp(d->d_dname, name) == 0)
			return(d);
	return(NULL);
}

/* getcl(name) -- returns pointer to the dest structure in the class list
	which has the specified name.  getcl returns NULL if this class is
	not in the class list		*/

struct dest *
getcl(name)
char *name;
{
	struct dest *d;

	FORALLC(d)
		if(strcmp(d->d_dname, name) == 0)
			return(d);
	return(NULL);
}

/* getp(name) -- returns pointer to the dest structure in the printer list
	which has the specified name.  getp returns NULL if this printer is
	not in the printer list		*/

struct dest *
getp(name)
char *name;
{
	struct dest *d;

	FORALLP(d)
		if(strcmp(d->d_dname, name) == 0)
			return(d);
	return(NULL);
}

/* newmem(c, p) -- places printer p in class c in destination structure */

newmem(c, p)
struct dest *c, *p;
{
	/* Insert printer in class' member list */

	insertl(p, c);

	/* Insert class in printer's class list */

	insertl(c, p);
}

dump()
{
	struct dest *d;
	struct destlist *l;
	struct outlist *o;
	char p[30];

	fprintf(stderr, "Destinations --\n");
	FORALLD(d) {
		fprintf(stderr, "%s: %c %s %s %s\n",
			d->d_dname,
			(d->d_status & D_PRINTER) ? 'p' : 'c',
			(d->d_status & D_ENABLED) ? "enabled" : "!enabled",
			(d->d_status & D_BUSY) ? "busy" : "!busy",
			d->d_device);
		for(o=(d->d_output)->ol_next; o != d->d_output; o=o->ol_next) {
			if(o->ol_print == NULL)
				sprintf(p, "not printing");
			else
				sprintf(p, "printing on %s",
					(o->ol_print)->d_dname);
			fprintf(stderr, "\t%d %s %s time:%d\n", o->ol_seqno,
				o->ol_name, p, o->ol_time);
		}
	}
	fflush(stderr);
	fprintf(stderr, "Printers --\n");
	FORALLP(d) {
		if(d->d_print == NULL)
			*p = NULL;
		else
			sprintf(p, "pid=%d, id=%s-%d",
			  d->d_pid,
			  ((d->d_print)->ol_dest)->d_dname,
			  (d->d_print)->ol_seqno);
		fprintf(stderr, "\t%s %s\n", d->d_dname, p);
		for(l=(d->d_class)->dl_next; l != d->d_class; l=l->dl_next) {
			fprintf(stderr, "\t\t%s\n", (l->dl_dest)->d_dname);
		}
	}
	fflush(stderr);
	fprintf(stderr, "Classes --\n");
	FORALLC(d) {
		fprintf(stderr, "\t%s\n", d->d_dname);
		for(l=(d->d_class)->dl_next; l != d->d_class; l=l->dl_next) {
			fprintf(stderr, "\t\t%s\n", (l->dl_dest)->d_dname);
		}
	}
	fflush(stderr);
}
