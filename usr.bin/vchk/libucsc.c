/*
 * (C) 1983 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

/* LIBUCSC functions --- DO NOT FIX BUGS HERE.  FIX THEM IN THE ACTUAL LIBRARY
 * and then re-install them in this file.
 * @(#)libucsc.c	1.7
 */
#ifdef PWB
#define mc68000
#include "v3.h"
#include "fcntl.h"
#endif

#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <errno.h>


extern errno;
extern char *sys_errlist[];
#define SE sys_errlist[errno]
#define FLIM 10
/* With the exception of malloc's which have been changed to getmem's
 * these are exactly as they are in libucsc
 * Please don't fix bugs here, but in libucsc and then here.
 */
char *pindex();

char *
cindex(s, p)
	register char *s, *p;
{
	register char *r;
	register char c;
/* printf("cindex(`%s',`%s')\n",s,p); */

	while (c = *s++) {
		if (c == '\\') {
			if (!*s++) break;
		} else
			for (r=p; *r; r++)
				if (c == *r) return s-1;
	}
	return 0;
}

char *
pindex (s, c)
	register char *s;
{
	register char v;
/* printf("pindex(`%s',`%c')\n",s,c); */
	if (!s) return 0;
	while (v = *s++) {
		if (v == '\\') {
			if (!*s++) break;
		} else if (v == c) return s-1;
	}
	s--;
	return c ? 0 : s;
}

/*	 CPS (receiver pointer, string to copy)
 *  This function copies a given string into another, up to and including
 *  the '\0'.  The number of characters (not including the delimiter)
 *  copied is returned.  Cps always does the copy in a way that prevents
 *  the 'ripple' effect that can result if not careful.
 */

int cps (A,B) register char *A, *B;
{    register int n;  int l;		/* initialize char counter	     */
     char *b = B;			/* hold copy of B in to compute len  */
  if (!A || !B) return 0;		/* if either is null return no copy  */
  if (A <= B)				/* reciever comes befor sender       */
  { do *A++ = *B; while (*B++);		/* Copy the string over until delim  */
    return B - b - 1;			/* return length copied		     */
  }
  while (*B++);				/* find length of B string	     */
  for (l=n=B-b-1, B=b; n>=0; n--)	/* prevent ripple effect by copying  */
    A[n] = B[n];			/*  the string backwards	     */
  return l;				/* return count			     */
}

int divs (x, ds)  char *x, *ds;
{    char k, *d, *l, *p, **r, ***q;
     unsigned int *m, n, s, t, z, Z = (sizeof (unsigned int));
     int i;
     union { int i, *pi;  char *pc, **ppc, ***pppc; } c;
  if (!(z=strlen(x)))     return  0;	/* null str. so exit without hassel  */
  s = strlen(ds) + 1;			/* number of levels of indirection   */
				/* get two words for each level and  */
  if ((c.i=getmem(s * Z * 2,0,"divs","internals")) == 0)
    return NULL;			/*  die after cleaning up if can't   */
  m = (unsigned *) c.pi;		/* m will hold # of chars for each ds*/
  for (i=s+s-1; i>=0; i--) m[i] = 0;	/* clear core since alloc doesn't    */
  l = x+z-1;				/* save last character for end check */
  for (p=x; *p; p++)			/* for each character in the string  */
    for (d=ds; *d; d++)			/* for each of the division set chars*/
      if (*p == *d) m[d-ds]++;		/* tally m[div char] for all of str. */
  if (*l != *ds) m[0]++;		/* last chr of str. should be 1st ds */
  t = m[s] = m[0] + 1;			/* number of words for first list    */
  for (i=1; i<s-1; i++)			/* for each remaining div character  */
  { t += (m[s+i] = m[i] + (m[i-1]*2));	/* # of words for ith div char list  */
    m[i] += m[i-1];			/* # non-null ptrs at the ith level  */
  }
  n = t * Z;				/* total # of bytes to allocate      */
  r = c.ppc;				/* make all m values pointers        */
  q = c.pppc;				/* also need way to set ptrs in m buf*/
			/* alloc space for pointers and str. */
  if ((c.i=getmem(n,1,"divs","data space")) == 0)
    return  free(m), -1;		/* clean up and return error if can't*/
  m[0] = c.i;				/* pointer to start of area	     */
  for (i=1; i<s-1; i++)			/* setup pointers to resp div lists  */
    m[i] = (m[s+i-1]*Z) + m[i-1];	/* set ptr for each type of divider  */
  for (i=0; i<s-1; i++) m[s+i] = m[i];	/* set 'memory' pointers to primarys */
  r[s+s-1] = x;				/* set pointer to string as last seg */
  for (r[s-1]=x; *r[s-1]; r[s-1]++)	/* for each char in string	     */
    for (d=ds; *d; d++)			/* for each of the division set chars*/
      if (*r[s-1] == *d)		/* if we have a division character   */
	for (i=s-2; i>=d-ds; i--)
	{ k = *d;
	  *q[i]++ = r[s+i+1];
	  if (i == s-2)
	  { *r[s-1] = '\0';
	    r[s+s-1] = r[s-1] + 1;
	  } else
	  { *q[i+1]++ = 0;
	    q[s+i+1] = q[i+1];
	  }
	}
  if (x[z-1] || k != *ds)
    for (i=s-2; i>=0; i--)
    { *q[i]++ = r[s+i+1];
      if (i != s-2) *q[i+1] = 0;
    }
  if (s == 1)
  { free(c.i);
    c.pc = x;
  } else *q[0] = 0;
  free(m);
  return c.i;
}

