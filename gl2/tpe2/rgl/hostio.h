#define SERIAL_COM	234
#define ETHER_COM	235
#define I488_COM	236
#define I3270_COM	237

extern unsigned char *wp, *rp;
extern int wc, rc;

#define WBUFFSIZE	1437

#ifdef TCP
extern char TelnetInput();

#define IAC 255			/* Telnet escape character */
#define getrawhostchar()  (--rc >= 0 ? *rp++ : fillhostbuffer())
#define gethostchar()	(((getrawhostchar() & 255) != IAC) ? *(rp-1) : \
				TelnetInput())
#else
#define gethostchar() 	(--rc >= 0 ? *rp++ : fillhostbuffer())
#endif TCP

#define puthostchar(c)  {*wp++ = (c); if(++wc >= WBUFFSIZE) flushhostbuffer();}
