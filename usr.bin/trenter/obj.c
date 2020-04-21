/*	@(#)obj.c	1.3	*/
#include	"Param.h"
#include	"Object.h"

/*
**	Field types
*/

char	*Ftypes[] = {
	"_null_",
	STRING,
	STRINGQ,
	BOOL,
	OBJ,
	OBJQ,
	NULL
};

/*
**	Optional field attributes
*/

char	*Fattrs[] = {
			/* IO Modes */
	IN,
	OUT,
	IO,
	IO2,
	IO3,
			/* Misc options */
	VERIFY,
	PRE,
	POST,
	REQ,
	HELP,
	MULTI,
	NAMEF,
	NULL
};

/*
**	Optional object attributes
*/

char	*Oattrs[] = {
	SFILE,
	NULL
};

/*
**	Object bits
*/

Bit	ob_bits[] =
{
        "Multi",	0,	0,
        "Req",		0,	1,
	"Namef",	0,	2,
        NULL
};

/*
**	Input parser's help message
*/

char	*Ohelp =
"The following sequences are recognized during prompting:\n\
	?	Print message explaining current field\n\
	??	Print this message\n\
	-	Return to previous field for correction\n\
	-field	Return to given field for correction\n\
	!e	Invoke editor to enter or change data for current field\n\
	>	Move to next un-filled field\n\
	=field	Print value of given field\n\
These sequences may be escaped with a backslash.";

Objtab	**Otab;
char	Sfbuf[SFBUFSZ];

OBJECT
ocreat(otp, name)
OBJTYPE	otp;
char	*name;
{
	auto	OBJECT	op;
	reg	int	i;

	if(otp == NULL || !q_mem(Otab, otp))
		return((OBJECT) NULL);
	if((op = olook(otp, name)) != NULL)
		return(op);
	op = get_one(Objent);
	if(name)
		op->o_name = str_cpy(name);
	op->o_otp = otp;
	q_a(&otp->o_q, op);
	if(otp->o_temp)
	for(i = 0; otp->o_temp[i]; i++) {
		q_a(&op->o_vq, get_one(Objval));
		if(name && is_namef(otp->o_temp[i]))
			osetv(op, otp->o_temp[i]->o_fname, name);
	}
	return(op);
}

OBJTYPE
otcreat(type)
char	*type;
{
	printf("Creating new object type: %s\n", type);
	fprintf(stderr, "Can't create new object types on fly, yet\n");
	return((Objtab *) NULL);
/*
	auto	Objtab	*otp;

	if((otp = otlook(type)) != NULL)
		return((Objtab *) NULL);
	otp = get_one(Objent);
	otp->o_type = str_cpy(type);
	q_a(&otp->o_tq, op);
	return(otp);
*/
}

#define	T_PRE	0
#define	T_POST	1
#define	PROMPC	':'

#define	QUEST	'?'
#define	INQ	'='
#define	BACK	'-'
#define	NEXT	'>'
#define	SHESC	'!'
#define	EDIT	'e'
#define	BSLASH	'\\'

char	Spec[] = { QUEST, INQ, BACK, NEXT, SHESC };
char	*_ogetf();
int	Bcnt;

/*
**	Fill the given object by prompting for its fields.  If a field
**	already has a value, it is printed.  If skf is false, the user
**	get a chance to change the existing value.
*/

ofill(op, skf)
OBJECT	op;
int	skf;
{
	_ofill(op, skf, stdin);
}

_ofill(op, skf, fp)
OBJECT	op;
int	skf;
FILE	*fp;
{
	reg	Objtab	*otp;
	reg	Objtmp	*tmp;
	reg	int	i;
	auto	char	*t;

	otp = op->o_otp;
	if(otp->o_temp && op->o_vq)
	for(i = 0; tmp = otp->o_temp[i]; i++) {
		reg	char	*s = NULL;
		auto	int	rf = is_req(tmp);

		if(tmp->o_pre && !pchk(T_PRE, tmp, op))
			continue;
		switch(tmp->o_iomode) {
		case FA_IN:
		case FA_IO:
		case FA_IO2:
			if(!tmp->o_istr)
				continue;
			s = get_some(char, strlen(tmp->o_istr) + 55);
			t = _spar(tmp->o_istr, tmp);
			strcpy(s, t);
			cfree(t);
			if(is_multi(tmp)) {
				t = str_cat(s, ":\n");
				cfree(s);
				s = t;
			}
			break;

		case FA_IO3:
			if(!tmp->o_ostr)
				continue;
			s = get_some(char, strlen(tmp->o_ostr) +
						strlen(tmp->o_istr) + 55);
			t = _spar(tmp->o_ostr, tmp);
			strcpy(s, t);
			cfree(t);
			if(tmp->o_istr) {
				strcat(s, " ");
				t = _spar(tmp->o_istr, tmp);
				strcat(s, t);
				cfree(t);
			}
			if(is_multi(tmp)) {
				t = str_cat(s, ":\n");
				cfree(s);
				s = t;
			}
			break;

		default:
			continue;
		}
		if((t = (char *) ogetv(op, tmp->o_fname)) != NULL) {
			auto	char	*ps;

			ps = get_some(char, strlen(s) + strlen(t) + 30);
			if(is_multi(tmp))
				sprintf(ps, "%s%s\n", s, t);
			else
				sprintf(ps, "%s = %s", s, t);
			if(!skf) {
				t = _ogetf(ps, 0, op, tmp, fp);
				if(!t) {
					i -= Bcnt;
					skf = 0;
					cfree(ps);
					continue;
				}
				osetv(op, tmp->o_fname, t);
				cfree(t);
			} else
				printf("%s\n", ps);
			cfree(ps);
		} else {
			t = _ogetf(s, rf, op, tmp, fp);
			if(!t) {
				i -= Bcnt;
				skf = 0;
				continue;
			}
			osetv(op, tmp->o_fname, t);
			cfree(t);
			pchk(T_POST, tmp, op);
		}
		if(s)
			cfree(s);
	}
}

