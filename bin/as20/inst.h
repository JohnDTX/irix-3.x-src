typedef unsigned long addr_t;
typedef struct {unsigned short dummy,inst;} instw_t;

union addrmode_u {
	struct {
		unsigned
			Dn:1,
			An:1,
			:2,
			/* address register indirect */
			Anind:1,
			/* An ind. pre A_incr */
			Anindpi:1,
			/* An ind. post decr */
			Anindpd:1,
			/* An with disp */
			Andisp:1,
			/* An indexed - any mode A_*/
			AnIndexed:4,
			/* absolutes */
			absw:1,
			absl:1,
			dispb:1,
			dispw:1,
			displ:1,
			/* Immediates - with ranges */
			imm0t7:1,
			imm1t8:1,
			/********
			imm1t32:1,
			immb:1,
			*/
			immw:3,
			imml:1,:1,
			:3,
			/* pc with displacement */
			Pcdisp:1,
			/* pc any indexed mode */
			PcIndexed:4;
		} basemode;
	struct 
	{
		unsigned
			anyreg:2,:2,
			anyind:4,
			anyindx:4,
			anyabs:2,
			anydisp:3,
			anyimm:6,:4,
			pcdisp:1,
			pcindx:4;
	} group;
	addr_t l;
	instw_t i;
};


struct template_s {
	unsigned short inst;
	addr_t src,dest;
	int (*doinst)();
	short arg,flags;
	};

typedef struct template_s template_t;

template_t template[0x500];

/* the instruction is hashed.  Lookup returns a pointer to an
   inst_s structure, from which is taken the instptr.  This
   is a pointer into the array of templates.  The current
   instruction is compared against the templates until either a
   match is found or a zeroed template is reached.  When a match
   is found, the source and destination address tokens, the inst
   in the template, and the single argument in the template are
   passed to the routine indicated in the template.  This routine
   generates the binary instruction.
*/
struct inst_s { char *name; struct inst_s *next; 
		struct template_s *templateptr;} ;
typedef struct inst_s inst_t ;

#define NINSTBUCKETS 0x100
inst_t *inst_bucket[NINSTBUCKETS];

/* the array of inst_s structures which are hashed and to which pointers are
   placed in the inst_bucket array 
*/
#define NBASEINSTS 0x200
inst_t base_inst[NBASEINSTS];
		
#define OPERAND_S_SIZE 0x10

/* NOTE! this is replicated in ps.h */
struct op_immabs
{
	short isnrexpr,isunsigned;
	/*
	symtabptr sym;
	*/
	struct loc_s * sym;
	/* if the sym structure is not null, the
		  address is an offset
	*/
	long addr;
} ;

struct op_immabs_wa
{
	/* word addressible version of op_immabs */
	short isnrexpr,isunsigned;
	/*
	symtabptr sym;
	*/
	struct loc_s *sym;
	union
	{
		long addrl;
		struct {short w0,w1;} addrw;
	}u;
};

struct operand_s
{
	union addrmode_u addrmode;

	struct {
		unsigned
			/* set if the immediate/abs data is an unresolved
			   label
			*/
			immabs_label:1,
			basedisp_label:1,
			outerdisp_label:1,
			imm_word:1,
			/* in these sizes, 0->unused, 
				1->none, 2->word, 3->long*/
			basedisp_size:2,
			outerdisp_size:2,

			isindexed:1,
			pcmode:1,
			isbitfield:1;
	} info;

	struct op_immabs imm;
	struct op_immabs basedisp;
	struct op_immabs outerdisp;

	struct 
	{
		/*
		unsigned 
			regno:4,
			islongindex:1,
			ispc:1,issr:1,isccr:1
			:3,
			scale:2;
		*/
		unsigned char regno,islongindex,scale,pad;
	} indexreg;

	/* bitfield structure */
	struct
	{
		unsigned width_in_reg:1,offset_in_reg:1, :6;

		unsigned char width_reg, offset_reg, width;

		unsigned long offset;
	} bf;

	/* two possible other registers in the operand */
	unsigned char reg0,reg1;
	unsigned short pad;

} operand0,operand1,extra_operand;

typedef struct operand_s operand_t;

union instword_u
{
	/* structures should reflect the bit fields in the inst words */
	struct 
	{ unsigned
		:16,
		:4,
		r9:3,
		:3,
		mode:3,
		reg:3;
	} ea_r9;
	struct 
	{ unsigned
		:16,
		:4,
		reg:3,
		mode:3,
		:6;
	} ea6;
	struct
	{ unsigned
		:16,
		:4,
		r9:3,
		:6,
		r0:3;
	} r9_r0;
	struct
	{ unsigned
		:16,
		:4,
		imm1t8:3,
		:1,
		size:2,
		mode:3,
		reg:3;
	} ea_imm;
	struct
	{ unsigned
		:16,
		:8,
		immb0:8;
	} immb0;
	struct
	{
		unsigned short dummy; /* high order word - unused */
		unsigned short instword;
	} words;
} ;

union format_word
{
	struct 
	{ unsigned
		regno:4,
		islong:1,
		scale:2,
		mustbezero:1,
		disp:8,
		:16;
	} brief_format;
	struct
	{ unsigned
		regno:4,
		islong:1,
		scale:2,
		mustbeone:1,
		bs:1,	
		is:1,
		bd_size:2,
		mustbezero:1,
		postindexed:1,
		od_size:2,
		:16;
	} full_format;
	unsigned long all;
} ;
		

#define I_NOMODE 0xff
#define I_NEEDSREG 0x80
struct instea_s 
{
	unsigned char mode,reg;
} instea[0x21];

unsigned short *bins ;

