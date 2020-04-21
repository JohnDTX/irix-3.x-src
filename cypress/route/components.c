/*
 * components.c -- routines for finding the connected components of the
 *     network and for determining if there are any cycles. 
 *     Routines include :
 *          CompAndCycles()
 *          MapAdjToAux()
 *          CnComponents()
 *          DepthFirstSearch(iCurrNode, cCycles)
 *          MarkEdge(iNode, iTop)
 *
 * author -- Cathy Privette
 * 
 * date -- 6/30/85
 *
 * modification history
 *
 */

#include <stdio.h>
#include <strings.h>
#include "defs.h"

#define EMPTY -1

extern int cError;
extern struct AdjList AdjListTable[];
extern int cConnectCurr;
extern int cAdjListCurr;    /* count of entries in AdjListTable */


int cCnComponents; /* count of the number of connected components */
int Mark[MAXENTRIES]; /* marks the node as to which component it belongs to */
struct AuxAdjNode *AuxAdjTable[MAXENTRIES]; /* auxiliery adjacency table */
struct AuxAdjNode *Ptr[MAXENTRIES]; /* Ptr[n] pts to the next node adj to node n */

/*
 * CompAndCycles() -- The aux. adj. list is used to determine
 *     the number of connected components in the network and if there are any
 *     cycles in the network. 
 */

CompAndCycles()
{

    CnComponents();
    
    return;
}

/*
 * MapAdjToAux() -- maps AdjListTable to an auxiliery adjacency list,
 *     AuxAdjTable.
 */

MapAdjToAux()
{
    int iCurrNode;
    struct AdjList *pAdjNext;
    struct AuxAdjNode *pAuxNext;

    /* initialize AuxAdjTable */

    for (iCurrNode = 0; iCurrNode < cAdjListCurr; iCurrNode++ )
        AuxAdjTable[iCurrNode] = NULL;

    /* for each node in AdjListTable */

    for (iCurrNode = 0; iCurrNode < cAdjListCurr; iCurrNode++ ) {
        
#ifdef DEBUG
    printf("\nCurrent Node No. %d, Net No. %s\n", iCurrNode,              
            AdjListTable[iCurrNode].sbNetNumber);
#endif

        /* for each node adjacent to the current node */

        for ( pAdjNext = AdjListTable[iCurrNode].pAdjNext; 
              pAdjNext != NULL; pAdjNext = pAdjNext->pAdjNext) {
            
             pAuxNext = New(struct AuxAdjNode);
             if (pAuxNext == NULL)
                 FatalError("MapAdjToAux");
             pAuxNext->iAdjNode = SearchAdjListTable(pAdjNext->sbNetNumber);
             if (pAuxNext->iAdjNode == -1)
                 FatalError("Error in MapAdjToAux -- adjacent node not found\n");
             else {
                 pAuxNext->fEdge = FALSE;
                 pAuxNext->pAuxNext = AuxAdjTable[iCurrNode];
                 AuxAdjTable[iCurrNode] = pAuxNext;

#ifdef DEBUG
    printf("    Adj. node no. %d, Net no. %s\n", pAuxNext->iAdjNode,
            AdjListTable[pAuxNext->iAdjNode].sbNetNumber);
#endif
                 }
             }
         }   
     return;
}

/*
 * CnComponents() -- uses a depth first search to determine the number of
 *     connected components in the network and the number of cycles.
 */

