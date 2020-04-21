/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

struct subjl /*subject list for a word*/
{
	struct subj *s_ptr;		/*pointer to a subject*/
	struct subjl *nxt;
};

struct word {
	char	*name;			/*word*/
	struct subjl *subjs;		/*list of subjects*/
	int	rootp;			/*pos. of last char in root word*/
	struct word *lc;		/*left child, alph precursor*/
	struct word *rc;		/*right child, alph successor*/
	char	act;			/*n:no match, r:root match, l:lowercase
						literal match, c:capitalized literal */
	int	bal;			/*balance factor for avl tree*/
};

struct s_ele /*word list for a subject*/
{
	struct word *ptr;		/*pointer to a word in master word list*/
	struct s_ele *nxte;
};


struct subj /*subject entry in master list of subjects*/
{
	struct s_ele *wds;		/*list of words in subject*/
	char	type;			/*c,l,n if subject is cap, literal, neither*/
	char	ckd;			/*0,1 if subject (not) checked in this context*/
	int	id;			/*subject number*/
	struct s_ele *sec;		/*pointer to 1st word in secondary component*/
	struct subj *nxts;		/*pointer to next subect*/
};


