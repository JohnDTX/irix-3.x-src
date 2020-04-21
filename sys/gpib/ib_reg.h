/*
 * definitions for
 *	National Instruments GPIB-796 part number 320010-01
 * ieee488 interface.
 */
# define def define


# def NIBIREGS		12		/*#input regs in GPIB-796*/
# def NIBOREGS		16		/*#output regs in GPIB-796*/
# def NIBXREGS		4		/*#aux regs in GPIB-796*/


/*----- register definitions (input side)*/

/*
 * (DIR) data in register --
- used directly for non-dma input from GPIB.
- used indirectly for dma input from GPIB.
- GPIB handshake is heldoff until IB_DIR is
  examined.  handshake may be set automatic
  or not by IB_AUXMR.AUXMR_REGA.
- read-only, TLC.
 */
# def IB_DIR	0x0

/*
 * (ISR1) interrupt status register 1 --
- flags for various intr conditions, whether
  or not enabled.
- read-only, cleared on read, TLC.
 */
# def IB_ISR1	0x1
# def	ISR1_CPTI	(1<<7)		/*(CPT) cmd pass-thru intr*/
# def	ISR1_APTI	(1<<6)		/*(APT) addr pass-thru intr*/
# def	ISR1_DETI	(1<<5)		/*(DET) dev exec-trig intr*/
# def	ISR1_ENDI	(1<<4)		/*(END_RX) end rcvd intr*/
# def	ISR1_DCI	(1<<3)		/*(DEC) dev clr intr*/
# def	ISR1_ERRI	(1<<2)		/*(ERR) err intr*/
# def	ISR1_ORI	(1<<1)		/*(DO) rdy for output intr*/
# def	ISR1_IRI	(1<<0)		/*(DI) GPIB input rdy*/

/*
 * (ISR2) interrupt status register 2 --
- more flags for various intr conditions,
  whether or not enabled.
- read-only, cleared on read, TLC.
 */
# def IB_ISR2	0x2
# def	ISR2_ANYI	(1<<7)		/*(INT) any intr*/
# def	ISR2_SRQI	(1<<6)		/*(SRQI) SRQ intr*/
# def	ISR2_LO		(1<<5)		/*(LOK) lockout state*/
# def	ISR2_RMT	(1<<4)		/*(REM) remote state*/
# def	ISR2_CRI	(1<<3)		/*(CO) ready for cmd intr*/
# def	ISR2_LOCHGI	(1<<2)		/*(LOKC) lockout change intr*/
# def	ISR2_RMTCHGI	(1<<1)		/*(REMC) rmt status change intr*/
# def	ISR2_ADCHGI	(1<<0)		/*(ADSC) adrsd status change intr*/

/*
 * (SPSR) serial poll status register --
- when not active ctlr, used as status bit for
  pending service request (via IB_SPMR).
- when active ctlr, read as response to a spoll.
- read-only, TLC.
 */
# def IB_SPSR	0x3
# def	SPSR_POLLING	(1<<6)		/*(PEND) spoll-pending*/
# def	SPSR_REPLYF	(~SPSR_POLLING)	/*(S8,S5-S1) spoll reply (in)*/

/*
 * (ADSR) address status register --
- state of address recognition sequence.
- whether or not TLC is CIC.
- read-only, TLC.
 */
# def IB_ADSR	0x4
# def	ADSR_CIC	(1<<7)		/*(CIC) ctlr-in-charge*/
# def	ADSR_ATN_	(1<<6)		/*(ATN*) GPIB ATN**/
# def	ADSR_SPMS	(1<<5)		/*(SPMS) spoll mode state*/
# def	ADSR_LPAS	(1<<4)		/*(LPAS) lstnr primary adrsd state*/
# def	ADSR_TPAS	(1<<3)		/*(TPAS) tlkr primary adrsd state*/
# def	ADSR_LA		(1<<2)		/*(LA) lstnr active state*/
# def	ADSR_TA		(1<<1)		/*(TA) tlkr active state*/
# def	ADSR_MINOR	(1<<0)		/*(MJMN) major-minor adrsd state*/

/*
 * (CPTR) command pass-through register --
- when not active ctlr, used to receive undefined
  GPIB cmds (feature controlled by IB_AUXMR.AUXMR_REGB),
  and extended address bytes (in dual extended adrs mode,
  feature controlled by IB_ADMR).
- when conduction a ppoll, read as the ppoll response.
- read-only, TLC.
 */
# def IB_CPTR	0x5

