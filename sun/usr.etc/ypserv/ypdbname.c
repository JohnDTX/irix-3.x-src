#ifdef sgi
/*
 * Yellow pages utility routine to map client map names to names
 * that are short enough to work with 14 character file names.
 */

/*
 * Translation table for mapping dbnames.
 *
 * The mapped name must be short enough to remain unique
 * after the addition of the suffixes ".t.dir" and ".t.pag".
 * The 'makedbm' program adds ".t" and the dbm package
 * adds the ".dir" and ".pag".  With 14 character names,
 * 10 characters is the limit.
 *
 * Let's not try to be cute for now, just an easy table
 * lookup will do the trick.
 */
static char *yp_name_map[] = {

	/*    From                 To	*/
	"passwd.byname",	"pw.name",
	"passwd.byuid",		"pw.uid",
	"group.byname",		"grp.name",
	"group.bygid",		"grp.gid",
	"hosts.byname",		"hosts.name",
	"hosts.byaddr",		"hosts.addr",
	"protocols.byname",	"proto.name",
	"protocols.bynumber",	"proto.nbr",
	"services.byname",	"svc.name",
	"networks.byname",	"nets.name",
	"networks.byaddr",	"nets.addr",
	"mail.aliases",		"aliases",
	"netgroup",		"netgrp",
	"netgroup.byuser",	"netgrp.usr",
	"netgroup.byhost",	"netgrp.hst",
	"ethers.byaddr",	"ether.addr",
	"ethers.byname",	"ether.name",
	"rpc.bynumber",		"rpc.nbr",

	0	/* end of list */
};

/*
 * YP database name translation
 *
 * The map names as defined by SUN are too long to work as file
 * names on systems that have traditional 14 character file names.
 * Hide this limitation from YP clients by simply mapping the map
 * names to shorter names.
 */
int
yp_make_dbname(name, newname, len)
register char *name;	/* input name to be translated */
char *newname;		/* buffer for returned name */
int len;		/* length of 'newname' buffer */
{
	register char **np;

	/* 
	 * Buzz the translation table looking at even numbered
	 * entries for a matching name.
	 */
	for (np = yp_name_map; *np; np += 2) {
		/*
		 * If the name matches, the following odd numbered
		 * entry is the translation.
		 */
		if (strcmp(*np, name) == 0) {
			np++;
			break;
		}
	}

	/*
	 * If no match, just use the original name
	 */
	if (*np == 0)
		strncpy(newname, name, len);
	else
		strncpy(newname, *np, len);
}
#endif sgi
