/*	in_cksum.c	6.1	83/07/29	*/

#include "../h/param.h"
#include "../h/mbuf.h"
#include "../netinet/in.h"
#include "../netinet/in_systm.h"

/*
 * Checksum routine for Internet Protocol family headers (VAX Version).
 *
 * This routine is very heavily used in the network
 * code and should be modified for each CPU to be as fast as possible.
 */

in_cksum(m, len)
register struct mbuf *m;		/* a5 */
	register int len;		/* d7 */
{
	register u_short *w;		/* on 68010/68020, known to be a4 */
	register u_int sum = 0;		/* on 68010/68020, known to be d6 */
	register int mlen = 0;

#define	ADDL		asm("	addl a4@+, d6");
#define	ADDX		asm("	movl a4@+, d0"); asm("	addxl d0, d6");
#define	ADD_LASTCARRY	asm("	moveq #0, d0"); asm("	addxl d0, d6");

#define FOLD_SUM	asm("	movl d6, d0"); asm("	swap d0"); \
	asm("	addw d0, d6"); ADD_LASTCARRY; asm("	andl #0xFFFF, d6");


	for (;;) {
		/*
		 * Each trip around loop adds in
		 * words from one mbuf segment.
		 */
		w = mtod(m, u_short *);
		if (mlen == -1) {
			/*
			 * There is a byte left from the last segment;
			 * add it into the checksum.  Don''t have to worry
			 * about a carry-out here because we make sure
			 * that high part of (32 bit) sum is small below.
			 */
			sum += *(u_char *)w;
			w = (u_short *)((char *)w + 1);
			mlen = m->m_len - 1;
			len--;
		} else
			mlen = m->m_len;
		m = m->m_next;
		if (len < mlen)
			mlen = len;
		len -= mlen;

#ifdef PM2
		/* We must check for odd alignment on the 68010
		 * This temporary hack (XXX speed it up) computes a sum
		 * that will fit in < 32 bits, provided we have < 32k words.
		 */
		if (((int)w&0x1) && mlen >= 2) {
			sum += *(u_char *)w << 8;
			w = (u_short *)((u_char *)w + 1);
			while ((mlen -= 2) > 0) {
				asm("	moveq #0,d0");
				asm("	movw a4@+,d0");
				asm("	rolw #8,d0");
				asm("	addl d0,d6");
			}
			if (!mlen) {	/* get trailing byte */
				sum += *(u_char *)w;
				w = (u_short *)((u_char *)w + 1);
			} else {
 				ASSERT(-1 == mlen);
			}
			/* leave mlen=0 or -1 depending on whether the 1st
			 * byte of the next mbuf is most or least significant.
			 */
		} else {
#endif
		/*
		 * Force to long boundary so we do longword aligned
		 * memory operations.  It is too hard to do byte
		 * adjustment, do only word adjustment.
		 */
		if (((int)w&0x2) && mlen >= 2) {
			sum += *w++;
			mlen -= 2;
		}

		/*
		 * Do as much of the checksum as possible 32 bits at at time.
		 * In fact, this loop is unrolled to make overhead from
		 * branches &c small.
		 *
		 * We can do a 16 bit ones complement sum 32 bits at a time
		 * because the 32 bit register is acting as two 16 bit
		 * registers for adding, with carries from the low added
		 * into the high (by normal carry-chaining) and carries
		 * from the high carried into the low on the next word
		 * by use of the adwc instruction.  This lets us run
		 * this loop at almost memory speed.
		 *
		 * Here there is the danger of high order carry out.
		 */
		while ((mlen -= 32) >= 0) {
			ADDL; ADDX; ADDX; ADDX; ADDX; ADDX; ADDX; ADDX;
			ADD_LASTCARRY;
		}
		if ((mlen += 32) >= 16) {
			ADDL; ADDX; ADDX; ADDX;
			ADD_LASTCARRY;
			mlen -= 16;
		}
		if (mlen >= 8) {
			ADDL; ADDX;
			ADD_LASTCARRY;
			mlen -= 8;
		}
		/*
		 * Now eliminate the possibility of carry-out''s by
		 * folding back to a 16 bit number (adding high and
		 * low parts together.)  Then mop up trailing words
		 * and maybe an odd byte.
		 */
		FOLD_SUM;

		while ((mlen -= 2) >= 0)
			sum += *w++;
		if (mlen == -1)
			sum += *(u_char *)w << 8;
#ifdef PM2
		}
#endif
		if (len == 0)
			break;
		/*
		 * Locate the next block with some data.
		 * If there is a word split across a boundary we
		 * will wrap to the top with mlen == -1 and
		 * then add it in shifted appropriately.
		 */
		for (;;) {
			if (m == 0) {
				printf("cksum: out of data\n");
				goto done;
			}
			if (m->m_len)
				break;
			m = m->m_next;
		}
	}
done:
	/*
	 * Add together high and low parts of sum
	 * and carry to get cksum.
	 */
	FOLD_SUM;

	return (sum ^ 0xffff);
}
