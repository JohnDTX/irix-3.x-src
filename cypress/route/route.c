#include "defs.h"

/* route.c - invoked to construct the cypress and internet routing tables
 *           for the node that is passed as an argument.
 *
 * author - Greg Smith
 *
 * date - 30 June 1985
 *
 * modification history 
 *
 * 9-11-85 - modified to comply with new form of line names, e.g.
 *	     purdue1-cypress/arizona-cypress
 *
 * 10-4-85 - modified to generate non-verbose output as default.
 *
 */

Route(sbSource, Verbose)

char *sbSource;    /* source node name consisting of the node name */
		   /* only, in the form name-cypress               */
int Verbose;       /* flags whether verbose routing info is to be  */
                   /* generated                                    */

{
    extern struct Connect ConnectTable[];
    int SearchConnectTable();   /* searches the connection table for */
				/* a given name, returns position in */
				/* connection table		     */
    int SearchAdjListTable();   /* searches the adjacency table for  */
                                /* a given network number, returns   */
                                /* position in adjacency table       */
    int ConnectSrcInd;          /* index of source in connect table  */
    int AdjSrcInd;	        /* index of source in adjacency list */
    int cLocalNets = 0;         /* number of local networks contained*/
				/* in Internet routing table         */
    char sbNodeName[50];        /* source site name in the form      */
				/* sitename-cypress                  */
    char sbMsg[90];             /* used to display error message     */

    /* make a copy of the source location */
    strcpy(sbNodeName, sbSource);

    /* find the position of the source in the */
    /* connection table                       */
    ConnectSrcInd = SearchConnectTable(sbNodeName);
    if (ConnectSrcInd < 0) {
        /* we've been asked to supply routing */
        /* information for an unknown site    */
	sprintf(sbMsg, "source site %s is not found in connect.dat", sbSource);
        FatalError(sbMsg);
        return(ERROR);
        }
   else {
        /* find the source node's position in */
	/* the adjacency list		      */
        AdjSrcInd = SearchAdjListTable(ConnectTable[ConnectSrcInd].sbNetNumber);

        /* generate the Internet and Cypress routing tables */
        CreateCypressTable(AdjSrcInd, sbNodeName);
        OutputCypressTable(AdjSrcInd, ConnectSrcInd, sbNodeName, Verbose);
        }
}

/*
 * CreateCypressTable - constructs the shortest path first tree for the node 
 *     passed as an argument using the AdjList and Connect tables.  Returns -1
 *     -1 if unable to construct tables.
 *
 *     Algorithm used is Dijkstra's algorithm as given in "graphs, networks,
 *     and algorithms" by Swamy & Thulasiraman, Wiley Interscience Press, 
 *     pp 494.
 *
 */

CreateCypressTable(AdjSrcInd, sbSourceName) 

int AdjSrcInd;	      /* position of source node in adjacency list */
char *sbSourceName;   /*     name in the form of cypress-node      */

{
    extern int cConnectCurr;    /* number of entries in connect table*/
    extern int cAdjListCurr;    /* number of entries in Adjacency tbl*/
    int LineWeight();           /* returns the weight associated with*/
                                /* the line between the two arguments*/
                                /* and INFINITY is there is no line  */
    int FindMin();              /* finds the vertex with minimum     */
                                /* label that is not marked permanent*/
 
    /* for each of the following arrays, position */
    /* i in the array corresponds to position i in*/
    /* the AdjList array                          */
    int Perm[MAXENTRIES];       /* array indicating whether a vertex */
                                /* has been permanently marked       */
    int Label[MAXENTRIES];      /* array containing the cost from the*/
                                /* current vertex to this vertex     */
    extern int Pred[];          /* predecessor array, Pred[i] is vtx */
				/* i's predecessor in the network    */

    int MinCost;                /*     minimun cost of a path        */
    int Iteration;              /*   iteration through the loop      */
    int NewMin;                 /*   new minimum value of label      */
    int LastLabeled;            /*  last vertex labeled permanent    */
    int i;                      /*       loop index variable         */

    /* initialize */
    cAdjListCurr = cConnectCurr;
    LastLabeled = AdjSrcInd;
    Iteration = 0;
    Label[AdjSrcInd] = 0;
    Perm[AdjSrcInd] = TRUE;
    Pred[AdjSrcInd] = AdjSrcInd;

    for (i = 0; i < cConnectCurr; i++)
         if (i != AdjSrcInd) {
             Label[i] = INFINITY;
             Perm[i] = FALSE;
             Pred[i] = i;
             }

    while (Iteration++ < cConnectCurr) {
        /* update the Label array for each vertex */
        /* which is not yet labeled permanently   */

        for (i = 0; i < cConnectCurr; i++) 
            if (Perm[i] == FALSE) {
                MinCost = Min(Label[i], Label[LastLabeled] + 
                          LineWeight(LastLabeled, i));

                if (MinCost < Label[i]) {
                    Label[i] = MinCost;
                    Pred[i] = LastLabeled;
                    }
            }

        /* find the vertex with smallest label among those */
        /* not permanently labeled                         */
        NewMin = FindMin(Label, Perm);
        Perm[NewMin] = TRUE;
        LastLabeled = NewMin;
        }
}


