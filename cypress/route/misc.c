#include <stdio.h>
#include <strings.h>
#include "defs.h"

/* 
 * misc.c - routines used by the parser and by the routines that build
 * the various tables, the routing routines, and the graph routines.
 * 
 * programmers -- Greg Smith & Cathy Privette
 * 
 * date -- 3-20-1985
 *
 * modification history
 *
 * 9-11-85 - SeparateName modified to separate names at a '/' rather than
 *	     a '-'.  This is because it will be used to separate line names
 *	     which are now in the form site1-cypress/site2-cypress
 *
 *	     MakeLineName modified to construct line names in the form given
 *	     above.
 */

char *malloc();

/*
 * MakeString(sb) - create a copy of string sb.
 *
 * date -- 3-20-1985
 *
 */

char *MakeString(sb)

char *sb;

{
    char *sbNew = malloc(strlen(sb) + 1);

    if (sbNew == NULL)
	FatalError("MakeString");

    strcpy(sbNew, sb);
    return(sbNew);
}


extern int cLineNum;
extern int cError;

/*
 * PrintMsg.c - invoked to print a message on the standard output
 *	        file.
 * 
 * date -- 20 June 1985
 *
 *
 */

PrintMsg(sbMessage)
 
char *sbMessage;
 
{
    printf("line %d: %s\n", cLineNum, sbMessage);
}

/*
 * yyerror.c - invoked to print an error message on the standard error 
 *	       file.  Different from others in that it increments the count
 *	       of errors encountered.
 * 
 * date -- 20 June 1985
 *
 */

yyerror(sbErrorMsg)
 
char *sbErrorMsg;
 
{
    extern char *sbFileName;

    fprintf(stderr, "%s line %d: %s\n", sbFileName, cLineNum, sbErrorMsg);
    cError += 1;
}

/*
 * FatalError.c - display a message and die
 *
 * date -- 25 June 1985
 *
 */

FatalError(sbMsg)

char *sbMsg;

{
    fprintf(stderr, "%s\n", sbMsg);
    exit(1);
}


/*
 * CmpNetName.c - the comparison routine used by the qsort library routine
 *		  to sort the connection table by node name.
 *
 * date -- 28 June 1985
 *
 */

CmpNetName(sbNode1, sbNode2)

struct Connect *sbNode1;
struct Connect *sbNode2;

{
    return(strcmp(sbNode1->sbNodeName, sbNode2->sbNodeName));
}

/*
 * Cmp_C_Number - used as a comparison routine by the call to qsort in 
 * Output to sort the Cypress routing information by c-number.
 * 
 */

Cmp_C_Number(arg1, arg2)

struct CypRoute *arg1, *arg2;

{
    return(strcmp(arg1->c_number, arg2->c_number));
}

/*
 * CmpLineName - comparison routine used by sort when sorting line
 * table.
 * 
 */

CmpLineName(arg1, arg2)

struct Line *arg1, *arg2;

{
    return(strcmp(arg1->sbLineName, arg2->sbLineName));
}

/*
 * DisplayLine.c - invoked to display the phone line table.
 * 
 */

DisplayLine()

{

    extern int cLineCurr;    /* count of entries in the line table     */
    extern struct Line LineTable[];

    int i;			  /*		loop index variable	    */

    /* display the phone line table */
    printf("Line Table \n");
    for (i = 0; i < cLineCurr; i++) {
       printf("\nLine Name: %s\n\n", LineTable[i].sbLineName);
       printf("\t\tSpeed: %d\n", LineTable[i].speed);
       printf("\t\tProtocol: %s\n", LineTable[i].sbProtocol);
       printf("\t\tBill to: %s\n", LineTable[i].sbBillTo);
       printf("\t\tCircuit Number: %s\n", LineTable[i].sbCircuitNum);
    }
}


/*
 * DisplayConnect.c - invoked to display the connect table.
 * 
 * 
 */

DisplayConnect()

{
    extern int cConnectCurr;
    extern struct Connect ConnectTable[];
    int i;


    if (cConnectCurr > 0) {
        printf("\n\nConnection Table\n\n");
        printf("%-20s %-20s \t\t%-10s\n", "Node Name", "Internet Addr", "Links");
        }

    for (i = 0; i < cConnectCurr; i++) {
         printf("\n%-20s %-20s\n", ConnectTable[i].sbNodeName,
				   ConnectTable[i].sbNetNumber);

         /* display the connections */
         pLinks = ConnectTable[i].pConnections;

         while (pLinks != NULL) {
	     printf("\t\t\t\t\t\t%-20s %-5d\n", pLinks->sb, pLinks->w);
             pLinks = pLinks->pConnections;
             }
         }
}


