/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#include <stdio.h>
#include "ehash.h"
#include "edict.h"
#define TOKSIZE 32
#define DEBUG_RTWD 0

int	initflg = 0;


/*
   rootword identifies certain suffixes, e.g., -ed, -ing, -es, -s, -'s, -',
   and returns the number of characters in the word prior to the 
   ending, the length of the rootword.
   if the parameter token is not a string, this is an error and
   rootword returns 0.
*/

rootword (token)

char	token[TOKSIZE];

{	
	int	len;			/*length of token*/
	int	n;

	if (initflg == 0) {
		egetd (); 
		initflg = 1;
	}
	if (string (token, TOKSIZE))		/*if token is a string*/ {
		len = strlen (token);
		switch (*(token + len - 1)) {
		case '\'': 
			return (len - 1);
		case 'd' : 
			if (*(token + len - 2) == 'e')
				return (except(ed, token, len, 2));
			else 
				return (len);
		case 'e' : 
			if (*(token + len - 3) == 'n' && 
			    *(token + len - 2) == 'c') {
				if (*(token + len - 4) == 'a') {
					n = except(ance, token, len, 4);
					if (n < 3) 
						return(len);
					else 
						return(n);
				} else if (*(token + len - 4) == 'e') {
					n = except(ence, token, len, 4);
					if (n < 3) 
						return(len);
					else 
						return(n);
				} else 
					return(len);
			}
		case 'g' : 
			if (*(token + len - 3) == 'i' && *(token + len - 2) == 'n') {
				n = except(ing, token, len, 3);
				if (n < 3)
					return (len);
				else
					return (n);
			} else
				return (len);
		case 's' : 
			switch (*(token + len - 2)) {
			case 's': 
				if (*(token + len - 4) == 'n' && 
				    *(token + len - 3) == 'e') {
					n = except(ness, token, len, 4);
					if (n < 3) 
						return(len);
					else 
						return(n);
				} else 
					return(len);
			case 'i': 
				return (len);
			case 'o': 
				return (len);
			case 'u': 
				return (len);
			case 'a': 
				return (len);
			case '\'': 
				return (len - 2);
			case 'e': 
				switch (*(token + len - 3)) {
				case 'i': 
					return (len - 2);
				case 'z': 
					return (len - 2);
				case 'x': 
					return (len - 2);
				case 'o': 
					return (len - 1);
				case 's': 
					if (*(token + len - 4) == 's')
						return (len - 2);
					else 
						return (len - 1);
				case 'h': 
					if (*(token + len - 4) == 's' || 
					    *(token + len - 4) == 'c')
						return (len - 2);
					else 
						return (len);
				}
				return (len - 1);			/*-s ending*/
			}
			return (len - 1);
		case 't' : 
			if (*(token + len - 2) == 'n') {
				switch (*(token + len - 3)) {
				case 'e' : 
					if (*(token + len - 4) == 'm') {
						n = except(ment, token, len, 4);
						if (n < 3) 
							return(len);
						else 
							return(n);
					}
				}
			}
		case 'n' :  
			if (*(token + len - 6) == 'z' && *(token + len - 5) == 'a') { 
				if (*(token + len - 4) == 't' && *(token + len - 3) == 'i' && 
				    *(token + len - 2) == 'o')
					return (len - 5);
			} else 
				return (len);
		}
		return (len);
	} else 
		return (0);				/*token is not a string*/
}



/*
   except, given a suffix, checks to see if the word (token) is on the
   exception list for that suffix. If it is the word has no suffix and
   the length of the root is the length of token, otherwise the
   length of the root is the length of token minus the ending length.
   except returns the length of the "root". For example, the word "deed"
   is on the exception list for the ending -ed and except would return
   4; "cased" is not on this exception list and except would return a
   length of 3.
*/



except (endg, token, len, end)

int	(*endg) ();			/*pointer to a function for this end'g*/
char	*token;				/*the word*/
int	len, end;			/*lengths of token & possible suffix*/

{	
	int	found;
	char	rtok[TOKSIZE];

	strn_rev (token, rtok, (len - end));		/*reverse string minus ending*/
	if (DEBUG_RTWD) 
		printf ("\nexcept: %s", rtok);
	found = (*endg) (rtok, 1, 0);		/*check except list*/
	if (DEBUG_RTWD) 
		printf ("\nexcept: found = %d", found);
	if (found)
		return (len);			/*exception word, has no suffix*/
	else
		return (len - end);

}


