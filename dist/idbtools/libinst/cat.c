#define NULL 0

/* catenate product, image, and subsys names into standard form */

char *
cat (pname, iname, sname)
	register char	*pname;
	register char	*iname;
	register char	*sname;
{
	register char	*p;
	static char	buff [1024];
	
	p = buff;
	while (*p = *pname++) ++p;
	if (iname == NULL) return (buff);
	*p++ = '.';
	while (*p = *iname++) ++p;
	if (sname == NULL) return (buff);
	*p++ = '.';
	while (*p = *sname++) ++p;
	return (buff);
}

/* uncatenate buff into product, image, and subsys names */

uncat (buff, pname, iname, sname)
	register char	*buff;
	register char	*pname;
	register char	*iname;
	register char	*sname;
{
	while (*buff != '\0' && *buff != '.') {
		if (pname != NULL) *pname++ = *buff;
		++buff;
	}
	if (*buff == '.') ++buff;
	while (*buff != '\0' && *buff != '.') {
		if (iname != NULL) *iname++ = *buff;
		++buff;
	}
	if (*buff == '.') ++buff;
	while (*buff != '\0') {
		if (sname != NULL) *sname++ = *buff;
		++buff;
	}
	if (pname != NULL) *pname++ = '\0';
	if (iname != NULL) *iname++ = '\0';
	if (sname != NULL) *sname++ = '\0';
}