char *
_ogetf(promp, rf, op, tmp, fp)
char	*promp;
int	rf;
OBJECT	op;
Objtmp	*tmp;
FILE	*fp;
{

	Bcnt = 0;
	while(1) {
		reg	char	*p;
		reg	char	*s = NULL;
		auto	int	lf = FALSE;
/*
		auto	int	mf = rf ? is_multi(tmp) : 0;
*/
		auto	int	mf = is_multi(tmp);

		while(1) {
			if(!s) {
				lf = FALSE;
				printf("%s", promp);
				if(!mf || hasval(op, tmp->o_fname))
					printf("%c ", PROMPC);
			}
			if(fgets(Sfbuf, SFBUFSZ, fp) == NULL) {
				if(s)
					break;
				if(fp == stdin)
					putchar('\n');
				continue;
			}
			p = Sfbuf;
			if(*(p + 1)) {
				if(!mf || is_spec(p)) {
					zapnl(p);
					s = str_cpy(p);
					break;
				}
				if(*p == '.' && *(p + 1) == '\n') {
					if((!s && rf) || !lf) {
						if(!lf && s) {
							cfree(s);
							s = NULL;
						}
						continue;
					}
					zapnl(s);
					break;
				}
cc:
				if(*p != '\n')
					lf = TRUE;
				s = str_cat(s, p);
				continue;
			}
			if(!rf)
				return((char *) NULL);
			if(mf)			/* YUK! */
				goto cc;
		}
		switch(*s) {
		case QUEST:
			if(!*(s + 1))
				ophelp(op, tmp->o_fname);
			else if(*(s + 1) == QUEST && !*(s + 2))
				printf("%s\n", Ohelp);
			else
				goto out;
			cfree(s);
			continue;

		case INQ:
			p = s;
			if(*++p) {
				reg	char	*t = (char *) ogetv(op, p);

				if(f_look(p, op->o_otp->o_temp) == -1)
					printf("Field not found.\n");
				else if((t = (char *) ogetv(op, p)) == NULL)
					printf("%s: no value.\n", p);
				else
					printf("%s = %s\n", p, t);
			} else
				printf("Field name must be specified.\n");
			cfree(s);
			continue;

		case BACK:
		{
			reg	int	i, oi;

			oi = f_look(tmp->o_fname, op->o_otp->o_temp);
			if(!s[1]) {
				Bcnt = 1;
				if(oi > 0) {
					auto	Objtmp	*ttp;

					i = oi;
					while((ttp = op->o_otp->o_temp[--i]) >=
							op->o_otp->o_temp[0]) {
						if(ttp->o_pre &&
							  !pchk(T_PRE, ttp, op))
							continue;
						break;
					}
					if(ttp)
						Bcnt = (oi -i) + 1;
				}
				cfree(s);
				return((char *) NULL);
			}
			i = f_look(&s[1], op->o_otp->o_temp);
			if(i != -1) {
				oi -= i;
				if(oi > 0)  {
					Bcnt = oi + 1;
					cfree(s);
					return((char *) NULL);
				}
			}
			printf("Field not found.\n");
			cfree(s);
			continue;
		}

		case NEXT:
		{
			auto	int	i, oi;
			auto	Objtmp	*ttp;

			if(*(s + 1)) {
				cfree(s);
				continue;
			}
			oi = f_look(tmp->o_fname, op->o_otp->o_temp);
			for(i = oi; ttp = op->o_otp->o_temp[i]; i++) {
				if(ttp->o_pre && !pchk(T_PRE, ttp, op))
					continue;
				if(!ogetv(op, ttp->o_fname))
					break;
			}
			oi -= i;
			Bcnt = oi + 1;
			cfree(s);
			return((char *) NULL);
			/* NOTREACHED */
		}

		case SHESC:
		{
			reg	char	*cmd;
			auto	char	mf = is_multi(tmp);
			auto	char	*fn;
			auto	FILE	*fp;
			extern	char	*Edit;

			if(!(*(s + 1)) || *(s + 1) != EDIT || *(s + 2))
				goto out;
			fn = mktemp(".tXXXXXX");
			if((fp = fopen(fn, "w")) == NULL) {
				fprintf(stderr, "cannot create temp!\n");
				cfree(s);
				return((char *) NULL);
			}
			p = (char *) ogetv(op, tmp->o_fname);
			if(p)
				fprintf(fp, "%s\n", p);
			fflush(fp);
			cmd = get_some(char, 50);
			sprintf(cmd, "%s %s", Edit, fn);
			printf("Executing %s...\n", Edit);
			system(cmd);
			cfree(cmd);
			if((fp = fopen(fn, "r")) == NULL) {
				fprintf(stderr, "cannot read temp!\n");
				cfree(s);
				return((char *) NULL);
			}
			cmd = get_some(char, 120);
			p = NULL;
			while(1) {
				if(fgets(cmd, 120, fp) == NULL)
					break;
				if(cmd[1] == NULL) {
					if(!mf)
						break;
				}
				p = str_cat(p, cmd);
				if(!mf)
					break;
			}
			cfree(cmd);
			fclose(fp);
			unlink(fn);
			cfree(s);
			if(!p)
				continue;
			czapnl(p);
			return(p);
		}

		case BSLASH:
			p = str_cpy(&s[1]);
			cfree(s);
			return(p);

		default:
out:
			if(s && tmp->o_verify) {
				auto	char	*ts;

				p = str_cpy(tmp->o_verify);
				while((ts = strtok(p, "| ")) != NULL) {
					Routine	rtn;
					int	nf = FALSE;
					int	rf = FALSE;
					int	rr;

					p = NULL;
					if(*ts == '!')
						nf = TRUE, ts++;
					if((rtn = act_get(ts, Oftab)) != NULL) {
						rf = TRUE;
						rr = (rtn)(tmp->o_fname, s);
					}
					if(!nf) {
						if((rf && rr) || str_equ(ts, s))
							break;
					} else {
						if((rf&&!rr) || !str_equ(ts, s))
							break;
					}
				}
				if(!ts) {
					printf("Invalid entry");
					if(hashelp(op, tmp->o_fname))
						printf(" (? for help)");
					printf(".\n");
/*
					ophelp(op, tmp->o_fname);
*/
					continue;
				}
			}
			return(s);
		}
	}
	/* NOTREACHED */
}

