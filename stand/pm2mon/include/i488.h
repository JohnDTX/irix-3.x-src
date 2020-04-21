/*	defines for National Instruments GPIB-796 board			*/
/*	that provides an IEEE-488 interface to the Multibus		*/

/*	Device Strucure for Multibus system w/ PM1 board (ie byte	*/
/*	swapping is taken care of)					*/

struct ni488device {
	u_char ni_isr1_imr1;	/* interrupt status; interrupt mask	*/
	u_char ni_dir_cdor;	/* data in; command/data out		*/
	u_char ni_spsr_spmr;	/* serial poll status; serial poll mode */
	u_char ni_isr2_imr2;	/* interrupt status 2; interrupt mask 2	*/
	u_char ni_cptr_auxmr;	/* command pass through; auxiliary mode */
	u_char ni_adsr_admr;	/* address status; address mode		*/
	u_char ni_adr1_eosr;	/* address register 1; end of string reg*/
	u_char ni_adr0_adr;	/* address register 0; address register	*/
	u_char ni_bcr1;		/* byte count register high byte	*/
	u_char ni_bcr0;		/* byte count register low byte		*/
	u_char ni_ccfr;		/* carry cycle function			*/
	u_char ni_sr_cr0;	/* status; control register 0		*/
	u_char ni_acr1;		/* DMA address middle byte		*/
	u_char ni_acr0;		/* DMA address low byte			*/
	u_char ni_cr1;		/* control register 1			*/
	u_char ni_acr2;		/* DMA address high byte		*/
};

/*	Bits, Bytes, and commands		*/

/*	CR0 - Control Register 0 bits		*/
#define CR0_GO		0x01		/* go - execute command		*/
#define CR0_DMAEN	0x02		/* enable DMA			*/
#define CR0_LMRESET	0x04		/* local master reset		*/
#define CR0_IE		0x08		/* interrupt enable 		*/
#define CR0_MREAD	0x10		/* DMA from memory to GPIB	*/
#define CR0_TEST	0x20		/* test - isolate from GPIB	*/
#define CR0_ECC		0x40		/* enable carry cycle		*/

/*	CR1 - Control Register 1 bits		*/
#define CR1_BRST50	0x04		/* burst timeout ~50us		*/
#define CR1_BRST400	0x07		/* burst timeout ~400us		*/
#define CR1_SC		0x04		/* board is system controller	*/
#define CR1_NBRST	0x40		/* not burst mode		*/
#define CR1_MIE		0x80		/* master interrupt enable	*/

/*	AUXMR - auxiliary commands and hidden registers			*/
/*	Auxiliary commands:						*/
#define AUX_CMD		0x00		/* bits to show command		*/
#define AC_PON		0x00 | AUX_CMD	/* power on			*/
#define AC_CRESET	0x02 | AUX_CMD	/* chip reset			*/
#define AC_FINHAND	0x03 | AUX_CMD	/* finish handshake		*/
#define AC_SENDEOI	0x06 | AUX_CMD	/* send eoi on last byte	*/

/*	Hidden Registers						*/

/*		ICR							*/
#define AUX_ICR		0x20		/* internal counter register	*/
#define AUXICR_MAGIC	0x05 | AUX_ICR	/* internal counter register	*/

/*		PPR							*/
#define AUX_PPR		0x60		/* Parallel Poll register	*/
#define AUXPPR_UNCONFIG	0x10 | AUX_PPR	/* don't do // polls		*/

/*		AUXRA							*/
#define AUX_AREG	0x80		/* Auxiliary register A		*/
#define AUXA_HOLD	0x01 | AUX_AREG	/* Holdoff on all data		*/

/*		AUXRB							*/
#define AUX_BREG	0xA0		/* Auxiliary register B		*/
#define AUXB_INV	0x08 | AUX_BREG	/* invert INT pin from TLC	*/
#define AUXB_TRI	0x04 | AUX_BREG	/* tri-state timming		*/

/*		AUXRE							*/
#define AUX_EREG	0xC0		/* Auxiliary register E		*/

/* 	End of hidden registers						*/

/*	ADMR - Address Mode Register					*/
#define ADMR_NORMAL	0x01		/* Normal dual addressing	*/
#define ADMR_TRMMAGIC	0x30		/* Transmit/Receive magic	*/

/*	ADR - Address register bits		*/
#define ADR_ADRMASK	0x1F		/* address mask			*/
#define ADR_DISLISTEN	0x20		/* disable listener function	*/
#define ADR_DISTALK	0x40		/* disable talker function	*/
#define ADR_SEL2	0x80		/* select secondary address	*/

/*	ADSR - Addressed Status Register				*/
#define ASR_TALKER	0x02		/* gift of the gab		*/
#define ASR_LISTENER	0x04		/* a good child			*/

/*	SPSR - Serial Poll register					*/
#define SP_PEND		0x40		/* serial poll is pending	*/
#define SP_REQ		0x40		/* request serial poll 		*/

/*	I1 - Interrupt status and mask #1  ( EI - enable interrupt)	*/
#define I1_DI		0x01		/* data in			*/
#define I1_DO		0x02		/* data out			*/
#define I1_ERR		0x04		/* data out			*/
#define I1_DEC		0x08		/* device clear			*/
#define I1_ENDIE	0x10		/* EI on END			*/

/*	I2 - Interrupt status and mask #2				*/
#define I2_ASTATECHG	0x01		/* EI on addressed state change	*/
#define I2_DMAIEN	0x10		/* enable dma input		*/
#define I2_DMAOEN	0x20		/* enable dma output		*/
#define I2_INT		0x80		/* interrupt from GPIB		*/

/*	SR - Status Register						*/
#define SR_NEXMEM	0x10		/* DMA to non-existant memory	*/
#define SR_NFIN		0x20		/* not finished			*/
#define SR_DONE		0x80		/* DMA is done			*/
