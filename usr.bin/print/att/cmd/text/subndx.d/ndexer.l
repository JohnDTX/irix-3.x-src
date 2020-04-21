%{
/*	Copyright (c) 1984 AT&T	*/
%}
%{
/*	  All Rights Reserved  	*/
%}

%{
/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
%}
%{
/*	The copyright notice above does not evidence any   	*/
%}
%{
/*	actual or intended publication of such source code.	*/
%}

%{
%}

%{

#include <stdio.h>

#include "dstructs.h"


#define DEBUG 0

#define MAXTERMS 20		/*max no. of terms in a subject*/
#define TOKSIZE  32
#define TRUE 1
#define FALSE 0
#define FAILED 9
#define ERROR0  fprintf(stderr, "\nndx: error0- too many terms in subject %d\n", subjnum);
#define ERROR1 fprintf (stderr, "\nndx: error1- storage allocation failure\n")
#define ERROR2 fprintf(stderr, "\nndx: error2- illegal symbol, %s, found \n \
		in line %d in subject file.\n", yytext, s_count);
#define ERROR5 fprintf (stderr, "\nndx: error5- subject term, %s, too long \n \
		max. allowed is %d characters", yytext, TOKSIZE);

int	lpass1 = TRUE;			/*flag used by yywrap*/

char	token[TOKSIZE];			/*an indexible string*/
char	lc_token[TOKSIZE];		/*lower case version of token*/

char	stype = 'N';			/*B,C,L,N if current subject is a
					  cap & literal, cap, literal, none*/

int	subjnum = 0;			/* current subject number read */
int	s_count = 0;			/* subject counter*/
int	t_count = 0;			/* counts the terms in the current subj*/

int	*fsec = FALSE;			/* true if the current token begins the
					secondary component of the currrent subj*/
int 	*growth;


struct subj	*subject = NULL;		/*ptr to current subject*/

struct word 	*wlist = NULL;			/*header of global word table (tree)*/
struct word 	*(*newnode);
typedef struct word * WORDPTR;


struct subj	*slist = NULL;			/* header of global subject list*/
struct subj	*subj_insert();


	/*Global declarations for 4.2 follow*/

#define MOD	%
#define C_SIZE	2	/*max. sents in the current indexing context*/
#define	MAXACTIVE	(100*C_SIZE)
#define	MAXACCEPT	(50*C_SIZE)
#define	MAXREJECT	(50*C_SIZE)

FILE	*pagefile;
FILE	*c_bufr;			/*context buffer for reading*/
FILE	*c_bufw;			/*context buffer for writing*/
char	tmpbuf[L_tmpnam];
int	s_cnt;				/*context sentence counter*/
int	c_len[C_SIZE];			/*length in chars of each context sentence*/

char	root_token[TOKSIZE];
char	*pagereg;
char	*sectreg;

struct word *w_ptr = NULL;

struct active
	{struct word *w_ptr;		/*pointer to a word in  wlist*/
	char        m_type;};		/*type of match found, literal or root*/

struct active activ_list[MAXACTIVE];	/*active words, circular array*/

struct subj  *accept_list[MAXACCEPT];	/*list of subjects found in curr context*/

struct subj  *reject_list[MAXREJECT];	/*list of subjs rejected in curr context*/

int	iac = -1, ir = -1;			/*indicies for accept and reject lists*/
int	head[C_SIZE];			/*heads of active lists for context sents*/
int	tail[C_SIZE];		 	/*tails of active lists for context sents*/
int	old_c, new_c;			/*id of oldest and newest context sentences*/

int	tot_act;			/*total currently active words*/
int	prev_c;				/*id of previous context*/

struct word *find_word ();

struct word  *find_root ();

int	heading = FALSE;

%}

%START IX SL MD DV ABS CMD

DL	[ \t\n]
STET    [~]

%%