char	*
_spar(s, tmp)
char	*s;
Objtmp	*tmp;
{
	reg	char	*rs = get_some(char, strlen(s) + 55);
	reg	char	*p, *q;

	p = (s - 1);
	q = rs;
	while(*++p) {
		if(tmp && *p == '\\' && *(p + 1) == '%') {
			p++;
			continue;
		}
		if(*p == '\\' && *(p + 1) == 'n') {
			*q++ = '\n';
			p++;
		} else if(tmp && *p == '%') {
			p++;
			if(!strncmp(p, "FN", 2)) {
				strcat(q, tmp->o_fname);
				q += strlen(tmp->o_fname);
				p++;
			} else if(!strncmp(p, "FT", 2)) {
				strcat(q, tmp->o_ftype);
				q += strlen(tmp->o_ftype);
				p++;
			}
		} else
			*q++ = *p;
	}
	return(rs);
}

pchk(type, tmp, obj)
char	type;
Objtmp	*tmp;
OBJECT	obj;
{
	reg	char	*fld;
	reg	char	*p;
	reg	char	*s;
	reg	char	*t;

	if(type == T_PRE)
		fld = str_cpy(tmp->o_pre);
	else {
		Routine	rtn;

		if(!tmp->o_post)
			return;
		s = tmp->o_post[0];
		if((rtn = act_get(s, Oftab)) != NULL) {
			(rtn)(tmp->o_fname, ogetv(obj, tmp->o_fname));
			return(TRUE);
		}
		fld = str_cpy(s);
	}
	p = strchr(fld, ':');
	if(!p) {
		p = fld;
		fld = str_cpy(tmp->o_fname);
	} else
		*p++ = NULL;
	while((s = strtok(p, "| ")) != NULL) {
		Routine	rtn;

		p = NULL;
		if((t = (char *) ogetv(obj, fld)) == NULL)
			continue;
		if(*s != '!') {
			if(str_equ(t, s))
				goto yup;
		} else {
			s++;
			if(!str_equ(t, s))
				goto yup;
		}
	}
	return(FALSE);
yup:
	if(type == T_POST)
		printf("%s\n", tmp->o_post[1]);
	return(TRUE);
}

