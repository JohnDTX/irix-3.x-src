/*
 * paths.c -- routines to compute and print the path and the cost of the
 *     path to be taken between two specified sites :
 *     GetSrcAndDest(sbSrcName, sbDestName, piALSrc, piALDest)
 *     PrintPath()
 *     ComputeCostSrcToDest()
 *
 * author -- Cathy Privette
 *
 * date -- 7/21/85
 *
 * modification history
 *
 * 9/11/85 - modified to conform to new convention for line names.
 * 9/19/85 - modified to fix a bug, wouldn't build line names correctly
 *	     when it needed to follow a trail of lines to compute cost.
 */

#include <stdio.h>
#include <strings.h>
#include "defs.h"

extern int Pred[];    /* array containing the min. cost tree */
extern struct Connect ConnectTable[];
extern struct AdjList AdjListTable[];
extern struct Line LineTable[];

/*
 * GetSrcAndDest(sbSrcName, sbDestName, piALSrc, piALDest) -- reads both the
 *     source and destination from stdin. Assumes input will be the node name 
 *     as it appears in /etc/hosts without "-cypress" on the end. Returns the
 *     source and destination names and indices in AdjListTable.
 */

GetSrcAndDest(sbSrcName, sbDestName, piALSrc, piALDest)
char *sbSrcName, *sbDestName;
int *piALSrc, *piALDest;
{
    int iCT;

    printf("\nSource site name (e.g. purdue-cypress) : ");
    scanf("%s", sbSrcName);
    printf("\nDestination site name (e.g. decwrl-cypress) : ");
    scanf("%s", sbDestName);
    printf("\n");

    /* find the indices of source and destination in AdjListTable */

    if ((iCT = SearchConnectTable(sbSrcName)) == -1)
        FatalError("Error in GetSrcAndDest -- source not in ConnectTable");
    else if ((*piALSrc = SearchAdjListTable(ConnectTable[iCT].sbNetNumber))
              == -1)
        FatalError("Error in GetSrcAndDest -- source net no. not in AdjListTable");

    if ((iCT = SearchConnectTable(sbDestName)) == -1)
        FatalError("Error in GetSrcAndDest -- destination not in ConnectTable");
    else if ((*piALDest = SearchAdjListTable(ConnectTable[iCT].sbNetNumber))
              == -1)
        FatalError("Error in GetSrcAndDest -- destination net no. not in AdjListTable");

    return;
}
  
/*  
 * PrintPath() -- prints the path to take to reach a specified destination
 *     from a specified source.
 */

PrintPath()
{
    int iPred;
    char sbSrcName[NAMESIZE], sbDestName[NAMESIZE];
    int iALSrc, iALDest;  /* indices into AdjListTable of the */
                          /* source and destination           */

    /* get the source and destination names and indices */

    GetSrcAndDest(sbSrcName, sbDestName, &iALSrc, &iALDest);

    /* build the SPF tree with the dest. as the root; builds the Pred array */

    if (CreateCypressTable(iALDest, sbDestName) == -1)
        FatalError("Error in PrintPath -- unable to construct SPF tree");

    /* traverse Pred array printing the path to take */
    /* to reach the destination from the source.     */

    printf("\nSource = %s\n", AdjListTable[iALSrc].sbNodeName);
    for (iPred = Pred[iALSrc]; iPred != Pred[iPred]; iPred = Pred[iPred]) {
        printf("\t-> %s\n", AdjListTable[iPred].sbNodeName);
        }
    printf("\t-> %s = Destination\n", AdjListTable[iPred].sbNodeName);

    return;
}

/*
 * ComputeCostSrcToDest() -- computes the cost of sending a specified size
 *     packet, from a specified source, to a specified destination.
 */

ComputeCostSrcToDest()
{
    int iPred, iLT;
    char sbSrcName[NAMESIZE], sbDestName[NAMESIZE];
    char sbNdName[NAMESIZE];
    char sbLineName[NAMESIZE];
    int iALSrc, iALDest;      /* indices into AdjListTable of */
                              /* the source and destination   */
    int totaltime, time;      /* total time to send a packet  */  
                              /* and time for 1 line          */
    int nbytes, nbits;        /* packet size in bytes and bits */

    /* get source and destination names and indices in AdjListTable, */
    /* and get the packet size to be sent in bytes.                  */

    GetSrcAndDest(sbSrcName, sbDestName, &iALSrc, &iALDest); 
    printf("Number of bytes in packet : ");
    scanf("%d", &nbytes);
    printf("\n");
    nbits = nbytes * 8;

    /* build the SPF tree with destination as the root; builds Pred array */

    if (CreateCypressTable(iALDest, sbDestName) == -1)
        FatalError("Error in ComputeCostSrcToDest -- unable to construct SPF tree");

    printf("\nThe line-by-line cost of sending a %d byte packet \n", nbytes);
    printf("from %s to %s is:\n\n", sbSrcName, sbDestName);

    /* traverse Pred array computing the cost of the path from */
    /* source to destination as we go.                         */

    totaltime = 0;
    strcpy(sbNdName, AdjListTable[iALSrc].sbNodeName);

    for (iPred = Pred[iALSrc]; iPred != Pred[iPred]; iPred = Pred[iPred]) {
        MakeLineName(sbNdName, AdjListTable[iPred].sbNodeName, sbLineName);

        if ((iLT = SearchLineTable(sbLineName)) == -1) {
            MakeLineName(AdjListTable[iPred].sbNodeName, sbNdName, sbLineName);

            if ((iLT = SearchLineTable(sbLineName)) == -1)
                FatalError("Error in ComputeCostSrcToDest -- line not found");
            }
        time = Round((float)nbits / ((float)LineTable[iLT].speed / 1000.));
        printf("Line name = %s, Line speed = %d bps, Time = %d ms\n",
                sbLineName, LineTable[iLT].speed, time);
        totaltime += time;

	/* since we found a line site1/site2, copy site2's name */
	/* into sbNdName so it will be site1 for the next pass  */
        strcpy(sbNdName, AdjListTable[iPred].sbNodeName);
        }

    MakeLineName(sbNdName, AdjListTable[iPred].sbNodeName, sbLineName);
    if ((iLT = SearchLineTable(sbLineName)) == -1) {
        MakeLineName(AdjListTable[iPred].sbNodeName, sbNdName, sbLineName);

        if ((iLT = SearchLineTable(sbLineName)) == -1)
            FatalError("Error in ComputeCostSrcToDest -- line not found");
        }
    time = Round((float)nbits / ((float)LineTable[iLT].speed / 1000.));
    printf("Line name = %s, Line speed = %d bps, Time = %d ms\n",
            sbLineName, LineTable[iLT].speed, time);
    totaltime += time;

    printf("\nTotal cost is %d ms\n", totaltime);

    return;
}
