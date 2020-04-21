#include "inst.h"
#include "idb.h"
#include <filehdr.h>

/* compute checksum according to the "sum -r" algorithm; error return (-1) */

Checksum
chksumbuff (sum, buff, n)
	register Checksum	sum;
	register char		*buff;
	register int		n;
{
	register char		*buffe;

	for (buffe = buff + n; buff < buffe; ++buff) {
		if (sum & 01) {
			sum = (sum >> 1) + 0x8000;
		} else {
			sum >>= 1;
		}
		sum += *buff;
		sum &= 0xFFFF;
	}
	return (sum);
}

Checksum
chksum (name, sizep, sump)
	char		*name;
	long		*sizep;
	long		*sump;
{
	unsigned int	sum;
	char		buff [8192];
	int		n, f;

	*sizep = *sump = 0;
	if ((f = open (name, 0)) < 0) {
		return (-1);
	}
	sum = 0;
	while ((n = read (f, buff, sizeof (buff))) > 0) {
		*sizep += n;
		sum = chksumbuff (sum, buff, n);
	}
	close (f);
	*sump = sum;
	return (n);
}

Checksum
xchksum (name, sizep, sump, rec)
	char		*name;
	long		*sizep;
	long		*sump;
	Rec		*rec;
{
	unsigned int	sum;
	unsigned char	buff [8192];
	int		n, f, size;
	FILHDR		*hdr;
#if R2300 || IP4
	int		strip;
#endif

	*sizep = *sump = 0;
	if ((f = open (name, 0)) < 0) {
		return (-1);
	}
#if R2300 || IP4
	strip = (rec->mode & 0111) && idb_getattr ("nostrip", rec) == NULL;
#endif
	sum = 0;
	size = maxint (int);
	while (size && (n = read (f, buff, sizeof (buff))) > 0) {
#if R2300 || IP4
		if (strip) {
			hdr = (FILHDR *) buff;
			if (n >= sizeof (FILHDR) &&
			    (hdr->f_magic == MIPSEBMAGIC ||
			    hdr->f_magic == MIPSELMAGIC ||
			    hdr->f_magic == MIPSEBUMAGIC ||
			    hdr->f_magic == MIPSELUMAGIC) &&
			    hdr->f_symptr != 0 && hdr->f_nsyms != 0) {
				size = hdr->f_symptr;
				hdr->f_symptr = 0;
				hdr->f_nsyms = 0;
			}
			strip = 0;
		}
#endif
		if (n > size) n = size;
		*sizep += n;
		size -= n;
		sum = chksumbuff (sum, buff, n);
	}
	close (f);
	*sump = sum;
	return (n < 0 ? -1 : 0);
}
