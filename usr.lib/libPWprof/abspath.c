char *abspath(p)

char *p;

{
static char Sccsid[] = "@(#)abspath.c	1.1";
int state;
int slashes;
char *stktop;
char *slash="/";
char *inptr;
char pop();
char c;

	state = 0;
	stktop = inptr = p;
	while (c = *inptr)
		{
		 switch (state)
			{
			 case 0: if (c=='/') state = 1;
				 push(&inptr,&stktop);
				 break;
			 case 1: if (c=='.') state = 2;
					else state = 0;
				 push(&inptr,&stktop);
				 break;
			 case 2:      if (c=='.') state = 3;
				 else if (c=='/') state = 5;
				 else             state = 0;
				 push(&inptr,&stktop);
				 break;
			 case 3: if (c=='/') state = 4;
					else state = 0;
				 push(&inptr,&stktop);
				 break;
			 case 4: for (slashes = 0; slashes < 3; )
					{
					 if(pop(&stktop)=='/') ++slashes;
					 if (stktop < p) return(-1);
					}
				 push(&slash,&stktop);
				 slash--;
				 state = 1;
				 break;
			 case 5: pop(&stktop);
				 if (stktop < p) return(-1);
				 pop(&stktop);
				 if (stktop < p) return(-1);
				 state = 1;
				 break;
			}
		}
	*stktop='\0';
	return(p);
}

push(chrptr,stktop)

char **chrptr;
char **stktop;

{
	**stktop = **chrptr;
	(*stktop)++;
	(*chrptr)++;
}

char pop(stktop)

char **stktop;

{
char chr;
	(*stktop)--;
	chr = **stktop;
	return(chr);
}	
