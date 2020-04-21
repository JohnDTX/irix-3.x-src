/*
**	This file describes the irisboot header
**
**	header of 8 longs	magic number 0604 or 0605 (last flag)
**				doloadheader
**				doloadsymbols
**				headerloc
**				loadloc
**				initstack
**
*/

#define IBMAGIC	0701
#define IBLAST	0707
#define FMAGIC  0407

struct ibhdr {
	long fmagic;
	long doloadheader;
	long doloadsymbols;
	long headerloc;
	long loadloc;
 	long initstack;
	long space1;
	long space2;
};