exec (comm) char *comm;
{    int s, c, p;
#ifdef	vax
  if (p = vfork())
#else
  if (p = fork())
#endif
  { if (p == -1) return 077;
    while ((c = wait(&s)) != p)
      if (c == -1) return 0377;
    return (s >> 8) & 0377;
  } else
  {    register char **cm = (char **)divs(comm," "),
                      *m  = cm[0],
		      *n  = m + strlen(cm[0]);
    if (cm == (char **) NULL) exit(3);
    while (n > m && *n != '/') n--;
    if (n > m) cm[0] = n + 1;
    setuid(0);
    setgid(0);
    execvp (m, cm);
    exit(1);
  }
}
/*	FNAME (pathname)	-- return a pointer to the last segment
 */

char * fname (p) register char *p;
{	char *rindex();
	register char *q;
  
  if (!(q = rindex(p,'/'))) return p;
  else return q+1;
}

int loadfyl (fn,ds)  char *fn, *ds;
{    char k, *b, *d, **r, ***q;
     int i, n;
     unsigned int s, t, z, Z = sizeof (unsigned int);
     unsigned int *m;
     FILE *f;
     struct stat sb;
     union { unsigned int i, *pi;  char **ppc, ***pppc; } c;

  if (stat(fn,&sb) == -1) return NULL;	/* file not there or permission chk  */
  if ((z=sb.st_size)== 0) return NULL;	/* file is much too large or weird   */
  s = strlen(ds) + 1;			/* number of levels of indirection   */
  if ((f = fopen(fn,"r")) == NULL)	/* attempt to open 'fn' for reading  */
    return NULL;			/* open file and die if we can't     */
	/* get two words for each level and  */
  if ((c.i=getmem(Z * s * 2,0,"loadfyl","internals")) == NULL)
    return fclose(f), NULL;		/*  die after cleaning up if can't   */
  m = c.pi;				/* m will hold # of chars for each ds*/
  for (i=s+s-1; i>=0; i--) m[i] = 0;	/* clear core since alloc doesn't    */
  while ((n = getc(f)) != EOF)		/* scan file summing # of 'ds' chars */
  { for (d=ds; *d; d++)			/* for each of the division set chars*/
      if (n == *d)			/* if file char is a divisor char    */
      { m[d-ds]++;			/* tally m[div char] for all of file */
	break;				/* err to continue checking (dups)   */
      }
  }
  if (n != *ds) m[0]++;			/* last chr of file should be 1st ds */
  t = m[s] = m[0] + 1;			/* number of words for first list    */
  for (i=1; i<s-1; i++)			/* for each remaining div character  */
  { t += (m[s+i] = m[i] + (m[i-1]<<1));	/* # of words for ith div char list  */
    m[i] += m[i-1];			/* # non-null ptrs at the ith level  */
  }
  n = (t * Z) + z + 1;			/* total # of bytes to allocate      */
  r = c.ppc;				/* make all m values pointers        */
  q = c.pppc;				/* also need way to set ptrs in m buf*/
				/* alloc space for pointers and file */
  if ((c.i = getmem(n,0,"loadfyl","data space")) == NULL)
    return fclose(f), free(m), NULL;	/* clean up and return error if can't*/
  m[0] = c.i;				/* pointer to start of area	     */
  for (i=1; i<s; i++)			/* setup pointers to resp div lists  */
    m[i] = (m[s+i-1] * Z) + m[i-1];	/* set ptr for each type of divider  */
  for (i=0; i<s; i++) m[s+i] = m[i];	/* set 'memory' pointers to primarys */
  rewind(f);				/* rewind file, read in again        */
  if ((n=read(fileno(f),b=r[s-1],z))==-1)
    return fclose(f), free(c.i), free(m), NULL;
  fclose(f);				/* and then close it.		     */
  if (n != z)				/* if file has changed size on us ...*/
    return free(*m), free(m), NULL;	/*   free memory and return failed   */

  for (; r[s-1]<b+n; r[s-1]++)		/* for each character in the file ...*/
    for (d=ds; *d; d++)			/* for each of the division set chars*/
      if (*r[s-1] == *d)		/* if we have a division character   */
      { for (i=s-2; i>=d-ds; i--)
	{ k = *d;
	  *q[i]++ = r[s+i+1];
	  if (i == s-2)
	  { *r[s-1] = '\0';
	    r[s+s-1] = r[s-1] + 1;
	  } else
	  { *q[i+1]++ = 0;
	    q[s+i+1] = q[i+1];
	  }
	}
	break;
      }

  if (b[z-1] || k != *ds)
    for (i=s-2; i>=0; i--)
    { *q[i]++ = r[s+i+1];
      if (i == s-2) *r[s-1] = '\0';
      else *q[i+1] = 0;
    }

  if (s > 1) *q[0] = 0; else *r[0] = '\0';
  free(m);
  return c.i;
}
/*	MINPATH (pathname)
 *  This function takes a pointer to a pathname and deletes all redundant
 *  information (like, /./'s, name/..'s, and extra /'s) from it without
 *  changing the meaning of pathname.  The function returns -1 if there are
 *  enough '..'s to backup past the root (and the pathname starts at the root).
 *  Zero is returned if the path is ok.
 */