is_spec(p)
char	*p;
{
	reg	int	r = FALSE;

	if(strchr(Spec, *p))
		switch(*p) {
		case BACK:
		case NEXT:
		case INQ:
			r = TRUE;
			break;

		case QUEST:
			if(*(p + 1) == '\n' || (*(p + 1) == QUEST &&
							   *(p + 2) == '\n'))
				r = TRUE;
			break;

		case SHESC:
			if(*(p + 1) == EDIT && *(p + 2) == '\n')
				r = TRUE;
		}
	return(r);
}

OBJECT
olook(otp, name)
OBJTYPE	otp;
char	*name;
{
	reg	int	i;
	reg	Objent	*op = NULL;

	if(otp && q_mem(Otab, otp) && otp->o_q)
	for(i = 0; op = otp->o_q[i]; i++)
		if(str_equ(op->o_name, name))
			break;
	return(op);
}

OBJECTQ
otqlook(tp)
OBJTYPE	tp;
{
	return(tp->o_q);
}


OBJTYPE
otlook(type)
char	*type;
{
	reg	int	i;
	reg	Objtab	*otp;

	if(Otab)
	for(i = 0; otp = Otab[i]; i++)
		if(str_equ(otp->o_type, type))
			return(otp);
	return((Objtab *) NULL);
}

f_look(fname, temp)
char	*fname;
Objtmp	**temp;
{
	reg	int	i;
	reg	Objtmp	*otmp;

	if(temp)
	for(i = 0; otmp = temp[i]; i++)
		if(str_equ(otmp->o_fname, fname))
			return(i);
	return(-1);
}

#define	BSZ	1024

char	*nextok();
char	Ninit;

o_init(f, rf)
char	*f;
{
	Otab = getobj(Obdefs, NULL, NULL);
/*
	if(f)
		q_cat(&Otab, getobj(NULL, f, NULL));
*/
	if(f) {
		Objtab	**q;
		int	i;

		q = getobj(NULL, f, NULL);
		if(q) for(i = 0; q[i]; i++)
			q_a(&Otab, q[i]);
	}
	if(rf)
		o_read();
}

char	**n_wv;
char	*n_file;
FILE	*n_fp;
char	Wf;

Objtab	**
getobj(wv, file, fp)
char	**wv;
char	*file;
FILE	*fp;
{
	reg	char	*s;
	reg	int	i;
	auto	Objtmp	*otmp = NULL;
	auto	Objtab	**rq = NULL;
	auto	Objtab	*otp = NULL;

	n_wv = wv;		/* communication with nextok() */
	n_file = file;
	n_fp = fp;
	Ninit = FALSE;
	while(1) {
		s = nextok(0);
		if(!s)
			break;
		if((s[0] == '' && s[1] == '') || str_equ(s, "OBJECT")) {
			reg	char	*ts;

			if(otp && otmp) {
				if(!otmp->o_fname)
					cfree(otmp);
				else  {
					if(!otmp->o_ftype)
						otmp->o_ftype = FT_STR;
					q_a(&otp->o_temp, otmp);
				}
				otmp = NULL;
			}
			otp = get_one(Objtab);
			if(Wf) {
				s += 2;
				otp->o_type = s;
			} else if((otp->o_type = nextok(0)) == NULL)
					break;
			while(1) {
				ts = nextok(1);		/* peek at next token */
				if(!ts)
					break;
				i = word_ind(ts, Oattrs);
				if(i == -1)
					break;
				(void) nextok(0);    /* eat what we peeked at */
				switch(i) {
				case OA_SFILE:
					otp->o_sfile = nextok(0);
					break;
				}
			}
			q_a(&rq, otp);
			continue;
		}
		if((i = word_ind(s, Ftypes)) != -1) {
			otmp->o_ftype = i;
			if(i == FT_OBJ || i == FT_OBJQ)
				if((otmp->o_objt = nextok(0)) == NULL)
					continue;
		} else if((i = word_ind(s, Fattrs)) != -1) {
			switch(i) {
			case FA_IN:
				if((otmp->o_istr = nextok(0)) == NULL)
					continue;
				otmp->o_iomode = FA_IN;
				break;

			case FA_OUT:
				if((otmp->o_ostr = nextok(0)) == NULL)
					continue;
				otmp->o_iomode = FA_OUT;
				break;

			case FA_IO:
				if((otmp->o_istr = nextok(0)) == NULL)
					continue;
				otmp->o_iomode  = FA_IO;
				break;

			case FA_IO2:
			case FA_IO3:
				if((otmp->o_ostr = nextok(0)) == NULL)
					continue;
				if((otmp->o_istr = nextok(0)) == NULL)
					continue;
				otmp->o_iomode = i;
				break;

			case FA_PRE:
				if((otmp->o_pre = nextok(0)) == NULL)
					continue;
				break;

			case FA_POST:
			{
				reg	char	*p;
				auto	char	*q;

				q = nextok(0);
				p = strchr(q, ':');
				if(!p)
					q_a(&otmp->o_post, q);
				else {
					*p++ = NULL;
					q_a(&otmp->o_post, q);
					q_a(&otmp->o_post, p);
				}
				break;
			}

			case FA_REQ:
				obis(otmp, "Req");
				break;

			case FA_MULTI:
				obis(otmp, "Multi");
				break;

			case FA_NAMEF:
				obis(otmp, "Namef");
				break;

			case FA_VERIFY:
				if((otmp->o_verify = nextok(0)) == NULL)
					continue;
				break;

			case FA_HELP:
				if((otmp->o_help = nextok(0)) == NULL)
					continue;
				break;

			default:
				error("Unknown attr in getobj()");
			}
		} else {
			if(otp && otmp) {
				if(!otmp->o_fname)
					cfree(otmp);
				else  {
					if(!otmp->o_ftype)
						otmp->o_ftype = FT_STR;
					q_a(&otp->o_temp, otmp);
				}
			}
			otmp = get_one(Objtmp);
			otmp->o_fname = s;
		}
	}
	if(otp && otmp) {
		if(!otmp->o_fname)
			cfree(otmp);
		else  {
			if(!otmp->o_ftype)
				otmp->o_ftype = FT_STR;
			q_a(&otp->o_temp, otmp);
		}
	}
	return(rq);
}