/*
 * (ADR0, ADR1) address register --
- used to specify the address or addresses
  to which the TLC responds.  depends on the
  addressing mode (IB_ADMR).
- may be set by poking IB_ASLR.  thus IB_ADR0,
  IB_ADR1 are effectively hidden registers
  which happen to be readable.
- read-only, TLC.
 */
# def IB_ADR0	0x6
# def IB_ADR1	0x7
# def	ADR1_EOI	(1<<7)		/*(EOI) end or identify*/

/*
 * (BCR0, BCR1) byte count register --
- set to negative byte count for dma xfers.
- read as amt unfinished on dma err.
- read/write, cleared by local master reset or pon.
 */
# def IB_BCR0	0x8	/*bcr<7:0>*/
# def IB_BCR1	0x9	/*bcr<15:8>*/

/*
 * (SR) status register --
- various status bits.
- some are cleared on read.
- bits <1:0> are undefined.
- read-only.
 */
# def IB_SR	0xA
# def	SR_DMADONE	(1<<7)		/*(DONE) dma done*/
# def	SR_INTR_	(1<<6)		/*(IR*) TLC IR**/
# def	SR_DMAFIN_	(1<<5)		/*(FIN*) TLC FIN**/
# def	SR_NXM		(1<<4)		/*(NEX) non-existent mem*/
# def	SR_BIT0_	(1<<3)		/*(DIO1*) GPIB DIO line 1*/
# def	SR_NDAC_	(1<<2)		/*(NDAC*) GPIB NDAC**/
# def	SR_JUNK1	(1<<1)
# def	SR_JUNK0	(1<<0)

/*
 * (CCFR) carry cycle function register --
- used to specify an automatic action (new contents
  of IB_AUXMR) at the end of a dma xfer.
- feature controlled by IB_CR0.
- read/write.
 */
# def IB_CCFR	0xB
/*----- */


/*----- register definitions (output side)*/

/*
 * (CDOR) data out register --
- used directly for non-dma output to GPIB.
- used indirectly for dma output to GPIB.
- handshake is automatic when IB_DOR is written.
- write-only, TLC.
 */
# def IB_DOR	0x0

/*
 * (IMR1) interrupt mask register 1 --
- intr enables for various conditions.
- write-only, TLC.
 */
# def IB_IMR1	0x1
# def IMR1_MAG_PIO (ISR1_CPTI|ISR1_APTI|ISR1_DETI|ISR1_DCI\
	|ISR1_ERRI|ISR1_IRI|ISR1_ORI)
# def IMR1_MAG_DMAI (ISR1_ENDI|ISR1_ERRI)
# def IMR1_MAG_DMAO (ISR1_ERRI)

/*
 * (IMR2) interrupt mask register 2 --
- more intr enables for various conditions.
- write-only, TLC.
 */
# def IB_IMR2	0x2
# def	IMR2_DMAOEN	(1<<5)		/*(DMAO) dma output enab*/
# def	IMR2_DMAIEN	(1<<4)		/*(DMAI) dma input enab*/
# def IMR2_MAG_PIO (ISR2_SRQI|ISR2_CRI\
	|ISR2_LOCHGI|ISR2_RMTCHGI|ISR2_ADCHGI)
# def IMR2_MAG_DMAI (IMR2_DMAIEN)
# def IMR2_MAG_DMAO (IMR2_DMAOEN)

/*
 * (SPSR) serial poll mode register --
- used to specify response to spoll.
- used to request service.
- write-only, TLC.
 */
# def IB_SPMR	0x3
# def	SPMR_REPLYF	(~SPRM_RSV)	/*(S8,S5-S1) spoll reply (out)*/
# def	SPMR_RSV	(1<<6)		/*(rsv) request service*/

/*
 * (ADMR) address mode register --
- used to specify address recognition method.
- the actual address bits come from IB_ADR0, IB_ADR1.
- write-only, TLC.
 */
# def IB_ADMR	0x4
# def	ADMR_DEAF	(1<<7)		/*(ton) talk only*/
# def	ADMR_MUTE	(1<<6)		/*(lon) listen only*/
# def	ADMR_XRF	(03<<4)		/*(TRM1-0) xmit/rcv mode*/
# def	  ADMR_XRF_MAG	(ADMR_XRF)	/*magic# for above (send and rcv)*/
# def	ADMR_AMF	(03<<0)		/*(ADM1-0) address mode*/
# def	  ADMR_AMF_MULE	(00<<0)		/*tlk- or lstn- only*/
# def	  ADMR_AMF_DUAL	(01<<0)		/*normal dual adrsg*/
# def	  ADMR_AMF_SEXT	(02<<0)		/*extended sgl adrsg*/
# def	  ADMR_AMF_DEXT	(03<<0)		/*extended dual adrsg*/
# def ADMR_MAG (ADMR_XRF_MAG|ADMR_AMF_DUAL)