<IX>([a-zA-Z0-9][a-zA-Z0-9&'/-]*)|([A-Z][.])  
				{copy_in (yytext);
				 c_len[new_c] = c_len[new_c] + yyleng;
				 if (yyleng < TOKSIZE)
					{
					 w_ptr = find_word (wlist, yytext);
					 if (w_ptr != NULL)
						w_activate (w_ptr, 'L');	/*lit match*/
					 else 
					    {strcpy (lc_token, yytext);
					     if (ulcase (lc_token))
						{w_ptr = find_word (wlist, lc_token);
						 if (w_ptr != NULL)
							w_activate (w_ptr, 'C');
						}
					    }
					 strcpy (root_token, yytext);
					 ulcase (root_token);
					 root_token[rootword(root_token)] = '\0';
					 match_roots (wlist, root_token);
					}
				}
<IX>[.!?][\"']*([\\][*]F)*[\"']*[ \t]*[\n]		
				{copy_in (yytext);
				 c_len[new_c] = c_len[new_c] + yyleng;
				 s_cnt = s_cnt +1;		/*update sent counter*/
				 if (s_cnt >= C_SIZE || heading == TRUE)  /*if context full*/
				    {subj_search ();
				     write_context ();
				     wri_subjmacro ();
				     return_lists ();
				     old_c = (old_c +1) MOD C_SIZE;}
				 if (heading == TRUE)
				     {init_vars ();
				      heading = FALSE;}
				 else
				     {prev_c = new_c;
				      new_c = (s_cnt + 1) MOD C_SIZE;	/*get new index*/
				      c_len[new_c] = 0;
				      head[new_c] = tail[prev_c];
				      tail[new_c] = head[new_c];}
				}
<IX>^[.']([SN]H?([ ][0-9]))[ \t]+  {flush_context ();
				 heading = TRUE;
				 fprintf (pagefile, "%s",yytext);
				}
<IX>^[.']((HU)|(H[ ][0-9]))[ \t]+  {flush_context ();
				 heading = TRUE;
				 fprintf (pagefile, "%s",yytext);
				}
<IX>[\n]			{copy_in (yytext);
				 c_len[new_c] = c_len[new_c] + yyleng;
				 if (heading == TRUE)
				    {flush_context ();
				     heading = FALSE;
				    }
				}
<IX>^[\\][!].*[\n]		{flush_context ();
				 fprintf (pagefile, "%s", yytext);
				}
<IX>[\\][\n]			{copy_in (yytext);
				 c_len[new_c] = c_len[new_c] + yyleng;}
<IX>[\\]["].*[\n]		{fprintf (pagefile, "%s", yytext);}  /*comment*/
<IX>[\\][*]F			{copy_in (yytext);	/*footnote label*/
				 c_len[new_c] = c_len[new_c] + yyleng;}
<IX>[\\]f(([(]..)|[^(])		{copy_in (yytext);	/*font change*/
				 c_len[new_c] = c_len[new_c] + yyleng;}
<IX>[\\]s([0-9]|([+-][0-9]))	{copy_in (yytext);	/*size change*/
				 c_len[new_c] = c_len[new_c] + yyleng;}
<IX>[\\][^*fs \t\n]+		{copy_in (yytext);	/*escape seq*/
				 c_len[new_c] = c_len[new_c] + yyleng;}
<IX>^[.]((de)|(am)).*[\n]	{flush_context ();
				 fprintf (pagefile, "%s", yytext);
				 BEGIN MD;
				}
<MD>^[.][.].*[\n]		{fprintf (pagefile, "%s", yytext);
				 BEGIN IX;
				}
<MD>.*[\n]			{fprintf (pagefile, "%s", yytext);}
<IX>^[.']TL.*[\n].*[\n]		{flush_context ();
				 fprintf (pagefile, "%s", yytext);
				}
<IX>^[.'](I|B|R|UL)[ \t\n]+	{copy_in(yytext);
				 c_len[new_c] = c_len[new_c] + yyleng;}
<IX>^[.']LI[ \t\n]+		{flush_context ();
				copy_in(yytext);
				 c_len[new_c] = c_len[new_c] + yyleng;}
<IX>^[.']((ul)|(cu)|(uf)).*[\n] {copy_in (yytext);
				 c_len[new_c] = c_len[new_c] + yyleng;}
<IX>^[.']tm.*[\n]		{;}
<IX>^[.']((di)|(da)).*[\n]      {flush_context ();
				 fprintf (pagefile, "%s", yytext);
				 BEGIN DV;
				}
<DV>^[.]((di)|(da)).*[\n]   {fprintf (pagefile, "%s", yytext);
				 BEGIN IX;
				}
<DV>.*[\n]			{fprintf (pagefile, "%s", yytext);}
<IX>^[.']AS.*[\n]		{flush_context ();
				 fprintf (pagefile, "%s", yytext);
				 BEGIN ABS;
				}
<ABS>^[.]AE.*[\n]                 {fprintf (pagefile, "%s", yytext);
				 BEGIN IX;
				}
<ABS>.*[\n]			{fprintf (pagefile, "%s", yytext);}
<IX>^[.']			{flush_context();
				 fprintf (pagefile, "%s", yytext);
				 BEGIN CMD;
				}
<CMD>.*[\n]			{fprintf (pagefile, "%s", yytext);
				 BEGIN IX;
				}
<IX>.				{copy_in (yytext);
				 c_len[new_c] = c_len[new_c] + yyleng;}



<SL>{STET}				{ stype = 'L';}		/*literal subj*/
<SL>^[0-9][0-9]*{DL}			{ if (s_count++ > 0) 
						{subject->type = stype;
						subj_insert();}
					  t_count = 0;
					  *fsec = FALSE;
					  stype = 'N';
					  subjnum = atoi (yytext);
					  init_subj (s_count);}
<SL>[A-Z][.]				{ if (++t_count > MAXTERMS)
						ERROR0;
					  if (t_count == 1)
						{ if (stype == 'L')
							stype = 'B';  /*both*/
						  else  stype = 'C';}  /*cap*/
					  strcpy (token, yytext);
					  build_subj (fsec);}
<SL>[A-Z][^ \t\n,]*			{ if (++t_count > MAXTERMS)
						ERROR0;
					  if (t_count == 1)
						{ if (stype == 'L')
							stype = 'B';  /*both*/
						  else  stype = 'C';}  /*cap*/
					 if (yyleng < TOKSIZE)
						{
						  strcpy (token, yytext);
						  build_subj (fsec);
						}
					 else
						ERROR5;
					}
<SL>[a-z0-9][^ \t\n,]*			{ if (++t_count > MAXTERMS)
						ERROR0;
					 if (yyleng < TOKSIZE)
						{
						  strcpy (token, yytext);
						  build_subj (fsec);
						}
					 else
						ERROR5;
					}
<SL>[,]{DL}*				{ *fsec = TRUE;}	/*begin sec component*/
<SL>{DL}{DL}*				{;}
<SL>[.]					{ fprintf (stderr, "\nndx: error2- unrecognized symbol, %s, found ",yytext);
					  fprintf (stderr, "in subject file.\n");}
%%
init_subj (subjno)

int subjno;
{	struct subj *s_malloc ();

	subject = (struct subj *) s_malloc ();
	subject->wds  = NULL;
	subject->type  = stype;
	/*subject->refs  = NULL*/
	subject->ckd  = '0';
	subject->id  = subjno;
	subject->sec  = NULL;
	subject->nxts = NULL;}



print_subj()			/*For debugging, prints internal subj list*/

{struct s_ele	*w;

	w = subject->wds;
	while (w != NULL)
		{
		printf ("name: %s\n", (*w->ptr).name);
		w = w->nxte;
		}
	printf ("type: %c\nckd: %c\nid: %d\nsec: %d\n",
		  subject->type, subject->ckd, subject->id, subject->sec);
	if (subject->sec != NULL)
		{
	 	 printf ("sec name: %s\n",subject->sec->ptr->name);
		}
}


/**************************************************************************
*   									   *
*   Build_subject builds the list of terms for the current subject.  It	   *
*   adds the next term in the subject to the master word list if necessary *
*   getting a pointer to the word in this list and adds this pointer to the*
*   subject's list of terms.						   *
*   called by:								   *
*   calls  :								   *
****************************************************************************/


build_subj ( flag)

int  *flag;			/*0,1 if this is not/is the 1st secondary term*/

{
struct s_ele	*ele;			/*pointer to entry in subjects list of terms*/
static struct s_ele  *prev_word;	/*pointer to previous entry in subject's list*/
char	*malloc();
struct word *word_insert ();




	*growth = 0;
	ele = (struct s_ele *) malloc ((unsigned) sizeof(struct s_ele));
	wlist = word_insert (wlist, growth, newnode);
	ele->ptr = *newnode;
	ele->nxte = NULL;

	if (subject->wds == NULL)
		subject->wds = ele;		/*add 1st term in subject*/
	else
		prev_word->nxte = ele;		/*add next term in subject*/

	prev_word = ele;			/*reset ptr to tail of list*/

	if (*flag > 0)  
		{subject->sec = ele;		/*set ptr to secondary component*/
		*flag = 0;}			/*and reset flag*/
}



#define MATCH strcmp (tree->name, token) 

struct word *word_insert (tree, growth, nunode)
struct	word	*(*nunode), *tree;
int	*growth;
{
	struct	word	*t1,*t2;
	struct	word	*new_word_node();
	int printtree ();

	/*printtree (tree, 'S');*/
	if (tree == NULL)
		{tree = new_word_node ();
		 *nunode = tree;
		 *growth = 1;
		 return (tree);
		}
	else if (MATCH  < 0)
			{tree->lc = word_insert (tree->lc, growth, nunode);
			if (*growth)
				switch (tree->bal)
				{case 1: tree->bal = 0;
					 *growth = 0;
					 break;
				 case 0: tree->bal = -1;
					 break;
				 case -1: t1 = tree->lc;
					 if (t1->bal == -1)
					   {tree->lc = t1->rc;
					   t1->rc = tree;
					   t1->bal = 0;
					   tree->bal = 0;
					   tree = t1;
					    *growth = 0;
					    return (t1);
					    break;}
					else
					   {t2 = t1->rc;
					    t1->rc = t2->lc;
				   	    t2->lc = t1;
					    tree->lc = t2->rc;
					    t2->rc = tree;
					    if (t2->bal == -1)
						tree->bal = 1;
					    else
						tree->bal = 0;
	 				    if (t2->bal == 1)
						t1->bal = -1;
					    else
						t1->bal = 0;
					    t2->bal = 0;
					    *growth = 0;
					    tree = t2;
					    return (t2);
					    break;}
				}                                 /*end case*/
			}
	else if (MATCH > 0)
		{tree->rc = word_insert (tree->rc, growth, nunode);
		if (*growth)
			switch (tree->bal)
				{case -1: tree->bal = 0;
					 *growth = 0;
					 break;
				 case 0: tree->bal = 1;
					 break;
				 case 1: t1 = tree->rc;
					 if (t1->bal == 1)
					   {tree->rc = t1->lc;
					   t1->lc = tree;
					   t1->bal = 0;
					   tree->bal = 0;
					   tree = t1;
					    *growth = 0;
					    return(t1);
					    break;}
					else
					   {t2 = t1->lc;
					    t1->lc = t2->rc;
					    t2->rc = t1;
					    tree->rc = t2->lc;
					    t2->lc = tree;
					    if (t2->bal == 1)
						tree->bal = -1;
					    else
						tree->bal = 0;
	 				    if (t2->bal == -1)
						t1->bal = 1;
					    else
						t1->bal = 0;
					    t2->bal = 0;
					    *growth = 0;
					    tree = t2;
					    return (t2);
    					    break;}
			
				}
		}
	else
		{ *growth = 0;
		  *nunode = tree;}
	return(tree);
	
}




struct word *new_word_node()
{
	struct word *wd, *w_malloc();
	char *calloc();
	int   i, word_length;
	int   rootword();
	unsigned size = 1;

	word_length = strlen (token) + 1;
    	if (DEBUG) printf("new_word_node: word_length=%d\n",word_length);
    	if (DEBUG) printf ("new_word_node: token= %s\n", token);
	wd = (struct word *) w_malloc ();
	if (wd != NULL)
                {wd->name = calloc ( (unsigned)word_length, size);
		if (wd->name == NULL)
			printf("ndx: storage exceeded\n     reduce the size of the subject list");
		else
			strcpy (wd->name, token);
		wd->subjs = NULL;
		wd->lc = NULL;
		wd->rc = NULL;
		wd->act = 'N';
		wd->rootp = rootword (token);
		wd->bal = 0;
		if (DEBUG) printf("word:  ");
		if (DEBUG) printf("%s  %c  %d\n",wd->name,wd->act,wd->rootp);
		}
	else
		ERROR1;                            /*storage alloc failure*/
	return (wd);
}




/*add the current (global) subject entry, subject, to the head of the
  subject list, slist*/

struct subj *subj_insert ()

{	struct subj *s;

	if (slist != NULL)
		{s = slist;		/*look for this subj on the list*/
		 while ((s != NULL) && (s_match(subject, s) == FALSE))
			s = s->nxts;
		 if (s != NULL)
			return (s);	/*subj found, return its ptr*/
		}
		subject->nxts = slist;	/*subj not on list, add to head*/
		slist = subject;
		w_update (slist);	/*update all words in this subj*/
		return (slist);

}



/* Compare the entries for two subjects in the subject list, returning 1,0
   if they do,do not match*/

s_match (subj1, subj2)

struct subj *subj1, *subj2;

{	struct s_ele *wd1, *wd2;
	int i = 0;

	if (subj1->type == subj2->type &&
	      subj1->sec == subj2->sec)
		{wd1 = subj1->wds;
		 wd2 = subj2->wds;
		 while ((wd1 == wd2) && (++i <= MAXTERMS))
			{if (wd1 == NULL)
				return (1);		/*match*/
			 else
				{wd1 = wd1->nxte;
				 wd2 = wd2->nxte;}
			}
		}
	return (0);					/*mismatch*/
}




w_update (s)

struct subj	*s;			/*subject pointer*/
{
	struct s_ele	*wds;		/*subjects lists of terms*/

	struct word	*wd;		/*next term*/
	struct subjl	*sbj;		/*word's subject list*/
	char		*malloc ();

	wds = s->wds;					/*get term list*/
	while (wds != NULL)
    	    {wd = wds->ptr;				/*get next term*/
	     sbj = (struct subjl *) malloc (sizeof(struct subjl));	/*create a new list item*/
	     sbj->s_ptr = s;
	     if (wd->subjs == NULL)
		sbj->nxt = NULL;
	     else
		sbj->nxt = wd->subjs;	
	     wd->subjs = sbj;			/*add to head of word's subj list*/

	     wds = wds->nxte;
	     }
}



/*prnt_slist prints the list of subjects and can only be used when
the list is complete since it uses subject, the current subject ptr*/

prnt_slist ()
{
	printf ("\nsubject list:\n");
	subject = slist;
	while (subject != NULL)
		{print_subj();
		 printf("\n");
		 subject = subject->nxts;}
	printf ("\nend of subject lst\n");
}


printtree (tree, c)

char c;
struct word *tree;
{
	if (tree != NULL)
		{printtree (tree->lc,'L');
		 printf("%s  %c\n", tree->name, c);
		 printtree (tree->rc, 'R');
		}
}




#define ERROR3	fprintf (stderr, "\nndx: error3- active list overflow on token %s\n", yytext)
#define ERROR4	fprintf (stderr, "\nndx: error4-  read unexpected eof")
#define ERROR6  fprintf (stderr, "\nndx:Too many subjects (>%d) found in one sentence.\n	Some subjects will not be reported as found here. (error6)\n",MAXACCEPT);

w_activate (w_ptr, m_type)

struct word *w_ptr;			/*pointer to current word*/
char	    m_type;			/*type of match found */
{
	int i;

	if (tot_act < MAXACTIVE-1)
		{if ((i = already_act (w_ptr)) != -1)
		    {if (m_type == 'L')		
			activ_list[i].m_type = m_type;}
		 else
		   {i = tail[new_c];
		    activ_list[i].w_ptr = w_ptr;
		    activ_list[i].m_type = m_type;
		    w_ptr->act = m_type;
		    tail[new_c] = (tail[new_c] +1) MOD MAXACTIVE;
		    ++tot_act;}
		}
	 else
	    ERROR3;
}




int already_act (wptr)

struct word	*wptr;			/*pointer to current word*/
{
	int	i;

	for (i = head[new_c]; i != tail[new_c]; i = (++i) MOD MAXACTIVE)
		{if (activ_list[i].w_ptr == wptr)
		    return (i);}
	return (-1);
}



match_roots (tree, root)

struct word	*tree;			/*top of tree or subtree*/
char		*root;			/*root word*/
{
	struct word  *r_ptr;
	int	len;

	r_ptr = find_root (tree, root);
	if (r_ptr != NULL)
	   {					/*partial match found, is it a root*/
	    len = strlen (root);
	    if (min (r_ptr->rootp, len) < 4)
	        {if (r_ptr->rootp - len == 0)
		    w_activate (r_ptr, 'R');}
	    else
	        {if (abs (r_ptr->rootp - len) <= 1)
		    w_activate (r_ptr, 'R');}
	    match_roots (r_ptr->lc, root);
	    match_roots (r_ptr->rc, root);	/*check for all other matches*/
	   }
}




struct word *find_root (tree, token)		/*is token a leading substring of a
						  subj word or vice versa*/
struct word	*tree;
char		*token;
{
	int	len, nchars, diff, found;
	struct word	*wptr;

	wptr = tree;
	found = FALSE;
	len = strlen (token);
	while (wptr != NULL && found == FALSE)
            {diff = *(wptr->name) - *token;             /*compare 1st letters*/
             if (diff == 0)                     	/*if they match compare roots*/
		   {
		    if (DEBUG) printf ("\nfr: token= %s len= %d word= %s rootp= %d",
			    token, len, wptr->name, wptr->rootp);
		    nchars = min (wptr->rootp, len);
		    diff = strncmp (wptr->name, token, nchars);
		    if (DEBUG) printf ("\nfr: nchar= %d  diff= %d\n",nchars, diff);
		    if (diff < 0)
			wptr = wptr->lc;
		    else if (diff >0)
			wptr = wptr->rc;
		    else
			found = TRUE;
		    }
             else                                       /*1st letters mismatch*/
                if (diff < 0)
                    wptr = wptr->lc;
                else
                    wptr = wptr->rc;
            }
	if (DEBUG) printf("\nfr: returning ptr to %s",wptr->name);
	return (wptr);
}




subj_search()

{	int	 found, i;
	struct word	*wd, *w;		/*word entry in wlist*/
	struct subjl	*sl;		/*subject list for wd*/
	struct subj	*s;		/*subject in subject list sl*/
	struct s_ele	*wl;		/*word list for subjects*/
	char 		m_type;		/*type of match, literal or root*/
	int		Maxacc = FALSE; /*flag, True if accept_list overflows*/

	if (head[old_c] != tail[old_c])		/*if active list isn't empty*/
	    {i = head[old_c];
	     while ( i != tail[old_c] && Maxacc == FALSE)
		{wd = activ_list[i].w_ptr;	/*get next active word*/
		 m_type = activ_list[i].m_type;
		 sl = wd->subjs;		/*get word's subj list*/
		 while (sl != NULL)		/*go thru word's sujb list*/
		    {s = sl->s_ptr;
		    found = FALSE;
		     if (s->ckd == '0')		/*if subj has not been checked*/
		     	{if (s->type == 'N' || m_type == 'L')
			   {wl = s->wds;		/*get list of subj terms*/
			    if (wl == NULL)
				found = FALSE;
			    else 
				found = TRUE;		/*initialize outcome*/
			    while (wl != NULL && found == TRUE)
				{w = wl->ptr;
				 if (w->act == 'N')
				    found = FALSE;	/*inactive term*/
				 else if (w->act == 'R' && s->type != 'N')
				    found = FALSE;	/*insufficient match*/
				 else
				    wl = wl->nxte;	/*get next term*/
				 }
			    }
			 else
				found = FALSE;		/*insufficient match*/
	 		if (found == TRUE)
			   {if (iac < MAXACCEPT-1)
				{
		 		accept_list[++iac] = s;	/*add s to accept list*/
				s->ckd = '1';		/*tag subject as checked*/
				}
			   else
				{
				ERROR6;
				Maxacc = TRUE;
				break;
				}
			   }
	 		else
			   {if (ir < MAXREJECT-1)
				{
		 		reject_list[++ir] = s;		/*add s to reject list*/
				s->ckd = '1';		/*tag subject as checked*/
				}
			   }
			 }
		    sl = sl->nxt;
		    }
		i = (i + 1) MOD MAXACTIVE;
		}
	    }						/*end while*/
}




copy_in (text)			/*copies a string to the context buf*/

char 	*text;			/*string*/
{

	while (*text != '\0')
		fputc (*text++, c_bufw);
		fflush(c_bufw);
}




struct word *find_word (tree, token)

struct word *tree;
char        *token;
{
	struct word *wptr;
	int	fnd;

	wptr = tree;
	while (wptr != NULL && (fnd = strcmp (wptr->name, token)) != 0)
	    {
	     if (fnd < 0)
		wptr = wptr->lc;
	     else
		wptr = wptr->rc;
	     }
	return (wptr);
}




flush_context ()

{	while (c_len[old_c] > 0 )
		{subj_search ();
		 write_context ();
		 wri_subjmacro ();
		 return_lists ();
		 old_c = (old_c + 1) MOD C_SIZE;
		}
	init_vars ();
}


write_context ()

{	int nchars;		/*no. of chars to be copied*/
	char ch;

	nchars = c_len[old_c];
	while (nchars > 0)
	    {
	     ch = fgetc (c_bufr);
	     if (ch == EOF)
		{ERROR4; nchars = 0;}
	     else
		fputc (ch, pagefile);
	     if (DEBUG)
		printf("write context: ch = %c\n", ch);
	     nchars--;
	    }
	c_len[old_c] = 0;
}




wri_subjmacro ()		/*write subj indexing macros to pagefile and*/
				/*reset subject status*/
{
	int	 i;
	char *pg;	/*TROFF/NROFF page register*/

	pg = pagereg;
	for (i = 0; i <= iac; i++)
		if (accept_list[i] == 0)
			fprintf(stderr, "wri_subjmacro: null pointer in accept array, i=%d\n", i);
		else
	    {fprintf (pagefile, ".tm %d %s %s\n", accept_list[i]->id,sectreg,pg);
	     accept_list[i]->ckd = '0';
	    }
}




return_lists ()

{	int i;

	i = ir;
	for ( i = 0; i <= ir; i++)
	    reject_list[i]->ckd = '0';		/*reset status of subjs on reject list*/
	for ( i = head[old_c]; i != tail[old_c]; i = (++i) MOD MAXACTIVE)
	    {activ_list [i].w_ptr->act = 'N';	/*reset match status to none*/
	     tot_act = tot_act - 1;}
	if (new_c != old_c)		/*if they exist, refresh items that*/
					/*will be carried over in new window*/
		for (i = head[new_c]; i != tail[new_c]; i = (++i) MOD MAXACTIVE)
		     activ_list[i].w_ptr->act = activ_list[i].m_type;
	ir = iac = -1;
}




init_vars ()

{
	int	i;

	s_cnt = 0;
	tot_act = 0;
	new_c = old_c = 1;

	for ( i = 0; i < C_SIZE; i++)
		{head[i] = 0;
		 tail[i] = 0;
		 c_len[i] = 0;}
}







min (i, j)

int 	i,j;
{
	if (i < j)
		return (i);
	return (j);
}





ndx_text (pfname)

char	*pfname;
 
{
	char	*tmpnam ();

	tmpnam (tmpbuf);
	c_bufw = fopen (tmpbuf, "w");		/*open buffer for writing*/
	pagefile = fopen (pfname, "a");		/*open paging file for writing*/
	c_bufr = fopen (tmpbuf, "r");		/*open buffer for reading*/
	init_vars ();
}

yywrap ()

{	if (lpass1)				/*if subj file has been processed*/
	    {
		if (s_count > 0)
			{
			subject->type = stype;
			subj_insert();
			prnt_slist();
			printtree (wlist, 'T');
			BEGIN IX;
			lpass1 = FALSE;
			}
		else
			{
			lpass1 = FAILED;
			fprintf(stderr, "\nndx: error6- subject file is empty\n");
			}
	     }
	else
	    {
		subj_search ();
		write_context ();
		wri_subjmacro ();
		return_lists ();
	     }
	return (1);
}




char	*filename = "-";


main (argc,argv)

int	argc;
char	*argv[];

{
	char	*malloc();
	struct word *w_malloc();
	register int rc = 0;
	int     i;

	newnode = (WORDPTR *)malloc (sizeof(WORDPTR *));
	*newnode = (struct word *) w_malloc ();
	growth  = (int *) malloc ((unsigned) sizeof(int));
	fsec    = (int *) malloc ((unsigned) sizeof(int));
	
	BEGIN SL;
	if (argc < 5)
		rc++;
	else
		{for (i = 1; i < 3; i++)
			{if (freopen (argv[i], "r", stdin) == NULL)
				{fprintf (stderr, "\nndx: cannot open %s\n",argv[i]);
				 rc++;}
			 else
				{filename = argv[i];
				 if (lpass1 != FAILED) yylex();}

			if (*argv[4] == 'm') 
			  {
			  pagereg = "\\nP";	/*mm page reg. due to 3b mm bug*/
			  sectreg = "\\n(H1";	/*section reg. for sect-page nos.*/
			  }
			else
			  {
			  pagereg = "\\n%";	/*troff/nroff page register*/
			  sectreg = "0";	/*no section-page numbering*/
			  }
			if (i == 1) ndx_text (argv[3]);
			}
		}
	fclose (c_bufw); fclose (c_bufr);
	unlink (tmpbuf);
	exit (rc);
}