/*      
 * FindMin - invoked to find the vertex with minimum label among all of
 *           the vertices that have not been permanently marked
 *
 */

FindMin(Label, Perm)

int Label[];
int Perm[];

{
    extern int cConnectCurr;
    int i;
    int Minimum = INFINITY + 1;
    int MinIndex;           /* saves the index of the minimum label */

    for (i = 0; i < cConnectCurr; i++)
        if (Perm[i] == FALSE && Label[i] < Minimum) {
            Minimum = Label[i];
            MinIndex = i;
            }

    return(MinIndex);
}

/*
 * LineWeight - returns the weight associated with the line between the
 *              source and destination vertices or infinity if there is
 *              no connection
 *
 */

LineWeight(SrcVtx, DestVtx)

int SrcVtx;
int DestVtx;

{
    char *sbDestName;                    /* name of destination vertex */
    struct AdjList *pAdjNext;  
    extern struct AdjList AdjListTable[]; 

    /* get the name of the destination node */
    sbDestName = AdjListTable[DestVtx].sbNodeName;

    /* get a pointer to the nodes adjacent to */
    /* the source node                        */
    pAdjNext = AdjListTable[SrcVtx].pAdjNext;

    /* traverse through the adjacency list until you   */
    /* either find the node or until you reach the end */
    while (pAdjNext != NULL && pAdjNext->sbNodeName != sbDestName)
        pAdjNext = pAdjNext->pAdjNext;

    return(pAdjNext == NULL ? INFINITY : pAdjNext->wt);
}        

/* 
 * OutputCypressTable - invoked to output the routing tables.  The Pred
 *     array is set up so that Pred[i] is that node's predecessor in the
 *     shortest path from the source.  The source node's predecessor is
 *     itself.
 *
 *     The c-number - interface pairs for the various routes are obtained
 *     by invoking GetInterface and are then sorted by c-number and output.
 *     A negative interface number is entered in the table for the source
 *     node since it is an error for the source node to be requesting info
 *     on how to route to itself.
 */

OutputCypressTable(AdjSrcInd, ConnectSrcInd, sbSourceName, Verbose)

int AdjSrcInd;                   /* index of source node in AdjListTable  */
int ConnectSrcInd;               /* index of source node in ConnectTable  */
char *sbSourceName;              /*       in the form name-cypress        */
int Verbose;                     /* flags whether verbose routing info is */
                                 /* to be generated, default is no        */