minpath(s) register char *s;
{	char *S, *b, *e;
	int root = 0;
	register char *p, *n;
  /* hack added for vchk to allow preceeding ./ in pathnames */
	if (*s == '.' && s[1] == '/') s += 2;
  b = s;
  if (*s == '/') s++;
  if (s != b) root++;
  S = s;
  while (*s)				/* more segments to path */
  { n = s;				/* s is start of next segment */
    if (*n == '/')
    { while (*n && *n == '/') n++;
      e = s;
      while (*e++ = *n++) ;
      continue;
    }
    while (*n && *n != '/') n++;
    if (*n) n++;
    if (*s == '.')
    { if (s[1] == '/' || s[1] == '\0')
      { p = s;
	while (*p++ = *n++) ;
	continue;
      }
      if (s[1] == '.' && (s[2] == '/' || s[2] == '\0'))	/* encountered .. */
      { if (s == S && root) return -1;
	if (b < s)
	{ p = b;
	  while (*p++ = *n++) ;
	  if (b > S)				/* if b can be backed up */
	  { b -= 2;
	    while (b > S && *b != '/') b--;
	  }
	  if (*b == '/') b++;
	  n = b;
	} else b = n;				/* skip over perm .. */
	s = n;
	continue;
      }
    }
    if (b < s) b = s;
    s = n;
  }
  while (*--s == '/' && s >= S) *s = '\0';	/* erase trailing /'s */
  if (!root && *S == '\0')
  { *S++ = '.';
    *S = '\0';
  }
  if (*S == '/')
  { e = S + 1;
    while (*S++ = *e++) ;
  }
  return 0;
}