/*
**	Return next token from template file.  If f is true, the access
**	is a 'peek' -- the token returned will also be returned by a
**	subsequent nextok(0).
*/

char *
nextok(f)
char	f;
{
	static	int	ind;
	auto	char	buf[BSZ];
	auto	char	*r;

	if(!Ninit) {
		Ninit = TRUE;
		ind = 0;
		if(!n_wv) {
			Wf = FALSE;
			if(n_file) {
				char	*cmd = get_some(char,20+strlen(n_file));

				strcpy(cmd, "/lib/cpp ");
				strcat(cmd, n_file);
				if((n_fp = popen(cmd, "r")) == NULL)
					return(NULL);
				cfree(cmd);
			} else if(!n_fp)
				return(NULL);
		} else
			Wf = TRUE;
	}
	if(!n_wv || !n_wv[ind]) {
		if(n_wv && !Wf) {
			q_free(n_wv);
			n_wv = NULL;
		}
		if(Wf)
			return(NULL);
		ind = 0;
reread:
		if(fgets(buf, BSZ, n_fp) == NULL)
			return(NULL);
		if(buf[0] == '\n' || buf[0] == '#')
			goto reread;
		n_wv = word_vec(buf);
	}
	r = n_wv[ind];
	if(!f)
		ind++;
	return(r);
}

hashelp(op, fname)
OBJECT	op;
char	*fname;
{
	reg	Objtmp	**tmpq;

	if(op && op->o_otp && ((tmpq = op->o_otp->o_temp) != NULL)) {
		reg	Objtmp	*tp;
		reg	int	find;

		find = f_look(fname, tmpq);
		if(find == -1)
			return(FALSE);
		tp = tmpq[find];
		if(tp && tp->o_help)
			return(TRUE);
	}
	return(FALSE);
}

hasval(op, fname)
OBJECT	op;
char	*fname;
{
	reg	int	r = FALSE;
	reg	Objtmp	**tmpq;

	if(op && op->o_otp && ((tmpq = op->o_otp->o_temp) != NULL)) {
		reg	Objtmp	*tp;
		reg	Objval	*vp;
		reg	int	find;

		find = f_look(fname, tmpq);
		if(find == -1)
			return(r);
		tp = tmpq[find];
		vp = op->o_vq[find];
		switch(tp->o_ftype) {
		case FT_STR:
			if(vp->v_str)
				r = TRUE;
			break;

		case FT_STRQ:
			if(vp->v_strq)
				r = TRUE;
			break;

		case FT_BOOL:
			if(vp->v_int)
				r = TRUE;
			break;

		case FT_OBJ:
			if(vp->v_obj)
				r = TRUE;
			break;

		case FT_OBJQ:
			if(vp->v_objq)
				r = TRUE;
			break;
		}
	}
	return(r);
}

otlock(tp)
OBJTYPE	tp;
{
	reg	char	*lockf;
	reg	int	i;

	lockf = get_some(char, strlen(tp->o_sfile) + strlen(LOCKFS) + 10);
	sprintf(lockf, "%s%s", tp->o_sfile, LOCKFS);
	while(access(lockf, 0) == 0) {
		fprintf(stderr, "Database update in progress; hold...\n");
		sleep(5);
	}
	if((i = creat(lockf, 0444)) == -1)
		error("cannot create lock file!");
	close(i);
	cfree(lockf);
}

otunlock(tp)
OBJTYPE	tp;
{
	reg	char	*lockf;

	lockf = get_some(char, strlen(tp->o_sfile) + strlen(LOCKFS) + 10);
	sprintf(lockf, "%s%s", tp->o_sfile, LOCKFS);
	unlink(lockf);
	cfree(lockf);
}

