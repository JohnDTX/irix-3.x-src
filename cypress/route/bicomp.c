/*
 * bicomponents.c -- routines for finding the biconnected components 
 *     in the network:
 *         BiComponents()
 *         PrintIsolNode(iCurrNode)
 *         BiDFS(iCurrNode, iParent)
 *         Out(iCurrNode, iAdjNode, iParent, RootBlock)
 *         AddBlock(iCurrNode)
 *         PushEdgeStack(iCurrNode, iAdjNode)
 *
 * author -- Cathy Privette
 *
 * date -- 7/5/85
 *
 * modification history
 *
 */

#include <stdio.h>
#include "defs.h"

#define ROOTPARENT -1

extern int cAdjListCurr;
extern struct AdjList AdjListTable[];
extern struct AuxAdjNode *AuxAdjTable[];
extern int cCnComponents;               /* no. of connected components */
extern int Mark[];                      /* marks a node as to which */
                                        /* component it belongs to  */
extern struct AuxAdjNode *Ptr[];        /* Ptr[n] pts to the next adj */
                                        /* node on node n's adj. list */

int cNodes = 0;                         /* count of nodes visited */
int cBlockNum = 0;                      /* count of blocks (biconnected */
                                        /* components) found            */
int DFNumber[MAXENTRIES];               /* DFS number of each node visited  */
int Low[MAXENTRIES];                    /* Low[n] is the lowest numbered node */
                                        /* we can get back to from node n     */
struct BlkXRef *ArtPt[MAXENTRIES];      /* articulation pt list - ArtPt[j]    */
                                        /* pts to a linked list of containing */
                                        /* block numbers if j is an art. pt.  */
struct StackEntry *pTopStack;           /* ptr to the top of the edge stack   */

/* 
 * BiComponents() -- uses a depth first search to determine the biconnected 
 *     components and the articulation points of the network. The algorithm
 *     is by Tarjan. In the following a block is a biconnected component.
 *     This routine must be called from Components, which sets up the   
 *     AuxAdjList.
 */

BiComponents()
{
    int iCurrNode;                /* index of the current node */
    int cArtPt = 0;               /* count of the no. of articulation points */
    struct BlkXRef *pBlock;       /* ptr to a block cross reference node */

    pTopStack = NULL;
    cCnComponents = 0;

    /* initialize Mark, Ptr, DFNumber, Low, ArtPt arrays */

    for (iCurrNode = 0; iCurrNode < cAdjListCurr; iCurrNode++) {
        Mark[iCurrNode] = 0;
        Ptr[iCurrNode] = AuxAdjTable[iCurrNode];
        DFNumber[iCurrNode] = 0;
        Low[iCurrNode] = 0;
        ArtPt[iCurrNode] = NULL;
        }

    /* for each node in the network */

    for (iCurrNode = 0; iCurrNode < cAdjListCurr; iCurrNode++) {
        if (Mark[iCurrNode] == 0) {
            cCnComponents++;
            printf("\n\nConnected Component %d consists of Biconnected Components :\n", cCnComponents);
            
            /* Is it an isolated node ? */

            if ( Ptr[iCurrNode] != NULL )
                BiDFS(iCurrNode, ROOTPARENT);
            else
                PrintIsolNode(iCurrNode);
            }
        }

    for (iCurrNode = 0; iCurrNode < cAdjListCurr; iCurrNode++) {
        if (ArtPt[iCurrNode] != NULL)
            cArtPt++;
        }
    if (cArtPt > 0)
        printf("\n\nArticulation Points and Containing Biconnected Components\n");

    /* for each node, if it is an articulation point */
    /* print its containing bicomponents (blocks).   */

    for (iCurrNode = 0; iCurrNode < cAdjListCurr; iCurrNode++) {
        if (ArtPt[iCurrNode] != NULL) {
            printf("\nArticulation Point %s \n",
                   AdjListTable[iCurrNode].sbNodeName);
            printf("\tContaining Biconnected Components :  ");
            for (pBlock = ArtPt[iCurrNode]; pBlock != NULL;          
                 pBlock = pBlock->pNextBlock) 
                printf("%d  ", pBlock->BlockNum);
            printf("\n");
            }
        }
    return;
}   

/*
 * PrintIsolNode(iCurrNode) -- prints the block number of an isolated node.
 */

PrintIsolNode(iCurrNode)
int iCurrNode;
{
    Mark[iCurrNode] = cCnComponents;
    DFNumber[iCurrNode] = ++cNodes;
    Low[iCurrNode] = cNodes;
    cBlockNum++;
    printf("\nBiconnected component number %d", cBlockNum);
    printf(" consists of the isolated node %s\n", 
             AdjListTable[iCurrNode].sbNodeName);
    return;
}

/*
 * BiDFS(iCurrNode, iParent) -- Recursive routine which performs a 
 *     depth first search beginning at iCurrNode. Edges encountered are
 *     stacked until an articulation point is found. Then the edges in
 *     the biconnected component are printed and the articulation point
 *     is cross referenced by placing the containing block number in a
 *     linked list for that point. The method of finding the articulation
 *     point is due to an algorithm by Tarjan.
 */

