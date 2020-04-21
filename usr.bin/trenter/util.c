/*	@(#)util.c	1.2	*/
#include	"Param.h"
#include	"Acttab.h"
#include	"Bits.h"
#include	<string.h>

bit_set(fp, bn, ba)
reg	int	*fp;
reg	String	bn;
reg	Bit	*ba;
{
	for( ; ba->b_name; ba++) {
		if(str_equ(bn, ba->b_name)) {
			fp[ba->b_indx] |= 1 << ba->b_bitno;
			return;
		}
	}
	error("Bit not found!");
	/* NOTREACHED */
}

bit_clr(fp, bn, ba)
reg	int	*fp;
reg	String	bn;
reg	Bit	*ba;
{
	for( ; ba->b_name; ba++) {
		if(str_equ(bn, ba->b_name)) {
			fp[ba->b_indx] &= ~(1 << ba->b_bitno);
			return;
		}
	}
	error("Bit not found!");
	/* NOTREACHED */
}

bit_tst(fp, bn, ba)
reg	int	*fp;
reg	String	bn;
reg	Bit	*ba;
{
	for( ; ba->b_name; ba++) {
		if(str_equ(bn, ba->b_name))
			return(fp[ba->b_indx] &	(1 << ba->b_bitno));
	}
	error("Bit not found!");
	/* NOTREACHED */
}

Routine
act_get(name, table)
String	name;
Acttab	*table;
{
	reg	String	s;

	while(s = table->act_name) {
		if(!strncmp(s, name, strlen(s)))
			return(table->act_func);
		table++;
	}
	return((Routine) NULL);
}

error(message)
String	message;
{
	fflush(stdout);
	fprintf(stderr, "internal error: %s (errno=%d)\n", message, errno);
	fflush(stderr);
	exit(2);
}

char	*
falloc(x, y)
int	x;
int	y;
{
	if(calloc(x, y) == 0)
		error("No memory");
}

char	*
frealloc(x, y)
int	x;
int	y;
{
	if(realloc(x, y) == 0)
		error("No memory!");
}

/*
**	Get a yes/no response. If f is true, demand a 'no' response.
*/

getyn(f)
{
	reg	char	*buf = get_some(char, 10);
	reg	int	r = 0;

	fgets(buf, 10, stdin);
	if(*buf == 'y' || *buf == 'Y')
		r = 1;
	cfree(buf);
	return(r);
}

#define	QtoSQ(q)	((struct _q *) (((char *) q) - 4))

struct	_q	{
	short	q_len;
	short	q_gen;
	char	*q_felem;
};

typedef	struct _q	Q;

int	q_incr[] = {4, 8, 20, 42, 64, 100, 256, 512, 1024, 2000};

#define	MAXQG	(sizeof(q_incr) / sizeof(q_incr[0]))

int	qovincr = 1000;

q_opt(opt)
char	*opt;
{
	switch(*opt++) {
	case 'i':		/* new q_incr values */
	{
		reg	char	*p, *s;
		reg	int	i;

		s = opt;
		i = 0;
		while(p = strtok(s, " \t,")) {
			s = NULL;
			if(i > MAXQG) {
				fprintf(stderr, "Too many increments -- extra ignored.\n");
				break;
			}
			q_incr[i++] = atoi(p);
		}
		break;
	}

	case 'o':
		qovincr = atoi(opt);
		break;

	default:
		fprintf(stderr, "Unknown Q option -- ignored.\n");
		break;
	}
}

q_add(qp, elem)
queue	*qp;
qelem	elem;
{
	reg	Q	*sq;
	reg	int	realsize;

	if(!qp)
		error("Add to NULL queue!");
	if(!elem)
		error("Add NULL to queue!");
	if(!(*qp)) {	/* Add to empty queue */
		sq = (Q *) falloc((uns) sizeof(Q) + (q_incr[0] * sizeof(qelem)),
					(uns) 1);
		(*qp) = &sq->q_felem;
	} else {
		sq = QtoSQ(*qp);
		realsize = sq->q_gen >= MAXQG ?
			(sq->q_gen - MAXQG) * qovincr + q_incr[MAXQG - 1] :
			q_incr[sq->q_gen];
		if(sq->q_len + 1 == realsize) {
			sq->q_gen++;
			realsize = sq->q_gen >= MAXQG ?
				(sq->q_gen - MAXQG) * qovincr+q_incr[MAXQG-1] :
				q_incr[sq->q_gen];
			sq = (Q *) frealloc(sq, (uns) (sizeof(Q) +
					  (realsize * sizeof(qelem))));
			(*qp) = &sq->q_felem;
		}
	}
	(&sq->q_felem)[sq->q_len++] = elem;
	(&sq->q_felem)[sq->q_len] = NULL;
}

q_cat(qp, q)
queue	*qp;
queue	q;
{
	reg	int	i;

	if(q)
		for(i = 0; q[i]; i++)
			q_add(qp, q[i]);
}