{
    int i;
    int Interface[MAXENTRIES];             /*  array of interface numbers  */
    extern int cAdjListCurr;               /* # of entries in AdjListTable */
    extern struct AdjList AdjListTable[];  /*    network adjacency graph   */
    extern int Pred[];			   /*    array of predecessors     */
    extern int GetInterface();             /*   returns interface number   */
    extern int Cmp_C_Number();		   /* comparison routine for sort  */
    struct CypRoute Routes[MAXENTRIES];    /* array of network number -    */
					   /* interface pairs              */
    
    for (i = 0; i < cAdjListCurr; i++)
         Interface[i] = 0;

    for (i = 0; i < cAdjListCurr; i++)
        if (i != AdjSrcInd) {
            Routes[i].c_number = AdjListTable[i].sbNetNumber;
	    Routes[i].Interface = GetInterface(i, Interface, ConnectSrcInd,
			   		       sbSourceName);
 	    }
        else {
	    Routes[i].c_number = AdjListTable[i].sbNetNumber;
	    Routes[i].Interface = ERROR;
  	    }

    /* sort the entries by c-number */
    qsort(Routes, cAdjListCurr, sizeof(struct CypRoute), Cmp_C_Number);
    if (Verbose) {
        printf("#\n# Cypress Routing Table\n");
        printf("#\n# Source site is %s\n#\n", sbSourceName);

        /* output the Cypress routing information */
        for (i=0; i < cAdjListCurr; i++) {
             printf("%s %d\t\t# ", Routes[i].c_number, Routes[i].Interface);
             printf("dest: %s -> ", AdjListTable[i].sbNodeName);
             if (i == AdjSrcInd)
                printf("ERROR\n");
             else {
	         /* search through the connection table for this */
	         /* interface number and print the corresponding */
                 /* line name				         */
                 pLinks = ConnectTable[ConnectSrcInd].pConnections;

                 while (pLinks->w != Routes[i].Interface)
                    pLinks = pLinks->pConnections;

                 printf("line: %s\n", pLinks->sb);
	     }
	}
    }
    else
        /* output the Cypress routing information */
        for (i=0; i < cAdjListCurr; i++)
          if (i != AdjSrcInd)
             printf("%s %d\n", Routes[i].c_number, Routes[i].Interface);
}


/*
 * GetInterface.c - finds the interface number that is used by the source
 *     site to get to the site passed as a parameter.  The routine starts
 *     at the end of the path and recursively calls itself until it reaches
 *     the start.  Each return updates the interface number of all 
 *     intermediate hops so subsequent searches will stop as soon as they 
 *     reach a site for which the interface is known.
 *
 */

int GetInterface(Start, Interface, SourceIndex, sbSourceName)

int Start;            /* vertex for which interface number is sought */
int Interface[];      /*        array of interface numbers           */
int SourceIndex;      /*    index of source site in ConnectTable     */
char *sbSourceName;   /*  name of source site in form cypress-site   */

{
    extern struct AdjList AdjListTable[];
    extern int SearchConnectTable();  /* index of node in ConnectTable */
    extern int SearchConnections();   /* searches a ConnectTable entry */
                                      /* for a cypress leased line     */
    extern int MakeLineName();        /* creates cypress leased line   */
				      /* from a source and destination */
                                      /* node name		       */
    extern int Pred[];		      /*    array of shortest paths    */
    int InterfaceNum;                 /*    logical interface number   */
    char *Cypress[10];                /* holds cypress part of node    */
				      /* name that is returned by      */
				      /* MakeLineName                  */
    char *sbFirstHop[50];             /* name of first hop in path     */
    char *sbSourceSite[50];           /* source site without "cypress-"*/
				      /* prefix			       */
    char *sbLine1[50];                /* cypress line name in form     */
				      /* site1-site2		       */
    char *sbLine2[50];                /* cypress line name in form     */
				      /* site2-site1		       */

    /* check and see if we've already found */
    /* the interface for this destination   */
    if (Interface[Start] != 0)
        return(Interface[Start]);
    else /* check and see if this vertices predecessor is */
         /* the source in the path                        */

         if (Pred[Start] == Pred[Pred[Start]]) {

             /* we have the starting vertices name in the form */
             /* "name-cypress" but leased line names are in the*/
             /* form "site1-cypress/site2-cypress".  We need   */
	     /* the leased line name so we can search the      */
	     /* connect table for the interface number for that*/
	     /* line					       */

             /* we don't know if this site is the first or second part */
             /* of the line name so we must build both                 */
	     MakeLineName(AdjListTable[Start].sbNodeName, sbSourceName, sbLine1);
	     MakeLineName(sbSourceName, AdjListTable[Start].sbNodeName, sbLine2);

             /* get the interface number */
             InterfaceNum = SearchConnections(SourceIndex, sbLine1, sbLine2);

             if (InterfaceNum < 0)
                 FatalError("cypress tables incorrect in GetInterface");
             else {

                 /* update the interface number for this site */
                 /* so we won't have to go through this again */
                 Interface[Start] = InterfaceNum;
                 return(InterfaceNum);
                 }
             }
        else { /* this vertices interface number is zero and it is */
	       /* not the first hop in the path, continue looking  */
	       /* until we either find a vertex that precedes this */
	       /* in the path whose interface number has already   */
	       /* been found or until we find the start of the path*/
	       Interface[Start] = GetInterface(Pred[Start], Interface,
				               SourceIndex, sbSourceName);
               return(Interface[Start]);
	       }
}