/*
 * input.c - input routine invoked by the parser.  Reads until EOF from the
 *           connect and phone data files which are assumed to be
 *           open.
 *
 */

input()

{
    extern  FILE * fdCurr;
    int     ch;

    return ((ch = getc (fdCurr)) == EOF ? 0 : ch);

}

/*
 * unput.c - invoked by the scanner to return a character to the input stream
 *           pointed to by fdCurr.
 *
 */

unput(ch)

int ch;

{
    extern  FILE * fdCurr;

    ungetc (ch, fdCurr);
}

/*
 * SearchAdjListTable(sbNetNumber) -- searches AdjListTable and returns the
 *     index of NetNumber in AdjListTable. If it is not there it returns -1.
 */

SearchAdjListTable(sbNetNumber)

char *sbNetNumber;

{
    extern int cAdjListCurr;
    extern struct AdjList AdjListTable[];
    int low, high, mid, cond;
 
    low = 0;
    high = cAdjListCurr - 1;

    while(low <= high) {
        mid = (low + high) / 2;
	if ((cond = strcmp(sbNetNumber, AdjListTable[mid].sbNetNumber)) < 0)
            high = mid - 1;
        else if (cond > 0) 
            low = mid + 1;
        else
            return(mid);
        }
    return(-1);
}

/* 
 * SearchConnectTable(sbNdName) -- Searches the ConnectTable and returns the
 *     index of sbNdName in ConnectTable. If it is not there it returns -1.
 * 
 */

SearchConnectTable(sbNdName)
char *sbNdName;
{
    int     low,
            high,
            mid,
            cond;
    extern struct Connect   ConnectTable[];

    low = 0;
    high = cConnectCurr - 1;

    while (low <= high) {
	mid = (low + high) / 2;

	if ((cond = strcmp (sbNdName, ConnectTable[mid].sbNodeName)) < 0)
	    high = mid - 1;
	else
	    if (cond > 0)
		low = mid + 1;
	    else
		return (mid);
    }
    return (-1);
}

/*
 * SearchLineTable - invoked to search the cypress line table for the
 * cypress leased line given as an argument.
 * 
 */

SearchLineTable(sbLineName)

char *sbLineName;

{
    int low, high, mid, cond;
    extern int cLineCurr;
    extern struct Line LineTable[];

    low = 0;
    high = cLineCurr;    

    while (low <= high) {
        mid = (low + high) / 2;
        if ((cond = strcmp(sbLineName, LineTable[mid].sbLineName)) < 0)
             high = mid - 1;
        else if (cond > 0)
		 low = mid + 1;
        else return(mid);
        }
    return(-1);
}


/*
 * ComparAdjListNetNo(pEntry1, pEntry2) -- Compares the network number
 *     in AdjListTable entry1 and entry2 and returns an integer <, =, or
 *     > 0 depending on entry1's network number being <, =, or > entry2's
 *     network number. To be used by qsort.
 */

ComparAdjListNetNo(pEntry1, pEntry2)
struct AdjList *pEntry1, *pEntry2;

{
    return(strcmp(pEntry1->sbNetNumber, pEntry2->sbNetNumber));
}

/*
 * SeparateName(sbOrigName, sbName1, sbName2) -- separates a name of the 
 *     form "name1/name2" in sbOrigName into two names, "name1" and "name2"
 *     in sbName1 and sbName2 respectively.
 *
 */

SeparateName(sbOrigName, sbName1, sbName2)
char *sbOrigName, *sbName1, *sbName2;
{
    while ((*sbName1 = *sbOrigName) != '/') { 
        sbName1++ ;
        sbOrigName++ ;
        }

    *sbName1 = '\0';   /* end of name1 */
    sbOrigName++ ;

    while ((*sbName2 = *sbOrigName) != '\0') {
        sbName2++ ;
        sbOrigName++ ;
        }

    return;
}

/*
 * MakeLineName - constructs a name for a cypress leased line from its 
 *                arguments
 *
 */

MakeLineName(sbSite1, sbSite2, sbLineName)

char *sbSite1;
char *sbSite2;
char *sbLineName;

{
    strcpy(sbLineName, sbSite1);
    strcat(sbLineName, "/");
    strcat(sbLineName, sbSite2);
}