char * mkdir (s)  register char *s;
{	int pd[2], lnr = 0, nr = 0, fk = 0, u, x, r, t, fd;
	char *sprintf(), *rv, buf[256];
	static char ebuf[256];
/*
  if (!geteuid())
  { u = umask(0777);
    umask(u & 0777);
    t = strlen(s);
    if (s[t-1] == '/') return "filename ends in / ";
    if (strlen(s) > sizeof buf - 4)
      return sprintf(ebuf,"filename too long [> %s chars]",sizeof buf - 4);
    strcpy(buf,s);
    strcat(buf,"/.");
    if (mknod(s,(S_IFDIR|0777),0)) return "mknod";
    if (ln(s,buf))
    { unlink (s);
      return "linking .";
    }
    strcat(buf,".");
    if (p = rindex(s,'/'))
    { *p = '\0';
      x = ln(s,buf);
      *p = '/';
    } else x = ln(".",buf);
    if (x)
    { buf[t+2] = '\0';
      unlink(buf);
      buf[t] = '\0';
      unlink(buf);
      return "linking ..";
    }
    return (char *)0;
  }
*/

  if (pipe(pd)) return "making pipe";

  while ((t = fork()) == -1)
  { if (fk++ <= FLIM && errno == EAGAIN) continue;
    return "fork";
  }

  if (t)
  { close(pd[1]);
    while ((nr += read(pd[0],buf+nr,sizeof buf - nr)) > lnr) lnr = nr;
    close(pd[0]);
    rv = buf;
    if (!strncmp(buf,"mkdir: ",7)) rv = buf+7;
    x = strlen(rv) - 1;
    if (rv[x] == '\n') rv[x] = '\0';
    while ((x = wait(&r)) != t)
      if (x == -1) return sprintf(ebuf,"mkdir never finished(%s): %s",SE,rv);
    if (r) return sprintf(ebuf,"mkdir failed(%d): %s",r,rv);
    return (char *)0;
  }

  close(pd[0]);
#ifdef PWB
  close(1);
  if (fcntl(pd[1], F_DUPFD, 1) != 1) {
	printf("fcntl pd[1] dup failure\n");
	return (char *)0;
  }
#else
  dup2(pd[1],1);
#endif

  close(pd[1]);
#ifdef PWB
  close(2);
  if (fcntl(1, F_DUPFD, 2) != 2) {
	printf("fcntl 1 dup failure\n");
	return (char *)0;
  }
#else
  dup2(1,2);
#endif

  close(0);
  if ((fd = open("/dev/null",0)) != 0)
#ifdef PWB
	  if (fcntl(1, F_DUPFD, 0) != 0) {
		printf("fcntl 0 dup failure\n");
		return (char *)0;
	  }
#else
	  dup2(1,0);
#endif
  for (x=3; x<20; x++) close(x);
  setuid(geteuid());
  setgid(getegid());
  execl("/bin/mkdir","mkdir",s,0);
  write(1,"execl failed: ",14);
  write(1,SE,strlen(SE));
  exit(99);
}

char *nth(l) short l;			/* CNVT 1 to 1st, 2 to 2nd, etc. */
{    static char *nthtab[4] = { "th", "st", "nd", "rd" };
     static char buf[10], *pat = "%d%s";
     short k = l % 100;
  if (l < 0)  { l = -l;  k = l % 100;  pat = "-%d%s";  }
  if ((k >= 11) && (k <= 13)) k = 0;
  else if ((k %= 10) > 3) k = 0;
  sprintf(buf,pat,l,nthtab[k]);
  return buf;
}
#define when break; case
#define otherwise break; default

/* PCHR (character) Print CHaRacter no matter what it's value in a readable way.
 */

char *pchr(c) char c;
{	static char r[4];		/* r is to return created string */
	register char *p = r;		/* p is set to access r */

  if (c & 0200) *p++ = '`';
  c &= 0177;
  if (c >= ' ')
    switch (c)
    { when '\\': *p++ = '\\'; *p++ = '\\';
      when '`':  *p++ = '\\'; *p++ = '`';
      when '^':  *p++ = '\\'; *p++ = '^';
      when 0177: *p++ = '\\'; *p++ = '~';
      otherwise: *p++ = c;
    }
  else
  { *p++ = '^';
    *p++ = c + '@';
  }
  *p++ = '\0';
  return r;
}
extern char LPRENS[];
extern char RPRENS[];