char	*
oname(op)
OBJECT	op;
{
	return(op->o_name);
}

orename(op, name)
OBJECT	op;
char	*name;
{
	if(!op)
		return(FALSE);
	if(op->o_name)
		cfree(op->o_name);
	op->o_name = str_cpy(name);
	return(TRUE);
}

oprt(op)
OBJECT	op;
{
	_oprt(op, stdout);
}

_oprt(op, fp)
OBJECT	op;
FILE	*fp;
{
	reg	Objtab	*otp;
	reg	Objtmp	*temp;
	auto	int	i;
	auto	int	sf = (fp != stdout);
	auto	char	*s;

	otp = op->o_otp;
	if(otp->o_temp && op->o_vq)
	for(i = 0; temp = otp->o_temp[i]; i++) {
		auto	int	mf = is_multi(temp);
		reg	char	*ostr = NULL;

		if(!sf && !hasval(op, temp->o_fname))
			continue;
		ostr = temp->o_ostr;
		if(!ostr && temp->o_iomode == FA_IO)
			ostr = temp->o_istr;
		if(sf || ostr) {
			reg	Objval	*vp = op->o_vq[i];
			auto	int	x;

			if(!sf) {
				fprintf(fp, "%s:", ostr);
				if(mf)
					fputc('\n', fp);
				else
					fputc(' ', fp);
			} else {
				fprintf(fp, "%s", temp->o_fname);
				fputc(ODELIMC, fp);
				if(mf)
					fputc('\n', fp);
			}
			switch(temp->o_ftype) {
			case FT_STR:
				if(!vp->v_str) {
					if(sf)
						fprintf(fp, "NULL\n");
					if(mf)
						fprintf(fp, ".\n");
				} else if(sf && mf) {
			/*
					s = _spar(vp->v_str, 0);
					fprintf(fp, "%s\n.\n", s);
					cfree(s);
					fprintf(fp, "%s\n.\n", vp->v_str);
			*/
					fprintf(fp, "%s.\n", vp->v_str);
				} else
					fprintf(fp, "%s\n", vp->v_str);
				break;

			case FT_STRQ:
				if(!vp->v_strq) {
					if(sf)
						fprintf(fp, "NULL\n");
				} else {
					for(x = 0; s = vp->v_strq[x]; x++) {
						if(strchr(s, ' ') || strchr(s, '\t'))
							fprintf(fp,"\"%s\"", s);
						else
							fprintf(fp,"%s", s);
						if(vp->v_strq[x+1])
							fputc(' ', fp);
					}
					fputc('\n', fp);
				}
				break;

			case FT_BOOL:
				fprintf(fp, "%d\n", vp->v_int);
				break;


			case FT_OBJ:
				if(!vp->v_obj) {
					if(sf)
						fprintf(fp, "NULL\n");
				} else
					fprintf(fp, "%s\n", vp->v_obj->o_name);
				break;

			case FT_OBJQ:
				if(!vp->v_objq) {
					if(sf)
						fprintf(fp, "NULL\n");
				} else {
					for(x = 0; vp->v_objq[x]; x++) {
						fprintf(fp, "%s",
							 vp->v_objq[x]->o_name);
						if(vp->v_objq[x+1])
							fputc(' ', fp);
					}
					fputc('\n', fp);
				}
				break;
			}
		}
	}
}

ophelp(op, fname)
OBJECT	op;
char	*fname;
{
	reg	Objtmp	**tmpq;

	if(op && op->o_otp && ((tmpq = op->o_otp->o_temp) != NULL)) {
		reg	Objtmp	*tp;
		reg	int	find;

		find = f_look(fname, tmpq);
		tp = tmpq[find];
		if(tp && tp->o_help) {
			auto	char	*t = _spar(tp->o_help, tp);

			printf("%s\n", t);
			cfree(t);
		}
	}
}

OBJECT	_oread();

otread(tp, file)
OBJTYPE	tp;
char	*file;
{
	reg	FILE	*fp;
	reg	int	i;
	reg	char	*ofile;

	if(!tp)
		return(FALSE);
	ofile = tp->o_sfile;
	if(file)
		tp->o_sfile = file;
	if(tp->o_sfile == NULL)
		goto restore;
	if(!file)
		otlock(tp);
	if((fp = fopen(tp->o_sfile, "r")) == NULL)
		goto unlck;
/*
	if(tp->o_q) {
		q_free(tp->o_q);
		tp->o_q = NULL;
	}
*/
	(void) _oread(tp, fp, 0);
unlck:
	if(!file)
		otunlock(tp);
restore:
	tp->o_sfile = ofile;
	return(TRUE);
}

struct	_cyt	{
	OBJTYPE	c_tp;
	FILE	*c_fp;
	char	*c_ofile;
};

typedef	struct	_cyt	Cyt;

Cyt	**Cyq;

/*
**	Note:  the alternate file args are implemented by changing the
**	save file name in the OBJTYPE struct for the  duration of the
**	call.  Since otcycle() is returning before the file is closed,
**	the default save file will be changed between cycle start and end.
*/