BiDFS(iCurrNode, iParent)
int iCurrNode, iParent;
{
    int iAdjNode;
    int RootBlock = 0;       /* block number of the root of the DFS tree */
    struct BlkXRef *pBlock;

    Mark[iCurrNode] = cCnComponents;
    DFNumber[iCurrNode] = ++cNodes;
    Low[iCurrNode] = cNodes;

    /* for each node adjacent to iCurrNode */

    while ( Ptr[iCurrNode] != NULL) {
        iAdjNode = Ptr[iCurrNode]->iAdjNode;
        if (Mark[iAdjNode] == 0) {   /* node not visited yet */
            if (RootBlock != 0) {     /* root is an art. pt., put its  */
                                      /* containing block on its list  */
                                      /* for cross referencing.        */
                pBlock = New(struct BlkXRef);
                if (pBlock == NULL) 
                    FatalError("BiDFS");
                pBlock->BlockNum = RootBlock;
                pBlock->pNextBlock = ArtPt[iCurrNode];
                ArtPt[iCurrNode] = pBlock;
                RootBlock = 0;
                }
            PushEdgeStack(iCurrNode, iAdjNode);
            BiDFS(iAdjNode, iCurrNode);
            Low[iCurrNode] = Min(Low[iCurrNode], Low[iAdjNode]);
            if ( Low[iAdjNode] >= DFNumber[iCurrNode])
                
                /* a biconnected component has been found */
                
                RootBlock = Out(iCurrNode, iAdjNode, iParent, RootBlock);
            }
        else if ((DFNumber[iAdjNode] < DFNumber[iCurrNode]) &&
                 (iAdjNode != iParent)) {  /* node has been visited, is this */
                                           /* a new edge ?                   */
            PushEdgeStack(iCurrNode, iAdjNode);
            Low[iCurrNode] = Min(Low[iCurrNode], DFNumber[iAdjNode]);
            }
        Ptr[iCurrNode] = Ptr[iCurrNode]->pAuxNext;
        }
    return;
}

/*
 * Out(iCurrNode, iAdjNode, iParent, RootBlock) -- This routine is called 
 *     an articulation point has been found. It will print the biconnected
 *     component number and the edges in the biconnected component, which
 *     are on the top of the edge stack.
 */
 
Out(iCurrNode, iAdjNode, iParent, RootBlock)
int iCurrNode, iAdjNode, iParent, RootBlock;
{
    char sbNdName1[NAMESIZE], sbNdName2[NAMESIZE], sbNdName3[NAMESIZE],
         sbLineName[NAMESIZE];

    cBlockNum++;      /* increment block number */
    if (iParent == ROOTPARENT && ArtPt[iCurrNode] == NULL)

        /* Root may be an art. pt., update the block number */
        /* containing the root.                             */

        RootBlock = cBlockNum;

    if (iParent != ROOTPARENT || ArtPt[iCurrNode] != NULL)

       /* Add the current block no. to the cross reference  */
       /* for this articulation point.                      */
 
       AddBlock(iCurrNode);

    /* print the biconnected component */

    printf("\nBiconnected Component Number %d consists of edges :\n",
             cBlockNum);
    while (pTopStack->Node1 != iCurrNode || pTopStack->Node2 != iAdjNode) {

        /* print the edges in this block and if the root is not an     */
        /* art. pt. find the art. pt. in the block containing the root */

        MakeLineName(AdjListTable[pTopStack->Node1].sbNodeName,
		     AdjListTable[pTopStack->Node2].sbNodeName,
		     sbLineName);
        printf("\tLine Name = %s", sbLineName);
        printf("\tEdge = ( %s , %s )\n", AdjListTable[pTopStack->Node1].sbNodeName,
               AdjListTable[pTopStack->Node2].sbNodeName);
        if (ArtPt[pTopStack->Node1] != NULL)
            AddBlock(pTopStack->Node1);
        if (ArtPt[pTopStack->Node2] != NULL)
            AddBlock(pTopStack->Node2);
        pTopStack = pTopStack->pPrevEntry;
        }
    MakeLineName(AdjListTable[pTopStack->Node1].sbNodeName,
	         AdjListTable[pTopStack->Node2].sbNodeName,
		 sbLineName);
    printf("\tLine Name = %s", sbLineName);
    printf("\tEdge = ( %s , %s )\n", AdjListTable[pTopStack->Node1].sbNodeName,
           AdjListTable[pTopStack->Node2].sbNodeName);
    if (ArtPt[pTopStack->Node1] != NULL)
        AddBlock(pTopStack->Node1);
    if (ArtPt[pTopStack->Node2] != NULL)
        AddBlock(pTopStack->Node2);
    pTopStack = pTopStack->pPrevEntry;

    return(RootBlock);
}

/*
 * AddBlock(iCurrNode) -- Add the current block number to the list of
 *     containing blocks for the articulation point, iCurrNode, unless
 *     it was previously added. In this case the call is ignored. This
 *     routine builds the block cross reference.
 */

AddBlock(iCurrNode)
int iCurrNode;
{
    struct BlkXRef *pBlock;

    if (ArtPt[iCurrNode] != NULL && ArtPt[iCurrNode]->BlockNum == cBlockNum)
        return;
    
    pBlock = New(struct BlkXRef);
    if (pBlock == NULL) 
        FatalError("AddBlock");
    pBlock->BlockNum = cBlockNum;
    pBlock->pNextBlock = ArtPt[iCurrNode];
    ArtPt[iCurrNode] = pBlock;
    return;
}

/*
 * PushEdgeStack(iCurrNode, iAdjNode) -- pushes an edge on to the edge stack.
 */

PushEdgeStack(iCurrNode, iAdjNode)
int iCurrNode, iAdjNode;
{
    struct StackEntry *pNewEntry;

    pNewEntry = New(struct StackEntry);
    if (pNewEntry == NULL) 
        FatalError("PushEdgeStack");
    pNewEntry->Node1 = iCurrNode;
    pNewEntry->Node2 = iAdjNode;
    pNewEntry->pPrevEntry = pTopStack;
    pTopStack = pNewEntry;

    return;
}
