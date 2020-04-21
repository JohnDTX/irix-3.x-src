/*--------------------------------------------------------------------------*/
/*									    */
/*			    PROPRIETARY INFORMATION			    */
/*									    */
/*  	These  coded instructions, statements,  and computer programs	    */
/*	contain unpublished proprietary information and are protected	    */
/*	by Federal copyright law.  They may not be disclosed to third	    */
/*	parties or copied or duplicated in any form without the prior	    */
/*	written consent of Silicon Graphics, Inc.			    */
/*									    */
/*--------------------------------------------------------------------------*/


/*
 * cmds.h
 *	-- GE and FBC opcodes reflecting GE rev2 and Beta microcode.
 *	   define GFBETA for rev2
 */

#define	GEwaiting		-1

# ifdef GFBETA

#define	GEpopmm			0x00
#define	GEloadmm		0x01
#define	GEstoremm		0x03
#define	GEpushmm		0x04
#define	GEloadviewport		0x05
#define	GEsethitmode		0x06
#define	GEclearhitmode		0x07
#define	GEpassthru		0x08
#define GEpushviewport		0x09
#define GEpopviewport		0x0a	/* 10	*/
#define GEstoreviewport		0x0b	/* 11	*/
#define GEreconfigure		0x0c	/* 12	*/
#define GEswitchpipes		0x0d	/* 13	*/
#define GEnoop			0x0f	/* 15	*/
#define	GEmove			0x10	/* 16	*/
#define	GEdraw			0x11	/* 17	*/
#define	GEpoint			0x12	/* 18	*/
#define	GEcurve			0x13	/* 19	*/
#define GEmoverel		0x14	/* 20	*/
#define GEdrawrel		0x15	/* 21	*/
#define GEpointrel		0x16	/* 22	*/
#define GEmidmm0		0x20	/* 32	*/
#define GEmidmm1		0x21	/* 33	*/
#define GEmidmm2		0x22	/* 34	*/
#define GEmidmm3		0x23	/* 35	*/
#define GEfirstmm0		0x24	/* 36	*/
#define GEfirstmm1		0x25	/* 37	*/
#define GEfirstmm2		0x26	/* 38	*/
#define GEfirstmm3		0x27	/* 39	*/
#define GElastmm0		0x28	/* 40	*/
#define GElastmm1		0x29	/* 41	*/
#define GElastmm2		0x2a	/* 42	*/
#define GElastmm3		0x2b	/* 43	*/
#define GEcompletemm0		0x2c	/* 44	*/
#define GEcompletemm1		0x2d	/* 45	*/
#define GEcompletemm2		0x2e	/* 46	*/
#define GEcompletemm3		0x2f	/* 47	*/
#define	GEmovepoly		0x30	/* 48	*/
#define	GEdrawpoly		0x31	/* 49	*/
#define	GEclosepoly		0x33	/* 51	*/
#define GEcurvepoly		0x37	/* 55	*/
#define GEtransformpoint	0x38	/* 56	*/

# else  GFBETA

#define	GEpopmm			0x00
#define	GEloadmm		0x01
#define	GEmultmm		0x02
#define	GEstoremm		0x03
#define	GEpushmm		0x04
#define	GEloadviewport		0x05
#define	GEsethitmode		0x06
#define	GEclearhitmode		0x07
#define	GEpassthru		0x08
#define	GEmove			0x10	/* 16 */
#define	GEdraw			0x11	/* 17 */
#define	GEpoint			0x12	/* 18 */
#define	GEcurve			0x13	/* 19 */
#define	GEmovepoly		0x30	/* 48 */
#define	GEdrawpoly		0x31	/* 49 */
#define	GEclosepoly		0x33	/* 51 */

# endif GFBETA

#define FBCinitfbc		0x00
#define	FBCsethitmode		0x06
#define	FBCclearhitmode		0x07
#define FBCforcecompletion	0x08
#define FBCreadrgb		0x0a	/* 10	*/
#define FBCstorevp		0x0b	/* 11	*/
#define FBCdrawpixels		0x0d	/* 13	*/
#define FBCreadpixels		0x0e	/* 14 ( 16-bit mode )	*/
#define	FBCmove			0x10	/* 16	*/
#define FBCdraw			0x11	/* 17	*/
#define	FBCpoint		0x12	/* 18	*/
#define FBCcolor		0x14	/* 20	*/
#define FBCwrten		0x15	/* 21	*/
#define FBCconfig		0x16	/* 22	*/
#define FBCloadmasks		0x17	/* 23	*/
#define FBCfbviewport		0x18	/* 24	*/
#define FBClinestyle		0x19	/* 25	*/
#define FBCcharposnabs		0x1a	/* 26	*/
#define FBCcharposnrel  	0x1b	/* 27	*/
#define FBCdrawchars		0x1c	/* 28	*/
#define FBCselectcursor		0x1d	/* 29	*/
#define FBCdrawcursor		0x1e	/* 30	*/
#define FBCundrawcursor		0x1f	/* 31	*/
#define FBClinestipple		0x20	/* 32	*/
#define FBCpolystipple		0x21	/* 33	*/
#define FBCsaveregs		0x22	/* 34	*/
#define FBCunsaveregs		0x23	/* 35	*/
#define FBCfeedback		0x25	/* 37	*/
#define FBCeof			0x26	/* 38	*/
#define FBCreadcharposn	 	0x27	/* 39	*/
#define FBCboundingbox		0x28	/* 40	*/
#define FBCnamestack		0x29	/* 41	*/
#define FBCloadname		0x2a	/* 42	*/
#define FBCincname		0x2b	/* 43	*/
#define	FBCmovepoly		0x30	/* 48	*/
#define	FBCdrawpoly		0x31	/* 49	*/
#define	FBCclosepoly		0x33	/* 51	*/
#define FBCxformpt		0x38	/* 56	*/
#define FBCblockfill		0x39	/* 57	*/
#define FBCcopyscreen		0x3d	/* 61	*/

/* diagnostics	*/

#define FBCdumpup		0x24	/* 36	*/
#define FBCtestram		0x36	/* 54	*/
#define BPCreadbus		0x37	/* 55	*/
#define FBCdumpram		0x3a	/* 58	*/
#define BPCloadreg		0x3b	/* 59	*/
#define BPCcommand		0x3c	/* 60	*/
#define FBCdblfeed		0x3e	/* 62	*/
