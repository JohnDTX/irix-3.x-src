char line[100];
short ix;

getchr()
{
   short c;

  if ((c=getchar())==015) putchar(c=012);
#ifdef PM2
  if (c==022) {initscreen(); putchar(c=012);}
#else
#  ifndef PM3
  else putchar(c);
#  endif
#endif
  return(c);
}

getlin(prompt)
   char prompt;
{
  char c;
  int si;
  short newline=0;

  si=ix;
  while ((c=getchr()) != '\n' )
    {
	if (c != ' ' && c != '\t') line[si++]=c;	/* strip blanks  */
	if (c == 025)					/* ctrl U	*/
	     {
		si=ix;
		putchar(012);
		newline=1;
	     }
	if (c==010 || c==0177)		/*  rubout or backspace  */
	  {
	    if (c==0177) putchar(010);	/* back up for real	*/
	    if ( si>(ix+1))
		{
		   si-=2;
		   putchar(' ');
		   putchar(010);
		}
	    else
		{
		  si=ix;
		  if (!newline) putchar(prompt);
		  else putchar(' ');
		}
	  }
    }
  line[si++]=c;			/* append car. ret and EOF	*/
  line[si]=0;
}