OBJECT
otcycle(tp, mode, file)
OBJTYPE	tp;
char	mode;
char	*file;
{
	reg	Cyt	*cyt = NULL;
	reg	int	i;

	if(!tp)
		return(FALSE);
	switch(mode) {
	case CYSTART:
		otcycle(tp, CYEND, NULL);
		cyt = get_one(Cyt);
		cyt->c_ofile = tp->o_sfile;
		if(file)
			tp->o_sfile = file;
		if(!tp->o_sfile) {
			cfree(cyt);
			tp->o_sfile = cyt->c_ofile;
			break;
		}
		cyt->c_tp = tp;
		q_a(&Cyq, cyt);
		if(!file)
			otlock(tp);
		if((cyt->c_fp = fopen(tp->o_sfile, "r")) != NULL) {
			if(tp->o_q) {
		/*
				q_fre(tp->o_q);
		*/
				q_free(tp->o_q);
				tp->o_q = NULL;
			}
		} else {
			otunlock(tp);
			tp->o_sfile = cyt->c_ofile;
		}
		break;
	
	case CYREAD:
	case CYEND:
		if(Cyq)
		for(i = 0; cyt = Cyq[i]; i++)
			if(cyt->c_tp == tp)
				break;
		if(cyt) {
			if(mode == CYREAD) {	
				OBJECT	obj = _oread(tp, cyt->c_fp, 1);

				if(obj)
					return(obj);
			}
			fclose(cyt->c_fp);
			q_r(&Cyq, cyt);
			otunlock(tp);
			tp->o_sfile = cyt->c_ofile;
			cfree(cyt);
		}
		break;
	}
	return(NULL);
}

OBJECT
_oread(tp, fp, f)
OBJTYPE	tp;
FILE	*fp;
char	f;
{
	reg	int	i;
	auto	char	*mf = NULL;
	auto	char	*mstr = NULL;
	auto	OBJECT	obj = NULL;
	static	char	*svl = NULL;

	while(1) {
		reg	char	outf = FALSE;
		reg	char	*p;

		if(svl) {
			strcpy(Sfbuf, svl);
			cfree(svl);
			svl = NULL;
		} else if(fgets(Sfbuf, SFBUFSZ, fp) == NULL) {
			outf = TRUE;
			goto out;
		}
		if(mf) {
			if(str_equ(Sfbuf, ".\n")) {
				osetv(obj, mf, mstr);
				cfree(mf);
				cfree(mstr);
				mf = NULL;
				mstr = NULL;
			} else
				mstr = str_cat(mstr, Sfbuf);
			continue;
		}
		zapnl(Sfbuf);
		p = strchr(Sfbuf, ODELIMC);
		if(!p) {
out:
			if(obj && f) { 	/* called by otcycle()? */
				svl = str_cpy(Sfbuf);
				return(obj);
			}
			if(outf)
				goto ret;
			if((obj = olook(tp, Sfbuf)) == NULL)
				obj = ocreat(tp, Sfbuf);
		} else {
			*p++ = NULL;
			if(!*p)
				mf = str_cpy(Sfbuf);
			else
				osetv(obj, Sfbuf, p);
		}
	}
ret:
	if(!f)
		fclose(fp);
	return(NULL);
}

o_read()
{
	reg	int	i;
	reg	OBJTYPE	otp;

	for(i = 0; otp = Otab[i]; i++)
		(void) otread(otp, NULL);
}

orem(op)
OBJECT	op;
{
	if(op->o_name)
		cfree(op->o_name);
	q_r(&op->o_otp->o_q, op);
	cfree(op);
	return(TRUE);
}

