# include "sys/types.h"
# include "Xns.h"

bootstr(tgt,len,ext,file)
    char *tgt;
    int len;
    char *ext,*file;
{
    extern char *strlcpy();

    register char *tp,*last;

    tp = tgt;
    last = tp+len-7;

    if( *file == 000 )
    {
	*tp++ = SERV_BOOTIRIS;
	file = "config";
    }
    else
    {
	*tp++ = SERV_SENDFILE;
    }
    last++;
    *tp++ = ':';

    if( *ext == 000 )
	ext = "*";
    tp = strlcpy(tp,last,ext);
    last++;
    *tp++ = ':';

    tp = strlcpy(tp,last,file);
    *tp++ = 000;
    *tp++ = 000;
    *tp++ = 000;
    *tp = 000;
}
