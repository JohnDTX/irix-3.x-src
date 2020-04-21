char getstrbuf[100];

/*static*/ char *pushedstring;

ungetstr(str)
    char *str;
{
    pushedstring = str;
}

char *getstr()
{
    if( pushedstring != 0 )
    {
	strncpy(getstrbuf,pushedstring,sizeof getstrbuf-1);
	pushedstring = 0;
	return getstrbuf;
    }

    getnline(getstrbuf,sizeof getstrbuf);

    return getstrbuf;
}