/*
 * (AUXMR) auxiliary mode register --
- used to access (write-only) hidden registers.
- write-only, TLC.
 */
# def IB_AUXMR	0x5
# def	AUXMR_SLF	(07<<5)		/*(CNT2-CNT0) reg select*/
# def	AUXMR_ARGF	(037<<0)	/*(COM4-COM0) value for reg*/

/* operation select field of IB_AUXMR */
# def AUXMR_OP		(00<<5)
# def	OP_IXP		(000<<0)	/*immed exec pon*/
# def	OP_RESET	(002<<0)	/*chip reset*/
# def	OP_FHS		(003<<0)	/*finish handshake*/
# def	OP_TRIG		(004<<0)	/*trigger*/
# def	OP_RTL		(005<<0)	/*return to local*/
# def	OP_GTL		(015<<0)	/*same as OP_RTL*/
# def	OP_XEOI		(006<<0)	/*send EOI*/
# def	OP_IV2C		(007<<0)	/*invalid 2ary cmd or adrs byte*/
# def	OP_V2C		(017<<0)	/*valid 2ary cmd or adrs byte*/
# def	OP_CPPF		(001<<0)	/*clr ppoll flag*/
# def	OP_SPPF		(011<<0)	/*set ppoll flag*/
# def	OP_TCA		(021<<0)	/*take ctrl asynch*/
# def	OP_TCS		(022<<0)	/*take ctrl synch*/
# def	OP_TCSE		(032<<0)	/*take ctrl synch on end*/
# def	OP_GTSB		(020<<0)	/*go to standby*/
# def	OP_LSTN		(023<<0)	/*listen*/
# def	OP_LICM		(033<<0)	/*listen in contin mode*/
# def	OP_LUL		(034<<0)	/*local unlisten*/
# def	OP_XPP		(035<<0)	/*exec ppoll*/
# def	OP_SIFC		(036<<0)	/*set IFC*/
# def	OP_CIFC		(026<<0)	/*clr IFC*/
# def	OP_SREN		(037<<0)	/*set REN*/
# def	OP_CREN		(027<<0)	/*clr REN*/
# def	OP_DSC		(024<<0)	/*disable system ctrl*/

/* (AUXRA) reg A select field of IB_AUXMR */
# def AUXMR_REGA	(04<<5)
# def	REGA_8BEND	(01<<4)		/*(BIN) 8-bit end chr*/
# def	REGA_XEOS	(01<<3)		/*(XEOS) xmit END with EOS*/
# def	REGA_REOS	(01<<2)		/*(REOS) show END when rcv EOS*/
# def	REGA_HOFF	(03<<0)		/*(HLDE,HLDA) holdoff function*/
# def	  REGA_HOFF_NOT	(00<<0)		/*normal handshake*/
# def	  REGA_HOFF_ALL	(01<<0)		/*holdoff on all data*/
# def	  REGA_HOFF_END	(02<<0)		/*holdoff on END*/
# def	  REGA_HOFF_FAK	(03<<0)		/*contin (ignore data, hoff on END)*/
# def REGA_MAG	(REGA_8BEND)

/* (AUXRB) reg B select field of IB_AUXMR */
# def AUXMR_REGB	(05<<5)
# def	REGB_ISS	(01<<4)		/*(ISS) individ status select*/
# def	REGB_ISENSE	(01<<3)		/*(INV) polarity of INT pin*/
# def	REGB_TRI	(01<<2)		/*(TRI) 3-state timing*/
# def	REGB_SPEOI	(01<<1)		/*(SPEOI) send spoll EOI*/
# def	REGB_CPTEN	(01<<0)		/*(CPT_ENAB) cmd pass-thru enab*/
# def REGB_MAG	(REGB_ISS|REGB_ISENSE|REGB_TRI|REGB_CPTEN)

/* (PPR) parallel poll reg select field of IB_AUXMR */
# def AUXMR_PPR		(03<<5)
# def	PPR_UNC		(01<<4)		/*(U) ppoll unconfigure*/
# def	PPR_SSENSE	(01<<3)		/*(S) status bit polarity*/
# def	PPR_REPLYF	(07<<0)		/*(P3-P1) ppoll reply line#-1*/

