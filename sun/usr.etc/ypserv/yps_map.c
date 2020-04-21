#ifndef lint
/* @(#)ypserv_map.c	2.1 86/04/16 NFSSRC */
static	char sccsid[] = "@(#)ypserv_map.c 1.1 86/02/05 Copyr 1985 Sun Micro";
#endif

#include "ypsym.h"
#include <ctype.h>
char current_map[YPDBPATH_LENGTH + YPMAXDOMAIN + YPMAXMAP + 3];
static char map_owner[MAX_MASTER_NAME + 1];

#ifdef sgi
/*
 * Outer layer for dbminit.
 *
 * The map names as defined by SUN are too long to work as file
 * names on systems that have traditional 14 character file names.
 * Hide this limitation from YP clients by simply mapping the map
 * names to shorter names and then calling dbminit.
 *
 * The routine that does the name mapping is external since it is
 * also required by the YP database creation script.
 */
int
ypdbminit(dbname)
char *dbname;
{
	char realname[512];

	if (yp_make_dbname(dbname, realname, sizeof realname) < 0)
		return(-1);
	return(dbminit(realname));
}
#endif

/*
 * This performs an existence check on the dbm data base files <name>.pag and
 * <name>.dir.  pname is a ptr to the filename.  This should be an absolute
 * path.
 * Returns TRUE if the map exists and is accessable; else FALSE.
 *
 * Note:  The file name should be a "base" form, without a file "extension" of
 * .dir or .pag appended.  See ypmkfilename for a function which will generate
 * the name correctly.  Errors in the stat call will be reported at this level,
 * however, the non-existence of a file is not considered an error, and so will
 * not be reported.
 */
bool
ypcheck_map_existence(pname)
	char *pname;
{
	char dbfile[MAXNAMLEN + 1];
	struct stat filestat;
	int len;

	if (!pname || ((len = strlen(pname)) == 0) ||
	    (len + 5) > (MAXNAMLEN + 1) ) {
		return(FALSE);
	}
		
	errno = 0;
	(void) strcpy(dbfile, pname);
	(void) strcat(dbfile, ".dir");

	if (stat(dbfile, &filestat) != -1) {
		(void) strcpy(dbfile, pname);
		(void) strcat(dbfile, ".pag");

		if (stat(dbfile, &filestat) != -1) {
			return(TRUE);
		} else {

			if (errno != ENOENT) {
				(void) fprintf(stderr,
				    "ypserv:  Stat error on map file %s.\n",
				    dbfile);
			}

			return(FALSE);
		}

	} else {

		if (errno != ENOENT) {
			(void) fprintf(stderr,
			    "ypserv:  Stat error on map file %s.\n",
			    dbfile);
		}


		return(FALSE);
	}

}

/*
 * The retrieves the order number of a named map from the order number datum
 * in the map data base.  
 */
bool
ypget_map_order(map, domain, order)
	char *map;
	char *domain;
	unsigned *order;
{
	datum key;
	datum val;
	char toconvert[MAX_ASCII_ORDER_NUMBER_LENGTH + 1];
	unsigned error;

	if (ypset_current_map(map, domain, &error) ) {
		key.dptr = order_key;
		key.dsize = ORDER_KEY_LENGTH;
		val = fetch(key);

		if (val.dptr != (char *) NULL) {

			if (val.dsize > MAX_ASCII_ORDER_NUMBER_LENGTH) {
				return(FALSE);
			}

			/*
			 * This is getting recopied here because val.dptr
			 * points to static memory owned by the dbm package,
			 * and we have no idea whether numeric characters
			 * follow the order number characters, nor whether
			 * the mess is null-terminated at all.
			 */

			bcopy(val.dptr, toconvert, val.dsize);
			toconvert[val.dsize] = '\0';
			*order = (unsigned long) atol(toconvert);
			return(TRUE);
		} else {
		    return(FALSE);
		}
		    
	} else {
		return(FALSE);
	}
}

/*
 * The retrieves the master server name of a named map from the master datum
 * in the map data base.  
 */
bool
ypget_map_master(map, domain, owner)
	char *map;
	char *domain;
	char **owner;
{
	datum key;
	datum val;
	unsigned error;

	if (ypset_current_map(map, domain, &error) ) {
		key.dptr = master_key;
		key.dsize = MASTER_KEY_LENGTH;
		val = fetch(key);

		if (val.dptr != (char *) NULL) {

			if (val.dsize > MAX_MASTER_NAME) {
				return(FALSE);
			}

			/*
			 * This is getting recopied here because val.dptr
			 * points to static memory owned by the dbm package.
			 */
			bcopy(val.dptr, map_owner, val.dsize);
			map_owner[val.dsize] = '\0';
			*owner = map_owner;
			return(TRUE);
		} else {
		    return(FALSE);
		}
		    
	} else {
		return(FALSE);
	}
}

/*
 * This makes a map into the current map, and calls dbminit on that map so
 * that any successive dbm operation is performed upon that map.  Returns an
 * YP_xxxx error code in error if FALSE.  
 */
bool
ypset_current_map(map, domain, error)
	char *map;
	char *domain;
	unsigned *error;
{
	char mapname[YPDBPATH_LENGTH + YPMAXDOMAIN + YPMAXMAP + 3];
	int lenm, lend;

	if (!map || ((lenm = strlen(map)) == 0) || (lenm > YPMAXMAP) ||
	    !domain || ((lend = strlen(domain)) == 0) || (lend > YPMAXDOMAIN)) {
		*error = YP_BADARGS;
		return(FALSE);
	}

	ypmkfilename(domain, map, mapname);

	if (strcmp(mapname, current_map) == 0) {
		return(TRUE);
	}

	ypclr_current_map();

	if (ypdbminit(mapname) >= 0) {
		(void) strcpy(current_map, mapname);
		return(TRUE);
	}

	ypclr_current_map();
	
	if (ypcheck_domain(domain)) {

		if (ypcheck_map_existence(mapname)) {
			*error = YP_BADDB;
		} else {
			*error = YP_NOMAP;
		}
		
	} else {
		*error = YP_NODOM;
	}

	return(FALSE);
}

/*
 * This checks to see if there is a current map, and, if there is, does a
 * dbmclose on it and sets the current map name to null.  
 */
void
ypclr_current_map()

{

	if (current_map[0] != '\0') {
		(void) dbmclose();
		current_map[0] = '\0';
	}

}