q_rem(qp, elem)
queue	*qp;
qelem	elem;
{
	reg	queue	fq;
	reg	queue	tq;
	reg	Q	*sq;

	if(!qp)
		error("Rem from empty queue!");
	if(!(*qp) || !elem)
		return;
	sq = QtoSQ(*qp);
	if((*qp)[1] == NULL && (*qp)[0] == elem) {
		cfree((String) sq);
		(*qp) = NULL;
		return;
	}
	fq = (*qp);
	while(*fq && *fq != elem)
		fq++;
	if(*fq) {
		tq = fq++;
		sq->q_len--;
		while(*tq++ = *fq++)
			;
	}
}

q_rems(qp, elem)
queue	*qp;
qelem	elem;
{
	reg	queue	fq;
	reg	queue	tq;
	reg	Q	*sq;

	if(!qp)
		error("Rems from empty queue!");
	if(!(*qp) || !elem)
		return;
	sq = QtoSQ(*qp);
	if((*qp)[1] == NULL) {		/* one element and one NULL */
		if(str_equ((*qp)[0], elem)) {
			cfree((String) sq);
			(*qp) = NULL;
		}
		return;
	}
	fq = (*qp);
	while(*fq && !str_equ(*fq, elem))
		fq++;
	if(*fq) {
		tq = fq++;
		sq->q_len--;
		while(*tq++ = *fq++)
			;
	}
}

q_mem(q, elem)
queue	q;
qelem	elem;
{
	reg	int	i;

	if(q) {
		if(!elem)
			error("NULL q_mem of queue!");
		for(i = 0; q[i]; i++) {
			if(q[i] == elem)
				return(TRUE);
		}
	}
	return(FALSE);
}

q_m(q, elem)
queue	q;
qelem	elem;
{
	reg	int	i;

	if(q) {
		if(!elem)
			error("NULL q_m of queue!");
		for(i = 0; q[i]; i++) {
			if(str_equ(q[i], elem))
				return(TRUE);
		}
	}
	return(FALSE);
}

q_len(q)
queue	q;
{
	reg	int	r = QtoSQ(q)->q_len;

	return(r ? r + 1 : 0);
}

q_ind(q, elem)
queue	q;
qelem	elem;
{
	reg	int	i = -1;

	if(!q)
		error("Ind of NULL queue!");
	if(!elem)
		error("NULL ind of queue!");
	while(*q) {
		++i;
		if(*q++ == elem)
			return(i);
	}
	return(-1);
}

q_fre(q)
queue	q;
{
	reg	queue	oq;

	if(!q)
		return;
	oq = q;
	while(*q)
		cfree(*q++);
	q_free(oq);
}

q_free(q)
queue	q;
{
	cfree((String) QtoSQ(q));
}

int	Rdrc;

char	*
rdcmd(cmd, f)
char	*cmd;
char	*f;
{
	reg	FILE	*fp;
	reg	char	*os = NULL;
	auto	char	buf[40];
	reg	char	*p = NULL;

	if(f) {
		reg	char	*buf = get_some(char, strlen(f)+strlen(cmd)+2);

		sprintf(buf, "%s %s", cmd, f);
		fp = popen(buf, "r");
		cfree(buf);
	} else
		fp = popen(cmd, "r");
	if(!fp)
		error("bad cmd line in rdcl()");
	while(fgets(buf, 40, fp) != NULL)
		os = str_cat(os, buf);
	Rdrc = pclose(fp);
	if(os) {
		p = &os[strlen(os) - 1];
		if(*p == '\n')
			*p = '\0';
	}
	return(os);
}

String
str_cat(s0, s1)
String	s0;
String	s1;
{
	reg	String	s;
	reg	String	cp;

	if(!s0) {
		if(!s1)
			return(NULL);
		s0 = "";
	}
	if(!s1)
		s1 = "";
	s = get_some(char, str_len(s0) + str_len(s1));
	return(strcat(strcpy(s, s0), s1));
}

String
str_cpy(s)
String	s;
{
	reg	String	ds;

	if(s == NULL)
		return(NULL);
	ds = get_some(char, str_len(s));
	strcpy(ds, s);
	return (ds);
}

word_ind(s, wp)
String	s;
String	*wp;
{
	reg	int	ind = 0;

	while(*wp) {
		if(str_equ(s, *wp))
			return(ind);
		ind++;
		wp++;
	}
	return(-1);
}

String	*
word_vec(s)
String	s;
{
	reg	String	w;
	reg	String	cp = s;
	auto	String	*wv = NULL;		/* must be addressible */
	auto	int	bf;

	while(*cp && *cp != '\n') {
		bf = 0;
		while(*cp && isspace(*cp++))
			;
		s = --cp;
		if(*cp == '"') {		/* Gobule a string specially */
			s++;
			while(*++cp && *cp != '"' && *cp != '\n')
				;
			cp++;
		} else {
			while(*cp && !isspace(*cp++))	/* normal "word" */
				;	/* should eat strings here also! */
			if(!isspace(*(cp - 1)))
				bf = 1, cp++;
		}
		w = falloc((uns) (cp - s), (uns) 1);
		strncpy(w, s, (cp - s) - 1);
		q_add((queue *) &wv, (qelem) w);
		if(bf)
			break;
	}
	return(wv);
}
