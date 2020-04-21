/*
 *	fsort.c - 
 *		interface between the C library routine qsort()
 *		and the fortran version of the dbx test routine.
 *	
 *	qsort() is a C library routine which takes four arguments.  
 *	These are
 *		the array of record indices (index)
 *		the number of records (nrecs)
 *		the size of an element (the size of an integer)
 *		the address of a routine to pass pointers to two
 *			elements to compare them (cmprec)
 *	
 *	The comparison routine (cmprec) is written in FORTRAN.
 *	Since there is no method of telling qsort() this, 
 *	we generate a wrapper for cmprec (see sort.cf) and call a C 
 *	routine en route to qsort().  This C routine passes 
 *	qsort() a pointer to (the C entry of) cmprec.
 *	
 *	The wrapper for our FORTRAN program to call fsort() through
 *	is generated using fsort.fc.
 *	
 *
*/
 
int cmprec();

/* CENTRY */
fsort(sortarray,nelements,elsize)
int sortarray[];
int nelements,elsize;
{

	qsort(sortarray,nelements,elsize,cmprec);

}
/* ENDCENTRY */

