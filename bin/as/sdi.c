#include "mical.h"

static char *sdi = "~|^`s.sdi.c R1.7 on 6/27/80";

/*
 * module to handle span-dependent instructions, e.g. jbr
 * see CACM Vol 21 No 4 April 1978 for a description of the
 * algorithm for resolving sdi's between the 1st and 2nd pass
 */


struct sdi {	/* information for span dependent instructions (sdi's) */
	struct sdi *sdi_next;           /* next sdi in list */
	struct csect *sdi_csect;	/* csect of the sdi */
	long int sdi_loc;		/* location of the sdi */
	struct sym_bkt *sdi_sym;	/* symbol part of the sdi address */
	long int sdi_off;		/* offset part of the sdi address */
	int sdi_gleng;			/* length of the most general form */
	int sdi_leng;			/* actual length of the sdi */
	long int sdi_span;		/* span of  the sdi */
	long int sdi_base;		/* origin of the span of the sdi */
	struct blist *sdi_bounds;	/* length & bounds info for forms of sdi */
} *sdi_list = 0;	/* linked list of sdi's descriptors */

struct blist {	/* length and bounds information for various forms of sdi's */
	struct blist *b_next;	/* next element in list */
	int b_length;		/* length of this form of the sdi */
	long int b_lbound;	/* lower and upper bound on the span */
	long int b_ubound;	/* for this form of the sdi */
};

/*
 * routine to create a sdi descriptor and insert it into the list
 */
makesdi(op, gleng, base, bounds)
struct oper *op;	/* the operand of the sdi */
int gleng;		/* the length of the most general form of the sdi */
long int base;		/* origin of the the initial span of the sdi, to be */
			/* corrected between pass1 and pass2 by subtracting */
			/* from the value of the symbol of the operand */
			/* i.e. 0 for resolving short absolute address modes */
			/* Dot[+increment] for PC relative addresses */
struct blist *bounds;	/* list of lengths & bounds for the sdi */
{
	register struct sdi *s, **p;
	extern struct csect *Cur_csect;

	if (op->flags_o&O_COMPLEX) {	/* not a simple address */
		Sys_Error("makesdi: flags_o & O_COMPLEX: %x\n", (char *) op->flags_o);
		/*NOTREACHED*/
	}
	if (op->sym_o == 0) {		/* absolute address */
		Sys_Error("makesdi: absolute address; line no=%d\n", Line_no);
		/*NOTREACHED*/
	}
	if ((s = (struct sdi *)calloc(1,sizeof *s)) == NULL)
		Sys_Error("sdi storage exceeded\n", (char *) NULL);
	s->sdi_loc = Dot;
	s->sdi_csect = Cur_csect;
	s->sdi_sym = op->sym_o;
	s->sdi_off = op->value_o;
	s->sdi_gleng = gleng;
	s->sdi_base = base;
	s->sdi_span = - base;
	s->sdi_bounds = bounds;
	s->sdi_leng = bounds->b_length;	/* shortest length */
	for (p = &sdi_list; *p; p = &(*p)->sdi_next)
		if (s->sdi_loc < (*p)->sdi_loc)
			break;
	s->sdi_next = *p;
	*p = s;
	return(s->sdi_leng);	/* return the current length */
}

/*
 * insert a new blist element into a blist
 * The blist is sorted by increasing b_length
 */
struct blist *sdi_bound(leng, lbound, ubound, next)
int leng;		/* length of this form of the sdi */
long int lbound;	/* lower bound of span */
long int ubound;	/* upper bound */
struct blist *next;	/* target blist */
{
	register struct blist *b, **p;
	static bct;

	if ((b = (struct blist *)calloc(1,sizeof *b)) == NULL)
		Sys_Error("sdi bound list storage exceeded: %d bounds\n", bct);
	bct++;
	b->b_length = leng;
	b->b_lbound = lbound;
	b->b_ubound = ubound;
	for (p = &next; *p; p = &(*p)->b_next)
		if (leng <= (*p)->b_length)
			break;
	b->b_next = *p;
	*p = b;
	return(*p);
}

/*
 * resolve sdi's between pass1 and pass2
 * basic algorithm is to repeatedly look for sdi that must use the
 * long form, and update the span of other sdi's.
 * When this terminates, all remaining sdi's can use the short form
 */
sdi_resolve()
{
	register struct sdi *s;
	register struct sym_bkt *p;
	register int t;
	int changed;

	for (s = sdi_list; s; s = s->sdi_next) {
		p = s->sdi_sym;
		if (p && s->sdi_csect == p->csect_s && (p->attr_s & S_DEF))
			s->sdi_span += s->sdi_sym->value_s;
	}

	do {
		changed = 0;
		for (s = sdi_list; s; s = s->sdi_next) {
			if ((t = sdi_len(s) - s->sdi_leng) > 0) {
				longsdi(s, t);
				changed = 1;
			} else if (t < 0)
				Sys_Error("Pathological sdi\n", (char *) NULL);
		}
	} while (changed);
}

/*
 * update sdi list assuming the specified sdi must be long
 */
longsdi(s, delta)
register struct sdi *s;
register int delta;
{
	register struct sdi *t;
	register long loc, span, base;

	s->sdi_leng += delta;	/* update the length of this sdi */
	/* get the real location of the sdi */
	loc = s->sdi_loc + sdi_inc(s->sdi_csect, s->sdi_loc);
	for (t = sdi_list; t; t = t->sdi_next) {
		if (t != s && s->sdi_csect == t->sdi_sym->csect_s) {
			span = t->sdi_span;
			base = t->sdi_base;
			if (span < 0 && span+base <= loc && loc < base)
				t->sdi_span -= delta;
			else if (span >= 0 && base <= loc && loc < span+base)
				t->sdi_span += delta;
			if (base > loc)
				t->sdi_base += delta;
		}
	}
}

/*
 * compute the length of the specified sdi by searching the bounds list
 */
sdi_len(s)
register struct sdi *s;
{
	register struct blist *b;

	if (s->sdi_sym == 0)
		return(s->sdi_gleng);
	if (!(s->sdi_sym->attr_s & S_DEF) || s->sdi_csect != s->sdi_sym->csect_s)
		return(s->sdi_gleng);
	for (b = s->sdi_bounds; b; b = b->b_next)
		if (b->b_lbound <= s->sdi_span && s->sdi_span <= b->b_ubound)
			return(b->b_length);
	return(s->sdi_gleng);
}

/*
 * return the total number of extra bytes due to long sdi's before
 * the specified offset in the specified csect
 */
sdi_inc(csect, offset)
register struct csect *csect;
long int offset;
{
	register struct sdi *s;
	register int total;

	total = 0;
	for (s = sdi_list; s; s = s->sdi_next) {
		if (csect == s->sdi_csect) {
			if (offset <= s->sdi_loc)
				break;
			total += s->sdi_leng - s->sdi_bounds->b_length;
		}
	}
	return(total);
}

/*
 * release all sdi descriptors
 */
sdi_free()
{
	register struct sdi *s, *t;
	for (s = sdi_list; s; s = t) {
		t = s->sdi_next;
		b_free(s->sdi_bounds);
		free(s);
	}
	sdi_list = (struct sdi *)0;
}

/*
 * release a bounds list
 */
b_free(p)
register struct blist *p;
{
	register struct blist *q;
	while(p) {
		q = p->b_next;
		free(p);
		p = q;
	}
}