/*
 * SearchConnections - traverses the entry in the ConnectTable whose index
 *    is passed as a parameter and returns the interface number corresponding
 *    to either the second or third argument or -1 if it is not found.
 *
 */

int SearchConnections(Index, sbLine1, sbLine2)

int Index;         /* position in ConnectTable     */
char *sbLine1;     /* name in the form site1-site2 */
char *sbLine2;     /* name in the form site2-site1 */

{
    extern struct Connect ConnectTable[];
    struct Links *pConnections; 

    /* intialize the pointer to the beginning of the */
    /* list of connections                           */
    pConnections = ConnectTable[Index].pConnections;

    while (pConnections != NULL && 
           strcmp(sbLine1, pConnections->sb) != 0 &&
           strcmp(sbLine2, pConnections->sb) != 0)
        pConnections = pConnections->pConnections;

    return(pConnections == NULL ? -1 : pConnections->w);
}
    

/*
 * WeightLines - invoked to add a weight to the line connecting two
 * cypress sites.
 * 
 */

WeightLines()

{
    extern int CmpLineName();              /* sort comparison routine */
    extern struct AdjList AdjListTable[];  /* network adjacency list  */
    extern struct Line LineTable[];	   /* network interconnections*/
    struct AdjList *pAdjList;              /* used to traverse list of*/
					   /* adjacent nodes          */
    extern int cLineCurr;		   /* number of connections   */
    extern int SearchLineTable();	   /* searches the line table */
					   /* for a specific line,    */
					   /* returns its position in */
					   /* the line table          */
    extern int MakeLineName();		   /* creates cypress leased  */
					   /* line name from a source */
					   /* and destination node    */
					   /* name		      */
    char Cypress[10];                      /* holds cypress part of   */
					   /* node name returned by   */
					   /* MakeLineName            */
    char sbLine1[50];			   /* cypress line name in the*/
					   /* form site1-site2        */
    char sbLine2[50];			   /* cypress line name in the*/
					   /* form site2-site1        */
    char sbSite1[50];			   /* site portion of cypress */
					   /* leased line name        */
    char sbSite2[50];                      /* site portion of cypress */
					   /* leased line name        */
    char sbMsg[90];			   /* used to construct error */
					   /* messages		      */
    int LineIndex;			   /* index of entry in line  */
					   /* table                   */
    int i;

    /* sort the phone line table according to line name */
    qsort(LineTable, cLineCurr, sizeof(struct Line), CmpLineName);

    /* step through the adjacency list calculating line */
    /* weights for all of the connections		*/
    for (i = 0; i < cAdjListCurr; i++) {
         pAdjList = AdjListTable[i].pAdjNext;

         while (pAdjList != NULL) {
             /* we now have the source and destination sites and can */
	     /* construct the line name.  We don't know if the line  */
	     /* name is in the form site1/site2 or site2/site1 so we */
	     /* must construct both				     */
	     MakeLineName(AdjListTable[i].sbNodeName, pAdjList->sbNodeName, sbLine1);
	     MakeLineName(pAdjList->sbNodeName, AdjListTable[i].sbNodeName, sbLine2);
	     /* we've got the line name, now search for it in the */
	     /* line table					  */
	     LineIndex = SearchLineTable(sbLine1);
             if (LineIndex < 0)
	         LineIndex = SearchLineTable(sbLine2);

	     /* if we didn't find it serious problems */
	     if (LineIndex < 0) {
	         sprintf(sbMsg, "Unable to find %s or %s in line table",
		         sbLine1, sbLine2);
		 FatalError(sbMsg);
 	         }
	     else {
	         pAdjList->wt = ComputeLineWt(LineTable[LineIndex].speed);
		 pAdjList = pAdjList->pAdjNext;
	         }
             }
	 }
}

/*
 * ComputeLineWt.c -- routines to compute a line's weight based on the 
 *     line's speed in relation to a base speed. The base speed is the
 *     defined value BASESPEED set to 9600 ( because this is the average
 *     expected line speed at the time the routine was written ). A line
 *     of this speed has the base weight of 100.
 * 
 * date -- 7/9/85
 *
 * modification history
 *
 */

ComputeLineWt(speed)
int speed;
{
        
    return( Round( (float)BASEWT * ((float)BASESPEED / (float)speed) ) );

}

Round(fl)
float fl;
{

    return( (int)(fl + .5) );

}
