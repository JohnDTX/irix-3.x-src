#
#include "defs.h"

char	*distfile = NULL;
char    *myname ;       /* What host is this for. */
char	hostname[32];	/* For gethostname(2) if no -h flag */
struct  in_addr myaddr;
struct  hynode *mynode;
int     mynet;
int	debug = 0;	/* debugging flag */
int     oflag;          /* output old style hyroute table. */
int     iflag;          /* install generated table. */
FILE	*fin = NULL;	/* input file pointer */
int	nerrs;

main(argc, argv)
	int argc;
	char *argv[];
{
	register char *arg;
	char *myprog;

	myprog = *argv;
	while (--argc > 0) {
		if ((arg = *++argv)[0] != '-')
			break;
		while (*++arg)
			switch (*arg) {
			case 'h':
				if (--argc <= 0)
					usage();
			        myname = *++argv;
			        break;
			case 'f':
				if (--argc <= 0)
					usage();
				distfile = *++argv;
				if (distfile[0] == '-' && distfile[1] == '\0')
					fin = stdin;
				break;
			case 'd':
				debug++;
				break;
			case 'o':
			        oflag++;
				break;
		        case 'i':
			        iflag++;
				break;
			default:
				usage(myprog);
			}
	}
	if (!myname) {
		gethostname (hostname, sizeof hostname);
		myname = (char *) (strcat (hostname, "-hy"));
	}
	if (fin == NULL) {
		if(distfile == NULL) {
			if((fin = fopen("Hy.net","r")) == NULL)
				fin = fopen("hy.net", "r");
		} else
			fin = fopen(distfile, "r");
		if(fin == NULL) {
			perror(distfile ? distfile : "hy.net");
			usage(myprog);
			exit(1);
		}
	}
	xinit();
	yyparse();
	fclose(fin);
	docmds();
	if (nerrs) fprintf(stderr,"%d errors detected.\n",nerrs);
	exit(nerrs != 0);
}

int            cgroup = 0; /* Points at the group  last created. */
struct hynode *fadapt = NULL; /* First adapter. */

xinit()
{
  struct hostent *h;
  if ((h = gethostbyname(myname)) == NULL) {
    fprintf(stderr,"Unknown host: %s.\n",myname);
    exit(1);
  }
  myaddr.s_addr = ((struct in_addr *)(h->h_addr))->s_addr;
  mynet = inet_netof(myaddr);
}

usage(s)
     char *s;
{
    fprintf(stderr,"Usage: %s [-f input file] [-h hostname]\n",s);
    exit(1);
}

/* This is called after yyparse has set up the state.  It then outputs
 * in the necessary format.  Those supported are?
 *   1. Original glaser driver hyroute table.
 *   2. Install said table.
 *   3. New format list of destinations.
 *   4. Install said table.
 */

extern int mx;

docmds() 
{
  register struct hynode *nd ;
  register int i;
  int ttt, lerrs;

  if (!mynode) {
    fprintf(stderr,"Host %s not in description file.\n",myname);
    nerrs++;
  }
  for (nd = fadapt ; nd != NULL; nd = nd->nd_next) {
    lerrs = ttt = 0;
    for (i = 0 ; i < NTRUNKS ; i++) {
      if ((nd->nd_net[i] == mynode->nd_net[i]) && (mynode->nd_net[i] != 0)) {
	ttt |= 1 << ((NTRUNKS - 1) - i);
      }
    }
    ttt |= ttt << NTRUNKS ;
    ttt = ttt << 8;
    if (!nd->nd_adapter) {
      fprintf(stderr,"No adapter specified for %s.\n",nd->nd_hname);
      lerrs++;
    }
    if (!ttt) {
      fprintf(stderr,"No trunks to try to reach %s.\n",nd->nd_hname);
      lerrs++;
    }
    if (!lerrs) {
      printf("direct  %s",nd->nd_hname);
      for (i = (1 + mx - strlen(nd->nd_hname)); i > 0 ; i--)
	printf(" ");
      printf(" %x  %x  0;\n",nd->nd_adapter,ttt);
    }
    nerrs += lerrs;
  }
}