CnComponents()
{
    int iCurrNode;          /* index of the current node */
    int cCycles = 0;        /* count of the number of cycles */

    /* initialize Mark and Ptr arrays and the no. of connected components */

    cCnComponents = 0;
    for (iCurrNode = 0; iCurrNode < cAdjListCurr; iCurrNode++ ) {
        Mark[iCurrNode] = 0;
        Ptr[iCurrNode] = AuxAdjTable[iCurrNode];
        }

    /* for each node in the network */
 
    for (iCurrNode = 0; iCurrNode < cAdjListCurr; iCurrNode++ ) {
        if (Mark[iCurrNode] == 0) {
            cCnComponents++;
            cCycles = DepthFirstSearch(iCurrNode, cCycles);
            }
        }

    /* How many connected components are there ? */

    if (cCnComponents == 1)
        printf("\nThe network is fully connected. \n");
    else
        printf("\nThere are %d connected components in the network\n",
                cCnComponents);

    /* Are there any cycles in the network ? */

    if (cCycles < 1) 
        printf("\nThe network contains no cycles.\n");
    else
        printf("\nThe network contains cycles. \n");

    return;
}

/*
 * DepthFirstSearch(iCurrNode, cCycles) -- performs
 *     a depth first search from iCurrNode, numbers each node encountered in
 *     the search with the component number it belongs to, cCnComponents, and
 *     increments the count of cycles if a cycle is encountered in the search.
 */

DepthFirstSearch(iCurrNode, cCycles)
int iCurrNode, cCycles;
{
    int iNode;      /* index of a node encountered on the dfs initiated */
                    /* from iCurrNode.                                  */
    int sp = EMPTY; /* stack pointer, pts. to the last full element     */
    int iTop;       /* index into AuxAdjTable of the node on top of the stack */
    int Stack[MAXENTRIES]; /* holds the indices into AuxAdjTable of visited */
                             /* vertices with possible edges yet to be traversed */

    Mark[iCurrNode] = cCnComponents;
    Stack[++sp] = iCurrNode;  /* stack the current beginning of the dfs */

    /* continue the search until no more vertices can be reached */

    while (sp > EMPTY) {
        
        /* go to the next node adjacent to the node on top of the stack */

        iTop = Stack[sp];
        while (Ptr[iTop] != NULL) {
            iNode = Ptr[iTop]->iAdjNode;

#ifdef DEBUG
     printf("\nTop of stack %d, Adj. node %d\n", iTop, iNode);
#endif
 
            if (Mark[iNode] == 0) {      /* Node not visited yet */
                Ptr[iTop]->fEdge = TRUE; /* Mark edge traversed */
                MarkEdge(iNode, iTop);   /* Mark it in the opposite direction */
                Mark[iNode] = cCnComponents; /* Mark the node visited with the */
                                             /* component number it belongs to */
                Stack[++sp] = iNode;      /* Next node to branch out from */
                }
            else if (!Ptr[iTop]->fEdge) { /* node visited but edge not traversed */
                cCycles++;               /* there exists a cycle */
                Ptr[iTop]->fEdge = TRUE; /* Mark the edge traversed */
                MarkEdge(iNode, iTop);   /* Mark it in the opposite direction */
                }
            Ptr[iTop] = Ptr[iTop]->pAuxNext; /* update adj. list ptr. */
            iTop = Stack[sp];    
            }
        sp--;  /* Pop from top of the stack */
        }
    return(cCycles);
}

/*
 * MarkEdge(iNode, iTop) -- Find the edge representing the edge (iNode, iTop)
 *     and mark it as being traversed.
 */

MarkEdge(iNode, iTop)
int iNode, iTop;
{
    struct AuxAdjNode *pAuxNext;

    /* search iNode's adjacency list until the node representing */ 
    /* iTop is found.                                            */ 

    for (pAuxNext = AuxAdjTable[iNode];
         pAuxNext != NULL && pAuxNext->iAdjNode != iTop;
         pAuxNext = pAuxNext->pAuxNext)
        /* null */;
    if (pAuxNext->iAdjNode == iTop) {
        pAuxNext->fEdge = TRUE; /* Mark the edge traversed */

#ifdef DEBUG
    printf("Edge %d, %d found and marked\n", iNode, pAuxNext->iAdjNode);
#endif
        }
    else
        FatalError("Error in MarkEdge -- edge not found in AuxAdjTable");

    return;
}
