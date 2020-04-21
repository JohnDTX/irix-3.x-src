/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
static char sccsid[] = "@(#)dbm.c	5.3 (Berkeley) 85/08/15";
#endif not lint

#include	"dbm.h"

#define	NODB	((DBM *)0)

static DBM *cur_db = NODB;

static char no_db[] = "dbm: no open database\n";

dbminit(file)
	char *file;
{
	if (cur_db != NODB)
		dbm_close(cur_db);

	cur_db = dbm_open(file, 2, 0);
	if (cur_db == NODB) {
		cur_db = dbm_open(file, 0, 0);
		if (cur_db == NODB)
			return (-1);
	}
	return (0);
}

void
dbmclose()
{
	if (cur_db == NODB)
		return;
	
	dbm_close(cur_db);
	cur_db = NODB;
}

long
forder(key)
datum key;
{
	if (cur_db == NODB) {
		printf(no_db);
		return (0L);
	}
	return (dbm_forder(cur_db, key));
}

datum
fetch(key)
datum key;
{
	datum item;

	if (cur_db == NODB) {
		printf(no_db);
		item.dptr = 0;
		return (item);
	}
	return (dbm_fetch(cur_db, key));
}

delete(key)
datum key;
{
	if (cur_db == NODB) {
		printf(no_db);
		return (-1);
	}
	if (dbm_rdonly(cur_db))
		return (-1);
	return (dbm_delete(cur_db, key));
}

store(key, dat)
datum key, dat;
{
	if (cur_db == NODB) {
		printf(no_db);
		return (-1);
	}
	if (dbm_rdonly(cur_db))
		return (-1);

	return (dbm_store(cur_db, key, dat, DBM_REPLACE));
}

datum
firstkey()
{
	datum item;

	if (cur_db == NODB) {
		printf(no_db);
		item.dptr = 0;
		return (item);
	}
	return (dbm_firstkey(cur_db));
}

datum
nextkey(key)
datum key;
{
	datum item;

	if (cur_db == NODB) {
		printf(no_db);
		item.dptr = 0;
		return (item);
	}
#ifdef sgi
	/*
	 * Position to the given key
	 */
	item = dbm_fetch(cur_db, key);
	if (item.dptr == 0)
		return (item);
	/*
	 * There is actually no interaction between dbm_fetch
	 * and dbm_nextkey.  We must supply the interaction
	 * here by setting the fields used to implement
	 * dbm_nextkey.
	 */
	cur_db->dbm_blkptr = cur_db->dbm_pagbno;
	cur_db->dbm_keyptr = finddatum(cur_db->dbm_pagbuf, key) + 2;
	return (dbm_nextkey(cur_db));
#else
	/*
	 * This is the implementation as it came from
	 * Berkeley.  This is not really compatible
	 * with the old libdbm, because the key is
	 * actually ignored by dbm_nextkey.
	 */
	return (dbm_nextkey(cur_db, key));
#endif
}
