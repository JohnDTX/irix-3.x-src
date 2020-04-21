/*
 * AdjList.c -- routines to build the adjcency list.
 *     BuildAdjList()
 *     CreateAdjNode(iCT, iAL)
 *     MapAdjList()
 *
 * author -- Cathy Privette
 *
 * date -- 6/22/85
 *
 * modification history
 *
 * 9-11-85 -- modified to conform with new standard for line names
 *	      site1-cypress/site2-cypress
 *
 */

#include <stdio.h>
#include <strings.h>
#include "defs.h"

extern int ComparAdjListNetNo();
extern struct Connect ConnectTable[];
extern struct AdjList AdjListTable[];
extern int cConnectCurr;
extern int cAdjListCurr;
extern int cError;

/*
 * BuildAdjList() -- Builds the adjacency list from the ConnectTable.
 *     Build an auxiliery adj. list to be used in finding cycles, 
 *     connected components, and biconnected components.
 * 
 */

BuildAdjList()
{
    char sbNdName1[NAMESIZE], sbNdName2[NAMESIZE];
    char sbLkName1[NAMESIZE], sbLkName2[NAMESIZE];
    char *slash = "/";
    int iAL, iCT;
    struct Links *pLink;

    /* for each node in the ConnectTable, build an adjacency list. */

    for (iAL = 0; iAL < cConnectCurr; iAL++) {
        AdjListTable[iAL].sbNodeName = ConnectTable[iAL].sbNodeName;
        AdjListTable[iAL].sbNetNumber = ConnectTable[iAL].sbNetNumber;
        AdjListTable[iAL].wt = 1;
        AdjListTable[iAL].pAdjNext = NULL;

        /* for each line connected to this node in the ConnectTable,
           add an adjacent node to the adjacency list for this node. */

        for (pLink = ConnectTable[iAL].pConnections; pLink != NULL; 
             pLink = pLink->pConnections) {

	    /* separate the line name at the slash */
            SeparateName(pLink->sb, sbLkName1, sbLkName2); 

            /* find out which end of the line name is not the current node,
               the node represented by the other end of the line name will 
               be adjacent to the current node. */

            if ( strcmp(AdjListTable[iAL].sbNodeName, sbLkName1) == 0) {
                if ((iCT = SearchConnectTable(sbLkName2)) != -1)
                    CreateAdjNode(iCT, iAL);
                else
                    FatalError("Error in BuildAdjList -- Adjacent node not in Connect Table");
                }
            else if ( strcmp(AdjListTable[iAL].sbNodeName, sbLkName2) == 0) {
                if ((iCT = SearchConnectTable(sbLkName1)) != -1)
                    CreateAdjNode(iCT, iAL);
                else
                    FatalError("Error in BuildAdjList -- Adjacent node not in Connect Table");
                }
            else
                FatalError("Error in BuildAdjList -- Incorrect line name");

            }
        }   

    /* Sort the AdjListTable by net numbers. */

    cAdjListCurr = cConnectCurr; /* count of entries in AdjListCurr */
    qsort(AdjListTable, cAdjListCurr, sizeof(struct AdjList), ComparAdjListNetNo);

    MapAdjToAux();  /* build auxiliary adj. list */

    return;

}

/*
 * CreateAdjNode(iCT, iAL) -- create a node in the adjacency list and connect
 *     it into the list. 
 *
 */

CreateAdjNode(iCT, iAL)
int iCT, iAL;
{
    struct AdjList *pAdjNode;

    pAdjNode = New(struct AdjList);
    if (pAdjNode == NULL) 
        FatalError("CreateAdjNode");
    pAdjNode->sbNodeName = ConnectTable[iCT].sbNodeName;
    pAdjNode->sbNetNumber = ConnectTable[iCT].sbNetNumber;
    pAdjNode->wt = 1;
    pAdjNode->pAdjNext = AdjListTable[iAL].pAdjNext;
    AdjListTable[iAL].pAdjNext = pAdjNode;

    return;
}

/*
 * MapAdjList() -- Walks through the adjacency list and prints each node
 *     and what's adjacent to it.
 *
 */

MapAdjList()
{
    int iAdjList;
    struct AdjList *pAdjNext;

    /* for each node in the AdjListTable, print the node. */

    printf("\nAdjacency List\n\n");

    for (iAdjList = 0; iAdjList < cAdjListCurr; iAdjList++ ) {
        printf("\nNode Name = %s\tC-Number = %s\n",
                AdjListTable[iAdjList].sbNodeName, 
                AdjListTable[iAdjList].sbNetNumber);
        printf("\n\tAdjacent nodes:\n");

        /* print each node adjacent to it */

        for (pAdjNext = AdjListTable[iAdjList].pAdjNext;
             pAdjNext != NULL; pAdjNext = pAdjNext->pAdjNext )
            printf("\tName = %s, C-Number = %s, Cost = %d\n",
                     pAdjNext->sbNodeName, pAdjNext->sbNetNumber,
			           pAdjNext->wt);
        }
    return;
}
