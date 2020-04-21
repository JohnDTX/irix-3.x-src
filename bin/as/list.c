#include "mical.h"

FILE *List_file;	/* listing output file descriptor */

/*
 * produce listing of comment lines
 */
listline()
{
	if (listf == 0 || Pass != 2)
		return;
	if (iline[0] != '\n' || iline[1] != 0)
		listleft("");
	if (fprintf(List_file, "%s", iline) < 0)
		Sys_Error("listline:  fprintf failed\n", (char *) NULL);
}

listdata(ct, type, v)
register ct;
long v[];
{
	register i, j;
	char b[64], b1[8], b2[8], b3[8], b4[8], b5[8], b6[8];
	int inc;

	if (listf == 0 || Pass != 2)
		return;
	inc = (type == B) ? 5 : 3;	/* max number of items per line */
	if ((i = ct) < 0)
		ct = -ct;
	for (j = 0; j < ct || (j == 0 && ct == 0); j += inc) {
		b2[0] = b3[0] = b4[0] = b5[0] = b6[0] = 0;
		if (j == 0 && i >= 0) {
#ifdef PWB
			sprintf(b1, "%.6lx ", (long)Dot);
#else
			sprintf(b1, "%06lx ", (long)Dot);
#endif
			listcvt(b1);
		} else
			sprintf(b1, "       ");
		if (type != B) {
		    if (j+1 <= ct) {
#ifdef PWB
			sprintf(b2, "%.4lx ", v[j] & 0xFFFFL); listcvt(b2); }
#else
			sprintf(b2, "%04lx ", v[j] & 0xFFFFL); listcvt(b2); }
#endif
		    if (j+2 <= ct) {
#ifdef PWB
			sprintf(b3, "%.4lx ", v[j+1] & 0xFFFFL); listcvt(b3); }
#else
			sprintf(b3, "%04lx ", v[j+1] & 0xFFFFL); listcvt(b3); }
#endif
		    if (j+3 <= ct) {
#ifdef PWB
			sprintf(b4, "%.4lx ", v[j+2] & 0xFFFFL); listcvt(b4); }
#else
			sprintf(b4, "%04lx ", v[j+2] & 0xFFFFL); listcvt(b4); }
#endif
		} else {
		    if (j+1 <= ct) {
#ifdef PWB
			sprintf(b2, "%.2lx ", v[j] & 0xFFL); listcvt(b2); }
#else
			sprintf(b2, "%02lx ", v[j] & 0xFFL); listcvt(b2); }
#endif
		    if (j+2 <= ct) {
#ifdef PWB
			sprintf(b3, "%.2lx ", v[j+1] & 0xFFL); listcvt(b3); }
#else
			sprintf(b3, "%02lx ", v[j+1] & 0xFFL); listcvt(b3); }
#endif
		    if (j+3 <= ct) {
#ifdef PWB
			sprintf(b4, "%.2lx ", v[j+2] & 0xFFL); listcvt(b4); }
#else
			sprintf(b4, "%02lx ", v[j+2] & 0xFFL); listcvt(b4); }
#endif
		    if (j+4 <= ct) {
#ifdef PWB
			sprintf(b5, "%.2lx ", v[j+3] & 0xFFL); listcvt(b5); }
#else
			sprintf(b5, "%02lx ", v[j+3] & 0xFFL); listcvt(b5); }
#endif
		    if (j+5 <= ct) {
#ifdef PWB
			sprintf(b6, "%.2lx ", v[j+4] & 0xFFL); listcvt(b6); }
#else
			sprintf(b6, "%02lx ", v[j+4] & 0xFFL); listcvt(b6); }
#endif
		}
		sprintf(b, "%s%s%s%s%s%s", b1, b2, b3, b4, b5, b6);
		listleft(b);
		if (j == 0) {
			if (fprintf(List_file, "%s", iline) < 0)
				Sys_Error("listdata:  fprintf failed\n", (char *) NULL);
		}
		else if (fprintf(List_file, "\n") < 0)
			Sys_Error("listdata:  fprintf failed\n", (char *) NULL);
	}
}

listinst(length, code)
register char *code;
{
	register i;
	long a[5];

	if ((length & 1) != 0)
		if (fprintf(List_file, "listinst: length not even\n") < 0)
			Sys_Error("listinst:  fprintf failed\n", (char *) NULL);
	if (length > 10) {
		if (fprintf(List_file, "listinst: length %d too large\n", length) < 0)
			Sys_Error("listinst:  fprintf failed\n", (char *) NULL);
		return;
	}
	i = 0;
	while (length > 0) {
		length -= 2;
		a[i] = *code++ << 8;
		a[i++] |= (*code++ & 0xFF);
	}
	listdata(i, W, a);
}

listwords(Which)
  register int Which;
  {	register int i, j;
	register struct oper *p;
	long a[OPERANDS_MAX * 2];

	for (i=0, j=0, p=operands; i < numops; i++, p++) {
	  switch (Which) {
	    case L:	a[j++] = p->value_o >> 16;
			a[j++] = p->value_o;
			break;
	    case W:	a[j++] = p->value_o;
			break;
	    case B:	a[j++] = p->value_o;
			break;
	  }
	}
	listdata(j, Which, a);
}

listascii(p, zero)
register char *p;
{
	register n;
	long a[100];

	n = 0;
	while(*p)
		a[n++] = *p++;
	if (zero)
		a[n++] = 0;
	listdata(n, B, a);
}

listleft(str)
char *str;
{
	if (fprintf(List_file, "%-23s ", str) < 0)
		Sys_Error("listleft:  fprintf failed\n", (char *) NULL);
}

listcvt(p)
register char *p;
{
	while (*p) {
		if (*p >= 'a' && *p <= 'f')
			*p = *p + ('A' - 'a');
		p++;
	}
}