#define NKP 40		/* should be the same as NBRKT in vchk.h */
char *
pmatch (s)
	register char *s;
{	int pt[NKP];		/* maximum number of kinds of parenthesis */
	static char prens[1+NKP<<1];
	static int nkp;
	char *index();
	register char *i, *n;
	int t;
	register int j;

	if (!nkp) {		/* initialize prens list */
		nkp = cps(prens,LPRENS);
		cps(prens+nkp,RPRENS);
	}

	for (j=0; j<nkp; j++)
		pt[j] = 0;

	if (!(i = index(LPRENS,*s)) || !*i)
		return 0;
	t = i - LPRENS;
	pt[t] = 1;

	while (*++s) {
		if (*s == '\\') {
			if (*++s == 0)
				return 0;
			continue;
		}
		if ((i = index(prens,*s)) == 0) continue;
		t = i - prens;
		if (t >= nkp) {
			t -= nkp;
			--pt[t];
			for (j=0; j<nkp; j++)
				if (pt[j]) break;
			if (j >= nkp)
				return s;
		} else
			pt[t]++;
	}
	return 0;
}
/* Note: The variable 'nc' (for number of characters) is set initially so
 *       that if any of the filenames get longer between the two passes they
 *       will still be able to fit in the space allotted (dont rely on it).
 */

#define NS 14
#define CN (char ***) NULL
char *** rdir (s)  char *s;				   /* read directory */
{    register char *bp, *ep, *cp;			   /* file name point*/
     char **lp;	  int lr;				   /* ret from rdir  */
     union {  char ***d,  **r,  *w;   int ap;  } a;	   /* cvt pointer typ*/
     unsigned int i, fd, siz, nw=0, nc=NS*5;		   /* dir index, tmps*/
     unsigned int Z = sizeof(unsigned);			   /* mach dependant */
     struct {short ino;  char nm[NS];}	 dblk[32];	   /* dir block=32 en*/
  if	((s == 0) || (*s == '\0'))	return CN;	   /* default dir=.  */
  if	((fd = open(s,0)) == -1)	return CN;	   /* die if no file */
  while	((lr=read(fd,dblk,512)>>4)>0)	/* while more dir entries to check   */
  { if (lr == -1) return close(fd), CN;	/* die if read error		     */
    for   (i=0; i<lr; i++)		/* for each entry: determine if void */
      if (dblk[i].ino)			/* if this is a non-null entry.	     */
      { nw++;  ep= (bp=dblk[i].nm) +NS;	/* compute beginning and ending ptrs */
	while (*bp&&(bp++<ep));
	nc += NS - (ep - bp) + 1;	/* update character count.	     */
      }
  }
  siz = ((nw + 3) * Z) + nc;		/* compute number of chars to alloc  */
  if ((a.ap=getmem(siz,0,"dir list",s))==NULL)
    return close(fd), CN;		/* allocate memory or die if not enou*/
  *a.d = a.r + 2;			/* set major start& end of word list */
  lp= a.d[0];  cp=(char *)(*a.d+nw+1);	/* setup pointers to list and chars  */
  lseek (fd, 0l, 0);			/* back to beginning of file	     */
  while ((lr=read(fd,dblk,512)>>4)>0)	/* while there is more stuff in dir  */
  { if (lr == -1)			/* read error check (should never be */
      return free(a.ap), close(fd), CN;	/* free area and close directory     */
    for (i=0; i<lr && lp-*a.d<nw; i++)	/* for each entry in this block      */
      if (dblk[i].ino)			/* if it is a non-null one.	     */
      { *lp++=cp; ep=(bp=dblk[i].nm)+NS;/* setup pointers for copy operation */
	while (*bp && (bp < ep))	/* copy NS or less characters.       */
	  *cp++ = *bp++;		/* copy in char by char. not delim   */
	*cp++ = '\0';			/* terminate each filename with a nul*/
      }	
  }
  *lp = 0;  a.d[1] = lp;		/* terminate list of file names      */
  return close(fd), a.d;		/* return what alloc returned so free*/
}

