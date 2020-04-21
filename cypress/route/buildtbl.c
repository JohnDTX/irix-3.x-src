/* 
 * buildtbl.c - invoked to parse the input data files and build the connect
 * table and line table.
 * 
 * Author:	Greg Smith
 * 		Dept. of Computer Sciences
 * 		Purdue University
 * 
 * Date:	Sat Aug 10 1985
 *
 */


#include <stdio.h>
#include <netdb.h>
#include "defs.h"

extern int cLineNum;
extern int cLineCurr;
extern int cConnectCurr;
extern int cAdjListCurr;
extern int cError;
FILE *fdCurr;
extern char *sbFileName;
extern struct Connect ConnectTable[];
extern int CmpNetName();

BuildTable()

{

    /* open the input data files in turn and build */
    /* the corresponding table                     */
    sbFileName = "connect.dat";
    if ((fdCurr = fopen(CONNECTFILE, "r")) == NULL)
        FatalError("input data file connect.dat not found");
    else {
        cLineNum = 1;
        yyparse();
        fclose(fdCurr);

        /* building adjacency list table requires connection */
        /* table to be sorted by node name                   */
        qsort(ConnectTable, cConnectCurr, sizeof(struct Connect), CmpNetName);
        }

    sbFileName = "line.dat";
    if ((fdCurr = fopen(LINEFILE, "r")) == NULL)
        FatalError("input data file line.dat not found");
    else {
        cLineNum = 1;
        yyparse();
        fclose(fdCurr);
    }

    if (cError == 0) {
        /* build the adjacency list representation of the network */
        BuildAdjList();

        /* now add the weights to the lines connecting cypress sites */
        WeightLines();
    }
}
