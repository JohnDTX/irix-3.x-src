#define SERIAL_COM	234
#define ETHER_COM	235
#define I3270_COM	237

#define TCP_TYPE	1
#define XNS_TYPE	2
#define SERIAL_TYPE	3

extern unsigned char *wp, *rp;
extern unsigned wc;
extern int rc;

/*#define RWBUFSIZE	1024
#define WFLUSHLIMIT	100*/
#define WFLUSHLIMIT	1431
#define WBUFFSIZE	1431

#define gethostchar() 	(--rc >= 0 ? *rp++ : fillhostbuffer())
#define puthostchar(c)  \
	    { *wp++ = (c); if (++wc >= WFLUSHLIMIT) flushhost(); }