osetv(op, fname, val)
OBJECT	op;
char	*fname;
int	val;
{
	reg	Objtmp	**temp;
	reg	Objtmp	*tp;
	reg	Objval	*vp;
	reg	int	find;
	reg	int	r = NULL;

	if(!op)
		return(NULL);
	if(str_equ((char *) val, "NULL"))
		return(NULL);
	temp = op->o_otp->o_temp;
	find = f_look(fname, temp);
	if(find == -1)
		return(NULL);
	tp = temp[find];
	vp =  op->o_vq[find];
	if(!vp)
		return(NULL);
	if(is_namef(tp) && val)
		orename(op, val);
	switch(tp->o_ftype) {
	case FT_STR:
		if(vp->v_str)
			cfree(vp->v_str);
		if(val) {
			vp->v_str = str_cpy((char *) val);
			r = (int) vp->v_str;
		} else {
			vp->v_str = (char *) NULL;
			r = NULL;
		}
		break;

	case FT_STRQ:
		if(val) {
			char	**wv = word_vec(val);
			int	i;

			for(i = 0; wv[i]; i++)
				q_a(&vp->v_strq, str_cpy((char *) wv[i]));
			q_fre(wv);
			r = (int) vp->v_strq;
		} else {
			if(vp->v_strq)
				q_free(vp->v_strq);
			vp->v_strq = (char **) NULL;
			r = NULL;
		}
		break;

	case FT_BOOL:
		r = vp->v_int = val;
		break;

	case FT_OBJ:
		if(val) {
			OBJTYPE	otp = otlook(tp->o_objt);

			if(otp) {
				vp->v_obj = olook(otp, val);
				if(!vp->v_obj)
					vp->v_obj = ocreat(otp, val);
				r = (int) vp->v_obj;
			}
		} else {
			vp->v_obj = (OBJECT) NULL;
			r = NULL;
		}
		break;

	case FT_OBJQ:
		if(val) {
			OBJTYPE	otp = otlook(tp->o_objt);
			char	**wv = word_vec(val);
			int	i;

			if(otp)
			for(i = 0; wv[i]; i++) {
				OBJECT	ob = olook(otp, wv[i]);

				if(!ob) {
					fprintf(stderr, "creating new %s object: %s\n", tp->o_objt, wv[i]);
					ob = ocreat(otp, wv[i]);
			/*
					_ofill(ob, 1, stdin);
			*/
				}
				q_a(&vp->v_objq, ob);
			}
			q_fre(wv);
			r =  (int) vp->v_objq;
		} else {
			if(vp->v_objq)
				q_free(vp->v_objq);
			vp->v_objq = (OBJECTQ) NULL;
			r = NULL;
		}
		break;
	}
	return(r);
}

char	Nmf;

ogetv(op, fname)
OBJECT	op;
char	*fname;
{
	reg	int	r;

	Nmf = 0;
	r = _ogetv(op, fname);
	if(!r && Nmf) {
		r = (int) op->o_name;
		osetv(op, fname, r);
	}
	return(r);
}

_ogetv(op, fname)
OBJECT	op;
char	*fname;
{
	reg	Objtmp	**temp;
	reg	Objtmp	*tp;
	reg	Objval	*vp;
	reg	int	find;

	temp = op->o_otp->o_temp;
	find = f_look(fname, temp);
	if(find < 0)
		return(-1);
	vp = op->o_vq[find];
	tp = temp[find];
	if(is_namef(tp))
		Nmf = 1;
	switch(tp->o_ftype) {
	case FT_STR:
		return((int) vp->v_str);

	case FT_STRQ:
		return((int) vp->v_strq);

	case FT_BOOL:
		return(vp->v_int);

	case FT_OBJ:
		return((int) vp->v_obj);

	case FT_OBJQ:
		return((int) vp->v_objq);
	}
}

owrite(obj)
OBJECT	obj;
{
}

oappend(tp, obj, f, file)
OBJTYPE	tp;
OBJECT	obj;
char	f;
char	*file;
{
	auto	FILE	*fp;
	reg	char	*savef;
	reg	char	*ofile;
	reg	int	i;
	auto	char	r = FALSE;

	if(!tp || !obj)
		return(FALSE);
	ofile = tp->o_sfile;
	if(file)
		tp->o_sfile = file;
	if((savef = tp->o_sfile) == NULL)
		goto restore;
	if(!file)
		otlock(tp);
	if(access(savef, 0) != 0) {
		int	sm = umask(0);

		close(creat(savef, 0666));
		umask(sm);
	}
	if((fp = fopen(savef, "a")) != NULL) {
		r = TRUE;
		if(!f)
			fprintf(fp, "%s\n", obj->o_name);
		_oprt(obj, fp);
	}
	if(!file)
		otunlock(tp);
	fclose(fp);
restore:
	tp->o_sfile = ofile;
	return(r);
}

otwrite(tp, file)
OBJTYPE	tp;
char	*file;
{
	auto	FILE	*fp;
	reg	char	*savef;
	reg	char	*ofile;
	reg	OBJECT	obj;
	reg	int	i;
	auto	char	r = FALSE;

	if(!tp)
		return(FALSE);
	ofile = tp->o_sfile;
	if(file)
		tp->o_sfile = file;
	if((savef = tp->o_sfile) == NULL)
		goto restore;
	if(!file)
		otlock(tp);
	if(access(savef, 0) != 0) {
		int	sm = umask(0);

		close(creat(savef, 0666));
		umask(sm);
	}
	if((fp = fopen(savef, "w")) != NULL) {
		r = TRUE;
		if(tp->o_q)
		for(i = 0; obj = tp->o_q[i]; i++) {
			fprintf(fp, "%s\n", obj->o_name);
			_oprt(obj, fp);
		}
	}
	if(!file)
		otunlock(tp);
	fclose(fp);
restore:
	tp->o_sfile = ofile;
	return(r);
}

o_write()
{
	reg	int	i;
	reg	OBJTYPE	otp;

	if(Otab)
	for(i = 0; otp = Otab[i]; i++)
		(void) otwrite(otp, NULL);
}
