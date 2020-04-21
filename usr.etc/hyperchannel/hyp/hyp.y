%{
#include "defs.h"
%}
%token	HOST	1
%token	NET	2
%token	TRUNK	3
%token	SPECIAL	4
%token  LP      5
%token  RP      6
%token  SEMI    7
%token  HLIST   8
%token  NUMB    9
%token  HEXNUM  10
%token  COMMA   11
%union {
	int intval;
	char *string;
}

%type <intval> adapter, numlist, NUMB, HEXNUM
%type <string> hostlist, hostname, HOST

%%

desc :   /* VOID */
     |   net adapterlist
     |   adapterlist net
     |   desc net adapterlist
     |   desc adapterlist net
     ;

net :    NET trunklist
    |    net NET trunklist
    ;
trunklist : trunk
          | trunk trunk 
          | trunk trunk trunk
          | trunk trunk trunk trunk
          ;

trunk :  TRUNK numlist hostlist
      ;

numlist :  numlist COMMA NUMB = {
            validate_trunk($3);
	}
	| NUMB = {
	    validate_trunk($1);
	}
        ;

hostlist : /* VOID */ {
                 $$ = NULL ;
	 }
         | HOST = {
	    (void) h_insert($1);
	 }
         | hostlist HOST = {
	    (void) h_insert($2);
	 }
         ;

hostname : /* VOID */  {
                 $$ = NULL ;
	 }
         |    HOST = {
		$$ = $1 ;/* yylval.string; */
	 }
         ;
adapterlist : HLIST alist
            ;

alist : hostname adapter = {
                 set_adapter($1,$2);
	    }
            | alist hostname adapter = {
	         set_adapter($2,$3);
	    }
            ;

adapter : HEXNUM = {
             $$ = $1 ; /* yylval.intval; */
	}
        ;

%%
int	yylineno = 1;
int     paren = 0;
extern  FILE *fin;
extern struct hostent *gethostbyname();
extern int mynet;
extern struct in_addr myaddr;
extern char *myname;
extern struct hynode *mynode, *fadapt;
struct hynode *cadapt = NULL; /* current adapter */
extern int cgroup,nerrs ;
int adapternum;
int ctrunk;

yylex()
{
	register int c;
	register char *cp1, *cp2;
	static char yytext[INMAX];
	
again:
	switch (c = getc(fin)) {
	case EOF:  /* end of file */
		return(0);

	case '#':  /* start of comment */
		while ((c = getc(fin)) != EOF && c != '\n')
			;
		if (c == EOF)
			return(0);
	case '\n':
		yylineno++;
	case ' ':
	case ':':
	case '\t':  /* skip blanks */
		goto again;
	case ';': /*SEMI */
	        return(SEMI);
	case ',': /*COMMA */
	        return(COMMA);
	}
	cp1 = yytext;
	cp2 = &yytext[INMAX - 1];
	for (;;) {
		if (cp1 >= cp2) {
			yyerror("input line too long\n");
			break;
		}
		*cp1++ = c;
		c = getc(fin);
		if (c == EOF || any(c, " \t();\n#:,")) {
		        *cp1 = NULL;
			ungetc(c, fin);
			break;
		}
	}
	if ((sscanf(yytext,"0x%x",&yylval.intval) > 0) && (strlen(yytext) < 7))
	        c = HEXNUM;
	else if (sscanf(yytext,"%d",&yylval.intval) > 0) {
	        c = NUMB;
	      }
	else if (!strcmp(yytext, "net")) {
	        (void) start_group();
		c = NET;
	      }
	else if (!strcmp(yytext, "trunk")) {
	        (void) start_trunk();
		c = TRUNK;
	      }
	else if (!strcmp(yytext, "hosts"))
		c = HLIST;
	else {
		yylval.string = makens(yytext);
		c = HOST;
        }
	return(c);
}

yyerror(s)
char *s;
{
  fprintf(stderr,"%s at line %d.\n",s,yylineno);
  nerrs++;
}

any(c, str)
	register int c;
	register char *str;
{
	while (*str)
		if (c == *str++)
			return(1);
	return(0);
}

/*
 * Checks for a valid trunk number.
 */

validate_trunk(t)
int t;
{
  if ((t >= 0) && (t <= 3))
    ctrunk |= 1 << t;
  else {
    fprintf(stderr,"Line %d: invalid trunk %d.\n",yylineno,t);
  }
}

/*
 * Allocate a group structure.
 */

start_group()
{
    cgroup++;
}

/*
 * start a trunk.
 */

start_trunk()
{
  ctrunk = 0;
}

/*
 * Insert a new host into the array of IP addresses.  Check for
 * duplications.  Fills in name and address and back pointer.
 */

struct hynode hlist[HYRSIZE];
struct hynode *get_adapter();

h_insert(h)
char *h;
{
  int i ;
  struct hynode *hh;

  if (h == NULL || !(hh = get_adapter(h))) return(NULL);

  for (i = 0; i < NTRUNKS; i++) {
    if ((ctrunk >> i) & 1) {
      if (hh->nd_net[i] ) {
	fprintf(stderr,"Host %s trunk %d is on two different nets.\n",h, i);
	nerrs++;
      }
      else {
	hh->nd_net[i] = cgroup;
	hh->nd_use++;
      }
    }
  }
}

/* This binds adapter number ad to hostname h */
set_adapter(h,ad)
char *h;
int  ad;
{
  struct hynode *hh;

  if (hh = get_adapter(h)) {
    if (hh->nd_adapter) {
      fprintf(stderr,"Duplicate adapter assignment: %s 0x%d\n",h,ad);
      nerrs++;
    }
    else {
      hh->nd_adapter = ad;
    }
  }
}


struct  hynode *get_adapter(h)
char *h;
{
  int i,hashval;
  struct hostent *hent;
  struct in_addr in;
  struct hynode *hh;

  if ((hent = gethostbyname(h)) == NULL) {
    fprintf(stderr,"Unknown host: %s.\n",h);
    nerrs++;
    return(NULL);
  }
  
  in.s_addr = ((struct in_addr *)(hent->h_addr))->s_addr ;

  hashval = inet_lnaof(in);
  hh = &hlist[hashval];
  for (i=0 ; i < HYRSIZE; i++,hh++ ) {
    if (hh >=  &hlist[HYRSIZE]) hh = &hlist[0];
    if ((hh->nd_addr.s_addr == in.s_addr) || (hh->nd_use == 0) ) break;
  }
  if (i >= HYRSIZE) {
    fprintf(stderr,"No room for host - %s\n", h);
    exit(1);
  }

  if (mynet != inet_netof(in)) {
    fprintf(stderr, "host %s on different net from %s.\n",h,myname);
    nerrs++;
    return(NULL);
  }
  if (!cadapt) fadapt = hh;
  if (!hh->nd_addr.s_addr) {
    hh->nd_addr.s_addr = in.s_addr;
    hh->nd_hname = makens(h);
    if (cadapt) cadapt->nd_next = hh;
    cadapt = hh;
    hh->nd_next = NULL;
    if (in.s_addr == myaddr.s_addr) mynode = hh;
  }
  return(hh);
}

int mx; 
/* Copy a string, after making some storage for it. */
char *makens(name)
	char *name;
{
	register char *nl;
	int ml;

	ml = strlen(name);
        mx = (ml > mx) ? ml : mx ;

	nl = malloc(ml);
	if (nl == NULL) {
		perror("ran out of memory\n");
		exit(1);
	}
	return(strcpy(nl,name));
}