/* (AUXRE) reg E select field of IB_AUXMR */
# def AUXMR_REGE	(06<<5)
# def	REGE_DHDC	(01<<1)		/*(DHDC) DAC holdoff on DCAS*/
# def	REGE_DHDT	(01<<0)		/*(DHDT) DAC holdoff on DTAS*/
# def REGE_MAG	0

/* (ICR) clk divider reg field of IB_AUXMR */
# def AUXMR_CDR		(01<<5)
# def	CDR_CNTF	(017<<0)	/*clk<3:0> divider*/
# def	  CDR_CNTF_MAG	(05<<0)		/*magic# for above (5Mhz)*/
# def CDR_MAG	(CDR_CNTF_MAG)

/*
 * (ADR) address select register --
- used to write IB_ADR0, IB_ADR1.
- write-only, TLC.
 */
# def IB_ASLR	0x6
# def	ASLR_SLF	(01<<7)		/*(ARS) address reg select*/
# def	ASLR_DT		(1<<6)		/*(DT) disable talker*/
# def	ASLR_DL		(1<<5)		/*(DL) disable listener*/
# def	ASLR_ADF	(037<<0)	/*(AD5-1) address bits*/

/*
 * (EOSR) end of string register --
- used to specify the value used for end of
  message recognition (in and out).
- feature controlled by IB_AUXMR.AUXMR_REGA.
- write-only, TLC.
 */
# def IB_EOSR	0x7

/*
 * (CR0) control register 0 --
- control and go functions for dma.
- write-only, cleared by pon.
 */
# def IB_CR0	0xA
# def	CR0_CCEN	(1<<6)		/*(ECC) carry cycle enab*/
# def	CR0_TESTEN	(1<<5)		/*(TEST) test mode enab*/
# def	CR0_DMAO	(1<<4)		/*(MEMRD) dma is mem read*/
# def	CR0_DMAIE	(1<<3)		/*(FIN_IE) dma finished ie*/
# def	CR0_LMRESET	(1<<2)		/*(LMR) local master reset*/
# def	CR0_DMAEN	(1<<1)		/*(DMA_EN) dma enab*/
# def	CR0_GO		(1<<0)		/*(GO) start dma*/
# def CR0_MAG_PIO (0)
# def CR0_MAG_DMAI (CR0_DMAIE|CR0_CCEN|CR0_DMAEN)
# def CR0_MAG_DMAO (CR0_DMAIE|CR0_CCEN|CR0_DMAEN|CR0_DMAO)

/*
 * (CR1) control register 1 --
- more dma control functions.
- write-only, cleared by local master reset or pon.
 */
# def IB_CR1	0xF
# def	CR1_MIE		(1<<7)		/*(MIE) master ie*/
# def	CR1_BMEN_	(1<<6)		/*(BURST_MODE*) dma burst mode enab*/
# def	CR1_CBRQEN	(1<<5)		/*(CBRQ_EN) common bus request enab*/
# def	CR1_SC		(1<<3)		/*(SC) system ctlr*/
# def	CR1_BMTF	(07<<0)		/*(BT2-BT0) burst mode timeout bits*/
# def	  CR1_BMTF_0	(00<<0)		/*  1.6 +- 1.6 usec*/
# def	  CR1_BMTF_1	(01<<0)		/*  4.8 +- 1.6 usec*/
# def	  CR1_BMTF_2	(02<<0)		/* 10.2 +- 1.6 usec*/
# def	  CR1_BMTF_3	(03<<0)		/* 24.0 +- 1.6 usec*/
# def	  CR1_BMTF_4	(04<<0)		/* 49.6 +- 1.6 usec*/
# def	  CR1_BMTF_5	(05<<0)		/*100.8 +- 1.5 usec*/
# def	  CR1_BMTF_6	(06<<0)		/*203.2 +- 1.6 usec*/
# def	  CR1_BMTF_7	(07<<0)		/*408.0 +- 1.6 usec*/
# def CR1_MAG	(CR1_MIE|CR1_BMEN_|CR1_BMTF_0)

/*
 * (ACR0, ACR1, ACR2) memory address registers --
- for dma xfers.
- write-only, cleared by local master reset or pon.
 */
# def IB_MAR0	0xC	/*acr<7:0>*/
# def IB_MAR1	0xD	/*acr<15:8>*/
# def IB_MAR2	0xE	/*acr<23:16>*/
/*----- */


# undef def
