/*
 * $Author: root $
 * $Date: 89/03/27 18:32:27 $
 * $Header: /d2/3.7/src/usr.etc/hyperchannel/hyroute/RCS/hyr_sym.h,v 1.1 89/03/27 18:32:27 root Exp $
 * $Log:	hyr_sym.h,v $
 * Revision 1.1  89/03/27  18:32:27  root
 * Initial check-in for 3.7
 * 
 * Revision 1.1  87/10/12  13:31:01  ciemo
 * Initial revision
 * 
 * $Revision: 1.1 $
 * $Source: /d2/3.7/src/usr.etc/hyperchannel/hyroute/RCS/hyr_sym.h,v $
 * $State: Exp $
 */
/*	hyr_sym.h	4.1	83/07/03	*/
/*static char h_sccsid[] = "@@(#)hyr_sym.h	2.1 Hyperchannel Routing Daemon 82/11/29";*/

struct sym {
	int	s_flags;
	char	s_name[32];
	long	s_lastok;
	long	s_fulladdr;
	unsigned short	s_dst;
	unsigned short	s_ctl;
	unsigned short	s_access;
	unsigned short	s_ngate;
	struct sym	*s_next;
	struct sym	*s_gate[32];
};

#define	HS_DIR		0x01
#define	HS_INDIR	0x02
#define	HS_GATE		0x04

	extern struct sym *curgate;
	extern struct sym *sym_head;
	extern int lex_error;
