/* 
 * main.c - main procedure for Cypress routing programs.
 * 
 * Author:      Greg Smith
 *              Dept. of Computer Sciences
 *              Purdue University
 * 
 * Date:        Wed Jul 31 1985
 */

#include <stdio.h>
#include <netdb.h>
#include "defs.h"

int Verbose = FALSE;                  /* flags whether verbose routing info   */
                                      /* is generated                         */
int iConnect = 0;                     /* index into connection table          */
int cError = 0;                       /* errors encountered in parsing        */
int cLineNum = 1;                     /* line number in file being parsed     */
int cLineCurr = 0;                    /* count of entries in phone line table */
int cConnectCurr = 0;                 /* count of entries in connection table */
int cAdjListCurr = 0;                 /* count of entries in adjacency list   */
int cHostCache = 0;                   /* count of entries in host cache       */

FILE *fdCurr;
char *sbFileName;
char sbSource[NAMESIZE];                 /* host name to be used as source    */
                                         /* for generating verbose routes     */
int Pred[MAXENTRIES];                    /* used to store minimum cost path   */
struct Line LineTable[MAXENTRIES];       /* phone line table, entries describe*/
                                         /* the lines at a single site        */
struct AdjList AdjListTable[MAXENTRIES]; /* adjacency list table, represents  */
                                         /* the network as a graph, each entry*/
                                         /* is a node in the graph            */
struct Connect ConnectTable[MAXENTRIES]; /* connection table, each entry gives*/
                                         /* the sites that may be reached from*/
                                         /* a particular site                 */

struct host HostCache[MAXENTRIES];       /* cache of hosts and their network  */
                                         /* addresses                         */


main(argc, argv)

int argc;
char *argv[];

{
    char *ch;

    if (argc < 2) 
	Options();
    else {
        /* build the cache of hosts */
        BuildCache();

	/* build the connect and line tables */
	BuildTable();

	if (cError == 0) {

	    /* either generate routing information if no args preceded */
	    /* by a '-' or parse arguments                             */
	    if (argc == 2 && *argv[1] != '-')
		Route(argv[1], Verbose);
	    else {
		/* check for command line options */
		while (--argc > 0 && (*++argv)[0] == '-')
		    for (ch = argv[0] + 1; *ch != '\0'; ch++)
			 switch(*ch) {
			      case 'a' : MapAdjList();
					 break;

			      case 'b' : BiComponents();
					 break;

			      case 'c' : CompAndCycles();
					 break;

			      case 'i' : DisplayConnect();
					 break;

			      case 'l' : DisplayLine();
					 break;

			      case 'p' : PrintPath();
					 break;

			      case 't' : ComputeCostSrcToDest();
					 break;


                              case 'v' : printf("Enter host name: ");
                                         scanf("%s", sbSource);
                                         
                                         /* make sure the entry is valid */
                                         if ((iConnect = SearchConnectTable(sbSource)) < 0)
                                            FatalError("unknown host");
                                         else {
                                            Verbose = TRUE;
                                            Route(sbSource, Verbose);
                                            }
                                         break;                            

			      default  : printf("Invalid option %c\n", *ch);
					 break;
			      }
	    }
    
	/* since there were no errors, update the cache */
	/* of hosts before exiting                      */
	UpdateCache();

	}
    
	else if (cError == 1)
		 fprintf(stderr, "\n\n1 error\n");
	     else fprintf(stderr, "\n\n%d errors\n", cError); 

	exit(cError);                   
    }
}

/*
 * Options - display list of options.
 * 
 */

Options()

{
    printf ("\noptions:\t");
    printf ("a - display adjacency list\n");
    printf ("\t\tb - display biconnected components\n");
    printf ("\t\tc - display connected components\n");
    printf ("\t\ti - display cypress connection table\n");
    printf ("\t\tl - display line table\n");
    printf ("\t\tp - display path between two Cypress sites\n");
    printf ("\t\tt - display per packet cost between two Cypress sites\n");
    printf ("\t\tv - display verbose routing information for a Cypress site\n");
}