#define DS	(sizeof d)
#define Sd	(sizeof (struct direct))
#define DZ	(BUFSIZ / sizeof (struct direct))
#define DNL	(sizeof d[0].d_name)
#define R return cps(path,*(p+1) ? p+1 : ".")
#define len(x) cps(x,x)			/* cps with same arg just rets length*/
#define NSL -1	/* No Space Left in 'path' buffer -- buf short or path long  */
#define CSD -2	/* Can't Stat Dot -- need inumber for current directory	     */
#define CCD -3	/* Can't Change Directory- chdir ("..") failed.  (privledg?) */
#define ERD -4	/* Error Reading Directory -- current directory can't be read*/
#define CFE -5	/* Can't Find Entry for current directory in parent (weird)  */
#define COP -6	/* Can't Open Parent directory. Protection, Too many files ..*/
#define CSP -7	/* Can't Stat DOTDOT -- need inumber for current directory   */
#define CSR -8	/* Can't stat "/" !!!  something is very very wrong!         */

gadn (path, size) char *path; int size;			/* Get Ascii Dir Name*/
{    struct stat c, b;					/* inode buffer	     */
     struct direct d[DZ+1];				/* directory block   */
     extern char *rjc();				/* Right Justify Copy*/
     int nr, fd;
     dev_t rdev;					/* dev is min + maj  */
     ino_t rino;
     static char S= '/', N= '\0',			/* Slash, Null	     */
	 *cd= ".", *pd= "..";				/* useful constants  */
     register int r, l = 2; 				/* various stuff, len*/
     register char *p;					/* right just path   */
     char nbuf[DNL+2];					/* dir name buffer   */

  if (!path || size-- < 2) return 0;			/*Rediculous argument*/
  *(p = path + size) = N;
  if (stat("/",&c)) R, CSR;				/* find root device  */

  rdev = c.st_dev;
  rino = c.st_ino;

  while (1)						/* until root is cd  */
  { if (stat(cd,&c) == -1) R, CSD;			/*Can't Stat cur Dir */
    if (c.st_ino==rino && c.st_dev==rdev)
    { *--p = S;
      if (!p[1]) p--;
      R;
    }
    if ((fd = open (pd, 0)) != -1)			/* open parent, read */
    { if (fstat(fd,&b)) R, CSP;				/* can't stat parent */
      if (chdir(pd)== -1) R, CCD;			/*Can't Change Direct*/
      while (nr = read(fd,d,DS) / Sd)			/* block at a time   */
      { if (nr == -1) R, close(fd), ERD;		/*Err Reading Direct.*/
        if (c.st_dev == b.st_dev) 
	{ for (r=0; r<nr; r++)				/* for next entry ...*/
	    if (d[r].d_ino == c.st_ino)  goto F;	/* Found right entry */
	} else
	for (r=0; r<nr; r++)
	{ strncpy(nbuf,d[r].d_name,DNL + 1);
	  nbuf[DNL] = N;
	  if (stat(nbuf,&b)) continue;
	  if (b.st_ino==c.st_ino && b.st_dev==c.st_dev) goto F;
	}
      }							/* else get next blk */
      R, CFE;						/*Can't Find Entry   */
    F:close(fd);  d[r].d_name[DNL] = N;			/* close dir, del nam*/
      if ((l += len(d[r].d_name)) > size) R, NSL;	/*No Space Left again*/
      p = rjc(p,d[r].d_name,N);  *--p = S;  l++;	/* copy in and rep S */
    } else R, COP;					/*Can't Open Parent d*/
  }
}
char *rjc (r, t, d) register char *r, *t;  char d;
{
     char save = *r;
     int l;

  l = cpu(t, t, &d);
  cpu(r-l, t, &d);
  *r = save;
  return r-l;
}
/*	CPU (reciever string, sending string, set of delimiters (string))
 *  This procedure will copy the sending string into the reciever up to and
 *  including any one of the delimiters in the delimiter string.
 *  The actual number of characters in the reciever up to the delimiter
 *  is returned.
 */

int cpu (x, y, d)			/* CoPy Until (any one of set 'd')   */
     register char *x, *y, *d;		/* reciever, sender, set of delimiter*/
{    char *D = d;			/* save start of delimiter string;   */
     char *Y = y;			/* save start of transmitter str.    */

  while (*y)				/* while still chars in y string.    */
  { for (d=D; *d; d++)			/* check for each of the delimiters  */
      if (*d == *y)			/* if delim is there then use it     */
      { *y = '\0';			/* change real delim to system delim */
	x[cps (x,Y)] = *d;		/* copy string and replace character */
	*y = *d;			/* replace the original string also  */
	return y - Y;			/* return number of characters in str*/
      }
    y++;				/* next char of transmitter string   */
  }
  return cps(x,Y);			/* use '\0' if delimiter isn't there */
}

char *tvtods(t) time_t t;
{	struct tm *tv, *localtime();
	static char buf[14];
	register char *p;

  tv = localtime(&t);
  p = buf;
  tv->tm_mon++;
  *p++ = tv->tm_year / 10;
  *p++ = tv->tm_year % 10;
  *p++ = tv->tm_mon / 10;
  *p++ = tv->tm_mon % 10;
  *p++ = tv->tm_mday / 10;
  *p++ = tv->tm_mday % 10;
  *p++ = tv->tm_hour / 10;
  *p++ = tv->tm_hour % 10;
  *p++ = tv->tm_min / 10;
  *p++ = tv->tm_min % 10;
  *p++ = tv->tm_sec / 10;
  *p++ = tv->tm_sec % 10;
  *p = '\0';
  for (p=buf; p<buf+12; p++)
    *p += '0';
  return buf;
}

#define TIMEZONE 8		/* hours behind GMT */
#define	YR 0
#define MO 1
#define DY 2
#define HR 3
#define MN 4
#define SD 5
#define ERRTHRESH	6
#define NOTDIGIT	7
#define BADYEAR		8
#define BADMONTH	9
#define BADDAY		10
#define BADHOUR		11
#define BADMIN		12
#define BADSEC		13

/* Date String TO Time Vector (human date string)  -- Function to convert a
 *				string of the form YYMMDDhhmmss (localtime)
 *				into an integer value (time_t) (the number of
 *				seconds since the epoch -- GMT.)
 */
				
time_t dstotv (hd)  register char *hd;
{	short p, v, l;
	time_t  r;
	struct tm *tz, *localtime();		/* used to correct for dst */
	static short mlen [] = { 31,  59,  90, 120, 151, 181,
				212, 243, 273, 304, 334, 365 };

  for (p=YR; p<ERRTHRESH; p++)
  { if   (*hd<'0' || *hd++>'9' ||
          *hd<'0' || *hd++>'9')	p  = NOTDIGIT;
    				v  = (hd[-2] - '0') * 10 + hd[-1] - '0';
    switch (p)
    { when YR:			v -= 68;
		       if (v<2) p  = BADYEAR;
	          		l  = (v%4 == 0) ? 1 : 0;
				r  = (time_t)((v-2)*365 + (v-1)/4);
      when MO: if (v<1 || v>12) p  = BADMONTH;
		       if (--v)	r += (time_t)(mlen[v-1]);
	             if (v > 1) r += (time_t)l;		   /* add leap day */
      when DY: if (v<1 || v>31)	p  = BADDAY;
				r += (time_t)(v-1);
      when HR:        if (v>23)	p  = BADHOUR;
				r  = r * 24L + (time_t)v;
				r += (time_t)(TIMEZONE);
      when MN:	      if (v>59)	p  = BADMIN;
				r  = r * 60L + (time_t)v;
      when SD:	      if (v>59)	p  = BADSEC;
				r  = r * 60L + (time_t)v;
    }
  }
  if (p != ERRTHRESH) return (time_t)(- (p - ERRTHRESH));
  tz = localtime(&r);
  if (tz->tm_isdst) r -= 3600L;
  return r;
}

char *day[] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday",
		"Friday", "Saturday" },
     *mont[] = { "January", "February", "March", "April", "May", "June", "July",
		"August", "September", "October", "November", "December" };

char *fmtym(t) time_t t;		/* FORMAT THE TIME NICELY */
{    static char buf[256];  extern char *nth();
     struct tm *tvec;  char m;
     extern struct tm *localtime();
  
  tvec = localtime(&t);
  m = 'p';
  if (tvec->tm_hour < 12) m = 'a';
  else tvec->tm_hour -= 12;
  if (tvec->tm_hour == 0) tvec->tm_hour = 12;
  sprintf(buf,"%s, %s %s %d, at %d:%02d %cm",day[tvec->tm_wday],
      mont[tvec->tm_mon], nth(tvec->tm_mday), 1900+tvec->tm_year,
      tvec->tm_hour, tvec->tm_min, m);
  return buf;
}
